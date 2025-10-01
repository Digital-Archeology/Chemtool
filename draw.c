/*
 *    draw functions inside the pixmap
 */

#include <string.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include "ct1.h"
#pragma GCC diagnostic pop
void
Add_atom (int event_x, int event_y)
/* reads the string from the text entry box and inserts it at the current
   cursor position in the current justification */
{
  struct xy_co *coord;
  struct dc *hpc;
  gchar *name;
  int x, y;
  int i;

  name = (char*)gtk_entry_get_text (GTK_ENTRY (textbox)); 
  if (strlen (name))
    {
      x = round_coord (event_x, size_factor);
      y = round_coord (event_y, size_factor);
      coord = position (x, y, x, y, No_angle);
      for (i = 0; i < (int)strlen (name); i++)
	{
	  if (name[i] == ' ')
	    name[i] = '\\';
	  if (name[i] == '*' && i > 0)
	    {			/* convert @* to @+dotsymbol */
	      if (name[i - 1] == '@')
		name[i] = '\267';
	    }
	}
      del_char (select_char (event_x, event_y, size_factor));
      add_char (coord->x, coord->y, name, text_direct, 0, curpen, serif_flag, curfontsize);
      modify = True;
      Display_Mol ();
    } else {
    hpc = select_char (event_x, event_y, size_factor);
	if (hpc) {
		hpc->direct=text_direct;
		hpc->color=curpen;
		hpc->font=serif_flag;
		hpc->size=curfontsize;
     		modify = True;
      		Display_Mol ();
	}
    } 
}

void
Fetch_atom (int event_x, int event_y)
/* loads a label from the current cursor position into the text entry box */
{
  struct dc *hpc;

  hpc = select_char (event_x, event_y, size_factor);
  if (hpc)
    gtk_entry_set_text (GTK_ENTRY (textbox), hpc->c);
}


void
Del_atom (int event_x, int event_y)
/* deletes the label at the current cursor position */
{
  del_char (select_char (event_x, event_y, size_factor));
  modify = True;
  Display_Mol ();
}

void
Set_vector (int event_x, int event_y, int curbond)
/* draws a circle of transient marker boxes corresponding to the current
   grid angles around the cursor position */
{
  int angle_count[] = { 0, 12, 10, 10, 8 };
  int d, dmax;
  struct xy_co *coord;

  hp->x = round_coord (event_x, size_factor);
  hp->y = round_coord (event_y, size_factor);
  if (curbond != 8){ /* snap to close bonds unless half arrow is drawn */
  coord = position (hp->x, hp->y, hp->x, hp->y, draw_angle);
  hp->x = coord->x;
  hp->y = coord->y;
  }
  hp->tx = hp->x;
  hp->ty = hp->y;
  dmax = angle_count[draw_angle];
  for (d = 0; d < dmax; d++)
    {
      coord = calc_angle (hp->x, hp->y, draw_angle, d, 1);
      gdk_draw_rectangle (picture, drawing_area->style->black_gc,
			  TRUE, coord->x * size_factor - 2,
			  coord->y * size_factor - 2, 4, 4);
#if 0
/* draw a second ring of markers farther out */
      gdk_draw_rectangle (picture, drawing_area->style->black_gc,
			  TRUE, (2*coord->x-hp->x)* size_factor - 2,
			  (2*coord->y-hp->y)*size_factor - 2, 4, 4);
      gdk_draw_rectangle (picture, drawing_area->style->black_gc,
			  TRUE, (3*coord->x-2*hp->x)* size_factor - 2,
			  (3*coord->y-2*hp->y)*size_factor - 2, 4, 4);
#endif
    }
  CopyPlane ();
}

void
Add_vector (int cbond)
/* adds a bond vector from the previous to the current cursor position */
{
  if ((hp->x - hp->tx) || (hp->y - hp->ty))
    {
      add_struct (hp->x, hp->y, hp->tx, hp->ty, cbond, 0, 0, 0,curpen);
  modify = True;
    }
  Display_Mol ();
}

