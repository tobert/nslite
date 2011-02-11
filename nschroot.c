/*
 * nschroot - chroot into a fresh Linux namespace
 * Copyright 2011, Albert P. Tobey <tobert@gmail.com>
 * https://github.com/tobert/nslite
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
#include <stdio.h>

static void usage(char *program)
{
    printf("%s: chroot into a new Linux namespace.\n\n", program);
    printf("Usage:\n");
    printf("  %s <chroot path> <executable> [executable args]\n", program);
    exit(1);
}

int main(int argc, char *argv[])
{
    struct stat sb;
    pid_t child;
    int i, status;
    char *program, *newroot, *command;
    char *newenv[] = { NULL };
    char **newargv;

    if (argc < 3)
        usage(argv[0]);

    program = argv[0];
    newroot = argv[1];
    command = argv[2];

    if (stat(newroot, &sb) == -1)
        err(1, "'%s'", newroot);

    if ((sb.st_mode & S_IFMT) != S_IFDIR)
        errx(1, "stat(): '%s' is not a directory\n", newroot);
    
    newargv = malloc(sizeof(argv));
    for (i=2; i<=argc; i++) {
        newargv[i-2] = argv[i];
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

        if (execve(command, newargv, newenv) == -1)
            err(1, "'%s'", command);
    }
    else {
        errx(1, "error in nsfork(): probably lacking privileges or running on an old kernel\n");
    }

    free(newargv);

    return(0);
}

