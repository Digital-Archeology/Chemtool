/*
 *    functions for the graphic output of the pixmap
 */

#include <string.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include "ct1.h"
#pragma GCC diagnostic pop
#ifdef GTK2
PangoFontDescription *font[7], *smallfont[7], *symbfont[7], 
                     *ssymbfont[7], *slfont[7], 
                     *boldfont[7], *textfont[7];
#else
GdkFont *font[7], *smallfont[7], *symbfont[7], 
                  *ssymbfont[7], *slfont[7], 
                  *boldfont[7], *textfont[7];
#endif

#ifndef MIN
#define MIN(a, b)        ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b)        ((a) > (b) ? (a) : (b))
#endif

void
CreatePix ()
/* initializes fonts, linestyles and factors */
{
  float ztab[] = { 0.4, 0.6, 0.8, 1, 1.2 };
  int ltab[] = { 0, 0, 2, 2, 2 };
/*  int ftab[] = { 2, 3, 4, 5, 6 };*/

  size_factor = ztab[zoom_factor];
  if (bondlen_mm != 0 && bondlen_mm != 10.668) size_factor *= bondlen_mm/10.668;
  head.pix_width = 1600;
  head.pix_height = 1600;
  Set_Line (ltab[zoom_factor]);


}

void
FreePix ()
/* erases the drawing area */
{
  GdkRectangle update_rect;
  int i;

  gdk_draw_rectangle (picture,
		      background_gc,
		      TRUE, 0, 0,
		      (gint)drawing_area->allocation.width,
		      (gint)drawing_area->allocation.height);
  if (gridtype==1) {
  	for (i=0+gridx;i<=(int)drawing_area->allocation.width;i=i+50)
  	    gdk_draw_line (picture, drawing_area->style->bg_gc[GTK_STATE_INSENSITIVE],
		   (gint) (i * size_factor), (gint) (0),
		   (gint) (i * size_factor), (gint) (drawing_area->allocation.height));
	for (i=0+gridy;i<=(int)drawing_area->allocation.height;i=i+50)
	    gdk_draw_line (picture, drawing_area->style->bg_gc[GTK_STATE_INSENSITIVE],
		   (gint) (0), (gint) (i*size_factor),
		   (gint) (drawing_area->allocation.width), (gint) (i * size_factor));
		}
   if (gridtype==2){

	for (i=-2560+gridx;i<=5280;i=i+64){
	    gdk_draw_line (picture, drawing_area->style->bg_gc[GTK_STATE_INSENSITIVE],
		   (gint) (0+gridx), (gint) ((gridy+i)*size_factor),
		   (gint) (5500+gridx)*size_factor, 
		   (gint) (3200+gridy+i) * size_factor);
	    gdk_draw_line (picture, drawing_area->style->bg_gc[GTK_STATE_INSENSITIVE],
		   (gint) (0+gridx), (gint) ((i+gridy)*size_factor),
		   (gint) (5500+gridx)*size_factor, 
		   (gint) (-3200+gridy+i) * size_factor);
			}
		}

  update_rect.x = 0;
  update_rect.y = 0;
  update_rect.width = drawing_area->allocation.width;
  update_rect.height = drawing_area->allocation.height;
  gtk_widget_draw ((GtkWidget *) drawing_area, &update_rect);

}



void
CopyPlane ()
/* updates the display with the contents of the background pixmap */
{
  gdk_draw_pixmap (drawing_area->window,
		   drawing_area->style->
		   fg_gc[GTK_WIDGET_STATE (drawing_area)], picture, 0, 0, 0,
		   0, head.pix_width, head.pix_height);
}

