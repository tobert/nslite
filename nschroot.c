/*
 * nschroot - chroot into a fresh Linux namespace
 * Copyright 2011, Albert P. Tobey <tobert@gmail.com>
 * https://github.com/tobert/nschroot
 * 
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the Artistic License 2.0.  See the file LICENSE for details.
 */

#include "nsfork.h"
#include <err.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    struct stat sb;
    pid_t child;
    int i, status, argp, child_wait_stdin;
    char *program, *newroot, *command;
    char *newenv[] = { NULL };
    char **newargv;

    if (argc < 3)
        errx(1, "Usage: %s <directory> <command> [args]\n", argv[0]);

    /* lame for now - need to add real options parsing at some point */
    child_wait_stdin = 0;
    argp = 0;
    if (strcmp(argv[0], "--child-wait-stdin")) {
        argp = 1;
        child_wait_stdin = 1;
    }

    program = argv[argp];
    newroot = argv[argp+1];
    command = argv[argp+2];

    if (stat(newroot, &sb) == -1)
        err(1, "'%s'", newroot);

    if ((sb.st_mode & S_IFMT) != S_IFDIR)
        errx(1, "stat(): '%s' is not a directory\n", newroot);
    
    newargv = malloc(sizeof(argv));
    for (i=2; i<=argc; i++) {
        newargv[i-2] = argv[i+argp];
    }
    newargv[i] = "\0";

    child = nsfork(0);
    /* parent process - wait here for the child to exit */
    if (child > 0) {
        /* TODO: good enough for testing but needs to be checked */
        waitpid(child, &status, 0);
    }
    /* child process */
    else if (child == 0) {
        if (chdir(newroot) == -1)
            err(1, NULL);

        if (chroot(".") == -1)
            err(1, NULL);

        if (stat(command, &sb) == -1)
            err(1, "'%s'", command);

        /* optionally block on stdin */
        if (child_wait_stdin == 1) {
            while ((child_wait_stdin = getchar()) != EOF) {
                if (child_wait_stdin == '\n')
                    break;
            }
        }

        if (execve(command, newargv, newenv) == -1)
            err(1, "'%s'", command);
    }
    else {
        errx(1, "error in nsfork(): probably lacking privileges or running on an old kernel\n");
    }

    free(newargv);

    return(0);
}

