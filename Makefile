
CFLAGS=-O2 -Wall
LDFLAGS=-lrt

all: nschroot nslite
#all: nschroot nsexec

%.o: %.c
	gcc -g $(CFLAGS) -o $@ -c $<

#nsexec: nsfork.o nsexec.o
#	gcc nsfork.o nsexec.o -o $@

nschroot: nsfork.o nschroot.o
	gcc $(LDFLAGS) nsfork.o nschroot.o -o $@

nslite: nsfork.o nslite.o
	gcc $(LDFLAGS) nsfork.o nslite.o -o $@

clean:
	rm -f *.o nschroot nsexec
