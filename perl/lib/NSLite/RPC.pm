package NSLite::RPC;

use strict;
use warnings;
use Carp;
use IPC::Open3;
use IO::File;
use JSON::Any;

# this must match nslite_rpc.h NSLITE_PREFIX_BYTES
use constant NSLITE_PREFIX_BYTES => 2;

=head1 NAME

NSLite::RPC - RPC with the nslite shim

=head1 DESCRIPTION

nslite speaks an RPC dialect ripped off from JSON-RPC but with binary message length headers
prefixed to every message to make interaction with C easier and much easier to do correctly
without deadlock and crazy I/O handlers.

This module implements everything that's needed to do the encoding/decoding but leaves
dispatch from the messages up to you for now.

=head1 SYNOPSIS

 use NSLite::RPC;
 use IO::Select;

 my $nslite = NSLite::RPC->new("./nslite", "/path/to/chroot", "/bin/busybox", "sh");
 my $status = $nslite->run();

 my $select = IO::Select->new();
 $select->add($nslite->stdout);
 $select->add($nslite->stderr);

 while (my @ready = $select->can_read()) {
    foreach my $r (@ready) {
        if (fileno($r) == fileno($nslite->stdout)) {
            my $data = $nslite->read_message;
            ...
        }
        elsif ( fileno($r) == fileno($nslite->stderr)) {
            warn "Error on child's stderr!";
            warn $nslite->stderr->getline();
        }
        else {
            confess "unrecognized filehandle returned by select";
        }
    }
 }

=head1 METHODS

=over 4

=item new()

Create an NSLite::RPC object. This is only initialization, nothing is executed until you call run().

 my $nslite = NSLite::RPC->new('./nslite', '/srv/roots/root1', '/bin/busybox', 'sh');

=cut

sub new {
    my($class, $command, @args) = @_;

    # TODO: arg checking
 
    my $self = {
        command => $command,
        args    => \@args,
        _stdin  => IO::File->new(),
        _stdout => IO::File->new(),
        _stderr => IO::File->new(),
        json    => JSON::Any->new()
    };

    return bless $self, $class;
}

=item run()

Run the nslite process. This fires up nslite with IPC::Open3 and sets up stdin/stdout
in binmode with buffering disabled.

 $nslite->run();

=cut

sub run {
    my $self = shift;

    my $pid = open3(
        $self->{_stdin},
        $self->{_stdout},
        $self->{_stderr},
        $self->{command},
        @{$self->{args}}
    );

    # dealing with binary data (length prefix), JSON will be fine
    $self->{_stdin}->binmode();
    $self->{_stdout}->binmode();

    return $pid;
}

=item stdin(), stdout(), stderr()

Get the IO::Handle object for the given child IO fd.

 my $child_stdin  = $nslite->stdin;
 my $child_stdout = $nslite->stdout;
 my $child_stderr = $nslite->stderr;

=cut

sub stdin  { shift->{_stdin} }
sub stdout { shift->{_stdout} }
sub stderr { shift->{_stderr} }

=item read_message()

Read a single message from the child's stdout. JSON is automatically decoded (using JSON::Any).

 my $data = $nslite->read_message;

=cut

sub read_message {
    my $self = shift;
    my($lenbuf, $databuf);

    if ($self->stdout->sysread($lenbuf, NSLITE_PREFIX_BYTES) != NSLITE_PREFIX_BYTES) {
        confess "sysread() of message size from child stdout failed: $!";
    }

    my $size = unpack('n', $lenbuf);

    if ($self->stdout->sysread($databuf, $size) != $size) {
        confess "sysread() of $size byte message from child stdout failed: $!";
    }

    my $data = $self->{json}->from_json($databuf);

    return $data;
}

=item write_message()

Write a single message to the child's stdout. The input data structure will be
converted to JSON and length prefixed automatically.

 $nslite->write_message($data);

=cut

sub write_message {
    my($self, $data) = @_;

    my $json = $self->{json}->to_json($data);
    my $length = length($json);
    my $bin_length = pack('n', $length);

    my $wrote = $self->stdin->syswrite($bin_length, NSLITE_PREFIX_BYTES);
    if ($wrote != NSLITE_PREFIX_BYTES) {
        confess "syswrite() of message length to child stdin failed: $!";
    }

    $wrote = $self->stdin->syswrite($json, $length);
    if ($wrote != $length) {
        confess "syswrite() of JSON message failed: $!";
    }

    return $wrote;
}

1;

# vim: et ts=4 sw=4 ai smarttab

__END__

=back

=head1 AUTHORS

Al Tobey <tobert@gmail.com>

=head1 COPYRIGHT AND LICENSE

This software is copyright (c) 2011 by Al Tobey.

This is free software; you can redistribute it and/or modify it under the terms
of the Artistic License 2.0.  (Note that, unlike the Artistic License 1.0,
version 2.0 is GPL compatible by itself, hence there is no benefit to having an
Artistic 2.0 / GPL disjunction.)  See the file LICENSE for details.

=cut

