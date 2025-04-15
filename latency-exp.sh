rep0="127.0.0.1"
rep1="127.0.0.1"
rep2="127.0.0.1"
client0="127.0.0.1"

suffix=$1
conc=$2

ROOT=$HOME

rm ./log/*.err ./log/*.log
ssh $rep0 "cd $ROOT/janus-tapir;  ulimit -n 20000;  mkdir -p $ROOT/janus-tapir/log;  nohup  ./build/deptran_server -d 30 -f 'par-dist/latency-exp-$suffix.yml' -f 'config/brq.yml' -f 'config/tpcc.yml' -f 'config/concurrent_$conc.yml' -P 'p1_0' -p 5559 -t 10 -r '$ROOT/janus-tapir/log' 1>'$ROOT/janus-tapir/log/proc-p1_0.log' 2>'$ROOT/janus-tapir/log/proc-p1_0.err' &"
sleep 0.1

ssh $rep1 "cd $ROOT/janus-tapir;  ulimit -n 20000;  mkdir -p $ROOT/janus-tapir/log;  nohup  ./build/deptran_server -d 30 -f 'par-dist/latency-exp-$suffix.yml' -f 'config/brq.yml' -f 'config/tpcc.yml' -f 'config/concurrent_$conc.yml' -P 'localhost_0' -p 5558 -t 10 -r '$ROOT/janus-tapir/log' 1>'$ROOT/janus-tapir/log/proc-localhost_0.log' 2>'$ROOT/janus-tapir/log/proc-localhost_0.err' &"
sleep 0.1

ssh $rep2 "cd $ROOT/janus-tapir;  ulimit -n 20000;  mkdir -p $ROOT/janus-tapir/log;  nohup  ./build/deptran_server -d 30 -f 'par-dist/latency-exp-$suffix.yml' -f 'config/brq.yml' -f 'config/tpcc.yml' -f 'config/concurrent_$conc.yml' -P 'p2_0' -p 5560 -t 10 -r '$ROOT/janus-tapir/log' 1>'$ROOT/janus-tapir/log/proc-p2_0.log' 2>'$ROOT/janus-tapir/log/proc-p2_0.err' &"
sleep 0.1

sleep 10
echo 'start clients...'
ssh $client0 "cd $ROOT/janus-tapir;  ulimit -n 20000;  mkdir -p $ROOT/janus-tapir/log;  nohup  ./build/deptran_server -d 30 -f 'par-dist/latency-exp-$suffix.yml' -f 'config/brq.yml' -f 'config/tpcc.yml' -f 'config/concurrent_$conc.yml' -P 'c3_0' -p 5557 -t 10 -r '$ROOT/janus-tapir/log' 1>'$ROOT/janus-tapir/log/proc-c3_0.log' 2>'$ROOT/janus-tapir/log/proc-c3_0.err' &"
sleep 0.1

ssh $client0 "cd $ROOT/janus-tapir;  ulimit -n 20000;  mkdir -p $ROOT/janus-tapir/log;  nohup  ./build/deptran_server -d 30 -f 'par-dist/latency-exp-$suffix.yml' -f 'config/brq.yml' -f 'config/tpcc.yml' -f 'config/concurrent_$conc.yml' -P 'c1_0' -p 5555 -t 10 -r '$ROOT/janus-tapir/log' 1>'$ROOT/janus-tapir/log/proc-c1_0.log' 2>'$ROOT/janus-tapir/log/proc-c1_0.err' &"
sleep 0.1

ssh $client0 "cd $ROOT/janus-tapir;  ulimit -n 20000;  mkdir -p $ROOT/janus-tapir/log;  nohup  ./build/deptran_server -d 30 -f 'par-dist/latency-exp-$suffix.yml' -f 'config/brq.yml' -f 'config/tpcc.yml' -f 'config/concurrent_$conc.yml' -P 'c2_0' -p 5556 -t 10 -r '$ROOT/janus-tapir/log' 1>'$ROOT/janus-tapir/log/proc-c2_0.log' 2>'$ROOT/janus-tapir/log/proc-c2_0.err' &"
sleep 0.1