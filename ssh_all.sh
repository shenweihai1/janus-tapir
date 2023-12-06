leaders_pub=(
20.7.73.161
172.177.43.196
172.177.44.39
20.242.21.226
20.98.252.16
20.242.21.227
20.98.239.172
20.98.251.234
20.242.22.50
20.7.74.198
)
p1s_pub=(
20.7.75.5
20.114.170.255
20.97.161.113
20.97.161.36
20.97.162.180
20.97.162.170
20.119.180.46
20.161.4.243
20.110.131.171
20.22.210.204
)
p2s_pub=(
20.230.88.107
20.119.184.193
20.119.186.135
20.119.185.89
20.119.186.174
20.119.186.202
20.114.138.242
20.119.187.240
20.119.234.31
20.119.234.161
)
leaders=(
10.1.0.48
10.1.0.49
10.1.0.50
10.1.0.51
10.1.0.53
10.1.0.52
10.1.0.54
10.1.0.55
10.1.0.56
10.1.0.57
)
p1s=(
10.1.0.28
10.1.0.29
10.1.0.37
10.1.0.36
10.1.0.39
10.1.0.38
10.1.0.35
10.1.0.34
10.1.0.45
10.1.0.32
)
p2s=(
10.1.0.88
10.1.0.89
10.1.0.91
10.1.0.90
10.1.0.92
10.1.0.93
10.1.0.94
10.1.0.95
10.1.0.96
10.1.0.97
)

n_partitions=$1
leaders=("${leaders[@]:0:$n_partitions}")
learners=("${learners[@]:0:$n_partitions}")
p1s=("${p1s[@]:0:$n_partitions}")
p2s=("${p2s[@]:0:$n_partitions}")
allHosts=( "${leaders[@]}" "${learners[@]}" "${p1s[@]}" "${p2s[@]}" )

leaders_pub=("${leaders_pub[@]:0:$n_partitions}")
learners_pub=("${learners_pub[@]:0:$n_partitions}")
p1s_pub=("${p1s_pub[@]:0:$n_partitions}")
p2s_pub=("${p2s_pub[@]:0:$n_partitions}")
allHosts_pub=( "${leaders_pub[@]}" "${learners_pub[@]}" "${p1s_pub[@]}" "${p2s_pub[@]}" )

#cmd="ssh-keyscan $host >> $HOME/.ssh/known_hosts"
for i in "${!allHosts[@]}"
do
  host=${allHosts[$i]}
  echo "ssh to reqest to $host"
  cmd="echo 'connected'"
  ssh $host $cmd
done

for i in "${!allHosts_pub[@]}"
do
  host=${allHosts_pub[$i]}
  echo "ssh to reqest to $host"
  cmd="echo 'connected'"
  ssh $host $cmd
done
