use v5.12;
use strict;
use warnings;

package Test::Fenix::Run;

use strict;
use warnings;

use Path::Tiny;

use Test::Fenix::Config;

=head1 NAME

Test::Fenix::Run - utility functions for running Fenix programs

=head1 FUNCTIONS

=over 4

=item test_run

    test_run($prog);

Run a Fenix program and return the text sent to its standard output stream.
The compiled file C<$prog-E<gt>{test}-E<gt>{exe}> should be
a C<Path::Tiny> object referencing a file in the current directory.

The path to the Fenix interpreter is obtained from C<Test::Fenix::Config>.

=cut

sub test_run($)
{
	my ($test) = @_;

	my $fxi = Test::Fenix::Config::config->{cmd_fxi};
	my $output;

	Test::More::subtest('run a Fenix program' => sub {
		Test::More::plan tests => 2;

		my $cmd = Test::Command->new(cmd => [
			$fxi, $test->{test}{exe}->basename,
		]);
		$cmd->exit_is_num(0);
		$cmd->stderr_is_eq('');

		$output = $cmd->stdout_value;
	});

	return $output;
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
