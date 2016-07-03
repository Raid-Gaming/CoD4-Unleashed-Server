gcc -g -m32 -Wall -O1 -s -fvisibility=hidden -mtune=core2 -c *.c

gcc -m32 -s -shared -fvisibility=hidden -o antikoncept.so *.o
rm *.o
