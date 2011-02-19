# nslite - chroot/exec with namespace support

Linux has had namespace support for a little while. Probably the most common
user of it is LXC. For a lot of purposes, LXC is overkill. I was curious about
how to use the underlying syscalls for another project and wrote nsfork.c.
After discussing it with a friend who's doing some nice work with busybox
inside chroots, I decided to throw this together for general use.

## Requirements

Linux >= 2.6.24 with CONFIG_NAMESPACES=y

Root or CAP_SYS_ADMIN privileges

## Build

make

The included Makefile should work on any modern distribution.  Let me know
if that proves false.

## Example

    mkdir -p /tmp/root/bin
    cp `which busybox.static` /tmp/root/bin/busybox
    sudo ./nschroot /tmp/root /bin/busybox msh
    mkdir /proc
    mount none /proc -t proc
    ps -ef
    ls -l /

## nslite-rpc

All commands and responses are to be prefixed with their size in a binary
16 bit unsigned integer in network order. (e.g. perl pack("n", $number)).

NOTE: this is an experiment at the moment.

### Commands

    { "method": "status", "id": 1 }
    { "method": "kill", "params": [9], "id": 1 }

### Response

    { "result": 1, "error": null, "id": 1 }

## Ideas / To do

* fill in nsexec
* add user/group switching
* add capabilities(7) support
* LD_PRELOAD example
* busybox applet?

## Author

Al Tobey <tobert@gmail.com> @AlTobey

## License

This is free software; you can redistribute it and/or modify it under the
terms of the Artistic License 2.0.  See the file LICENSE for details.

