dnl Process this file with autoconf to produce a configure script.
AC_INIT(Chemtool,1.6.13,martin@ruby.chemie.uni-freiburg.de,chemtool)
AC_CONFIG_SRCDIR(main.c)
AM_INIT_AUTOMAKE(chemtool, 1.6.13)

AC_CONFIG_HEADER(config.h)
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION(0.14.1)
dnl Checks for programs.
AC_PROG_CC
AC_PROG_MAKE_SET
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PATH_PROG(CP,cp)
AC_PATH_PROG(RM,rm)

dnl Checks for libraries.
dnl Replace `main' with a function in -libs:
AC_CHECK_LIB(ibs, main)
dnl Replace `main' with a function in -lm:
AC_CHECK_LIB(m, acos)


AC_DEFINE([LIBUNDO],1,[Define if you want to use the built-in undo functionality]) 

AC_DEFINE([MENU],1,[Define if you want to use pull-down menus instead of button bars])

AC_ARG_ENABLE(enable_gtk1,
[  --enable-gtk1           Use GTK+-1.x even if GTK+-2.x is available [no]],enable_gtk1=no)

AC_PATH_PROG(PKG_CONFIG, pkg-config, no)

AC_MSG_CHECKING([for GTK version and compile options ])

if test x$PKG_CONFIG = xno -o x$enable_gtk1 = xyes ; then
AM_PATH_GTK(1.0.2, , exit 1)
else
if $PKG_CONFIG gtk+-2.0 --cflags ; then
       GTK_CFLAGS="-DGTK2 `$PKG_CONFIG gtk+-2.0 --cflags` "
       GTK_LIBS=`$PKG_CONFIG gtk+-2.0 --libs`
dnl    use_gtk2=yes
       AC_DEFINE([GTK2], [1], [the GTK2 library])
       enable_stockgtk=yes
else
if $PKG_CONFIG --cflags gtk+ ; then
	GTK_CFLAGS=`$PKG_CONFIG gtk+ --cflags`       
	GTK_LIBS=`$PKG_CONFIG gtk+ --libs`
dnl    use_gtk2=no
fi
fi
fi

dnl AC_MSG_RESULT using GTK2 : $use_gtk2

dnl Checks for header files.
AC_PATH_XTRA
AC_HEADER_DIRENT
AC_HEADER_STDC

LDFLAGS="$X_LIBS"
AC_CHECK_LIB([X11],[XOpenDisplay])
dnl just an ugly hack to work around the more aggressive header checks
dnl in recent autoconf. Not sure if we should really rely on include
dnl paths being provided in CPPFLAGS, not CFLAGS, or if a nested set
dnl of tests encompassing all dependent headers would be in order.
dnl
dnl saved_CPPFLAGS=$CPPFLAGS
dnl CPPFLAGS="$CFLAGS $CPPFLAGS"

AC_CHECK_HEADERS(strings.h unistd.h libintl.h)

dnl CPPFLAGS=$saved_CPPFLAGS

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_STRUCT_TM

dnl Checks for library functions.
AC_FUNC_FNMATCH
AC_FUNC_MEMCMP
AC_FUNC_MMAP
AC_CHECK_FUNCS(getcwd getwd mkdir strtod strtol)