void
Add_ring (int ring_angle, int curbond, int ringtype)
/* adds a polygon clockwise around the previous and current position */
{
  double length;		/* length of the drawn bond */
  double radius;		/* the radius of the polygon */
  int type;			/* type of ring */
  int i;			/* dummy */
  double angle;			/* absolute angle on drawing surface */
  double o_angle;		/* outside angle of the polygon */
  double i_angle;		/* inside angle of the polygon */
  double mx, my;		/* center point */
  int aromatic = 0;		/* logical: draw aromate? */
  float percent = 0.7;		/* percentage of ring radius/polygon radius */
  int x0, y0, x1, y1;		/* start and end points of the drawn side 
				   of the polygon */

/*  fprintf(stderr, "Begin: %d, %d, %d, %d\n", hp->x, hp->tx, hp->y, hp->ty);*/

  Unmark_all ();

  if (ringtype > 0)
    {
      type = ringtype;
      ringtype = 0;
    }
  else
    {
  switch (ring_angle)
	{
	case 1:		/* Hexagon */
	default:
	  type = 6;
	  break;
	  ;;
	case 2:		/* Pentagon */
	case 3:
	  type = 5;
	  break;
	  ;;
	case 4:		/* Octagon */
	  type = 8;
	  break;
	  ;;
	}
    }
  /* calculate the parameters of the polygon */
  o_angle = 2.0 * M_PI / type;

  i_angle = M_PI - o_angle;

  length =
    sqrt ((double)((hp->tx - hp->x) * (hp->tx - hp->x) +
	  (hp->ty - hp->y) * (hp->ty - hp->y)));

  /* 'radius' is one cathete of a triangle between one half of a side of the
   * polygon and the line between one point and the center */
  radius = 0.5 * length / cos (0.5 * i_angle);
/*  fprintf(stderr, "o_angle: %f, i_angle: %f, length: %f, radius: %f\n", 
 *   	    o_angle, i_angle, length, radius);*/

  /* check the bondstyle, treat rings specially */
  switch (curbond)
    {
    case 1:
      /* use only one type of kekule for uneven pointed rings 
       * otherwise, we'd wind up with one excess double bond */
      if (type % 2)
	{
	  aromatic = 2;
	}
      else
	{
	  aromatic = 1;
	}
      curbond = 0;
      break;
      ;;
    case 2:
      aromatic = 2;
      curbond = 0;
      break;
    case 11:
      /* circle bond */
      aromatic = 3;
      curbond = 0;
      break;
      ;;
    default:
      aromatic = 0;
      break;
      ;;
    }

  if ((hp->x - hp->tx) || (hp->y - hp->ty))
    {
      /* if angle > 180�, subtract from 2pi */
      if (hp->ty > hp->y)
	{
	  /* first we calculate the angle of the given bond
	   * relative to the drawing surface */
	  angle = (2 * M_PI - acos ((hp->tx - hp->x) / length));
	}
      else
	{
	  angle = (acos ((hp->tx - hp->x) / length));
	}
      /* cos() is the angle between two points on the polygon 
       * and the line from one of those points to the center 
       * 'angle' is used to compensate for arbitrary bonds drawn 
       * by the user */
      mx = hp->x + cos (angle - (0.5 * i_angle)) * radius;
      my = hp->y - sin (angle - (0.5 * i_angle)) * radius;
/*	fprintf(stderr, "mx: %f, my: %f, angle: %f\n", mx, my, angle);*/

      /* draw the polygon */
      for (i = 0; i < type; i++)
  {
    x0 =
      (int) rint (mx +
            radius * sin (i * o_angle +
            (3 * M_PI / type - angle)));
    y0 =
      (int) rint (my -
            radius * cos (i * o_angle +
            (3 * M_PI / type - angle)));
    x1 =
      (int) rint (mx +
            radius * sin ((i + 1) * o_angle +
            (3 * M_PI / type - angle)));
    y1 =
      (int) rint (my -
            radius * cos ((i + 1) * o_angle +
            (3 * M_PI / type - angle)));
    /* if conjugated double bonds are selected,
     * draw kekule style rings
     * draw every second bond as double bond */
    if ((aromatic == 1 && (i + 1) % 2) || (aromatic == 2 && i % 2))
      {
        add_struct (x0, y0, x1, y1, 2, 1, 1, 0, curpen);
      }
    else
      {
        add_struct (x0, y0, x1, y1, curbond, 1, 1, 0, curpen);
      }
  }
      /* draw a benzene ring into the polygon if curbond is ring */
      if (aromatic == 3)
	{
	  /* get the middle of the drawn bond */
	  x0 = (hp->tx + hp->x) / 2;
	  y0 = (hp->ty + hp->y) / 2;
	  /* get the point on the line from the center 70% to the
	   * point above */
	  x0 = (int)((x0 + percent * mx) / (1.0 + percent));
	  y0 = (int)((y0 + percent * my) / (1.0 + percent));
	  add_struct ((int)mx, (int)my, x0, y0, 11, 1, 1, 0,curpen);
	}

    }
  mark.flag = 1;
  Display_Mol ();
  modify = True;
}

