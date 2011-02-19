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

#include <yajl/yajl_parse.h>
#include <yajl/yajl_gen.h>

#include "nslite_rpc.h"

/*
static int nslite_yajl_write_map_key(void *ctx, const unsigned char *data, unsigned int size)
{
    return 1;
}

static int nslite_yajl_write_start_map(void *ctx)
{
    return 1;
}

static int nslite_yajl_write_end_map(void *ctx)
{
    return 1;
}

int nslite_gen_response(const nslite_response *rsp, char *buf)
{
    // nslite_yajl_gen_ctx *ctx;
    return 1;
}
*/
