use v5.12;
use strict;
use warnings;

package Test::Fenix::Image::Format;

use strict;
use warnings;

=head1 NAME

Test::Fenix::Image::Format - base class for handling different images

=head1 SYNOPSIS

  use Path::Tiny;
  use Test::Fenix::Image::Format::<impl>;

  my $fmt = Test::Fenix::Image::Format::<impl>->new();
  my $im = GD::Image->new(...);
  $fmt->save(path($fmt->filename('test')), $im);

  my $out_path = path($fmt->filename('out'));
  system { 'fenix-map' } ('fenix-map', $fmt->opt, 'test.map', $out_path);
  my $im = $fmt->load($out_path);  # a GD::Image object

Note that the C<save()> and C<load()> methods expect the first
argument to be a C<Path::Tiny> object, not a plain string.

=head1 METHODS

=over 4

=item new

    my $fmt = Test::Fenix::Image::Format::DerivedClass(...);

Create a C<Test::Fenix::Image::Format> object with no parameters.
Not to be used directly; instantiate an object from a derived class instead.

=cut

sub new
{
	my $type = shift;
	my $type_name = ref $type || $type;

	return bless {
	}, $type_name;
}

=item opt

    my $option_name = $fmt->opt;

Return the name of the option to be passed to the I<fenix-map>
program to extract a file in this format from a Fenix map file.

=cut

sub opt($)
{
	my ($self) = @_;

	die ref($self)."->opt() must be overridden\n";
}

=item filename

    my $fname = $fmt->filename($basename)

Return a filename formed by the specified basename and
the format-specific extension.

=cut

sub filename($ $)
{
	my ($self, $_name) = @_;

	die ref($self)."->filename() must be overridden\n";
}

=item load

    my $img = $fmt->load($path)

Load a file in this format from the specified path (represented
as a C<Path::Tiny> object).

=cut

sub load($ $)
{
	my ($self, $_file) = @_;

	die ref($self).'->ld() must be overridden';
}

=item save

    $fmt->save($path, $image)

Save the specified C<GD::Image> object to the specified path
(represented as a C<Path::Tiny> object).

=cut

sub save($ $ $)
{
	my ($self, $_path, $_image) = @_;

	die ref($self).'->save() must be overridden';
}

=back

=head1 SEE ALSO

C<Test::Fenix::Image::Format::GIF>, C<Test::Fenix::Image::Format::PNG>

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

package Test::Fenix::Image::Format::PNG;

use strict;
use warnings;

use GD::Image;

use base qw(Test::Fenix::Image::Format);

=head1 NAME

Test::Fenix::Image::Format::PNG - load and save images in the PNG format

=head1 METHODS

=over

=item new

    my $fmt = Test::Fenix::Image::Format::PNG->new;

Create a format object with no parameters.

=cut

sub filename($ $)
{
	my ($self, $name) = @_;

	return "$name.png";
}

sub opt($)
{
	my ($self) = @_;

	return '-g';
}

sub load($ $)
{
	my ($self, $path) = @_;

	return GD::Image->newFromPng($path);
}

sub save($ $ $)
{
	my ($self, $path, $image) = @_;

	return $path->spew({binmode => ':raw'}, $image->png);
}

=back

=cut

package Test::Fenix::Image::Format::GIF;

use strict;
use warnings;

use base qw(Test::Fenix::Image::Format);

=head1 NAME

Test::Fenix::Image::Format::GIF - load and save images in the GIF format

=head1 METHODS

=over 4

=item new

    my $fmt = Test::Fenix::Image::Format::GIF->new;

Create a format object with no parameters.

=cut

sub filename($ $)
{
	my ($self, $name) = @_;

	return "$name.gif";
}

sub save($ $ $)
{
	my ($self, $path, $image) = @_;

	return $path->spew({binmode => ':raw'}, $image->gif);
}

=back

=cut

1;
