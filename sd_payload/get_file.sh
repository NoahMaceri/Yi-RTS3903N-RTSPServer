source load_libs.sh

if [ "$#" -ne 1 ]; then
    echo "please enter program name"
    exit 1
fi

while killall -0 "$1"; do
    sleep 0.5
done
echo "Killed $1"

rm $1
echo "Removed $1"

echo "Start the transfer"
./socat -u TCP-LISTEN:9876,reuseaddr OPEN:$1,creat