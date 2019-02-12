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
use Test::Fenix::Image;

my $config = Test::Fenix::Config::config;

sub run_test($ $)
{
	my ($file, $n) = @_;
SKIP:
	{
		# Generate the source image
		my $img = Test::Fenix::Image::generate('seq', $n);

		Test::Fenix::Image::test_image_to_map($img, $file->{src});

		my $cmd = Test::Command->new(cmd => [
			$config->{cmd_map}, '-l', $file->{src}{map},
		]);
		$cmd->exit_is_num(0);
		$cmd->stdout_isnt_eq('');
		$cmd->stderr_is_eq('');
		$cmd->stdout_like(qr{\Q$n\E x \Q$n\E}x);

		my $res = Test::Fenix::Image::test_map_to_image($file->{dst});
		is Test::Fenix::Image::compare_images($img, $res), '',
		    'compare the images';
	}
}

MAIN:
{
	my $tempd_base = File::Temp->newdir('map.XXXXXX', TMPDIR => 1);
	BAIL_OUT 'Could not create a temporary directory'
	    unless defined $tempd_base;
	my $tempd = path($tempd_base);
	chdir $tempd or die "Could not change to $tempd: $!\n";

	my @patterns = Test::Fenix::Image::generate_patterns();

	plan tests => scalar @patterns;

	for my $pattern (@patterns) {
		eval {
			subtest "pattern $pattern" => sub {
				plan tests => scalar @{$config->{conversions}};

				for my $test (@{$config->{conversions}}) {
					my ($srcfmt, $dstfmt, $n) = @{$test};
					my $src = Test::Fenix::Image::format($srcfmt);
					my $dst = Test::Fenix::Image::format($dstfmt);
					my %file = (
						src => {
							format_name => $srcfmt,
							format => $src,
							file => $tempd->child(
							    $src->filename('image')),
							map => $tempd->child(
							    'image.map'),
						},
						dst => {
							format_name => $dstfmt,
							format => $dst,
							file => $tempd->child(
							    $dst->filename('image')),
						}
					);
					$file{dst}{map} = $file{src}{map};
					my $name = "$srcfmt to $dstfmt, ${n}x${n}";
					subtest $name => sub {
						run_test \%file, $n;
					}
				}
			};
		};
		warn $@ if $@;
	}

	chdir $config->{testdir}
	    or die "Could not change to $config->{testdir}: $!\n";
}
