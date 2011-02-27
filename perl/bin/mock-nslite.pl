#!/usr/bin/perl
$| = 1;

use strict;
use warnings;
use 5.010;
use Data::Dumper;
use IO::Select;

use FindBin qw($Bin);
use lib "$Bin/../lib";
use NSLite::Mock;

=head1 NAME

mock-nslite.pl - pretend to be nslite but don't do anything in reality

=head1 DESCRIPTION

Mock an nslite shim but don't call anything, just respond with fake but parseable/usable
data over RPC for testing.

=head1 SYNOPSIS

Arguments are all optional. If root/executable are specified, the status messages
and similar will draw from those parameters to look more realistic.

 mock-nslite.pl <root> <executable> [executable options ...]

=cut

my $mock = NSLite::Mock->new();

my $select = IO::Select->new();
$select->add($mock->stdin);

# nslite always calls pre_launch_hook before execing
$mock->write_message({method => "pre_launch_hook", params => [$$], id => 1});

while (my @ready = $select->can_read()) {
    if (@ready) {
        my $data = $mock->read_message();
        if (exists $data->{method}) {
            dispatch_method($mock, $data);
        }
        elsif (exists $data->{result}) {
            dispatch_result($mock, $data);
        }
        else {
            die "Invalid packet: ".Dumper($data);
        }
    }
}
die "Loop died.";

sub dispatch_result {
    my($mock, $data) = @_;

    if ($data->{id} == 1) {
        warn "Got response on pre_launch_hook...";
    }
}

sub dispatch_method {
    my($mock, $data) = @_;

    given ($data->{method}) {
        when ('status') {
            $mock->write_message({
                result => { pid => $$, program => $0, args => [@ARGV] },
                error  => undef,
                id     => $data->{id}
            });
        }
        when ('shutdown') {
            $mock->write_message({
                # just testing at the moment, this will probably have a bunch of data with it
                result => 'shutdown_ok',
                error  => undef,
                id     => $data->{id}
            });
            exit 0;
        }
        default {
            warn "Got an unknown method: ".Dumper($data);
        }
    }
}

# vim: et ts=4 sw=4 ai smarttab

__END__

=head1 AUTHOR

Al Tobey <tobert@gmail.com>

=head1 COPYRIGHT AND LICENSE

This software is copyright (c) 2011 by Al Tobey.

This is free software; you can redistribute it and/or modify it under the terms
of the Artistic License 2.0.  (Note that, unlike the Artistic License 1.0,
version 2.0 is GPL compatible by itself, hence there is no benefit to having an
Artistic 2.0 / GPL disjunction.)  See the file LICENSE for details.

=cut

