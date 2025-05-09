import logging
import string
import StringIO
import time
import traceback
import random
import os
import os.path

import yaml
import boto3

from fabric.api import env, task, run, sudo, local
from fabric.api import put, execute, cd, runs_once, reboot, settings
from fabric.contrib.files import exists
from fabric.decorators import roles, parallel, hosts
from fabric.context_managers import prefix
from pylib.ec2 import instance_by_pub_ip, EC2_REGIONS, get_created_instances
from pylib.security_group import sec_grp_name, save_sec_grp

# Static ip address ranges allowed to contact the instances
ALLOWED_IP_RANGES = [ '128.122.140.0/24' ]

def Xput(*args, **kwargs):
    res = put(*args, **kwargs)
    if res.failed:
        raise RuntimeError('failed transferring {}'.format(args[0]))
    return res

@task
@roles('all')
@parallel(pool_size=10)
def disable_ssh_host_check():
    Xput('config/ssh_config', '/home/ubuntu/.ssh/config')

@task
@roles('leaders')
def ping():
    created_instances = get_created_instances()
    for region, instances in created_instances.iteritems():
        for instance in instances:
            if instance.public_ip_address == env.roledefs['leaders'][0]: 
                continue
            ip = instance.public_ip_address
            run('ping -c 3 {}'.format(ip))

@task
@roles('leaders')
def sshping():
    created_instances = get_created_instances()
    for region, instances in created_instances.iteritems():
        for instance in instances:
            if instance.public_ip_address == env.roledefs['leaders'][0]: 
                continue
            ip = instance.public_ip_address
            run('ssh {} /bin/echo here'.format(ip))


@task
@roles('leaders')
def put_janus_config(copy_configs=[]):
    execute('ec2.set_instance_roles')
    
    # copy the specified files to the server
    if isinstance(copy_configs, basestring):
        copy_configs = copy_configs.split(':')
    for c in copy_configs:
        fn = os.path.basename(c)
        dest_fn = os.path.join(env.nfs_home, "config", fn)
        put(c, dest_fn) 
    
    # write hosts config for the active aws instances
    aws_hosts_fn = 'aws_hosts.yml'
    aws_hosts = {'host': {}}
    host = aws_hosts['host']


    created_instances = get_created_instances()
    region_data = {region: [] for region in created_instances.keys()}
    leader_ip_address = env.roledefs['leaders'][0]
    for region, instances in created_instances.iteritems():
        cnt = 0
        for instance in instances:
            proc_name = "{region}-{cnt}".format(region=region, cnt=cnt)
            if instance.public_ip_address != leader_ip_address:
                host[proc_name] = instance.public_ip_address 
                region_data[region].append( (proc_name,
                                             instance.public_ip_address) )
                cnt += 1

    config_contents = StringIO.StringIO(
        yaml.dump(aws_hosts, default_flow_style=False))
    dest_fn = os.path.join(env.nfs_home, "config", aws_hosts_fn)
    put(config_contents, dest_fn) 

    aws_hosts_region_fn = 'aws_hosts_region.yml'
    contents = StringIO.StringIO(
        yaml.dump(region_data, default_flow_style=False))
    dest_fn = os.path.join(env.nfs_home, "config", aws_hosts_region_fn)
    put(contents, dest_fn)


@task
@runs_once
@roles('leaders')
def config_nfs_server():
    cmds = [
        'apt-get update -qq',
        'apt-get -y install nfs-kernel-server',
        'service nfs-kernel-server stop',
        'mkdir -p /export',
        'chmod 777 /export',
    ]
    for c in cmds:
        sudo(c)
    
    Xput('config/etc/hosts.deny', '/etc/hosts.deny', use_sudo=True)
    hosts_allow_fn = 'config/etc/hosts.allow'
    template_ha = string.Template(open(hosts_allow_fn).read())
    exports_fn = 'config/etc/exports'
    template_e = string.Template(open(exports_fn).read())

    export_options = "(rw,fsid=0,insecure,no_subtree_check,async)"
    ip_options = []
    for ip in env.roledefs['all']:
        ip_options.append("{ip}{opt}".format(ip=ip, opt=export_options))
    contents = StringIO.StringIO(template_e.substitute(host_and_options= \
                                                       ' '.join(ip_options)))
    Xput(contents, "/etc/exports", use_sudo=True)

    ip_list = ' '.join(env.roledefs['all'])
    contents = StringIO.StringIO(template_ha.substitute(ip_list=ip_list))
    logging.info("/etc/hosts.allow :\n{}".format(contents.getvalue()))
    Xput(contents, '/etc/hosts.allow', use_sudo=True)

    sudo('exportfs -a')
    reboot()


