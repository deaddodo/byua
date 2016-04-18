all:
	cc -c -Wall -Werror -fPIC kaeru.c shared.c gif.c
	cc -shared -fPIC -o libkaeru.so kaeru.o shared.o gif.o
	rm -f *.o
