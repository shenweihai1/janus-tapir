#git clone https://github.com/shenweihai1/janus-tapir.git
#cd janus-tapir
#git submodule init
#git submodule update
#python2 waf configure -J build # python3 not work
#./build/deptran_server -f config/tapir.yml -f config/3c3s3r1p.yml -f config/rw.yml -d 10 -P localhost
./test_run.py
