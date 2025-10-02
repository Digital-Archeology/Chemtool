#ifndef INOUT_COMMON_H
#define INOUT_COMMON_H

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include "ct1.h"
#pragma GCC diagnostic pop
#include "inout.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include "gdk/gdkprivate.h"
#include <locale.h>
#ifdef EMF
#include <libEMF/emf.h>
#endif

#ifndef ENABLE_NLS
#define _(Text) Text
#else
#include <libintl.h>
#define _(Text) gettext(Text)
#endif

#ifdef GTK2
extern PangoFontDescription *font[7], *smallfont[7], *symbfont[7],
                     *ssymbfont[7], *slfont[7],
                     *boldfont[7], *textfont[7];
#else
extern GdkFont *font[7], *smallfont[7], *symbfont[7], *boldfont[7], *slfont[7];
#endif

#endif /* INOUT_COMMON_H */
