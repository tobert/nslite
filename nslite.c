/*
 * nslite - start a process in a namespaced chroot with control over stdio
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
#include <stdio.h>
#include <time.h>
#include <string.h>

typedef struct child_status {
    pid_t pid;
    char *command;
    char *args;
    struct timespec *started;
    struct timespec *terminated;
} child_status_t;

static void usage(char *program)
{
    printf("%s: namespaced process manager.\n\n", program);
    printf("Usage:\n");
    printf("  %s <chroot path> <executable> [executable args]\n", program);
    exit(1);
}

static void report(struct child_status *st)
{
    printf("status\n");
}

int main(int argc, char *argv[])
{
    pid_t child;
    int i, status;
    struct stat sb;
    char *program, *newroot, *command;
    char *newenv[] = { NULL };
    char **newargv;
    child_status_t cs;

    if (argc < 3)
        usage(argv[0]);

    program = argv[0];
    newroot = argv[1];
    command = argv[2];

    if (stat(newroot, &sb) == -1)
        err(1, "'%s'", newroot);

    if ((sb.st_mode & S_IFMT) != S_IFDIR)
        errx(1, "stat(): '%s' is not a directory\n", newroot);

    cs.started = malloc(sizeof(struct timespec));
    if (cs.started == NULL)
        err(1, NULL);

    cs.terminated = malloc(sizeof(struct timespec));
    if (cs.terminated == NULL)
        err(1, NULL);

    cs.command = command;
    cs.args = malloc(4096); /* cheezy, unsafe */
    cs.args[0] = "\0";
    for (i=3; i<=argc; i++) {
        if (i != 3)
            strcat(cs.args, " ");
        strcat(cs.args, argv[i]);
    }

    clock_gettime(CLOCK_REALTIME, cs.started);
    child = nsfork(0);

    /* parent process - wait here for the child to exit */
    if (child > 0) {
        cs.pid = child;
        waitpid(child, &status, 0);
        clock_gettime(CLOCK_REALTIME, cs.terminated);
        report(&cs);
    }
    /* child process */
    else if (child == 0) {
        if (chdir(newroot) == -1)
            err(1, NULL);

        if (chroot(".") == -1)
            err(1, NULL);

        if (access(command, F_OK|X_OK) != 0)
            errx(1, "Command '%s' does not exist or is not executable.", command);

        newargv = malloc(sizeof(argv));
        for (i=2; i<=argc; i++) {
            newargv[i-2] = argv[i];
        }
        newargv[i] = "\0";

        fprintf(stderr,"Command: %s, argv0: %s, argv1: %s\n", command, newargv[0], newargv[1]);

        if (execve(command, newargv, newenv) == -1) {
            free(newargv);
            err(1, "'%s'", command);
        }
    }
    else {
        fprintf(stderr, "error in nsfork(): probably lacking privileges or running on an old kernel\n");
    }

    free(cs.started);
    free(cs.terminated);

    return(0);
}

