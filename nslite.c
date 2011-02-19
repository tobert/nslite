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
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <linux/limits.h> /* for ARG_MAX */

#include "nsfork.h"
#include "nslite_rpc.h"

typedef struct child_status {
    pid_t pid;
    char *command;
    char *args;
    struct rusage *usage1;
    struct rusage *usage2;
    time_t started;
} child_status_t;

static void usage(char *program)
{
    printf("%s: namespaced process manager.\n\n", program);
    printf("Usage:\n");
    printf("  %s <chroot path> <executable> [executable args]\n", program);
    exit(1);
}

void child_status_init(int argc, char *argv[], pid_t child_pid, struct child_status *st)
{
    char *argstr;

    /* flatten the child argv into a single string */
    if (argc >= 2) {
        argstr = malloc(ARG_MAX);
        if (argstr == NULL)
            err(1, "malloc failed");
        argstr[0] = '\0';

        strcat(argstr, argv[1]);

        for (int i=2; i<argc; i++) {
            strcat(argstr, " ");
            strcat(argstr, argv[i]);
        }

        /* need to read up on realloc() ... seems to be the right thing here */
        st->args = malloc(strlen(argstr));
        if (st->args == NULL) {
            free(argstr);
            err(1, "realloc failed");
        }
        strcpy(st->args, argstr);
    }
    else {
        st->args = NULL;
    }

    st->pid = child_pid;
    st->command = argv[0];
    st->started = time(NULL);

    printf("st->pid: %d is %s %s\n", st->pid, st->command, st->args);

    st->usage1 = malloc(sizeof(struct rusage));
    st->usage2 = malloc(sizeof(struct rusage));

    if (getrusage(RUSAGE_CHILDREN, st->usage1) == -1)
        err(1, "getrusage failed");
}

void child_main(int argc, char *argv[])
{
    printf("init\n");
}

void parent_rpc_loop(struct child_status *st)
{
    fd_set rfds;
    struct timeval tv = {5, 0};
    int ret;
    char buf[BUFSIZ];
    size_t bufsz;

    FD_ZERO(&rfds);
    FD_SET(0, &rfds); /* stdin */

    ret = select(1, &rfds, NULL, NULL, &tv);

    if (ret == -1)
        err(1, "select failed");
    /* data ready */
    else if (ret) {
        printf("Go!\n");
        read(0, &buf, NSLITE_PREFIX_BYTES);
        bufsz = nslite_packet_size(&buf);
        read(0, &buf, bufsz);
    }
    /* timeout */
    else {
        printf("timeout\n");
    }
}

/*
 * argv[
 *  1: new root directory
 *  2: command to run inside chroot
 *  3 ...: arguments to the chrooted command
 * ]
 */
int main(int argc, char *argv[])
{
    pid_t child;
    struct stat sb;
    char *program, *newroot, *command;
    char *newenv[] = { NULL };
    int newargc;
    char **newargv;
    struct child_status cs;

    /* a pipe is used to have the child process block until the parent signals
     * that it should continue */
    int p2c_pipe[2];
    char throwaway;

    if (argc < 3)
        usage(argv[0]);

    program = argv[0];
    newroot = argv[1];
    command = argv[2];

    /* make sure at the chroot directory exists */
    if (stat(newroot, &sb) == -1)
        err(1, "'%s'", newroot);
    if (!S_ISDIR(sb.st_mode))
        errx(1, "stat(): '%s' is not a directory\n", newroot);

    /* copy command + remaining args into new process's argv array */
    newargv = calloc(argc - 1, sizeof(char *));
    for (newargc=0; newargc<argc-2; newargc++) {
        newargv[newargc] = argv[newargc+2];
    }
    newargv[newargc+1] = "\0";

    /* open a pipe - the child will block on the pipe for a cheezy semaphore */
    if (pipe(p2c_pipe) == -1)
        err(1, "pipe creation failed");

    /* fork, child will be in a new namespace */
    child = nsfork(0);

    /* child process */
    if (child == 0) {
        close(p2c_pipe[1]);

        if (chdir(newroot) == -1)
            err(1, "chdir failed");

        if (chroot(".") == -1)
            err(1, "chroot failed");

        if (access(command, F_OK|X_OK) != 0)
            errx(1, "Command '%s' does not exist or is not executable.", command);

        /* block on the pipe waiting for a single, arbitrary byte */
        if (read(p2c_pipe[0], &throwaway, 1) < 1)
            err(1, "semaphore read on pipe failed: '%s'", &throwaway);

        if (execve(command, newargv, newenv) == -1) {
            free(newargv);
            err(1, "execve failed");
        }
    }
    /* parent process - wait here for the child to exit */
    else if (child > 0) {
        close(p2c_pipe[0]);

        child_status_init(newargc, newargv, child, &cs);

        if (write(p2c_pipe[1], "x", 1) < 1)
            err(1, "semaphore write on pipe failed");

        parent_rpc_loop(&cs);
    }
    else {
        fprintf(stderr, "error in nsfork(): probably lacking privileges or running on an old kernel\n");
    }

    return 0;
}

