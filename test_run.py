#!/usr/bin/env python3
from subprocess import call
from time import time
import argparse

run_app_     = "build/deptran_server"
config_path_ = "config/"

modes_ = [
            #"none",
            #"tpl_ww",
            #"occ",
            #"tpl_ww_paxos",
            #"occ_paxos",
            "tapir",
            #"rcc",
            #"brq"
        ]
sites_ =       [
		#"1c1s1p",
                #"2c2s1p",
                #"64c8s1p",
                "3c3s3r1p",
                #"64c8s3r1p"
	]
benchmarks_ =  [
    "rw",
    #"tpca",
    #"tpcc",
    #"tpcc_no",
    #"tpcc_pm",
    #"tpcc_dl",
    #"tpcc_os",
    #"tpcc_sl",
    #"tpccd"
]

concurrent_ = [
#    "concurrent_1",
#    "concurrent_10",
    "concurrent_100"
]

def run(m, s, b, c):
    pm = config_path_ + m + ".yml"
    ps = config_path_ + s + ".yml"
    pb = config_path_ + b + ".yml"
    pc = config_path_ + c + ".yml"

    output_path = m + '-' + s + '-' + b + ".res"
    t1 = time();
    res = "INIT"
    try:
        f = open(output_path, "w")
        print(" ".join([run_app_, "-f", pm, "-f", ps, "-f", pb, "-P", "localhost", "-d", "10"]))
        r = call([run_app_, "-f", pm, "-f", ps, "-f", pb, "-P", "localhost", "-d", "10"],
                 stdout=f, stderr=f, timeout=5*60)
        res = "OK" if r == 0 else "Failed"
    except:
        res = "Timeout"
    t2 = time();
    print("%-15s%-10s%-10s%-15s%-6s \t %.2fs" % (m, s, b, c, res, t2-t1))
    pass

def main():
    global modes_
    global sites_
    global benchmarks_
    parser = argparse.ArgumentParser();
    parser.add_argument('-m', '--mode', help='running modes', default=modes_,
                        nargs='+', dest='modes')
    parser.add_argument('-s', '--site', help='sites', default=sites_,
                        nargs='+', dest='sites')
    parser.add_argument('-b', '--bench', help='sites', default=benchmarks_,
                        nargs='+', dest='benchmarks')
    args = parser.parse_args()
    modes_ = args.modes
    sites_ = args.sites
    benchmarks_ = args.benchmarks
    print("%-15s%-10s%-10s%-15s%-6s \t %-5s" % ("mode", "site", "bench", "concurrent", "result", "time"))
    for m in modes_:
        for b in benchmarks_:
            for s in sites_:
                for c in concurrent_:
                   run(m, s, b, c)
    pass

if __name__ == "__main__":
    main()