dnl Optional stuff
AC_ARG_WITH(localedir,
[  --with-localedir=DIR	  Set locale directory=DIR [DATADIR/locale]],
localedir=$withval,
[localedir=$datadir/locale])
dnl AC_DEFINE_UNQUOTED(LOCALEDIR=$withval),
dnl [AC_DEFINE_UNQUOTED(LOCALEDIR="\"\$datadir/locale"\")])
dnl "

dnl Optional stuff
AC_ARG_WITH(kdedir,
[  --with-kdedir=DIR	  Set KDE directory=DIR [/opt/kde3,/opt/kde2,/opt/kde]],
kdedir=$withval,
[kdedir="/opt/kde3 /opt/kde2 /opt/kde"])
AC_MSG_CHECKING([for KDE ])
for the_kdedir in $kdedir; do
if test -d $the_kdedir/share/mimelnk ; then
	kdemimedir=$the_kdedir
	AC_MSG_RESULT([apparently in $kdemimedir])
	break;
else
	KDEMIMEDIR=$KDEDIR
	if test -d $KDEMIMEDIR/share/mimelnk; then
	AC_MSG_RESULT([apparently in $KDEMIMEDIR])
	kdemimedir=$KDEMIMEDIR
	fi
fi
done
if test "x$kdemimedir" = "x" ; then
	AC_MSG_RESULT([no])
fi
dnl AC_DEFINE_UNQUOTED(KDEDIR=$withval),
dnl [AC_DEFINE_UNQUOTED(KDEDIR="\"\/opt/kde"\")])
dnl "

dnl Optional stuff
AC_ARG_WITH(gnomedir,
[  --with-gnomedir=DIR	  Set GNOME directory=DIR [/usr]],
gnomedir=$withval,
[gnomedir=/usr])
AC_MSG_CHECKING([for GNOME ])
if test -d $gnomedir/share/mime-info ; then
	gnomemimedir=$gnomedir
	AC_MSG_RESULT([apparently in $gnomemimedir])
else
	GNOMEMIMEDIR=$GNOMEDIR
	if test -d $GNOMEMIMEDIR/share/mime-info; then
	AC_MSG_RESULT([apparently in $GNOMEMIMEDIR])
	gnomemimedir=$GNOMEMIMEDIR
	fi
fi
if test "x$GNOMEMIMEDIR" = "x" ; then
	AC_MSG_RESULT([no])
fi
dnl AC_DEFINE_UNQUOTED(GNOMEMIMEDIR=$withval),
dnl [AC_DEFINE_UNQUOTED(GNOMEMIMEDIR="\"\/usr"\")])
dnl "

dnl AC_MSG_CHECKING(whether to enable locales)
dnl AC_ARG_ENABLE(locales,
dnl [  --enable-locales        Support localised messages [yes]],,enable_locales=yes)
dnl if test x$enable_locales = xyes; then
dnl if test "$ac_cv_header_libintl_h" = "yes" 
dnl then
dnl	AC_MSG_RESULT(yes)
dnl        AC_DEFINE_UNQUOTED([LOCALEDIR=\"\$datadir/locale\"],[],[])
dnl else
dnl	AC_MSG_RESULT(no)
dnl	AC_DEFINE([DISABLE_NLS],[1],[])
dnl fi
dnl else	
dnl	AC_MSG_RESULT(no)
dnl	AC_DEFINE([DISABLE_NLS],[1],[])
dnl fi

AC_MSG_CHECKING(whether to enable libundo)
AC_ARG_ENABLE(undo,
[  --enable-undo           Build with libundo [yes]],,enable_undo=yes)
if test x$enable_undo = xyes; then
	AC_MSG_RESULT(yes)
	AC_DEFINE([LIBUNDO],[1],[the undo library is available])
else
	AC_MSG_RESULT(no)
fi

AC_MSG_CHECKING(whether to use pull-down menus)
AC_ARG_ENABLE(menu,
[  --enable-menu           Enable new style pull-down-menu [yes]],,enable_menu=yes)
if test x$enable_menu = xyes; then
	AC_MSG_RESULT(yes)
	AC_DEFINE([MENU],[1],[])
else
	AC_MSG_RESULT(no)
fi

AC_MSG_CHECKING(whether to enable emf output)
AC_ARG_ENABLE(emf,
[  --enable-emf            Enable enhanced metafile output via libEMF [no]],,enable_emf=no)
if test x$enable_emf = xyes; then
	AC_MSG_RESULT(yes)
	AC_DEFINE([EMF], [1], [use optional libEMF instead of fig2dev for EMF output])
	EMFLIBS="-lEMF -lstdc++"
else
	AC_MSG_RESULT(no)
	EMFLIBS=""
fi


AC_MSG_CHECKING(whether standard GTK dialogs should be used instead of our own)
AC_ARG_ENABLE(stockgtk,
[  --enable-stockgtk	  use standard GTK menu and fileselection widgets [no]],,enable_stockgtk=no)

if test x$enable_stockgtk = xyes; then
	AC_MSG_RESULT(yes)
	MYGTKSRCS=""
	MYGTKOBJS=""
else
	AC_MSG_RESULT(no)
	ac_sys_system=`uname -s`
	if test "$ac_sys_system" = "AIX" -o "$ac_sys_system" = "CYGWIN"; then
	AC_MSG_RESULT([         sorry, overriding library functions is impossible on your system, enforcing "yes"])
	MYGTKSRCS=""
	MYGTKOBJS=""
	else
	MYGTKSRCS="gtkmenu.c gtkfilesel.c"
	MYGTKOBJS="gtkmenu.o gtkfilesel.o"
		if test "$ac_sys_system" = "Darwin" ; then
		LDFLAGS="-force_flat_namespace ${LDFLAGS}"
		fi
	fi
fi

dnl X_LIBS="$X_PRE_LIBS $X_LIBS $XLIB"

AC_SUBST(localedir)
AC_SUBST(localedir)
AC_SUBST(kdemimedir)
AC_SUBST(gnomemimedir)
AC_SUBST(EMFLIBS)
AC_SUBST(MYGTKSRCS)
AC_SUBST(MYGTKOBJS)
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)
dnl AC_SUBST(X_LIBS)

AC_OUTPUT(Makefile src-cht/Makefile m4/Makefile po/Makefile.in po/Makefile )
