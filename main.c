/*
 *                            Chemtool version 1.6.14
 *
 *      written by Thomas Volk 
 *      Jan 1998
 *      extensions and GTK-based rewrite by Martin Kroeker 1999-2007,2010,2011,2013
 *      with contributions by Radek Liboska and Michael Banck
 *      You can use Chemtool under the terms 
 *      of the GNU General Public License
 *
 *      This software comes with ABSOLUTELY NO WARRANTY
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <ctype.h>
#include <gdk/gdkx.h>
#include "bitmap1.h"
#include "templates.h"
#include "chemcurs.h"
#include "chemtool.xpm"
#include <gdk/gdkkeysyms.h>
#include <stdio.h>
#include <stdlib.h>
#if 0
#include <unistd.h> /* memmove*/
#endif
#include <strings.h>
#include <string.h>
#include <signal.h>
#include <locale.h>
#include "ct.h"
#ifndef ENABLE_NLS
#define _(Text) Text
#else
#include <libintl.h>
#define _(Text) gettext(Text)
#endif


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"


GtkWidget *filew;
GtkWidget *templates;
GtkWidget *messagew, *message;
GtkWidget *expw;
GtkWidget *mainw;
GtkWidget *window;
GtkWidget *scrolled_window;
GtkWidget *hexbutton, *pent1button, *pent2button, *octbutton, *ltextbutton,
  *ctextbutton, *rtextbutton, *bondbutton, *movebutton, *markbutton,*colorbutton,
  *rotatebutton, *hflipbutton, *vflipbutton, *copybutton, *addbutton,
  *tempbutton, *cleanbutton, *rescalebutton, *splinebutton,*fontbutton,
  *bgcolorbutton;
#if 0
GtkWidget *fontsel,*fontselw;
#endif
GtkWidget *textlabel;
GtkStyle *normalfontstyle,*seriffontstyle;
GtkWidget *scale, *pscale;
GtkWidget *bracketbutton, *rbracketbutton, *r2bracketbutton, *bracebutton, *box1button,
  *box2button, *box3button, *box4button, *boxbutton;
GtkWidget *boxmenu,*penmenu;
GtkWidget *drawing_area, *preview_area;


#if (GTK_MINOR_VERSION >2)
/* --- Widgets used by the Configurable Options dialog --- */

GtkWidget *printer_dialog;

/* Widgets used for layout structure */

GtkWidget *master_vbox;
GtkWidget *notebook;
GtkWidget *table;

/* Widgets that contain data and are accessed from elsewhere */

GtkWidget *color_button;
GtkWidget *printer_name_entry;
GtkWidget *print_scale_spin;
GtkWidget *datadir_entry;
GtkWidget *datamask_entry;
GtkWidget *whiteout_check;
GtkWidget *intl_check;
GtkWidget *paper_size_combo;
GtkWidget *orientation_combo;
GtkWidget *print_cmd_combo;
GtkWidget *eps_preview_combo;
GtkWidget *bond_length_entry;
GtkWidget *dbond_dist_spin;

#else
GtkWidget *papermenu, *papersizes, *orientmenu, *orientations, *printcmdmenu,
  *printcmds, *prqueue;
GtkWidget *epsoptionmenu,*epsoptions,*whiteoutbutton,*intlbutton;
GtkWidget *colorsel;
GtkWidget *colorseldialog;
GtkWidget *defaultdir, *defaultext,*base_bondlen,*doubledist;
#endif

GtkWidget *bondomenu, *bondmenu, *menuitem;
GtkWidget *fontmenu,*fontsizes;
GtkWidget *msgtext;
GtkWidget *pdbhbox, *labbutton[5],*sdfhbox,*sdflabel,*sdfbutton[3];
GtkWidget *babelcmds,*babelcmdmenu;
GtkWidget *babelexpcmds,*babelexpmenu;
GdkCursor *cursor_pencil, *cursor_text, *cursor_bonds, *cursor_markTLC,
  *cursor_markBRC, *cursor_move, *cursor_rescale, *cursor_rotate,
  *cursor_busy;
GdkBitmap *source, *mask;
GtkWidget *oldpixmap;
GdkColor fg = { 0, 0, 0, 0 };	/* Black */
GdkColor bg = { 0, 32767, 32767, 65535 };	/* Blue */
GdkColor wh = { 0, 65535, 65535, 65535 };	/* White */
GdkColor background={0,0xeeee,0x0000,0x0000};
#ifdef GTK2
GtkTextBuffer *msgtextbuffer;
GtkTextIter iter;
GtkAdjustment *msgadjustment;
#endif
char filename[512];
char expname[512];
static int loadsave = 1;
static int expmode = 0;
static int curbond = 0;
static int ringtype = 0;
int newpapersize = 0;
int neworientation = 0;
int newprintcommand = 0;
int newepsoption = 0;
int splinepoint = 0;
int oldwhiteout = 0;
int oldintlchars= 0;
int textentry=-1;

int pendown = 0;
int batchmode = 0;

struct
{
  int x;
  int y;
}
spline[4];

static gint
configure_event (GtkWidget * mainw, GdkEventConfigure * event)
/* callback function to create an empty white drawing area on startup */
{
  (void) event;

  if (picture)
    gdk_pixmap_unref (picture);

  picture = gdk_pixmap_new (mainw->window,
			    mainw->allocation.width + 100,
			    mainw->allocation.height + 100, -1);

  gdk_draw_rectangle (picture,
		      mainw->style->white_gc,
		      (gint) TRUE, 0, 0,
		      (gint) mainw->allocation.width,
		      (gint) mainw->allocation.height);
  Display_Mol ();
  return (gint) TRUE;
}

static gint
configure_preview (GtkWidget * mainw, GdkEventConfigure * event)
/* callback function to create an empty white drawing area on startup */
{
  (void) event;
  if (preview) {
      gdk_pixmap_unref (preview);
	preview=NULL;
	}
  preview = gdk_pixmap_new (mainw->window, 200, 100, -1);

  gdk_draw_rectangle (preview,
		      mainw->style->white_gc,
		      (gint) TRUE, 0, 0, (gint) 200, (gint) 100);
  return (gint) TRUE;
}

static gint
expose_event (GtkWidget * mainw, GdkEventExpose * event)
/* callback function to redraw parts of the drawing area */
{

  gdk_draw_pixmap (mainw->window,
		   mainw->style->fg_gc[GTK_WIDGET_STATE (mainw)],
		   picture,
		   event->area.x, event->area.y,
		   event->area.x, event->area.y,
		   (gint) event->area.width, (gint) event->area.height);
  return (gint) FALSE;
}


void
yesnodialog (void (*YesFunc) (void))
/* Popup to show a warning message when unsaved changes are about to be
   destroyed by Load or Quit (Modal toplevel dialog) */
{
  GtkWidget *button, *label;
  GtkWidget *yesno_window;

  yesno_window = gtk_dialog_new ();
  (void)gtk_signal_connect_object (GTK_OBJECT (yesno_window), "destroy",
			     GTK_SIGNAL_FUNC (gtk_grab_remove),
			     GTK_OBJECT (yesno_window));
  (void)gtk_signal_connect_object (GTK_OBJECT (yesno_window), "destroy",
			     GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     GTK_OBJECT (yesno_window));
  gtk_container_border_width (GTK_CONTAINER (yesno_window), 5);

  label =
    gtk_label_new (_
		   ("The current drawing is not saved !\nDo you really wish to continue ?"));

  gtk_misc_set_padding (GTK_MISC (label), 10, 10);

  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (yesno_window)->vbox), label, TRUE,
		      (gboolean) TRUE, 0);
  gtk_widget_show (label);

  button = gtk_button_new_with_label (_("Yes"));

  (void)gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
			     GTK_SIGNAL_FUNC (gtk_grab_remove),
			     GTK_OBJECT (yesno_window));
  (void)gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
			     GTK_SIGNAL_FUNC (YesFunc),
			     GTK_OBJECT (yesno_window));
  (void)gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
			     GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     GTK_OBJECT (yesno_window));
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (yesno_window)->action_area),
		      button, (gboolean) TRUE, (gboolean) TRUE, 0);
  gtk_widget_show (button);

  button = gtk_button_new_with_label (_("No"));

  (void)gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
			     GTK_SIGNAL_FUNC (gtk_grab_remove),
			     GTK_OBJECT (yesno_window));
  (void)gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
			     GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     GTK_OBJECT (yesno_window));
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (yesno_window)->action_area),
		      button, (gboolean) TRUE, (gboolean) TRUE, 0);
  gtk_widget_show (button);

  gtk_widget_realize (yesno_window);
  gtk_grab_add (yesno_window);
#ifdef GTK2
  gtk_window_set_transient_for (GTK_WINDOW (yesno_window),
     	 			GTK_WINDOW(mainw));
#else
  gtk_window_set_transient_for (GTK_WINDOW (yesno_window),
				GTK_WINDOW (gtk_widget_get_toplevel
					    (yesno_window)));
#endif
  gtk_window_set_position (GTK_WINDOW (yesno_window), GTK_WIN_POS_MOUSE);
  gtk_widget_show (yesno_window);
}

void
yesnodialog2 (void (*YesFunc) (void), char *somefile)
/* Popup to show a warning message when unsaved changes are about to be
   destroyed by Load or Quit (Modal toplevel dialog) */
{
  GtkWidget *button, *label;
  GtkWidget *yesno_window;
  char tmpstr[512];

  yesno_window = gtk_dialog_new ();
  (void)gtk_signal_connect_object (GTK_OBJECT (yesno_window), "destroy",
			     GTK_SIGNAL_FUNC (gtk_grab_remove),
			     GTK_OBJECT (yesno_window));
  (void)gtk_signal_connect_object (GTK_OBJECT (yesno_window), "destroy",
			     GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     GTK_OBJECT (yesno_window));
  gtk_container_border_width (GTK_CONTAINER (yesno_window), 5);

  snprintf (tmpstr,512, _("File\n%s\nalready exists !\nOverwrite it ?"), somefile);
  label = gtk_label_new (tmpstr);

  gtk_misc_set_padding (GTK_MISC (label), 10, 10);

  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (yesno_window)->vbox), label, TRUE,
		      (gboolean) TRUE, 0);
  gtk_widget_show (label);

  button = gtk_button_new_with_label (_("Yes"));

  (void)gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
			     GTK_SIGNAL_FUNC (gtk_grab_remove),
			     GTK_OBJECT (yesno_window));
  (void)gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
			     GTK_SIGNAL_FUNC (YesFunc),
			     GTK_OBJECT (yesno_window));
  (void)gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
			     GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     GTK_OBJECT (yesno_window));
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (yesno_window)->action_area),
		      button, (gboolean) TRUE, (gboolean) TRUE, 0);
  gtk_widget_show (button);

  button = gtk_button_new_with_label (_("No"));

  (void)gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
			     GTK_SIGNAL_FUNC (gtk_grab_remove),
			     GTK_OBJECT (yesno_window));
  (void)gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
			     GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     GTK_OBJECT (yesno_window));
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (yesno_window)->action_area),
		      button, (gboolean) TRUE, (gboolean) TRUE, 0);
  gtk_widget_show (button);

  gtk_widget_realize (yesno_window);
  gtk_grab_add (yesno_window);
#ifdef GTK2  
  gtk_window_set_transient_for (GTK_WINDOW (yesno_window),
	 			GTK_WINDOW(mainw));
#else
  gtk_window_set_transient_for (GTK_WINDOW (yesno_window),
				GTK_WINDOW (gtk_widget_get_toplevel
					    (yesno_window)));
#endif
  gtk_window_set_position (GTK_WINDOW (yesno_window), GTK_WIN_POS_MOUSE);
  gtk_widget_show (yesno_window);
}

static gint
button_press_event (GtkWidget * mainw, GdkEventButton * event)
/* Handle mouse button presses depending on current drawing mode */
{
  int event_x, event_y;
  int i, spt;
  char *tmpstr;

  (void) mainw;
    
  draw_ok= 1; /* valid data, allow drawing */
  if (event->button == 1 && picture != NULL)	/* Button 1 actions: */
    {
      event_x = (int) event->x;
      event_y = (int) event->y;

      switch (drawmode)
	{
	case 1:
	  /* Text mode: Insert a label */
	  if (event->state & GDK_CONTROL_MASK)
	  Add_number (event_x, event_y);
	  else
	  Add_atom (event_x, event_y);
#ifdef LIBUNDO
	  undo_snapshot ();
#endif
	  break;
	case 0:
	  /* Line mode: show marker boxes around current position */
	default:
	  Set_vector (event_x, event_y, curbond);
	  break;
	case 2:
	  /* Bondtype mode: change bond at cursor to current default */
	  Set_bondtype (event_x, event_y, curbond);
#ifdef LIBUNDO
	  undo_snapshot ();
#endif
	  break;
	case 3:
	case 6:
	case 7:
	  /* Move and rotate modes : initialize operation */
	  Set_pmove (event_x, event_y);
	  break;
	case 4:
	  /* Mark mode : Set corner of rectangle */
	  gdk_window_set_cursor (drawing_area->window, cursor_markBRC);
	  Set_start_rec (event_x, event_y);
	  break;
	case 8:
	  spline[splinepoint].x = round_coord (event_x, size_factor);
	  spline[splinepoint].y = round_coord (event_y, size_factor);
	  splinepoint++;
	  modify = 1;
	  if (splinepoint < 4)
	    {
	      for (i = 1; i < splinepoint; i++)
		gdk_draw_line (picture, mygc[curpen],
			       (gint) (spline[i - 1].x * size_factor),
			       (gint) (spline[i - 1].y * size_factor),
			       (gint) (spline[i].x * size_factor),
			       (gint) (spline[i].y * size_factor));
	      CopyPlane ();
	    }
	  else
	    {
	      spt = 0;
	      if (curbond == 8)
		spt = 1;
	      if (curbond == 9)
		spt = 2;
	      if (curbond == 10)
		spt = -1;
	      if (curbond == 12)
		spt = -2;
	      add_spline (spline[0].x, spline[0].y, spline[1].x, spline[1].y,
			  spline[2].x, spline[2].y, spline[3].x, spline[3].y,
			  spt, 0, curpen);
	      splinepoint = 0;
	      Display_Mol ();
	    }
	  break;
	  ;;
	}
    }
  if (event->button == 2 && picture != NULL)
    /* Second (middle) button */
    {
      event_x = (int) event->x;
      event_y = (int) event->y;
      if (drawmode == 2)
	{
	  /* in bondtype mode, reverse direction of current bond */
	  Invert_vector (event_x, event_y);
#ifdef LIBUNDO
	  undo_snapshot ();
#endif
	}
      else if (drawmode == 1){
	  if (event->state & GDK_CONTROL_MASK){
	tmpstr=(char*)gtk_entry_get_text (GTK_ENTRY (textbox));
	atnum=atoi(tmpstr);
	if (atnum>0)atnum--;
	}
	  else
	Fetch_atom (event_x, event_y);
	}
      else
	{
	  /* in all other modes, set bondtype of current bond to next in list */
	  Add_double (event_x, event_y);
#ifdef LIBUNDO
	  undo_snapshot ();
#endif
	}
    }
  if (event->button == 3 && picture != NULL)
/* Button 3 */
    {
      event_x = (int) event->x;
      event_y = (int) event->y;
      switch (drawmode)
	{
	case 0:
	  /* in line mode, delete bond at cursor */
	  if (event->state & GDK_CONTROL_MASK)
	    Del_rec ();
	  else
	    Del_vector (event_x, event_y);
#ifdef LIBUNDO
	  undo_snapshot ();
#endif
	  break;
	case 1:
	  if (event->state & GDK_CONTROL_MASK)
	  atnum=0;
	  else
/* in text mode, deletes label at cursor */
	  Del_atom (event_x, event_y);
#ifdef LIBUNDO
	  undo_snapshot ();
#endif
	  break;
	case 4:
#ifdef LIBUNDO
	  undo_snapshot ();
#endif
	  Del_rec ();
	  break;
	case 3:
	case 6:
/* in move and rotate modes - abort operation */
#ifdef LIBUNDO
          do_undo();
#endif          
	  break;
	case 8:
	  /* in spline mode, abort current spline */
	  splinepoint = 0;
	  Display_Mol ();
	  break;
	default:
	  break;
	}
    }
  return (gint) TRUE;
}

static gint
button_release_event (GtkWidget * mainw, GdkEventButton * event)
/* completes drawing actions upon release of the mouse button */
{
  (void) mainw;
  
  draw_ok = 0; 
  if (event->button == 1 && picture != NULL)
/* release of the first (left) button */
    {
      if (drawmode == 0)
	{
	  /* in line mode, adds bond from previous to current position */
	  if (event->state & GDK_CONTROL_MASK)
	    {
	      Add_ring (draw_angle, curbond, ringtype);
#ifdef GTK2
              gtk_text_buffer_insert (msgtextbuffer,&iter, "\n", -1);
              gtk_adjustment_set_value(msgadjustment,gtk_adjustment_get_value(msgadjustment)+12.);
#else              
	      gtk_text_insert (GTK_TEXT (msgtext), NULL, NULL, NULL, "\n", 1);
#endif
	      ringtype = 0;
	    }
	  else
	    Add_vector (curbond);	/* add bond, redisplay */

#ifdef LIBUNDO
	  undo_snapshot ();
#endif
	}
      else
	{
	  if (drawmode == 4)
	    {
	      /* in mark mode, defines final position for opposite corner of rectangle */
	      if (event->state & GDK_CONTROL_MASK)
		{
		  Mark_rec (1);
		}
	      else
		{
		  Mark_rec (0);
		}
	      gdk_window_set_cursor (drawing_area->window, cursor_markTLC);
	      FreePix ();
	      CreatePix ();
	      Display_Mol ();
#if 0
		Movemode(mainw);
#endif
	    }
#ifdef LIBUNDO
	  if (drawmode > 2) 
	    undo_snapshot ();
#endif
	}
    }
  return (gint) TRUE;
}


static gint
motion_notify_event (GtkWidget * widget, GdkEventMotion * event)
/* handles mouse movement while a button is down */
{
  (void) widget;
  gint x, y;
  GdkModifierType state;

  if (event->is_hint != 0)
    (void) gdk_window_get_pointer (event->window, &x, &y, &state);
  else
    {
      x = (gint) event->x;
      y = (gint) event->y;
      state = event->state;
    }

  if (( (state & GDK_BUTTON1_MASK)|| pendown==1) && picture != NULL)
    {
      /* dragging with button 1 (usually left) down */
      switch (drawmode)
	{
	case 0:
	  /* in line mode, draw bond from last to current cursor position */
	  if (draw_ok) Put_vector (x, y);
	  break;

	case 3:
	  /* in move mode, shift marked fragment to current cursor position */
	  if (event->state & GDK_CONTROL_MASK)
	    {
	      Put_pmove (x, y, 1);
	    }
	  else if (event->state & GDK_SHIFT_MASK)
	    { 
	      Put_pmove (x, y, 2);
	    }
	  else    
	    {
	      Put_pmove (x, y, 0);
	    }
	  break;
	case 4:
	  /* in mark mode, draw rectangle between initial and current position */
	  if (draw_ok) Put_rec (x, y);
	  break;
	case 6:
	  /* in rotate mode, rotate marked fragment by an amount proportional to
	     the distance between initial and current cursor position */
	  if (event->state & GDK_CONTROL_MASK)
	    {
	      Put_protate (x, y, 1);
	    }
	  else
	    {
	      Put_protate (x, y, 0);
	    }
	  break;
	case 7:
	  if (state & GDK_CONTROL_MASK)
	    Put_pscale (x, y, 1);
	  else
	    Put_pscale (x, y, 0);
	  break;
	default:
	  break;
	}
    }
  return (gint) TRUE;
}


static gint
key_press_event (GtkWidget * mainw, GdkEventKey * event)
/* Handle key presses depending on current drawing mode */
{
  struct dc *hpc;
  static char label[MAXCL];
  int direction=0;
  int dx=0;
  char errtext[101];
int x,y;

#ifdef GTK2
  if (gtk_widget_is_focus(textbox)) return FALSE;
#endif

  if (event->state & GDK_CONTROL_MASK)
    {
      if (!isdigit ((int) event->keyval))
	return ((gint) FALSE);
      ringtype = (int) (event->keyval - 48);
      if (ringtype < 3)
	ringtype += 10;
      snprintf (errtext, 100,
		_("\nNext ring drawn will have %d sides"), ringtype);
#ifdef GTK2
      gtk_text_buffer_insert (msgtextbuffer, &iter, errtext, -1);
      gtk_adjustment_set_value(msgadjustment, gtk_adjustment_get_value(msgadjustment)+12.);
#else
      gtk_text_insert (GTK_TEXT (msgtext), NULL, NULL, NULL, errtext,
		       (gint) strlen (errtext));
#endif
      return ((gint) TRUE);
    }
  if (event->state & GDK_MOD1_MASK) /* ALT+Cursor shifts grid */
  	{
  	if (gridtype == 0) return TRUE; /* nothing to do */
	switch (event->keyval){
	case GDK_Left:
	gridx--;
	if (gridx<-50) gridx=0;
	 gtk_signal_emit_stop_by_name (GTK_OBJECT(mainw), 
	                                             "key_press_event");
	 break; 
	case GDK_Right:
	gridx++;
	if (gridx>50) gridx=0;
	 gtk_signal_emit_stop_by_name (GTK_OBJECT(mainw), 
	                                             "key_press_event");
	 break; 
	case GDK_Up:
	gridy--;
	if (gridy<-50) gridy=0;
	 gtk_signal_emit_stop_by_name (GTK_OBJECT(mainw), 
	                                             "key_press_event");
	 break; 
	case GDK_Down:
	gridy++;
	if (gridy>50) gridy=0;
	 gtk_signal_emit_stop_by_name (GTK_OBJECT(mainw), 
	                                             "key_press_event");
	 break; 
	default:
	  return (gint) FALSE;
	}
      Display_Mol ();
	return TRUE;
    }	
  if (drawmode == 0 && picture != NULL)	/* Button 1 actions: */
    {
	if (textentry >-1){ /* label input in progress */
	if (event->keyval == GDK_Return) {
	  label[textentry] = '\0';
          add_char (hp->tx, hp->ty, label, direction, 0, curpen,0,curfontsize);
          textentry=-1;
	  Display_Mol ();
	return(TRUE);
          } else  if (isprint((int)event->keyval)) {
		label[textentry++]=(char)event->keyval;
		gtk_entry_set_text(GTK_ENTRY(textbox),label);
	return(TRUE);
	 }
	 if (event->keyval == GDK_BackSpace) {
		label[--textentry]='\0';
		gtk_entry_set_text(GTK_ENTRY(textbox),label);
		}
	}				
      switch (event->keyval)
	{
	case 'c':
	case 'o':
	case 'n':
	case 's':
	case 'p':
	case 'h':
	case 'f':
	case 'i':
	case 'r':
	case 'b':
	case 'd':
	  direction = Middle_Text;
	  label[0] = toupper ((int) event->keyval);
	  label[1] = '\0';
          add_char (hp->tx, hp->ty, label, direction, 0,curpen,0,curfontsize);
	  break;
	case 'l':
	  direction = Left_Text;
	  strcpy (label, "Cl");
          add_char (hp->tx, hp->ty, label, direction, 0,curpen,0,curfontsize);
	  break;
	case '1':
	  direction = Left_Text;
 	  strcpy (label, "CH");
          add_char (hp->tx, hp->ty, label, direction, 0, curpen,0,curfontsize);
	  break;
	case '2':
	  direction = Left_Text;
	  strcpy (label, "CH_2");
          add_char (hp->tx, hp->ty, label, direction, 0, curpen,0,curfontsize);
	  break;
	case '3':
	  direction = Left_Text;
	  strcpy (label, "CH_3");
          add_char (hp->tx, hp->ty, label, direction, 0, curpen,0,curfontsize);
	  break;
	case '*':
	  direction = Middle_Text;
	  label[0] = '@';
	  label[1] = '\267';
	  label[2] = '\0';
          add_char (hp->tx, hp->ty, label, direction, 0, curpen,0,curfontsize);
	  break;
	case '+':
	  direction = Middle_Text;
	  label[0] = '@';
	  label[1] = '+';
	  label[2] = '\0';
          add_char (hp->tx, hp->ty, label, direction, 0, curpen,0,curfontsize);
	  break;
	case '-':
	  direction = Middle_Text;
	  label[0] = '@';
	  label[1] = '-';
	  label[2] = '\0';
          add_char (hp->tx, hp->ty, label, direction, 0, curpen,0,curfontsize);
	  break;
	case ' ':
		direction=Left_Text;
/*		fprintf(stderr,"start label entering\n");*/
		memset(label,0,25);
		textentry=0;
	break;
	
	case GDK_KP_Left:
	case GDK_KP_4:
	  hpc=select_char(hp->tx,hp->ty,1);
	  if (hpc ) { 
	  dx =3*hpc->direct *(int)strlen(hpc->c);
	  if (event->state & GDK_SHIFT_MASK) {/* dots */
	  add_char (hp->tx-15+dx,hp->ty-4,"@\267",Middle_Text,0,curpen,0,curfontsize);
	  add_char (hp->tx-15+dx,hp->ty+4,"@\267",Middle_Text,0,curpen,0,curfontsize);
	  }else{
	  add_struct(hp->tx-15+dx,hp->ty-12,hp->tx-15+dx,hp->ty+8,0,0,0,1,curpen);
	  }
	  }
	  break;
	case GDK_KP_Up:
	case GDK_KP_8:
	  hpc=select_char(hp->tx,hp->ty,1);
	  if (hpc ) { 
	  dx =3*hpc->direct *(int)strlen(hpc->c);
	  if (event->state & GDK_SHIFT_MASK) {/* dots */
	  add_char (hp->tx-4+dx,hp->ty-20,"@\267",Middle_Text,0,curpen,0,curfontsize);
	  add_char (hp->tx+8+dx,hp->ty-20,"@\267",Middle_Text,0,curpen,0,curfontsize);
	  }else{
	  add_struct(hp->tx-10+dx,hp->ty-20,hp->tx+10+dx,hp->ty-20,0,0,0,1,curpen);
	  }
	  }
	  break;
	case GDK_KP_Right:
	case GDK_KP_6:
	  hpc=select_char(hp->tx,hp->ty,1);
	  if (hpc ) { 
	  dx =5 *((int)strlen(hpc->c)-1);
	  if (event->state & GDK_SHIFT_MASK) {/* dots */
	  add_char (hp->tx+13+dx,hp->ty-4,"@\267",Middle_Text,0,curpen,0,curfontsize);
	  add_char (hp->tx+13+dx,hp->ty+4,"@\267",Middle_Text,0,curpen,0,curfontsize);
	  }else{
	  add_struct(hp->tx+15+dx,hp->ty-12,hp->tx+15+dx,hp->ty+8,0,0,0,1,curpen);
	}
	}
          break;
	case GDK_KP_Down:
	case GDK_KP_2:
	  hpc=select_char(hp->tx,hp->ty,1);
	  if (hpc ) { 
	  dx =3*hpc->direct *(int)strlen(hpc->c);
	  if (event->state & GDK_SHIFT_MASK) {/* dots */
	  add_char (hp->tx-4+dx,hp->ty+15,"@\267",Middle_Text,0,curpen,0,curfontsize);
	  add_char (hp->tx+8+dx,hp->ty+15,"@\267",Middle_Text,0,curpen,0,curfontsize);
	  }else{
	  add_struct(hp->tx-10+dx,hp->ty+15,hp->tx+10+dx,hp->ty+15,0,0,0,1,curpen);
	}
	}
	  break;
	case GDK_KP_Home:
	case GDK_KP_7:
	  hpc=select_char(hp->tx,hp->ty,1);
	  if (hpc ) {
	  dx =3*hpc->direct *(int)strlen(hpc->c);
	  if (event->state & GDK_SHIFT_MASK) {/* dots */
	  add_char (hp->tx-20+dx,hp->ty-10,"@\267",Middle_Text,0,curpen,0,curfontsize);
	  add_char (hp->tx-5+dx,hp->ty-20,"@\267",Middle_Text,0,curpen,0,curfontsize);
	  }else{
	  add_struct(hp->tx-20+dx,hp->ty-10,hp->tx-5+dx,hp->ty-20,0,0,0,1,curpen);
	}
	}
	  break;
	case GDK_KP_Page_Up:
	case GDK_KP_9:
	  hpc=select_char(hp->tx,hp->ty,1);
	  if (hpc) { 
	  dx =5 *(int)strlen(hpc->c);
	  if (event->state & GDK_SHIFT_MASK) {/* dots */
	  add_char (hp->tx+20+dx,hp->ty-10,"@\267",Middle_Text,0,curpen,0,curfontsize);
	  add_char (hp->tx+5+dx,hp->ty-20,"@\267",Middle_Text,0,curpen,0,curfontsize);
	  }else{
	  add_struct(hp->tx+20+dx,hp->ty-10,hp->tx+5+dx,hp->ty-20,0,0,0,1,curpen);
	}
	}
	    break;
	case GDK_KP_End:
	case GDK_KP_1:
	  hpc=select_char(hp->tx,hp->ty,1);
	  if (hpc ) {
	  dx =3*hpc->direct *(int)strlen(hpc->c);
	  if (event->state & GDK_SHIFT_MASK) {/* dots */
	  add_char (hp->tx-20+dx,hp->ty+5,"@\267",Middle_Text,0,curpen,0,curfontsize);
	  add_char (hp->tx-5+dx,hp->ty+15,"@\267",Middle_Text,0,curpen,0,curfontsize);
	  }else{
	  add_struct(hp->tx-20+dx,hp->ty+5,hp->tx-5+dx,hp->ty+15,0,0,0,1,curpen);
	}
	}
	  break;
	case GDK_KP_Page_Down:
	case GDK_KP_3:
	  hpc=select_char(hp->tx,hp->ty,1);
	  if (hpc ) {
	  dx =5 *((int)strlen(hpc->c)-1);
	  if (event->state & GDK_SHIFT_MASK) {/* dots */
	  add_char (hp->tx+20+dx,hp->ty+5,"@\267",Middle_Text,0,curpen,0,curfontsize);
	  add_char (hp->tx+5+dx,hp->ty+15,"@\267",Middle_Text,0,curpen,0,curfontsize);
	  }else{
	  add_struct(hp->tx+20+dx,hp->ty+5,hp->tx+5+dx,hp->ty+15,0,0,0,1,curpen);
	}
	}
	  break;
	case GDK_Left:
    if (event->state & GDK_SHIFT_MASK) {
      pendown = 1;
      draw_ok = 1;
      XWarpPointer (GDK_DISPLAY (), None, None, 0, 0, 0, 0, -1, 0);
      (void) gdk_window_get_pointer (drawing_area->window, &x, &y, NULL);
      Put_vector (x, y);
    } else {
      pendown = 0;
      draw_ok = 0;
      XWarpPointer (GDK_DISPLAY (), None, None, 0, 0, 0, 0, -1, 0);
    }
    gtk_signal_emit_stop_by_name (GTK_OBJECT (mainw),
                                 "key_press_event");
	 break; 
	case GDK_Right:
    if (event->state & GDK_SHIFT_MASK) {
      pendown = 1;
      draw_ok = 1;
      XWarpPointer (GDK_DISPLAY (), None, None, 0, 0, 0, 0, 1, 0);
      (void) gdk_window_get_pointer (drawing_area->window, &x, &y, NULL);
      Put_vector (x, y);
    } else {
      pendown = 0;
      draw_ok = 0;
      XWarpPointer (GDK_DISPLAY (), None, None, 0, 0, 0, 0, 1, 0);
    }
    gtk_signal_emit_stop_by_name (GTK_OBJECT (mainw),
                                 "key_press_event");
	 break; 
	case GDK_Up:
    if (event->state & GDK_SHIFT_MASK) {
      pendown = 1;
      draw_ok = 1;
      XWarpPointer (GDK_DISPLAY (), None, None, 0, 0, 0, 0, 0, -1);
      (void) gdk_window_get_pointer (drawing_area->window, &x, &y, NULL);
      Put_vector (x, y);
    } else {
      pendown = 0;
      draw_ok = 0;
      XWarpPointer (GDK_DISPLAY (), None, None, 0, 0, 0, 0, 0, -1);
    }
    gtk_signal_emit_stop_by_name (GTK_OBJECT (mainw),
                                 "key_press_event");
	 break; 
	case GDK_Down:
    if (event->state & GDK_SHIFT_MASK) {
      pendown = 1;
      draw_ok = 1;
      (void) gdk_window_get_pointer (drawing_area->window, &x, &y, NULL);
      Put_vector (x, y);
      XWarpPointer (GDK_DISPLAY (), None, None, 0, 0, 0, 0, 0, 1);
    } else {
      pendown = 0;
      draw_ok = 0;
      XWarpPointer (GDK_DISPLAY (), None, None, 0, 0, 0, 0, 0, 1);
    }
    gtk_signal_emit_stop_by_name (GTK_OBJECT (mainw),
                                 "key_press_event");
	 break; 
	case GDK_Return:
		(void)gdk_window_get_pointer(drawing_area->window,&x,&y,NULL);
		if (pendown==0){
		Set_vector(x,y,curbond);
		}else{
		draw_ok=1;
		Add_vector(curbond);
		pendown=0;
		Set_vector(x,y,curbond);
		}
		break;
	default:
	  return (gint) FALSE;
	}

      Display_Mol ();

#ifdef LIBUNDO
      undo_snapshot ();
#endif
    }
  else if (drawmode == 3 && picture != NULL) 
    {  
      switch (event->keyval){
	case GDK_Left:
    if (event->state & GDK_SHIFT_MASK) {
      XWarpPointer (GDK_DISPLAY (), None, None, 0, 0, 0, 0, -5, 0);
    } else {
      XWarpPointer (GDK_DISPLAY (), None, None, 0, 0, 0, 0, -1, 0);
    }
    gtk_signal_emit_stop_by_name (GTK_OBJECT (mainw),
                                 "key_press_event");
    (void) gdk_window_get_pointer (drawing_area->window, &x, &y, NULL);
    Put_pmove (x, y, 0);
	 break; 
	case GDK_Right:
    if (event->state & GDK_SHIFT_MASK) {
      XWarpPointer (GDK_DISPLAY (), None, None, 0, 0, 0, 0, 5, 0);
    } else {
      XWarpPointer (GDK_DISPLAY (), None, None, 0, 0, 0, 0, 1, 0);
    }
    gtk_signal_emit_stop_by_name (GTK_OBJECT (mainw),
                                 "key_press_event");
    (void) gdk_window_get_pointer (drawing_area->window, &x, &y, NULL);
    Put_pmove (x, y, 0);
	 break; 
	case GDK_Up:
    if (event->state & GDK_SHIFT_MASK) {
      XWarpPointer (GDK_DISPLAY (), None, None, 0, 0, 0, 0, 0, -5);
    } else {
      XWarpPointer (GDK_DISPLAY (), None, None, 0, 0, 0, 0, 0, -1);
    }
    gtk_signal_emit_stop_by_name (GTK_OBJECT (mainw),
                                 "key_press_event");
    (void) gdk_window_get_pointer (drawing_area->window, &x, &y, NULL);
    Put_pmove (x, y, 0);
	 break; 
	case GDK_Down:
    if (event->state & GDK_SHIFT_MASK) {
      XWarpPointer (GDK_DISPLAY (), None, None, 0, 0, 0, 0, 0, 5);
    } else {
      XWarpPointer (GDK_DISPLAY (), None, None, 0, 0, 0, 0, 0, 1);
    }
    gtk_signal_emit_stop_by_name (GTK_OBJECT (mainw),
                                 "key_press_event");
    (void) gdk_window_get_pointer (drawing_area->window, &x, &y, NULL);
    Put_pmove (x, y, 0);
	 break; 
	case GDK_Return:
		Unmark_all();
	break;
	default:
	  return (gint) FALSE;
	}

      Display_Mol ();
      }
  else if (importflag != 0 && event->keyval == GDK_Return)
    {
      pdbstore ();
      gtk_toggle_button_set_active ((GtkToggleButton *) hexbutton, TRUE);
      snprintf (errtext, 100,
		_("\nImported %d bonds and %d labels"), hp->n, hp->nc);
#ifdef GTK2
      gtk_text_buffer_insert (msgtextbuffer, &iter, errtext, -1);
      gtk_adjustment_set_value (msgadjustment, gtk_adjustment_get_value(msgadjustment)+12.);
#else
      gtk_text_insert (GTK_TEXT (msgtext), NULL, NULL, NULL, errtext,
		       (gint) strlen (errtext));
#endif
    }


  return (gint) TRUE;
}