void
Put_vector (int event_x, int event_y)
/* draws the transient line from the previous to the current cursor position */
{
  int x, y;
  struct xy_co *coord;

  x = round_coord (event_x, size_factor);
  y = round_coord (event_y, size_factor);
  coord = position (x, y, hp->x, hp->y, draw_angle);
  x = coord->x;
  y = coord->y;
  if (x != hp->x || y != hp->y)
    {
      if (hp->x > 0)
	gdk_draw_line (picture, background_gc,
		       hp->x * size_factor, hp->y * size_factor,
		       hp->tx * size_factor, hp->ty * size_factor);
      hp->tx = x;
      hp->ty = y;
      if (hp->x > 0)
	gdk_draw_line (picture, drawing_area->style->black_gc,
		       hp->x * size_factor, hp->y * size_factor,
		       hp->tx * size_factor, hp->ty * size_factor);

      CopyPlane ();
    }
}

void
Del_vector (int event_x, int event_y)
/* deletes the bond vector closest to the current cursor position */
{
  del_struct (select_vector (event_x, event_y, size_factor));
  modify = True;
  Display_Mol ();
}
void
Invert_vector (int event_x, int event_y)
/* exchanges start and endpoint coordinates of a bond (to change direction of
   a wedge or arrow ) */
{
  struct data *hdc;
  int tmp_x, tmp_y;

  hdc = select_vector (event_x, event_y, size_factor);
  if (!hdc)
    return;
  tmp_x = hdc->x;
  tmp_y = hdc->y;
  hdc->x = hdc->tx;
  hdc->y = hdc->ty;
  hdc->tx = tmp_x;
  hdc->ty = tmp_y;
  modify = True;
  Display_Mol ();
}

void
Add_double (int event_x, int event_y)
/* Changes bondtype :
   0 - single
   1 - leftdouble
   2 - rightdouble
   3 - triple
   4 - middouble 
   5 - wedge
   6 - dashed wedge
   7 - wiggly
   8 - single arrow
   9 - half arrow
   10 - wide single
      angle = (2 * M_PI - acos ((x1 - x0) / length));
   12 - dotted line
   13 - crossing single line
   14 - left partial double
      angle = (acos ((x1 - x0) / length));
   16 - dashed bond
   17 - curved half arrow (shortcut)
   18 - uniform triple
   19 - quadruple
   20 - orbital
   21 - filled orbital
 */
{
  struct data *hdc;

  hdc = select_vector (event_x, event_y, size_factor);
  if (hdc)
    {
      if (hdc->bond >= 19)
	hdc->bond = 0;
      else
	++hdc->bond;
      if (hdc->bond==17) hdc->bond++; /* no bond can become a curved arrow */	
      if (hdc->bond>19) hdc->bond = 0; /* nor can it turn into an orbital */
      modify = True;
    }
  Display_Mol ();
}

