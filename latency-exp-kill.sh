cmd2="skill deptran_server; pkill deptran_server; skill deptran_server; pkill deptran_server; skill gdb; "

rep0="20.81.128.213"
rep1="20.81.133.172"
rep2="68.154.32.129"
client0="172.206.25.207"

ssh $rep0 "$cmd2"
ssh $rep1 "$cmd2"
ssh $rep2 "$cmd2"
ssh $client0 "$cmd2"