#ifdef LIBUNDO
void
do_undo ()
{
  int d;
  struct data *hp_b;
  struct dc *hp_c;
  struct spline *hp_sp;
  if (undo_get_undo_count () < 1)
    return;

  undo_undo ();

  if (hp->n > 0)
    {
      hp_b = da_root.next;
      if (hp_b->prev != &da_root)
	{
	  for (d = 0; d < hp->n; d++)
	    {
	      hp_b = hp_b->prev;
	      if (hp_b->prev == &da_root)
		{
		  da_root.next = hp_b;
		  break;
		}
	      da_root.next = hp_b;
	    }
	}
      for (d = 0; d < hp->n; d++)
	hp_b = hp_b->next;
      new = hp_b;
    }
  else
    {
      new = da_root.next;
      new->prev = &da_root;
    }

  if (hp->nc > 0)
    {
      hp_c = dac_root.next;
      if (hp_c->prev != &dac_root)
	{
	  for (d = 0; d < hp->nc; d++)
	    {
	      hp_c = hp_c->prev;
	      if (hp_c->prev == &dac_root)
		{
		  dac_root.next = hp_c;
		  break;
		}
	      dac_root.next = hp_c;
	    }
	}
      for (d = 0; d < hp->nc; d++)
	hp_c = hp_c->next;
      new_c = hp_c;
    }
  else
    {

  new_c = dac_root.next;
  new_c->prev = &dac_root;
    }

  if (hp->nsp > 0)
    {
      hp_sp = sp_root.next;
      if (hp_sp->prev != &sp_root)
	{
	  for (d = 0; d < hp->nsp; d++)
	    {
	      hp_sp = hp_sp->prev;
	      if (hp_sp->prev == &sp_root)
		{
		  sp_root.next = hp_sp;
		  break;
		}
	      sp_root.next = hp_sp;
	    }
	}
      for (d = 0; d < hp->nsp; d++)
	hp_sp = hp_sp->next;
      sp_new = hp_sp;
    }
  else
    {
      sp_new = sp_root.next;
      sp_new->prev = &sp_root;
    }
  FreePix ();
  CreatePix ();
  Display_Mol ();
}

void
do_redo ()
{
  int d;
  struct data *hp_b;
  struct dc *hp_c;
  struct spline *hp_sp;

  if (undo_get_redo_count () > 0)
    undo_redo ();

  if (hp->n > 0)
    {
      hp_b = da_root.next;
      if (hp_b->prev != &da_root)
	{
	  for (d = 0; d < hp->n; d++)
	    {
	      hp_b = hp_b->prev;
	      if (hp_b->prev == &da_root)
		{
		  da_root.next = hp_b;
		  break;
		}
	      da_root.next = hp_b;
	    }
	}
      for (d = 0; d < hp->n; d++)
	hp_b = hp_b->next;
      new = hp_b;
    }
  else
    {
      new = da_root.next;
      new->prev = &da_root;
    }

  if (hp->nc > 0)
    {
      hp_c = dac_root.next;
      if (hp_c->prev != &dac_root)
	{
	  for (d = 0; d < hp->nc; d++)
	    {
	      hp_c = hp_c->prev;
	      if (hp_c->prev == &dac_root)
		{
		  dac_root.next = hp_c;
		  break;
		}
	      dac_root.next = hp_c;
	    }
	}
      for (d = 0; d < hp->nc; d++)
	hp_c = hp_c->next;
      new_c = hp_c;
    }
  else
    {
      new_c = dac_root.next;
      new_c->prev = &dac_root;
    }

  if (hp->nsp > 0)
    {
      hp_sp = sp_root.next;
      if (hp_sp->prev != &sp_root)
	{
	  for (d = 0; d < hp->nsp; d++)
	    {
	      hp_sp = hp_sp->prev;
	      if (hp_sp->prev == &sp_root)
		{
		  sp_root.next = hp_sp;
		  break;
		}
	      sp_root.next = hp_sp;
	    }
	}
      for (d = 0; d < hp->nsp; d++)
	hp_sp = hp_sp->next;
      sp_new = hp_sp;
    }
  else
    {
      sp_new = sp_root.next;
      sp_new->prev = &sp_root;
    }

  FreePix ();
  CreatePix ();
  Display_Mol ();
}
#endif

void
Zoom (GtkWidget * mainw, gpointer inout)
/* increase or decrease zoom scale (two steps to either side ) */
{
  (void) mainw;
  if (inout && !strcmp ((char *) inout , "1"))
    {
      if (zoom_factor != 0)
	zoom_factor--;
    }
  else if (!strcmp( (char*)inout ,"0"))
    {
      if (zoom_factor < 4)
	zoom_factor++;
    }
  if (batchmode==1) return;  
  gtk_option_menu_set_history(GTK_OPTION_MENU(fontmenu), (guint)zoom_factor+1);
  curfontsize=zoom_factor+1;
  FreePix ();
  CreatePix ();
  Display_Mol ();

  if (importflag != 0)
    pdbrotate (0, 0, 2);
}
void
Grid (GtkWidget * mainw)
/* show or hide grid */
{
  (void) mainw;
      if (gridtype < 2)
	gridtype ++;
	  else
	gridtype = 0;
  FreePix ();
  CreatePix ();
  Display_Mol ();

  if (importflag != 0)
    pdbrotate (0, 0, 2);
}

void
change_color (GtkWidget * mainw, gpointer data)
/* update current pencolor and color selection button */
{
  GdkPixmap *pixmap;
  GtkWidget *pixmapwid;
  GtkStyle *style=gtk_widget_get_style (mainw);;

  curpen=atoi(data);
  pixmap = gdk_pixmap_create_from_xpm_d (mainw->window, &mask,
					 &style->bg[GTK_STATE_NORMAL],
					 (gchar **) xpm_color[curpen]);
  pixmapwid = gtk_pixmap_new (pixmap, mask);
  gtk_widget_show (pixmapwid);
  gdk_pixmap_unref (pixmap);
  gtk_container_remove (GTK_CONTAINER (colorbutton),oldpixmap);
  gtk_container_add (GTK_CONTAINER (colorbutton), pixmapwid);
  oldpixmap=pixmapwid;
}

void
CheckAndClear ()
/* called by Clear button, pops up an "are you sure" dialog if the current
   drawing changed since last save, or calls real Clear function directly */
{
  if (modify == 0 || hp->n + hp->nc + hp->nsp == 0)
    {
      Clear ();
    }
  else
    {
      yesnodialog (Clear);
    }
}
void
Clear ()
/* deletes all atoms and bonds from memory and reinitializes drawing area */
{
  char msgtmp[100];
  clear_data ();
  modify = 0;
  atnum = 0;
  mark.x = 300;
  mark.y = 300;
  hp->n = 0;
  hp->nc = 0;
  hp->nsp = 0;
  hp->x = 200;
  hp->y = 200;
  hp->tx = 200;
  hp->ty = 200;
  if (pdbn)
    {
      free (pdbx);
      free (pdby);
      free (pdbz);
      free (bondfrom);
      free (bondto);
      free (bondtype);
      bondfrom = NULL;
      bondto = NULL;
      bondtype = NULL;
      pdbx = NULL;
      pdby = NULL;
      pdbz = NULL;
      free (atjust);
      atjust = NULL;
      free (atcode);
      atcode = NULL;
      pdbn = 0;
    }
  importflag = 0;
  addflag = 0;
  refx=refy=0;
#ifdef LIBUNDO
  undo_snapshot ();
  new = da_root.next;
  new->prev = &da_root;
#endif
  strcpy (filename, _("unnamed"));
  gtk_window_set_title (GTK_WINDOW (window), "Chemtool 1.6.14");
  snprintf(msgtmp,99,_("\nReady"));
#ifdef GTK2
  gtk_text_buffer_insert (msgtextbuffer, &iter, msgtmp, -1);
  gtk_adjustment_set_value (msgadjustment, gtk_adjustment_get_value (msgadjustment)+12.);  
#else
  gtk_text_insert (GTK_TEXT (msgtext), NULL, NULL, NULL, msgtmp,
		   (gint)strlen(msgtmp));
#endif
  FreePix ();
  CreatePix ();
  Display_Mol ();
}

void
do_fw ()
{
  int error;
  char errtext[1024];
  size_t msglen = 254;

  strcpy (formula, "program cht not available or unable to write to");
  strcpy (weight, filename);
  strcat (weight, ".rad");
  strcpy (eweight, " ?");
  strcpy (compos, " ?");
  error = export_fw (filename);
  if (error != 0)
    {
      snprintf (errtext, msglen, _("\nHelper process failed - %s %s %s %s!"),
		formula, weight, eweight, compos);
    }
  else
    {
      snprintf (errtext, msglen, _("\n%s    %s  %s  %s"), formula, weight,
		eweight, compos);
    }
#ifdef GTK2
  gtk_text_buffer_insert(msgtextbuffer, &iter, errtext, -1);
  gtk_adjustment_set_value(msgadjustment,gtk_adjustment_get_value(msgadjustment)+12.); 
#else     
  gtk_text_insert (GTK_TEXT (msgtext), NULL, NULL, NULL, errtext,
		   (gint) strlen (errtext));
#endif
}

void
print_ps ()
{
  char errtext[255];
  size_t msglen = 254;

  if (hp->n + hp->nc + hp->nsp == 0)
    return;
  if (print_ps_pic() == 0)
    {
	snprintf(errtext,msglen,_("\nDrawing printed!"));
    }
  else
    {
	snprintf(errtext,msglen,_("\nFailed to print drawing !"));
    }
#ifdef GTK2
     gtk_text_buffer_insert (msgtextbuffer, &iter, errtext, -1);
     gtk_adjustment_set_value (msgadjustment, gtk_adjustment_get_value(msgadjustment)+12.);
#else     
     gtk_text_insert (GTK_TEXT (msgtext), NULL, NULL, NULL,
		       errtext,(gint)strlen(errtext));
#endif
}

#if (GTK_MINOR_VERSION >2)
/* ************** Callback for Configurable Options Dialog ************** */

int options_dialog_ok (GtkWidget *widget, gpointer data) {
  (void) widget;
  (void) data;
    int newint;
    GdkColor color;

    papersize = gtk_combo_box_get_active (GTK_COMBO_BOX (paper_size_combo));
    orient = gtk_combo_box_get_active (GTK_COMBO_BOX (orientation_combo));
    printcmd = gtk_combo_box_get_active (GTK_COMBO_BOX (print_cmd_combo));
    epsoption = gtk_combo_box_get_active (GTK_COMBO_BOX (eps_preview_combo));
    use_whiteout = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (whiteout_check));
    use_intlchars = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (intl_check));
    strcpy (queuename, gtk_entry_get_text (GTK_ENTRY (printer_name_entry)));
    printscale =
        gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (print_scale_spin)) / 100.0;
    strcpy (datadir, gtk_entry_get_text (GTK_ENTRY (datadir_entry)));
    strcpy (datamask, gtk_entry_get_text (GTK_ENTRY (datamask_entry)));
    
    newint =  atof(gtk_entry_get_text (GTK_ENTRY (bond_length_entry)));
    if (newint != 0 && newint != bondlen_mm) {
        size_factor /= bondlen_mm/10.668;
        bondlen_mm = newint;
        size_factor *= bondlen_mm/10.668;
    }
    newint = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (dbond_dist_spin));
    if (newint >= 0 && db_dist != newint) {
        db_dist = newint;
        mb_dist = (int) ( ((float)db_dist) * 2.5);
    }
    
    gtk_color_button_get_color (GTK_COLOR_BUTTON (color_button), &color);
    gdk_colormap_alloc_color(gdk_colormap_get_system(), &color, FALSE, TRUE);
    gdk_gc_set_foreground(background_gc, &color);
    gdk_gc_set_background(background_gc, &color);
  snprintf(bghexcolor,10,"#%2.2x%2.2x%2.2x",(unsigned char)(color.red/256.),(unsigned char)(color.green/256.),(unsigned char)(color.blue/256.));

    FreePix();
    CreatePix();
    Display_Mol();
    
    gtk_widget_hide (printer_dialog);
    return TRUE;
}

void prepare_options_dialog () {
    char msgtmp[100];

    gtk_color_button_set_color (GTK_COLOR_BUTTON (color_button), &background);
    if (use_whiteout == 1) {
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (whiteout_check), TRUE);
    }
    else {
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (whiteout_check), FALSE);
    }
    if (use_intlchars == 1) {
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (intl_check), TRUE);
    }
    else {
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (intl_check), FALSE);
    }

    gtk_entry_set_text (GTK_ENTRY (datadir_entry), datadir);
    gtk_entry_set_text (GTK_ENTRY (datamask_entry), datamask);
    gtk_combo_box_set_active (GTK_COMBO_BOX (eps_preview_combo), epsoption);
    
    
    snprintf (msgtmp, 100, "%6.4f", bondlen_mm);
    gtk_entry_set_text (GTK_ENTRY (bond_length_entry), msgtmp);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (dbond_dist_spin), db_dist);

    gtk_combo_box_set_active (GTK_COMBO_BOX (print_cmd_combo), printcmd);
    gtk_entry_set_text (GTK_ENTRY (printer_name_entry), queuename);
    gtk_combo_box_set_active (GTK_COMBO_BOX (paper_size_combo), papersize);
    gtk_combo_box_set_active (GTK_COMBO_BOX (orientation_combo), orient);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (print_scale_spin),
        printscale * 100);
}

int print_setup_menu_activate (GtkWidget *widget, gpointer data) {
  (void) widget;
  (void) data;
    prepare_options_dialog ();
    gtk_widget_show (printer_dialog);
    return TRUE;
}

#else
void
setup_printer (GtkWidget * mainw, gpointer change)
{
char tmpstr[10];
double bondlen_new;
int doubledist_new;

  (void) mainw;

  if (atoi (change) == 1)
    {
      papersize = newpapersize;
      orient = neworientation;
      printcmd = newprintcommand;
      epsoption = newepsoption;
      oldwhiteout = use_whiteout;
      oldintlchars = use_intlchars;
      printscale =
	gtk_spin_button_get_value_as_float ((GtkSpinButton *) pscale) / 100.;
      queuename = (char*)gtk_entry_get_text (GTK_ENTRY (prqueue));
      strcpy (datadir, gtk_entry_get_text (GTK_ENTRY (defaultdir)));
      strcpy (datamask, gtk_entry_get_text (GTK_ENTRY (defaultext)));
	bondlen_new=atof(gtk_entry_get_text (GTK_ENTRY(base_bondlen)));
 if (bondlen_new != 0 && bondlen_mm != bondlen_new){
 	 size_factor /= bondlen_mm/10.668;
 	 bondlen_mm = bondlen_new;
 	 size_factor *= bondlen_mm/10.668;
 	}
	doubledist_new=atoi(gtk_entry_get_text (GTK_ENTRY(doubledist)));
 if (doubledist_new >= 0 && db_dist != doubledist_new){
	db_dist = doubledist_new;
	mb_dist = (int) ( ((float)db_dist) * 2.5);
 	}
    }
  else
    {
      gtk_option_menu_set_history (GTK_OPTION_MENU (papermenu), (guint)papersize);
      gtk_option_menu_set_history (GTK_OPTION_MENU (orientmenu), (guint)orient);
      gtk_spin_button_set_value ((GtkSpinButton *) pscale, printscale * 100.);
      gtk_option_menu_set_history (GTK_OPTION_MENU (epsoptionmenu), (guint)epsoption);
      gtk_option_menu_set_history (GTK_OPTION_MENU (printcmdmenu), (guint)printcmd);
      gtk_entry_set_text (GTK_ENTRY (prqueue), queuename);
      gtk_entry_set_text (GTK_ENTRY (defaultdir), datadir);
      gtk_entry_set_text (GTK_ENTRY (defaultext), datamask);
      snprintf(tmpstr,10,"%6.4f",bondlen_mm);
      gtk_entry_set_text (GTK_ENTRY (base_bondlen),tmpstr);
      snprintf(tmpstr,10,"%d",db_dist);
      gtk_entry_set_text (GTK_ENTRY (doubledist),tmpstr);
      if (oldwhiteout != use_whiteout) {
      			/* toggle button triggers immediately, and resetting 
      			   the button state fires another event, which we use
      			   here to actually reset the value */
		if (use_whiteout == 1) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(whiteoutbutton),FALSE);
			else
				       gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(whiteoutbutton),TRUE);
      }				       
      if (oldintlchars != use_intlchars) {
      			/* toggle button triggers immediately, and resetting 
      			   the button state fires another event, which we use
      			   here to actually reset the value */
		if (use_intlchars == 1) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(intlbutton),FALSE);
			else
				       gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(intlbutton),TRUE);
      }				       
    }
}

int
newpaper (GtkWidget * mainw, gpointer newpaper)
{
  int i;

  (void) mainw;

  for (i = 0; i < 11; i++)
    {
      if (!strcmp (newpaper, paper[i]))
	{
	  newpapersize = i;
	  return TRUE;
	}
    }
  return TRUE;
}

int
neworient (GtkWidget * mainw, gpointer newo)
{
  (void) mainw;
  neworientation = atoi (newo);
  return TRUE;
}

int
newprcmd (GtkWidget * mainw, gpointer newc)
{
  (void) mainw;
  newprintcommand = atoi (newc);
  return TRUE;
}

int
newepsopt (GtkWidget * mainw, gpointer newo)
{
  (void) mainw;
  newepsoption = atoi (newo);
  return TRUE;
}

int 
toggle_whiteout (GtkWidget *mainw, gpointer dummy)
{
  (void) mainw;
if (use_whiteout==0)
	use_whiteout=1;
else
   	use_whiteout=0;
return TRUE;
}

int 
toggle_intlchars (GtkWidget *mainw, gpointer dummy)
{
  (void) mainw;
if (use_intlchars==0)
	use_intlchars=1;
else
   	use_intlchars=0;
return TRUE;
}

static void
set_bgcolor (GtkWidget * mainw, GtkColorSelection *colorsel)
/* Handle color data provided by the standard GTK color selector widget */
{
gdouble thecolor[4];

  (void) mainw;

gtk_color_selection_get_color(colorsel,thecolor);
bgred=(int)(thecolor[0]*65535);
bggreen=(int)(thecolor[1]*65535);
bgblue=(int)(thecolor[2]*65535);
background.red=(gushort)bgred;
background.green=(gushort)bggreen;
background.blue=(gushort)bgblue;
  (void)gdk_color_alloc(gdk_colormap_get_system(),&background);
  gdk_gc_set_foreground(background_gc,&background);
  gdk_gc_set_background(background_gc,&background);
  snprintf(bghexcolor,10,"#%2.2x%2.2x%2.2x",(unsigned char)(bgred/256.),(unsigned char)(bggreen/256.),(unsigned char)(bgblue/256.));
#ifdef GTK2
  gtk_label_set_text(GTK_LABEL(GTK_BIN(bgcolorbutton)->child),bghexcolor);
#else
  gtk_label_set_text(GTK_LABEL(GTK_BUTTON(bgcolorbutton)->child),bghexcolor);
#endif
  FreePix();
  CreatePix();
  Display_Mol();
}

#endif

static void
set_fontsize (GtkWidget *mainw, gpointer newfont)
{
  (void) mainw;

  curfontsize=GPOINTER_TO_INT(newfont);
}

int babelcmd (GtkWidget *mainw, gpointer newc)
{
/*@ignore@ splint does not recognize strdup */
  babel=strdup(newc);
/*@end@*/  

  (void) mainw;

  if (loadsave==7){
    char expn[512];
    int n,i;
  
    strcpy(expn,filename);
    n = (int) strlen (expn);
    for (i = 0; i < (int) strlen (expn); i++)
      {
        if (expn[i] == '.')
	  n = i;
        if (expn[i] == '/')
	  n = (int) strlen (expn);
      } 
    expn[n] = '\0';
    strcat(expn,".");
    strcat(expn,babel);
    gtk_file_selection_set_filename (GTK_FILE_SELECTION (filew), expn);
  }
  return TRUE;
}
  
void
Set_Textmode (GtkWidget * mainw, GdkEvent * the_event)
/* callback to force (left-justified) textmode when the text entry
  widget acquires the focus in one of the drawing modes. This is
  necessary to prevent interpretation of the typed text as shortcuts */
{
  (void) mainw;
  (void) the_event;
    
  if (drawmode != 1)
    gtk_toggle_button_set_active ((GtkToggleButton *) ltextbutton,
				  (gboolean) TRUE);
}

void
Change_Font (GtkWidget * mainw)
{
  (void) mainw;

if (serif_flag == 0) {
		serif_flag=1;
  gtk_widget_set_style(GTK_WIDGET(textlabel),seriffontstyle);
	}
	else
	{ 
		serif_flag=0;
  gtk_widget_set_style(GTK_WIDGET(textlabel),normalfontstyle);
	}
}

void
Change_Text (GtkWidget * mainw, gpointer textdir)
/* callback function for the (left, centered,right) text buttons,
   sets drawmode to text with appropriate justification mode and handles 
   'radio button' status of the toolbar line - this is rather  messy, as 
   we have to find out which button to deactivate */
{
  (void) mainw;

  if (importflag != 0)
    return;
  if (drawmode == 3)
    {
      gtk_toggle_button_set_active ((GtkToggleButton *) movebutton,
				    (gboolean) FALSE);
    }
  if (drawmode == 4)
    {
      gtk_toggle_button_set_active ((GtkToggleButton *) markbutton,
				    (gboolean) FALSE);
    }
  if (drawmode == 6)
    {
      gtk_toggle_button_set_active ((GtkToggleButton *) rotatebutton,
				    (gboolean) FALSE);
    }
  if (drawmode == 7)
    {
      gtk_toggle_button_set_active ((GtkToggleButton *) rescalebutton,
				    (gboolean) FALSE);
    }
  if (drawmode == 8)
    {
      gtk_toggle_button_set_active ((GtkToggleButton *) splinebutton,
				    (gboolean) FALSE);
    }
  if (drawmode == 2)
    {
      gtk_toggle_button_set_active ((GtkToggleButton *) bondbutton,
				    (gboolean) FALSE);
    }
  if (drawmode == 0)
    {
      switch (draw_angle)
	{
	case 1:
	  gtk_toggle_button_set_active ((GtkToggleButton *) hexbutton,
					(gboolean) FALSE);
	  break;
	case 2:
	  gtk_toggle_button_set_active ((GtkToggleButton *) pent1button,
					(gboolean) FALSE);
	  break;
	case 3:
	  gtk_toggle_button_set_active ((GtkToggleButton *) pent2button,
					(gboolean) FALSE);
	  break;
	case 4:
	  gtk_toggle_button_set_active ((GtkToggleButton *) octbutton,
					(gboolean) FALSE);
	  break;
	default:
	  fprintf (stderr, _("invalid angle mode %d\n"), draw_angle);
	}
    }
  if (drawmode == 1)
    {
      switch (text_direct)
	{
	case 0:
	  gtk_toggle_button_set_active ((GtkToggleButton *) ltextbutton,
					(gboolean) FALSE);
	  break;
	case -1:
	  gtk_toggle_button_set_active ((GtkToggleButton *) ctextbutton,
					(gboolean) FALSE);
	  break;
	case -2:
	  gtk_toggle_button_set_active ((GtkToggleButton *) rtextbutton,
					(gboolean) FALSE);
	  break;
	default:
	  fprintf (stderr, _("invalid text mode %d\n"), text_direct);
	}
    }
  drawmode = 1;
  text_direct = atoi (textdir);

  gdk_window_set_cursor (drawing_area->window, cursor_text);
}

void
set_bond (GtkWidget * mainw, gpointer bondnum)
{
  (void) mainw;

  curbond = atoi (bondnum);
}

void
Bondmode (GtkWidget * mainw)
/* Callback to enter bondtype changing mode - handles 'radio button'
   status of the toolbar after checking which button to deactivate */
{
  (void) mainw;

  if (importflag != 0)
    return;
  if (drawmode == 8)
    {
      gtk_toggle_button_set_active ((GtkToggleButton *) splinebutton,
				    (gboolean) FALSE);
    }
  if (drawmode == 7)
    {
      gtk_toggle_button_set_active ((GtkToggleButton *) rescalebutton,
				    (gboolean) FALSE);
    }
  if (drawmode == 6)
    {
      gtk_toggle_button_set_active ((GtkToggleButton *) rotatebutton,
				    (gboolean) FALSE);
    }
  if (drawmode == 3)
    {
      gtk_toggle_button_set_active ((GtkToggleButton *) movebutton,
				    (gboolean) FALSE);
    }
  if (drawmode == 4)
    {
      gtk_toggle_button_set_active ((GtkToggleButton *) markbutton,
				    (gboolean) FALSE);
    }

  if (drawmode == 0)
    {
      switch (draw_angle)
	{
	case 1:
	  gtk_toggle_button_set_active ((GtkToggleButton *) hexbutton,
					(gboolean) FALSE);
	  break;
	case 2:
	  gtk_toggle_button_set_active ((GtkToggleButton *) pent1button,
					(gboolean) FALSE);
	  break;
	case 3:
	  gtk_toggle_button_set_active ((GtkToggleButton *) pent2button,
					(gboolean) FALSE);
	  break;
	case 4:
	  gtk_toggle_button_set_active ((GtkToggleButton *) octbutton,
					(gboolean) FALSE);
	}
    }
  if (drawmode == 1)
    {
      switch (text_direct)
	{
	case 0:
	  gtk_toggle_button_set_active ((GtkToggleButton *) ltextbutton,
					(gboolean) FALSE);
	  break;
	case -1:
	  gtk_toggle_button_set_active ((GtkToggleButton *) ctextbutton,
					(gboolean) FALSE);
	  break;
	case -2:
	  gtk_toggle_button_set_active ((GtkToggleButton *) rtextbutton,
					(gboolean) FALSE);
	}
    }
  drawmode = 2;
  gdk_window_set_cursor (drawing_area->window, cursor_bonds);
}

