export LD_LIBRARY_PATH=/lib:/home/lib:/home/rt/lib:/home/app/locallib:/var/tmp/sd/lib

if [ "$#" -ne 1 ]; then
    echo "Illegal number of parameters"
    exit 1
fi

./socat -u TCP-LISTEN:9876,reuseaddr OPEN:$1,creat