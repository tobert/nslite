#!/usr/bin/perl

use strict;
use warnings;
use 5.010;
use IO::Select;
use Data::Dumper;

use FindBin qw($Bin);
use lib "$Bin/../lib";
use NSLite::RPC;

=head1 NAME

drive-nslite.pl - example nslite master process

=head1 DESCRIPTION

nslite allows its parent process to control it over a variation on JSON-RPC (see README.md).
This script implements an example master program using NSLite::RPC.

=head1 SYNOPSIS

This program accepts a 'stop' command on stdin. Ctrl-C also seems to work, but results in
an unclean shutdown.

 drive-nslite.pl <root> <executable> [executable options ...]
 ~/src/nslite/perl/bin $ ./drive-nslite.pl ./mock-nslite.pl /srv/roots/0 /bin/sh
 ~/src/nslite $ sudo ./perl/bin/drive-nslite.pl ./nslite.pl /srv/roots/0 /bin/sh

=cut

my($nslite_bin, $root, $command, @args) = @ARGV;
my $id = 2;
my $shutdown_id = undef;
my $run = 1;

die "Not enough arguments."
    unless (@ARGV >= 3);
die "'$root' does not exist or is not a directory: $!"
    unless (-d $root);
die "'$command' ($root/$command) does not exist or is not executable: $!"
    unless (-x "$root/$command");

my $nslite = NSLite::RPC->new($nslite_bin, $root, $command, @args);

say "Starting nslite ...";
my $pid = $nslite->run();
say "Child process is $pid.";

my $select = IO::Select->new();
   $select->add(\*STDIN);
   $select->add($nslite->stdout);
   $select->add($nslite->stderr);

while ($run == 1) {
    my @ready = $select->can_read(2);

    # timeout
    unless (@ready) {
        #say "select timed out ... requesting status";
        $nslite->write_message({
            method => "status",
            params => [],
            id     => ++$id
        });
    }

    foreach my $r (@ready) {
        if (fileno($r) == fileno($nslite->stdout)) {
            my $data = $nslite->read_message();

            if (exists $data->{method}) {
                dispatch_method($data);
            }
            elsif (exists $data->{result}) {
                dispatch_result($data);
            }
            else {
                die "Invalid packet: ".Dumper($data);
            }
        }
        elsif (fileno($r) == fileno($nslite->stderr)) {
            print STDERR "NSLITE STDERR: " . $nslite->stderr->getline;
        }
        elsif (fileno($r) == fileno(STDIN)) {
            my $input = <STDIN>;
            chomp $input;
            given ($input) {
                when ('stop') {
                    $nslite->write_message({
                        method => 'shutdown',
                        params => [],
                        id     => ++$id
                    });
                }
                default {
                    say "unrecognized input '$_'";
                }
            }
        }
        else {
            die "error on descriptor '$r'";
        }
    }
}
die "Loop died in $0" if ($run == 1);

sub dispatch_method {
    my $data = shift;
    given ($data->{method}) {
        when ('pre_launch_hook') {
            my $resp = {
                result => 'ok',
                error  => undef,
                id     => $data->{id}
            };
            $nslite->write_message($resp);
        }
        default {
            die "Invalid method in call '$_'";
        }
    }
}

sub dispatch_result {
    my $data = shift;
    if ($data->{result} and $data->{result} eq 'shutdown_ok') {
        # global run variable for shutdown
        $run--;
    }
    warn Dumper($data);
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

