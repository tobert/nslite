
CFLAGS=-std=c99 -O2 -Wall -Werror -Wfatal-errors -DTESTING
LDFLAGS=-lrt

all: nschroot nslite test_nslite_rpc_parse
#all: nschroot nsexec

%.o: %.c
	gcc -g $(CFLAGS) -o $@ -c $<

#nsexec: nsfork.o nsexec.o
#	gcc nsfork.o nsexec.o -o $@

nschroot: nsfork.o nschroot.o
	gcc $(LDFLAGS) nsfork.o nschroot.o -o $@

nslite: nsfork.o nslite_rpc.o nslite_rpc_parse.o nslite_rpc_gen.o nslite.o nslite_rpc.h
	gcc $(LDFLAGS) -lyajl nslite_rpc.o nslite_rpc_parse.o nslite_rpc_gen.o nsfork.o nslite.o -o $@

test_nslite_rpc_parse: nslite_rpc_parse.o test_nslite_rpc_parse.o nslite_rpc.h
	gcc $(LDFLAGS) -lyajl nslite_rpc_parse.o test_nslite_rpc_parse.o -o $@

clean:
	rm -f *.o nschroot nsexec