void
Display_Mol ()
/* calls the individual bond and text drawing functions to display the
   picture */
{
  struct data *hpc;
  struct dc *hpc_c;
  struct spline *hpc_sp;
  int d,i;
  char *c;
  struct xy_co *coord;
  int tmpx1,tmpx2,tmpy1,tmpy2,tmptx1,tmptx2,tmpty1,tmpty2;
  GdkRectangle update_rect;

  if (head.pix_width == 0 || head.pix_height == 0)
    return;
  gdk_draw_rectangle (picture,
		      background_gc,
		      TRUE, 0, 0, head.pix_width, head.pix_height);

  if (gridtype==1) {
  	for (i=0+gridx;i<=head.pix_width;i=i+64)
  	    gdk_draw_line (picture,drawing_area->style->bg_gc[GTK_STATE_INSENSITIVE],
	   (gint) (i * size_factor), (gint) (0),
		   (gint) (i * size_factor), (gint) (head.pix_height));
	for (i=0+gridy;i<=head.pix_height;i=i+64)
	    gdk_draw_line (picture, drawing_area->style->bg_gc[GTK_STATE_INSENSITIVE],
		   (gint) (0), (gint) (i*size_factor),
		   (gint) (head.pix_width), (gint) (i * size_factor));
   }

   if (gridtype==2){

	for (i=-2560+gridx;i<=6400;i=i+64){
	    gdk_draw_line (picture, drawing_area->style->bg_gc[GTK_STATE_INSENSITIVE],
		   (gint) (0+gridx), (gint) ((gridy+i)*size_factor),
		   (gint) (5500+gridx)*size_factor, 
		   (gint) (3200+gridy+i) * size_factor);
	    gdk_draw_line (picture, drawing_area->style->bg_gc[GTK_STATE_INSENSITIVE],
		   (gint) (0+gridx), (gint) ((i+gridy)*size_factor),
		   (gint) (5500+gridx)*size_factor, 
		   (gint) (-3200+gridy+i) * size_factor);
			}
		}
		
  hpc = da_root.next;
  if (!hpc)
    fprintf (stderr,
	     "Help - somebody ate my atoms (hp->n=%d, da_root.next NULL)!!!\n",
	     hp->n);
  for (d = 0; d < hp->n; d++)
    {
    int x=hpc->x;
    int y=hpc->y;
    int tx=hpc->tx;
    int ty=hpc->ty;
#if 1
//constant clipping factor works better than original 4*size_factor
	if ( !use_whiteout) {
	        int siz,strl;
	        int j;
		i=has_label(hpc->x,hpc->y);
		if (i>=0) {
                 hpc_c = dac_root.next;
                 for (j = 0; j < i; j++)hpc_c=hpc_c->next;
		 siz=hpc_c->size;  // font size
                 if (y-ty==0&& hpc_c->direct ==0) { // horizontal bond, left justified label
                      strl=0;
                      for (i=0;i<(int)strlen(hpc_c->c);i++) { // count regular characters in label
                        if(hpc_c->c[i] != '_' && hpc_c->c[i] != '^' 
                        && hpc_c->c[i] != '{' && hpc_c->c[i] != '{')
                        strl++;
                      }
                   if (tx>x) {
                      x+=(strl-1)*3.2*(siz+1); // move bond endpoint off label
                      if (hpc_c->c[strlen(hpc_c->c)-1]=='H') x+=5;
                      if (hpc_c->c[strlen(hpc_c->c)-2]=='_')
                        x-=1.6*(siz+1);        // adjust for trailing subscripts
                      if (hpc_c->c[strlen(hpc_c->c)-1]=='}')
                        x-=4.8*(siz+1);
                   }     
                   else {
                      }
		 }
		int ox=x;
				x += (3.2*(siz+1)/calc_vector(abs(tx-x),abs(ty-y))) *(tx-x);
				y += (3.2*(siz+1)/calc_vector(abs(tx-ox),abs(ty-y))) *(ty-y);
				}
		i=has_label(hpc->tx,hpc->ty);
		if (i>=0) {
                 hpc_c = dac_root.next;
                 for (j = 0; j < i; j++)hpc_c=hpc_c->next;
		 siz=hpc_c->size;
                 if (y-ty==0 ){
                      strl=0;
                      for (i=0;i<(int)strlen(hpc_c->c);i++) {
                        if(hpc_c->c[i] != '_' && hpc_c->c[i] != '^' 
                        && hpc_c->c[i] != '{' && hpc_c->c[i] != '{')
                        strl++;
                      }
                   if ( hpc_c->direct<-1) {
                      if (tx>x)
                        tx-=(strlen(hpc_c->c)-1)*3.2*(siz+1);
//QUAK                      else
//QUAK                       tx+=(strlen(hpc_c->c)-1)*3.2*(siz+1);
                   }
                   else if ( hpc_c->direct==0 && tx<x) {
                        tx+=(strl-1)*3.2*(siz+1);
                      if (hpc_c->c[strlen(hpc_c->c)-2]=='_')
                        tx-=1.6*(siz+1);
                      if (hpc_c->c[strlen(hpc_c->c)-1]=='}')
                        tx-=4.8*(siz+1);
                    }    
                }  
		int otx=tx;
				tx -= (3.2*(siz+1)/calc_vector(abs(tx-x),abs(ty-y))) *(tx-x);
				ty -= (3.2*(siz+1)/calc_vector(abs(otx-x),abs(ty-y))) *(ty-y);
				}
		}		
#endif
      switch (hpc->bond)
	{
	case 0:
	  Drawline (x, y, tx, ty, hpc->smarked + hpc->tmarked,hpc->color);	/*single */
	  break;
	case 1:
	  {			/*leftdouble */
	    coord = multi_bonds (x, y, tx, ty, mb_dist);
	    Drawline (coord->x, coord->y, coord->tx, coord->ty,
		      hpc->smarked + hpc->tmarked,hpc->color);
	    Drawline (x, y, tx, ty,
		      hpc->smarked + hpc->tmarked,hpc->color);
	  }
	  break;
	case 2:
	  {			/*rightdouble */
	    coord = multi_bonds (tx, ty, x, y, mb_dist);
	    Drawline (coord->x, coord->y, coord->tx, coord->ty,
		      hpc->smarked + hpc->tmarked,hpc->color);
	    Drawline (x, y, tx, ty,
		      hpc->smarked + hpc->tmarked,hpc->color);
	  }
	  break;
	case 5:
	  DrawWedge (x, y, tx, ty,
		     hpc->smarked + hpc->tmarked, hpc->color);
	  break;
	case 6:
	  DrawDashedWedge (x, y, tx, ty,
			   hpc->smarked + hpc->tmarked,hpc->color);
	  break;
	case 7:
	  DrawWiggly (x, y, tx, ty,
		      hpc->smarked + hpc->tmarked,hpc->color);
	  break;
	case 8:
	  DrawArrow (x, y, tx, ty, 1,
		     hpc->smarked + hpc->tmarked,hpc->color);
	  break;
	case 9:
	  DrawArrow (x, y, tx, ty, 2,
		     hpc->smarked + hpc->tmarked,hpc->color);
	  break;
	case 10:
	  DrawWide (x, y, tx, ty,
		    hpc->smarked + hpc->tmarked,hpc->color);
	  break;
	case 11:
	  DrawCircle (x, y, tx, ty,
		      hpc->smarked + hpc->tmarked,hpc->color);
	  break;
	case 12:
	  DrawDotted (x, y, tx, ty,
		      hpc->smarked + hpc->tmarked,hpc->color);
	  break;
	case 13:
/* "crossing" bonds are drawn in their own loop later, to make sure they end up on top */
	  break;
	case 4:
	  {			/*middouble */
	    coord = center_double_bond (x, y, tx, ty, db_dist);
	    Drawline (coord->x, coord->y, coord->tx, coord->ty,
		      hpc->smarked + hpc->tmarked,hpc->color);
	    coord++;
	    Drawline (coord->x, coord->y, coord->tx, coord->ty,
		      hpc->smarked + hpc->tmarked,hpc->color);
	  }
	  break;
	case 3:
	  {			/*triple */
	    coord = multi_bonds (x, y, tx, ty, mb_dist);
	    Drawline (coord->x, coord->y, coord->tx, coord->ty,
		      hpc->smarked + hpc->tmarked,hpc->color);
	    Drawline (x, y, tx, ty,
		      hpc->smarked + hpc->tmarked,hpc->color);
	    coord = multi_bonds (tx, ty, x, y, mb_dist);
	    Drawline (coord->x, coord->y, coord->tx, coord->ty,
		      hpc->smarked + hpc->tmarked,hpc->color);
	    Drawline (x, y, tx, ty,
	      hpc->smarked + hpc->tmarked,hpc->color);
	  }
	  break;
	case 14:
	  {			/*left partial double */
	    coord = multi_bonds (x, y, tx, ty, mb_dist);
	    DrawDashed (coord->x, coord->y, coord->tx, coord->ty,
			hpc->smarked + hpc->tmarked,hpc->color);
	    Drawline (x, y, tx, ty,
		      hpc->smarked + hpc->tmarked,hpc->color);
	  }
	  break;
	case 15:
	  {			/*right partial double */
	    coord = multi_bonds (tx, ty, x, y, mb_dist);
	    DrawDashed (coord->x, coord->y, coord->tx, coord->ty,
			hpc->smarked + hpc->tmarked,hpc->color);
	    Drawline (x, y, tx, ty,
		      hpc->smarked + hpc->tmarked,hpc->color);
	  }
	  break;
	case 16:		/*zebra dashed single bond */
	  DrawStripe (x, y, tx, ty,
		      hpc->smarked + hpc->tmarked,hpc->color);
	  break;
	case 18:
	  {			/*triple, equal lengths */
	    coord = center_double_bond (x, y, tx, ty, 2*db_dist);
	    Drawline (coord->x, coord->y, coord->tx, coord->ty,
		      hpc->smarked + hpc->tmarked,hpc->color);
	    coord++;
	    Drawline (coord->x, coord->y, coord->tx, coord->ty,
		      hpc->smarked + hpc->tmarked,hpc->color);
	  Drawline (x, y, tx, ty, hpc->smarked + hpc->tmarked,hpc->color);	/*single */
	  }
	  break;
	case 19:
	  {			/*quadruple */
	    coord = center_double_bond (x, y, tx, ty, db_dist+3);
	    tmpx1=coord->x;
	    tmpy1=coord->y;
            tmptx1=coord->tx;
            tmpty1=coord->ty;
            coord++;	    
	    tmpx2=coord->x;
	    tmpy2=coord->y;
            tmptx2=coord->tx;
            tmpty2=coord->ty;
	    coord = center_double_bond (tmpx1, tmpy1, tmptx1, tmpty1, db_dist);
	    Drawline (coord->x, coord->y, coord->tx, coord->ty,
		      hpc->smarked + hpc->tmarked,hpc->color);
	    coord++;
	    Drawline (coord->x, coord->y, coord->tx, coord->ty,
		      hpc->smarked + hpc->tmarked,hpc->color);
	    coord = center_double_bond (tmpx2, tmpy2,tmptx2, tmpty2, db_dist);
	    Drawline (coord->x, coord->y, coord->tx, coord->ty,
		      hpc->smarked + hpc->tmarked,hpc->color);
	    coord++;
	    Drawline (coord->x, coord->y, coord->tx, coord->ty,
		      hpc->smarked + hpc->tmarked,hpc->color);
	  }
	  break;
	}
      hpc = hpc->next;
      if (!hpc)
	{
	  fprintf (stderr,
		   "Help - somebody ate my atoms (d=%d,hp->n=%d)!!!\n", d,
		   hp->n);
	  hp->n = d;
	  continue;
	}
    }

/* second loop - only for drawing "crossing" bonds (type 13) */
  hpc = da_root.next;
  for (d = 0; d < hp->n; d++)
    {
    int x=hpc->x;
    int y=hpc->y;
    int tx=hpc->tx;
    int ty=hpc->ty;
#if 1
//constant clipping factor works better than original 4*size_factor
	if ( !use_whiteout) {
	        int siz,strl;
	        int j;
		i=has_label(hpc->x,hpc->y);
		if (i>=0) {
                 hpc_c = dac_root.next;
                 for (j = 0; j < i; j++)hpc_c=hpc_c->next;
		 siz=hpc_c->size;  // font size
                 if (y-ty==0&& hpc_c->direct ==0) { // horizontal bond, left justified label
                      strl=0;
                      for (i=0;i<(int)strlen(hpc_c->c);i++) { // count regular characters in label
                        if(hpc_c->c[i] != '_' && hpc_c->c[i] != '^' 
                        && hpc_c->c[i] != '{' && hpc_c->c[i] != '{')
                        strl++;
                      }
                   if (tx>x) {
                      x+=(strl-1)*3.2*(siz+1); // move bond endpoint off label
                      if (hpc_c->c[strlen(hpc_c->c)-1]=='H') x+=5;
                      if (hpc_c->c[strlen(hpc_c->c)-2]=='_')
                        x-=1.6*(siz+1);        // adjust for trailing subscripts
                      if (hpc_c->c[strlen(hpc_c->c)-1]=='}')
                        x-=4.8*(siz+1);
                   }     
                   else {
                      }
		 }
		int ox=x;
				x += (3.2*(siz+1)/calc_vector(abs(tx-x),abs(ty-y))) *(tx-x);
				y += (3.2*(siz+1)/calc_vector(abs(tx-ox),abs(ty-y))) *(ty-y);
				}
		i=has_label(hpc->tx,hpc->ty);
		if (i>=0) {
                 hpc_c = dac_root.next;
                 for (j = 0; j < i; j++)hpc_c=hpc_c->next;
		 siz=hpc_c->size;
                 if (y-ty==0 ){
                      strl=0;
                      for (i=0;i<(int)strlen(hpc_c->c);i++) {
                        if(hpc_c->c[i] != '_' && hpc_c->c[i] != '^' 
                        && hpc_c->c[i] != '{' && hpc_c->c[i] != '{')
                        strl++;
                      }
                   if ( hpc_c->direct<-1) {
                      if (tx>x)
                        tx-=(strlen(hpc_c->c)-1)*3.2*(siz+1);
//QUAK                      else
//QUAK                       tx+=(strlen(hpc_c->c)-1)*3.2*(siz+1);
                   }
                   else if ( hpc_c->direct==0 && tx<x) {
                        tx+=(strl-1)*3.2*(siz+1);
                      if (hpc_c->c[strlen(hpc_c->c)-2]=='_')
                        tx-=1.6*(siz+1);
                      if (hpc_c->c[strlen(hpc_c->c)-1]=='}')
                        tx-=4.8*(siz+1);
                    }    
                }  
		int otx=tx;
				tx -= (3.2*(siz+1)/calc_vector(abs(tx-x),abs(ty-y))) *(tx-x);
				ty -= (3.2*(siz+1)/calc_vector(abs(otx-x),abs(ty-y))) *(ty-y);
				}
		}		
#endif
	if (hpc->bond==13)
	  DrawAcross (x, y, tx, ty,
		      hpc->smarked + hpc->tmarked,hpc->color);
      hpc = hpc->next;
    }

/* */
  hpc_c = dac_root.next;
  for (d = 0; d < hp->nc; d++)
    {
      c = hpc_c->c;
      Drawstring (hpc_c->x, hpc_c->y, c, hpc_c->direct, hpc_c->marked, hpc_c->color,hpc_c->font,hpc_c->size,0);
      hpc_c = hpc_c->next;
    }

  hpc_sp = sp_root.next;
  for (d = 0; d < hp->nsp; d++)
    {
      Drawspline (hpc_sp->x0, hpc_sp->y0, hpc_sp->x1, hpc_sp->y1,
		  hpc_sp->x2, hpc_sp->y2, hpc_sp->x3, hpc_sp->y3,
		  hpc_sp->type, hpc_sp->marked,hpc_sp->color);
      hpc_sp = hpc_sp->next;
    }

  if (drawmode == 4 && addflag==1 && !xbmflag)
    {
      gdk_draw_rectangle (picture, mygc[2], TRUE, refx * size_factor - 1,
			  refy * size_factor - 1, 3, 3);
    }
  gdk_draw_pixmap (drawing_area->window,
		   drawing_area->style->
		   fg_gc[GTK_WIDGET_STATE (drawing_area)], picture, 0, 0, 0,
		   0, head.pix_width, head.pix_height);
  update_rect.x = 0;
  update_rect.y = 0;
  update_rect.width = (guint16)head.pix_width;
  update_rect.height = (guint16)head.pix_height;
  gtk_widget_draw ((GtkWidget *) drawing_area, &update_rect);
}

