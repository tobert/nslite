/*
 * nslite_rpc_write.c - write JSON-RPC responses
 * Copyright 2011, Albert P. Tobey <tobert@gmail.com>
 * https://github.com/tobert/nslite
 * 
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the Artistic License 2.0.  See the file LICENSE for details.
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "nslite_rpc.h"

size_t nslite_packet_size(const char * buffer)
{
    uint8_t size[NSLITE_PREFIX_BYTES];
    size_t packet_size = 0;

    size[0] = buffer[0];
    size[1] = buffer[1];

    packet_size |= size[0] & 0xff;
    packet_size <<= 8;
    packet_size |= size[1] & 0xff;

    return packet_size;
}

