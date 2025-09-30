/*@ignore@  splint should look away - we know things are redefined here */
/* GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GTK+ Team and others 1997-1999.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/. 
 */

#ifndef __GTK_FILESEL_H__
#define __GTK_FILESEL_H__


#include <gdk/gdk.h>
#include <gtk/gtkwindow.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define GTK_TYPE_FILE_SELECTION            (gtk_file_selection_get_type ())
#define GTK_FILE_SELECTION(obj)            (GTK_CHECK_CAST ((obj), GTK_TYPE_FILE_SELECTION, GtkFileSelection))
#define GTK_FILE_SELECTION_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_FILE_SELECTION, GtkFileSelectionClass))
#define GTK_IS_FILE_SELECTION(obj)         (GTK_CHECK_TYPE ((obj), GTK_TYPE_FILE_SELECTION))
#define GTK_IS_FILE_SELECTION_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_FILE_SELECTION))

typedef struct _GtkFileSelection       GtkFileSelection;
typedef struct _GtkFileSelectionClass  GtkFileSelectionClass;

struct _GtkFileSelection
{
  GtkWindow window;

  GtkWidget *dir_list;
  GtkWidget *file_list;
  GtkWidget *selection_entry;
  GtkWidget *selection_text;
  GtkWidget *main_vbox;
  GtkWidget *ok_button;
  GtkWidget *cancel_button;
  GtkWidget *help_button;

  /* These are not used.  Just fillers in the class structure */
  GtkWidget *history_pulldown;
  GtkWidget *history_menu;
  GList     *history_list;
  /* ***************** */
	
  GtkWidget *fileop_dialog;
  GtkWidget *fileop_entry;
  gchar     *fileop_file;
  gpointer   cmpl_state;
  
  GtkWidget *fileop_c_dir;
  GtkWidget *fileop_del_file;
  GtkWidget *fileop_ren_file;
  
  GtkWidget *button_area;
  GtkWidget *action_area;

  GtkWidget *history_combo;
  GList     *prev_history;
  GList     *next_history;
  GtkWidget *mask_entry;
  gchar     *mask;
  gchar     *saved_entry;

};

struct _GtkFileSelectionClass
{
  GtkWindowClass parent_class;
};


GtkType    gtk_file_selection_get_type            (void);
GtkWidget* gtk_file_selection_new                 (const gchar      *title);
void       gtk_file_selection_set_filename        (GtkFileSelection *filesel,
						   const gchar      *filename);
gchar*     gtk_file_selection_get_filename        (GtkFileSelection *filesel);
void	   gtk_file_selection_complete		  (GtkFileSelection *filesel,
						   const gchar	    *pattern);
void       gtk_file_selection_show_fileop_buttons (GtkFileSelection *filesel);
void       gtk_file_selection_hide_fileop_buttons (GtkFileSelection *filesel);

#if 0
static char *filesel_forward_pixmap[] = {
/* columns rows colors chars-per-pixel */
"16 16 2 1",
"  c Gray0",
". c none",
/* pixels */
"................",
"................",
"....... ........",
".......  .......",
".......   ......",
".......     ....",
"....... .    ...",
".       ..    ..",
".       ...    .",
".       ..    ..",
"....... .    ...",
".......     ....",
".......   ......",
".......  .......",
"....... ........",
"................"
};

static char *filesel_back_pixmap[] = {
/* columns rows colors chars-per-pixel */
"16 16 2 1",
"  c Gray0",
". c none",
/* pixels */
"................",
"........ .......",
".......  .......",
"......   .......",
"....     .......",
"...    . .......",
"..    ..       .",
".    ...       .",
"..    ..       .",
"...    . .......",
"....     .......",
"......   .......",
".......  .......",
"........ .......",
"................",
"................"
};

static char *filesel_up_pixmap[] = {
/* columns rows colors chars-per-pixel */
"16 16 2 1",
"  c Gray0",
". c none",
/* pixels */
"................",
"........ .......",
".......   ......",
"......     .....",
".....       ....",
".....   .   ....",
"....   ...   ...",
"...   .....   ..",
"..             .",
".......   ......",
".......   ......",
".......   ......",
".......   ......",
".......   ......",
".......   ......",
"................"
};

static char *filesel_home_pixmap[] = {
/* columns rows colors chars-per-pixel */
"16 16 2 1",
"  c Gray0",
". c none",
/* pixels */
"................",
"................",
"........ .. ....",
"....... . . ....",
"...... ...  ....",
"..... ..... ....",
".... ....... ...",
"...           ..",
".. .         . .",
".... ....... ...",
".... .  .  . ...",
".... .  .  . ...",
".... ....  . ...",
"....         ...",
"................",
"................"
};