void
Set_bondtype (int event_x, int event_y, int curbond)
{
  struct data *hdc;

  struct spline *hsp;
  struct xy_co *coord;
  int d;

  if (curbond == 17 || curbond > 19)
    return;			/* cannot change anything to 3-point spline */

  hdc = select_vector (event_x, event_y, size_factor);
  if (hdc)
    {
      hdc->bond = curbond;
      hdc->color = curpen; 
      modify = True;
    }
  else
    {
      if (hp->nsp)
	{
	  event_x = round_coord (event_x, size_factor);
	  event_y = round_coord (event_y, size_factor);
	  coord = position (event_x, event_y, event_x, event_y, No_angle);
	  hsp = sp_root.next;
	  for (d = 0; d < hp->nsp; d++)
	    {
/*
	fprintf(stderr,"event %d:%d, spline0 %d:%d ,spline3 %d:%d\n",
		coord->x,coord->y,hsp->x0, hsp->y0, hsp->x3, hsp->y3);
*/
	      if ((hsp->x0 == coord->x && hsp->y0 == coord->y)
		  || (hsp->x3 == coord->x && hsp->y3 == coord->y))
		{
		  switch (curbond)
		    {
		    case 0:
		      hsp->type = 0;
		      break;
		    case 8:
		      hsp->type = 1;
		      break;
		    case 9:
		      hsp->type = 2;
		      break;
		    case 10:
		      hsp->type = -1;
		      break;
		    case 12:
		      hsp->type = -2;
		      break;
		    default:
		      break;
		    }
		    hsp->color = curpen;
		  modify = True;
		}
	      hsp = hsp->next;
	    }
	}
    }
  Display_Mol ();
}

void
Set_start_rec (int event_x, int event_y)
/* defines first corner of the selection rectangle in mark mode */
{
  hp->x = event_x;
  hp->y = event_y;
  hp->tx = event_x;
  hp->ty = event_y;
  hp->startx = event_x;
  hp->starty= event_y;
}

void
Put_rec (int event_x, int event_y)
/* draws the rubberband rectangle in mark mode */
{
   gdk_draw_rectangle (picture, background_gc,
           FALSE, hp->x, hp->y, hp->tx - hp->x, hp->ty - hp->y);

  if (event_x > hp->x) /* motion right */
    {
      if (event_x >= hp->startx)  /* right of startpoint */
    {
      hp->tx = event_x;
      hp->x = hp->startx;  /* in case of too fast motion */
    }
      else
    {
      hp->x = event_x;  /* left of startpoint */
      hp->tx = hp->startx;
    }
    }
   else  /* motion left */
    {
      if (event_x > hp->startx)    /* right of startpoint */
    {
      hp->tx = event_x;
      hp->x = hp->startx;
    }
      else
    {
      hp->x = event_x;   /* left of startpoint */
      hp->tx = hp->startx;
    }
     }
   if (event_y > hp->y)   /* motion down */
    {
      if (event_y > hp->starty)   /* below startpoint */
    {
      hp->ty = event_y;
      hp->y = hp->starty;
    }
      else                    /* above startpoint */
    {
      hp->y = event_y;
      hp->ty = hp->starty;
    }
    }
   else        /* upward motion  */
    {
      if (event_y > hp->starty)     /* below startpoint */
    {
      hp->ty = event_y;
      hp->y = hp->starty;
    }
      else                    /* above startpoint */
    {
      hp->y = event_y;
      hp->ty = hp->starty;
    }
     }
  gdk_draw_rectangle (picture, drawing_area->style->black_gc,
              FALSE, hp->x, hp->y, hp->tx - hp->x, hp->ty - hp->y);

  CopyPlane ();
}

void
Del_rec ()
/* deletes the marked fragment inside the rectangle */
{
  if (hp->n + hp->nc + hp->nsp == 0)	/* is anything there at all ? */
    return;

  if (!mark.flag)
    return;			/* if nothing marked, nothing to do */

  modify = partial_delete();

/*  
  modify =
    partial_delete (round_coord (hp->x, size_factor),
		    round_coord (hp->y, size_factor), 
		    round_coord (hp->tx, size_factor),
		    round_coord (hp->ty, size_factor));
*/
  if (modify == True)
    mark.flag = False;

}