void
draw_preview_bonds (int x, int y, int tx, int ty, int b)
/* calls the individual bond functions to create the preview image */
{
  struct xy_co *coord;

  switch (b)
    {
    case 0:
      Drawline (x, y, tx, ty, 0, 0);	/*single */
      break;
    case 1:
      {				/*leftdouble */
	coord = multi_bonds (x, y, tx, ty, mb_dist);
	Drawline (coord->x, coord->y, coord->tx, coord->ty, 0,0);
	Drawline (x, y, tx, ty, 0, 0);
      }
      break;
    case 2:
      {				/*rightdouble */
	coord = multi_bonds (tx, ty, x, y, mb_dist);
	Drawline (coord->x, coord->y, coord->tx, coord->ty, 0,0);
	Drawline (x, y, tx, ty, 0,0);
      }
      break;
    case 5:
      DrawWedge (x, y, tx, ty, 0, 0);
      break;
    case 6:
      DrawDashedWedge (x, y, tx, ty, 0, 0);
      break;
    case 7:
      DrawWiggly (x, y, tx, ty, 0, 0);
      break;
    case 8:
      DrawArrow (x, y, tx, ty, 1, 0, 0);
      break;
    case 9:
      DrawArrow (x, y, tx, ty, 2, 0, 0);
      break;
    case 10:
      DrawWide (x, y, tx, ty, 0, 0);
      break;
    case 11:
      DrawCircle (x, y, tx, ty, 0, 0);
      break;
    case 12:
      DrawDotted (x, y, tx, ty, 0, 0);
      break;
    case 13:
      DrawAcross (x, y, tx, ty, 0, 0);
      break;
    case 4:
      {				/*middouble */
	coord = center_double_bond (x, y, tx, ty, db_dist);
	Drawline (coord->x, coord->y, coord->tx, coord->ty, 0, 0);
	coord++;
	Drawline (coord->x, coord->y, coord->tx, coord->ty, 0, 0);
      }
      break;
    case 3:
      {				/*triple */
	coord = multi_bonds (x, y, tx, ty, mb_dist);
	Drawline (coord->x, coord->y, coord->tx, coord->ty, 0, 0);
	Drawline (x, y, tx, ty, 0, 0);
	coord = multi_bonds (tx, ty, x, y, mb_dist);
	Drawline (coord->x, coord->y, coord->tx, coord->ty, 0, 0);
	Drawline (x, y, tx, ty, 0, 0);
      }
      break;
    case 14:
      {				/*left partial double */
	coord = multi_bonds (x, y, tx, ty, mb_dist);
	DrawDashed (coord->x, coord->y, coord->tx, coord->ty, 0, 0);
	Drawline (x, y, tx, ty, 0, 0);
      }
      break;
    case 15:
      {				/*right partial double */
	coord = multi_bonds (tx, ty, x, y, mb_dist);
	DrawDashed (coord->x, coord->y, coord->tx, coord->ty, 0, 0);
	Drawline (x, y, tx, ty, 0, 0);
      }
      break;
    case 16:			/*zebra dashed single bond */
      DrawStripe (x, y, tx, ty, 0, 0);
      break;
    }
}

void
Drawline (int x, int y, int tx, int ty, int active, int color)
/* draws a single bond */
{
  if ( xbmflag)
    gdk_draw_line (picture, drawing_area->style->black_gc,
		   (gint) (x * size_factor), (gint) (y * size_factor),
		   (gint) (tx * size_factor), (gint) (ty * size_factor));
  else if (active)
    gdk_draw_line (picture, hlgc,
		   (gint) (x * size_factor), (gint) (y * size_factor),
		   (gint) (tx * size_factor), (gint) (ty * size_factor));
  else
    gdk_draw_line (picture, mygc[color],
		   (gint) (x * size_factor), (gint) (y * size_factor),
		   (gint) (tx * size_factor), (gint) (ty * size_factor));
}

void
DrawAcross (int x, int y, int tx, int ty, int active,int color)
/* draws a single bond on a white background */
{
  gint npoints = 4;
  GdkPoint mypoints[4];
  struct xy_co *coord;
  GdkGC *thegc;


  thegc = background_gc;

  coord = center_double_bond (x, y, tx, ty, 6);
  mypoints[0].x = (coord->x + (coord->tx - coord->x) / 8) * size_factor;
  mypoints[0].y = (coord->y + (coord->ty - coord->y) / 8) * size_factor;
  mypoints[1].x = (coord->tx - (coord->tx - coord->x) / 8) * size_factor;
  mypoints[1].y = (coord->ty - (coord->ty - coord->y) / 8) * size_factor;
  coord++;
  mypoints[2].x = (coord->tx - (coord->tx - coord->x) / 8) * size_factor;
  mypoints[2].y = (coord->ty - (coord->ty - coord->y) / 8) * size_factor;
  mypoints[3].x = (coord->x + (coord->tx - coord->x) / 8) * size_factor;
  mypoints[3].y = (coord->y + (coord->ty - coord->y) / 8) * size_factor;
  gdk_draw_polygon (picture, thegc, TRUE, (GdkPoint *) mypoints,
		    (gint) npoints);

  if (xbmflag)
  thegc=drawing_area->style->black_gc;
  else if (active)
  thegc=hlgc;
  else 
  thegc=mygc[color];
    gdk_draw_line (picture, thegc,
		   (gint) (x * size_factor), (gint) (y * size_factor),
		   (gint) (tx * size_factor), (gint) (ty * size_factor));
}

void
DrawDashed (int x, int y, int tx, int ty, int active,int color)
/* draws a dashed bond */
{
  GdkGC *thegc;
  GdkSegment mypoints[6];
  int xlen, ylen, i;
  gint nsegments = 6;

  xlen = tx - x;
  ylen = ty - y;

  for (i = 0; i < nsegments; i++)
    {
      mypoints[i].x1 = (gint16) ((x + 2 * i * 0.09 * xlen) * size_factor);
      mypoints[i].y1 = (gint16) ((y + 2 * i * 0.09 * ylen) * size_factor);
      mypoints[i].x2 = (gint16) ((x + (2 * i + 1) * 0.09 * xlen) * size_factor);
      mypoints[i].y2 = (gint16) ((y + (2 * i + 1) * 0.09 * ylen) * size_factor);
    }
  if (xbmflag)
  thegc= drawing_area->style->black_gc;
  else if (active)
  thegc=hlgc;
  else
  thegc=mygc[color];
  
    gdk_draw_segments (picture,
		       thegc,
		       (GdkSegment *) mypoints, (gint) nsegments);
}

void
DrawDotted (int x, int y, int tx, int ty, int active, int color)
/* draws a dotted bond */
{
  GdkGC *thegc;
  GdkPoint mypoints[100];
  int xlen, ylen, i;
  gint npoints ;
  int len;
  float step;

  xlen = tx - x;
  ylen = ty - y;

len=(int) rint(calc_vector(abs(xlen),abs(ylen))/6.3);
 step = 1./len;
npoints=len;
if (npoints>99)npoints=99;

  for (i = 0; i < npoints; i++)
    {
      mypoints[i].x = (gint16) ((x + i * step * xlen) * size_factor);
      mypoints[i].y = (gint16) ((y + i * step * ylen) * size_factor);
    }
  if (xbmflag)
  thegc= drawing_area->style->black_gc;
  else if (active)
  thegc=hlgc;
  else
  thegc=mygc[color];

    gdk_draw_points (picture, thegc,
		     (GdkPoint *) mypoints, (gint) npoints);
}

void
DrawWide (int x, int y, int tx, int ty, int active,int color)
/* draws a wide single bond */
{
  gint npoints = 4;
  GdkPoint mypoints[4];
  struct xy_co *coord;
  GdkGC *thegc;

  if (xbmflag)
  thegc = drawing_area->style->black_gc;
  else if (active)
    thegc = hlgc;
  else
  thegc=mygc[color];
  coord = center_double_bond (x, y, tx, ty, 6); 
  mypoints[0].x = (gint16) (coord->x * size_factor);
  mypoints[0].y = (gint16) (coord->y * size_factor);
  mypoints[1].x = (gint16) (coord->tx * size_factor);
  mypoints[1].y = (gint16) (coord->ty * size_factor);
  coord++;
  mypoints[2].x = (gint16) (coord->tx * size_factor);
  mypoints[2].y = (gint16) (coord->ty * size_factor);
  mypoints[3].x = (gint16) (coord->x * size_factor);
  mypoints[3].y = (gint16) (coord->y * size_factor);
  gdk_draw_polygon (picture, thegc, TRUE, (GdkPoint *) mypoints,
		    (gint) npoints);

}

void
DrawStripe (int x, int y, int tx, int ty, int active, int color)
/* draws a striped single bond */
{
  int i;
  GdkPoint mypoints[4];
  struct xy_co *coord;
  GdkGC *thegc;

  if (xbmflag)
  thegc = drawing_area->style->black_gc;
  else if (active)
    thegc = hlgc;
  else
  thegc=mygc[color];
  coord = center_double_bond (x, y, tx, ty, 4); 
  mypoints[0].x = (gint16) (coord->x * size_factor);
  mypoints[0].y = (gint16) (coord->y * size_factor);
  mypoints[1].x = (gint16) (coord->tx * size_factor);
  mypoints[1].y = (gint16) (coord->ty * size_factor);
  coord++;
  mypoints[2].x = (gint16) (coord->x * size_factor);
  mypoints[2].y = (gint16) (coord->y * size_factor);
  mypoints[3].x = (gint16) (coord->tx * size_factor);
  mypoints[3].y = (gint16) (coord->ty * size_factor);
  for (i = 0; i < 10; i++)
    {
      gdk_draw_line (picture, thegc,
		     mypoints[0].x + i * (mypoints[1].x - mypoints[0].x) / 10,
		     mypoints[0].y + i * (mypoints[1].y - mypoints[0].y) / 10,
		     mypoints[2].x + i * (mypoints[3].x - mypoints[2].x) / 10,
		     mypoints[2].y + i * (mypoints[3].y -
					  mypoints[2].y) / 10);
    }
}


