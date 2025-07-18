if [ "$#" -ne 1 ]; then
    echo "please enter file name"
    exit 1
fi
parent_dir=$(dirname "$0")
parent_dir="$(realpath "$parent_dir/..")"
file_path="$parent_dir/$1"
echo "Start the transfer"
socat -u TCP-LISTEN:9876,reuseaddr OPEN:$file_path,creat