
CFLAGS=-O2 -Wall

all: libnsfork.a nschroot
#all: nschroot nsexec

%.o: %.c
	gcc -g $(CFLAGS) -o $@ -c $<

#nsexec: nsfork.o nsexec.o
#	gcc nsfork.o nsexec.o -o $@

nschroot: nsfork.o nschroot.o
	gcc $(LDFLAGS) nsfork.o nschroot.o -o $@

libnsfork.a: nsfork.o
	ar rcs libnsfork.a nsfork.o

clean:
	rm -f *.o nschroot nsexec libnsfork.a
