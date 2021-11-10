#/bin/sh
rm -rf build
mkdir build
pwd
cd build
pwd
cmake ..
pwd
if [[ -f build.ninja ]]; then
    BUILD="ninja"
else 
    BUILD="make"
fi
$BUILD
cd ..
