package NSLite::Mock;

use strict;
use warnings;
use IO::Handle qw(_IONBF _IOLBF _IOFBF);
use IO::File;
use JSON::Any;
use Data::Dumper;
use Carp;

# this must match nslite_rpc.h NSLITE_PREFIX_BYTES
use constant NSLITE_PREFIX_BYTES => 2;

=head1 NAME

NSLite::Mock - mock the nslite shim for testing

=head1 DESCRIPTION

Simple utilities to mock nslite for testing.

=head1 SYNOPSIS

 use NSLite::Mock;
 my $mock = NSLite::Mock->new();
 $mock->write_message({method => "pre_launch_hook", params => [$$], id => 1});
 my $data = $mock->read_message();

=head1 METHODS

=over 4

=item new()

Create a new object.

 my $mock = NSLite::Mock->new();

=cut

sub new {
    my($class) = @_;
    my $self = { _id => 0 };

    $self->{json} = JSON::Any->new();

    # IO::File has binmode, IO::Handle does not
    $self->{_stdin} = IO::File->new();
    $self->{_stdin}->fdopen(fileno(STDIN),"r");

    $self->{_stdout} = IO::File->new();
    $self->{_stdout}->fdopen(fileno(STDOUT),"w");

    $self->{_stderr} = IO::File->new();
    $self->{_stderr}->fdopen(fileno(STDERR),"w");
    
    $self->{_stdin}->binmode();
    $self->{_stdout}->binmode();

    return bless $self, $class;
}

=item stdin(), stdout(), stderr()

Get the IO::Handle object for the named descriptor.

 my $mock_stdin  = $mock->stdin;
 my $mock_stdout = $mock->stdout;
 my $mock_stderr = $mock->stderr;

=cut

sub stdin  { shift->{_stdin} }
sub stdout { shift->{_stdout} }
sub stderr { shift->{_stderr} }

=item read_message()

Read a message from stdin, return JSON.

=cut

sub read_message {
    my($self) = @_;

    my($lenbuf, $databuf);

    if ($self->stdin->sysread($lenbuf, NSLITE_PREFIX_BYTES) != NSLITE_PREFIX_BYTES) {
        confess "sysread() of message size from child stdout failed: $!";
    }

    my $size = unpack('n', $lenbuf);

    if ($self->stdin->sysread($databuf, $size) != $size) {
        confess "sysread() of $size byte message from child stdout failed: $!";
    }

    my $data = $self->{json}->from_json($databuf);

    return $data;
}

=item write_message()

Write a message to stdout. Takes a regular perl datastructure (usually a hash) and returns an RPC packet.

=cut

sub write_message {
    my($self, $data) = @_;

    my $json_str = $self->{json}->to_json($data);
    my $length = length($json_str);
    my $bin_length = pack('n', $length);

    my $wrote = $self->stdout->syswrite($bin_length, NSLITE_PREFIX_BYTES);
    if ($wrote != NSLITE_PREFIX_BYTES) {
        confess "syswrite() of message length to child stdin failed: $!";
    }

    $wrote = $self->stdout->syswrite($json_str, $length);
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

