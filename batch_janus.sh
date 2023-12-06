#!/bin/bash

skill deptran_server; pkill deptran_server; sleep 1

servers=(
  127.0.0.1  # nfs server
)

cmd1="" 
cmd2="skill deptran_server; pkill deptran_server; skill deptran_server; pkill deptran_server; skill gdb; sleep 1; "
cmd3="cd ~/janus && ./install_janus.sh"
cmd4="ps aux|grep deptran_server; hostname; echo ''" 
cmd5="cd ~/janus/log && rm -rf *"
cmd6="echo ''"

for host in ${servers[@]}
do
  if [ $1 == 'kill' ]; then
    echo "skill host $host"
    ssh $host "$cmd2" &
  elif [ $1 == 'install' ]; then
    echo "install janus dependencies: $host"
    ssh $host "$cmd3" &
  elif [ $1 == 'ps' ]; then
    echo "ps deptran_server: $host"
    ssh $host "$cmd4" &
    echo ""
  elif [ $1 == 'del' ]; then
    echo "delete logs: $host"
    ssh $host "$cmd5" &
    echo ""
  elif [ $1 == 'echo' ]; then
    ssh $host "$cmd6" 
    echo ""
  else
	  :
  fi 
done

# wait for completion
echo "Wait for jobs..."
FAIL=0

for job in `jobs -p`
do
    wait $job || let "FAIL+=1"
done

if [ "$FAIL" == "0" ];
then
    echo "YAY!"
else
    echo "FAIL! ($FAIL)"
fi

