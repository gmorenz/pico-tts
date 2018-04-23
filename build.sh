set -e

if [ ! -d "svox" ]; then
    git clone https://android.googlesource.com/platform/external/svox
else
    # git -C svox pull
    echo "Skip pull"
fi

mkdir -p src
cp svox/pico/lib/* src
# We insert a missing import, for uintptr_t
sed -i '/#include <stdio.h>/a #include <stdint.h>' src/picopal.h

cd src
gcc -fPIC -g -c *.c
ar rcs libpico.a *.o
cd ..
mv src/libpico.a .

mkdir -p include
cp src/*.h include

mkdir -p lang
cp svox/pico/lang/*.bin lang

gcc -g test.c libpico.a -I include -lm -lasound