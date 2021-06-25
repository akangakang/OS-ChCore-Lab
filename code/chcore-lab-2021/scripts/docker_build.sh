#!/bin/bash

rm -rf ./build

if [ $# == 0 ]; then
    docker run -it --rm -u $(id -u ${USER}):$(id -g ${USER}) -v $(pwd):/chos -w /chos ipads/chcore_builder:v1.0 ./scripts/build.sh 
else
    docker run -it --rm -u $(id -u ${USER}):$(id -g ${USER}) -v $(pwd):/chos -w /chos ipads/chcore_builder:v1.0 ./scripts/build.sh -DTEST=\"/$1.bin\"
fi