void
Mark_rec (int control_pressed)
/* mark all bonds and labels inside the rectangular selection area */
{
  struct data *hp_bond;
  struct dc *hp_atom;
  struct spline *hp_spline;
  int hn;
  int d;
  int marks = 0;

  if (hp->tx < hp->x)
    hp->tx = hp->x;
  if (hp->ty < hp->y)
    hp->ty = hp->y;
  gdk_draw_rectangle (picture, drawing_area->style->black_gc,
		      FALSE, hp->x, hp->y, hp->tx - hp->x, hp->ty - hp->y);
#ifdef LIBUNDO
  undo_snapshot ();
#endif
  CopyPlane ();
  if (hp->tx == hp->x && hp->ty == hp->y && !control_pressed)
    {
      mark.flag = False;
      Mark_atom(hp->x,hp->y);
      return;
    }
  else
    {
      mark.x = round_coord (hp->x, size_factor);
      mark.y = round_coord (hp->y, size_factor);
      mark.w = round_coord (hp->tx - hp->x, size_factor);
      mark.h = round_coord (hp->ty - hp->y, size_factor);
      mark.flag = True;

      hp_atom = dac_root.next;
      hn = hp->nc;
      for (d = 0; d < hn; d++)
	{
	  if (check_pos_rec
	      (hp_atom->x, hp_atom->y, mark.x, mark.y, mark.x + mark.w,
	       mark.y + mark.h))
	    {
	      hp_atom->marked = 1;
	      marks++;
	    }
	  else
	    {
	      if (!control_pressed)
		{
		  hp_atom->marked = 0;
		}
	    }
	  hp_atom = hp_atom->next;
	}

      hp_bond = da_root.next;
      hn = hp->n;
      for (d = 0; d < hn; d++)
	{
	  if (check_pos_rec
	      (hp_bond->x, hp_bond->y, mark.x, mark.y, mark.x + mark.w,
	       mark.y + mark.h))
	    {
	      hp_bond->smarked = 1;
	      marks++;
	    }
	  else
	    {
	      if (!control_pressed)
		{
		  hp_bond->smarked = 0;
		}
	    }
	  if (check_pos_rec
	      (hp_bond->tx, hp_bond->ty, mark.x, mark.y, mark.x + mark.w,
	       mark.y + mark.h))
	    {
	      hp_bond->tmarked = 1;
	      marks++;
	    }
	  else
	    {
	      if (!control_pressed)
		{
		  hp_bond->tmarked = 0;
		}
	    }
	  hp_bond = hp_bond->next;
	}

      hp_spline = sp_root.next;
      hn = hp->nsp;
      for (d = 0; d < hn; d++)
	{
	  if (check_pos_rec
	      (hp_spline->x0, hp_spline->y0, mark.x, mark.y, mark.x + mark.w,
	       mark.y + mark.h)
	      || check_pos_rec
	      (hp_spline->x1, hp_spline->y1, mark.x, mark.y, mark.x + mark.w,
	       mark.y + mark.h)
	      || check_pos_rec
	      (hp_spline->x2, hp_spline->y2, mark.x, mark.y, mark.x + mark.w,
	       mark.y + mark.h) || check_pos_rec
	      (hp_spline->x3, hp_spline->y3, mark.x, mark.y, mark.x + mark.w,
	       mark.y + mark.h))
	    {
	      hp_spline->marked = 1;
	      marks++;
	    }
	  else
	    {
	      if (!control_pressed)
		{
		  hp_spline->marked = 0;
		}
	    }
	  hp_spline = hp_spline->next;
	}
    }
	if (addflag ==1) /* check if marker is within region */
	  if (check_pos_rec
	      (refx, refy, mark.x, mark.y, mark.x + mark.w,
	       mark.y + mark.h)) refmark=1;
	       
/*reset mark.flag, if we did not find anything to mark */
/*fprintf(stderr,"marks %d\n",marks);*/
  if (marks == 0)
    mark.flag = False;
/*  if (marks == 1) {
  	Unmark_all();
  	Mark_atom(hp->x,hp->y);
  	}
*/
}

