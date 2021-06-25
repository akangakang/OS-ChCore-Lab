#!/bin/bash

if [ ! -d "build" ]; then
    mkdir build
    cd build
    cat > simulate.sh <<EOF
#!/bin/bash

qemu-system-aarch64 -machine raspi3 -nographic -serial null -serial mon:stdio -m size=1G -kernel \$basedir/kernel.img
EOF
    chmod +x simulate.sh
    cd ..
fi


# compile kernel
echo "compiling kernel ..."
cd build

cmake -DCMAKE_LINKER=aarch64-linux-gnu-ld -DCMAKE_C_LINK_EXECUTABLE="<CMAKE_LINKER> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>" .. -G Ninja "$@"

echo "before ninja"
ninja
echo "after ninja"
aarch64-linux-gnu-nm -n kernel.img > kernel.sym