static char *filesel_reload_pixmap[] = {
/* columns rows colors chars-per-pixel */
"16 16 2 1",
"  c Gray0",
". c none",
/* pixels */
"................",
"................",
"................",
"......    . ....",
"....  ....  ....",
"...  ....   ....",
"... ............",
"..  ......... ..",
".. .......... ..",
".. .........  ..",
".....   ...  ...",
".....  ...  ....",
"..... .   ......",
"................",
"................",
"................"
};
#endif

 static char *filesel_back_pixmap[] = {
 /* columns rows colors chars-per-pixel */
"18 18 31 1",
"  c #000000",
". c #0a0a0a",
"X c #0e160c",
"o c #333632",
"O c #373937",
"+ c #383b37",
"@ c #2f402b",
"# c #425e3c",
"$ c #44613d",
"% c #476440",
"& c #759b6c",
"* c #7ba172",
"= c #82a778",
"- c #83a879",
"; c #a9afa8",
": c #aeb2ac",
"> c #b4b9b3",
", c #abc1a6",
"< c #afc6aa",
"1 c #b1c7aa",
"2 c #b3c9ad",
"3 c #b6cbb0",
"4 c #baceb5",
"5 c #dfe8dd",
"6 c #e6ece4",
"7 c #e9efe7",
"8 c #eaefe8",
"9 c #ecf1eb",
"0 c #f0f4ef",
"q c #f1f5f1",
"w c None",
"wwwwwwwwwwwwwwwwww",
"wwwwwwwwwwwwwwwwww",
"wwwwwwwwwwwwwwwwww",
"wwwwwwwwwwwww  www",
"wwwwwwwwwww +; www",
"wwwwwwwww O:97 www",
"wwwwwww o:99q9 www",
"wwwww o:q999q9 www",
"www +>95699909 www",
"www@,<<2224443.www",
"www X#&----=-* www",
"wwwww X%&--=-* www",
"wwwwwww X#&--* www",
"wwwwwwwww X#&* www",
"wwwwwwwwwww X$ www",
"wwwwwwwwwwwww  www",
"wwwwwwwwwwwwwwwwww",
"wwwwwwwwwwwwwwwwww"
 };
