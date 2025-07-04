export LD_LIBRARY_PATH=/lib:/home/lib:/home/rt/lib:/home/app/locallib:/var/tmp/sd/lib

killall imager_streamer
killall rRTSPServer

./imager_streamer &
./rRTSPServer &