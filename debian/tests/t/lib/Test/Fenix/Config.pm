use v5.12;
use strict;
use warnings;

package Test::Fenix::Config;

use strict;
use warnings;

use Path::Tiny;

use Test::Fenix::Image;

my $config;

my @DEFAULT_CONVERSIONS = (
	['png', 'png', 4],
	['gif', 'png', 4],

	['png', 'png', 15],
	['gif', 'png', 15],

	['png', 'png', 16],
	['gif', 'png', 16],

	['png', 'png', 32],
	['gif', 'png', 32],
);

=head1 NAME

Test::Fenix::Config - utility functions for the Fenix test suite

=head1 SYNOPSIS

    use Test::Fenix::Config;

    my $cfg = Test::Fenix::Config::config();

    say $cfg->{'testdir'};

=head1 FUNCTIONS

=over 4

=item env_or_path

    my $path = env_or_path($varname, $default_path);

If a variable with the specified name exists in the environment and
has a non-empty value, use its value as a path; otherwise, use
the supplied C<Path::Tiny> object as a default path.  In either case,
return a C<Path::Tiny> object representing the absolute path.

=cut

sub env_or_path($ $)
{
	my ($var, $path) = @_;
	my $value = $ENV{$var};

	if (defined $value && $value ne '') {
		return path($value)->absolute;
	} else {
		return $path->absolute;
	}
}

=item config

    my $cfg = Test::Fenix::Config::config()

Determine some configuration settings from environment variables or
examining the filesystem.

=cut

sub config()
{
	return $config if defined $config;

	my @conversions;
	my $conv_override = $ENV{TEST_FENIX_CONVERSIONS};
	if (defined $conv_override) {
		for my $conv (split ',', $conv_override) {
			Test::More::BAIL_OUT("Invalid conversion spec: $conv")
			    unless $conv =~ /^
			    (?<src> \w+ ) -
			    (?<dst> \w+ ) -
			    (?<size> [1-9][0-9]* )
			$/x;
			my ($src_name, $dst_name, $size) =
			    ($+{src}, $+{dst}, $+{size});

			Test::Fenix::Image::format($src_name);
			my $dst = Test::Fenix::Image::format($dst_name);
			eval {
				$dst->opt
			};
			if ($@) {
				die "The '$dst_name' format may not be ".
				    "used for output\n"
			}

			push @conversions, [$src_name, $dst_name, $size];
		}
	} else {
		@conversions = @DEFAULT_CONVERSIONS;
	}

	my %cfg = (
		testdir => env_or_path('TESTDIR', path('t')),

		conversions => \@conversions,
	);
	%cfg = (
		%cfg,
		cmd_map => env_or_path('TEST_FENIX_MAP',
		    $cfg{testdir}->parent->child('map')->child('map')),
		cmd_fpg => env_or_path('TEST_FENIX_FPG',
		    $cfg{testdir}->parent->child('fpg')->child('fpg')),
		cmd_fxc => env_or_path('TEST_FENIX_FXC',
		    $cfg{testdir}->parent->child('fxc')->child('src')->child('fxc')),
		cmd_fxi => env_or_path('TEST_FENIX_FXI',
		    $cfg{testdir}->parent->child('fxi')->child('src')->child('fxi')),
	);

	$config = \%cfg;
	return $config;
}

=back

=head1 AUTHOR

Peter Pentchev E<lt>roam@ringlet.netE<gt>

=head1 COPYRIGHT AND LICENSE

Copyright (c) 2019  Peter Pentchev

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

=cut

1;
