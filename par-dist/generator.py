import collections

config={
    # each line is one shard
    "base_0": [(101, 7001),(201, 7101),(301, 7201)],
    "base_1": [(101, 8001),(201, 8101),(301, 8201)],
    "base_2": [(101, 9001),(201, 9101),(301, 9201)],
    "base_3": [(101, 7501),(201, 7601),(301, 7701)],
    "base_4": [(101, 8501),(201, 8601),(301, 8701)],
    "base_5": [(101, 9501),(201, 9601),(301, 9701)],
    "base_6": [(101, 10501),(201, 10601),(301, 10701)],
    "base_7": [(101, 11501),(201, 11601),(301, 11701)],
    "base_8": [(101, 12501),(201, 12601),(301, 12701)],
    "base_9": [(101, 13501),(201, 13601),(301, 13701)],
}

NoleaderClientsPerCore = 3 # must 3 for Janus, and for TAPIR

# 30 for clients, 10 for the leader, 10 for p1, 10 for p2
def getNoLeaderBasedShardsInfo(prefix=""):
    ips = []
    for e in open("../ips.pub","r").readlines():
        ips.append(e.strip())
    
    n_partitions=-1
    with open('../n_partitions', 'r') as file:
        file_contents = file.read()
        n_partitions = int(file_contents)

    data = collections.defaultdict(list)
    data["n_partitions"] = n_partitions
    for i in range(n_partitions):
        data["localhost"].append(ips[n_partitions*1+i]) # localhost
        data["p1"].append(ips[n_partitions*2+i]) # p1
        data["p2"].append(ips[n_partitions*3+i]) # p2

    return data

# Janus
# 30 for clients, 10 for the leader, 10 for p1, 10 for p2
def convertNoLeaderBasedYamlMulti(trd, shards):
    shardInfo = getNoLeaderBasedShardsInfo("pub")
    servers = ""
    clients = ""
    processes = ""
    hosts = ""
    for shardIdx in range(shards):
        base = config["base_"+str(shardIdx)]

        # servers
        for i in range(trd): 
            servers += '    - ["s{idx}{n0}:{p0}", "s{idx}{n1}:{p1}", "s{idx}{n2}:{p2}"]\n'.format(
                idx=shardIdx,
                n0=base[0][0]+i, p0=base[0][1]+i,
                n1=base[1][0]+i, p1=base[1][1]+i,
                n2=base[2][0]+i, p2=base[2][1]+i,
            )
        
        # clients
        for i in range(trd):
            cc = ""
            for j in range(1,NoleaderClientsPerCore+1):
                cc += "c{idx}{n0}".format(idx=shardIdx,n0=100*j+i+1)
                if j < NoleaderClientsPerCore:
                    cc += ", "
            clients += '    - [{c}]\n'.format(c=cc)

        # processes
        for i in range(trd):
            processes += "  s{idx}{n0}: localhost_{idx}\n".format(idx=shardIdx,n0=base[0][0]+i)
            processes += "  s{idx}{n1}: p1_{idx}\n".format(idx=shardIdx,n1=base[1][0]+i)
            processes += "  s{idx}{n2}: p2_{idx}\n".format(idx=shardIdx,n2=base[2][0]+i)
        
        for i in range(trd):
            for j in range(1,NoleaderClientsPerCore+1):
                processes += "  c{idx}{n0}: c{c}_{idx}\n".format(idx=shardIdx,n0=100*j+i+1,c=j)

        # hosts
        for j in [1,2,3]:
            hostname = "c{c}_{idx}".format(idx=shardIdx,c=j)
            if j == 1:
                host = shardInfo["localhost"][shardIdx]
            elif j == 2:
            	host = shardInfo["p1"][shardIdx]
	    elif j == 3:
            	host = shardInfo["p2"][shardIdx]

            hosts += "  {hostname}: {host}\n".format(hostname=hostname,host=host)
        
        hostname = "localhost_{idx}".format(idx=shardIdx)
        host = shardInfo["localhost"][shardIdx]
        hosts += "  {hostname}: {host}\n".format(hostname=hostname,host=host)
        hostname = "p1_{idx}".format(idx=shardIdx)
        host = shardInfo["p1"][shardIdx]
        hosts += "  {hostname}: {host}\n".format(hostname=hostname,host=host)
        hostname = "p2_{idx}".format(idx=shardIdx)
        host = shardInfo["p2"][shardIdx]
        hosts += "  {hostname}: {host}\n".format(hostname=hostname,host=host)
        hosts += "\n"

    # there are {shards} shards and each shards has {trd} threads (each thread has one Paxos stream)
    file_name="no_leader_multi_paxos{trd}_{shards}.yml".format(trd=trd, shards=shards)
    content = """
site:
  server: 
{s}
  client:
{c}
process:
{p}
# process_name - host_addr map
host:
{h}  
""".format(s=servers,c=clients,p=processes,h=hosts)
    
    f = open(file_name, "w")
    f.write(content)
    f.close()


if __name__ == "__main__":
    # one-shard experiment
    # trds = 32
    # shards = 1
    # for trd in range(1, trds+1):
    #     convertLeaderBasedYamlMulti(trd=trd, shards=shards)
    #     convertNoLeaderBasedYamlMulti(trd=trd, shards=shards)
    
    # multi-shard experiment
    trds = 32
    shards = 10
    for shard in range(1,shards+1):
        for trd in range(1, trds+1):
            #convertLeaderBasedYamlMulti(trd=trd, shards=shard)
            convertNoLeaderBasedYamlMulti(trd=trd, shards=shard)