void
DrawCircle (int x, int y, int tx, int ty, int active,int color)
/* draws a circle of radius (tx/ty)-(x/y) around (x/y) */
{
  GdkGC *thegc;
  int topleftx, toplefty;
  int radius;

  if (xbmflag)
  thegc = drawing_area->style->black_gc;
  else if (active )
    thegc = hlgc;
  else
    thegc=mygc[color];
  radius = calc_vector (abs (tx - x), abs (ty - y)) * size_factor;

  topleftx = x * size_factor - radius;
  toplefty = y * size_factor - radius;
  gdk_draw_arc (picture, thegc, FALSE, topleftx, toplefty, 2 * radius,
		2 * radius, 0, 360 * 64);
  if (drawmode == 3 && !xbmflag)
    {
      thegc = hlgc;
      gdk_draw_rectangle (picture, thegc, TRUE, x * size_factor - 1,
			  y * size_factor - 1, 3, 3);
      gdk_draw_rectangle (picture, thegc, TRUE, tx * size_factor - 1,
			  ty * size_factor - 1, 3, 3);
    }
}


void
DrawWiggly (int x, int y, int tx, int ty, int active, int color)
/* draws a wavy line */
{
  gint ax, ay, width, height, angle1, angle2;
  int narcs, veclen, boxlen;
  int i, xlen, ylen, ang;
  int boxcorx, boxcory;

  GdkGC *thegc;
  
  if (xbmflag)
  thegc = drawing_area->style->black_gc;
  else if (active)
  thegc = hlgc;
  else
  thegc=mygc[color];

  narcs = 5;

  xlen = tx - x;
  ylen = ty - y;

  veclen = calc_vector (abs (tx - x), abs (ty - y));
  ang = (int)
    (180. +
    asin (copysign ((float) ylen / (float) veclen, (double) (xlen * -ylen) )) * 180. /
    3.14159);

  boxlen = (int) (0.2 * veclen);
  boxcorx = (int) ((1. - (float) xlen / (float) veclen) * boxlen / 2);
  boxcory = (int) ((1. - (float) ylen / (float) veclen) * boxlen / 2);

  for (i = 0; i < narcs; ++i)
    {
      ax = (gint)((x + 0.2 * i * xlen - boxcorx) * size_factor);
      ay = (gint)((y + 0.2 * i * ylen - boxcory) * size_factor);
      width = boxlen * size_factor;
      height = boxlen * size_factor;
      angle1 = ang * 64;
      angle2 = (gint) (-180 * 64 * pow (-1, (double)i));
      if (x > tx)
	angle2 *= -1;
      gdk_draw_arc (picture, thegc, FALSE, ax, ay, width, height, angle1,
		    angle2);
    }

}

void
DrawArrow (int x, int y, int tx, int ty, int arrow_head, int active, int color)
/* draws an arrow with either full or one-sided arrowhead */
{
  int xlen, ylen, xbase, ybase;
  gint npoints;
  float veclen, headfact, scalefact;
  GdkPoint mypoints[3];
  GdkGC *thegc;

  if (xbmflag)
    thegc = drawing_area->style->black_gc;
  else if (active)
    thegc = hlgc;
  else
    thegc= mygc[color];
    
  xlen = tx - x;
  ylen = ty - y;
  veclen = sqrt ((double)(xlen * xlen + ylen * ylen));
  scalefact=64./veclen; /* keep arrowhead size constant (64=std length)*/ 
/*  headfact = 1. - 20. / veclen;*/
  headfact = 0.8;
  xbase = x + headfact * xlen*scalefact;
  ybase = y + headfact * ylen*scalefact;

  xbase = (int) (tx - 0.2 *xlen*scalefact);
  ybase = (int) (ty - 0.2 *ylen*scalefact);

  mypoints[0].x = tx * size_factor;
  mypoints[0].y = ty * size_factor;

if (select_char (tx,ty,1) != NULL){
      xbase = x+headfact*xlen-12*xlen/veclen;
      ybase = y+headfact*ylen-12*ylen/veclen;
      mypoints[0].x = (gint16) ((tx-12*xlen/veclen)*size_factor);
      mypoints[0].y = (gint16) ((ty-12*ylen/veclen)*size_factor);
}
  mypoints[1].x = (gint16) ((xbase + 0.1 * ylen) * size_factor);
  mypoints[1].y = (gint16) ((ybase - 0.1 * xlen) * size_factor);
  mypoints[1].x = (gint16) ((xbase + 0.1 * ylen*scalefact) * size_factor);
  mypoints[1].y = (gint16) ((ybase - 0.1 * xlen*scalefact) * size_factor);

  if (arrow_head == 1)
    {
      mypoints[2].x = (gint16) (xbase * size_factor);	/*on baseline */
      mypoints[2].y = (gint16) (ybase * size_factor);
    }
  else
    {

      mypoints[2].x = (gint16) ((xbase - 0.1 * ylen) * size_factor);
      mypoints[2].y = (gint16) ((ybase + 0.1 * xlen) * size_factor);

      mypoints[2].x = (gint16) ((xbase - 0.1 * ylen*scalefact) * size_factor);
      mypoints[2].y = (gint16) ((ybase + 0.1 * xlen*scalefact) * size_factor);

    }

  npoints = 3;
  gdk_draw_line (picture, thegc, x * size_factor, y * size_factor,
		 tx * size_factor, ty * size_factor);
  gdk_draw_polygon (picture, thegc, TRUE, (GdkPoint *) mypoints,
		    (gint) npoints);
}

void
DrawDashedWedge (int x, int y, int tx, int ty, int active,int color)
/* draws the dashed wedge for a bond behind the image plane */
{
  int i, xlen, ylen;
  GdkGC *thegc;
  int len;
  float step,factor;
  
  if (xbmflag)
  thegc = drawing_area->style->black_gc;
  else if (active)
    thegc = hlgc;
  else 
  thegc= mygc[color];
  
  xlen = tx - x;
  ylen = ty - y;

len=(int) rint(calc_vector(abs(xlen),abs(ylen))/8.);
if (len<8) len=8;
 step = 1./len;
 factor=0.0125;
 if (len>20) factor= 0.00125;
 if (len==8) {
   factor =0.03;
 }
  for (i = 1; i < len; i++)
    gdk_draw_line (picture, thegc,
		   (x + step * i * xlen - factor * ylen * i) * size_factor,
		   (y + step * i * ylen + factor * xlen * i) * size_factor,
		   (x + step * i * xlen + factor * ylen * i) * size_factor,
		   (y + step * i * ylen - factor * xlen * i) * size_factor);
}

void
DrawWedge (int x, int y, int tx, int ty, int active, int color)
/* draws the wedge-shaped single bond 'pointing towards the viewer' */
{
  int xlen, ylen;
  gint npoints;
  GdkPoint mypoints[3];
  GdkGC *thegc;
  struct data *hpc;
  struct xy_co *coord;
  int d;
  float area;

  if (xbmflag)
  thegc = drawing_area->style->black_gc;
  else if (active)
    thegc = hlgc; 
  else
    thegc = mygc[color];
  xlen = tx - x;
  ylen = ty - y;
  
  float c=1.;
  if (xlen*xlen+ylen*ylen <=1000.) c=2.;

  mypoints[0].x = (gint16) (x * size_factor);
  mypoints[0].y = (gint16) (y * size_factor);
  mypoints[1].x = (gint16) ((tx - 0.1 *c * ylen) * size_factor);
  mypoints[1].y = (gint16) ((ty + 0.1 *c* xlen) * size_factor);
  mypoints[2].x = (gint16) ((tx + 0.1 *c* ylen) * size_factor);
  mypoints[2].y = (gint16) ((ty - 0.1 *c* xlen) * size_factor);

  hpc = da_root.next;
  for (d = 0; d < hp->n; d++)
    {
      if (hpc->bond == 10)
	{
	  if ((abs (hpc->x - tx) < 3 && abs (hpc->y - ty) < 3)
	      || (abs (hpc->tx - tx) < 3 && abs (hpc->ty - ty) < 3))
	    {
	      coord =
		center_double_bond (hpc->x, hpc->y, hpc->tx, hpc->ty, 6);

	      if (abs (hpc->x - tx) < 3 && abs (hpc->y - ty) < 3)
		{

		  mypoints[1].x = (gint16) (coord->x * size_factor);
		  mypoints[1].y = (gint16) (coord->y * size_factor);
		  coord++;
		  mypoints[2].x = (gint16) (coord->x * size_factor);
		  mypoints[2].y = (gint16) (coord->y * size_factor);
		}
	      else
		{
		  mypoints[1].x = (gint16) (coord->tx * size_factor);
		  mypoints[1].y = (gint16) (coord->ty * size_factor);
		  coord++;
		  mypoints[2].x = (gint16) (coord->tx * size_factor);
		  mypoints[2].y = (gint16) (coord->ty * size_factor);
		}
	      area =
		0.5 * abs (mypoints[0].x * (mypoints[1].y - mypoints[2].y) +
			    mypoints[1].x * (mypoints[2].y - mypoints[0].y) +
			    mypoints[2].x * (mypoints[0].y - mypoints[1].y));

	      if (fabs (area) < 76. * size_factor)
		{
		  mypoints[1].x = (gint16) ((tx - 0.05 * ylen) * size_factor);
		  mypoints[1].y = (gint16) ((ty + 0.05 * xlen) * size_factor);
		  mypoints[2].x = (gint16) ((tx + 0.05 * ylen) * size_factor);
		  mypoints[2].y = (gint16) ((ty - 0.05 * xlen) * size_factor);
		}
	    }
	}
      hpc = hpc->next;
    }
  npoints = 3;
  gdk_draw_polygon (picture, thegc, TRUE, (GdkPoint *) mypoints,
		    (gint) npoints);
}

void
Drawspline (int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3,
	    int type, int active, int color)
