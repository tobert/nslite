
CFLAGS=-O2 -Wall

all: nschroot
#all: nschroot nsexec

%.o: %.c
	gcc -g $(CFLAGS) -o $@ -c $<

#nsexec: nsfork.o nsexec.o
#	gcc nsfork.o nsexec.o -o $@

nschroot: nsfork.o nschroot.o
	gcc $(LDFLAGS) nsfork.o nschroot.o -o $@

clean:
	rm -f *.o nschroot nsexec
