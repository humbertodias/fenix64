use v5.12;
use strict;
use warnings;

package Test::Fenix::Image::Generate;

use strict;
use warnings;

=head1 NAME

Test::Fenix::Image::Generate - utility functions for generating test images

=head1 FUNCTIONS

=over 4

=cut

sub _seq_colors($)
{
	my ($img) = @_;

	my @res;
	my $cidx = 0;

	for my $pos (0..2) {
		for my $value (0..63) {
			my @color = (0, 0, 0);
			$color[$pos] = $value * 4;
			$img->colorAllocate($color[0], $color[1], $color[2]);
			push @res, [@color];
		}
	}

	for my $value (0..63) {
		my $v = $value * 4;
		$img->colorAllocate($v, $v, $v);
		push @res, [$v, $v, $v];
	}

	return @res;
}

=item seq

    my $img = Test::Fenix::Image::Generate::seq($width);

Generate a square image with several sequential color gradients. 

=cut

sub seq($)
{
	my ($n) = @_;

	my $img = GD::Image->new($n, $n, 0);
	_seq_colors $img;
	my $current = 0;
	for my $y (0..$n - 1) {
		for my $x (0..$n - 1) {
			$img->setPixel($x, $y, $current);
			$current = ($current + 1) % 256;
		}
	}
	return $img;
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
