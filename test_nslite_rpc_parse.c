/*
 * nslite - start a process in a namespaced chroot with control over stdio
 * Copyright 2011, Albert P. Tobey <tobert@gmail.com>
 * https://github.com/tobert/nslite
 * 
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the Artistic License 2.0.  See the file LICENSE for details.
 */

#include "config.h"

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nslite_rpc.h"

int main(int argc, char *argv[])
{
    nslite_command cmd;
    const char *buffer = "{\"method\": \"foobar\", \"params\": [\"bar\"], id: 1}";
    int ret;

    ret = nslite_parse_command((unsigned char *)buffer, strlen(buffer), &cmd);

    return ret;
}

