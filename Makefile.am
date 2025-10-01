# Example for use of GNU gettext.
# Copyright (C) 2003 Free Software Foundation, Inc.
# This file is in the public domain.
#
# Makefile configuration - processed by automake.

# General automake options.
AUTOMAKE_OPTIONS = foreign no-dependencies
ACLOCAL_AMFLAGS = -I m4

# The list of subdirectories containing Makefiles.
SUBDIRS = m4 po src-cht

# The list of programs that are built.
bin_PROGRAMS = chemtool src-cht/cht

# The source files of the 'chemtool' program.
chemtool_SOURCES = main.c draw.c graph.c chemproc.c undo.c inout.c templates.h gettext.h

# Define a C macro LOCALEDIR indicating where catalogs will be installed.
localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

# Make sure the gettext.h include file is found.
AM_CPPFLAGS = -I. -I$(srcdir) @GTK_CFLAGS@
AM_CFLAGS = -Wall -Wextra -Werror -pedantic -Wformat=2 -Wstrict-prototypes -Wmissing-prototypes -Wold-style-definition -Wshadow

# Link time dependencies.
LDADD = @LIBINTL@ @GTK_LIBS@ @EMFLIBS@ 

# Additional files to be distributed.
EXTRA_DIST = autogen.sh autoclean.sh

# Manual files
dist_man_MANS = chemtool.1 cht.1

# Softlink installed program to batch version
install-exec-hook:
	cd $(DESTDIR)$(bindir) && \
	$(LN_S) chemtool chemtoolbg

