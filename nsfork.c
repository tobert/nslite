/* 
 * nsfork.c - like fork, but the child process is in a new namespace
 * Copyright 2011, Albert P. Tobey <tobert@gmail.com>
 * https://github.com/tobert/nschroot
 * 
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the Artistic License 2.0.  See the file LICENSE for details.
 *
 * minimum linux kernel: v2.6.24
 * execution requires root and/or CAP_SYS_ADMIN
 */

#include <unistd.h>
#include <sys/syscall.h>
#include <signal.h>
#define _GNU_SOURCE
#include <sched.h>

/* extra_flags:
 * set to 0 for regular nsfork, which leaves the new process in the same
 * network namespace, but places everything else in a new ns
 *
 *     CLONE_NEWNET - also create a new network namespace
 *     See also: clone(2)
 */
pid_t nsfork(int extra_flags)
{
    /* kernel sys_fork only passes SIGCHLD, which is exactly what's desired here
     *
     * I did find userspace code implementing clone-based forks that passed
     * CLONE_CHILD_SETTID but figure I want to be as close to regular fork() as
     * possible but create a new namespace
     * /usr/src/linux-2.6.37/arch/x86/kernel/process.c
     *
     * Also adding CLONE_IO to make it possible to implement a bit of a light
     * vfork/popen.  We need pipes to the opened process to capture its stdio.
     */
    int clone_flags = extra_flags | CLONE_NEWNS | CLONE_NEWIPC | CLONE_NEWPID | CLONE_NEWUTS | SIGCHLD;
    pid_t child_pid = 0;

    // manual sys_clone behaves like fork() but allows flags to create a namespace
    child_pid = syscall(SYS_clone, clone_flags, 0);

    return child_pid;
}