/* draws a spline curve, optionally with full of half arrowhead  */
{
  int xlen, ylen, xbase, ybase;
  double px0, py0, px1, py1;
  int i;
  double t;
  gint npoints;
  GdkPoint mypoints[21];
  GdkGC *thegc;
  
  if (xbmflag)
    thegc = drawing_area->style->black_gc;
  else if (active)
    thegc = hlgc;
  else
    thegc = mygc[color];
    
  if (drawmode == 3 && !xbmflag)
    {
      gdk_draw_rectangle (picture, thegc, TRUE, x0 * size_factor - 1,
			  y0 * size_factor - 1, 3, 3);
      gdk_draw_rectangle (picture, thegc, TRUE, x1 * size_factor - 1,
			  y1 * size_factor - 1, 3, 3);
      gdk_draw_rectangle (picture, thegc, TRUE, x2 * size_factor - 1,
			  y2 * size_factor - 1, 3, 3);
      gdk_draw_rectangle (picture, thegc, TRUE, x3 * size_factor - 1,
			  y3 * size_factor - 1, 3, 3);
    }
  px0 = (double)x0;
  py0 = (double)y0;
  for (i = 0; i < 21; i++)
    {
      t = (float) i / 20.;
      px1 =
	t * t * t * (double) x3 + 3. * t * t * (1. - t) * (double) x2 +
	3. * t * (1. - t) * (1. - t) * (double) x1 + (1. - t) * (1. -
								 t) * (1. -
								       t) *
	px0;
      py1 =
	t * t * t * (double) y3 + 3. * t * t * (1. - t) * (double) y2 +
	3. * t * (1. - t) * (1. - t) * (double) y1 + (1. - t) * (1. -
								 t) * (1. -
								       t) *
	py0;
      mypoints[i].x = (gint16) (px1 * size_factor);
      mypoints[i].y = (gint16) (py1 * size_factor);
    }
  npoints = 21;
  if (type >= 0) {
 /* move end of last line segment back a bit to keep it from protruding 
    beneath the arrowhead */
    xbase= mypoints[20].x;
    ybase= mypoints[20].y;
    mypoints[20].x = mypoints[20].x - (gint16) ( 0.03*(mypoints[20].x-mypoints[19].x));	
    mypoints[20].y = mypoints[20].y - (gint16) (0.03*(mypoints[20].y-mypoints[19].y));	
    gdk_draw_lines (picture, thegc, mypoints, npoints);
    mypoints[20].x=xbase;
    mypoints[20].y=ybase;
    }
    
  if (type == -1)
    gdk_draw_polygon (picture, thegc, TRUE, mypoints, npoints);
  if (type == -2)
    gdk_draw_points (picture, thegc, mypoints, npoints);

  if (type <= 0)
    return;

  for (i=0;i<4;i++){ /* find acceptable arrowhead length */
  xlen = mypoints[20].x - mypoints[19-i].x;
  ylen = mypoints[20].y - mypoints[19-i].y;
  xbase = mypoints[19-i].x;
  ybase = mypoints[19-i].y;
  if (xlen*xlen+ylen*ylen >200 *size_factor*size_factor) break;
  
  }

  if (xlen*xlen+ylen*ylen >400 *size_factor*size_factor) {
  xlen -= (mypoints[20].x - mypoints[19].x)/2;
  ylen -= (mypoints[20].y - mypoints[19].y)/2;
  xbase = mypoints[19].x +(mypoints[20].x - mypoints[19].x)/2 ;
  ybase = mypoints[19].y +(mypoints[20].y - mypoints[19].y)/2 ;
  }

  if (xlen != 0)
    xlen = (int) copysign (10. * size_factor, (double)xlen);
  if (ylen != 0)
    ylen = (int) copysign (10. * size_factor, (double)ylen);

  /* tip of arrow at original endpoint of last line */
  mypoints[0].x = mypoints[20].x;	
  mypoints[0].y = mypoints[20].y;	

  /* calculate both possible positions for arrow side */
  mypoints[1].x = (gint16) (xbase + 0.5 * ylen);
  mypoints[1].y = (gint16) (ybase - 0.5 * xlen);

  if (type == 1)
    {

      mypoints[2].x = (gint16) (xbase - 0.5 * ylen);
      mypoints[2].y = (gint16) (ybase + 0.5 * xlen);

      /* and choose the one facing outward (farther from the starting point) */
      px0 = (double) 
	((mypoints[1].x - x0 * size_factor) * (mypoints[1].x -
					      x0 * size_factor) +
	(mypoints[1].y - y0 * size_factor) * (mypoints[1].y -
					      y0 * size_factor));
      px1 = (double)
	((mypoints[2].x - x0 * size_factor) * (mypoints[2].x -
					      x0 * size_factor) +
	(mypoints[2].y - y0 * size_factor) * (mypoints[2].y -
					      y0 * size_factor));

      if (px0 < px1)
	{
	  mypoints[1].x = mypoints[2].x;
	  mypoints[1].y = mypoints[2].y;
	}

      mypoints[2].x = (gint16)xbase;	/*on baseline */
      mypoints[2].y = (gint16)ybase;
    }
  else
    {
      mypoints[2].x = (gint16) (xbase - 0.5 * ylen);	/* on opposite side of line */
      mypoints[2].y = (gint16) (ybase + 0.5 * xlen);
    }

  npoints = 3;
  gdk_draw_polygon (picture, thegc, TRUE, (GdkPoint *) mypoints,
		    (gint) npoints);
}