void
Movemode (GtkWidget * mainw)
/* Callback function to enter 'move marked fragment' mode - checks and
   updates radio button status of the toolbar */
{
  (void) mainw;

  if (importflag != 0)
    return;
  if (drawmode == 8)
    {
      gtk_toggle_button_set_active ((GtkToggleButton *) splinebutton,
				    (gboolean) FALSE);
    }
  if (drawmode == 7)
    {
      gtk_toggle_button_set_active ((GtkToggleButton *) rescalebutton,
				    (gboolean) FALSE);
    }
  if (drawmode == 6)
    {
      gtk_toggle_button_set_active ((GtkToggleButton *) rotatebutton,
				    (gboolean) FALSE);
    }
  if (drawmode == 4)
    {
      gtk_toggle_button_set_active ((GtkToggleButton *) markbutton,
				    (gboolean) FALSE);
    }
  if (drawmode == 2)
    {
      gtk_toggle_button_set_active ((GtkToggleButton *) bondbutton,
				    (gboolean) FALSE);
    }
  if (drawmode == 0)
    {
      switch (draw_angle)
	{
	case 1:
	  gtk_toggle_button_set_active ((GtkToggleButton *) hexbutton,
					(gboolean) FALSE);
	  break;
	case 2:
	  gtk_toggle_button_set_active ((GtkToggleButton *) pent1button,
					(gboolean) FALSE);
	  break;
	case 3:
	  gtk_toggle_button_set_active ((GtkToggleButton *) pent2button,
					(gboolean) FALSE);
	  break;
	case 4:
	  gtk_toggle_button_set_active ((GtkToggleButton *) octbutton,
					(gboolean) FALSE);
	}
    }
  if (drawmode == 1)
    {
      switch (text_direct)
	{
	case 0:
	  gtk_toggle_button_set_active ((GtkToggleButton *) ltextbutton,
					(gboolean) FALSE);
	  break;
	case -1:
	  gtk_toggle_button_set_active ((GtkToggleButton *) ctextbutton,
					(gboolean) FALSE);
	  break;
	case -2:
	  gtk_toggle_button_set_active ((GtkToggleButton *) rtextbutton,
					(gboolean) FALSE);
	}
    }
  drawmode = 3;
  gdk_window_set_cursor (drawing_area->window, cursor_move);
  Display_Mol ();
}

void
Markmode (GtkWidget * mainw)
/* Callback function to set 'mark fragment' mode, checks and updates
   radio button state of the toolbar */
{
  (void) mainw;

  if (importflag != 0)
    return;

  if (drawmode == 8)
    {
      gtk_toggle_button_set_active ((GtkToggleButton *) splinebutton,
				    (gboolean) FALSE);
    }
  if (drawmode == 7)
    {
      gtk_toggle_button_set_active ((GtkToggleButton *) rescalebutton,
				    (gboolean) FALSE);
    }
  if (drawmode == 6)
    {
      gtk_toggle_button_set_active ((GtkToggleButton *) rotatebutton,
				    (gboolean) FALSE);
    }
  if (drawmode == 3)
    {
      gtk_toggle_button_set_active ((GtkToggleButton *) movebutton,
				    (gboolean) FALSE);
    }
  if (drawmode == 2)
    {
      gtk_toggle_button_set_active ((GtkToggleButton *) bondbutton,
				    (gboolean) FALSE);
    }
  if (drawmode == 0)
    {
      switch (draw_angle)
	{
	case 1:
	  gtk_toggle_button_set_active ((GtkToggleButton *) hexbutton,
					(gboolean) FALSE);
	  break;
	case 2:
	  gtk_toggle_button_set_active ((GtkToggleButton *) pent1button,
					(gboolean) FALSE);
	  break;
	case 3:
	  gtk_toggle_button_set_active ((GtkToggleButton *) pent2button,
					(gboolean) FALSE);
	  break;
	case 4:
	  gtk_toggle_button_set_active ((GtkToggleButton *) octbutton,
					(gboolean) FALSE);
	}
    }
  if (drawmode == 1)
    {
      switch (text_direct)
	{
	case 0:
	  gtk_toggle_button_set_active ((GtkToggleButton *) ltextbutton,
					(gboolean) FALSE);
	  break;
	case -1:
	  gtk_toggle_button_set_active ((GtkToggleButton *) ctextbutton,
					(gboolean) FALSE);
	  break;
	case -2:
	  gtk_toggle_button_set_active ((GtkToggleButton *) rtextbutton,
					(gboolean) FALSE);
	}
    }
  drawmode = 4;
  gdk_window_set_cursor (drawing_area->window, cursor_markTLC);
}

void
Rotatemode (GtkWidget * mainw)
/* Callback function to set 'rotate marked fragment' mode - checks and
   updates radio button state of the toolbar */
{
  (void) mainw;

  if (drawmode == 8)
    {
      gtk_toggle_button_set_active ((GtkToggleButton *) splinebutton,
				    (gboolean) FALSE);
    }
  if (drawmode == 7)
    {
      gtk_toggle_button_set_active ((GtkToggleButton *) rescalebutton,
				    (gboolean) FALSE);
    }
  if (drawmode == 3)
    {
      gtk_toggle_button_set_active ((GtkToggleButton *) movebutton,
				    (gboolean) FALSE);
    }
  if (drawmode == 4)
    {
      gtk_toggle_button_set_active ((GtkToggleButton *) markbutton,
				    (gboolean) FALSE);
    }

  if (drawmode == 2)
    {
      gtk_toggle_button_set_active ((GtkToggleButton *) bondbutton,
				    (gboolean) FALSE);
    }
  if (drawmode == 0)
    {
      switch (draw_angle)
	{
	case 1:
	  gtk_toggle_button_set_active ((GtkToggleButton *) hexbutton,
					(gboolean) FALSE);
	  break;
	case 2:
	  gtk_toggle_button_set_active ((GtkToggleButton *) pent1button,
					(gboolean) FALSE);
	  break;
	case 3:
	  gtk_toggle_button_set_active ((GtkToggleButton *) pent2button,
					(gboolean) FALSE);
	  break;
	case 4:
	  gtk_toggle_button_set_active ((GtkToggleButton *) octbutton,
					(gboolean) FALSE);
	}
    }
  if (drawmode == 1)
    {
      switch (text_direct)
	{
	case 0:
	  gtk_toggle_button_set_active ((GtkToggleButton *) ltextbutton,
					(gboolean) FALSE);
	  break;
	case -1:
	  gtk_toggle_button_set_active ((GtkToggleButton *) ctextbutton,
					(gboolean) FALSE);
	  break;
	case -2:
	  gtk_toggle_button_set_active ((GtkToggleButton *) rtextbutton,
					(gboolean) FALSE);
	}
    }
  drawmode = 6;
  gdk_window_set_cursor (drawing_area->window, cursor_rotate);
}

void
Rescalemode (GtkWidget * mainw)
/* Callback function to set 'rescale marked fragment' mode - checks and
   updates radio button state of the toolbar */
{
  (void) mainw;

  if (importflag != 0)
    return;
  if (drawmode == 8)
    {
      gtk_toggle_button_set_active ((GtkToggleButton *) splinebutton,
				    (gboolean) FALSE);
    }
  if (drawmode == 3)
    {
      gtk_toggle_button_set_active ((GtkToggleButton *) movebutton,
				    (gboolean) FALSE);
    }
  if (drawmode == 4)
    {
      gtk_toggle_button_set_active ((GtkToggleButton *) markbutton,
				    (gboolean) FALSE);
    }

  if (drawmode == 2)
    {
      gtk_toggle_button_set_active ((GtkToggleButton *) bondbutton,
				    (gboolean) FALSE);
    }

  if (drawmode == 6)
    {
      gtk_toggle_button_set_active ((GtkToggleButton *) rotatebutton,
				    (gboolean) FALSE);
    }

  if (drawmode == 0)
    {
      switch (draw_angle)
	{
	case 1:
	  gtk_toggle_button_set_active ((GtkToggleButton *) hexbutton,
					(gboolean) FALSE);
	  break;
	case 2:
	  gtk_toggle_button_set_active ((GtkToggleButton *) pent1button,
					(gboolean) FALSE);
	  break;
	case 3:
	  gtk_toggle_button_set_active ((GtkToggleButton *) pent2button,
					(gboolean) FALSE);
	  break;
	case 4:
	  gtk_toggle_button_set_active ((GtkToggleButton *) octbutton,
					(gboolean) FALSE);
	}
    }
  if (drawmode == 1)
    {
      switch (text_direct)
	{
	case 0:
	  gtk_toggle_button_set_active ((GtkToggleButton *) ltextbutton,
					(gboolean) FALSE);
	  break;
	case -1:
	  gtk_toggle_button_set_active ((GtkToggleButton *) ctextbutton,
					(gboolean) FALSE);
	  break;
	case -2:
	  gtk_toggle_button_set_active ((GtkToggleButton *) rtextbutton,
					(gboolean) FALSE);
	}
    }
  drawmode = 7;
  gdk_window_set_cursor (drawing_area->window, cursor_rescale);
}

void
Splinemode (GtkWidget * mainw)
/* Callback function to set 'rescale marked fragment' mode - checks and
   updates radio button state of the toolbar */
{
  (void) mainw;

  if (drawmode == 3)
    {
      gtk_toggle_button_set_active ((GtkToggleButton *) movebutton,
				    (gboolean) FALSE);
    }
  if (drawmode == 4)
    {
      gtk_toggle_button_set_active ((GtkToggleButton *) markbutton,
				    (gboolean) FALSE);
    }

  if (drawmode == 2)
    {
      gtk_toggle_button_set_active ((GtkToggleButton *) bondbutton,
				    (gboolean) FALSE);
    }

  if (drawmode == 6)
    {
      gtk_toggle_button_set_active ((GtkToggleButton *) rotatebutton,
				    (gboolean) FALSE);
    }

  if (drawmode == 0)
    {
      switch (draw_angle)
	{
	case 1:
	  gtk_toggle_button_set_active ((GtkToggleButton *) hexbutton,
					(gboolean) FALSE);
	  break;
	case 2:
	  gtk_toggle_button_set_active ((GtkToggleButton *) pent1button,
					(gboolean) FALSE);
	  break;
	case 3:
	  gtk_toggle_button_set_active ((GtkToggleButton *) pent2button,
					(gboolean) FALSE);
	  break;
	case 4:
	  gtk_toggle_button_set_active ((GtkToggleButton *) octbutton,
					(gboolean) FALSE);
	}
    }
  if (drawmode == 1)
    {
      switch (text_direct)
	{
	case 0:
	  gtk_toggle_button_set_active ((GtkToggleButton *) ltextbutton,
					(gboolean) FALSE);
	  break;
	case -1:
	  gtk_toggle_button_set_active ((GtkToggleButton *) ctextbutton,
					(gboolean) FALSE);
	  break;
	case -2:
	  gtk_toggle_button_set_active ((GtkToggleButton *) rtextbutton,
					(gboolean) FALSE);
	}
    }
  drawmode = 8;
  gdk_window_set_cursor (drawing_area->window, cursor_pencil);
}

void
Change_Angle (GtkWidget * mainw, gpointer newangle)
/* Callback function to set 'line drawing' mode and appropriate grid
   definition - checks and updates radio button state of the toolbar */
{
  gboolean activate = (gboolean) FALSE;

  (void) mainw;

  if (importflag != 0)
    return;
  if (drawmode == 3)
    {
      gtk_toggle_button_set_active ((GtkToggleButton *) movebutton, activate);
    }
  if (drawmode == 4)
    {
      gtk_toggle_button_set_active ((GtkToggleButton *) markbutton, activate);
    }
  if (drawmode == 6)
    {
      gtk_toggle_button_set_active ((GtkToggleButton *) rotatebutton,
				    activate);
    }
  if (drawmode == 7)
    {
      gtk_toggle_button_set_active ((GtkToggleButton *) rescalebutton,
				    activate);
    }
  if (drawmode == 8)
    {
      gtk_toggle_button_set_active ((GtkToggleButton *) splinebutton,
				    activate);
    }
  if (drawmode == 2)
    {
      gtk_toggle_button_set_active ((GtkToggleButton *) bondbutton, activate);
    }
  if (drawmode == 1)
    {
      gtk_toggle_button_set_active ((GtkToggleButton *) ltextbutton,
				    activate);
      gtk_toggle_button_set_active ((GtkToggleButton *) ctextbutton,
				    activate);
      gtk_toggle_button_set_active ((GtkToggleButton *) rtextbutton,
				    activate);
    }

  drawmode = 0;
  gdk_window_set_cursor (drawing_area->window, cursor_pencil);

  switch (draw_angle)
    {
    case 1:
      gtk_toggle_button_set_active ((GtkToggleButton *) hexbutton, activate);
      break;
    case 2:
      gtk_toggle_button_set_active ((GtkToggleButton *) pent1button,
				    activate);
      break;
    case 3:
      gtk_toggle_button_set_active ((GtkToggleButton *) pent2button,
				    activate);
      break;
    case 4:
      gtk_toggle_button_set_active ((GtkToggleButton *) octbutton, activate);
    }
  draw_angle = atoi (newangle);

}

void
CheckAndLoad ()
/* Callback function for 'Load' button - pops up an 'are you sure' dialog
   if unsaved changes exist, else calls real load function directly */
{
  if (modify == 0 || hp->n + hp->nc + hp->nsp == 0)
    {
      Load ();
    }
  else
    {
      yesnodialog (Load);
    }
}

void
Load ()
/* called for loading drawings from file - sets i/o mode to loading and 
   pops up the file selection widget, making it modal */
{
  loadsave = 1;
  gtk_window_set_title (GTK_WINDOW (filew), _("Load from file..."));
  savedpicture = gdk_pixmap_ref(picture);
  if (preview){
    gdk_pixmap_unref (preview);
	      preview=NULL;
	      }
  preview = gdk_pixmap_new (filew->window, 200, 100, -1);

  gdk_draw_rectangle (preview,
		      filew->style->white_gc,
		      (gint) TRUE, 0, 0, (gint) 200, (gint) 100);

  picture = gdk_pixmap_ref(preview);
  gtk_widget_show (preview_area);
  gtk_widget_hide (pdbhbox);
  gtk_widget_hide (sdfhbox);
  if (babelin>-1) gtk_widget_hide (babelcmdmenu);
  if (babelout>-1) gtk_widget_hide (babelexpmenu);
#ifndef GTK2
  strcpy (datadir, gtk_entry_get_text (GTK_ENTRY (defaultdir)));
  if (datamask[0] != '\0')
    gtk_file_selection_complete (GTK_FILE_SELECTION (filew),
				 strcat (datadir, datamask));
  else
    gtk_file_selection_complete (GTK_FILE_SELECTION (filew),
				 strcat (datadir, "*"));
#endif
  gtk_widget_show (filew);
  gtk_window_set_modal (GTK_WINDOW (filew), (gboolean) TRUE);
}

void
Import ()
/* called for loading ISIS drawings from file - sets i/o mode to loading and 
   pops up the file selection widget, making it modal */
{
#if (GTK_MINOR_VERSION >2)
GtkFileFilter *mdlfilter;
#endif
  loadsave = 4;
  savedpicture = gdk_pixmap_ref(picture);
  gtk_window_set_title (GTK_WINDOW (filew), _("Import MDL file..."));
  if (preview) {
    gdk_pixmap_unref (preview);
    preview=NULL;
    }
  preview = gdk_pixmap_new (filew->window, 200, 100, -1);
  gdk_draw_rectangle (preview, filew->style->white_gc,
                      (gint)TRUE, 0, 0, (gint)200, (gint)100);
  picture = gdk_pixmap_ref(preview);
  gtk_widget_show (preview_area);                    
  gtk_widget_hide (pdbhbox);
  gtk_widget_show (sdfhbox);
  if (babelin >-1) gtk_widget_hide (babelcmdmenu);
  if (babelout>-1) gtk_widget_hide (babelexpmenu);
#ifndef GTK2
  strcpy (datadir, gtk_entry_get_text (GTK_ENTRY (defaultdir)));
#endif  
#if (GTK_MINOR_VERSION >2)
  mdlfilter = gtk_file_filter_new();
  gtk_file_filter_add_pattern(mdlfilter,"*.mol");
  gtk_file_filter_add_pattern(mdlfilter,"*.mdl");
  gtk_file_filter_add_pattern(mdlfilter,"*.MOL");
  gtk_file_filter_add_pattern(mdlfilter,"*.MDL");
  gtk_file_filter_add_pattern(mdlfilter,"*.sdf");
  gtk_file_filter_add_pattern(mdlfilter,"*.SDF");
#else    
  gtk_file_selection_complete (GTK_FILE_SELECTION (filew),
			       strcat (datadir, "*.mol,*.mdl,*.sdf"));
#endif
  gtk_widget_show (filew);
  gtk_window_set_modal (GTK_WINDOW (filew), (gboolean) TRUE);
}

void
Import_Babel ()
/* called for BABEL-based import of foreign files - sets i/o mode  
	and pops up the file selection widget, making it modal */
{
  loadsave = 6;
  savedpicture = gdk_pixmap_ref(picture);
  gtk_window_set_title (GTK_WINDOW (filew), _("Import via BABEL..."));
  gtk_widget_hide (preview_area);
  gtk_widget_hide (pdbhbox);
  gtk_widget_hide (sdfhbox);
  gtk_widget_hide (babelexpmenu);
  gtk_widget_show (babelcmdmenu);
#ifndef GTK2
  strcpy (datadir, gtk_entry_get_text (GTK_ENTRY (defaultdir)));
  gtk_file_selection_complete (GTK_FILE_SELECTION (filew),
			       strcat (datadir, "*"));
#endif
  gtk_widget_show (filew);
  gtk_window_set_modal (GTK_WINDOW (filew), (gboolean) TRUE);
}

void
Export_Babel ()
/* called for BABEL-based export of foreign files - sets i/o mode  
	and pops up the file selection widget, making it modal */
{
  loadsave = 7;
  savedpicture = gdk_pixmap_ref(picture);
  gtk_window_set_title (GTK_WINDOW (filew), _("Export via BABEL..."));
  gtk_widget_hide (preview_area);
  gtk_widget_hide (pdbhbox);
  gtk_widget_hide (sdfhbox);
  gtk_widget_hide (babelcmdmenu);
  gtk_widget_show (babelexpmenu);
#ifndef GTK2
  strcpy (datadir, gtk_entry_get_text (GTK_ENTRY (defaultdir)));
  gtk_file_selection_complete (GTK_FILE_SELECTION (filew),
			       strcat (datadir, "*"));
#endif
  gtk_widget_show (filew);
  gtk_window_set_modal (GTK_WINDOW (filew), (gboolean) TRUE);
}

void
Import_PDB ()
/* called for loading drawings from file - sets i/o mode to loading and 
   pops up the file selection widget, making it modal */
{
  loadsave = 5;
  savedpicture = gdk_pixmap_ref(picture);
  gtk_window_set_title (GTK_WINDOW (filew), _("Import PDB file..."));
  gtk_widget_hide (preview_area);
  if (babelin >-1) gtk_widget_hide (babelcmdmenu);
  if (babelout>-1) gtk_widget_hide (babelexpmenu);
  gtk_widget_show (pdbhbox);
  gtk_widget_hide (sdfhbox);
#ifndef GTK2
  strcpy (datadir, gtk_entry_get_text (GTK_ENTRY (defaultdir)));
  gtk_file_selection_complete (GTK_FILE_SELECTION (filew),
			       strcat (datadir, "*.pdb,*.ent"));
#endif
  gtk_widget_show (filew);
  gtk_window_set_modal (GTK_WINDOW (filew), (gboolean) TRUE);
}

void
Add ()
/* called for inserting drawings at the currently marked position - sets 
   i/o mode to adding and pops up the (modal) file selection dialog */
{
  if (addflag == 0)
    {
      refx = hp->tx;
      refy = hp->ty;
    }

  loadsave = 3;
  if(picture != NULL)savedpicture = gdk_pixmap_ref(picture);
  if (preview != NULL)picture = gdk_pixmap_ref(preview);
  if (picture) {
    gdk_pixmap_unref (picture);
	picture=NULL;
	}
  picture = gdk_pixmap_new (filew->window, 200, 100, -1);

  gdk_draw_rectangle (picture,
		      filew->style->white_gc,
		      (gint) TRUE, 0, 0, (gint) 200, (gint) 100);
  gtk_widget_show (preview_area);
  gtk_widget_hide (pdbhbox);
  gtk_widget_hide (sdfhbox);
  if (babelin >-1) gtk_widget_hide (babelcmdmenu);
  if (babelout>-1) gtk_widget_hide (babelexpmenu);
  gtk_window_set_title (GTK_WINDOW (filew), _("Add from file..."));
#ifndef GTK2
  strcpy (datadir, gtk_entry_get_text (GTK_ENTRY (defaultdir)));
  if (datamask[0] != '\0')
    gtk_file_selection_complete (GTK_FILE_SELECTION (filew),
				 strcat (datadir, datamask));
  else
    gtk_file_selection_complete (GTK_FILE_SELECTION (filew),
				 strcat (datadir, "*"));
#endif
  gtk_widget_show (filew);
  gtk_window_set_modal (GTK_WINDOW (filew), (gboolean) TRUE);
}

void
show_or_raise (GtkWidget * thewidget)
{
  if (!GTK_WIDGET_MAPPED (thewidget))
    gtk_widget_show (thewidget);
  else
    gdk_window_raise (thewidget->window);
}



void
Add_template (GtkWidget * mainw, gpointer template)
{
  int i, j, n;
  int xdiff, ydiff;
  int x, y, tx, ty, b;
  int x1, y1, x2, y2;

  if (addflag == 0)
    {
      refx = hp->tx;
      refy = hp->ty;
    }
  Unmark_all();
  xdiff = 0;
  ydiff = 0;
  xref = 0;
  yref = 0;
  i = GPOINTER_TO_INT(template) / 10;
  j = GPOINTER_TO_INT(template) - i * 10;
      if (template_refx[i][j] != 0 && template_refy[i][j]!=0)
	{
	  xdiff = refx - template_refx[i][j];
	  ydiff = refy - template_refy[i][j];
	}
  for (n = 0; n < template_nb[i][j]; n++)
    {
      x = template_x[i][j][n];
      y = template_y[i][j][n];
      tx = template_tx[i][j][n];
      ty = template_ty[i][j][n];
      b = template_b[i][j][n];


      x = x + xdiff;
      y = y + ydiff;
      tx = tx + xdiff;
      ty = ty + ydiff;
      if (GPOINTER_TO_INT(template) <175) /* templates beyond the 4th page are decoration */
      add_struct (x, y, tx, ty, b, 1, 1, 0,curpen);
      else
      add_struct (x, y, tx, ty, b, 1, 1, 1,curpen);
    }
  for (n = 0; n < template_nl[i][j]; n++)
    {
      x = template_lx[i][j][n] + xdiff;
      y = template_ly[i][j][n] + ydiff;
      add_char (x, y, template_lt[i][j][n], template_lo[i][j][n], 1,curpen,0,curfontsize);
    }
  for (n = 0; n < template_ncrv[i][j]; n++)
    {
      x = template_crv[i][j][n][0] + xdiff;
      y = template_crv[i][j][n][1] + ydiff;
      x1 = template_crv[i][j][n][2] + xdiff;
      y1 = template_crv[i][j][n][3] + ydiff;
      x2 = template_crv[i][j][n][4] + xdiff;
      y2 = template_crv[i][j][n][5] + ydiff;
      tx = template_crv[i][j][n][6] + xdiff;
      ty = template_crv[i][j][n][7] + ydiff;
      b = template_crv[i][j][n][8];
      add_spline (x, y, x1, y1, x2, y2, tx, ty, b, 1,curpen);
    }

#ifdef LIBUNDO
  undo_snapshot ();
#endif

  mark.flag = 1;
  mark.x = refx;
  mark.y = refy;
  mark.w = 200;
  mark.h = 200;

  Movemode (mainw);
  gtk_toggle_button_set_active ((GtkToggleButton *) movebutton,
				(gboolean) TRUE);
  CreatePix ();
  Display_Mol ();
}

void
SaveAs ()
/* Callback for saving a drawing to file - sets i/o mode to saving and pops
   up the (modal) file selection dialog with the current filename as the default */
{
  if (hp->n + hp->nc + hp->nsp == 0)
    return;
  loadsave = 2;
    savedpicture =  gdk_pixmap_ref(picture);
  if (filename[0] != '\0')
    gtk_file_selection_set_filename (GTK_FILE_SELECTION (filew), filename);
  gtk_window_set_title (GTK_WINDOW (filew), _("Save as..."));
  gtk_widget_hide (preview_area);
  gtk_widget_hide (pdbhbox);
  gtk_widget_hide (sdfhbox);
  if (babelin >-1) gtk_widget_hide (babelcmdmenu);
  if (babelout>-1) gtk_widget_hide (babelexpmenu);
  gtk_widget_show (filew);
  gtk_window_set_modal (GTK_WINDOW (filew), (gboolean) TRUE);
}

void
Save ()
/* Callback for saving drawing to file - uses current filename */
{
  int error;
  char errtext[255];

  if (hp->n + hp->nc + hp->nsp == 0)
  {
	  snprintf (errtext, sizeof (errtext), _("\nNothing to save") );
#ifdef GTK2
        gtk_text_buffer_insert(msgtextbuffer, &iter, errtext, -1);
        gtk_adjustment_set_value(msgadjustment, gtk_adjustment_get_value(msgadjustment)+12.);
#else        	
	gtk_text_insert (GTK_TEXT (msgtext), NULL, NULL, NULL, errtext,
		   (gint) strlen (errtext));
#endif
       return;
     }

  if (strcmp (filename, _("unnamed")) != 0)
    {
      {
	FILE *fp;
	if ((fp = fopen (filename, "w")) == NULL)
	  error = 1;
	else
	  {
	    error = save_mol (fp, 0);
	    if (!error)
	      fclose (fp);
	  }
      }
      if (error != 0)
	{
    snprintf (errtext, sizeof (errtext), _("\nWriting to %s failed !"), filename);
	}
      else
	{
    snprintf (errtext, sizeof (errtext),
		   _("\nDrawing saved in %s (%d bonds, %d labels)"),
		   filename, hp->n, hp->nc);
	  modify = 0;
	}
#ifdef GTK2
    gtk_text_buffer_insert (msgtextbuffer, &iter, errtext, -1);
    gtk_adjustment_set_value (msgadjustment, gtk_adjustment_get_value(msgadjustment)+12.);
#else    
    gtk_text_insert (GTK_TEXT (msgtext), NULL, NULL, NULL, errtext, (gint) strlen (errtext));
#endif
    }
  else
    SaveAs ();
  return;
}

void
Export ()
/* Callback for exporting a drawing to file - pops
   up the (modal) file selection dialog with the current filename as the default */
{
  char expn[512];
  char *exten[10] = { ".fig", ".tex", ".eps", ".xbm", ".svg", ".mol",".emf",".sxd",".png",".asy"};
  int i, n;

  if (hp->n + hp->nc + hp->nsp == 0)
    return;

  strcpy (expn, filename);
  n = (int) strlen (expn);
  for (i = 0; i < (int) strlen (expn); i++)
    {
      if (expn[i] == '.')
	n = i;
      if (expn[i] == '/')
	n = (int) strlen (expn);
    }
  expn[n] = '\0';
  strcat (expn, exten[expmode]);
  gtk_file_selection_set_filename (GTK_FILE_SELECTION (expw), expn);
  gtk_widget_show (expw);
  gtk_window_set_modal (GTK_WINDOW (filew), (gboolean) TRUE);
}

void
do_export (GtkWidget * mainw, GtkFileSelection * exp)
/* initiates export of current drawing to the foreign format selected via
   the export popup - pops down export dialog, calls appropriate function
   and displays messagebox with file statistics or error message afterwards */
{
  struct stat stbuf;
  (void) mainw;

  gtk_widget_hide (expw);
  if (hp->n + hp->nc + hp->nsp == 0)
    return;
  strcpy (expname,
	  gtk_file_selection_get_filename (GTK_FILE_SELECTION (exp)));
  if (stat (expname, &stbuf) == 0)
    yesnodialog2 (really_export, expname);
  else
    really_export ();
}

void
really_export ()
{
  float expscale;
  int error = 0;
  char errtext[1024];
  FILE *fp;
  
  expscale =
    gtk_spin_button_get_value_as_int ((GtkSpinButton *) scale) / 100.;

  switch (expmode)
    {
    case 0:
      error = export_xfig (expname);
      break;
    case 1:
      error = export_latex_pic (expname, expscale);
      break;
    case 2:
      error = export_ps_pic (expname, expscale);
      break;
    case 3:
      /* xbm output is slow - set 'busy' watch-shaped cursor */
/*      gdk_window_set_cursor (drawing_area->window, cursor_busy);*/
/*      while (g_main_iteration (FALSE));*/
      /* allow gtk to redraw after the popup, 
         which may have obscured parts of our molecule */
      if (mark.flag == 1)
	{
	  xbmflag = 1;
	  FreePix ();
	  CreatePix ();
	  Display_Mol ();
	}
      error = export_bitmap (expname);
      if (mark.flag == 1)
	{
	  xbmflag = 0;
	  FreePix ();
	  CreatePix ();
	  Display_Mol ();
	}
/*      gdk_window_set_cursor (drawing_area->window, previous_cursor);*/
      break;
    case 4:
      error = export_svg (expname);
      break;
    case 5:
      if (!(fp = fopen (expname,"w"))) {
        error = 1;
        break;
      }  
      error = export_mdl_mol (fp,0);
      break;
    case 6:  
#ifdef EMF
      error = export_emf (expname);
#else
	if (figversion >= 3) error = export_emf (expname);
#endif	
      break;
    case 7:
      error = export_sxd (expname);  
      break;
    case 8:
      error = export_png_pic (expname, expscale);
      break;
    case 9:
      error = export_asy (expname);  
      break;
   }
  if (error != 0)
    {
  snprintf (errtext, sizeof (errtext), _("Writing to\n %s\nfailed !\n"), expname);
      gtk_label_set_text (GTK_LABEL (message), errtext);
      gtk_widget_show (messagew);
      gtk_grab_add (messagew);
    }
  else
    {
  snprintf (errtext, sizeof (errtext),
		_("\nDrawing exported as %s (%d bonds, %d labels)"),
		expname, hp->n, hp->nc);
#ifdef GTK2
      gtk_text_buffer_insert (msgtextbuffer, &iter, errtext, -1);
      gtk_adjustment_set_value (msgadjustment, gtk_adjustment_get_value(msgadjustment)+12.);
#else      
      gtk_text_insert (GTK_TEXT (msgtext), NULL, NULL, NULL, errtext,
		       (gint) strlen (errtext));
#endif
    }
}

void
exp_mode (GtkWidget * mainw, gpointer mode)
/* callback function to set export format selected in the export dialog */
{
  char expn[255];
  char *exten[10] = { ".fig", ".tex", ".eps", ".xbm", ".svg", ".mol", ".emf", ".sxd", ".png", ".asy" };
  int i, n;

  (void) mainw;

  strcpy (expn, gtk_file_selection_get_filename (GTK_FILE_SELECTION (expw)));
  n = (int) strlen (expn);
  for (i = 0; i < (int) strlen (expn); i++)
    {
      if (expn[i] == '.')
	n = i;
      if (expn[i] == '/')
	n = (int) strlen (expn);
    }
  expn[n] = '\0';
  expmode = atoi (mode);
  if (expmode < 0 || expmode > 9)
    expmode = 0;
  strcat (expn, exten[expmode]);

  gtk_file_selection_set_filename (GTK_FILE_SELECTION (expw), expn);
}

void
pdb_mode (GtkWidget * mainw, gpointer mode)
/* callback function to set pdb label handling */
{
  (void) mainw;

  pdbmode = atoi (mode);
  if (pdbmode < 0 || pdbmode > 4)
    pdbmode = 0;
}

