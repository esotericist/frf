#/bin/sh
if [[ -v MSYSCON ]]; then
    BUILD="ninja"
else 
    BUILD="make"
fi

rm -rf build
mkdir build
pwd
cd build
pwd
cmake ..
pwd
$BUILD
cd ..