void
Unmark_all ()
/* remove all marks*/
{
  struct data *hp_bond;
  struct dc *hp_atom;
  struct spline *hp_spline;
  int hn;
  int d;

#ifdef LIBUNDO
  undo_snapshot ();
#endif
  mark.flag = False;

  hp_atom = dac_root.next;
  hn = hp->nc;
  for (d = 0; d < hn; d++)
    {
      hp_atom->marked = 0;
      hp_atom = hp_atom->next;
    }

  hp_bond = da_root.next;
  hn = hp->n;
  for (d = 0; d < hn; d++)
    {
      hp_bond->smarked = 0;
      hp_bond->tmarked = 0;
      hp_bond = hp_bond->next;
    }
      hp_spline = sp_root.next;
      hn = hp->nsp;
      for (d = 0; d < hn; d++)
	{
	hp_spline->marked = 0;
	hp_spline = hp_spline->next;
	}
    if (addflag==1) refmark=0;
}


void
Mark_atom (int t_x, int t_y)
/* marks a single position (for addition of fragments) */
{
  struct data *hdc;
  int d, dt,hn,n;

  refx=-1;
  refy=-1;
	t_x=round_coord(t_x,size_factor);
	t_y=round_coord(t_y,size_factor);
  hdc = da_root.next;
  hn = hp->n;
  for (n = 0; n < hn; n++)
    {
  d = calc_vector(abs(hdc->x-t_x),abs(hdc->y-t_y));
  dt = calc_vector(abs(hdc->tx-t_x),abs(hdc->ty-t_y));
	if (d <10*size_factor) {
	  refx = hdc->x;
  	  refy = hdc->y;
	  }else if (dt <10*size_factor) {
	  refx = hdc->tx;
  	  refy = hdc->ty;
	}
    	hdc=hdc->next;
    }
  if (refx<0 || refy<0)  {
/*  fprintf(stderr,"no match found for %d %d\n",t_x,t_y);*/
  addflag=0;
  return;
  }
  addflag = 1;
  Display_Mol ();
}

void
Center ()
/* callback function to center the molecule in the drawing area */
{
  FreePix ();
  CreatePix ();
  center_mol ();
  Display_Mol ();
  modify = True;
}

