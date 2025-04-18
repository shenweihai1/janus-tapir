rm ./log/*.err ./log/*.log
ssh 20.81.133.172 "cd /home/rolis/janus-tapir;  ulimit -n 20000;  mkdir -p /home/rolis/janus-tapir/log;  nohup  ./build/deptran_server -d 30 -f 'par-dist/latency-exp.yml' -f 'config/brq.yml' -f 'config/rw.yml' -f 'config/concurrent_10.yml' -P 'p1_0' -p 5559 -t 10 -r '/home/rolis/janus-tapir/log' 1>'/home/rolis/janus-tapir/log/proc-p1_0.log' 2>'/home/rolis/janus-tapir/log/proc-p1_0.err' &"
sleep 0.1

ssh 20.81.128.213 "cd /home/rolis/janus-tapir;  ulimit -n 20000;  mkdir -p /home/rolis/janus-tapir/log;  nohup  ./build/deptran_server -d 30 -f 'par-dist/latency-exp.yml' -f 'config/brq.yml' -f 'config/rw.yml' -f 'config/concurrent_10.yml' -P 'localhost_0' -p 5558 -t 10 -r '/home/rolis/janus-tapir/log' 1>'/home/rolis/janus-tapir/log/proc-localhost_0.log' 2>'/home/rolis/janus-tapir/log/proc-localhost_0.err' &"
sleep 0.1

ssh 68.154.32.129 "cd /home/rolis/janus-tapir;  ulimit -n 20000;  mkdir -p /home/rolis/janus-tapir/log;  nohup  ./build/deptran_server -d 30 -f 'par-dist/latency-exp.yml' -f 'config/brq.yml' -f 'config/rw.yml' -f 'config/concurrent_10.yml' -P 'p2_0' -p 5560 -t 10 -r '/home/rolis/janus-tapir/log' 1>'/home/rolis/janus-tapir/log/proc-p2_0.log' 2>'/home/rolis/janus-tapir/log/proc-p2_0.err' &"
sleep 0.1

sleep 10
echo 'start clients...'
ssh 172.206.25.207 "cd /home/rolis/janus-tapir;  ulimit -n 20000;  mkdir -p /home/rolis/janus-tapir/log;  nohup  ./build/deptran_server -d 30 -f 'par-dist/latency-exp.yml' -f 'config/brq.yml' -f 'config/rw.yml' -f 'config/concurrent_10.yml' -P 'c3_0' -p 5557 -t 10 -r '/home/rolis/janus-tapir/log' 1>'/home/rolis/janus-tapir/log/proc-c3_0.log' 2>'/home/rolis/janus-tapir/log/proc-c3_0.err' &"
sleep 0.1

ssh 172.206.25.207 "cd /home/rolis/janus-tapir;  ulimit -n 20000;  mkdir -p /home/rolis/janus-tapir/log;  nohup  ./build/deptran_server -d 30 -f 'par-dist/latency-exp.yml' -f 'config/brq.yml' -f 'config/rw.yml' -f 'config/concurrent_10.yml' -P 'c1_0' -p 5555 -t 10 -r '/home/rolis/janus-tapir/log' 1>'/home/rolis/janus-tapir/log/proc-c1_0.log' 2>'/home/rolis/janus-tapir/log/proc-c1_0.err' &"
sleep 0.1

ssh 172.206.25.207 "cd /home/rolis/janus-tapir;  ulimit -n 20000;  mkdir -p /home/rolis/janus-tapir/log;  nohup  ./build/deptran_server -d 30 -f 'par-dist/latency-exp.yml' -f 'config/brq.yml' -f 'config/rw.yml' -f 'config/concurrent_10.yml' -P 'c2_0' -p 5556 -t 10 -r '/home/rolis/janus-tapir/log' 1>'/home/rolis/janus-tapir/log/proc-c2_0.log' 2>'/home/rolis/janus-tapir/log/proc-c2_0.err' &"
sleep 0.1