void
sdf_mode (GtkWidget * mainw, gpointer mode)
/* callback function to set current sdf index and update preview */
{
char myfile[255];
char labeltext[40];

  (void) mainw;

  switch ( atoi (mode)) {
        case 0:
        default:
        	sdfindex=0;
        	break;
        case 1:
        	sdfindex--;
        	if (sdfindex <0 ) sdfindex = 0;
        	break;
        case 2:
        	sdfindex++;
        	break;
        }
   if (picture) {
      gdk_pixmap_unref (picture);
      picture = NULL;
   }
   picture = gdk_pixmap_new (filew->window, 200, 100, -1);
   gdk_draw_rectangle (picture, filew->style->white_gc,
   		       (gint)TRUE, 0, 0, (gint)200, (gint)100);
   strcpy (myfile,
           gtk_file_selection_get_filename (GTK_FILE_SELECTION(filew)));   		            		
   preview_mdl_mol (myfile, sdfindex);
   snprintf (labeltext,39,_("SDF entry: %d"),sdfindex+1);
   gtk_label_set_text(GTK_LABEL(sdflabel),labeltext);
}
   
int
CheckAndQuit ()
/* Callback function for the Quit button - pops up an 'are you sure' dialog
   if unsaved changes exist, or calls the real quit function directly */
{
  if (modify == 0 || hp->n + hp->nc + hp->nsp == 0)
    {
      Quit ();
      return ((int) FALSE);
    }
  else
    {
      yesnodialog (Quit);
      return ((int) TRUE);
    }
}

void
Quit ()
/* close down gtk function processing and terminate program */
{
  gtk_main_quit ();
}

void
file_ok_sel (GtkWidget * mainw, GtkFileSelection * fs)
/* callback function for the Ok button of the file selection dialog - pops
   down the file dialog, gets the name of the selected file, initiates load
   or save operations as defined by the current i/o mode and displays error
   message dialog if necessary */
{
  int error;
  char errtext[1024];
  char oldname[512];
  char *tempstr;
  struct stat stbuf;

  (void) mainw;

  gtk_widget_hide (filew);
  gtk_grab_remove (filew);
  picture = gdk_pixmap_ref(savedpicture);
  strcpy (oldname, filename);
  strcpy (filename,
	  gtk_file_selection_get_filename (GTK_FILE_SELECTION (fs)));
  if (datadir[0] == '\0')
    {
      tempstr = strrchr (filename, '/');
      size_t dirlen = tempstr ? (size_t)(tempstr - filename + 1) : 0;
      if (dirlen >= sizeof (datadir))
        dirlen = sizeof (datadir) - 1;
      memcpy (datadir, filename, dirlen);
      datadir[dirlen] = '\0';
#if (GTK_MINOR_VERSION >2)
      gtk_entry_set_text (GTK_ENTRY (datadir_entry), datadir);
#else
      gtk_entry_set_text (GTK_ENTRY (defaultdir), datadir);
#endif
    }
  switch (loadsave)
    {
    case 1:
      error = load_mol (filename);
      switch (error)
	{
	case 0:
	  modify = 0;
	  mark.flag = 0;
    snprintf (errtext, sizeof (errtext), "Chemtool %s (%s)", VERSION, filename);
	  gtk_window_set_title (GTK_WINDOW (window), errtext);
#ifdef LIBUNDO
	  undo_snapshot ();
#endif
	  break;
	case 1:
    snprintf (errtext, sizeof (errtext), _("Unable to open %s\n"), filename);
	  strcpy (filename, oldname);
	  gtk_label_set_text (GTK_LABEL (message), errtext);
	  gtk_widget_show (messagew);
	  gtk_grab_add (messagew);
	  break;
	case 2:
    snprintf (errtext, sizeof (errtext), _("%s\n does not appear to be a Chemtool file\n"),
		   filename);
	  strcpy (filename, oldname);
	  gtk_label_set_text (GTK_LABEL (message), errtext);
	  gtk_widget_show (messagew);
	  gtk_grab_add (messagew);
	  break;
	case 3:
	  modify = 0;
    snprintf (errtext, sizeof (errtext),
		   _("%s was created by a newer version.\nSome features may be lost.\n"),
		   filename);
	  gtk_label_set_text (GTK_LABEL (message), errtext);
	  gtk_widget_show (messagew);
	  gtk_grab_add (messagew);
    snprintf (errtext, sizeof (errtext), "Chemtool %s (%s)", VERSION, filename);
	  gtk_window_set_title (GTK_WINDOW (window), errtext);
#ifdef LIBUNDO
	  undo_snapshot ();
#endif
	  break;
	default:
	  clear_data ();
    snprintf (errtext, sizeof (errtext), _("Error loading %s \n"), filename);
	  gtk_label_set_text (GTK_LABEL (message), errtext);
	  gtk_widget_show (messagew);
	  gtk_grab_add (messagew);
	}
      CreatePix ();
      Display_Mol ();
      break;
    case 2:
  if (datamask[0] != '\0') {
        tempstr = strrchr (filename,'.');
        if (!tempstr || strcmp(tempstr,datamask)) {
          strcat (filename,".");
          strcat (filename,datamask);
        }
      }  
      if (stat (filename, &stbuf) == 0)
	yesnodialog2 (reallysave, filename);
      else
	reallysave ();
      break;
    case 3:
      error = add_mol (filename);
      strcpy (filename, oldname);
      Movemode (mainw);
      gtk_toggle_button_set_active ((GtkToggleButton *) movebutton,
				    (gboolean) TRUE);
      CreatePix ();
      Display_Mol ();
      break;
    case 4:
      error = import_mdl_mol (filename,sdfindex);
      switch (error)
	{
	case 0:
	  modify = 0;
	  if (strrchr (filename, '.')
	      && strrchr (filename, '.') > strrchr (filename, '/'))
	    filename[(int) (strrchr (filename, '.') - filename)] = '\0';
    snprintf (errtext, sizeof (errtext), "Chemtool %s (%s)", VERSION, filename);
	  gtk_window_set_title (GTK_WINDOW (window), errtext);
#ifdef LIBUNDO
	  undo_snapshot ();
#endif

    snprintf (errtext, sizeof (errtext),
		   _("\nChoose orientation (Ctrl-Mouse1 for z), press Enter when done"));
#ifdef GTK2
          gtk_text_buffer_insert (msgtextbuffer, &iter, errtext, -1);
          gtk_adjustment_set_value(msgadjustment, gtk_adjustment_get_value(msgadjustment)+12.);
#else          
	  gtk_text_insert (GTK_TEXT (msgtext), NULL, NULL, NULL, errtext,
			   (gint)strlen (errtext));
#endif
	  gtk_toggle_button_set_active ((GtkToggleButton *) rotatebutton,
					TRUE);
	  break;
	case 1:
    snprintf (errtext, sizeof (errtext), _("Unable to open %s\n"), filename);
	  strcpy (filename, oldname);
	  gtk_label_set_text (GTK_LABEL (message), errtext);
	  gtk_widget_show (messagew);
	  gtk_grab_add (messagew);
	  break;
	case 2:
    snprintf (errtext, sizeof (errtext), _("Problems converting %s\n"), filename);
	  gtk_label_set_text (GTK_LABEL (message), errtext);
	  gtk_widget_show (messagew);
	  gtk_grab_add (messagew);
	  break;
	}
      break;
    case 5:
      error = import_pdb (filename);
      switch (error)
	{
	case 0:
	  modify = 0;
	  if (strrchr (filename, '.')
	      && strrchr (filename, '.') > strrchr (filename, '/'))
	    filename[(int) (strrchr (filename, '.') - filename)] = '\0';
	  snprintf (errtext, 255, "Chemtool %s (%s)", VERSION, filename);
	  gtk_window_set_title (GTK_WINDOW (window), errtext);
#ifdef LIBUNDO
	  undo_snapshot ();
#endif
    snprintf (errtext, sizeof (errtext),
		   _("\nChoose orientation (Ctrl-Mouse1 for z), press Enter when done"));
#ifdef GTK2
          gtk_text_buffer_insert (msgtextbuffer, &iter, errtext, -1);
          gtk_adjustment_set_value(msgadjustment, gtk_adjustment_get_value(msgadjustment)+12.);
#else          
	  gtk_text_insert (GTK_TEXT (msgtext), NULL, NULL, NULL, errtext,
			   (gint)strlen (errtext));
#endif
	  gtk_toggle_button_set_active ((GtkToggleButton *) rotatebutton,
					TRUE);
	  break;
	case 1:
    snprintf (errtext, sizeof (errtext), _("Unable to open %s\n"), filename);
	  strcpy (filename, oldname);
	  gtk_label_set_text (GTK_LABEL (message), errtext);
	  gtk_widget_show (messagew);
	  gtk_grab_add (messagew);
	  break;
	}
	break;
    case 6:
      error = import_babel (filename);
      switch (error)
	{
	case 0:
	  modify = 0;
	  if (strrchr (filename, '.')
	      && strrchr (filename, '.') > strrchr (filename, '/'))
	    filename[(int) (strrchr (filename, '.') - filename)] = '\0';
    snprintf (errtext, sizeof (errtext), "Chemtool %s (%s)", VERSION, filename);
	  gtk_window_set_title (GTK_WINDOW (window), errtext);
#ifdef LIBUNDO
	  undo_snapshot ();
#endif

    snprintf (errtext, sizeof (errtext),
		   _
		   ("\nChoose orientation (Ctrl-Mouse1 for z), press Enter when done"));
#ifdef GTK2
          gtk_text_buffer_insert (msgtextbuffer, &iter, errtext, -1);
          gtk_adjustment_set_value(msgadjustment, gtk_adjustment_get_value(msgadjustment)+12.);
#else          
	  gtk_text_insert (GTK_TEXT (msgtext), NULL, NULL, NULL, errtext,
			   (gint)strlen (errtext));
#endif
	  gtk_toggle_button_set_active ((GtkToggleButton *) rotatebutton,
					TRUE);
	  break;
	case 1:
    snprintf (errtext, sizeof (errtext), _("Unable to open %s\n"), filename);
	  strcpy (filename, oldname);
	  gtk_label_set_text (GTK_LABEL (message), errtext);
	  gtk_widget_show (messagew);
	  gtk_grab_add (messagew);
	  break;
	case 2:
    snprintf (errtext, sizeof (errtext), _("Problems converting %s\n"), filename);
	  gtk_label_set_text (GTK_LABEL (message), errtext);
	  gtk_widget_show (messagew);
	  gtk_grab_add (messagew);
	  break;
	}
      break;
    case 7:
        error = export_babel(filename);    
    default:
      break;
    }
}

void
reallysave ()
{
  int error;
  char errtext[1024];
  FILE *fp;

  if ((fp = fopen (filename, "w")) == NULL)
    error = 1;
  else
    {
      error = save_mol (fp, 0);
      if (!error)
	fclose (fp);
    }
  if (error != 0)
    {
  snprintf (errtext, sizeof (errtext), _("Writing to\n %s\nfailed !\n"), filename);
    }
  else
    {
  snprintf (errtext, sizeof (errtext),
	       _("Drawing saved in\n %s\n (%d bonds, %d labels)\n"),
	       filename, hp->n, hp->nc);
      modify = 0;
    }
  gtk_label_set_text (GTK_LABEL (message), errtext);
  gtk_widget_show (messagew);
  gtk_grab_add (messagew);
  snprintf (errtext, sizeof (errtext), "Chemtool %s (%s)", VERSION, filename);
  gtk_window_set_title (GTK_WINDOW (window), errtext);
}

int
main (int argc, char **argv)
/* Main program - initializes widgets and data structures */
{
  int i, j;
  int tmplnum[125];
  int error;
  char bondnums[BONDTYPES][3];
  char bondcolors [BONDCOLORS][4];
  char msgtmp[100];
  const char *fontsizelabel[7]={"8","10","12","14","17","20","24"};
  GtkWidget *aboutw,*helpw,*helptext;
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *button;
  GtkWidget *expbutton[10];
  GtkWidget *tbutton[125];
#ifdef MENU
  GtkAccelGroup *accel_group;
  GtkWidget *menu_bar;
  GtkWidget *file, *file_menu;
  GtkWidget *new, *open, *add, *import, *imppdb, *impany,*export, *expany, 
          *printps, *save, *saveas, *quit;
  GtkWidget *edit, *edit_menu;
  GtkWidget *print_setup, *save_setup;
  GtkWidget *copy, *fliph, *flipv;
#ifdef LIBUNDO
  GtkWidget *undo, *redo;
#endif
  GtkWidget *view, *view_menu;
  GtkWidget *zoomin, *zoomout, *center,*grid;
  GtkWidget *tools, *tools_menu;
  GtkWidget *templatem, *cht, *clean;
  GtkWidget *help, *help_menu;
  GtkWidget *about,*using;
#else
  GtkWidget *loadbutton, *savebutton, *importbutton, *imppdbbutton,
    *exportbutton, *printbutton, *zoominbutton;
  GtkWidget    *psetupbutton, *savesetupbutton; 
  GtkWidget *centerbutton, *zoomoutbutton, *clearbutton, *fwbutton,
    *quitbutton, *aboutbutton;
#endif
  GtkWidget *label;
  GdkPixmap *pixmap;
  GtkWidget *pixmapwid;
  GtkAdjustment *adj_scale;
  GSList *group;
  GtkStyle *style;
  GtkTooltips *tooltips, *temptips;
  GtkWidget *templatebook, *page, *tvbox, *pbox;
  GtkWidget *vscroll,*textscroll;
#if (GTK_MINOR_VERSION <4)
  GtkAdjustment *adj_pscale;
  GtkWidget *printer_dialog, *papersizeitem[11], *orientationitem,
    *printcmditem;
  GtkWidget *epsoptitem; 
  GtkWidget *pokbutton, *pcabutton;
#endif      
  GtkWidget *fontsizeitem[7];  
  GtkWidget *babelcmditem,*babelexpcmditem;  
  GdkColor black={0,0x0000,0x0000,0x0000},blue={0,0x0000,0x0000,0xe000},
  green={0,0x0000,0xd000,0x0000},cyan={0,0,0xffff,0xffff},
  red={0,0xffff,0x0000,0x0000},magenta={0,0xffff,0x0000,0xffff},
  yellow={0,0xffff,0xffff,0x0000},white={0,0xffff,0xffff,0xffff};
#ifdef GTK2
  GtkTextBuffer *helptextbuffer;
  GtkTextIter ht_iter;
#endif
    
  char *helpmessage = _("Click and drag the mouse to draw bonds on the canvas. \n"
  		"The right mouse button is used to delete objects - either bonds\n"
  		"or text depending on which drawing mode is active.\n\n"
		"The buttons with different ringtypes on them select drawing modes\n"
		"with preferred angles, but you can actually draw at any angle in all modes.\n\n"
		"The button with the segmented line on it lets you draw curves by marking\n"
		"control points along the curve (a cubic spline).\n"
  		"You can select the bondtype and color using the appropriate button\n"
  		"or change them later by clicking on the desired bond in Bonds mode.\n\n"
		"To draw a cyclic system, simply press the Ctrl key together with the\n"
		"number key corresponding to the number of sides for the polygon, and\n"
		"then draw one side while pressing the Ctrl button.\n\n"
  		"For drawing labels, write them into the text box in the top right of the window\n"
  		"and place them on the canvas with the mouse. You can also use a number of\n"
  		"keyboard shortcuts for common labels while in bond drawing mode:\n"
  		"Simply press the 'c' key, or n,o,p,s,f to add the element symbol at\n"
  		" the current drawing position, 1,2 and 3 for CH, CH_2 and CH_3, l for Cl,\n"
  		" * for a big dot.\n\n"
  		"The keys of the numeric keypad each insert an 'electron pair' line\n"
  		"around an atom symbol in the position corresponding to the location\n"
  		"of the key around the center of the numeric keypad.\n\n"
  		"The text mode uses the following prefixes for special text:\n"
  		"_ for subscripts, ^ for superscripts, @ for symbols (greek characters),\n"
  		"| for italic (slanted) characters and # for bold text.\n"
  		"When the text box is empty, clicking on any label in the drawing area\n"
  		"copies that label into the box for reuse.\n\n"
  		"Drawing is best done with the mouse, but you can also use the\n"
  		"cursor keys in combination with the Ctrl key for exact positioning.\n"
  		"When used with the Alt key, the cursor keys move the rectangular\n"
  		"or rhombic grid that can be projected on the drawing area.\n\n"
  		"If you need general drawing functions not provided by chemtool,\n"
  		"try exporting to fig format and editing your figure in Brian Smith's\n"
  		"xfig program. Its companion transfig/fig2dev is required by chemtool\n"
  		"for printing and for exporting to eps or LaTeX, while the fig, XBM and\n"
  		"SVG output are generated directly. Another useful and highly recommended\n"
  		"helper program is Babel - either in its original version, or in the form\n"
  		"of the new OpenBabel project. Using either version, chemtool is able to\n"
  		"import foreign data from a variety of file formats, while only molfile\n"
  		"im- and export is built into chemtool.\n\n"
  		"More help is available in the manual page for chemtool and in the file\n"
  		" README included in the source distribution as well as on the website.\n"
  		"This should normally get installed in /usr/share/doc/packages/chemtool.\n" 
		"If you find any bugs or have a question or suggestion, please contact the\n"
		"main author, martin@ruby.chemie.uni-freiburg.de");
#ifdef ENABLE_NLS
  (void) gtk_set_locale ();	/* newer glibc requires this */
  /* but we have to guard against locales that use a comma in numbers */
  (void) setlocale (LC_NUMERIC, "C");
/*@ignore@*/
  (void) bindtextdomain ("chemtool", LOCALEDIR);
/*@end@*/  

#ifdef GTK2
  bind_textdomain_codeset ("chemtool", "UTF-8");
  (void) setlocale (LC_NUMERIC, "C");
#endif  

  (void) textdomain ("chemtool");
#endif

#ifdef LIBUNDO
  undo_new ("chemtool");
  undo_set_memory_limit (65536);
#endif
  setup_data ();		/*initialize chemtool structs */
#ifdef LIBUNDO
  undo_snapshot ();
#endif

  check_fig2dev ();
  check_fig2sxd();

if ( (strstr(argv[0],"chemtoolbg") && argc <3) || (argc==2 && !strcmp(argv[1],"-bg"))) {
   fprintf(stderr,"Usage: chemtoolbg [outtype] [infile]\n");
   fprintf(stderr,"       available outtypes: fig tex eps xbm svg mol emf sxd png asy\n");
   fprintf(stderr,"       Use \"-\" instead of infile in pipes, e.g. someprogram|chemtoolbg svg -\n\n");
   exit (1);
}
if (strstr(argv[0],"chemtoolbg") || (argc>1 && !strcmp(argv[1],"-bg"))) {
  char *mode[10] = { "fig", "tex", "eps", "xbm", "svg", "mol","emf","sxd","png","asy"};
  int shift=0;
  static int k;
  if (!strcmp(argv[1],"-bg")) shift = 1;
  batchmode = 1;
  mark.flag = False;
  head.width = 2000;
  head.height = 5000;
  zoom_factor = 2;
  size_factor = 0.8;
  pdbx = pdby = pdbz = NULL;
  atcode = NULL;
  atjust = bondtype = NULL;
  bondfrom = bondto = NULL;
  hp->x = hp->y = 200;
  hp->tx = hp->ty = 200;
  tmpx=(int*)NULL;
  tmpy=(int*)NULL;
  gridtype=0;
  gridx=gridy = 0;
  atnum = 0;
  queuename = malloc (33 * sizeof (char));
  bgred=bgblue=bggreen=65535;
  readrc();
  Load_Font();
        k=2+shift;
        if (k>=argc) exit(1);
        strcpy (filename, argv[k]);
      load_mol(filename);
       head.pix_width = 1600;
         head.pix_height = 1600;
         
      strcpy(expname,filename);
      if (!strcmp(expname,"-")) 
        strcpy(expname,"chemtool");
fprintf(stderr,"chemtoolbg %s...\n",expname);
      char *dot=strrchr(expname,'.');
      if (dot) *dot='\0';
      j=-1;
      k=1+shift;
      if (k>=argc) exit(1);
fprintf(stderr,"chemtoolbg mode %s...\n",argv[k]);
      for (i=0;i<10;i++) {
        if (!strcmp(mode[i],argv[k])) {
           j=i;
           break;
        }
      }
      if (j<0) exit(1);
      strcat (expname,".");  
      strcat (expname, mode[j]);
fprintf(stderr,"chemtoolbg %s...\n",expname);
      gtk_init (&argc, &argv);	/*initialize GTK environment */
      switch(j) {
      case 0:
      drawing_area = gtk_drawing_area_new ();
      gtk_drawing_area_size (GTK_DRAWING_AREA (drawing_area), 1600, 1600);
      export_xfig(expname);
      break;
      case 1:
      drawing_area = gtk_drawing_area_new ();
      gtk_drawing_area_size (GTK_DRAWING_AREA (drawing_area), 1600, 1600);
      export_latex_pic(expname,1.);
      break;
      case 2:
      drawing_area = gtk_drawing_area_new ();
      gtk_drawing_area_size (GTK_DRAWING_AREA (drawing_area), 1600, 1600);
      snprintf(bghexcolor,10,"#%2.2x%2.2x%2.2x",(unsigned char)(bgred/256.),(unsigned char)(bggreen/256.),(unsigned char)(bgblue/256.));
      export_ps_pic(expname,1.);
      break;
      case 3:
      break;
      case 4:
      snprintf(bghexcolor,10,"#%2.2x%2.2x%2.2x",(unsigned char)(bgred/256.),(unsigned char)(bggreen/256.),(unsigned char)(bgblue/256.));
      export_svg(expname);
      break;
      case 5:
      break;
      case 6:
#ifndef EMF
      drawing_area = gtk_drawing_area_new ();
      gtk_drawing_area_size (GTK_DRAWING_AREA (drawing_area), 1600, 1600);
      if (figversion>=3)
#endif      
      export_emf(expname);
      break;
      case 7:
      drawing_area = gtk_drawing_area_new ();
      gtk_drawing_area_size (GTK_DRAWING_AREA (drawing_area), 1600, 1600);
      if (have_fig2sxd) export_sxd(expname);
      break;
      case 8:
      drawing_area = gtk_drawing_area_new ();
      gtk_drawing_area_size (GTK_DRAWING_AREA (drawing_area), 1600, 1600);
      snprintf(bghexcolor,10,"#%2.2x%2.2x%2.2x",(unsigned char)(bgred/256.),(unsigned char)(bggreen/256.),(unsigned char)(bgblue/256.));
      export_png_pic(expname,1.);
      break;
      case 9:
      drawing_area = gtk_drawing_area_new ();
      gtk_drawing_area_size (GTK_DRAWING_AREA (drawing_area), 1600, 1600);
      snprintf(bghexcolor,10,"#%2.2x%2.2x%2.2x",(unsigned char)(bgred/256.),(unsigned char)(bggreen/256.),(unsigned char)(bgblue/256.));
      export_asy(expname);
      break;
      default:
      break;
      }
      exit(0);
}  

  gtk_init (&argc, &argv);	/*initialize GTK environment */


/* 
 * Create the 'About' pop-up 
 */
  aboutw = gtk_dialog_new ();
  label =
    gtk_label_new (_
		   (" Chemtool Version 1.6.14\nby\nMartin Kroeker,\nRadek Liboska,\nMichael Banck\nand\nThomas Volk\n\nhttp://ruby.chemie.uni-freiburg.de/~martin/chemtool/chemtool.html"));
#ifdef GTK2
  gtk_label_set_justify(GTK_LABEL(label),GTK_JUSTIFY_CENTER);
#endif  
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (aboutw)->vbox), label,
		      (gboolean) TRUE, (gboolean) TRUE, 0);
  gtk_widget_show (label);
  button = gtk_button_new_with_label ("Ok");
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (aboutw)->action_area), button,
		      (gboolean) TRUE, (gboolean) TRUE, 0);
  (void)gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
			     GTK_SIGNAL_FUNC (gtk_widget_hide),
			     GTK_OBJECT (aboutw));
  (void)gtk_signal_connect_object (GTK_OBJECT (aboutw), "delete_event",
			     GTK_SIGNAL_FUNC (gtk_widget_hide),
			     GTK_OBJECT (aboutw));
  gtk_widget_show (button);


  helpw = gtk_dialog_new();
  gtk_widget_set_usize(helpw,480,500);
  textscroll=gtk_scrolled_window_new(NULL,NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(textscroll),
  			GTK_POLICY_NEVER,GTK_POLICY_ALWAYS);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (helpw)->vbox), textscroll,
		      (gboolean) TRUE, (gboolean) TRUE, 0);
#ifdef GTK2
  helptext = gtk_text_view_new();
  gtk_text_view_set_editable(GTK_TEXT_VIEW(helptext), FALSE);
  helptextbuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(helptext));
  gtk_text_buffer_get_iter_at_offset (helptextbuffer, &ht_iter, 0);
#ifndef ENABLE_NLS
  gtk_text_buffer_insert (helptextbuffer, &ht_iter, helpmessage,-1);
#else
  gtk_text_buffer_insert (helptextbuffer, &ht_iter, gettext(helpmessage),-1);
#endif
#else  
  helptext = gtk_text_new(NULL,NULL);
  gtk_text_set_editable(GTK_TEXT(helptext), FALSE);
  gtk_text_freeze(GTK_TEXT(helptext));
#ifndef ENABLE_NLS
  gtk_text_insert(GTK_TEXT(helptext),NULL,NULL,NULL,helpmessage,(gint)strlen(helpmessage));
#else
  gtk_text_insert(GTK_TEXT(helptext),NULL,NULL,NULL,gettext(helpmessage),(gint)strlen(gettext(helpmessage)));
#endif
#endif
  gtk_widget_show(textscroll);
  gtk_container_add(GTK_CONTAINER(textscroll),helptext);
  button = gtk_button_new_with_label ("Ok");
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (helpw)->action_area), button,
		      (gboolean) TRUE, (gboolean) TRUE, 0);
  (void)gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
			     GTK_SIGNAL_FUNC (gtk_widget_hide),
			     GTK_OBJECT (helpw));
  (void)gtk_signal_connect_object (GTK_OBJECT (helpw), "delete_event",
			     GTK_SIGNAL_FUNC (gtk_widget_hide),
			     GTK_OBJECT (helpw));
  gtk_widget_show (button);
  gtk_widget_show (helptext);
  gtk_widget_realize(helpw);
/*
 *  Build the file selection pop-up  
 */


  filew = gtk_file_selection_new ("File selection");
  (void)gtk_signal_connect (GTK_OBJECT (filew), "delete_event",
		      (GtkSignalFunc) gtk_grab_remove, GTK_OBJECT (filew));
  (void)gtk_signal_connect (GTK_OBJECT (filew), "delete_event",
		      (GtkSignalFunc) gtk_widget_hide, GTK_OBJECT (filew));
  (void)gtk_signal_connect (GTK_OBJECT (filew), "delete_event",
		      restore_picture, NULL);
  (void)gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (filew)->ok_button),
		      "clicked", (GtkSignalFunc) file_ok_sel, filew);

  (void)gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION
					 (filew)->cancel_button),
			     "clicked", (GtkSignalFunc) gtk_grab_remove,
			     GTK_OBJECT (filew));
  (void)gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION
					 (filew)->cancel_button),
			     "clicked", restore_picture, NULL);

  (void)gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION
					 (filew)->cancel_button),
			     "clicked", (GtkSignalFunc) gtk_widget_hide,
			     GTK_OBJECT (filew));
#ifdef GTK2
  (void)gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (filew)->file_list),
		      "cursor_changed", (GtkSignalFunc) getpreview,
		      (gpointer) filew);
#else
  (void)gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (filew)->file_list),
		      "select_row", (GtkSignalFunc) getpreview,
		      (gpointer) filew);