void
Set_pmove (int event_x, int event_y)
/*  sets the reference position for moving/rotating the selected fragment */
{
  struct xy_co *coord;
  struct data *hp_bond;
  struct dc *hp_atom;
  struct spline *hp_spline;

  int x, y;
  int d, dtmp;
  draw_ok=1;
  x = round_coord (event_x, size_factor);
  y = round_coord (event_y, size_factor);
  coord = position (x, y, x, y, No_angle);
  hp->x = coord->x;
  hp->y = coord->y;
/*  xcent = hp->x;
  ycent = hp->y;*/
  get_center_marked (&xcent, &ycent);

  dtmp = 0;
/*  if (tmpx)
    {
      free (tmpx);
      tmpx = NULL;
    }
  if (tmpy)
    {
      free (tmpy);
      tmpy = NULL;
    }*/
  tmpx = realloc (tmpx,(2 * hp->n + hp->nc + 4 * hp->nsp +1) * sizeof (int));
  tmpy = realloc (tmpy,(2 * hp->n + hp->nc + 4 * hp->nsp +1) * sizeof (int));
  hp_bond = da_root.next;
  for (d = 0; d < hp->n; d++)
    {

      if (hp_bond->smarked == 1)
	{
	  tmpx[dtmp] = hp_bond->x - xcent;
	  tmpy[dtmp++] = hp_bond->y - ycent;
	}
      if (hp_bond->tmarked == 1)
	{
	  tmpx[dtmp] = hp_bond->tx - xcent;
	  tmpy[dtmp++] = hp_bond->ty - ycent;
	}
      hp_bond = hp_bond->next;
    }

  hp_atom = dac_root.next;
  for (d = 0; d < hp->nc; d++)
    {
      if (hp_atom->marked == 1)
	{
	  tmpx[dtmp] = hp_atom->x - xcent;
	  tmpy[dtmp++] = hp_atom->y - ycent;
	}
      hp_atom = hp_atom->next;
    }

  hp_spline = sp_root.next;
  for (d = 0; d < hp->nsp; d++)
    {
      if (hp_spline->marked == 1)
	{
	  tmpx[dtmp] = hp_spline->x0 - xcent;
	  tmpy[dtmp++] = hp_spline->y0 - ycent;
	  tmpx[dtmp] = hp_spline->x1 - xcent;
	  tmpy[dtmp++] = hp_spline->y1 - ycent;
	  tmpx[dtmp] = hp_spline->x2 - xcent;
	  tmpy[dtmp++] = hp_spline->y2 - ycent;
	  tmpx[dtmp] = hp_spline->x3 - xcent;
	  tmpy[dtmp++] = hp_spline->y3 - ycent;
	}
      hp_spline = hp_spline->next;
    }
    	if (addflag==1 && refmark==1){
	  tmpx[dtmp] = refx - xcent;
	  tmpy[dtmp++] = refy - ycent;
    	}	
}

void
Put_pmove (int event_x, int event_y, int axis)
/* updates the position of the marked fragment as the mouse is dragged */
{
  int x, y;
  int dx, dy;
  x = round_coord (event_x, size_factor);
  y = round_coord (event_y, size_factor);
  if (importflag != 0)
    return;
  if (!draw_ok) return;  
  if (mark.flag)
    {
      dx = x - hp->x;
      dy = y - hp->y;
	if (axis == 1) dy = 0;
	if (axis == 2) dx = 0;
      if (partial_move (dx, dy))
	modify = True;
      mark.x = mark.x + dx;
      mark.y = mark.y + dy;
      hp->x = x;
      hp->y = y;
      Display_Mol ();
    }
  else
    {
      if (move_pos (hp->x, hp->y, x, y))
	{
	  hp->x = x;
	  hp->y = y;
	  Display_Mol ();
	  modify = True;
	} 
#if 0
	/* automatically switch to rubberband rectangle if nothing selected */
	else {
	Set_start_rec(event_x,event_y);
    	drawmode = 4;
    	}
#endif
    }
}

void
Put_protate (int event_x, int event_y, int control_pressed)
/* updates the position of the marked fragment as it pivots around the
   reference position proportional to the x movement of the mouse */
{
  int x, y;
  int dx;
  static float angle;
  int i;
  static float ang = 0.;
  double rotsin = 0.;
  double rotcos = 0.;
  if (!draw_ok) return;
  x = round_coord (event_x, size_factor);
  y = round_coord (event_y, size_factor);
  if (importflag != 0)
    {
      pdbrotate (x, y, control_pressed);
      return;
    }
  if (!mark.flag)
    return;			/* rotation only makes sense for blocks */
  mark.x = 0;
  mark.y = 0;
  mark.w = 800;
  mark.h = 800;
  dx = x - hp->x;

  ang += (float) dx / 2.;
  if (ang < -360.)
    ang = ang + 360.;
  if (ang > 360.)
    ang = ang - 360.;
  switch (draw_angle)
    {
    case 1:
      angle = 30.0;
      break;
    case 2:
    case 3:
      angle = 18.0;
      break;
    case 4:
      angle = 45.0;
      break;
    }
  for (i = 0; i < 30; i++)
    {
      if (fabs ((angle + (angle * (float) i)) - fabs(ang)) < 4.0 && control_pressed)
	{
	  rotsin = sin ((angle + (angle * (float) i)) * M_PI / 180.);
	  rotcos = cos ((angle + (angle * (float) i)) * M_PI / 180.);
	  i = 30;		/* break loop */
	}
      else
	{
	  rotsin = sin (ang * M_PI / 180.);
	  rotcos = cos (ang * M_PI / 180.);
	}
    }
  if (partial_rotate (/*xcent, ycent,*/ rotsin, rotcos))
    modify = True;
  hp->x = x;
  hp->y = y;
  Display_Mol ();


}

