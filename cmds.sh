ssh 127.0.0.1 "set +o bgnice; cd /home/weihai/janus-tapir;  mkdir -p /home/weihai/janus-tapir/log;  nohup  ./build/deptran_server -b -d 10 -f 'config/tapir.yml' -f 'par-dist/no_leader_multi_paxos8_1.yml' -f 'config/rw.yml' -P 'c1_0' -p 5599 -t 10 -r '/home/weihai/janus-tapir/log' 1>'/home/weihai/janus-tapir/log/proc-c1_0.log' 2>'/home/weihai/janus-tapir/log/proc-c1_0.err' &"
sleep 0.1

ssh 127.0.0.1 "set +o bgnice; cd /home/weihai/janus-tapir;  mkdir -p /home/weihai/janus-tapir/log;  nohup  ./build/deptran_server -b -d 10 -f 'config/tapir.yml' -f 'par-dist/no_leader_multi_paxos8_1.yml' -f 'config/rw.yml' -P 'p1_0' -p 5600 -t 10 -r '/home/weihai/janus-tapir/log' 1>'/home/weihai/janus-tapir/log/proc-p1_0.log' 2>'/home/weihai/janus-tapir/log/proc-p1_0.err' &"
sleep 0.1

ssh 127.0.0.1 "set +o bgnice; cd /home/weihai/janus-tapir;  mkdir -p /home/weihai/janus-tapir/log;  nohup  ./build/deptran_server -b -d 10 -f 'config/tapir.yml' -f 'par-dist/no_leader_multi_paxos8_1.yml' -f 'config/rw.yml' -P 'c3_0' -p 5602 -t 10 -r '/home/weihai/janus-tapir/log' 1>'/home/weihai/janus-tapir/log/proc-c3_0.log' 2>'/home/weihai/janus-tapir/log/proc-c3_0.err' &"
sleep 0.1

ssh 127.0.0.1 "set +o bgnice; cd /home/weihai/janus-tapir;  mkdir -p /home/weihai/janus-tapir/log;  nohup  ./build/deptran_server -b -d 10 -f 'config/tapir.yml' -f 'par-dist/no_leader_multi_paxos8_1.yml' -f 'config/rw.yml' -P 'p2_0' -p 5593 -t 10 -r '/home/weihai/janus-tapir/log' 1>'/home/weihai/janus-tapir/log/proc-p2_0.log' 2>'/home/weihai/janus-tapir/log/proc-p2_0.err' &"
sleep 0.1

ssh 127.0.0.1 "set +o bgnice; cd /home/weihai/janus-tapir;  mkdir -p /home/weihai/janus-tapir/log;  nohup  ./build/deptran_server -b -d 10 -f 'config/tapir.yml' -f 'par-dist/no_leader_multi_paxos8_1.yml' -f 'config/rw.yml' -P 'localhost_0' -p 5598 -t 10 -r '/home/weihai/janus-tapir/log' 1>'/home/weihai/janus-tapir/log/proc-localhost_0.log' 2>'/home/weihai/janus-tapir/log/proc-localhost_0.err' &"
sleep 0.1

ssh 127.0.0.1 "set +o bgnice; cd /home/weihai/janus-tapir;  mkdir -p /home/weihai/janus-tapir/log;  nohup  ./build/deptran_server -b -d 10 -f 'config/tapir.yml' -f 'par-dist/no_leader_multi_paxos8_1.yml' -f 'config/rw.yml' -P 'c2_0' -p 5568 -t 10 -r '/home/weihai/janus-tapir/log' 1>'/home/weihai/janus-tapir/log/proc-c2_0.log' 2>'/home/weihai/janus-tapir/log/proc-c2_0.err' &"
sleep 0.1

