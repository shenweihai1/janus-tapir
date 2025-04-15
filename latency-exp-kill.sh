cmd2="skill deptran_server; pkill deptran_server; skill deptran_server; pkill deptran_server; skill gdb; sleep 1; "

rep0="127.0.0.1"
rep1="127.0.0.1"
rep2="127.0.0.1"
client0="127.0.0.1"

ssh $rep0 "$cmd2"
ssh $rep1 "$cmd2"
ssh $rep2 "$cmd2"
ssh $client0 "$cmd2"
