
CFLAGS=-O2 -Wall -fPIC

all: libnsfork.so libnsfork.a nschroot
#all: nschroot nsexec

%.o: %.c
	gcc -g $(CFLAGS) -o $@ -c $<

#nsexec: nsfork.o nsexec.o
#	gcc nsfork.o nsexec.o -o $@

nschroot: nsfork.o nschroot.o
	gcc $(LDFLAGS) nsfork.o nschroot.o -o $@

libnsfork.a: nsfork.o
	ar rcs libnsfork.a nsfork.o

libnsfork.so: nsfork.o
	gcc -shared -Wl,-soname,libnsfork.so.1 -o libnsfork.so.0.0.1 nsfork.o -lc

clean:
	rm -f *.o nschroot nsexec libnsfork.a libnsfork.so.0.0.1