void
Drawstring (int x, int y, char *cc, int direct, int active, int thecolor, int serif_mode, int size, int preview)
/* draws a label with sub- and superscripting, symbols and text justification */
{
  int ha, tw, a;
  int nch, textl, chl, chr;
  int d, hx, n;
  int shiftall = 0;
  int variance[MAXCL];
  gchar *c;
#ifdef GTK2
  int dummy;
  gchar l[8];
  gchar *c_p;
  gunichar hc, hl;
  gunichar text[MAXCL];
  gunichar symbol[MAXCL];
  gunichar slanted[MAXCL];
  gunichar bold[MAXCL];
  static PangoLayout *thelayout;
#else
  char *l, *hc, hl[2];
  unsigned char text[MAXCL];
  char symbol[MAXCL];
  char slanted[MAXCL];
  char bold[MAXCL];
#endif
  GdkGC *thegc;
  int save_x, sub_lastx, sup_lastx;
  int fontsize;
  
  fontsize=size;
  if (zoom_factor <2) fontsize-=2-zoom_factor;
  if (zoom_factor >2) fontsize+=zoom_factor-2;
  if (fontsize<0 ) fontsize=curfontsize;
  if (fontsize>6) fontsize=6;
  if (preview) fontsize=0;
  
  if (xbmflag)
  thegc = drawing_area->style->black_gc;
  else if (active)
    thegc = hlgc;
  else
    thegc = mygc[thecolor];
    
  x = x * size_factor;
  hx = x;
  y = y * size_factor;
  
#ifdef GTK2
memset(l,0,8*sizeof(gchar));
    c=g_locale_to_utf8(cc,-1,NULL,NULL,NULL);
  if (!c)  c=g_convert(cc,-1,"UTF-8","ISO8859-1",NULL,NULL,NULL); 
  if (!c) {
  fprintf(stderr,"invalid character in chemtool file\n");
  return;
  }
  c_p=c;
  if (!thelayout) thelayout = gtk_widget_create_pango_layout(drawing_area,"X");
  pango_layout_set_font_description(thelayout,font[fontsize]);
  pango_layout_get_pixel_size(thelayout, &tw, &a);  
  pango_layout_set_text(thelayout, "x", -1);
  pango_layout_set_font_description(thelayout,font[fontsize]);
  pango_layout_get_pixel_size(thelayout, &tw, &ha);
  ha = ha/2;
  n = (int) g_utf8_strlen(c,-1);
#else  
  c=cc;
  a = gdk_char_height (font[fontsize], 'X');
  ha = gdk_char_height (font[fontsize], 'x') / 2;
  a += 6;
  n = (int)strlen (c);
  tw = gdk_char_width (font[fontsize], 'x') ;
#endif

#ifdef GTK2
  for (d = 0; d < n; d++)
    {
      symbol[d] = FALSE;
      slanted[d] = FALSE;
      bold[d]=FALSE;
      
      hc = g_utf8_get_char(c);

      if (shiftall != 0)
	{
	  if (hc == '}')
	    {
	      shiftall = 0;
	      if (!(c=g_utf8_next_char(c)))continue;
	      --n;
	      hc=g_utf8_get_char(c);
	    }
	  variance[d] = shiftall;
	}
      if (shiftall == 0)
	{
	  if ((variance[d] = Extra_char (hc,ha)) != 0)
	    {
	      c=g_utf8_next_char(c);
	      if ((hc=g_utf8_get_char(c)))
		{
		  --n;
		  if (hc == '{')
		    {
		      --n;
		      shiftall = variance[d];
			c=g_utf8_next_char(c);		    
			hc=g_utf8_get_char(c);
		    }
		}
	      else
		{
		  variance[d] = 0;
		  n=d;
		  continue;
		}
	    }
	}
      text[d] = hc;
      c = g_utf8_next_char(c);
      if (text[d] == '@' && (hl=g_utf8_get_char(c)))
	{
	  symbol[d] = TRUE;
	  text[d] = hl;
    if (text[d] == '+')
      {
        text[d] = 8005;  /* QUAK */
      }
    else if (text[d] == '-')
      {
        text[d] = 8006;  /* QUAK */
      }
    c = g_utf8_next_char (c);
	  --n;
	}

      else if (text[d] == '|' && (hl=g_utf8_get_char(c)))
	{
	  slanted[d] = TRUE;
	  text[d] = hl;
          c = g_utf8_next_char(c);
	  --n;
	}

      else if (text[d] == '#' && (hl=g_utf8_get_char(c)))
	{
	  bold[d] = TRUE;
	  text[d] = hl;
          c = g_utf8_next_char(c);
	  --n;
	}

      else if (text[d] == '\\' && (hl=g_utf8_get_char(c)))
	{
	text[d] = ' ';
	}
    }

#else
  hc = c;
  for (d = 0; d < n; d++)
    {
      symbol[d] = FALSE;
      slanted[d] = FALSE;
      bold[d]=FALSE;
      if (shiftall != 0)
	{
	  if (*hc == '}')
	    {
	      shiftall = 0;
	      if (*(hc + 1))
		hc++;
	      --n;
	    }
	  variance[d] = shiftall;
	}
      if (shiftall == 0)
	{
	  if ((variance[d] = Extra_char (*hc)) != 0)
	    {
	      if (*(hc + 1))
		{
		  hc++;
		  --n;
		  if (*hc == '{')
		    {
		      shiftall = variance[d];
		      hc++;
		      --n;
		    }
		}
	      else
		{
		  variance[d] = 0;
		}
	    }
	}
      text[d] = (unsigned char) *hc++;
      if (text[d] == '@' && *hc)
	{
	  symbol[d] = TRUE;
	  text[d] = (unsigned char) *hc++;
	  --n;
	}

      else if (text[d] == '|' && *hc)
	{
	  slanted[d] = TRUE;
	  text[d] = (unsigned char) *hc++;
	  --n;
	}

      else if (text[d] == '#' && *hc)
	{
	  bold[d] = TRUE;
	  text[d] = (unsigned char) *hc++;
	  --n;
	}

      else if (text[d] == '\\' && *hc)
	{
	text[d] = ' ';
	}
    }
#endif

  nch = n;
  if (nch > 0)
    {
      chl = 0;
      chr = 0;
      x = save_x = sub_lastx = sup_lastx = 0;
      for (d = 0; d < nch; d++)
	{
#ifdef GTK2
/*          l = g_locale_to_utf8((const gchar*) &text[d],1,NULL,NULL,NULL); */
memset(l,0,8*sizeof(gchar));
g_unichar_to_utf8(text[d],l);
#endif          
	  if (variance[d])
	    {
#ifdef GTK2
              pango_layout_set_text(thelayout,l,-1);
              pango_layout_set_font_description(thelayout,smallfont[fontsize]);
              pango_layout_get_pixel_size(thelayout, &tw, &dummy);
#else             
	      tw = gdk_char_width (smallfont[fontsize], (gchar)text[d]) + 2;
#endif
	      if (symbol[d])
#ifdef GTK2
                {
                  pango_layout_set_text(thelayout,l,-1);
                  pango_layout_set_font_description(thelayout,ssymbfont[fontsize]);
                  pango_layout_get_pixel_size(thelayout, &tw, &dummy);
                }
#else                
		tw = gdk_char_width (ssymbfont[fontsize], (gchar)text[d]);
#endif
	      if (variance[d] < 0)
		{
		  if (save_x < x)
		    x = sub_lastx;
		  x = sub_lastx = x + tw;
		}
	      else
		{
		  if (save_x < x)
		    x = sup_lastx;
		  x = sup_lastx = x + tw;
		}
	    }
	  else /* not sub- or superscripted */
	    {
#ifdef GTK2
                  pango_layout_set_text(thelayout,l,-1);
                  pango_layout_set_font_description(thelayout,font[fontsize]);
                  pango_layout_get_pixel_size(thelayout, &tw, &dummy);
#else
	      tw = 1 + gdk_char_width (font[fontsize], (gchar)text[d]);
#endif
	      if (symbol[d])
#ifdef GTK2
                {
                  pango_layout_set_text(thelayout,l,-1);
                  pango_layout_set_font_description(thelayout,symbfont[fontsize]);
                  pango_layout_get_pixel_size(thelayout, &tw, &dummy);
                }
#else                
		tw = gdk_char_width (symbfont[fontsize], (gchar)text[d]);
#endif
	      if (bold[d])
#ifdef GTK2
                {
                  pango_layout_set_text(thelayout,l,-1);
                  pango_layout_set_font_description(thelayout,boldfont[fontsize]);
                  pango_layout_get_pixel_size(thelayout, &tw, &dummy);
                }
#else                
	      	tw = gdk_char_width (boldfont[fontsize], (gchar)text[d]);
#endif
	      if (slanted[d])
#ifdef GTK2
                {
                  pango_layout_set_text(thelayout,l,-1);
                  pango_layout_set_font_description(thelayout,slfont[fontsize]);
                  pango_layout_get_pixel_size(thelayout, &tw, &dummy);
                }
#else                
		tw = 1 + gdk_char_width (slfont[fontsize], (gchar)text[d]);
#endif
	      if (save_x < x) 
		{
		  if (sub_lastx < sup_lastx)
		    x = sup_lastx;
		  else
		    x = sub_lastx;
		}
	      x = save_x = sub_lastx = sup_lastx = x + tw;
	      chr = tw;
	    }
	  if (chl == 0)
	    chl = tw;
	}
	if (x != sub_lastx || x != sup_lastx) save_x+= tw; /* if the label ends with a sub/superscripted character, */
							   /* add its length to the current position (which is before */
							   /* the sub/superscript to allow vertical alignment of simultaneous */
							   /* sub- and superscripts */
      textl = save_x;
      switch (direct)
	{
	case 0:
	  hx = hx - chl / 2;
	  break;
	case -1:
	  hx = hx - (textl - 1) / 2;
	  break;
	case -2:
	  hx = hx + chr / 2 - (textl - 1);
	  break;
	}
    }
  nch = n;
  x = hx;
  save_x = sub_lastx = sup_lastx = x;
  for (d = 0; d < n; d++)
    {
#ifdef GTK2
/*          l = g_locale_to_utf8((const gchar*) &text[d],1,NULL,NULL,NULL);*/
memset(l,0,8*sizeof(gchar));
	g_unichar_to_utf8(text[d], l);
#endif          
      if (variance[d])
	{
#ifdef GTK2
                {
                  pango_layout_set_text(thelayout,l,-1);
                  pango_layout_set_font_description(thelayout,smallfont[fontsize]);
                  pango_layout_get_pixel_size(thelayout, &tw, &dummy);
                }
#else                
	  tw = gdk_char_width (smallfont[fontsize], (gchar)text[d]) + 2;
#endif
	  if (symbol[d])
#ifdef GTK2
                {
                  pango_layout_set_text(thelayout,l,-1);
                  pango_layout_set_font_description(thelayout,ssymbfont[fontsize]);
                  pango_layout_get_pixel_size(thelayout, &tw, &dummy);
                }
#else                
	    tw = gdk_char_width (ssymbfont[fontsize], (gchar)text[d]);
#endif
	  if (variance[d] < 0)
	    {
	      if (save_x < x)
		x = sub_lastx;
	      sub_lastx = x + tw;
	    }
	  else
	    {
	      if (save_x < x)
		x = sup_lastx;
	      sup_lastx = x + tw;
	    }
	}
      else
	{ /* not sub- or superscripted */
#ifdef GTK2
                  pango_layout_set_text(thelayout,l,-1);
                  pango_layout_set_font_description(thelayout,font[fontsize]);
                  pango_layout_get_pixel_size(thelayout, &tw, &dummy);
#else                
	  tw = 1 + gdk_char_width (font[fontsize], (gchar)text[d]);
#endif
	  if (symbol[d])
#ifdef GTK2
                {
                  pango_layout_set_text(thelayout,l,-1);
                  pango_layout_set_font_description(thelayout,symbfont[fontsize]);
                  pango_layout_get_pixel_size(thelayout, &tw, &dummy);
                }
#else                
	    tw = gdk_char_width (symbfont[fontsize], (gchar)text[d]);
#endif
	  if (bold[d])
#ifdef GTK2
                {
                  pango_layout_set_text(thelayout,l,-1);
                  pango_layout_set_font_description(thelayout,boldfont[fontsize]);
                  pango_layout_get_pixel_size(thelayout, &tw, &dummy);
                }
#else                
	    tw = gdk_char_width (boldfont[fontsize], (gchar)text[d]);
#endif	    
	  if (slanted[d])
#ifdef GTK2
                {
                  pango_layout_set_text(thelayout,l,-1);
                  pango_layout_set_font_description(thelayout,slfont[fontsize]);
                  pango_layout_get_pixel_size(thelayout, &tw, &dummy);
                }
#else                
	    tw = 1 + gdk_char_width (slfont[fontsize], (gchar)text[d]);
#endif
	  if (save_x < x)
	    {
	      if (sub_lastx < sup_lastx)
		x = sup_lastx;
	      else
		x = sub_lastx;
	    }
	  save_x = sub_lastx = sup_lastx = x + tw;
	}

/* erase a circular area slightly larger than the current character to
   create some space around the label */
   if (text[d] != 183) {
if (use_whiteout) fprintf(stderr,"whitespace\n");
    if (preview)
      gdk_draw_arc (picture, drawing_area->style->white_gc, 1,
		    x - 5 * size_factor,
		    y - a + ha - 5 * size_factor + variance[d],
		    tw + 8 * size_factor, a + 10 * size_factor, 0, 360 * 64);
	else
      if (use_whiteout) gdk_draw_arc (picture, background_gc, 1,
		    x - 2 * size_factor,
		    y -a + ha + size_factor * variance[d],
		    tw + 8 * size_factor, a + 2 * size_factor, 0, 360 * 64);
      x = x + tw;
      }
    }

  n = nch;
  x = hx;

#ifdef GTK2
  y -= a ;
#endif
  
 
  save_x = sub_lastx = sup_lastx = x;
  for (d = 0; d < n; d++)
    {
#ifdef GTK2
	memset(l,0,8*sizeof(gchar));
/*#ifdef GTK2*/
/*          l = g_locale_to_utf8((const gchar*) &text[d],1,NULL,NULL,NULL);*/
g_unichar_to_utf8(text[d],l);
	if (symbol[d]) {
	unsigned int unicodechar=text[d]+848;
	
	switch (unicodechar){ /* catch sequence mismatches with X Symbol font*/
			case 915: /*C*/
			case 947: /*c*/
				unicodechar+=20;
				break;
			case 918:
			case 950: /*f*/
				unicodechar+=16;
				break;
			case 919:	
			case 951: /*g*/
				unicodechar-=4;
				break;
			case 920:
			case 952: /*h*/
				unicodechar-=1;
				break;
			case 922: /*J is vartheta*/
				unicodechar=977;
				break;
			case 954: /*j is varphi*/
				unicodechar=981;
				break;
			case 923:
			case 924:
			case 925:
			case 955:
			case 956:
			case 957:
				unicodechar-=1;
				break;
			case 926: /*N*/
				unicodechar=78;
				break;
			case 958: /*n*/
				unicodechar=118;
				break;
			case 929: /*Q*/
				unicodechar=920;
				break;
			case 961: /*p*/
				unicodechar=952;
				break;
			case 962: 
				unicodechar-=1;
				break;
			case 966:/*v*/
				unicodechar=982;
				break;
			case 967: /*w*/
				unicodechar=969;
				break;
			case 968:
				unicodechar=958;
				break;
			case 969:
				unicodechar-=1;
				break;
			case 970:
				unicodechar=950;
				break;						
			case 930: /*R*/
				unicodechar-=1;
				break;
			case 934: /*V*/
				unicodechar=962;
				break;
			case 935: /*W*/
				unicodechar+=2;
				break;	
			case 936: /*X*/
				unicodechar=926;
				break;
			case 937: /*Y*/
				unicodechar-=1;
				break;
			case 938:
				unicodechar=90;
				break;	
			case 1031: /*centered dot*/									case 775: /* bullet */
				unicodechar=8226;
				break;	
			default:
				break;
			}				 

	g_unichar_to_utf8((gunichar)unicodechar,l);
	}
#else      
      hl[0] = (char)text[d];
      hl[1] = 0;
      l = &hl[0];
#endif

      if (variance[d])
	{
#ifdef GTK2
                {
                  pango_layout_set_text(thelayout,l,-1);
                  pango_layout_set_font_description(thelayout,smallfont[fontsize]);
                  pango_layout_get_pixel_size(thelayout, &tw, &dummy);
                }
#else                
	  tw = gdk_char_width (smallfont[fontsize], (gchar)text[d]) + 2;
#endif
	  if (symbol[d])
#ifdef GTK2
                {
                  pango_layout_set_text(thelayout,l,-1);
                  pango_layout_set_font_description(thelayout,ssymbfont[fontsize]);
                  pango_layout_get_pixel_size(thelayout, &tw, &dummy);
                }
#else                
	    tw = gdk_char_width (ssymbfont[fontsize], (gchar)text[d]);
#endif

	  if (variance[d] < 0)
	    {
	      if (save_x < x)
		x = sub_lastx;
	      sub_lastx = x + tw;
	    }
	  else
	    {
	      if (save_x < x)
		x = sup_lastx;
	      sup_lastx = x + tw;
	    }

	  if (symbol[d])
	    {
#ifdef GTK2
              pango_layout_set_text(thelayout,l,-1);
              pango_layout_set_font_description(thelayout,ssymbfont[fontsize]);
              gdk_draw_layout(picture,background_gc,x,y+ha+variance[d],thelayout);
              gdk_draw_layout(picture,thegc,x,y+ha+variance[d],thelayout);
#else
	      gdk_draw_text (picture, ssymbfont[fontsize],
			     background_gc, x,
			     y + ha + variance[d], l, (gint)strlen (l));
	      gdk_draw_text (picture, ssymbfont[fontsize], thegc, x,
			     y + ha + variance[d], l, (gint)strlen (l));
#endif
	    }
	  else
	    {
#ifdef GTK2
              pango_layout_set_text(thelayout,l,-1);
              pango_layout_set_font_description(thelayout,smallfont[fontsize]);
              gdk_draw_layout(picture,background_gc,x,y+ha+variance[d],thelayout);
              gdk_draw_layout(picture,thegc,x,y+ha+variance[d],thelayout);
#else
	      gdk_draw_text (picture, smallfont[fontsize],
			     background_gc, x,
			     y + ha + variance[d], l, (gint)strlen (l));
	      gdk_draw_text (picture, smallfont[fontsize], thegc, x,
			     y + ha + variance[d], l, (gint)strlen (l));
#endif
	    }
	  x = x + tw;
	}
      else /* not sub- or superscripted */
	{
#ifdef GTK2
              pango_layout_set_text(thelayout,l,-1);
              pango_layout_set_font_description(thelayout,font[fontsize]);
              pango_layout_get_pixel_size(thelayout,&tw,&dummy);
#else
	  tw = 1 + gdk_char_width (font[fontsize], (gchar)text[d]);
#endif	  
	  if (symbol[d])
#ifdef GTK2
            {
              pango_layout_set_text(thelayout,l,-1);
              pango_layout_set_font_description(thelayout,symbfont[fontsize]);
              pango_layout_get_pixel_size(thelayout,&tw,&dummy);
             } 
#else
	    tw = gdk_char_width (symbfont[fontsize], (gchar)text[d]);
#endif	    
	  if (bold[d])
#ifdef GTK2
            {
              pango_layout_set_text(thelayout,l,-1);
              pango_layout_set_font_description(thelayout,boldfont[fontsize]);
              pango_layout_get_pixel_size(thelayout,&tw,&dummy);
             } 
#else
	    tw = gdk_char_width (boldfont[fontsize], (gchar)text[d]);
#endif	    
	  if (slanted[d])
#ifdef GTK2
            {
              pango_layout_set_text(thelayout,l,-1);
              pango_layout_set_font_description(thelayout,slfont[fontsize]);
              pango_layout_get_pixel_size(thelayout,&tw,&dummy);
             } 
#else
	    tw = 1 + gdk_char_width (slfont[fontsize],(gchar) text[d]);
#endif
	  if (save_x < x)
	    {
	      if (sub_lastx < sup_lastx)
		x = sup_lastx;
	      else
		x = sub_lastx;
	    }

	  if (symbol[d])
	    {
#ifdef GTK2
              pango_layout_set_text(thelayout,l,-1);
              pango_layout_set_font_description(thelayout,symbfont[fontsize]);
              gdk_draw_layout(picture,background_gc,x,y+ha,thelayout);
              gdk_draw_layout(picture,thegc,x,y+ha,thelayout);
#else
	      gdk_draw_text (picture, symbfont[fontsize], background_gc,
			     x, y + ha, l, (gint)strlen (l));
	      gdk_draw_text (picture, symbfont[fontsize], thegc, x, y + ha, l,
			     (gint)strlen (l));
#endif
	    }
	  else if (bold[d])
	    {
#ifdef GTK2
              pango_layout_set_text(thelayout,l,-1);
              pango_layout_set_font_description(thelayout,boldfont[fontsize]);
              gdk_draw_layout(picture,background_gc,x,y+ha,thelayout);
              gdk_draw_layout(picture,thegc,x,y+ha,thelayout);
#else
	      gdk_draw_text (picture, boldfont[fontsize], background_gc,
			     x, y + ha, l, (gint)strlen (l));
	      gdk_draw_text (picture, boldfont[fontsize], thegc, x, y + ha, l, (gint)strlen (l));
#endif
	    }
	  else if (slanted[d])
	    {
#ifdef GTK2
              pango_layout_set_text(thelayout,l,-1);
              pango_layout_set_font_description(thelayout,slfont[fontsize]);
              gdk_draw_layout(picture,background_gc,x,y+ha,thelayout);
              gdk_draw_layout(picture,thegc,x,y+ha,thelayout);
#else
	      gdk_draw_text (picture, slfont[fontsize], background_gc,
			     x, y + ha, l, (gint)strlen (l));
	      gdk_draw_text (picture, slfont[fontsize], thegc, x, y + ha, l, (gint)strlen (l));
#endif
	    }
    else if (serif_mode)
	    {
#ifdef GTK2
              pango_layout_set_text(thelayout,l,-1);
              pango_layout_set_font_description(thelayout,textfont[fontsize]);
              gdk_draw_layout(picture,background_gc,x,y+ha,thelayout);
              gdk_draw_layout(picture,thegc,x,y+ha,thelayout);
#else
	      gdk_draw_text (picture, textfont[fontsize], background_gc,
			     x, y + ha, l, (gint)strlen (l));
	      gdk_draw_text (picture, textfont[fontsize], thegc, x, y + ha, l, (gint)strlen (l));
#endif
	    }
	  else
	    {
#ifdef GTK2
              pango_layout_set_text(thelayout,l,-1);
              pango_layout_set_font_description(thelayout,font[fontsize]);
              gdk_draw_layout(picture,background_gc,x,y+ha,thelayout);
              gdk_draw_layout(picture,thegc,x,y+ha,thelayout);
#else
	      gdk_draw_text (picture, font[fontsize], background_gc,
			     x, y + ha, l, (gint)strlen (l));
	      gdk_draw_text (picture, font[fontsize], thegc, x, y + ha, l, (gint)strlen (l));
#endif
	    }
	  x = x + tw;
	  save_x = sub_lastx = sup_lastx = x;
	}
    }
#ifdef GTK2
        free(c_p);
#endif
}

