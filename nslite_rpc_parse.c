/*
 * nslite_rpc_read.c - read JSON-RPC commands
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
#include <err.h>

#include <yajl/yajl_parse.h>
#include <yajl/yajl_gen.h>

#include "nslite_rpc.h"

static int nslite_yajl_read_integer(void *yajl_ctx, long data)
{
    printf("Integer: %li\n", data);
    nslite_yajl_parse_ctx *ctx = (nslite_yajl_parse_ctx *)yajl_ctx;

    if (ctx->map_key != NULL && strcmp(ctx->map_key, "id") == 0) {
        ctx->command->id = data;
        free(ctx->map_key);
    }
    return 1;
}

static int nslite_yajl_read_string(void *yajl_ctx, const unsigned char *data, unsigned int size)
{
    nslite_yajl_parse_ctx *ctx = (nslite_yajl_parse_ctx *)yajl_ctx;

    if (ctx->map_key != NULL) {
        if (strcmp(ctx->map_key, "method") == 0) {
            ctx->command->method = malloc(size);
            memcpy(ctx->command->method, data, size);
        }
        else if (strcmp(ctx->map_key, "id") == 0) {
            ctx->command->id = atoi((char *)data);
        }
        else if (strcmp(ctx->map_key, "params") == 0) {
            ctx->command->params = malloc(size);
            memcpy(ctx->command->params, data, size);
        }
        free(ctx->map_key);
    }
    else if (ctx->array_index >= 0) {
        printf("append array\n");
    }
    else {
        printf("ERROR: bare word\n");
    }
    return 1;
}

static int nslite_yajl_read_map_key(void *yajl_ctx, const unsigned char *data, unsigned int size)
{
    if (size > NSLITE_JSON_MAX_ELEMENT)
        err(1, "Key overflow");

    nslite_yajl_parse_ctx *ctx = (nslite_yajl_parse_ctx *)yajl_ctx;

    ctx->map_key = malloc(size);
    strncpy(ctx->map_key, (char *)data, size);
    return 1;
}

static int nslite_yajl_read_start_map(void *yajl_ctx)
{
    nslite_yajl_parse_ctx *ctx = (nslite_yajl_parse_ctx *)yajl_ctx;
    printf("Start Map\n");
    ctx->element = malloc(NSLITE_JSON_MAX_ELEMENT);
    return 1;
}

static int nslite_yajl_read_end_map(void *yajl_ctx)
{
    nslite_yajl_parse_ctx *ctx = (nslite_yajl_parse_ctx *)yajl_ctx;
    printf("End Map\n");
    free(ctx->element);
    return 1;
}

static int nslite_yajl_read_start_array(void *yajl_ctx)
{
    nslite_yajl_parse_ctx *ctx = (nslite_yajl_parse_ctx *)yajl_ctx;
    printf("Start Array\n");
    ctx->array_index = 0;
    return 1;
}

static int nslite_yajl_read_end_array(void *yajl_ctx)
{
    nslite_yajl_parse_ctx *ctx = (nslite_yajl_parse_ctx *)yajl_ctx;
    printf("End Array\n");
    ctx->array_index = -1;
    return 1;
}

static yajl_callbacks nslite_yajl_read_callbacks = {
    NULL, /* null */
    NULL, /* bool */
    nslite_yajl_read_integer,
    NULL, /* double */
    NULL, /* number */
    nslite_yajl_read_string,
    nslite_yajl_read_start_map,
    nslite_yajl_read_map_key,
    nslite_yajl_read_end_map,
    nslite_yajl_read_start_array,
    nslite_yajl_read_end_array
};

int nslite_parse_command(const unsigned char *buf, const int buflen, nslite_command *cmd)
{
    yajl_handle hand;
    yajl_parser_config cfg = { 1, 1 };
    nslite_yajl_parse_ctx ctx;
    int status;

    /* empty nslite_command is allocated by caller */
    ctx.command = cmd;

    hand = yajl_alloc(&nslite_yajl_read_callbacks, &cfg, NULL, &ctx);

    do {
        status = yajl_parse(hand, buf, buflen);
    } while (status == yajl_status_insufficient_data);

    status = yajl_parse_complete(hand);

    if (status != yajl_status_ok) {
        err(1, "JSON message parsing failed.");

        unsigned char *str = yajl_get_error(hand, 1, buf, buflen);
        fprintf(stderr, "%s\n", (char *)str);
        yajl_free_error(hand, str);
    }

    return 0;
}

