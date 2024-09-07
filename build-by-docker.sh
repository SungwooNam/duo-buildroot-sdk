#!/bin/bash

docker build -f ./docker/builder.dockerfile . -t duo-builder

cat << 'EOF' > temp.sh
#!/bin/bash
echo "Building duo firmware at $PWD"
set -e

./build.sh milkv-duo-sd

EOF

chmod +x temp.sh

docker run --rm \
    --user duo \
    --privileged \
    -v $(pwd):/home/duo/work \
    -v ~/.ssh:/home/duo/.ssh \
    -e MY=my \
    -w /home/duo/work \
    duo-builder \
    ./temp.sh
