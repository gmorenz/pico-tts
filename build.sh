set -e

cd src
gcc -fPIC -g -c *.c
ar rcs libpico.a *.o
cd ..
mv src/libpico.a .

gcc -g test.c libpico.a -I include -lm -lasound