/*
 * nslite_rpc.h - JSON-RPC over stdio
 * Copyright 2011, Albert P. Tobey <tobert@gmail.com>
 * https://github.com/tobert/nslite
 * 
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the Artistic License 2.0.  See the file LICENSE for details.
 */

/**
 * NSLITE_PREFIX_BYTES
 * The number of bytes that prefix each message going to or from
 * nslite. The number contained is the size of the message.
 * Mimics Erlang's binary port protocol that prefixes the size in bytes.
 *
 * 2^16 should be enough for anybody ;)
 */
#define NSLITE_PREFIX_BYTES 2

#define NSLITE_JSON_MAX_ELEMENT 64

typedef struct {
    char *method;
    unsigned int id;
    char *params; /* dynamic array */
} nslite_command;

typedef struct {
    char *result;
    unsigned int id;
    char *error;
} nslite_response;

typedef struct {
    unsigned char *buffer;
    nslite_command *command;
    char *map_key;
    int array_index;
    char *element;
} nslite_yajl_parse_ctx;

typedef struct {
    nslite_response *response;
} nslite_yajl_gen_ctx;

size_t nslite_packet_size(const char *);

int nslite_parse_command(const unsigned char *, const int, nslite_command *);
int nslite_gen_response(const nslite_response *, char *);