@task
@roles('all')
@parallel(pool_size=10)
def put_limits_config():
    source_fn = "config/etc/security/limits.conf"
    dest_fn = "/etc/security/limits.conf"
    Xput(source_fn, dest_fn, use_sudo=True)

@task
@roles('servers', 'leaders')
@parallel(pool_size=10)
def mount_nfs():
    try:
        sudo('mount /mnt')
    except:
        traceback.print_exc()


@task
@roles('servers', 'leaders')
#@parallel(pool_size=10)
def config_nfs_client(server_ip=None):
    if server_ip is None:
        execute('ec2.load_instances')
        instance = instance_by_pub_ip(env.roledefs['leaders'][0])
        if instance is not None and instance.public_ip_address is not None:
            server_ip = instance.public_ip_address
        else:
            raise RuntimeError("can't find leader instance or no public ip")

    logging.info("using {} for the nfs server".format(server_ip))
    fstab_fn = "config/etc/fstab"
    template = string.Template(open(fstab_fn).read())
    contents = StringIO.StringIO(template.substitute(server_ip=server_ip))
    Xput(contents, "/etc/fstab", use_sudo=True)
    #try:
    #    sudo('mount /mnt')
    #except:
    #    traceback.print_exc()



@task
@hosts('localhost')
def delete_security_group(region):
    if region is None:
        return

    client = boto3.client('ec2', region_name=region)
    try:
        client.delete_security_group(GroupName=sec_grp_name(region))
    except:
        traceback.print_exc()


@task
@hosts('localhost')
@runs_once
def setup_security_groups(regions=EC2_REGIONS.keys()):
    if 'security_groups' in env:
        return

    if isinstance(regions, basestring):
        regions = regions.split(":")

    execute('ec2.set_instance_roles')
    roledefs = env.roledefs
     
    sec_groups = {}
    for region in regions:
        client = boto3.client('ec2', region_name=region)
        res = None
        try:
            res = client.create_security_group(
                GroupName=sec_grp_name(region),
                Description='janus security group')
        except:
            traceback.print_exc()
            groups = client.describe_security_groups(GroupNames=[sec_grp_name(region)])
            group_id = groups['SecurityGroups'][0]['GroupId']
            res = {'GroupId': group_id}

        if res is not None and 'GroupId' in res:
            sec_groups[region] = res['GroupId']        
            save_sec_grp(region, sec_grp_name(region))
        else:
            raise RuntimeError("could not create security group.")
    
    env.security_groups = sec_groups
    
@task
@hosts('localhost')
def load_security_grp_ips():
    execute('ec2.load_instances')
    execute('cluster.setup_security_groups')
    sec_group_ids = env.security_groups
    created_instances = get_created_instances()
    logging.info("created instances: {}".format(created_instances))
    regions = created_instances.keys()
    logging.info("setup security group for regions: {}".format(regions))
    for region in regions:
        ec2 = boto3.resource('ec2', region_name=region)
        logging.info("adding ips to security group {}".format(sec_grp_name(region)))
        security_group = ec2.SecurityGroup(sec_group_ids[region])
        if security_group is not None:
            security_group.load()

            permissions = {
                'IpProtocol': '-1',
                'FromPort': -1,
                'ToPort': -1,
                'IpRanges': []
            }
             
            permissions['IpRanges'].append({ 'CidrIp': '0.0.0.0/0' })

            #for region2 in regions:
            #    for instance in created_instances[region2]:
            #        cidr = instance.public_ip_address + "/32"
            #        permissions['IpRanges'].append({ 'CidrIp': cidr })
            
            #for cidr in ALLOWED_IP_RANGES:
            #    permissions['IpRanges'].append({ 'CidrIp': cidr })

            logging.info("add rules to security group {}:\n{}".format(
                sec_grp_name(region), permissions))
            try:
                security_group.authorize_ingress(SourceSecurityGroupName=sec_grp_name(region))
                security_group.authorize_ingress(IpPermissions=[permissions])
            except:
                traceback.print_exc()
            
        else:
            raise RuntimeError("could not load security group")

