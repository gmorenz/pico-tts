set -e

if [ ! -d "svox" ]; then
    git clone https://android.googlesource.com/platform/external/svox
else
    git -C svox pull
fi

mkdir -p src
cp svox/pico/lib/* src
# We insert a missing import, for uintptr_t
sed -i '/#include <stdio.h>/a #include <stdint.h>' src/picopal.h

mkdir -p include
cp src/*.h include

mkdir -p lang
cp svox/pico/lang/*.bin lang