#endif



  preview_area = gtk_drawing_area_new ();
  gtk_drawing_area_size (GTK_DRAWING_AREA (preview_area), 200, 100);
  gtk_box_pack_start (GTK_BOX (GTK_FILE_SELECTION (filew)->action_area),
		      preview_area, (gboolean) TRUE, (gboolean) FALSE, 0);

  gtk_widget_show (preview_area);

  /* Signals used to handle backing pixmap */

  (void)gtk_signal_connect (GTK_OBJECT (preview_area), "expose_event",
		      (GtkSignalFunc) expose_event, NULL);
  (void)gtk_signal_connect (GTK_OBJECT (preview_area), "configure_event",
		      (GtkSignalFunc) configure_preview, NULL);

  pdbhbox = gtk_hbox_new ((gboolean) FALSE, 0);
  label = gtk_label_new (_("PDB labels:"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (pdbhbox), label, TRUE, TRUE, 0);
  labbutton[0] = gtk_radio_button_new_with_label (NULL, _("All"));
  gtk_box_pack_start (GTK_BOX (pdbhbox), labbutton[0], (gboolean) TRUE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect (GTK_OBJECT (labbutton[0]), "clicked",
		      GTK_SIGNAL_FUNC (pdb_mode), "0");

  gtk_widget_show (labbutton[0]);
  group = gtk_radio_button_group (GTK_RADIO_BUTTON (labbutton[0]));
  labbutton[1] = gtk_radio_button_new_with_label (group, _("non-H"));
  gtk_box_pack_start (GTK_BOX (pdbhbox), labbutton[1], (gboolean) TRUE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect (GTK_OBJECT (labbutton[1]), "clicked",
		      GTK_SIGNAL_FUNC (pdb_mode), "1");
  gtk_widget_show (labbutton[1]);
  labbutton[2] =
    gtk_radio_button_new_with_label (gtk_radio_button_group
				     (GTK_RADIO_BUTTON (labbutton[0])),
				     _("no numbers"));
  gtk_box_pack_start (GTK_BOX (pdbhbox), labbutton[2], (gboolean) TRUE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect (GTK_OBJECT (labbutton[2]), "clicked",
		      GTK_SIGNAL_FUNC (pdb_mode), "2");
  gtk_widget_show (labbutton[2]);
  labbutton[3] =
    gtk_radio_button_new_with_label (gtk_radio_button_group
				     (GTK_RADIO_BUTTON (labbutton[0])),
				     _("non H,no numbers"));
  (void)gtk_signal_connect (GTK_OBJECT (labbutton[3]), "clicked",
		      GTK_SIGNAL_FUNC (pdb_mode), "3");
  gtk_box_pack_start (GTK_BOX (pdbhbox), labbutton[3], (gboolean) TRUE,
		      (gboolean) TRUE, 0);
  gtk_widget_show (labbutton[3]);

  labbutton[4] =
    gtk_radio_button_new_with_label (gtk_radio_button_group
				     (GTK_RADIO_BUTTON (labbutton[0])),
				     _("None"));
  gtk_box_pack_start (GTK_BOX (pdbhbox), labbutton[4], (gboolean) TRUE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect (GTK_OBJECT (labbutton[4]), "clicked",
		      GTK_SIGNAL_FUNC (pdb_mode), "4");
  gtk_widget_show (labbutton[4]);

  gtk_box_pack_start (GTK_BOX (GTK_FILE_SELECTION (filew)->main_vbox),
		      pdbhbox, (gboolean) TRUE, (gboolean) TRUE, 0);
  gtk_widget_show (pdbhbox);

  sdfhbox = gtk_hbox_new ((gboolean) FALSE, 0);
  sdflabel = gtk_label_new (_("SDF entry:"));
  gtk_widget_show (sdflabel);
  gtk_box_pack_start (GTK_BOX (sdfhbox), sdflabel, TRUE, TRUE, 0);
  sdfbutton[0] = gtk_button_new_with_label (_("First"));
  gtk_box_pack_start (GTK_BOX (sdfhbox), sdfbutton[0], (gboolean) TRUE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect (GTK_OBJECT (sdfbutton[0]), "clicked",
		      GTK_SIGNAL_FUNC (sdf_mode), "0");

  gtk_widget_show (sdfbutton[0]);
  sdfbutton[1] = gtk_button_new_with_label (_("Previous"));
  gtk_box_pack_start (GTK_BOX (sdfhbox), sdfbutton[1], (gboolean) TRUE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect (GTK_OBJECT (sdfbutton[1]), "clicked",
		      GTK_SIGNAL_FUNC (sdf_mode), "1");
  gtk_widget_show (sdfbutton[1]);
  sdfbutton[2] =
    gtk_button_new_with_label (_("Next"));
  gtk_box_pack_start (GTK_BOX (sdfhbox), sdfbutton[2], (gboolean) TRUE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect (GTK_OBJECT (sdfbutton[2]), "clicked",
		      GTK_SIGNAL_FUNC (sdf_mode), "2");
  gtk_widget_show (sdfbutton[2]);
  gtk_box_pack_start (GTK_BOX (GTK_FILE_SELECTION (filew)->main_vbox),
		      sdfhbox, (gboolean) TRUE, (gboolean) TRUE, 0);
  gtk_widget_show (sdfhbox);
  check_babel();
  if (babelin>-1){
  babelcmdmenu = gtk_option_menu_new ();
  babelcmds = gtk_menu_new ();
  for (i=0;i<=babelin;i++){
  snprintf(msgtmp,100,"%s ( %s )",intype[i],inmode[i]);
  babelcmditem = gtk_menu_item_new_with_label (msgtmp);
  gtk_menu_append (GTK_MENU (babelcmds), babelcmditem);
  gtk_widget_show (babelcmditem);
  (void)gtk_signal_connect_object (GTK_OBJECT (babelcmditem), "activate",
			     GTK_SIGNAL_FUNC (gtk_menu_item_select),
			     (gpointer) babelcmditem);
  (void)gtk_signal_connect (GTK_OBJECT (babelcmditem), "activate",
		      GTK_SIGNAL_FUNC (babelcmd), inmode[i]);
  }
  gtk_option_menu_set_menu (GTK_OPTION_MENU (babelcmdmenu), babelcmds);
  gtk_widget_show (babelcmdmenu);
  gtk_box_pack_start(GTK_BOX(GTK_FILE_SELECTION(filew)->main_vbox),
  			babelcmdmenu, (gboolean)TRUE,(gboolean)TRUE,0);
  }			
  if (babelout>-1){
  babelexpmenu = gtk_option_menu_new ();
  babelexpcmds = gtk_menu_new ();
  for (i=0;i<=babelout;i++){
  snprintf(msgtmp,100,"%s ( %s )",outtype[i],outmode[i]);
  babelexpcmditem = gtk_menu_item_new_with_label (msgtmp);
  gtk_menu_append (GTK_MENU (babelexpcmds), babelexpcmditem);
  gtk_widget_show (babelexpcmditem);
  (void)gtk_signal_connect_object (GTK_OBJECT (babelexpcmditem), "activate",
			     GTK_SIGNAL_FUNC (gtk_menu_item_select),
			     (gpointer) babelexpcmditem);
  (void)gtk_signal_connect (GTK_OBJECT (babelexpcmditem), "activate",
		      GTK_SIGNAL_FUNC (babelcmd), outmode[i]);
  }
  gtk_option_menu_set_menu (GTK_OPTION_MENU (babelexpmenu), babelexpcmds);
  gtk_widget_show (babelexpmenu);
  gtk_box_pack_start(GTK_BOX(GTK_FILE_SELECTION(filew)->main_vbox),
  			babelexpmenu, (gboolean)TRUE,(gboolean)TRUE,0);
  }			
  gtk_widget_realize (filew);
/*
 *  Build the export pop-up  
 */


  expw = gtk_file_selection_new ("Export");
  (void)gtk_signal_connect (GTK_OBJECT (expw), "delete_event",
		      (GtkSignalFunc) gtk_grab_remove, GTK_OBJECT (expw));
  (void)gtk_signal_connect (GTK_OBJECT (expw), "delete_event",
		      (GtkSignalFunc) gtk_widget_hide, GTK_OBJECT (expw));
  (void)gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (expw)->ok_button),
		      "clicked", (GtkSignalFunc) do_export, expw);

  (void)gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION
					 (expw)->cancel_button),
			     "clicked", (GtkSignalFunc) gtk_grab_remove,
			     GTK_OBJECT (expw));
  (void)gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION
					 (expw)->cancel_button),
			     "clicked", (GtkSignalFunc) gtk_widget_hide,
			     GTK_OBJECT (expw));
  hbox = gtk_hbox_new (FALSE, 0);
  label = gtk_label_new (_("Latex / EPS scale factor :"));
  gtk_box_pack_start (GTK_BOX (hbox), label, (gboolean) TRUE,
		      (gboolean) FALSE, 0);
  gtk_widget_show (label);
  adj_scale = (GtkAdjustment *) gtk_adjustment_new (100., 1., 200., 1., 10., 0.);
  scale = gtk_spin_button_new (adj_scale, 0., 0);

  gtk_box_pack_start (GTK_BOX (hbox), scale, (gboolean) FALSE,
		      (gboolean) FALSE, 0);
  gtk_widget_show (scale);
  label = gtk_label_new ("%");
  gtk_box_pack_start (GTK_BOX (hbox), label, (gboolean) FALSE,
		      (gboolean) FALSE, 0);
  gtk_widget_show (label);
  label = gtk_label_new ("");
  gtk_box_pack_start (GTK_BOX (hbox), label, (gboolean) TRUE,
		      (gboolean) FALSE, 0);
  gtk_widget_show (label);

  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (GTK_FILE_SELECTION (expw)->main_vbox), hbox,
		      (gboolean) TRUE, (gboolean) TRUE, 0);
  hbox = gtk_hbox_new ((gboolean) FALSE, 0);
  expbutton[0] = gtk_radio_button_new_with_label (NULL, "XFig");
  gtk_box_pack_start (GTK_BOX (hbox), expbutton[0], (gboolean) TRUE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect (GTK_OBJECT (expbutton[0]), "clicked",
		      GTK_SIGNAL_FUNC (exp_mode), "0");

  gtk_widget_show (expbutton[0]);
  group = gtk_radio_button_group (GTK_RADIO_BUTTON (expbutton[0]));
  expbutton[1] = gtk_radio_button_new_with_label (group, "LaTeX");
  gtk_box_pack_start (GTK_BOX (hbox), expbutton[1], (gboolean) TRUE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect (GTK_OBJECT (expbutton[1]), "clicked",
		      GTK_SIGNAL_FUNC (exp_mode), "1");
  gtk_widget_show (expbutton[1]);
  expbutton[2] =
    gtk_radio_button_new_with_label (gtk_radio_button_group
				     (GTK_RADIO_BUTTON (expbutton[0])),
				     "EPS");
  gtk_box_pack_start (GTK_BOX (hbox), expbutton[2], (gboolean) TRUE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect (GTK_OBJECT (expbutton[2]), "clicked",
		      GTK_SIGNAL_FUNC (exp_mode), "2");
  gtk_widget_show (expbutton[2]);
  expbutton[3] =
    gtk_radio_button_new_with_label (gtk_radio_button_group
				     (GTK_RADIO_BUTTON (expbutton[0])),
				     "X Bitmap");
  (void)gtk_signal_connect (GTK_OBJECT (expbutton[3]), "clicked",
		      GTK_SIGNAL_FUNC (exp_mode), "3");
  gtk_box_pack_start (GTK_BOX (hbox), expbutton[3], (gboolean) TRUE,
		      (gboolean) TRUE, 0);
  gtk_widget_show (expbutton[3]);
  expbutton[4] =
    gtk_radio_button_new_with_label (gtk_radio_button_group
				     (GTK_RADIO_BUTTON (expbutton[0])),
				     "SVG");
  (void)gtk_signal_connect (GTK_OBJECT (expbutton[4]), "clicked",
		      GTK_SIGNAL_FUNC (exp_mode), "4");
  gtk_box_pack_start (GTK_BOX (hbox), expbutton[4], (gboolean) TRUE,
		      (gboolean) TRUE, 0);
  gtk_widget_show (expbutton[4]);
  expbutton[5] =
    gtk_radio_button_new_with_label (gtk_radio_button_group
				     (GTK_RADIO_BUTTON (expbutton[0])),
				     "MOL");
  (void)gtk_signal_connect (GTK_OBJECT (expbutton[5]), "clicked",
		      GTK_SIGNAL_FUNC (exp_mode), "5");
  gtk_box_pack_start (GTK_BOX (hbox), expbutton[5], (gboolean) TRUE,
		      (gboolean) TRUE, 0);
  gtk_widget_show (expbutton[5]);
  expbutton[6] =
    gtk_radio_button_new_with_label (gtk_radio_button_group
				     (GTK_RADIO_BUTTON (expbutton[0])),
				     "EMF");
  (void)gtk_signal_connect (GTK_OBJECT (expbutton[6]), "clicked",
		      GTK_SIGNAL_FUNC (exp_mode), "6");
  gtk_box_pack_start (GTK_BOX (hbox), expbutton[6], (gboolean) TRUE,
		      (gboolean) TRUE, 0);
#ifdef EMF
  gtk_widget_show (expbutton[6]);
#endif
  expbutton[7] =
    gtk_radio_button_new_with_label (gtk_radio_button_group
				     (GTK_RADIO_BUTTON (expbutton[0])),
				     "SXD");
  (void)gtk_signal_connect (GTK_OBJECT (expbutton[7]), "clicked",
		      GTK_SIGNAL_FUNC (exp_mode), "7");
  gtk_box_pack_start (GTK_BOX (hbox), expbutton[7], (gboolean) TRUE,
		      (gboolean) TRUE, 0);
  expbutton[8] =
    gtk_radio_button_new_with_label (gtk_radio_button_group
				     (GTK_RADIO_BUTTON (expbutton[0])),
				     "PNG");
  (void)gtk_signal_connect (GTK_OBJECT (expbutton[8]), "clicked",
		      GTK_SIGNAL_FUNC (exp_mode), "8");
  gtk_box_pack_start (GTK_BOX (hbox), expbutton[8], (gboolean) TRUE,
		      (gboolean) TRUE, 0);
  gtk_widget_show (expbutton[8]);
  expbutton[9] =
    gtk_radio_button_new_with_label (gtk_radio_button_group
				     (GTK_RADIO_BUTTON (expbutton[0])),
				     "ASY");
  (void)gtk_signal_connect (GTK_OBJECT (expbutton[9]), "clicked",
		      GTK_SIGNAL_FUNC (exp_mode), "9");
  gtk_box_pack_start (GTK_BOX (hbox), expbutton[9], (gboolean) TRUE,
		      (gboolean) TRUE, 0);
  gtk_widget_show (expbutton[9]);
  gtk_box_pack_start (GTK_BOX (GTK_FILE_SELECTION (expw)->main_vbox), hbox,
		      (gboolean) TRUE, (gboolean) TRUE, 0);
  gtk_widget_show (hbox);
/********************************************************************/

/* 
 * Build the (error) message pop-up  
 */

  messagew = gtk_dialog_new ();
  (void)gtk_signal_connect_object (GTK_OBJECT (messagew), "destroy",
			     (GtkSignalFunc) gtk_grab_remove,
			     GTK_OBJECT (messagew));

  (void)gtk_signal_connect_object (GTK_OBJECT (messagew), "destroy",
			     (GtkSignalFunc) gtk_widget_hide,
			     GTK_OBJECT (messagew));

  message = gtk_label_new (_("Unknown error"));
  gtk_misc_set_padding (GTK_MISC (message), 10, 10);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (messagew)->vbox), message,
		      (gboolean) TRUE, (gboolean) TRUE, 0);

  gtk_widget_show (message);

  button = gtk_button_new_with_label ("OK");
  /* Connect the ok_button */
  (void)gtk_signal_connect_object (GTK_OBJECT (button),
			     "clicked", (GtkSignalFunc) gtk_grab_remove,
			     GTK_OBJECT (messagew));
  (void)gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
			     (GtkSignalFunc) gtk_widget_hide,
			     GTK_OBJECT (messagew));
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (messagew)->action_area), button,
		      (gboolean) TRUE, (gboolean) TRUE, 0);
  gtk_widget_show (button);
/********************************************************************/

/* Configurable options dialog */

