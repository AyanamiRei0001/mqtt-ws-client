# !/bin/bash

# create build directory
if [ ! -e "build/" ];
then
    echo "don't exist directory. making build directory..."
    mkdir -p build/
fi
pushd build/ &>/dev/null
cmake .. && make -j 4
popd &>/dev/null