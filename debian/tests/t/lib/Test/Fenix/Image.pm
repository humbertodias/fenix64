use v5.12;
use strict;
use warnings;

package Test::Fenix::Image;

use strict;
use warnings;

use GD::Image;
use Path::Tiny;

use Test::Fenix::Image::Format;
use Test::Fenix::Image::Generate;

=head1 NAME

Test::Fenix::Image - utility functions for testing Fenix image handling

=head1 FUNCTIONS

=over 4

=cut

my %generate_patterns = (
	seq => \&Test::Fenix::Image::Generate::seq,
);

=item generate

    my $img = generate($type, $width);

Generate a square image with the specified pattern type and width.

=cut

sub generate($ $)
{
	my ($type, $n) = @_;
	my $pattern = $generate_patterns{$type};
	die "Internal error: bad type '$type'" unless defined $pattern;
	return $pattern->($n);
}

=item generate_patterns

    my @patterns = generate_patterns();

List the available image patterns.

=cut

sub generate_patterns()
{
	return sort keys %generate_patterns;
}

my %format_handlers = (
	png => sub { Test::Fenix::Image::Format::PNG->new() },
	gif => sub { Test::Fenix::Image::Format::GIF->new() },
);

=item format

    my $fmt = format($type);

Get a load/save handler for the specified image format (an object of
the appropriate C<Test::Fenix::Image::Format> subtype).

=cut

sub format($)
{
	my ($format) = @_;
	my $handler = $format_handlers{$format};
	die "Unknown image format: $format\n" unless defined $handler;
	return $handler->();
}

=item compare_images

    is '', compare_images($src, $dst), 'the images are the same';

Compare the height, width, and the RGB values of each pixel for
the two specified images.  Return an empty string if the images are
the same, a descriptive string usable in an error message if they differ.

=cut

sub compare_images($ $)
{
	my ($src, $dst) = @_;
	my ($height, $width) = ($src->height, $src->width);
	my ($dst_height, $dst_width) = ($dst->height, $dst->width);

	return "width $dst_width <> $width" if $dst_width != $width;
	return "height $dst_height <> $height" if $dst_height != $height;
	for my $y (0..$height - 1) {
		for my $x (0..$width - 1) {
			my $src_idx = $src->getPixel($x, $y);
			my $src_color = [$src->rgb($src_idx)];
			my $dst_idx = $dst->getPixel($x, $y);
			my $dst_color = [$dst->rgb($src_idx)];
			if ($dst_color->[0] != $src_color->[0] ||
			    $dst_color->[1] != $src_color->[1] ||
			    $dst_color->[2] != $src_color->[2]) {
				return "($x, $y) ".
			    	    "(@${src_color}) (@${dst_color})";
			}
		}
	}
	return '';
}

=item test_image_to_map

    test_image_to_map($image, $file);

Save a C<GD::Image> object to a file with the type specified in C<$file>,
then convert it to a *.map file.

NOTE: the C<$file-E<gt>{file}> and C<$file-E<gt>{map}> paths are expected to
refer to the current directory!

=cut

sub test_image_to_map($ $)
{
	my ($image, $file) = @_;

	Test::More::subtest('create a map file from the image' => sub {
		my $fname = path($file->{file}->basename);
		my $mapname = path($file->{map}->basename);
		unlink $fname;
		$file->{format}->save($fname, $image);

		unlink $mapname;
		my $cmd = Test::Command->new(cmd => [
			Test::Fenix::Config::config()->{cmd_map},
			'-m', $fname,
		]);
		$cmd->exit_is_num(0);
		$cmd->stdout_isnt_eq('');
		$cmd->stderr_is_eq('');
		Test::More::isnt index($cmd->stdout_value, $mapname), -1,
		    "'$mapname' present in the output";
		Test::More::ok -f $mapname, "'$mapname' was created";
	});
}

=item test_map_to_image

    test_map_to_image($file);

Convert a *.map file to the destination image type (*.gif or *.png
depending on what is specified in C<$file>), then try to load it and
return the new C<GD::Image> object.

NOTE: the C<$file-E<gt>{file}> and C<$file-E<gt>{map}> paths are expected to
refer to the current directory!

=cut

sub test_map_to_image($)
{
	my ($file) = @_;
	
	my $res;
	Test::More::subtest('create an image from a map file' => sub {
		my $mapname = path($file->{map}->basename);
		my $fname = path($file->{file}->basename);
		unlink $fname;
		my $cmd = Test::Command->new(cmd => [
			Test::Fenix::Config::config()->{cmd_map},
			$file->{format}->opt, $mapname,
		]);
		$cmd->exit_is_num(0);
		$cmd->stdout_isnt_eq('');
		$cmd->stderr_is_eq('');
		Test::More::isnt index($cmd->stdout_value, $fname), -1,
		    "'$fname' present in the output";
		Test::More::ok -f $fname, "'$fname' was created";

		$res = $file->{format}->load("$fname"),
	});

	return $res;
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
