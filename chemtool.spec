Summary: Chemtool is a program for 2D drawing of organic molecules
Summary(de): Chemtool ist ein Programm zum Zeichnen von Molekülformeln
Summary(pl): Chemtool - program do rysowania 2-wymiarowych czsteczek organicznych.
Name: chemtool
Version: 1.6.14
Release: 1
License: GPL
Group: Applications/Chemistry
Source: http://ruby.chemie.uni-freiburg.de/~martin/chemtool/%{name}-%{version}.tar.gz
URL: http://ruby.chemie.uni-freiburg.de/~martin/chemtool/chemtool.html
Packager: Martin Kroeker <martin@ruby.chemie.uni-freiburg.de>
Vendor: Thomas Volk & Martin Kroeker
Prefix: /usr
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
%define name0 cht
%define docdir %{_docdir}/%{name}-%{version}
%define	exampledir %{_datadir}/%{name}-%{version}/examples

%description
Chemtool is a program for drawing organic molecules easily and store them in
a variety of output formats including as a X bitmap, Xfig, SVG or EPS file. 
It runs under the X Window System using the GTK widget set.

%description -l de
Chemtool ist ein einfaches Zeichenprogramm für organische Moleküle. Lewisformeln
können in einer ganzen Reihe von Ausgabeformaten gespeichert werden, u.a. als
SVG- und EPS-Dateien oder auch im XFig-Format. Chemtool setzt X11 und die
GTK-Grafikbibliothek voraus.

%description -l pl
Chemtool jest programem do rysowania czsteczek organicznych i zapisu
ich jako pliki X-bitmap, Xfig lub EPS. Pracuje pod X Window uywajc
bibliotek GTK.

%prep
rm -rf $RPM_BUILD_ROOT

%setup
%build
./configure --prefix=%{_prefix}
make

%install
rm -rf $RPM_BUILD_ROOT
make prefix=$RPM_BUILD_ROOT%{_prefix} bindir=$RPM_BUILD_ROOT%{_bindir} \
    localstatedir=$RPM_BUILD_ROOT%{_localstatedir} \
    datadir=$RPM_BUILD_ROOT%{_datadir} \
    includedir=$RPM_BUILD_ROOT%{_includedir} \
    mandir=$RPM_BUILD_ROOT%{_mandir} \
    sysconfdir=$RPM_BUILD_ROOT%{_sysconfdir} install

install -d $RPM_BUILD_ROOT%{docdir}
install -d $RPM_BUILD_ROOT%{_datadir}/mimelnk/application
install -d $RPM_BUILD_ROOT%{_datadir}/icons/hicolor/32x32/apps
install -d $RPM_BUILD_ROOT%{_datadir}/applications
install -d $RPM_BUILD_ROOT%{_datadir}/pixmaps
install -d $RPM_BUILD_ROOT%{exampledir}
install -m 644 examples/* $RPM_BUILD_ROOT%{exampledir}
install -m 644 {COPYING,TODO,README,ChangeLog,using_chemtool.html}  $RPM_BUILD_ROOT%{docdir}
install -m 644 kde/mimelnk/application/x-chemtool.desktop $RPM_BUILD_ROOT%{_datadir}/mimelnk/application
install -m 644 gnome/gnome-application-%{name}.png $RPM_BUILD_ROOT%{_datadir}/icons/hicolor/32x32/apps/%{name}.png
install -m 644 gnome/chemtool.desktop  $RPM_BUILD_ROOT%{_datadir}/applications
install -m 644 gnome/chemtool.png $RPM_BUILD_ROOT%{_datadir}/pixmaps

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-, root, root)
%{_bindir}
%{exampledir}
%{_mandir}
%{_datadir}/pixmaps
%{_datadir}/applications
%{_datadir}/mimelnk
%{_datadir}/icons
%{_datadir}/locale
%doc %{docdir}


%changelog
* Mon Aug 12 2013 Martin Kroeker <martin@ruby.chemie.uni-freiburg.de>
- Updated build system to autoconf-2.69, updated version number.
* Wed Nov 11 2007 Martin Kroeker <martin@ruby.chemie.uni-freiburg.de>
- Updated version number
* Thu Jun 21 2007 Martin Kroeker <martin@ruby.chemie.uni-freiburg.de>
- Moved installation of manpages into the Makefile
* Thu Jun 8 2007 Martin Kroeker <martin@ruby.chemie.uni-freiburg.de>
- Fixed creation of examples and man directories 
- Changed packager name to myself (instead of Radek Liboska who 
  contributed the original spec file) 
* Mon May 28 2007 Martin Kroeker <martin@ruby.chemie.uni-freiburg.de>
- Replaced most of the install target with a make install
* Mon Apr 9 2007 Martin Kroeker <martin@ruby.chemie.uni-freiburg.de>
- Updated version number
- Renamed Copyright field to License 
* Tue Nov 21 2006 Martin Kroeker <martin@ruby.chemie.uni-freiburg.de>
- Updated version number
- Install a copy of the GPL license text (COPYING) with the other documentation 
* Thu Feb 23 2006 Martin Kroeker <martin@ruby.chemie.uni-freiburg.de>
- Updated version number
* Mon Jan 2 2006 Martin Kroeker <martin@ruby.chemie.uni-freiburg.de>
- Removed rpm icon resource
- Incorporated Polish translation of the descriptions from the old pld 
  spec file, added German translation
* Sun Jul 24 2005 Martin Kroeker <martin@ruby.chemie.uni-freiburg.de>
- Add nl to the list of installable locale files
* Fri Jun 17 2005 Martin Kroeker <martin@ruby.chemie.uni-freiburg.de>
- updated Version
* Sat Feb 12 2005 Martin Kroeker <martin@ruby.chemie.uni-freiburg.de>
- updated Version
* Mon Jan 31 2005 Martin Kroeker <martin@ruby.chemie.uni-freiburg.de>
- updated Version, configure, configure.in
* Sat Oct 16 2004 Martin Kroeker <martin@ruby.chemie.uni-freiburg.de>
- updated Version, Makefile.in
* Thu May 13 2004 Martin Kroeker <martin@ruby.chemie.uni-freiburg.de>
- updated Version, configure, Makefile.in
* Wed May  5 2004 Martin Kroeker <martin@ruby.chemie.uni-freiburg.de>
- updated Version
* Sat May  1 2004 Martin Kroeker <martin@ruby.chemie.uni-freiburg.de>
- updated Version, added html copy of usage instructions
* Sun Jul 27 2003 Radek Liboska <radek.liboska@uochb.cas.cz>
- RedHat 9 KDE+GNOME desktop style adopted
* Tue Jul 22 2003 Radek Liboska <radek.liboska@uochb.cas.cz>
- default "_path" vars used instead of the fixed paths
* Thu Nov  7 2002 Martin Kroeker <martin@ruby.chemie.uni-freiburg.de>
- updated Version, URLs and program description
* Wed Mar 14 2002 Radek Liboska <radek.liboska@uochb.cas.cz>
- the icon stuff and gnome menu integration
* Sat Mar  6 2002 Radek Liboska <radek.liboska@uochb.cas.cz>
- spec file clean up
* Fri Jul 27 2001 Radek Liboska <radek.liboska@uochb.cas.cz>
- 1.4 See %prefix/doc/%{name}-%{version}/ChangeLog for change log