#ifdef GTK2
int
Extra_char (gunichar c, int ha)
#else
int
Extra_char (char c)
#endif
/* process control characters for sub- and superscripting */
{
#ifndef GTK2
  int ha = gdk_char_height (font[zoom_factor], 'x') / 2;
#endif
  if (c == '_')
    return (ha);
  if (c == '^')
    return (-ha);
  return (0);
}

int
Load_Font ()
/* load the normal, subscript and symbol fonts for all zoom scales */
{

#ifdef GTK2

int i;

  gchar *pangosymtype[]= {"Symbol 8", "Symbol 10", "Symbol 12", "Symbol 14",
                          "Symbol 18", "Symbol 20", "Symbol 24" };
  gchar *pangossymtype[]= {"Symbol 8", "Symbol 8", "Symbol 10", "Symbol 12",
                          "Symbol 14", "Symbol 18", "Symbol 24" };
  gchar *pangofntype[]= { "Helvetica 8", "Helvetica 10", "Helvetica 12",
                          "Helvetica 14", "Helvetica 17", "Helvetica 20", 
                          "Helvetica 24"};
  gchar *pangosfntype[]= { "Helvetica 8", "Helvetica 8", "Helvetica 10",
                           "Helvetica 12", "Helvetica 14", "Helvetica 14", 
                           "Helvetica 20"};
  gchar *pangosltype[]= { "Helvetica oblique 8", "Helvetica oblique 10",
                          "Helvetica oblique 12",  "Helvetica oblique 14",
                          "Helvetica oblique 18", "Helvetica oblique 18",
                          "Helvetica oblique 24"};
  gchar *pangobtype[]= { "Helvetica bold 8", "Helvetica bold 10",
                         "Helvetica bold 12",  "Helvetica bold 14",
                         "Helvetica bold 18", "Helvetica bold 18",
                         "Helvetica bold 24"};
  gchar *pangotexttype[]= { "Times 8", "Times 10",
                            "Times 12",  "Times 14",
                            "Times 18", "Times 18",
                            "Times 24"};
  for (i=0;i<7;i++){
  font[i]=pango_font_description_from_string(pangofntype[i]);
  symbfont[i]=pango_font_description_from_string(pangosymtype[i]);
  ssymbfont[i]=pango_font_description_from_string(pangossymtype[i]);
  smallfont[i]=pango_font_description_from_string(pangosfntype[i]);
  slfont[i]=pango_font_description_from_string(pangosltype[i]);
  boldfont[i]=pango_font_description_from_string(pangobtype[i]);
  textfont[i]=pango_font_description_from_string(pangotexttype[i]);
  }                                                                                                                                                                                                                                                                                                                           

#else

int i,fnt;

/* NB: entries 0 and 1 are used for preview only, the default font is 4(!) */
  gchar *symtype75[] = { "*-symbol-*-*-8-*", "*-symbol-*-*-10-*",
    "*-symbol-*-*-12-*", "*-symbol-*-*-14-*", "*-symbol-*-*-18-*",
    "*-symbol-*-*-18-*", "*-symbol-*-*-24-*"
  };

  gchar *ssymtype75[] = { "*-symbol-*-*-8-*", "*-symbol-*-*-8-*",
    "*-symbol-*-*-10-*", "*-symbol-*-*-12-*", "*-symbol-*-*-14-*",
    "*-symbol-*-*-18-*", "*-symbol-*-*-24-*"
  };

  gchar *symtype100[] = { "*-symbol-*-*-11-*", "*-symbol-*-*-14-*",
    "*-symbol-*-*-17-*", "*-symbol-*-*-20-*", "*-symbol-*-*-25-*",
    "*-symbol-*-*-25-*", "*-symbol-*-*-34-*"
  };

  gchar *ssymtype100[] = { "*-symbol-*-*-11-*", "*-symbol-*-*-11-*",
    "*-symbol-*-*-14-*", "*-symbol-*-*-17-*", "*-symbol-*-*-20-*",
    "*-symbol-*-*-25-*", "*-symbol-*-*-34-*"
  };

  gchar *fntype75[] =
    { "*-helvetica-medium-r-normal--8-*", "*-helvetica-medium-r-normal--10-*",
    "*-helvetica-medium-r-normal--12-*", "*-helvetica-medium-r-normal--14-*",
    "*-helvetica-medium-r-normal--18-*",
    "*-helvetica-medium-r-normal--18-*", "*-helvetica-medium-r-normal--24-*"
  };

  gchar *fntype100[] =
    { "*-helvetica-medium-r-normal--11-*", "*-helvetica-medium-r-normal--14-*",
    "*-helvetica-medium-r-normal--17-*", "*-helvetica-medium-r-normal--20-*",
    "*-helvetica-medium-r-normal--25-*",
    "*-helvetica-medium-r-normal--25-*", "*-helvetica-medium-r-normal--34-*"
  };

  gchar *fallback[] = { "5x7", "6x10", "7x13", "9x15", "8x16",
    "10x20", "12x24"
  };

  gchar *sfntype75[] =
    { "*-helvetica-medium-r-normal--8-*", "*-helvetica-medium-r-normal--8-*",
    "*-helvetica-medium-r-normal--10-*", "*-helvetica-medium-r-normal--12-*",
    "*-helvetica-bold-r-normal--14-*",
    "*-helvetica-bold-r-normal--14-*", "*-helvetica-medium-r-normal--20-*"
  };

  gchar *sfntype100[] =
    { "*-helvetica-medium-r-normal--11-*", "*-helvetica-medium-r-normal--11-*",
    "*-helvetica-medium-r-normal--14-*", "*-helvetica-medium-r-normal--17-*",
    "*-helvetica-bold-r-normal--20-*",
    "*-helvetica-bold-r-normal--20-*", "*-helvetica-medium-r-normal--25-*"
  };
/*  {"5x7", "5x7", "6x10", "8x13", "7x14",
   "8x13bold", "10x20"};*/

  gchar *sltype75[] =
    { "*-helvetica-medium-o-normal--8-*", "*-helvetica-medium-o-normal--10-*",
    "*-helvetica-medium-o-normal--12-*", "*-helvetica-medium-o-normal--14-*",
    "*-helvetica-medium-o-normal--18-*",
    "*-helvetica-medium-o-normal--18-*", "*-helvetica-medium-o-normal--24-*"
  };
  gchar *sltype100[] =
    { "*-helvetica-medium-o-normal--11-*", "*-helvetica-medium-o-normal--14-*",
    "*-helvetica-medium-o-normal--17-*", "*-helvetica-medium-o-normal--20-*",
    "*-helvetica-medium-o-normal--25-*",
    "*-helvetica-medium-o-normal--25-*", "*-helvetica-medium-o-normal--34-*"
  };

  gchar *btype75[] =
    { "*-helvetica-bold-r-normal--8-*", "*-helvetica-bold-r-normal--10-*",
    "*-helvetica-bold-r-normal--12-*", "*-helvetica-bold-r-normal--14-*",
    "*-helvetica-bold-r-normal--18-*",
    "*-helvetica-bold-r-normal--18-*", "*-helvetica-bold-r-normal--24-*"
  };
  gchar *btype100[] =
    { "*-helvetica-bold-r-normal--11-*", "*-helvetica-bold-r-normal--14-*",
    "*-helvetica-bold-r-normal--17-*", "*-helvetica-bold-r-normal--20-*",
    "*-helvetica-bold-r-normal--25-*",
    "*-helvetica-bold-r-normal--25-*", "*-helvetica-bold-r-normal--34-*"
  };

  gchar *texttype75[] =
    { "*-times-medium-r-normal--8-*", "*-times-medium-r-normal--10-*",
    "*-times-medium-r-normal--12-*", "*-times-medium-r-normal--14-*",
    "*-times-medium-r-normal--18-*",
    "*-times-medium-r-normal--18-*", "*-times-medium-r-normal--24-*"
  };
  gchar *texttype100[] =
    { "*-times-medium-r-normal--11-*", "*-times-medium-r-normal--14-*",
    "*-times-medium-r-normal--17-*", "*-times-medium-r-normal--20-*",
    "*-times-medium-r-normal--25-*",
    "*-times-medium-r-normal--25-*", "*-times-medium-r-normal--34-*"
  };


for (i=0;i<7;i++) {

  if (font[i])
    gdk_font_unref (font[i]);
  if (symbfont[i])
    gdk_font_unref (symbfont[i]);
  if (smallfont[i])
    gdk_font_unref (smallfont[i]);
  if (ssymbfont[i])
    gdk_font_unref (ssymbfont[i]);
  if (slfont[i])
    gdk_font_unref (slfont[i]);
  if (boldfont[i])
    gdk_font_unref (boldfont[i]);
  if (textfont[i])
    gdk_font_unref (textfont[i]);
   
   fnt=i;
  if ((font[i] = gdk_font_load (fntype75[fnt])) == NULL)
     if((font[i] = gdk_font_load (fntype100[fnt])) == NULL)
    {
      fprintf (stderr, "failed to load font %s !!!\n", fntype75[fnt]);
      if ((font[i] = gdk_font_load (fallback[fnt])) == NULL)
	return (0);
    }
  if ((symbfont[i] = gdk_font_load (symtype75[fnt])) == NULL)
     if((symbfont[i] = gdk_font_load (symtype100[fnt])) == NULL)
    {
      fprintf (stderr, "Failed to load symbol font, using standard font\n");
      symbfont[i] = font[i];
    }
  if ((ssymbfont[i] = gdk_font_load (ssymtype75[fnt])) == NULL)
  	if((ssymbfont[i] = gdk_font_load (ssymtype100[fnt])) == NULL)
   ssymbfont[i] = font[i];
  if ((smallfont[i] = gdk_font_load (sfntype75[fnt])) == NULL)
      if ((smallfont[i] = gdk_font_load (sfntype100[fnt])) == NULL)
    smallfont[i] = font[i];
  if ((slfont[i] = gdk_font_load (sltype75[fnt])) == NULL)
	if ((slfont[i] = gdk_font_load (sltype100[fnt])) == NULL) 
    slfont[i] = font[i];
  if ((boldfont[i] = gdk_font_load (btype75[fnt])) == NULL)
	if ((boldfont[i] = gdk_font_load (btype100[fnt])) == NULL)
    boldfont[i] = font[i];
  if ((textfont[i] = gdk_font_load (texttype75[fnt])) == NULL)
	if ((textfont[i] = gdk_font_load (texttype100[fnt])) == NULL)
    textfont[i] = font[i];
 } 

#endif

  return (1);
}

void
Set_Line (int lsty)
/* define standard line attributes for bond drawing */
{
  int i;

  for (i=0; i<7; i++)
  gdk_gc_set_line_attributes (mygc[i], lsty, 0,
                              GDK_CAP_BUTT, GDK_JOIN_MITER);

  gdk_gc_set_line_attributes (background_gc, lsty, 0,
			      GDK_CAP_BUTT, GDK_JOIN_MITER);
  gdk_gc_set_line_attributes (drawing_area->style->black_gc, lsty, 0,
			      GDK_CAP_BUTT, GDK_JOIN_MITER);
}