#if (GTK_MINOR_VERSION >2)
  printer_dialog = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (printer_dialog), _("Configurable options"));
  gtk_window_set_resizable (GTK_WINDOW (printer_dialog), FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (printer_dialog), 12);
  master_vbox = gtk_vbox_new (FALSE, 12);
  gtk_container_add (GTK_CONTAINER (printer_dialog), master_vbox);
  
  hbox = gtk_hbutton_box_new ();
  gtk_button_box_set_layout (GTK_BUTTON_BOX (hbox), GTK_BUTTONBOX_END);
  gtk_box_set_spacing (GTK_BOX (hbox), 12);
  gtk_box_pack_end (GTK_BOX (master_vbox), hbox, FALSE, FALSE, 0);
  
  button = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
  gtk_container_add (GTK_CONTAINER (hbox), button);
  
  /* Thanks to using ...swapped, the callback_data gets passed as
     the first argument to the callback function, so when we click
     on the button, we hide the dialog, not the button. */
  
  g_signal_connect_swapped (G_OBJECT (button), "clicked",
      G_CALLBACK (gtk_widget_hide), G_OBJECT (printer_dialog));
  
  button = gtk_button_new_from_stock (GTK_STOCK_OK);
  gtk_container_add (GTK_CONTAINER (hbox), button);
  g_signal_connect (G_OBJECT (button), "clicked",
      G_CALLBACK (options_dialog_ok), NULL);
  
  notebook = gtk_notebook_new ();
  gtk_box_pack_end (GTK_BOX (master_vbox), notebook, TRUE, TRUE, 0);
  
  /* First page: General options */
  
  table = gtk_table_new (10, 3, FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 12);
  
  label = gtk_label_new (NULL);
  gtk_label_set_text_with_mnemonic (GTK_LABEL (label), _("_General"));
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), table, label);
  
  /* Adjust the table spacing before section headings etc. */
  
  gtk_table_set_col_spacing (GTK_TABLE (table), 1, 12);
  gtk_table_set_row_spacings (GTK_TABLE (table), 6);
  gtk_table_set_row_spacing (GTK_TABLE (table), 0, 12);
  gtk_table_set_row_spacing (GTK_TABLE (table), 3, 12);
  gtk_table_set_row_spacing (GTK_TABLE (table), 7, 12);
  gtk_table_set_row_spacing (GTK_TABLE (table), 2, 18);
  gtk_table_set_row_spacing (GTK_TABLE (table), 6, 18);
  
  /* Spacer label for left indent */
  
  label = gtk_label_new (NULL);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2, 0, 0, 6, 0);
  
  /* The section headings */
  
  label = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL (label), _("<b>Display</b>"));
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
  gtk_table_attach (GTK_TABLE (table), label, 0, 3, 0, 1, GTK_EXPAND|GTK_FILL,
      GTK_FILL, 0, 0);
  
  label = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL (label), _("<b>Saving and Exporting</b>"));
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
  gtk_table_attach (GTK_TABLE (table), label, 0, 3, 3, 4, GTK_EXPAND|GTK_FILL,
      GTK_FILL, 0, 0);
  
  label = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL (label), _("<b>Other</b>"));
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
  gtk_table_attach (GTK_TABLE (table), label, 0, 3, 8, 9, GTK_EXPAND|GTK_FILL,
      GTK_FILL, 0, 0);
  
  /* First section, "Display" */
  
  hbox = gtk_hbox_new (FALSE, 12);
  gtk_table_attach (GTK_TABLE (table), hbox, 1, 3, 1, 2, GTK_EXPAND|GTK_FILL,
      GTK_FILL, 0, 0);
  label = gtk_label_new (NULL);
  gtk_label_set_text_with_mnemonic (GTK_LABEL (label), _("_Background color:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  
  color_button = gtk_color_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox), color_button, FALSE, FALSE, 0);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), color_button);
  
  whiteout_check = gtk_check_button_new_with_mnemonic (
      _("Add filled _white rectangle under labels"));
  gtk_table_attach (GTK_TABLE (table), whiteout_check, 1, 3, 2, 3,
      GTK_EXPAND|GTK_FILL, GTK_FILL, 0, 0);
  
  /* Second section, "Saving and exporting" */
  
  datadir_entry = gtk_entry_new ();
  gtk_table_attach (GTK_TABLE (table), datadir_entry, 2, 3, 4, 5,
      GTK_EXPAND|GTK_FILL, GTK_FILL, 0, 0);
  datamask_entry = gtk_entry_new ();
  gtk_table_attach (GTK_TABLE (table), datamask_entry, 2, 3, 5, 6,
      GTK_EXPAND|GTK_FILL, GTK_FILL, 0, 0);
  
  eps_preview_combo = gtk_combo_box_new_text ();
  gtk_combo_box_append_text (GTK_COMBO_BOX (eps_preview_combo), _("None"));
  gtk_combo_box_append_text (GTK_COMBO_BOX (eps_preview_combo), _("EPSI"));
  gtk_combo_box_append_text (GTK_COMBO_BOX (eps_preview_combo), _("TIFF mono"));
  gtk_combo_box_append_text (GTK_COMBO_BOX (eps_preview_combo), _("TIFF color"));
  gtk_table_attach (GTK_TABLE (table), eps_preview_combo, 2, 3, 6, 7,
      GTK_EXPAND|GTK_FILL, GTK_FILL, 0, 0);
  
  label = gtk_label_new (NULL);
  gtk_label_set_text_with_mnemonic (GTK_LABEL (label), _("_Data directory:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), datadir_entry);
  gtk_table_attach (GTK_TABLE (table), label, 1, 2, 4, 5, GTK_EXPAND|GTK_FILL,
      GTK_FILL, 0, 0);
  
  label = gtk_label_new (NULL);
  gtk_label_set_text_with_mnemonic (GTK_LABEL (label), _("E_xtension:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), datamask_entry);
  gtk_table_attach (GTK_TABLE (table), label, 1, 2, 5, 6, GTK_EXPAND|GTK_FILL,
      GTK_FILL, 0, 0);
  
  label = gtk_label_new (NULL);
  gtk_label_set_text_with_mnemonic (GTK_LABEL (label),
      _("Preview image to add to _EPS files:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), eps_preview_combo);
  gtk_table_attach (GTK_TABLE (table), label, 1, 2, 6, 7, GTK_EXPAND|GTK_FILL,
      GTK_FILL, 0, 0);

  intl_check = gtk_check_button_new_with_mnemonic (
      _("Support national character sets in labels"));
  gtk_table_attach (GTK_TABLE (table), intl_check, 1, 3, 7, 8,
      GTK_EXPAND|GTK_FILL, GTK_FILL, 0, 0);
  
  /* Third section, "Other" */
  
  bond_length_entry = gtk_entry_new ();
  gtk_table_attach (GTK_TABLE (table), bond_length_entry, 2, 3, 9, 10,
      GTK_EXPAND|GTK_FILL, GTK_FILL, 0, 0);
  
  hbox = gtk_hbox_new (FALSE, 6);
  gtk_table_attach (GTK_TABLE (table), hbox, 2, 3, 10, 11, GTK_EXPAND|GTK_FILL,
      GTK_FILL, 0, 0);
  dbond_dist_spin = gtk_spin_button_new_with_range (1, 100, 1);
  gtk_box_pack_start (GTK_BOX (hbox), dbond_dist_spin, FALSE, FALSE, 0);
  label = gtk_label_new ("px");
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  
  label = gtk_label_new (NULL);
  gtk_label_set_text_with_mnemonic (GTK_LABEL (label), _("Base bond _length:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), bond_length_entry);
  gtk_table_attach (GTK_TABLE (table), label, 1, 2, 9, 10, GTK_EXPAND|GTK_FILL,
      GTK_FILL, 0, 0);
  
  label = gtk_label_new (NULL);
  gtk_label_set_text_with_mnemonic (GTK_LABEL (label),
      _("Double bond _separation:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), dbond_dist_spin);
  gtk_table_attach (GTK_TABLE (table), label, 1, 2, 10, 11, GTK_EXPAND|GTK_FILL,
      GTK_FILL, 0, 0);
  
  /* Second page, printing */
  
  table = gtk_table_new (5, 2, FALSE);

  gtk_container_set_border_width (GTK_CONTAINER (table), 12);
  
  label = gtk_label_new (NULL);
  gtk_label_set_text_with_mnemonic (GTK_LABEL (label), _("_Printing"));
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), table, label);
  
  /* Table spacings */
  
  gtk_table_set_col_spacing (GTK_TABLE (table), 0, 12);
  gtk_table_set_row_spacings (GTK_TABLE (table), 6);
  
  /* No sections are necessary on this page */
  
  print_cmd_combo = gtk_combo_box_new_text ();
  gtk_combo_box_append_text (GTK_COMBO_BOX (print_cmd_combo), "lpr");
  gtk_combo_box_append_text (GTK_COMBO_BOX (print_cmd_combo), "lp");
  gtk_combo_box_append_text (GTK_COMBO_BOX (print_cmd_combo), "kprinter");
  gtk_combo_box_append_text (GTK_COMBO_BOX (print_cmd_combo), "gtklp");
  gtk_table_attach (GTK_TABLE (table), print_cmd_combo, 1, 2, 0, 1,
      GTK_EXPAND|GTK_FILL, GTK_FILL, 0, 0);
  
  printer_name_entry = gtk_entry_new ();
  gtk_table_attach (GTK_TABLE (table), printer_name_entry,
      1, 2, 1, 2, GTK_EXPAND|GTK_FILL, GTK_FILL, 0, 0);
  
  paper_size_combo = gtk_combo_box_new_text ();
  for (i = 0; i < 11; i++)
    {
      gtk_combo_box_append_text (GTK_COMBO_BOX (paper_size_combo), paper[i]);
    }
  gtk_table_attach (GTK_TABLE (table), paper_size_combo, 1, 2, 2, 3,
      GTK_EXPAND|GTK_FILL, GTK_FILL, 0, 0);
  
  orientation_combo = gtk_combo_box_new_text ();
  gtk_combo_box_append_text (GTK_COMBO_BOX (orientation_combo), _("Portrait"));
  gtk_combo_box_append_text (GTK_COMBO_BOX (orientation_combo), _("Landscape"));
  gtk_table_attach (GTK_TABLE (table), orientation_combo, 1, 2, 3, 4,
      GTK_EXPAND|GTK_FILL, GTK_FILL, 0, 0);
  
  hbox = gtk_hbox_new (FALSE, 6);
  gtk_table_attach (GTK_TABLE (table), hbox, 1, 2, 4, 5, GTK_EXPAND|GTK_FILL,
      GTK_FILL, 0, 0);
  print_scale_spin = gtk_spin_button_new_with_range (1, 200, 1);
  gtk_box_pack_start (GTK_BOX (hbox), print_scale_spin, FALSE, FALSE, 0);
  label = gtk_label_new ("%");
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  
  label = gtk_label_new (NULL);
  gtk_label_set_text_with_mnemonic (GTK_LABEL (label), _("Print _command:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), print_cmd_combo);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1, GTK_EXPAND|GTK_FILL,
      GTK_FILL, 0, 0);
  
  label = gtk_label_new (NULL);
  gtk_label_set_text_with_mnemonic (GTK_LABEL (label), _("Printer _name:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), printer_name_entry);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2, GTK_EXPAND|GTK_FILL,
      GTK_FILL, 0, 0);
  
  label = gtk_label_new (NULL);
  gtk_label_set_text_with_mnemonic (GTK_LABEL (label), _("Paper si_ze:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), paper_size_combo);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3, GTK_EXPAND|GTK_FILL,
      GTK_FILL, 0, 0);
  
  label = gtk_label_new (NULL);
  gtk_label_set_text_with_mnemonic (GTK_LABEL (label), _("_Orientation:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), orientation_combo);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 3, 4, GTK_EXPAND|GTK_FILL,
      GTK_FILL, 0, 0);
  
  label = gtk_label_new (NULL);
  gtk_label_set_text_with_mnemonic (GTK_LABEL (label), _("Print sc_ale factor:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), print_scale_spin);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 4, 5, GTK_EXPAND|GTK_FILL,
      GTK_FILL, 0, 0);
  
  gtk_widget_show_all (GTK_WIDGET (master_vbox));
  
  (void)gtk_signal_connect_object (GTK_OBJECT (printer_dialog), "delete_event",
			     (GtkSignalFunc) gtk_widget_hide,
			     GTK_OBJECT (printer_dialog));

#else
/* old-style configuration menu for gtk1 builds */

/* Color selection dialog */
  colorseldialog= gtk_color_selection_dialog_new(_("Select background color"));
  
  colorsel=GTK_COLOR_SELECTION_DIALOG(colorseldialog)->colorsel;
  
  (void)gtk_signal_connect(GTK_OBJECT(colorsel), "color_changed", 
  		(GtkSignalFunc)set_bgcolor,(gpointer)colorsel);
  (void)gtk_signal_connect_object(GTK_OBJECT(GTK_COLOR_SELECTION_DIALOG(colorseldialog)->ok_button), "clicked", 
  		(GtkSignalFunc)gtk_widget_hide,GTK_OBJECT(colorseldialog));
  (void)gtk_signal_connect_object(GTK_OBJECT(GTK_COLOR_SELECTION_DIALOG(colorseldialog)->cancel_button), "clicked", 
  		(GtkSignalFunc)gtk_widget_hide,GTK_OBJECT(colorseldialog));
  gtk_widget_hide(GTK_COLOR_SELECTION_DIALOG(colorseldialog)->help_button);
  gtk_widget_realize(colorseldialog);
/********************************************************************/
/* Printer setup dialog */

  printer_dialog = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (printer_dialog),
			_("Configurable options"));
  gtk_widget_realize (printer_dialog);
  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (printer_dialog), vbox);
  gtk_widget_show (vbox);
  hbox = gtk_hbox_new (FALSE, 10);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);
  gtk_widget_show (hbox);
  label = gtk_label_new (_("Paper size:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);
  papermenu = gtk_option_menu_new ();
  papersizes = gtk_menu_new ();
  for (i = 0; i < 11; i++)
    {
      papersizeitem[i] = gtk_menu_item_new_with_label (paper[i]);
      gtk_menu_append (GTK_MENU (papersizes), papersizeitem[i]);
      gtk_widget_show (papersizeitem[i]);
      (void)gtk_signal_connect_object (GTK_OBJECT (papersizeitem[i]), "activate",
				 GTK_SIGNAL_FUNC (gtk_menu_item_select),
				 (gpointer) papersizeitem[i]);
      (void)gtk_signal_connect (GTK_OBJECT (papersizeitem[i]), "activate",
			  GTK_SIGNAL_FUNC (newpaper), (gpointer) paper[i]);
    }
  gtk_option_menu_set_menu (GTK_OPTION_MENU (papermenu), papersizes);
  gtk_widget_show (papermenu);
  gtk_box_pack_start (GTK_BOX (hbox), papermenu,
		      (gboolean) TRUE, (gboolean) TRUE, 0);

  hbox = gtk_hbox_new (FALSE, 10);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);
  gtk_widget_show (hbox);
  label = gtk_label_new (_("Orientation:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);
  orientmenu = gtk_option_menu_new ();
  orientations = gtk_menu_new ();
  orientationitem = gtk_menu_item_new_with_label (_("Portrait"));
  gtk_menu_append (GTK_MENU (orientations), orientationitem);
  gtk_widget_show (orientationitem);
  (void)gtk_signal_connect (GTK_OBJECT (orientationitem), "activate",
		      GTK_SIGNAL_FUNC (neworient), "0");
  orientationitem = gtk_menu_item_new_with_label (_("Landscape"));
  gtk_menu_append (GTK_MENU (orientations), orientationitem);
  gtk_widget_show (orientationitem);
  (void)gtk_signal_connect (GTK_OBJECT (orientationitem), "activate",
		      GTK_SIGNAL_FUNC (neworient), "1");
  gtk_option_menu_set_menu (GTK_OPTION_MENU (orientmenu), orientations);
  gtk_widget_show (orientmenu);
  gtk_box_pack_start (GTK_BOX (hbox), orientmenu,
		      (gboolean) TRUE, (gboolean) TRUE, 0);

  hbox = gtk_hbox_new (FALSE, 10);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);
  gtk_widget_show (hbox);

  label = gtk_label_new (_("Print scale factor :"));
  gtk_box_pack_start (GTK_BOX (hbox), label, (gboolean) FALSE,
		      (gboolean) FALSE, 0);
  gtk_widget_show (label);

  adj_pscale = (GtkAdjustment *) gtk_adjustment_new (70., 1., 200., 1., 10., 0.);
  pscale = gtk_spin_button_new (adj_pscale, 0., 0);

  gtk_box_pack_start (GTK_BOX (hbox), pscale, (gboolean) FALSE,
		      (gboolean) FALSE, 0);
  gtk_widget_show (pscale);
  label = gtk_label_new ("%");
  gtk_box_pack_start (GTK_BOX (hbox), label, (gboolean) FALSE,
		      (gboolean) FALSE, 0);
  gtk_widget_show (label);
  label = gtk_label_new ("");
  gtk_box_pack_start (GTK_BOX (hbox), label, (gboolean) TRUE,
		      (gboolean) FALSE, 0);
  gtk_widget_show (label);


  hbox = gtk_hbox_new (FALSE, 10);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);
  gtk_widget_show (hbox);
  label = gtk_label_new (_("Print command:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);
  printcmdmenu = gtk_option_menu_new ();
  printcmds = gtk_menu_new ();
  printcmditem = gtk_menu_item_new_with_label ("lpr");
  gtk_menu_append (GTK_MENU (printcmds), printcmditem);
  gtk_widget_show (printcmditem);
  (void)gtk_signal_connect_object (GTK_OBJECT (printcmditem), "activate",
			     GTK_SIGNAL_FUNC (gtk_menu_item_select),
			     (gpointer) printcmditem);
  (void)gtk_signal_connect (GTK_OBJECT (printcmditem), "activate",
		      GTK_SIGNAL_FUNC (newprcmd), "0");
  printcmditem = gtk_menu_item_new_with_label ("lp");
  gtk_menu_append (GTK_MENU (printcmds), printcmditem);
  gtk_widget_show (printcmditem);
  (void)gtk_signal_connect_object (GTK_OBJECT (printcmditem), "activate",
			     GTK_SIGNAL_FUNC (gtk_menu_item_select),
			     (gpointer) printcmditem);

  (void)gtk_signal_connect (GTK_OBJECT (printcmditem), "activate",
		      GTK_SIGNAL_FUNC (newprcmd), "1");
  printcmditem = gtk_menu_item_new_with_label ("kprinter");
  gtk_menu_append (GTK_MENU (printcmds), printcmditem);
  gtk_widget_show (printcmditem);
  (void)gtk_signal_connect_object (GTK_OBJECT (printcmditem), "activate",
			     GTK_SIGNAL_FUNC (gtk_menu_item_select),
			     (gpointer) printcmditem);

  (void)gtk_signal_connect (GTK_OBJECT (printcmditem), "activate",
		      GTK_SIGNAL_FUNC (newprcmd), "2");

  printcmditem = gtk_menu_item_new_with_label ("gtklp");
  gtk_menu_append (GTK_MENU (printcmds), printcmditem);
  gtk_widget_show (printcmditem);
  (void)gtk_signal_connect_object (GTK_OBJECT (printcmditem), "activate",
			     GTK_SIGNAL_FUNC (gtk_menu_item_select),
			     (gpointer) printcmditem);

  (void)gtk_signal_connect (GTK_OBJECT (printcmditem), "activate",
		      GTK_SIGNAL_FUNC (newprcmd), "3");
  gtk_option_menu_set_menu (GTK_OPTION_MENU (printcmdmenu), printcmds);
  gtk_widget_show (printcmdmenu);
  gtk_box_pack_start (GTK_BOX (hbox), printcmdmenu,
		      (gboolean) TRUE, (gboolean) TRUE, 0);

  hbox = gtk_hbox_new (FALSE, 10);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);
  gtk_widget_show (hbox);
  label = gtk_label_new (_("Preview image to add to eps files :"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);
  epsoptionmenu = gtk_option_menu_new ();
  epsoptions = gtk_menu_new ();
  epsoptitem = gtk_menu_item_new_with_label (_("None"));
  gtk_menu_append (GTK_MENU (epsoptions), epsoptitem);
  gtk_widget_show (epsoptitem);
  (void)gtk_signal_connect_object (GTK_OBJECT (epsoptitem), "activate",
			     GTK_SIGNAL_FUNC (gtk_menu_item_select),
			     (gpointer) epsoptitem);
  (void)gtk_signal_connect (GTK_OBJECT (epsoptitem), "activate",
		      GTK_SIGNAL_FUNC (newepsopt), "0");

  epsoptitem = gtk_menu_item_new_with_label (_("EPSI"));
  gtk_menu_append (GTK_MENU (epsoptions), epsoptitem);
  gtk_widget_show (epsoptitem);
  (void)gtk_signal_connect_object (GTK_OBJECT (epsoptitem), "activate",
			     GTK_SIGNAL_FUNC (gtk_menu_item_select),
			     (gpointer) epsoptitem);
  (void)gtk_signal_connect (GTK_OBJECT (epsoptitem), "activate",
		      GTK_SIGNAL_FUNC (newepsopt), "1");
  
  epsoptitem = gtk_menu_item_new_with_label (_("TIFF mono"));
  gtk_menu_append (GTK_MENU (epsoptions), epsoptitem);
  gtk_widget_show (epsoptitem);
  (void)gtk_signal_connect_object (GTK_OBJECT (epsoptitem), "activate",
			     GTK_SIGNAL_FUNC (gtk_menu_item_select),
			     (gpointer) epsoptitem);
  (void)gtk_signal_connect (GTK_OBJECT (epsoptitem), "activate",
		      GTK_SIGNAL_FUNC (newepsopt), "2");
  epsoptitem = gtk_menu_item_new_with_label (_("TIFF color"));
  gtk_menu_append (GTK_MENU (epsoptions), epsoptitem);
  gtk_widget_show (epsoptitem);
  (void)gtk_signal_connect_object (GTK_OBJECT (epsoptitem), "activate",
			     GTK_SIGNAL_FUNC (gtk_menu_item_select),
			     (gpointer) epsoptitem);

  (void)gtk_signal_connect (GTK_OBJECT (epsoptitem), "activate",
		      GTK_SIGNAL_FUNC (newepsopt), "2");
		      
  gtk_option_menu_set_menu (GTK_OPTION_MENU (epsoptionmenu), epsoptions);
  gtk_widget_show (epsoptionmenu);
  gtk_box_pack_start (GTK_BOX (hbox), epsoptionmenu,
		      (gboolean) TRUE, (gboolean) TRUE, 0);

  hbox = gtk_hbox_new (FALSE, 10);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);
  gtk_widget_show (hbox);
  label = gtk_label_new (_("Support national character sets in labels"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);
  intlbutton = gtk_check_button_new();
  gtk_box_pack_start (GTK_BOX (hbox), intlbutton, FALSE, FALSE, 0);
  (void)gtk_signal_connect (GTK_OBJECT(intlbutton), "toggled",
  			GTK_SIGNAL_FUNC(toggle_intlchars), NULL); 
  gtk_widget_show (intlbutton);
  
  hbox = gtk_hbox_new (FALSE, 10);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);
  gtk_widget_show (hbox);
  snprintf(bghexcolor,10,"#%2.2x%2.2x%2.2x",(unsigned char)(bgred/256.),(unsigned char)(bggreen/256.),(unsigned char)(bgblue/256.));
  label = gtk_label_new (_("Background color :"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);
  bgcolorbutton = gtk_button_new_with_label(bghexcolor);
  gtk_box_pack_start (GTK_BOX (hbox), bgcolorbutton, FALSE, FALSE, 0);
  (void)gtk_signal_connect_object (GTK_OBJECT(bgcolorbutton), "clicked",
  			GTK_SIGNAL_FUNC(gtk_widget_show), 
  			GTK_OBJECT(colorseldialog)); 
  gtk_widget_show (bgcolorbutton);

  hbox = gtk_hbox_new (FALSE, 10);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);
  gtk_widget_show (hbox);
  label = gtk_label_new (_("Add filled white rectangle under labels"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);
  whiteoutbutton = gtk_check_button_new();
  gtk_box_pack_start (GTK_BOX (hbox), whiteoutbutton, FALSE, FALSE, 0);
  (void)gtk_signal_connect (GTK_OBJECT(whiteoutbutton), "toggled",
  			GTK_SIGNAL_FUNC(toggle_whiteout), NULL); 
  gtk_widget_show (whiteoutbutton);
  
  hbox = gtk_hbox_new (FALSE, 10);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);
  gtk_widget_show (hbox);
  label = gtk_label_new (_("Printer name:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  prqueue = gtk_entry_new_with_max_length (32);
  gtk_widget_show (prqueue);
  gtk_box_pack_start (GTK_BOX (hbox), prqueue,
		      (gboolean) TRUE, (gboolean) TRUE, 0);

  hbox = gtk_hbox_new (FALSE, 10);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);
  gtk_widget_show (hbox);
  label = gtk_label_new (_("Data directory:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  defaultdir = gtk_entry_new_with_max_length ((guint16)PATH_MAX);
  gtk_widget_show (defaultdir);
  gtk_box_pack_start (GTK_BOX (hbox), defaultdir,
		      (gboolean) TRUE, (gboolean) TRUE, 0);

  hbox = gtk_hbox_new (FALSE, 10);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);
  gtk_widget_show (hbox);
  label = gtk_label_new (_("Extension:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  defaultext = gtk_entry_new_with_max_length (32);
  gtk_widget_show (defaultext);
  gtk_box_pack_start (GTK_BOX (hbox), defaultext,
		      (gboolean) TRUE, (gboolean) TRUE, 0);

  hbox = gtk_hbox_new (FALSE, 10);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);
  gtk_widget_show (hbox);
  label = gtk_label_new (_("Base bondlength (10.668mm) :"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);
  base_bondlen = gtk_entry_new_with_max_length (32);
      snprintf(msgtmp,100,"%6.4f",bondlen_mm);
      gtk_entry_set_text (GTK_ENTRY (base_bondlen),msgtmp);
  gtk_widget_show (base_bondlen);
  gtk_box_pack_start (GTK_BOX (hbox), base_bondlen,
		      (gboolean) TRUE, (gboolean) TRUE, 0);

  hbox = gtk_hbox_new (FALSE, 10);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);
  gtk_widget_show (hbox);
  label = gtk_label_new (_("Doublebond separation (4 pixel) :"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);
  doubledist = gtk_entry_new_with_max_length (32);
      snprintf(msgtmp,100,"%d",db_dist);
      gtk_entry_set_text (GTK_ENTRY (doubledist),msgtmp);
  gtk_widget_show (doubledist);
  gtk_box_pack_start (GTK_BOX (hbox), doubledist,
		      (gboolean) TRUE, (gboolean) TRUE, 0);


  (void)gtk_signal_connect_object (GTK_OBJECT (printer_dialog), "delete_event",
			     (GtkSignalFunc) gtk_widget_hide,
			     GTK_OBJECT (printer_dialog));

  hbox = gtk_hbox_new (FALSE, 10);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);
  gtk_widget_show (hbox);

  pokbutton = gtk_button_new_with_label (_("Ok"));
  gtk_box_pack_start (GTK_BOX (hbox), pokbutton,
		      (gboolean) TRUE, (gboolean) TRUE, 0);
  (void)gtk_signal_connect_object (GTK_OBJECT (pokbutton), "clicked",
			     GTK_SIGNAL_FUNC (gtk_widget_hide),
			     GTK_OBJECT (printer_dialog));
  (void)gtk_signal_connect (GTK_OBJECT (pokbutton), "clicked",
		      GTK_SIGNAL_FUNC (setup_printer), "1");
  gtk_widget_show (pokbutton);

  pcabutton = gtk_button_new_with_label (_("Cancel"));
  gtk_box_pack_start (GTK_BOX (hbox), pcabutton,
		      (gboolean) TRUE, (gboolean) TRUE, 0);
  (void)gtk_signal_connect_object (GTK_OBJECT (pcabutton), "clicked",
			     GTK_SIGNAL_FUNC (gtk_widget_hide),
			     GTK_OBJECT (printer_dialog));
  (void)gtk_signal_connect (GTK_OBJECT (pcabutton), "clicked",
		      GTK_SIGNAL_FUNC (setup_printer), "0");
  gtk_widget_show (pcabutton);


#endif
/****************************************************************/


/*************************************************************/
/* Popup to show a selection of templates to insert into the drawing */

  templates = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (templates), _("Templates"));
  gtk_widget_realize (templates);
  tvbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (templates), tvbox);
  gtk_widget_show (tvbox);
  (void)gtk_signal_connect_object (GTK_OBJECT (templates), "delete_event",
			     (GtkSignalFunc) gtk_widget_hide,
			     GTK_OBJECT (templates));
  temptips = gtk_tooltips_new ();
  templatebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (templatebook), GTK_POS_TOP);
  label = gtk_label_new (_("Carbocycles"));
  gtk_widget_show (label);
  page = gtk_frame_new (_("Carbocycles"));
  gtk_widget_show (page);
  gtk_notebook_append_page (GTK_NOTEBOOK (templatebook), page, label);
  pbox = gtk_table_new (5, 5, TRUE);
  gtk_widget_show (pbox);
  for (i = 0; i < 5; i++)
    {
      for (j = 0; j < 5; j++)
	{
	  tbutton[5 * i + j] = gtk_button_new ();
	  style = gtk_widget_get_style (templates);
	  pixmap = gdk_pixmap_create_from_xpm_d (templates->window, &mask,
						 &style->bg[GTK_STATE_NORMAL],
						 (gchar **)
						 template_xpm[i][j]);
	  pixmapwid = gtk_pixmap_new (pixmap, mask);
	  gtk_widget_show (pixmapwid);
	  gdk_pixmap_unref (pixmap);
	  gtk_container_add (GTK_CONTAINER (tbutton[5 * i + j]), pixmapwid);
	  gtk_widget_show (tbutton[5 * i + j]);
	  gtk_table_attach_defaults (GTK_TABLE (pbox), tbutton[5 * i + j], (guint)j,
				     (guint)j + 1, (guint)i, (guint)i + 1);
	  tmplnum[5 * i + j] = 10 * i + j;
	  (void)gtk_signal_connect (GTK_OBJECT (tbutton[5 * i + j]), "clicked",
			      GTK_SIGNAL_FUNC (Add_template),
			      GINT_TO_POINTER(tmplnum[5 * i + j]) );
	 if ((int)strlen(template_tip[i][j]) >0 ) {			      
#ifndef ENABLE_NLS
	  gtk_tooltips_set_tip (temptips, tbutton[5 * i + j],
				template_tip[i][j], NULL);
#else
	  gtk_tooltips_set_tip (temptips, tbutton[5 * i + j],
				gettext(template_tip[i][j]), NULL);
#endif
	 }	
	}
    }
  gtk_container_add (GTK_CONTAINER (page), pbox);
  label = gtk_label_new (_("Sugars"));
  gtk_widget_show (label);
  page = gtk_frame_new (_("Sugars"));
  gtk_widget_show (page);
  gtk_notebook_append_page (GTK_NOTEBOOK (templatebook), page, label);
  pbox = gtk_table_new (5, 5, TRUE);
  gtk_widget_show (pbox);
  for (i = 0; i < 5; i++)
    {
      for (j = 0; j < 5; j++)
	{
	  tbutton[25 + 5 * i + j] = gtk_button_new ();
	  style = gtk_widget_get_style (templates);
	  pixmap = gdk_pixmap_create_from_xpm_d (templates->window, &mask,
						 &style->bg[GTK_STATE_NORMAL],
						 (gchar **) template_xpm[i +
									 5]
						 [j]);
	  pixmapwid = gtk_pixmap_new (pixmap, mask);
	  gtk_widget_show (pixmapwid);
	  gdk_pixmap_unref (pixmap);
	  gtk_container_add (GTK_CONTAINER (tbutton[25 + 5 * i + j]),
			     pixmapwid);
	  gtk_widget_show (tbutton[25 + 5 * i + j]);
	  gtk_table_attach_defaults (GTK_TABLE (pbox),
				     tbutton[25 + 5 * i + j], (guint)j, (guint)j + 1, 
				     (guint)i, (guint)i + 1);
	  tmplnum[25 + 5 * i + j] = 10 * (i + 5) + j;
	  (void)gtk_signal_connect (GTK_OBJECT (tbutton[25 + 5 * i + j]), "clicked",
			      GTK_SIGNAL_FUNC (Add_template),
			      GINT_TO_POINTER(tmplnum[25 + 5 * i + j]) );
	 if ((int)strlen(template_tip[i+5][j]) >0 ) {			      
#ifndef ENABLE_NLS
	  gtk_tooltips_set_tip (temptips, tbutton[25 + 5 * i + j],
				template_tip[i + 5][j], NULL);
#else
	  gtk_tooltips_set_tip (temptips, tbutton[25 + 5 * i + j],
				gettext(template_tip[i + 5][j]), NULL);
#endif
	 }
	}
    }
  gtk_container_add (GTK_CONTAINER (page), pbox);

  label = gtk_label_new (_("Heterocycles"));
  gtk_widget_show (label);
  page = gtk_frame_new (_("Heterocycles"));
  gtk_widget_show (page);
  gtk_notebook_append_page (GTK_NOTEBOOK (templatebook), page, label);
  pbox = gtk_table_new (5, 5, TRUE);
  gtk_widget_show (pbox);
  for (i = 0; i < 5; i++)
    {
      for (j = 0; j < 5; j++)
	{
	  tbutton[50 + 5 * i + j] = gtk_button_new ();
	  style = gtk_widget_get_style (templates);
	  pixmap = gdk_pixmap_create_from_xpm_d (templates->window, &mask,
						 &style->bg[GTK_STATE_NORMAL],
						 (gchar **) template_xpm[i +
									 10]
						 [j]);
	  pixmapwid = gtk_pixmap_new (pixmap, mask);
	  gtk_widget_show (pixmapwid);
	  gdk_pixmap_unref (pixmap);
	  gtk_container_add (GTK_CONTAINER (tbutton[50 + 5 * i + j]),
			     pixmapwid);
	  gtk_widget_show (tbutton[50 + 5 * i + j]);
	  gtk_table_attach_defaults (GTK_TABLE (pbox),
				     tbutton[50 + 5 * i + j], (guint)j, 
				     (guint)j + 1, (guint)i,(guint)i + 1);
	  tmplnum[50 + 5 * i + j] = 10 * (i + 10) + j;
	  (void)gtk_signal_connect (GTK_OBJECT (tbutton[50 + 5 * i + j]), "clicked",
			      GTK_SIGNAL_FUNC (Add_template),
			      GINT_TO_POINTER(tmplnum[50 + 5 * i + j]) );
	 if ((int)strlen(template_tip[i+10][j]) >0 ) {			      
#ifndef ENABLE_NLS
	  gtk_tooltips_set_tip (temptips, tbutton[50 + 5 * i + j],
				template_tip[i + 10][j], NULL);
#else
	  gtk_tooltips_set_tip (temptips, tbutton[50 + 5 * i + j],
				gettext(template_tip[i + 10][j]), NULL);
#endif
	 }
	}
    }
  gtk_container_add (GTK_CONTAINER (page), pbox);

  label = gtk_label_new (_("Amino Acids"));
  gtk_widget_show (label);
  page = gtk_frame_new (_("Amino Acids"));
  gtk_widget_show (page);
  gtk_notebook_append_page (GTK_NOTEBOOK (templatebook), page, label);
  pbox = gtk_table_new (5, 5, TRUE);
  gtk_widget_show (pbox);
  for (i = 0; i < 5; i++)
    {
      for (j = 0; j < 5; j++)
	{
	  tbutton[75 + 5 * i + j] = gtk_button_new ();
	  style = gtk_widget_get_style (templates);
	  pixmap = gdk_pixmap_create_from_xpm_d (templates->window, &mask,
						 &style->bg[GTK_STATE_NORMAL],
						 (gchar **) template_xpm[i +
									 15]
						 [j]);
	  pixmapwid = gtk_pixmap_new (pixmap, mask);
	  gtk_widget_show (pixmapwid);
	  gdk_pixmap_unref (pixmap);
	  gtk_container_add (GTK_CONTAINER (tbutton[75 + 5 * i + j]),
			     pixmapwid);
	  gtk_widget_show (tbutton[75 + 5 * i + j]);
	  gtk_table_attach_defaults (GTK_TABLE (pbox),
				     tbutton[75 + 5 * i + j], 
				     (guint)j, (guint)j + 1, (guint)i,
				     (guint)i + 1);
	  tmplnum[75 + 5 * i + j] = 10 * (i + 15) + j;
	  (void)gtk_signal_connect (GTK_OBJECT (tbutton[75 + 5 * i + j]), "clicked",
			      GTK_SIGNAL_FUNC (Add_template),
			      GINT_TO_POINTER(tmplnum[75 + 5 * i + j]) );
	 if ((int)strlen(template_tip[i+15][j]) >0 ) {			      
#ifndef ENABLE_NLS
	  gtk_tooltips_set_tip (temptips, tbutton[75 + 5 * i + j],
				template_tip[i + 15][j], NULL);
#else
	  gtk_tooltips_set_tip (temptips, tbutton[75 + 5 * i + j],
				gettext(template_tip[i + 15][j]), NULL);
#endif
	 }
	}
    }
  gtk_container_add (GTK_CONTAINER (page), pbox);

  label = gtk_label_new (_("Symbols"));
  gtk_widget_show (label);
  page = gtk_frame_new (_("Symbols"));
  gtk_widget_show (page);
  gtk_notebook_append_page (GTK_NOTEBOOK (templatebook), page, label);
  pbox = gtk_table_new (5, 5, TRUE);
  gtk_widget_show (pbox);
  for (i = 0; i < 5; i++)
    {
      for (j = 0; j < 5; j++)
	{
	  tbutton[100 + 5 * i + j] = gtk_button_new ();
	  style = gtk_widget_get_style (templates);
	  pixmap = gdk_pixmap_create_from_xpm_d (templates->window, &mask,
						 &style->bg[GTK_STATE_NORMAL],
						 (gchar **) template_xpm[i +
									 20]
						 [j]);
	  pixmapwid = gtk_pixmap_new (pixmap, mask);
	  gtk_widget_show (pixmapwid);
	  gdk_pixmap_unref (pixmap);
	  gtk_container_add (GTK_CONTAINER (tbutton[100 + 5 * i + j]),
			     pixmapwid);
	  gtk_widget_show (tbutton[100 + 5 * i + j]);
	  gtk_table_attach_defaults (GTK_TABLE (pbox),
				     tbutton[100 + 5 * i + j], 
				     (guint)j, (guint)j + 1, (guint)i,
				     (guint)i + 1);
	  tmplnum[100 + 5 * i + j] = 10 * (i + 20) + j;
	  (void)gtk_signal_connect (GTK_OBJECT (tbutton[100 + 5 * i + j]), "clicked",
			      GTK_SIGNAL_FUNC (Add_template),
			      GINT_TO_POINTER(tmplnum[100 + 5 * i + j]) );
	 if ((int)strlen(template_tip[i+20][j]) >0 ) {			      
#ifndef ENABLE_NLS
	  gtk_tooltips_set_tip (temptips, tbutton[100 + 5 * i + j],
				template_tip[i + 20][j], NULL);
#else
	  gtk_tooltips_set_tip (temptips, tbutton[100 + 5 * i + j],
				gettext(template_tip[i + 20][j]), NULL);
#endif
	 }
	}
    }
  gtk_container_add (GTK_CONTAINER (page), pbox);

  gtk_widget_show (templatebook);
  gtk_box_pack_start (GTK_BOX (tvbox), templatebook, (gboolean) TRUE,
		      (gboolean) TRUE, 0);
  button = gtk_button_new_with_label (_("Close"));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (tvbox), button, (gboolean) TRUE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
			     GTK_SIGNAL_FUNC (gtk_widget_hide),
			     GTK_OBJECT (templates));

/**************************************************************/


/*
 * Create the main window 
 */

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_name (window, "Chemtool");
  gtk_window_set_title (GTK_WINDOW (window), "Chemtool 1.6.14");

/* all pop-ups should appear topmost and at the current cursor position,
   that is, near their respective toolbar buttons */
  gtk_window_set_transient_for (GTK_WINDOW (filew), GTK_WINDOW (window));
  gtk_window_set_position (GTK_WINDOW (filew), GTK_WIN_POS_MOUSE);
  gtk_window_set_transient_for (GTK_WINDOW (expw), GTK_WINDOW (window));
  gtk_window_set_position (GTK_WINDOW (expw), GTK_WIN_POS_MOUSE);
  gtk_window_set_transient_for (GTK_WINDOW (messagew), GTK_WINDOW (window));
  gtk_window_set_position (GTK_WINDOW (messagew), GTK_WIN_POS_MOUSE);

  tooltips = gtk_tooltips_new ();	/* initialize tool-tip message boxes */

/* initialize vertical stacking of button rows and drawing area */
  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (window), vbox);
  gtk_widget_show (vbox);

  (void)gtk_signal_connect (GTK_OBJECT (window), "delete_event",
		      GTK_SIGNAL_FUNC (CheckAndQuit), NULL);

#ifdef MENU
/* build the menubar */

  accel_group = gtk_accel_group_new ();

  /* the menuitems of the file-menu */
  file = gtk_menu_new ();

  new = gtk_menu_item_new_with_label (_("New"));
  gtk_menu_append (GTK_MENU (file), new);
  (void)gtk_signal_connect_object (GTK_OBJECT (new), "activate",
			     GTK_SIGNAL_FUNC (CheckAndClear), NULL);
  gtk_widget_add_accelerator (new, "activate", accel_group,
			      GDK_N, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_widget_show (new);

  open = gtk_menu_item_new_with_label (_("Open"));
  gtk_menu_append (GTK_MENU (file), open);
  (void)gtk_signal_connect_object (GTK_OBJECT (open), "activate",
			     GTK_SIGNAL_FUNC (CheckAndLoad), NULL);
  gtk_widget_add_accelerator (open, "activate", accel_group,
			      GDK_O, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_widget_show (open);

  add = gtk_menu_item_new_with_label (_("Add"));
  gtk_menu_append (GTK_MENU (file), add);
  (void)gtk_signal_connect_object (GTK_OBJECT (add), "activate",
			     GTK_SIGNAL_FUNC (Add), NULL);
  gtk_widget_add_accelerator (add, "activate", accel_group,
			      GDK_A, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_widget_show (add);

  import = gtk_menu_item_new_with_label (_("Import MOL"));
  gtk_menu_append (GTK_MENU (file), import);
  (void)gtk_signal_connect_object (GTK_OBJECT (import), "activate",
			     GTK_SIGNAL_FUNC (Import), NULL);
  gtk_widget_add_accelerator (import, "activate", accel_group,
			      GDK_I, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_widget_show (import);

  imppdb = gtk_menu_item_new_with_label (_("Import PDB"));
  gtk_menu_append (GTK_MENU (file), imppdb);
  (void)gtk_signal_connect_object (GTK_OBJECT (imppdb), "activate",
			     GTK_SIGNAL_FUNC (Import_PDB), NULL);
  gtk_widget_add_accelerator (imppdb, "activate", accel_group,
			      GDK_P, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_widget_show (imppdb);

  impany = gtk_menu_item_new_with_label (_("Import (Babel)"));
  gtk_menu_append (GTK_MENU (file), impany);
  (void)gtk_signal_connect_object (GTK_OBJECT (impany), "activate",
			     GTK_SIGNAL_FUNC (Import_Babel), NULL);
  gtk_widget_add_accelerator (impany, "activate", accel_group,
			      GDK_B, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_widget_show (impany);
  if (babelin<0) gtk_widget_set_sensitive(impany,FALSE);  
  export = gtk_menu_item_new_with_label (_("Export..."));
  gtk_menu_append (GTK_MENU (file), export);
  (void)gtk_signal_connect_object (GTK_OBJECT (export), "activate",
			     GTK_SIGNAL_FUNC (Export), NULL);
  gtk_widget_add_accelerator (export, "activate", accel_group,
			      GDK_E, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_widget_show (export);

  expany = gtk_menu_item_new_with_label (_("Export (Babel)"));
  gtk_menu_append (GTK_MENU (file), expany);
  (void)gtk_signal_connect_object (GTK_OBJECT (expany), "activate",
			     GTK_SIGNAL_FUNC (Export_Babel), NULL);
  gtk_widget_add_accelerator (expany, "activate", accel_group,
			      GDK_X, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_widget_show (expany);
  if (babelout<0) gtk_widget_set_sensitive(expany,FALSE);  

  printps = gtk_menu_item_new_with_label (_("Print"));
  gtk_menu_append (GTK_MENU (file), printps);
  (void)gtk_signal_connect_object (GTK_OBJECT (printps), "activate",
			     GTK_SIGNAL_FUNC (print_ps), NULL);
  gtk_widget_add_accelerator (printps, "activate", accel_group,
			      GDK_P, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_widget_show (printps);

  print_setup = gtk_menu_item_new_with_label (_("Setup Defaults"));
  gtk_menu_append (GTK_MENU (file), print_setup);
#if (GTK_MINOR_VERSION >2)
  (void)gtk_signal_connect_object (GTK_OBJECT (print_setup), "activate",
			     GTK_SIGNAL_FUNC (print_setup_menu_activate),
			     NULL);
#else
 (void)gtk_signal_connect_object (GTK_OBJECT (print_setup), "activate",
                            GTK_SIGNAL_FUNC (gtk_widget_show),
                            GTK_OBJECT (printer_dialog));
#endif
  gtk_widget_add_accelerator (print_setup, "activate", accel_group,
			      GDK_D, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_widget_show (print_setup);

  save_setup = gtk_menu_item_new_with_label (_("Save Config"));
  gtk_menu_append (GTK_MENU (file), save_setup);
  (void)gtk_signal_connect_object (GTK_OBJECT (save_setup), "activate",
			     GTK_SIGNAL_FUNC (writerc), NULL);
  gtk_widget_add_accelerator (save_setup, "activate", accel_group,
			      GDK_numbersign, GDK_CONTROL_MASK,
			      GTK_ACCEL_VISIBLE);
  gtk_widget_show (save_setup);

  save = gtk_menu_item_new_with_label (_("Save"));
  gtk_menu_append (GTK_MENU (file), save);
  (void)gtk_signal_connect_object (GTK_OBJECT (save), "activate",
			     GTK_SIGNAL_FUNC (Save), NULL);
  gtk_widget_add_accelerator (save, "activate", accel_group,
			      GDK_S, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_widget_show (save);

  saveas = gtk_menu_item_new_with_label (_("Save As..."));
  gtk_menu_append (GTK_MENU (file), saveas);
  (void)gtk_signal_connect_object (GTK_OBJECT (saveas), "activate",
			     GTK_SIGNAL_FUNC (SaveAs), NULL);
  /*  gtk_widget_add_accelerator (saveas, "activate", accel_group,
     GDK_F4, 0, 
     GTK_ACCEL_VISIBLE); */
  gtk_widget_show (saveas);

  quit = gtk_menu_item_new_with_label (_("Quit"));
  gtk_menu_append (GTK_MENU (file), quit);
  (void)gtk_signal_connect_object (GTK_OBJECT (quit), "activate",
			     GTK_SIGNAL_FUNC (CheckAndQuit), NULL);
  gtk_widget_add_accelerator (quit, "activate", accel_group,
			      GDK_Q, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_widget_show (quit);

  file_menu = gtk_menu_item_new_with_label (_("File"));
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (file_menu), file);
  gtk_widget_show (file_menu);

  /* the menuitems of the edit-menu */
  edit = gtk_menu_new ();

  copy = gtk_menu_item_new_with_label (_("Copy"));
  gtk_menu_append (GTK_MENU (edit), copy);
  (void)gtk_signal_connect (GTK_OBJECT (copy), "activate",
		      GTK_SIGNAL_FUNC (copy_obj), GTK_OBJECT (window));
/*  (void)gtk_signal_connect (GTK_OBJECT (copy), "activate",
                        GTK_SIGNAL_FUNC (gtk_toggle_button_set_active), 
                        GTK_OBJECT (movebutton));*/

  gtk_widget_add_accelerator (copy, "activate", accel_group,
			      GDK_V, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_widget_show (copy);

  fliph = gtk_menu_item_new_with_label (_("Flip horizontally"));
  gtk_menu_append (GTK_MENU (edit), fliph);
  (void)gtk_signal_connect (GTK_OBJECT (fliph), "activate",
		      GTK_SIGNAL_FUNC (flip_horiz), GTK_OBJECT (window));
  gtk_widget_show (fliph);

  flipv = gtk_menu_item_new_with_label (_("Flip vertically"));
  gtk_menu_append (GTK_MENU (edit), flipv);
  (void)gtk_signal_connect (GTK_OBJECT (flipv), "activate",
		      GTK_SIGNAL_FUNC (flip_vert), GTK_OBJECT (window));
  gtk_widget_show (flipv);

#ifdef LIBUNDO
  undo = gtk_menu_item_new_with_label (_("Undo"));
  gtk_menu_append (GTK_MENU (edit), undo);
  (void)gtk_signal_connect (GTK_OBJECT (undo), "activate",
		      GTK_SIGNAL_FUNC (do_undo), GTK_OBJECT (window));

  gtk_widget_add_accelerator (undo, "activate", accel_group,
			      GDK_U, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_widget_show (undo);
  redo = gtk_menu_item_new_with_label (_("Redo"));
  gtk_menu_append (GTK_MENU (edit), redo);
  (void)gtk_signal_connect (GTK_OBJECT (redo), "activate",
		      GTK_SIGNAL_FUNC (do_redo), GTK_OBJECT (window));

  gtk_widget_add_accelerator (redo, "activate", accel_group,
			      GDK_R, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_widget_show (redo);
#endif

  edit_menu = gtk_menu_item_new_with_label (_("Edit"));
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (edit_menu), edit);
  gtk_widget_show (edit_menu);

  /* the menuitems of the view-menu */
  view = gtk_menu_new ();

  zoomin = gtk_menu_item_new_with_label (_("Zoom in"));
  gtk_menu_append (GTK_MENU (view), zoomin);
  (void)gtk_signal_connect (GTK_OBJECT (zoomin), "activate",
		      GTK_SIGNAL_FUNC (Zoom), "0");
  gtk_widget_show (zoomin);

  zoomout = gtk_menu_item_new_with_label (_("Zoom out"));
  gtk_menu_append (GTK_MENU (view), zoomout);
  (void)gtk_signal_connect (GTK_OBJECT (zoomout), "activate",
		      GTK_SIGNAL_FUNC (Zoom), "1");
  gtk_widget_show (zoomout);

  gtk_widget_add_accelerator (zoomin, "activate", accel_group,
			      GDK_equal, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (zoomout, "activate", accel_group,
			      GDK_minus, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

  center = gtk_menu_item_new_with_label (_("Center"));
  gtk_menu_append (GTK_MENU (view), center);
  (void)gtk_signal_connect (GTK_OBJECT (center), "activate",
		      GTK_SIGNAL_FUNC (Center), GTK_OBJECT (window));
  gtk_widget_add_accelerator (center, "activate", accel_group,
			      GDK_C, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_widget_show (center);

  grid = gtk_menu_item_new_with_label (_("Grid rect/rhomb/off"));
  gtk_menu_append (GTK_MENU (view), grid);
  (void)gtk_signal_connect (GTK_OBJECT (grid), "activate",
		      GTK_SIGNAL_FUNC (Grid), GTK_OBJECT (window));
  gtk_widget_show (grid);

  view_menu = gtk_menu_item_new_with_label (_("View"));
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (view_menu), view);
  gtk_widget_show (view_menu);

  /* the menuitems of the tools-menu */

  tools = gtk_menu_new ();

  templatem = gtk_menu_item_new_with_label (_("Templates..."));
  gtk_menu_append (GTK_MENU (tools), templatem);
  (void)gtk_signal_connect_object (GTK_OBJECT (templatem), "activate",
			     GTK_SIGNAL_FUNC (show_or_raise),
			     GTK_OBJECT (templates));
  gtk_widget_add_accelerator (templatem, "activate", accel_group,
			      GDK_T, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_widget_show (templatem);

  cht = gtk_menu_item_new_with_label (_("Calculate Formula Weight"));
  gtk_menu_append (GTK_MENU (tools), cht);
  (void)gtk_signal_connect (GTK_OBJECT (cht), "activate",
		      GTK_SIGNAL_FUNC (do_fw), NULL);
  gtk_widget_add_accelerator (cht, "activate", accel_group,
			      GDK_F, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_widget_show (cht);

  clean = gtk_menu_item_new_with_label (_("Clean up drawing"));
  gtk_menu_append (GTK_MENU (tools), clean);
  (void)gtk_signal_connect (GTK_OBJECT (clean), "activate",
		      GTK_SIGNAL_FUNC (tidy_mol), NULL);
  gtk_widget_show (clean);

  tools_menu = gtk_menu_item_new_with_label (_("Tools"));
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (tools_menu), tools);
  gtk_widget_show (tools_menu);


  /* the menuitems of the help-menu */
  help = gtk_menu_new ();

  about = gtk_menu_item_new_with_label (_("About"));
  gtk_menu_append (GTK_MENU (help), about);
  (void)gtk_signal_connect_object (GTK_OBJECT (about), "activate",
			     GTK_SIGNAL_FUNC (gtk_widget_show),
			     GTK_OBJECT (aboutw));
/*  gtk_widget_add_accelerator (about, "activate", accel_group,
			      GDK_F1, 0, GTK_ACCEL_VISIBLE);*/
  gtk_widget_show (about);

  using = gtk_menu_item_new_with_label (_("Help"));
  gtk_menu_append (GTK_MENU (help), using);
  (void)gtk_signal_connect_object (GTK_OBJECT (using), "activate",
			     GTK_SIGNAL_FUNC (gtk_widget_show),
			     GTK_OBJECT (helpw));
  gtk_widget_add_accelerator (using, "activate", accel_group,
			      GDK_F1, 0, GTK_ACCEL_VISIBLE);
  gtk_widget_show (using);

  help_menu = gtk_menu_item_new_with_label (_("Help"));
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (help_menu), help);
  gtk_widget_show (help_menu);

  /* build the menubar */
  menu_bar = gtk_menu_bar_new ();
  gtk_box_pack_start (GTK_BOX (vbox), menu_bar, (gboolean) FALSE,
		      (gboolean) FALSE, 2);
  gtk_widget_show (menu_bar);
  gtk_menu_bar_append (GTK_MENU_BAR (menu_bar), file_menu);
  gtk_menu_bar_append (GTK_MENU_BAR (menu_bar), edit_menu);
  gtk_menu_bar_append (GTK_MENU_BAR (menu_bar), view_menu);
  gtk_menu_bar_append (GTK_MENU_BAR (menu_bar), tools_menu);
  gtk_menu_item_right_justify (GTK_MENU_ITEM (help_menu));
  gtk_menu_bar_append (GTK_MENU_BAR (menu_bar), help_menu);

  gtk_window_add_accel_group (GTK_WINDOW (window), accel_group);
#else

/* initialize horizontal packing of the first row of buttons */
/* define the first row of buttons, with label,callback and tooltip text */
/* Load | Add | Save | Import | Export | ZoomIn | Center | ZoomOut | Clear | Quit | About */
  hbox = gtk_hbox_new (TRUE, 0);
  loadbutton = gtk_button_new_with_label (_("Load"));
  gtk_box_pack_start (GTK_BOX (hbox), loadbutton, (gboolean) FALSE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect_object (GTK_OBJECT (loadbutton), "clicked",
			     GTK_SIGNAL_FUNC (CheckAndLoad), NULL);
  gtk_tooltips_set_tip (tooltips, loadbutton, _("Load a chemtool sketch"),
			NULL);
  gtk_widget_show (loadbutton);

  addbutton = gtk_button_new_with_label (_("Add"));
  gtk_box_pack_start (GTK_BOX (hbox), addbutton, (gboolean) TRUE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect_object (GTK_OBJECT (addbutton), "clicked",
			     GTK_SIGNAL_FUNC (Add), NULL);
  gtk_tooltips_set_tip (tooltips, addbutton, _("Add fragment from file"),
			NULL);
  gtk_widget_show (addbutton);
  tempbutton = gtk_button_new_with_label (_("Templates"));
  gtk_box_pack_start (GTK_BOX (hbox), tempbutton, (gboolean) TRUE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect_object (GTK_OBJECT (tempbutton), "clicked",
			     GTK_SIGNAL_FUNC (show_or_raise),
			     GTK_OBJECT (templates));
  gtk_tooltips_set_tip (tooltips, tempbutton, _("Add template structure"),
			NULL);
  gtk_widget_show (tempbutton);
  savebutton = gtk_button_new_with_label (_("Save"));
  gtk_box_pack_start (GTK_BOX (hbox), savebutton, (gboolean) TRUE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect_object (GTK_OBJECT (savebutton), "clicked",
			     GTK_SIGNAL_FUNC (Save), NULL);
  gtk_tooltips_set_tip (tooltips, savebutton, _("Save sketch to file"), NULL);
  gtk_widget_show (savebutton);

  exportbutton = gtk_button_new_with_label (_("Export"));
  gtk_box_pack_start (GTK_BOX (hbox), exportbutton, (gboolean) TRUE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect_object (GTK_OBJECT (exportbutton), "clicked",
			     GTK_SIGNAL_FUNC (Export), NULL);
  (void)gtk_signal_connect (GTK_OBJECT (exportbutton), "clicked",
		      GTK_SIGNAL_FUNC (gtk_file_selection_set_filename),
		      GTK_OBJECT (expw));
  gtk_tooltips_set_tip (tooltips, exportbutton,
			_("Create EPS, XFig, PicTeX or XBM file"), NULL);
  gtk_widget_show (exportbutton);

  printbutton = gtk_button_new_with_label (_("Print"));
  gtk_box_pack_start (GTK_BOX (hbox), printbutton, (gboolean) TRUE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect_object (GTK_OBJECT (printbutton), "clicked",
			     GTK_SIGNAL_FUNC (print_ps), NULL);
  gtk_tooltips_set_tip (tooltips, printbutton,
			_("Print file to a postscript printer"), NULL);
  gtk_widget_show (printbutton);

  psetupbutton = gtk_button_new_with_label (_("Setup"));
  gtk_box_pack_start (GTK_BOX (hbox), psetupbutton, (gboolean) TRUE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect_object (GTK_OBJECT (psetupbutton), "clicked",
			     GTK_SIGNAL_FUNC (gtk_widget_show),
			     GTK_OBJECT (printer_dialog));
  gtk_tooltips_set_tip (tooltips, psetupbutton,
			_("Setup default options"), NULL);
  gtk_widget_show (psetupbutton);

  savesetupbutton = gtk_button_new_with_label (_("Save Config"));
  gtk_box_pack_start (GTK_BOX (hbox), savesetupbutton, (gboolean) TRUE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect_object (GTK_OBJECT (savesetupbutton), "clicked",
			     GTK_SIGNAL_FUNC (writerc), NULL);
  gtk_tooltips_set_tip (tooltips, savesetupbutton,
			_("Save default options"), NULL);
  gtk_widget_show (savesetupbutton);

  importbutton = gtk_button_new_with_label (_("Import"));
  gtk_box_pack_start (GTK_BOX (hbox), importbutton, (gboolean) TRUE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect_object (GTK_OBJECT (importbutton), "clicked",
			     GTK_SIGNAL_FUNC (Import), NULL);
  gtk_tooltips_set_tip (tooltips, importbutton,
			_("Read a file written in MDL/molfile format"), NULL);
  gtk_widget_show (importbutton);

  imppdbbutton = gtk_button_new_with_label (_("ImportPDB"));
  gtk_box_pack_start (GTK_BOX (hbox), imppdbbutton, (gboolean) TRUE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect_object (GTK_OBJECT (imppdbbutton), "clicked",
			     GTK_SIGNAL_FUNC (Import_PDB), NULL);
  gtk_tooltips_set_tip (tooltips, imppdbbutton,
			_("Read a file written in PDB format"), NULL);
  gtk_widget_show (imppdbbutton);

  zoominbutton = gtk_button_new_with_label (_("Zoom In"));
  gtk_box_pack_start (GTK_BOX (hbox), zoominbutton, (gboolean) FALSE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect (GTK_OBJECT (zoominbutton), "clicked",
		      GTK_SIGNAL_FUNC (Zoom), "0");
  gtk_tooltips_set_tip (tooltips, zoominbutton, _("Increase zoom scale"),
			NULL);
  gtk_widget_show (zoominbutton);

  centerbutton = gtk_button_new_with_label (_("Center"));
  gtk_box_pack_start (GTK_BOX (hbox), centerbutton, (gboolean) FALSE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect (GTK_OBJECT (centerbutton), "clicked",
		      GTK_SIGNAL_FUNC (Center), GTK_OBJECT (window));
  gtk_tooltips_set_tip (tooltips, centerbutton,
			_("Center molecule in drawing area"), NULL);
  gtk_widget_show (centerbutton);

  zoomoutbutton = gtk_button_new_with_label (_("Zoom Out"));
  gtk_box_pack_start (GTK_BOX (hbox), zoomoutbutton, (gboolean) FALSE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect (GTK_OBJECT (zoomoutbutton), "clicked",
		      GTK_SIGNAL_FUNC (Zoom), "1");
  gtk_tooltips_set_tip (tooltips, zoomoutbutton, _("Decrease zoom scale"),
			NULL);
  gtk_widget_show (zoomoutbutton);

  clearbutton = gtk_button_new_with_label (_("Clear"));
  gtk_box_pack_start (GTK_BOX (hbox), clearbutton, (gboolean) FALSE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect (GTK_OBJECT (clearbutton), "clicked",
		      GTK_SIGNAL_FUNC (CheckAndClear), NULL);
  gtk_tooltips_set_tip (tooltips, clearbutton, _("Remove molecule"), NULL);
  gtk_widget_show (clearbutton);

  fwbutton = gtk_button_new_with_label (_("FW"));
  gtk_box_pack_start (GTK_BOX (hbox), fwbutton, (gboolean) FALSE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect (GTK_OBJECT (fwbutton), "clicked",
		      GTK_SIGNAL_FUNC (do_fw), NULL);
  gtk_tooltips_set_tip (tooltips, fwbutton, _("Calculate Formula Mass"),
			NULL);
  gtk_widget_show (fwbutton);

  quitbutton = gtk_button_new_with_label (_("Quit"));
  gtk_box_pack_start (GTK_BOX (hbox), quitbutton, (gboolean) FALSE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect (GTK_OBJECT (quitbutton), "clicked",
		      GTK_SIGNAL_FUNC (CheckAndQuit), NULL);
  gtk_tooltips_set_tip (tooltips, quitbutton, _("Exit Chemtool"), NULL);
  gtk_widget_show (quitbutton);

  aboutbutton = gtk_button_new_with_label (_("About"));
  gtk_box_pack_end (GTK_BOX (hbox), aboutbutton, (gboolean) FALSE,
		    (gboolean) TRUE, 0);
  (void)gtk_signal_connect_object (GTK_OBJECT (aboutbutton), "clicked",
			     GTK_SIGNAL_FUNC (gtk_widget_show),
			     GTK_OBJECT (aboutw));
  gtk_tooltips_set_tip (tooltips, aboutbutton, _("About Chemtool"), NULL);
  gtk_widget_show (aboutbutton);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, (gboolean) FALSE,
		      (gboolean) FALSE, 0);
  gtk_widget_show (hbox);

#endif /* menubar or button row */

/* end top row */
/* initialize horizontal packing of second row of buttons */
#ifdef GTK2
  hbox = gtk_hbox_new (FALSE, 4);
#else
  hbox = gtk_hbox_new (FALSE, 5);
#endif

  gtk_widget_realize (window);
  /* complete setup of base window, but do not show it yet - we just
     need a pointer to it and its color settings for the button pixmaps */

/* Second row has pixmap icons for line drawing on hexagonal, pentagonal
   (two orientations) and octagonal grid; left-justfied,centered and
   right-justified text; bondtype, move, mark and rotate mode; 
   vertical and horizontal flipping; copying; and a text entry box */

  style = gtk_widget_get_style (window);

  pixmap = gdk_pixmap_create_from_xpm_d (window->window, &mask,
					 &style->bg[GTK_STATE_NORMAL],
					 (gchar **) xpm_hex);
  pixmapwid = gtk_pixmap_new (pixmap, mask);
  gtk_widget_show (pixmapwid);
  gdk_pixmap_unref (pixmap);

  hexbutton = gtk_toggle_button_new ();
  gtk_container_add (GTK_CONTAINER (hexbutton), pixmapwid);
  gtk_box_pack_start (GTK_BOX (hbox), hexbutton, (gboolean) FALSE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect (GTK_OBJECT (hexbutton), "clicked",
		      GTK_SIGNAL_FUNC (Change_Angle), "1");
  gtk_tooltips_set_tip (tooltips, hexbutton,
			_("Draw at 0/30/60/90 degree angles"), NULL);
  gtk_widget_show (hexbutton);

  pent1button = gtk_toggle_button_new ();
  pixmap = gdk_pixmap_create_from_xpm_d (window->window, &mask,
					 &style->bg[GTK_STATE_NORMAL],
					 (gchar **) xpm_pent1);
  pixmapwid = gtk_pixmap_new (pixmap, mask);
  gtk_widget_show (pixmapwid);
  gdk_pixmap_unref (pixmap);
  gtk_container_add (GTK_CONTAINER (pent1button), pixmapwid);
  gtk_box_pack_start (GTK_BOX (hbox), pent1button, (gboolean) FALSE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect (GTK_OBJECT (pent1button), "clicked",
		      GTK_SIGNAL_FUNC (Change_Angle), "2");
  gtk_tooltips_set_tip (tooltips, pent1button,
			_("Draw at 0/36/72/... degree angles"), NULL);
#ifdef MENU
  gtk_widget_add_accelerator (pent1button, "clicked", accel_group,
			      GDK_F5, 0, GTK_ACCEL_VISIBLE);

#endif
  gtk_widget_show (pent1button);

  pent2button = gtk_toggle_button_new ();
  pixmap = gdk_pixmap_create_from_xpm_d (window->window, &mask,
					 &style->bg[GTK_STATE_NORMAL],
					 (gchar **) xpm_pent2);
  pixmapwid = gtk_pixmap_new (pixmap, mask);
  gtk_widget_show (pixmapwid);
  gdk_pixmap_unref (pixmap);

  gtk_container_add (GTK_CONTAINER (pent2button), pixmapwid);
  gtk_box_pack_start (GTK_BOX (hbox), pent2button, (gboolean) FALSE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect (GTK_OBJECT (pent2button), "clicked",
		      GTK_SIGNAL_FUNC (Change_Angle), "3");
  gtk_tooltips_set_tip (tooltips, pent2button,
			_("Draw at 18/54/90/... degree angles"), NULL);
  gtk_widget_show (pent2button);

  octbutton = gtk_toggle_button_new ();
  pixmap = gdk_pixmap_create_from_xpm_d (window->window, &mask,
					 &style->bg[GTK_STATE_NORMAL],
					 (gchar **) xpm_octa);
  pixmapwid = gtk_pixmap_new (pixmap, mask);
  gtk_widget_show (pixmapwid);
  gdk_pixmap_unref (pixmap);

  gtk_container_add (GTK_CONTAINER (octbutton), pixmapwid);
  gtk_box_pack_start (GTK_BOX (hbox), octbutton, (gboolean) FALSE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect (GTK_OBJECT (octbutton), "clicked",
		      GTK_SIGNAL_FUNC (Change_Angle), "4");
  gtk_tooltips_set_tip (tooltips, octbutton,
			_("Draw at 0/45/90... degree angles"), NULL);
  gtk_widget_show (octbutton);

  splinebutton = gtk_toggle_button_new ();
  pixmap = gdk_pixmap_create_from_xpm_d (window->window, &mask,
					 &style->bg[GTK_STATE_NORMAL],
					 (gchar **) xpm_spline);
  pixmapwid = gtk_pixmap_new (pixmap, mask);
  gtk_widget_show (pixmapwid);
  gdk_pixmap_unref (pixmap);

  gtk_container_add (GTK_CONTAINER (splinebutton), pixmapwid);
  gtk_box_pack_start (GTK_BOX (hbox), splinebutton, (gboolean) FALSE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect (GTK_OBJECT (splinebutton), "clicked",
		      GTK_SIGNAL_FUNC (Splinemode), NULL);
  gtk_tooltips_set_tip (tooltips, splinebutton,
			_("Draw curves based on 4 control points"), NULL);
  gtk_widget_show (splinebutton);

  ltextbutton = gtk_toggle_button_new ();
  pixmap = gdk_pixmap_create_from_xpm_d (window->window, &mask,
					 &style->bg[GTK_STATE_NORMAL],
					 (gchar **) xpm_ltext);
  pixmapwid = gtk_pixmap_new (pixmap, mask);
  gtk_widget_show (pixmapwid);
  gdk_pixmap_unref (pixmap);

  gtk_container_add (GTK_CONTAINER (ltextbutton), pixmapwid);
  gtk_box_pack_start (GTK_BOX (hbox), ltextbutton, (gboolean) FALSE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect (GTK_OBJECT (ltextbutton), "clicked",
		      GTK_SIGNAL_FUNC (Change_Text), "0");
  gtk_tooltips_set_tip (tooltips, ltextbutton, _("Draw left-justified text"),
			NULL);
  gtk_widget_show (ltextbutton);

  ctextbutton = gtk_toggle_button_new ();
  pixmap = gdk_pixmap_create_from_xpm_d (window->window, &mask,
					 &style->bg[GTK_STATE_NORMAL],
					 (gchar **) xpm_mtext);
  pixmapwid = gtk_pixmap_new (pixmap, mask);
  gtk_widget_show (pixmapwid);
  gdk_pixmap_unref (pixmap);

  gtk_container_add (GTK_CONTAINER (ctextbutton), pixmapwid);
  gtk_box_pack_start (GTK_BOX (hbox), ctextbutton, (gboolean) FALSE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect (GTK_OBJECT (ctextbutton), "clicked",
		      GTK_SIGNAL_FUNC (Change_Text), "-1");
  gtk_tooltips_set_tip (tooltips, ctextbutton, _("Draw centered text"), NULL);
  gtk_widget_show (ctextbutton);


  rtextbutton = gtk_toggle_button_new ();
  pixmap = gdk_pixmap_create_from_xpm_d (window->window, &mask,
					 &style->bg[GTK_STATE_NORMAL],
					 (gchar **) xpm_rtext);
  pixmapwid = gtk_pixmap_new (pixmap, mask);
  gtk_widget_show (pixmapwid);
  gdk_pixmap_unref (pixmap);

  gtk_container_add (GTK_CONTAINER (rtextbutton), pixmapwid);
  gtk_box_pack_start (GTK_BOX (hbox), rtextbutton, (gboolean) FALSE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect (GTK_OBJECT (rtextbutton), "clicked",
		      GTK_SIGNAL_FUNC (Change_Text), "-2");
  gtk_tooltips_set_tip (tooltips, rtextbutton, _("Draw right-justified text"),
			NULL);
  gtk_widget_show (rtextbutton);

  fontbutton = gtk_button_new ();
  pixmap = gdk_pixmap_create_from_xpm_d (window->window, &mask,
					 &style->bg[GTK_STATE_NORMAL],
					 (gchar **) xpm_font);
  pixmapwid = gtk_pixmap_new (pixmap, mask);
  gtk_widget_show (pixmapwid);
  gdk_pixmap_unref (pixmap);
#if 0
fontselw=gtk_window_new(GTK_WINDOW_TOPLEVEL);
fontsel=gtk_font_selection_new();
gtk_widget_show(fontsel);
gtk_container_add(GTK_CONTAINER(fontselw),fontsel);
gtk_widget_realize(fontselw);
#endif
  gtk_container_add (GTK_CONTAINER (fontbutton), pixmapwid);
  gtk_box_pack_start (GTK_BOX (hbox), fontbutton, (gboolean) FALSE,
		      (gboolean) TRUE, 0);
#if 0
  (void)gtk_signal_connect_object (GTK_OBJECT (fontbutton), "clicked",
		      GTK_SIGNAL_FUNC(gtk_widget_show), GTK_OBJECT(fontselw));
#else
  (void)gtk_signal_connect (GTK_OBJECT (fontbutton), "clicked",
		      GTK_SIGNAL_FUNC (Change_Font), NULL);
#endif
  gtk_tooltips_set_tip (tooltips, fontbutton, _("Set current textfont"),
			NULL);
  gtk_widget_show (fontbutton);

  bondomenu = gtk_option_menu_new ();
  bondmenu = gtk_menu_new ();
  group = NULL;
  for (i = 0; i < BONDTYPES; i++)
    {
      int ii = i;

	menuitem = gtk_menu_item_new();
      pixmap = gdk_pixmap_create_from_xpm_d (window->window, &mask,
					     &style->bg[GTK_STATE_NORMAL],
					     (gchar **) xpm_bond[i]);
      pixmapwid = gtk_pixmap_new (pixmap, mask);
      gtk_widget_show (pixmapwid);
      gdk_pixmap_unref (pixmap);
      gtk_container_add (GTK_CONTAINER (menuitem), pixmapwid);
      gtk_container_set_border_width (GTK_CONTAINER (menuitem), 0);

      gtk_menu_append (GTK_MENU (bondmenu), menuitem);
      gtk_widget_show (menuitem);
      snprintf (bondnums[i],3, "%d", ii);
      (void)gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
			  GTK_SIGNAL_FUNC (set_bond), bondnums[i]);
    }
  gtk_option_menu_set_menu (GTK_OPTION_MENU (bondomenu), bondmenu);
  gtk_box_pack_start (GTK_BOX (hbox), bondomenu, (gboolean) FALSE,
		      (gboolean) TRUE, 0);
  gtk_tooltips_set_tip (tooltips, bondomenu, _("Choose default bond type"),
			NULL);
  gtk_widget_show (bondomenu);

  penmenu = gtk_menu_new ();
  group = NULL;
  for (i = 0; i < BONDCOLORS; i++)
    {
      int ii = i;

      menuitem = gtk_menu_item_new();
      pixmap = gdk_pixmap_create_from_xpm_d (window->window, &mask,
					     &style->bg[GTK_STATE_NORMAL],
					     (gchar **) xpm_color[i]);
      pixmapwid = gtk_pixmap_new (pixmap, mask);
      gtk_widget_show (pixmapwid);
      gdk_pixmap_unref (pixmap);
      gtk_container_add (GTK_CONTAINER (menuitem), pixmapwid);
      gtk_container_set_border_width (GTK_CONTAINER (menuitem), 0);

      gtk_menu_append (GTK_MENU (penmenu), menuitem);
      gtk_widget_show (menuitem);
      snprintf (bondcolors[i],3, "%d", ii);
      (void)gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
			  GTK_SIGNAL_FUNC (change_color), bondcolors[i]);
    }
  colorbutton= gtk_button_new();
  pixmap = gdk_pixmap_create_from_xpm_d (window->window, &mask,
					 &style->bg[GTK_STATE_NORMAL],
					 (gchar **) xpm_color);
  pixmapwid = gtk_pixmap_new (pixmap, mask);
  gtk_widget_show (pixmapwid);
  gdk_pixmap_unref (pixmap);
  gtk_container_add (GTK_CONTAINER (colorbutton), pixmapwid);
  gtk_box_pack_start (GTK_BOX (hbox), colorbutton, (gboolean) FALSE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect_object (GTK_OBJECT (colorbutton), "clicked",
		      GTK_SIGNAL_FUNC (show_penmenu), NULL);
  gtk_tooltips_set_tip (tooltips, colorbutton,
			_("Select pen color"),
			NULL);
  oldpixmap=pixmapwid;
  gtk_widget_show (colorbutton);

  bondbutton = gtk_toggle_button_new_with_label (_("Bonds"));
  gtk_box_pack_start (GTK_BOX (hbox), bondbutton, (gboolean) FALSE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect (GTK_OBJECT (bondbutton), "clicked",
		      GTK_SIGNAL_FUNC (Bondmode), NULL);
  gtk_tooltips_set_tip (tooltips, bondbutton, _("Toggle bond types"), NULL);
  gtk_widget_show (bondbutton);

  markbutton = gtk_toggle_button_new ();
  pixmap = gdk_pixmap_create_from_xpm_d (window->window, &mask,
					 &style->bg[GTK_STATE_NORMAL],
					 (gchar **) xpm_mark);
  pixmapwid = gtk_pixmap_new (pixmap, mask);
  gtk_widget_show (pixmapwid);
  gdk_pixmap_unref (pixmap);
  gtk_container_add (GTK_CONTAINER (markbutton), pixmapwid);
  gtk_box_pack_start (GTK_BOX (hbox), markbutton, (gboolean) FALSE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect (GTK_OBJECT (markbutton), "clicked",
		      GTK_SIGNAL_FUNC (Markmode), NULL);
  gtk_tooltips_set_tip (tooltips, markbutton,
			_("Mark objects for moving, rotating or deleting"),
			NULL);
  gtk_widget_show (markbutton);


  movebutton = gtk_toggle_button_new ();
  pixmap = gdk_pixmap_create_from_xpm_d (window->window, &mask,
					 &style->bg[GTK_STATE_NORMAL],
					 (gchar **) xpm_mov);
  pixmapwid = gtk_pixmap_new (pixmap, mask);
  gtk_widget_show (pixmapwid);
  gdk_pixmap_unref (pixmap);
  gtk_container_add (GTK_CONTAINER (movebutton), pixmapwid);
  gtk_box_pack_start (GTK_BOX (hbox), movebutton, (gboolean) FALSE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect (GTK_OBJECT (movebutton), "clicked",
		      GTK_SIGNAL_FUNC (Movemode), NULL);
  gtk_tooltips_set_tip (tooltips, movebutton, _("Move marked object"), NULL);
  gtk_widget_show (movebutton);



  rotatebutton = gtk_toggle_button_new ();
  pixmap = gdk_pixmap_create_from_xpm_d (window->window, &mask,
					 &style->bg[GTK_STATE_NORMAL],
					 (gchar **) xpm_rot);
  pixmapwid = gtk_pixmap_new (pixmap, mask);
  gtk_widget_show (pixmapwid);
  gdk_pixmap_unref (pixmap);
  gtk_container_add (GTK_CONTAINER (rotatebutton), pixmapwid);
  gtk_box_pack_start (GTK_BOX (hbox), rotatebutton, (gboolean) FALSE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect (GTK_OBJECT (rotatebutton), "clicked",
		      GTK_SIGNAL_FUNC (Rotatemode), NULL);
  gtk_tooltips_set_tip (tooltips, rotatebutton, _("Rotate marked object"),
			NULL);
  gtk_widget_show (rotatebutton);



  hflipbutton = gtk_button_new ();
  pixmap = gdk_pixmap_create_from_xpm_d (window->window, &mask,
					 &style->bg[GTK_STATE_NORMAL],
					 (gchar **) xpm_horiz);
  pixmapwid = gtk_pixmap_new (pixmap, mask);
  gtk_widget_show (pixmapwid);
  gdk_pixmap_unref (pixmap);
  gtk_container_add (GTK_CONTAINER (hflipbutton), pixmapwid);
  gtk_box_pack_start (GTK_BOX (hbox), hflipbutton, (gboolean) FALSE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect_object (GTK_OBJECT (hflipbutton), "clicked",
			     GTK_SIGNAL_FUNC (flip_horiz),
			     GTK_OBJECT (window));
  gtk_tooltips_set_tip (tooltips, hflipbutton, _("Flip object horizontally"),
			NULL);
  gtk_widget_show (hflipbutton);


  vflipbutton = gtk_button_new ();
  pixmap = gdk_pixmap_create_from_xpm_d (window->window, &mask,
					 &style->bg[GTK_STATE_NORMAL],
					 (gchar **) xpm_vert);
  pixmapwid = gtk_pixmap_new (pixmap, mask);
  gtk_widget_show (pixmapwid);
  gdk_pixmap_unref (pixmap);
  gtk_container_add (GTK_CONTAINER (vflipbutton), pixmapwid);
  gtk_box_pack_start (GTK_BOX (hbox), vflipbutton, (gboolean) FALSE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect_object (GTK_OBJECT (vflipbutton), "clicked",
			     GTK_SIGNAL_FUNC (flip_vert),
			     GTK_OBJECT (window));
  gtk_tooltips_set_tip (tooltips, vflipbutton, _("Flip object vertically"),
			NULL);
  gtk_widget_show (vflipbutton);



  copybutton = gtk_button_new ();
  pixmap = gdk_pixmap_create_from_xpm_d (window->window, &mask,
					 &style->bg[GTK_STATE_NORMAL],
					 (gchar **) xpm_copy);
  pixmapwid = gtk_pixmap_new (pixmap, mask);
  gtk_widget_show (pixmapwid);
  gdk_pixmap_unref (pixmap);
  gtk_container_add (GTK_CONTAINER (copybutton), pixmapwid);
  gtk_box_pack_start (GTK_BOX (hbox), copybutton, (gboolean) FALSE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect_object (GTK_OBJECT (copybutton), "clicked",
			     GTK_SIGNAL_FUNC (copy_obj), GTK_OBJECT (window));
  (void)gtk_signal_connect_object (GTK_OBJECT (copybutton), "clicked",
			     GTK_SIGNAL_FUNC (gtk_toggle_button_set_active),
			     GTK_OBJECT (movebutton));
#ifdef MENU
  (void)gtk_signal_connect_object (GTK_OBJECT (copy), "activate",
			     GTK_SIGNAL_FUNC (gtk_toggle_button_set_active),
			     GTK_OBJECT (movebutton));
#endif
  gtk_tooltips_set_tip (tooltips, copybutton, _("Copy marked object"), NULL);
  gtk_widget_show (copybutton);

  rescalebutton = gtk_toggle_button_new ();
  pixmap = gdk_pixmap_create_from_xpm_d (window->window, &mask,
					 &style->bg[GTK_STATE_NORMAL],
					 (gchar **) xpm_scale);
  pixmapwid = gtk_pixmap_new (pixmap, mask);
  gtk_widget_show (pixmapwid);
  gdk_pixmap_unref (pixmap);
  gtk_container_add (GTK_CONTAINER (rescalebutton), pixmapwid);
  gtk_box_pack_start (GTK_BOX (hbox), rescalebutton, (gboolean) FALSE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect (GTK_OBJECT (rescalebutton), "clicked",
		      GTK_SIGNAL_FUNC (Rescalemode), NULL);
  gtk_tooltips_set_tip (tooltips, rescalebutton, _("Rescale marked object"),
			NULL);
  gtk_widget_show (rescalebutton);

  boxmenu = gtk_menu_new ();
  bracketbutton = gtk_menu_item_new ();
  pixmap = gdk_pixmap_create_from_xpm_d (window->window, &mask,
					 &style->bg[GTK_STATE_NORMAL],
					 (gchar **) xpm_bracket);
  pixmapwid = gtk_pixmap_new (pixmap, mask);
  gtk_widget_show (pixmapwid);
  gdk_pixmap_unref (pixmap);
  gtk_container_add (GTK_CONTAINER (bracketbutton), pixmapwid);

  gtk_menu_append (GTK_MENU (boxmenu), bracketbutton);
  (void)gtk_signal_connect_object (GTK_OBJECT (bracketbutton), "activate",
			     GTK_SIGNAL_FUNC (add_bracket),
			     GTK_OBJECT (window));
  gtk_tooltips_set_tip (tooltips, bracketbutton,
			_("Draw brackets around object"), NULL);
  gtk_widget_show (bracketbutton);
  rbracketbutton = gtk_menu_item_new ();
  pixmap = gdk_pixmap_create_from_xpm_d (window->window, &mask,
					 &style->bg[GTK_STATE_NORMAL],
					 (gchar **) xpm_r_bracket);
  pixmapwid = gtk_pixmap_new (pixmap, mask);
  gtk_widget_show (pixmapwid);
  gdk_pixmap_unref (pixmap);
  gtk_container_add (GTK_CONTAINER (rbracketbutton), pixmapwid);
  gtk_menu_append (GTK_MENU (boxmenu), rbracketbutton);
  (void)gtk_signal_connect_object (GTK_OBJECT (rbracketbutton), "activate",
			     GTK_SIGNAL_FUNC (add_r_bracket),
			     GTK_OBJECT (window));
  gtk_tooltips_set_tip (tooltips, rbracketbutton,
			_("Draw rounded brackets around object"), NULL);
  gtk_widget_show (rbracketbutton);
  r2bracketbutton = gtk_menu_item_new ();
  pixmap = gdk_pixmap_create_from_xpm_d (window->window, &mask,
					 &style->bg[GTK_STATE_NORMAL],
					 (gchar **) xpm_r2_bracket);
  pixmapwid = gtk_pixmap_new (pixmap, mask);
  gtk_widget_show (pixmapwid);
  gdk_pixmap_unref (pixmap);
  gtk_container_add (GTK_CONTAINER (r2bracketbutton), pixmapwid);
  gtk_menu_append (GTK_MENU (boxmenu), r2bracketbutton);
  (void)gtk_signal_connect_object (GTK_OBJECT (r2bracketbutton), "activate",
			     GTK_SIGNAL_FUNC (add_r2_bracket),
			     GTK_OBJECT (window));
  gtk_tooltips_set_tip (tooltips, r2bracketbutton,
			_("Draw round brackets around object"), NULL);
  gtk_widget_show (r2bracketbutton);
  bracebutton = gtk_menu_item_new ();
  pixmap = gdk_pixmap_create_from_xpm_d (window->window, &mask,
					 &style->bg[GTK_STATE_NORMAL],
					 (gchar **) xpm_brace);
  pixmapwid = gtk_pixmap_new (pixmap, mask);
  gtk_widget_show (pixmapwid);
  gdk_pixmap_unref (pixmap);
  gtk_container_add (GTK_CONTAINER (bracebutton), pixmapwid);

  gtk_menu_append (GTK_MENU (boxmenu), bracebutton);
  (void)gtk_signal_connect_object (GTK_OBJECT (bracebutton), "activate",
			     GTK_SIGNAL_FUNC (add_brace),
			     GTK_OBJECT (window));
  gtk_tooltips_set_tip (tooltips, bracebutton, _("Draw braces around object"),
			NULL);
  gtk_widget_show (bracebutton);
  box1button = gtk_menu_item_new ();
  pixmap = gdk_pixmap_create_from_xpm_d (window->window, &mask,
					 &style->bg[GTK_STATE_NORMAL],
					 (gchar **) xpm_box1);
  pixmapwid = gtk_pixmap_new (pixmap, mask);
  gtk_widget_show (pixmapwid);
  gdk_pixmap_unref (pixmap);
  gtk_container_add (GTK_CONTAINER (box1button), pixmapwid);
  gtk_menu_append (GTK_MENU (boxmenu), box1button);
  (void)gtk_signal_connect_object (GTK_OBJECT (box1button), "activate",
			     GTK_SIGNAL_FUNC (add_box1), GTK_OBJECT (window));
  gtk_tooltips_set_tip (tooltips, box1button,
			_("Draw simple box around object"), NULL);
  gtk_widget_show (box1button);
  box2button = gtk_menu_item_new ();
  pixmap = gdk_pixmap_create_from_xpm_d (window->window, &mask,
					 &style->bg[GTK_STATE_NORMAL],
					 (gchar **) xpm_box2);
  pixmapwid = gtk_pixmap_new (pixmap, mask);
  gtk_widget_show (pixmapwid);
  gdk_pixmap_unref (pixmap);
  gtk_container_add (GTK_CONTAINER (box2button), pixmapwid);
  gtk_menu_append (GTK_MENU (boxmenu), box2button);
  (void)gtk_signal_connect_object (GTK_OBJECT (box2button), "activate",
			     GTK_SIGNAL_FUNC (add_box2), GTK_OBJECT (window));
  gtk_tooltips_set_tip (tooltips, box2button,
			_("Draw shaded box around object"), NULL);
  gtk_widget_show (box2button);
  box3button = gtk_menu_item_new ();
  pixmap = gdk_pixmap_create_from_xpm_d (window->window, &mask,
					 &style->bg[GTK_STATE_NORMAL],
					 (gchar **) xpm_box3);
  pixmapwid = gtk_pixmap_new (pixmap, mask);
  gtk_widget_show (pixmapwid);
  gdk_pixmap_unref (pixmap);
  gtk_container_add (GTK_CONTAINER (box3button), pixmapwid);
  gtk_menu_append (GTK_MENU (boxmenu), box3button);
  (void)gtk_signal_connect_object (GTK_OBJECT (box3button), "activate",
			     GTK_SIGNAL_FUNC (add_box3), GTK_OBJECT (window));
  gtk_tooltips_set_tip (tooltips, box3button,
			_("Draw fancy box around object"), NULL);
  gtk_widget_show (box3button);
  box4button = gtk_menu_item_new ();
  pixmap = gdk_pixmap_create_from_xpm_d (window->window, &mask,
					 &style->bg[GTK_STATE_NORMAL],
					 (gchar **) xpm_box4);
  pixmapwid = gtk_pixmap_new (pixmap, mask);
  gtk_widget_show (pixmapwid);
  gdk_pixmap_unref (pixmap);
  gtk_container_add (GTK_CONTAINER (box4button), pixmapwid);
  gtk_menu_append (GTK_MENU (boxmenu), box4button);
  (void)gtk_signal_connect_object (GTK_OBJECT (box4button), "activate",
			     GTK_SIGNAL_FUNC (add_box4), GTK_OBJECT (window));
  gtk_tooltips_set_tip (tooltips, box4button,
			_("Draw rounded box around object"), NULL);
  gtk_widget_show (box4button);
  gtk_widget_realize (boxmenu);

  boxbutton = gtk_button_new ();
  pixmap = gdk_pixmap_create_from_xpm_d (window->window, &mask,
					 &style->bg[GTK_STATE_NORMAL],
					 (gchar **) xpm_bracket);
  pixmapwid = gtk_pixmap_new (pixmap, mask);
  gtk_widget_show (pixmapwid);
  gdk_pixmap_unref (pixmap);
  gtk_container_add (GTK_CONTAINER (boxbutton), pixmapwid);
  gtk_box_pack_start (GTK_BOX (hbox), boxbutton, (gboolean) FALSE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect_object (GTK_OBJECT (boxbutton), "clicked",
			     GTK_SIGNAL_FUNC (show_boxmenu), NULL);
  gtk_tooltips_set_tip (tooltips, boxbutton,
			_("Draw brackets and boxes around object"), NULL);
  gtk_widget_show (boxbutton);

  cleanbutton = gtk_button_new ();
  pixmap = gdk_pixmap_create_from_xpm_d (window->window, &mask,
					 &style->bg[GTK_STATE_NORMAL],
					 (gchar **) xpm_clean);
  pixmapwid = gtk_pixmap_new (pixmap, mask);
  gtk_widget_show (pixmapwid);
  gdk_pixmap_unref (pixmap);
  gtk_container_add (GTK_CONTAINER (cleanbutton), pixmapwid);
  gtk_box_pack_start (GTK_BOX (hbox), cleanbutton, (gboolean) FALSE,
		      (gboolean) TRUE, 0);
  (void)gtk_signal_connect (GTK_OBJECT (cleanbutton), "clicked",
		      GTK_SIGNAL_FUNC (tidy_mol), NULL);
  gtk_tooltips_set_tip (tooltips, cleanbutton,
			_("Removes duplicate bonds, etc."), NULL);

  gtk_widget_show (cleanbutton);

  gtk_box_pack_start (GTK_BOX (vbox), hbox, (gboolean) FALSE, (gboolean) TRUE,
		      0);

  gtk_widget_show (hbox);


/*end second row */
  hbox=gtk_hbox_new(FALSE,0);
  normalfontstyle=gtk_style_copy(gtk_widget_get_default_style());
  seriffontstyle=gtk_style_copy(gtk_widget_get_default_style());
#ifdef GTK2
  pango_font_description_free(normalfontstyle->font_desc);
  normalfontstyle->font_desc=pango_font_description_from_string("Helvetica Bold 12");
  pango_font_description_free(seriffontstyle->font_desc);
  seriffontstyle->font_desc=pango_font_description_from_string("Times Medium 12");
#else 
  gdk_font_unref(normalfontstyle->font);
  normalfontstyle->font=gdk_font_load("*-helvetica-bold-r-normal--12-*");
  gdk_font_unref(seriffontstyle->font);
  seriffontstyle->font=gdk_font_load("*-times-medium-r-normal--12-*");
  if (!normalfontstyle->font)
  	normalfontstyle=gtk_style_copy(gtk_widget_get_default_style());
  if (!seriffontstyle->font)
  	seriffontstyle=gtk_style_copy(gtk_widget_get_default_style());
#endif
  textlabel = gtk_label_new(_("Text :"));
  gtk_widget_set_style(GTK_WIDGET(textlabel),normalfontstyle);
  gtk_widget_show(textlabel);
  gtk_box_pack_start (GTK_BOX (hbox), textlabel, (gboolean) FALSE,
		      (gboolean) TRUE, 0);
  textbox = gtk_entry_new_with_max_length (100);
  gtk_widget_set_usize(textbox,700,20);
  gtk_box_pack_start (GTK_BOX (hbox), textbox, (gboolean) FALSE,
		      (gboolean) FALSE, 0);
  (void)gtk_signal_connect (GTK_OBJECT (textbox), "focus_in_event",
		      (GtkSignalFunc) Set_Textmode, NULL);

  gtk_tooltips_set_tip (tooltips, textbox,
			_
			("Prefix a character with _ for subscript, ^ for superscript, @ for symbols, | for italic, # for bold text; e.g. H_2O, @a_D^2^0"),
			NULL);
  gtk_widget_show (textbox);

  fontmenu = gtk_option_menu_new ();
 gtk_tooltips_set_tip (tooltips, fontmenu, _("Select current text size"), NULL);
  fontsizes = gtk_menu_new ();
  for (i = 0; i < 7; i++)
    {
      fontsizeitem[i] = gtk_menu_item_new_with_label (fontsizelabel[i]);
      gtk_menu_append (GTK_MENU (fontsizes), fontsizeitem[i]);
      gtk_widget_show (fontsizeitem[i]);
      (void)gtk_signal_connect (GTK_OBJECT (fontsizeitem[i]), "activate",
				 GTK_SIGNAL_FUNC (set_fontsize),
				 GINT_TO_POINTER(i));
    }
  gtk_option_menu_set_menu (GTK_OPTION_MENU (fontmenu), fontsizes);
  gtk_widget_show (fontmenu);
  gtk_option_menu_set_history(GTK_OPTION_MENU(fontmenu), (guint)3);
  gtk_box_pack_start (GTK_BOX (hbox), fontmenu,
		      (gboolean) FALSE, (gboolean) FALSE, 0);

  gtk_box_pack_start (GTK_BOX (vbox), hbox, (gboolean) FALSE, (gboolean) FALSE,
		      0);
  gtk_widget_show (hbox);

/* drawing area */
/***********************the canvas pixmap ***********************/
  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_set_border_width (GTK_CONTAINER (scrolled_window), 10);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
				  GTK_POLICY_ALWAYS, GTK_POLICY_ALWAYS);
  gtk_box_pack_start (GTK_BOX (vbox), scrolled_window, (gboolean) TRUE,
		      (gboolean) TRUE, 0);
  gtk_widget_show (scrolled_window);

/*fprintf(stderr,"screen dimensions %d x %d\n",gdk_screen_width(),gdk_screen_height() );*/
  int canvash=600;
  if (gdk_screen_height() < 600) canvash= 460;
  
  gtk_widget_set_usize (window, 800, canvash-20);

  if (!picture)
    picture = gdk_pixmap_new (window->window, 800, canvash, -1);

  drawing_area = gtk_drawing_area_new ();
  gtk_drawing_area_size (GTK_DRAWING_AREA (drawing_area), 1600, 1600);

  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW
					 (scrolled_window), drawing_area);

  gtk_widget_show (drawing_area);

  mygc[0]=gdk_gc_new(window->window);
  (void)gdk_color_alloc(gdk_colormap_get_system(),&black);
  gdk_gc_set_foreground(mygc[0],&black);

  mygc[1]=gdk_gc_new(window->window);
  (void)gdk_color_alloc(gdk_colormap_get_system(),&blue);
  gdk_gc_set_foreground(mygc[1],&blue);

  mygc[2]=gdk_gc_new(window->window);
  (void)gdk_color_alloc(gdk_colormap_get_system(),&green);
  gdk_gc_set_foreground(mygc[2],&green);

  mygc[3]=gdk_gc_new(window->window);
  (void)gdk_color_alloc(gdk_colormap_get_system(),&cyan);
  gdk_gc_set_foreground(mygc[3],&cyan);

  mygc[4]=gdk_gc_new(window->window);
  (void)gdk_color_alloc(gdk_colormap_get_system(),&red);
  gdk_gc_set_foreground(mygc[4],&red);

  mygc[5]=gdk_gc_new(window->window);
  (void)gdk_color_alloc(gdk_colormap_get_system(),&magenta);
  gdk_gc_set_foreground(mygc[5],&magenta);

  mygc[6]=gdk_gc_new(window->window);
  (void)gdk_color_alloc(gdk_colormap_get_system(),&yellow);
  gdk_gc_set_foreground(mygc[6],&yellow);
  
  mygc[7]=gdk_gc_new(window->window);
  (void)gdk_color_alloc(gdk_colormap_get_system(),&white);
  gdk_gc_set_foreground(mygc[7],&white);

/* highlighting for mark/move/copy */
  hlgc=gdk_gc_new(window->window);
  gdk_gc_set_foreground(hlgc,&blue);
  gdk_gc_set_line_attributes(hlgc,
                             3,GDK_LINE_SOLID,GDK_CAP_BUTT,GDK_JOIN_MITER);

  /* Signals used to handle backing pixmap */

  (void)gtk_signal_connect (GTK_OBJECT (drawing_area), "expose_event",
		      (GtkSignalFunc) expose_event, NULL);
  (void)gtk_signal_connect (GTK_OBJECT (drawing_area), "configure_event",
		      (GtkSignalFunc) configure_event, NULL);

  (void)gtk_signal_connect (GTK_OBJECT (drawing_area), "motion_notify_event",
		      (GtkSignalFunc) motion_notify_event, NULL);

  (void)gtk_signal_connect (GTK_OBJECT (drawing_area), "button_press_event",
		      (GtkSignalFunc) button_press_event, NULL);

  (void)gtk_signal_connect (GTK_OBJECT (drawing_area), "button_release_event",
		      (GtkSignalFunc) button_release_event, NULL);

  (void)gtk_signal_connect (GTK_OBJECT (window), "key_press_event",
		      (GtkSignalFunc) key_press_event, NULL);

  gtk_widget_set_events (drawing_area, GDK_EXPOSURE_MASK
			 | GDK_LEAVE_NOTIFY_MASK
			 | GDK_BUTTON_PRESS_MASK
			 | GDK_BUTTON_RELEASE_MASK
			 | GDK_POINTER_MOTION_MASK
			 | GDK_POINTER_MOTION_HINT_MASK);
   
/*status line - used for cht output*/
  hbox = gtk_hbox_new (FALSE, 5);
  gtk_widget_set_usize (hbox, 800, 20);
#ifdef GTK2
  msgtext = gtk_text_view_new();
  gtk_text_view_set_editable(GTK_TEXT_VIEW(msgtext), (gboolean) FALSE);
  msgtextbuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (msgtext));
#else    
  msgtext = gtk_text_new (NULL, NULL);
  gtk_text_set_editable (GTK_TEXT (msgtext), (gboolean) FALSE);
  gtk_box_pack_start (GTK_BOX (hbox), msgtext, (gboolean) TRUE,
		      (gboolean) TRUE, 0);
#endif

#ifdef GTK2
  vscroll = gtk_scrolled_window_new(NULL,NULL);
  msgadjustment = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW(vscroll));
  gtk_widget_set_usize(vscroll,790,12);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(vscroll),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
  gtk_container_add(GTK_CONTAINER(vscroll),msgtext);                                
#else
  vscroll = gtk_vscrollbar_new (GTK_TEXT (msgtext)->vadj);
#endif

  gtk_box_pack_start (GTK_BOX (hbox), vscroll, (gboolean) FALSE,
		      (gboolean) FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, (gboolean) FALSE,
		      (gboolean) FALSE, 0);
  snprintf(msgtmp,99,_("Ready"));
#ifdef GTK2
  gtk_text_buffer_get_iter_at_offset (msgtextbuffer, &iter, 0);
  gtk_text_buffer_insert (msgtextbuffer, &iter, msgtmp, -1);
#else   
  gtk_text_insert (GTK_TEXT (msgtext), NULL, NULL, NULL, msgtmp,(gint)strlen(msgtmp));
#endif
  gtk_widget_show (msgtext);
  gtk_widget_show (vscroll);
  gtk_widget_show (hbox);
  gtk_widget_show (window);
  set_icon(window);
/* initialize cursor shapes */
  source =
    gdk_bitmap_create_from_data (NULL, (gchar *) pencil_bits, pencil_width,
				 pencil_height);
  mask =
    gdk_bitmap_create_from_data (NULL, (gchar *) pencil_mask_bits,
				 pencil_width, pencil_height);
  cursor_pencil =
    gdk_cursor_new_from_pixmap (source, mask, &fg, &bg, pencil_x_hot,
				pencil_y_hot);
  gdk_pixmap_unref (source);
  gdk_pixmap_unref (mask);
  source =
    gdk_bitmap_create_from_data (NULL, (gchar *) text_bits, text_width,
				 text_height);
  mask =
    gdk_bitmap_create_from_data (NULL, (gchar *) text_mask_bits, text_width,
				 text_height);
  cursor_text =
    gdk_cursor_new_from_pixmap (source, mask, &fg, &bg, text_x_hot,
				text_y_hot);
  gdk_pixmap_unref (source);
  gdk_pixmap_unref (mask);
  source =
    gdk_bitmap_create_from_data (NULL, (gchar *) bonds_bits, bonds_width,
				 bonds_height);
  mask =
    gdk_bitmap_create_from_data (NULL, (gchar *) bonds_mask_bits, bonds_width,
				 bonds_height);
  cursor_bonds =
    gdk_cursor_new_from_pixmap (source, mask, &fg, &wh, bonds_x_hot,
				bonds_y_hot);
  gdk_pixmap_unref (source);
  gdk_pixmap_unref (mask);
  cursor_markTLC = gdk_cursor_new (GDK_TOP_LEFT_CORNER);
  cursor_markBRC = gdk_cursor_new (GDK_BOTTOM_RIGHT_CORNER);
  cursor_move = gdk_cursor_new (GDK_FLEUR);
  cursor_rescale = gdk_cursor_new (GDK_SIZING);
  cursor_rotate = gdk_cursor_new (GDK_EXCHANGE);
/*  cursor_busy = gdk_cursor_new (GDK_WATCH); */

/*@ignore@ splint does not know all signal names */
  (void) signal (SIGSEGV, ct_crash);
  (void) signal (SIGFPE, ct_crash);
  (void) signal (SIGBUS, ct_crash);
  (void) signal (SIGHUP, ct_crash);
  (void) signal (SIGPIPE, SIG_IGN); 
/*@end@*/

  if (!Load_Font() )
    {
      fprintf (stderr, _("chemtool: can't load any font\n"));
      exit (1);
    }

  gtk_toggle_button_set_active ((GtkToggleButton *) hexbutton, TRUE);
  /* default to linedrawing at hexagonal angles */
  text_direct = -1;		/* and centered text */
  expmode = 0;			/*default to xfig export */
  orient = 0;
  printcmd = 0;
  epsoption = 0;
  use_whiteout = 0;
  use_intlchars = 0;
  strcpy (filename, _("unnamed"));
  gtk_file_selection_set_filename (GTK_FILE_SELECTION (expw), filename);
  printscale = 0.7;
  papersize = 0;
  queuename = getenv ("PRINTER");
  if (queuename == (char *) NULL)
    {
      queuename = malloc (33 * sizeof (char));
      strcpy (queuename, "lp");
    }
  if (figversion ==0)  {

#ifdef MENU
  	gtk_widget_set_sensitive(printps,FALSE);
#else 
	gtk_widget_set_sensitive(printbutton,FALSE);
#endif	
  	
	gtk_widget_set_sensitive(expbutton[0],FALSE);
	gtk_widget_set_sensitive(expbutton[1],FALSE);
	gtk_widget_set_sensitive(expbutton[2],FALSE);
	gtk_widget_set_sensitive(expbutton[8],FALSE);
	}
#ifndef EMF
  if (figversion >=3 )gtk_widget_show (expbutton[6]);
#endif
  if (have_fig2sxd) gtk_widget_show(expbutton[7]);
  bgred=bgblue=bggreen=65535;
  readrc ();
  background.red=(gushort)bgred;
  background.green=(gushort)bggreen;
  background.blue=(gushort)bgblue;
  background_gc=gdk_gc_new(window->window);
  (void)gdk_color_alloc(gdk_colormap_get_system(),&background);
  gdk_gc_set_foreground(background_gc,&background);
  gdk_gc_set_background(background_gc,&background);
  snprintf(bghexcolor,10,"#%2.2x%2.2x%2.2x",(unsigned char)(bgred/256.),(unsigned char)(bggreen/256.),(unsigned char)(bgblue/256.));
  
#ifndef GTK2
  gtk_label_set_text(GTK_LABEL(GTK_BUTTON(bgcolorbutton)->child),bghexcolor);
  gtk_entry_set_text (GTK_ENTRY (prqueue), queuename);
  gtk_option_menu_set_history (GTK_OPTION_MENU (papermenu), (guint)papersize);
  gtk_option_menu_set_history (GTK_OPTION_MENU (printcmdmenu), (guint)printcmd);
  gtk_option_menu_set_history (GTK_OPTION_MENU (epsoptionmenu), (guint) epsoption);
  gtk_spin_button_set_value ((GtkSpinButton *) pscale, printscale * 100.);
  if (use_whiteout==1) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(whiteoutbutton),TRUE);
  if (use_intlchars==1) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(intlbutton),TRUE);
  snprintf(msgtmp,100,"%6.4f",bondlen_mm);
  gtk_entry_set_text (GTK_ENTRY (base_bondlen),msgtmp);
  if (datadir[0] != '\0')
  {
    gtk_entry_set_text (GTK_ENTRY (defaultdir), datadir);
    gtk_entry_set_text (GTK_ENTRY (defaultext), datamask);
    gtk_file_selection_complete (GTK_FILE_SELECTION (filew),
                                 strcat (datadir, datamask));
    strcpy (datadir, gtk_entry_get_text (GTK_ENTRY (defaultdir)));
  }
#else              
  if (datadir[0] != '\0')
    {
      char dir_mask[PATH_MAX];
      strcpy (dir_mask, datadir);
      strcat (dir_mask, datamask);
      gtk_file_selection_complete (GTK_FILE_SELECTION (filew), dir_mask);
    }
#endif

  mark.flag = False;
  head.width = 2000;
  head.height = 5000;
  zoom_factor = 2;
  pdbx = pdby = pdbz = NULL;
  atcode = NULL;
  atjust = bondtype = NULL;
  bondfrom = bondto = NULL;
  hp->x = hp->y = 200;
  hp->tx = hp->ty = 200;
  tmpx=(int*)NULL;
  tmpy=(int*)NULL;
  gridtype=0;
  gridx=gridy = 0;
  atnum = 0;
  if (argc == 2)
    {
      strcpy (filename, argv[1]);
if (strstr(argv[0],"chemtoolsvg")) {
      load_mol(filename);
      strcpy(expname,filename);
      char *dot=strrchr(expname,'.');
      if (dot) *dot='\0';
      strcat(expname,".svg");
      export_svg(expname);
      exit(0);
}  
#if 0
      if(strrchr(filename,'/')) { /* change working directory to obtain absolute path */
      char dirname[256];
        strcpy(dirname,filename);  /* and strip path from filename - gtkfilesel may */
        char *start= strrchr(dirname,'/');
        *start='\0'; /* get confused by relative paths, stripping off */
        start++;
        chdir(dirname);            /* a level with every invocation of the file dialog */
        memmove(filename,start,strlen(start)+1);
      }
#endif
      error = load_mol (filename);
      switch (error)
	{
	case 0:
	  modify = 0;
	  mark.flag = 0;
	  snprintf (msgtmp,99, "Chemtool %s (%s)", VERSION, filename);
	  gtk_window_set_title (GTK_WINDOW (window), msgtmp);
#ifdef LIBUNDO
	  undo_snapshot ();
#endif
	  break;
	case 1:
	  snprintf (msgtmp,99, _("Unable to open %s\n"), filename);
	  strcpy (filename, "");
	  gtk_label_set_text (GTK_LABEL (message), msgtmp);
	  gtk_widget_show (messagew);
	  gtk_grab_add (messagew);
	  break;
	case 2:
	  snprintf (msgtmp,99, _("%s\n does not appear to be a Chemtool file\n"),
		   filename);
	  strcpy (filename, "");
	  gtk_label_set_text (GTK_LABEL (message), msgtmp);
	  gtk_widget_show (messagew);
	  gtk_grab_add (messagew);
	  break;
	case 3:
	  modify = 0;
	  snprintf (msgtmp,99,
		   _("%s was created by a newer version.\nSome features may be lost.\n"),
		   filename);
	  gtk_label_set_text (GTK_LABEL (message), msgtmp);
	  gtk_widget_show (messagew);
	  gtk_grab_add (messagew);
	  snprintf (msgtmp,100, "Chemtool %s (%s)", VERSION, filename);
	  gtk_window_set_title (GTK_WINDOW (window), msgtmp);
#ifdef LIBUNDO
	  undo_snapshot ();
#endif
	  break;
	default:
	  clear_data ();
	  snprintf (msgtmp,100, _("Error loading %s \n"), filename);
	  gtk_label_set_text (GTK_LABEL (message), msgtmp);
	  gtk_widget_show (messagew);
	  gtk_grab_add (messagew);
	}
    }
  FreePix ();
  CreatePix ();
  Display_Mol ();

  gtk_main ();			/* enter the gtk eventloop */

  return 0;
}

void
show_boxmenu ()
{
  gtk_menu_popup (GTK_MENU (boxmenu), NULL, NULL, NULL, NULL, 0, 0);
}

void
show_penmenu ()
{
  gtk_menu_popup (GTK_MENU (penmenu), NULL, NULL, NULL, NULL, 0, 0);
}

void
ct_crash (int sig)
{
  FILE *fp;
  fp = fopen ("crashdump.cht", "w");
  save_mol (fp, 0);
  switch (sig)
    {
    case SIGSEGV:
      fprintf (stderr, _("Memory allocation problem (SIGSEGV) -"));
      break;
    case SIGFPE:
      fprintf (stderr, _("Invalid math somewhere (SIGFPE) -"));
      break;
    case SIGBUS:
      fprintf (stderr, _("Memory access problem (SIGBUS) -"));
      break;
    case SIGHUP:
      fprintf (stderr, _("Ordered to quit (SIGHUP) - "));
    }
  fprintf (stderr, _(" dumping current drawing to file crashdump.cht\n"));
  fclose (fp);
  exit (1);
}

void
restore_picture ()
{
  if (savedpicture)
    picture = gdk_pixmap_ref(savedpicture);
}


void
getpreview (GtkWidget * w, gint row, gint column, GdkEventButton * bevent,
	    gpointer data)
{
  (void) w;
  (void) row;
  (void) column;
  (void) data;
  char myfile[255];

  if (bevent && bevent->type == GDK_2BUTTON_PRESS)
    return;
  if (loadsave != 1 && loadsave != 3 && loadsave != 4)
    return;

  strcpy (myfile,
	  gtk_file_selection_get_filename (GTK_FILE_SELECTION (filew)));
  if (picture){
    gdk_pixmap_unref (picture);
	picture=NULL;
	}
  picture = gdk_pixmap_new (filew->window, 200, 100, -1);

  gdk_draw_rectangle (picture,
		      filew->style->white_gc,
		      (gint) TRUE, 0, 0, (gint) 200, (gint) 100);

  if (loadsave == 4) {
    sdfindex = 0;
    preview_mdl_mol(myfile,sdfindex);
    } else
  load_preview (myfile);
}

void
 set_icon(GtkWidget *w) {
 /* Set icon for the main chemtool window */
   GdkWindow *win;
   GdkPixmap *icon;
   GdkBitmap *iconmask;
 
   gtk_widget_realize(w);
   win = w->window;
   if (!win) return;
 
   icon = gdk_pixmap_create_from_xpm_d(win, &iconmask, NULL, chemtool_xpm);
   gdk_window_set_icon(win, NULL, icon, iconmask);
 }

#pragma GCC diagnostic pop