void
Set_pscale (int event_x, int event_y)
/*  sets the reference position for moving/rotating the selected fragment */
{
  struct xy_co *coord;
  struct data *hp_bond;
  struct dc *hp_atom;
  struct spline *hp_spline;

  int x, y;
  int d, dtmp;

  draw_ok=1;
  x = round_coord (event_x, size_factor);
  y = round_coord (event_y, size_factor);
  coord = position (x, y, x, y, No_angle);
  hp->x = coord->x;
  hp->y = coord->y;
  if (tmpx)
    {
      free (tmpx);
      tmpx = NULL;
    }
  if (tmpy)
    {
      free (tmpy);
      tmpy = NULL;
    }
  tmpx = malloc ((2 * hp->n + hp->nc + 4 * hp->nsp) * sizeof (int));
  tmpy = malloc ((2 * hp->n + hp->nc + 4 * hp->nsp) * sizeof (int));

  get_center_marked (&xcent, &ycent);

  dtmp = 0;
  hp_bond = da_root.next;
  for (d = 0; d < hp->n; d++)
    {

      if (hp_bond->smarked == 1)
	{
	  tmpx[dtmp] = hp_bond->x - xcent;
	  tmpy[dtmp++] = hp_bond->y - ycent;
	}
      if (hp_bond->tmarked == 1)
	{
	  tmpx[dtmp] = hp_bond->tx - xcent;
	  tmpy[dtmp++] = hp_bond->ty - ycent;
	}
      hp_bond = hp_bond->next;
    }

  hp_atom = dac_root.next;
  for (d = 0; d < hp->nc; d++)
    {
      if (hp_atom->marked == 1)
	{
	  tmpx[dtmp] = hp_atom->x - xcent;
	  tmpy[dtmp++] = hp_atom->y - ycent;
	}
      hp_atom = hp_atom->next;
    }

  hp_spline = sp_root.next;
  for (d = 0; d < hp->nsp; d++)
    {
      if (hp_spline->marked == 1)
	{
	  tmpx[dtmp] = hp_spline->x0 - xcent;
	  tmpy[dtmp++] = hp_spline->y0 - ycent;
	  tmpx[dtmp++] = hp_spline->x1 - xcent;
	  tmpy[dtmp++] = hp_spline->y1 - ycent;
	  tmpx[dtmp++] = hp_spline->x2 - xcent;
	  tmpy[dtmp++] = hp_spline->y2 - ycent;
	  tmpx[dtmp++] = hp_spline->x3 - xcent;
	  tmpy[dtmp++] = hp_spline->y3 - ycent;
	}
      hp_spline = hp_spline->next;
    }
}

void
Put_pscale (int event_x, int event_y, int aniso)
/* updates the position of the marked fragment as the mouse is dragged */
{
  int x;
  int dx;
  (void) event_y;
  if (!draw_ok) return;
  x = round_coord (event_x, size_factor);

  if (mark.flag)
    {
      dx = x - hp->x;
      if (partial_rescale (dx, aniso))
	modify = True;
      Display_Mol ();
    }
}

void
Add_number (int event_x, int event_y)
/* insert an automatically incremented atom number at the current
   cursor position*/
{
  gchar name[16];
  int x, y;

  snprintf (name, sizeof name, "%d", ++atnum);
  if (strlen (name))
    {
      x = round_coord (event_x, size_factor);
      y = round_coord (event_y, size_factor);
      del_char (select_char (event_x, event_y, size_factor));
      add_char (x, y, name, Middle_Text, 0, curpen, serif_flag, 0);
      modify = True;
      Display_Mol ();
    }
}
