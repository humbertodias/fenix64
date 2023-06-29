use v5.12;
use strict;
use warnings;

package Test::Fenix::Compile;

use strict;
use warnings;

use Path::Tiny;

use Test::Fenix::Config;

=head1 NAME

Test::Fenix::Compile - utility functions for compiling Fenix programs

=head1 FUNCTIONS

=over 4

=item test_compile

    test_compile($prog);

Compile a Fenix program.  The source file C<$prog-E<gt>{test}-E<gt>{src}> and
the desired compiled file C<$prog-E<gt>{test}-E<gt>{exe}> should be
C<Path::Tiny> objects referencing files in the current directory.

The path to the Fenix compiler is obtained from C<Test::Fenix::Config>.

=cut

sub test_compile($)
{
	my ($test) = @_;

	my $fxc = Test::Fenix::Config::config->{cmd_fxc};

	Test::More::subtest('compile a Fenix program' => sub {
		Test::More::plan tests => 4;

		my $cmd = Test::Command->new(cmd => [
			$fxc, '-d', $test->{test}{src}->basename,
		]);
		$cmd->exit_is_num(0);
		$cmd->stdout_like(qr/END/);
		$cmd->stderr_like(qr/----- Main procedure/);

		Test::More::ok -f $test->{test}{exe},
		    'the compiled program exists';
	});
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
