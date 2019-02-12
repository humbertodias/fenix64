#!/usr/bin/perl
#
# Copyright (c) 2019  Peter Pentchev <roam@ringlet.net>
# All rights reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

use v5.12;
use strict;
use warnings;

use File::Temp;
use Path::Tiny;
use Test::Command;
use Test::More;

use lib ($ENV{TESTDIR} || 't').'/lib';

use Test::Fenix::Config;
use Test::Fenix::Compile;
use Test::Fenix::Run;

use constant FHELLO_FILE => 'fhello.txt';

my $config = Test::Fenix::Config::config;

sub fhello_prepare($)
{
	my ($test) = @_;

	return 1 unless -f FHELLO_FILE;
	unlink FHELLO_FILE or die "Could not remove ".FHELLO_FILE.": $!\n";
}

sub fhello_verify($ $)
{
	my ($test, $output) = @_;

	subtest 'check fhello' => sub {
		plan tests => 2;

		my $text = eval {
			path(FHELLO_FILE)->slurp_utf8
		};
		is $@ // '', '', 'read the file';
		is $text, "Hello world!\r\n", 'file contents';
	};
}

sub hello_prepare($)
{
}

sub hello_verify($ $)
{
	my ($test, $output) = @_;

	subtest 'check hello' => sub {
		plan tests => 1;

		like $output, qr/Hello world!/, 'hello there';
	};
}

my %tests = (
	fhello => {
		filename => 'fhello.prg',
		run => {
			prepare_sub => \&fhello_prepare,
			verify_sub => \&fhello_verify,
		},
	},

	hello => {
		filename => 'hello.prg',
		run => {
			prepare_sub => \&hello_prepare,
			verify_sub => \&hello_verify,
		},
	},
);

sub run_test($)
{
	my ($prog) = @_;

	# Copy the program over
	my $text = $prog->{orig}{src}->slurp_utf8;
	$prog->{test}{src}->spew_utf8($text);

	# Compile it
	Test::Fenix::Compile::test_compile($prog);

	# Prepare the testbed
	$prog->{run}{prepare}->();

	# Run it
	my $output = Test::Fenix::Run::test_run($prog);

	# Make sure it worked
	$prog->{run}{verify}->($output);
}

MAIN:
{
	my $tempd_base = File::Temp->newdir('comprun.XXXXXX', TMPDIR => 1);
	BAIL_OUT 'Could not create a temporary directory'
	    unless defined $tempd_base;
	my $tempd = path($tempd_base);
	chdir $tempd or die "Could not change to $tempd: $!\n";

	my $datadir = $config->{testdir}->child('data');

	plan tests => scalar keys %tests;

	for my $test_name (sort keys %tests) {
		eval {
			subtest "program $test_name" => sub {
				plan tests => 4;
	
				my $test = $tests{$test_name};
				my $fname = $test->{filename};
				my $binname = $fname;
				$binname =~ s/\.prg$/.dcb/;
				isnt $binname, $fname, 'sanity check';
	
				$test->{orig} = {
					src => $datadir->child($fname),
				};
	
				$test->{test} = {
					src => $tempd->child($fname),
					exe => $tempd->child($binname),
				};
	
				$test->{run}{prepare} = sub {
					$test->{run}{prepare_sub}->($test)
				};
				$test->{run}{verify} = sub {
					my ($output) = @_;
	
					$test->{run}{verify_sub}->($test, $output)
				};
	
				run_test $test;
			};
		};
		warn $@ if $@;
	}

	chdir $config->{testdir}
	    or die "Could not change to $config->{testdir}: $!\n";
}