static char *filesel_forward_pixmap[] = {
 /* columns rows colors chars-per-pixel */
"18 18 33 1",
"  c #000000",
". c #0a0a0a",
"X c #0e160c",
"o c #333632",
"O c #373937",
"+ c #383b37",
"@ c #414c3e",
"# c #425e3c",
"$ c #45623e",
"% c #749a6b",
"& c #82a778",
"* c #84a87a",
"= c #8bae81",
"- c #91b286",
"; c #a0a99e",
": c #a5ada3",
"> c #a7bea2",
", c #b0b4af",
"< c #b0c7aa",
"1 c #b2c8ac",
"2 c #b9ceb4",
"3 c #c1d3bb",
"4 c #dde6db",
"5 c #dfe8dd",
"6 c #e5ece3",
"7 c #e8eee6",
"8 c #edf2ec",
"9 c #f0f4ef",
"0 c #f1f5f1",
"q c #f7f9f6",
"w c #f8faf7",
"e c #f8faf8",
"r c None",
 /* pixels */
"rrrrrrrrrrrrrrrrrr",
"rrrrrrrrrrrrrrrrrr",
"rrrrrrrrrrrrrrrrrr",
"rrr  rrrrrrrrrrrrr",
"rrr ,+ rrrrrrrrrrr",
"rrr q7:O rrrrrrrrr",
"rrr w086:o rrrrrrr",
"rrr w00886;o rrrrr",
"rrr w8887654:+ rrr",
"rrr.322221111>@rrr",
"rrr -&*&&*&%#X rrr",
"rrr -*&*&%$X rrrrr",
"rrr =*&%#X rrrrrrr",
"rrr *%#X rrrrrrrrr",
"rrr $X rrrrrrrrrrr",
"rrr  rrrrrrrrrrrrr",
"rrrrrrrrrrrrrrrrrr",
"rrrrrrrrrrrrrrrrrr"
 };

 static char *filesel_home_pixmap[] = {
 /* columns rows colors chars-per-pixel */
"18 18 36 1",
"  c #000000",
". c #181b16",
"X c #20241e",
"o c #292c27",
"O c #323431",
"+ c #363d31",
"@ c #3f3f3f",
"# c #495044",
"$ c #53584f",
"% c #676a65",
"& c #696b67",
"* c #737472",
"= c #7f7f7f",
"- c #7a8771",
"; c #7f8b76",
": c #81887b",
"> c #899580",
", c #919d8a",
"< c #95a08e",
"1 c #9aa593",
"2 c #9ea897",
"3 c #a2a59f",
"4 c #a1ac99",
"5 c #afb9a8",
"6 c #b5bfb0",
"7 c #bfc7ba",
"8 c #c2cabe",
"9 c #c7cec3",
"0 c #c8cfc4",
"q c #c9d0c5",
"w c #ccd2c8",
"e c #d0d6cc",
"r c #e9ece7",
"t c #edefeb",
"y c #eff1ee",
"u c None",
 /* pixels */
"uuuuuuuuuuuuuuuuuu",
"uuuuuuuuu uuuuuuuu",
"uuuuuuuu@ou@%Ouuuu",
"uuuuuuu@y4X%q$uuuu",
"uuuuuu@rq84%q$uuuu",
"uuuuu@r00q846$uuuu",
"uuuu@r0000084#uuuu",
"uuu@y00000qq81.uuu",
"uu@yw000q0q9982.uu",
"u *3w00000q888:+  ",
"uuu=e0000089q5+uuu",
"uuu=e0<421q095+uuu",
"uuu=wq40819995+uuu",
"uuu=e048w1qq95+uuu",
"uuu=e82wq<7775+uuu",
"uuu*,>;,,->>>>+uuu",
"uuu            uuu",
"uuuuuuuuuuuuuuuuuu"
 };
 static char *filesel_reload_pixmap[] = {
 /* columns rows colors chars-per-pixel */
"18 18 23 1",
"  c #000000",
". c #0e0e0e",
"X c #0d110a",
"o c #11150d",
"O c #141a10",
"+ c #1c1f19",
"@ c #1b2215",
"# c #1e2618",
"$ c #26301e",
"% c #2a3421",
"& c #2f3c25",
"* c #37452b",
"= c #38472c",
"- c #3b4a2e",
"; c #3e4d30",
": c #415233",
"> c #4a5d3a",
", c #4c603c",
"< c #50643f",
"1 c #526740",
"2 c #546942",
"3 c #586d44",
"4 c None",
 /* pixels */
"444444444444444444",
"444444444 44444444",
"44444444O 44444444",
"4444444O,o  444444",
"444444O321>&# 4444",
"4444 4 :2=$:>;X444",
"444  44 ; 4.+%* 44",
"44  4444  444 %@44",
"44@ 4444444444X@44",
"44@@4444 44444 X44",
"44X*X.44 O4444  44",
"44 %;&& o2O44  444",
"444 #::,123O444444",
"44444 o$=1: 444444",
"44444444 - 4444444",
"44444444  44444444",
"444444444444444444",
"444444444444444444"
};
/* XPM */
static char *filesel_up_pixmap[] = {
/* columns rows colors chars-per-pixel */
"18 18 31 1",
"  c #000000",
". c #0a0a0a",
"X c #0e160c",
"o c #333632",
"O c #373937",
"+ c #383b37",
"@ c #2f402b",
"# c #425e3c",
"$ c #44613d",
"% c #476440",
"& c #759b6c",
"* c #7ba172",
"= c #82a778",
"- c #83a879",
"; c #a9afa8",
": c #aeb2ac",
"> c #b4b9b3",
", c #abc1a6",
"< c #afc6aa",
"1 c #b1c7aa",
"2 c #b3c9ad",
"3 c #b6cbb0",
"4 c #baceb5",
"5 c #dfe8dd",
"6 c #e6ece4",
"7 c #e9efe7",
"8 c #eaefe8",
"9 c #ecf1eb",
"0 c #f0f4ef",
"q c #f1f5f1",
"w c None",
/* pixels */
"wwwwwwwwwwwwwwwwww",
"wwwwwwwwwwwwwwwwww",
"wwwwwwwwwwwwwwwwww",
"wwwwwww @ wwwwwwww",
"wwwwwwwX,+wwwwwwww",
"wwwwww #<> wwwwwww",
"wwwwwwX&19owwwwwww",
"wwwww %-25: wwwwww",
"wwwwwX&-26qowwwwww",
"wwww #=-299: wwwww",
"wwwwX&=-4999+wwwww",
"www #-==49q9: wwww",
"wwwX&---49qq9Owwww",
"ww $****29997; www",
"ww      .      www",
"wwwwwwwwwwwwwwwwww",
"wwwwwwwwwwwwwwwwww",
"wwwwwwwwwwwwwwwwww"
 };

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __GTK_FILESEL_H__ */

/*@end@*/
