Summary:	Fenix programming language compiler and interpreter
Summary(es):	Compilador e intérprete para el lenguaje de programación Fenix
Name:		fenix
Version:	0.85CVS
Release:	1
License:	GPL
Group:		X11/Development/Tools
Source0:	%{name}-%{version}.tar.bz2
URL:		http://fenix.divsite.net
BuildRequires:	SDL-devel >= 1.2.8
BuildRequires:  SDL_mixer-devel >= 1.2.6
BuildRequires:  libungif-devel >= 4.0
Requires:	SDL >= 1.2.8
Requires:	SDL_mixer >= 1.2.6
Requires:	libungif
BuildRoot:	%{_tmppath}/%{name}-%{version}-root-%(id -u -n)

%description
Fenix is a programming language aimed at games/multimedia
which greatly simplifies the task of accesing media content.

%description -l es
Fenix es un lenguage de programación orientado al mundo de los
videojuegos y del multimedia que simplifica en gran manera el acceso
a los contenidos multimedia.

%prep
%setup -q

%build
cd fxc/
%{__make} target=linux debug=false
cd ../fxi
%{__make} target=linux debug=false
cd ../map
%{__make} target=linux debug=false
cd ../fpg
%{__make} target=linux debug=false

%install
rm -rf $RPM_BUILD_ROOT

cd fxc/
%{__make} install \
	DESTDIR=$RPM_BUILD_ROOT
cd ../fxi/
%{__make} install \
	DESTDIR=$RPM_BUILD_ROOT
cd ../fpg/
%{__make} install \
	DESTDIR=$RPM_BUILD_ROOT
cd ../map/
%{__make} install \
	DESTDIR=$RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-, root, root, 0755)
%attr(0644,root,root) %doc COPYING
%{_bindir}/*

%define date	%(echo `LC_ALL="C" date +"%a %b %d %Y"`)
%changelog
* %{date} Fenix Team <fenix@divsite.net>
First attempt of creating a Fenix RPM for Fedora 4.
