/*
 *    functions for calculation and memory-management
 */

#include <string.h>

#include "ct1.h"
#ifndef MIN
#define MIN(a, b)        ((a) < (b) ? (a) : (b))
#endif
#ifndef M_PI
#define M_PI           3.14159265358979323846  /* pi */
#endif
#ifndef M_PI_4
#define M_PI_4         0.78539816339744830962  /* pi/4 */
#endif
#define boxgap 25

#ifdef LIBUNDO
#include "undo.h"
#endif

void
del_struct (struct data *hpc)
/* remove the data structure corresponding to a bond */
{
  if (hp->n && hpc)
    {
      (hpc->prev)->next = hpc->next;
      (hpc->next)->prev = hpc->prev;
#ifdef LIBUNDO
      undo_free (hpc);
#else
      free (hpc);
#endif
      hp->n--;
    }
}

struct dc *
select_char (int mx, int my, float s_f)
/* find the label closest to the current cursor position */
{
  int x, y, d;
  struct dc *hpc;
  struct xy_co *coord;

  x = round_coord (mx, s_f);
  y = round_coord (my, s_f);
  coord = position (x, y, x, y, No_angle);
  x = coord->x;
  y = coord->y;

  hpc = dac_root.next;
  for (d = 0; d < hp->nc; d++)
    {
      if (x == hpc->x && y == hpc->y)
	return (hpc);
      hpc = hpc->next;
    }
  return (NULL);
}

void
add_char (int x, int y, char *c, int direct, int marked, int color, int font, int size)
/* add a new label */
{
  struct dc *last;

  strcpy (new_c->c, c);
  new_c->x = x;
  new_c->y = y;
  new_c->direct = direct;
  new_c->marked = marked;
  new_c->color = color;
  new_c->font = font;
  new_c->size = size;
#ifdef LIBUNDO
  new_c->next = (struct dc *) undo_malloc (sizeof (struct dc));
#else
  new_c->next = (struct dc *) malloc (sizeof (struct dc));
#endif
  last = new_c;
  new_c = new_c->next;
  hp->nc++;
  new_c->prev = last;
  new_c->next = NULL;
}

void
del_char (struct dc *hpc)
/* remove a label */
{
  if (hp->nc && hpc)
    {
      (hpc->prev)->next = hpc->next;
      (hpc->next)->prev = hpc->prev;
#ifdef LIBUNDO
      undo_free (hpc);
#else
      free (hpc);
#endif
      hp->nc--;
    }
}

int
round_coord (int i, float step)
/* performs adaptive rounding of the input value */
{
  int h;

  h = i / step;
  if (((float) i / step - h) > 0.5)
    h++;
  return (h);
}

void
add_struct (int x, int y, int tx, int ty, int bond, int smarked, int tmarked, int decoration, int color)
/* adds the data corresponding to a new bond */
{
  struct data *last;
  struct xy_co *coord;
  int lx,ly;

  if (bond == 17)
    {
      add_spline (x, y, (int) (x + 0.5 * (tx - x)), (int)(y - 0.5 * (ty - y)),
		 (int) (x + 0.5 * (tx - x)), (int)(y - 0.5 * (ty - y)), 
		 tx, ty, 1, smarked,color);
      return;
    }
  if (bond == 20)
    {
	coord=multi_bonds(x,y,tx+(tx-x),ty+(ty-y),60);
	lx=coord->tx;
	ly=coord->ty;
	coord=multi_bonds(tx+tx-x,ty+ty-y,x,y,60);
      add_spline (x, y, lx,ly, coord->x, coord->y,
		 x, y , 0, smarked,color);
      return;
    }
  if (bond == 21)
    {
	coord=multi_bonds(x,y,tx+(tx-x),ty+(ty-y),60);
	lx=coord->tx;
	ly=coord->ty;
	coord=multi_bonds(tx+tx-x,ty+ty-y,x,y,60);
      add_spline (x, y, lx,ly, coord->x, coord->y,
		 x, y , -1, smarked,color);
      return;
    }

  new->x = x;
  new->y = y;
  new->tx = tx;
  new->ty = ty;
  new->bond = bond;
  new->smarked = smarked;
  new->tmarked = tmarked;
  new->decoration = decoration;
  new->color = color;
#ifdef LIBUNDO
  new->next = (struct data *) undo_malloc (sizeof (struct data));
#else
  new->next = (struct data *) malloc (sizeof (struct data));
#endif
  last = new;
  new = new->next;
  hp->n++;
  new->prev = last;
  new->next = NULL;
}

void
add_spline (int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3,
	    int type, int marked, int color)
/* adds the data corresponding to a new spline curve*/
{
  struct spline *last;

  sp_new->x0 = x0;
  sp_new->y0 = y0;
  sp_new->x1 = x1;
  sp_new->y1 = y1;
  sp_new->x2 = x2;
  sp_new->y2 = y2;
  sp_new->x3 = x3;
  sp_new->y3 = y3;
  sp_new->type = type;
  sp_new->marked = marked;
  sp_new->color = color;
#ifdef LIBUNDO
  sp_new->next = (struct spline *) undo_malloc (sizeof (struct spline));
#else
  sp_new->next = (struct spline *) malloc (sizeof (struct spline));
#endif
  last = sp_new;
  sp_new = sp_new->next;
  hp->nsp++;
  sp_new->prev = last;
  sp_new->next = NULL;
}

void
del_spline (struct spline *hpsp)
/* remove the data structure corresponding to a spline curve */
{
  if (hp->nsp && hpsp)
    {
      (hpsp->prev)->next = hpsp->next;
      (hpsp->next)->prev = hpsp->prev;
#ifdef LIBUNDO
      undo_free (hpsp);
#else
      free (hpsp);
#endif
      hp->nsp--;
    }
}

struct data *
select_vector (int mx, int my, float s_f)
/* returns the bond that is closest to the current cursor position */
{
  int v1, v2, v, d, x, y, tx, ty;
  struct data *hpc, *hmin;
  int vmin;

  hpc = da_root.next;
  vmin = 999999;
  hmin = 0;
  for (d = 0; d < hp->n; d++)
    {
      x = hpc->x * s_f;
      tx = hpc->tx * s_f;
      y = hpc->y * s_f;
      ty = hpc->ty * s_f;
      v = calc_vector (abs (tx - x), abs (ty - y));
      v1 = calc_vector (abs (mx - x), abs (my - y));
      v2 = calc_vector (abs (tx - mx), abs (ty - my));
      if (v1 + v2 <= v)
	{
	  vmin = MIN (vmin, v - v1 - v2);
	  hmin = hpc;
	}
      hpc = hpc->next;
    }
  if (hmin)
    return (hmin);
  else
    return (NULL);
}

struct xy_co *
multi_bonds (int mx, int my, int mtx, int mty, int r)
/* calculates the line endpoints required for drawing a double or triple bond */
{
  int x = 0, y = 0, tx = 0, ty = 0, dx, dy;
  struct xy_co *coord = &xycoord[0];
  float alpha;
  int v;

  dx = mtx - mx;
  dy = mty - my;
  v = calc_vector (abs(dx), abs(dy));
  alpha = (float) asin ((float) dx / v);
  if ((dx < 0 && dy < 0) || !dy)
    {
      x = (int) (r * cos (alpha - M_PI_4));
      y = (int) (r * sin (alpha - M_PI_4));
      tx = (int) (r * cos (alpha + M_PI_4));
      ty = (int) (r * sin (alpha + M_PI_4));
    }
  if (dx > 0 && dy > 0)
    {
      x = (int)(-r * cos (alpha + M_PI_4));
      y = (int)(r * sin (alpha + M_PI_4));
      tx = (int)(-r * cos (alpha - M_PI_4));
      ty = (int)(r * sin (alpha - M_PI_4));
    }
  if (dx < 0 && dy > 0)
    {
      x = (int) (-r * cos (M_PI + alpha - M_PI_4));
      y = (int) (r * sin (M_PI + alpha - M_PI_4));
      tx = (int) (r * cos (alpha + M_PI_4));
      ty = (int) (-r * sin (alpha + M_PI_4));
    }
  if (dx > 0 && dy < 0)
    {
      x = (int) (-r * cos (alpha + M_PI_4));
      y = (int) (-r * sin (alpha + M_PI_4));
      tx = (int) (-r * cos (alpha - M_PI_4));
      ty = (int) ( -r * sin (alpha - M_PI_4));
    }
  if (!dx && dy < 0)
    {
      x = (int) (-r * cos (M_PI_4));
      y = (int) (-r * sin (M_PI_4));
      tx = (int) (-r * cos (M_PI_4));
      ty = (int) (r * sin (M_PI_4));
    }
  if (!dx && dy > 0)
    {
      x = (int) (r * cos (M_PI_4));
      y = (int) (r * sin (M_PI_4));
      tx = (int) (r * cos (M_PI_4));
      ty = (int) (-r * sin (M_PI_4));
    }

      x = (int) (-r * cos (alpha + M_PI_4));
      y = (int) (-r * sin (alpha + M_PI_4));
      tx = (int)( -r * cos (alpha - M_PI_4));
      ty = (int) (-r * sin (alpha - M_PI_4));
if (dy >0) {
      x = (int) (r * cos (alpha - M_PI_4));
      y = (int) (-r * sin (alpha - M_PI_4));
      tx = (int) (r * cos (alpha + M_PI_4));
      ty = (int) (-r * sin (alpha + M_PI_4));
}
  coord->x = mx + x;
  coord->y = my + y;
  coord->tx = mtx + tx;
  coord->ty = mty + ty;
  return (coord);
}

struct xy_co *
center_double_bond (int mx, int my, int mtx, int mty, int r)
/* returns the line endpoints for drawing a centered double bond */
{
  int x = 0, y = 0, tx = 0, ty = 0;
  int sx = 0, sy = 0, stx = 0, sty = 0;
  struct xy_co *coord = &xycoord[0];
  struct xy_co *coord2 = &xycoord[1];
  float alpha;
  int v, dx, dy;;

  dx = mtx - mx;
  dy = mty - my;
  v = calc_vector (dx, dy);
  alpha = (float) asin ((float) dx / v);

  if ((dx < 0 && dy < 0) || !dy)
    {
      x = (int) (r * cos (alpha - M_PI));
      y = (int) (r * sin (alpha - M_PI));
      tx = (int) (r * cos (alpha + M_PI));
      ty = (int) (r * sin (alpha + M_PI));
      sx = (int) (-r * cos (alpha - M_PI));
      sy = (int) (-r * sin (alpha - M_PI));
      stx = (int) (-r * cos (alpha + M_PI));
      sty = (int) (-r * sin (alpha + M_PI));
    }
  if (dx > 0 && dy > 0)
    {
      x = (int) (-r * cos (alpha + M_PI));
      y = (int) (r * sin (alpha + M_PI));
      tx = (int) (-r * cos (alpha - M_PI));
      ty = (int) (r * sin (alpha - M_PI));
      sx = (int) (r * cos (alpha + M_PI));
      sy = (int) (-r * sin (alpha + M_PI));
      stx = (int) (r * cos (alpha - M_PI));
      sty = (int) (-r * sin (alpha - M_PI));
    }
  if (dx < 0 && dy > 0)
    {
      x = (int) (-r * cos (M_PI + alpha - M_PI));
      y = (int) (r * sin (M_PI + alpha - M_PI));
      tx = (int) (r * cos (alpha + M_PI));
      ty = (int) (-r * sin (alpha + M_PI));
      sx = (int) (r * cos (M_PI + alpha - M_PI));
      sy = (int) (-r * sin (M_PI + alpha - M_PI));
      stx = (int) (-r * cos (alpha + M_PI));
      sty = (int) (r * sin (alpha + M_PI));
    }
  if (dx > 0 && dy < 0)
    {
      x = (int) (-r * cos (alpha + M_PI));
      y = (int) (-r * sin (alpha + M_PI));
      tx = (int) (-r * cos (alpha - M_PI));
      ty = (int) (-r * sin (alpha - M_PI));
      sx = (int) (r * cos (alpha + M_PI));
      sy = (int) (r * sin (alpha + M_PI));
      stx = (int) (r * cos (alpha - M_PI));
      sty = (int) (r * sin (alpha - M_PI));
    }
  if (!dx && dy < 0)
    {
      x =  (int) (-r * cos (M_PI));
      y =  (int) (-r * sin (M_PI));
      tx =  (int) (-r * cos (M_PI));
      ty =  (int) (r * sin (M_PI));
      sx =  (int) (r * cos (M_PI));
      sy =  (int) (r * sin (M_PI));
      stx =  (int) (r * cos (M_PI));
      sty =  (int) (-r * sin (M_PI));
    }
  if (!dx && dy > 0)
    {
      x =  (int) (r * cos (M_PI));
      y =  (int) (r * sin (M_PI));
      tx =  (int) (r * cos (M_PI));
      ty =  (int) (-r * sin (M_PI));
      sx =  (int) (-r * cos (M_PI));
      sy =  (int) (-r * sin (M_PI));
      stx =  (int) (-r * cos (M_PI));
      sty =  (int) (r * sin (M_PI));
    }

  coord->x = mx + x;
  coord->y = my + y;
  coord->tx = mtx + tx;
  coord->ty = mty + ty;
  coord2->x = mx + sx;
  coord2->y = my + sy;
  coord2->tx = mtx + stx;
  coord2->ty = mty + sty;
  return (coord);
}

int
calc_vector (int dx, int dy)
/* returns the length of a bond vector */
{
  return ((int) sqrt ((double)(dx * dx + dy * dy)));
}

struct xy_co *
calc_angle (int x, int y, int ang_type, int angle, int factor)
/* returns the nearest grid position to the cursor */
{
  struct xy_co *coord = &xycoord[0];

  int hex[] = { 0, -64, 32, -55, 55, -32,
    64, 0, 55, 32, 32, 55,
    0, 64, -32, 55, -55, 32,
    -64, 0, -55, -32, -32, -55
  };
  int pent[] = { 0, -64, 37, -51,
    60, -19, 60, 19,
    37, 51, 0, 63,
    -37, 51, -60, 19,
    -60, -19, -37, -51
  };
  int pent_sw[] = { 19, -60, 51, -37,
    63, 0, 51, 37,
    19, 60, -19, 60,
    -51, 37, -63, 0,
    -51, -37, -19, -60
  };
  int octa_angle[] = { 0, 64,
    45, 45,
    63, 0,
    45, -45,
    0, -63,
    -45, -45,
    -63, 0,
    -45, 45
  };

  switch (ang_type)
    {
    case Hex:
      coord->x = x + hex[angle * 2] / factor;
      coord->y = y + hex[angle * 2 + 1] / factor;
      break;
    case Pent:
      coord->x = x + pent[angle * 2] / factor;
      coord->y = y + pent[angle * 2 + 1] / factor;
      break;
    case Pent_switch:
      coord->x = x + pent_sw[angle * 2] / factor;
      coord->y = y + pent_sw[angle * 2 + 1] / factor;
      break;
    case Octa:
      coord->x = x + octa_angle[angle * 2] / factor;
      coord->y = y + octa_angle[angle * 2 + 1] / factor;
      break;
    }
  return (coord);
}

struct xy_co *
position (int x, int y, int ax, int ay, int ang_type)
{
  int angle_count[] = { 0, 12, 10, 10, 8 };
  int d, i, dmax, dx, dy, v, bx, by, bv;
  int hx, hy;
  struct dc *hp_a;
  struct data *hp_b;
  struct spline *hp_s;
  struct xy_co *coord = &xycoord[0];

  bx = x;
  by = y;
  bv = 16;

  hp_b = da_root.next;
  for (d = 0; d < hp->n; d++)
    {
      dx = abs (x - hp_b->x);
      dy = abs (y - hp_b->y);
      v = calc_vector (dx, dy);
      if (v < bv)
	{
	  bv = v;
	  bx = hp_b->x;
	  by = hp_b->y;
	}
      dx = abs (x - hp_b->tx);
      dy = abs (y - hp_b->ty);
      v = calc_vector (dx, dy);
      if (v < bv)
	{
	  bv = v;
	  bx = hp_b->tx;
	  by = hp_b->ty;
	}
      hp_b = hp_b->next;
    }

  hp_a = dac_root.next;
  for (d = 0; d < hp->nc; d++)
    {
      dx = abs (x - hp_a->x);
      dy = abs (y - hp_a->y);
      v = calc_vector (dx, dy);
      if (v < bv)
	{
	  bv = v;
	  bx = hp_a->x;
	  by = hp_a->y;
	}
      hp_a = hp_a->next;
    }

  hp_s = sp_root.next;
  for (d = 0; d < hp->nsp; d++)
    {
      dx = abs (x - hp_s->x0);
      dy = abs (y - hp_s->y0);
      v = calc_vector (dx, dy);
      if (v < bv)
	{
	  bv = v;
	  bx = hp_s->x0;
	  by = hp_s->y0;
	}
      dx = abs (x - hp_s->x1);
      dy = abs (y - hp_s->y1);
      v = calc_vector (dx, dy);
      if (v < bv)
	{
	  bv = v;
	  bx = hp_s->x1;
	  by = hp_s->y1;
	}
      dx = abs (x - hp_s->x2);
      dy = abs (y - hp_s->y2);
      v = calc_vector (dx, dy);
      if (v < bv)
	{
	  bv = v;
	  bx = hp_s->x2;
	  by = hp_s->y2;
	}
      dx = abs (x - hp_s->x3);
      dy = abs (y - hp_s->y3);
      v = calc_vector (dx, dy);
      if (v < bv)
	{
	  bv = v;
	  bx = hp_s->x3;
	  by = hp_s->y3;
	}
      hp_s = hp_s->next;
    }

  dmax = angle_count[ang_type];
  for (i = 1; i < 4; i++)
    {
      for (d = 0; d < dmax; d++)
	{
	  coord = calc_angle (ax, ay, ang_type, d, i);
	  hx = coord->x;
	  hy = coord->y;
	  dx = abs (x - hx);
	  dy = abs (y - hy);
	  v = calc_vector (dx, dy);
	  if (v < bv)
	    {
	      bv = v;
	      bx = hx;
	      by = hy;
	    }
/*retry at double distance */
	  hx = 2*hx-ax ;
	  hy = 2*hy-ay ;
	  dx = abs (x - hx);
	  dy = abs (y - hy);
	  v = calc_vector (dx, dy);
	  if (v < bv)
	    {
	      bv = v;
	      bx = hx;
	      by = hy;
	    }
/*retry at greater distance */
	  hx = 3*coord->x-2*ax ;
	  hy = 3*coord->y-2*ay ;
	  dx = abs (x - hx);
	  dy = abs (y - hy);
	  v = calc_vector (dx, dy);
	  if (v < bv)
	    {
	      bv = v;
	      bx = hx;
	      by = hy;
	    }
	}
    }
  coord->x = bx;
  coord->y = by;
  return (coord);
}

struct xy_co *
intersect (int ax, int ay, int atx, int aty, int bx, int by, int btx, int bty)
/* returns the intersection of two lines */
{
struct xy_co *coord = &xycoord[0];
  /* the direction vectors of the two lines */
  int avx = atx - ax, avy = aty - ay, bvx = btx - bx, bvy = bty - by; 
  float mu; /* the factor of the second line */

  mu = (float)(avx * (ay - by) + avy * (bx - ax)) / (float)(bvy * avx - avy * bvx);

  coord->x = bx + (int)rint(mu * bvx);
  coord->y = by + (int)rint(mu * bvy);

  return (coord);
}

void
setup_data (void)
/* initializes the bond and label arrays */
{
  new = &da_root;
  new->prev = NULL;
#ifdef LIBUNDO
  new->next = (struct data *) undo_malloc (sizeof (struct data));
#else
  new->next = (struct data *) malloc (sizeof (struct data));
#endif
  new = new->next;
  new->prev = &da_root;

  new_c = &dac_root;
  new_c->prev = NULL;
#ifdef LIBUNDO
  new_c->next = (struct dc *) undo_malloc (sizeof (struct dc));
#else
  new_c->next = (struct dc *) malloc (sizeof (struct dc));
#endif
  new_c = new_c->next;
  new_c->prev = &dac_root;

  sp_new = &sp_root;
  sp_new->prev = NULL;
#ifdef LIBUNDO
  sp_new->next = (struct spline *) undo_malloc (sizeof (struct spline));
#else
  sp_new->next = (struct spline *) malloc (sizeof (struct spline));
#endif
  sp_new = sp_new->next;
  sp_new->prev = &sp_root;

#ifdef LIBUNDO
  hp = (struct new *) undo_malloc (sizeof (struct new));
#else
  hp = (struct new *) malloc (sizeof (struct new));
#endif
  hp->n = 0;
  hp->nc = 0;
  hp->nsp = 0;
}

void
clear_data (void)
/* clears the bond and label lists */
{
  struct data *hp_b;
  struct dc *hp_a;
  struct spline *hp_sp;
  int hn, hnc, hnsp, d;

  hn = hp->n;
  hnc = hp->nc;
  hnsp = hp->nsp;

  hp_b = da_root.next;
  for (d = 0; d < hn; d++)
    {
      del_struct (hp_b);
      hp_b = hp_b->next;
    }

  hp_a = dac_root.next;
  for (d = 0; d < hnc; d++)
    {
      del_char (hp_a);
      hp_a = hp_a->next;
    }

  hp_sp = sp_root.next;
  for (d = 0; d < hnsp; d++)
    {
      del_spline (hp_sp);
      hp_sp = hp_sp->next;
    }
}

int
check_pos_rec (int x, int y, int rx, int ry, int rtx, int rty)
/* checks if a given point is inside the rectangle (rx|ry)(rtx|rty)  */
{
  if (x > rx && y > ry && x < rtx && y < rty)
    return (1);
  return (0);
}

int
partial_delete ()
/* deletes the bonds and labels inside the selection rectangle */
{
  int d, r = False;
  int hn;
  struct data *hp_bond;
  struct dc *hp_atom;
  struct spline *hp_spline;

  hp_atom = dac_root.next;
  hn = hp->nc;
  for (d = 0; d < hn; d++)
    {
      if (hp_atom->marked)
	{
	  del_char (hp_atom);
	  r = True;
	}
      hp_atom = hp_atom->next;
      if (!hp_atom)
	break;
    }

  hp_bond = da_root.next;
  hn = hp->n;
  for (d = 0; d < hn; d++)
    {
      if (hp_bond->smarked || hp_bond->tmarked)
	{
	  del_struct (hp_bond);
	  r = True;
	}
      hp_bond = hp_bond->next;
      if (!hp_bond)
	break;
    }

  hp_spline = sp_root.next;
  hn = hp->nsp;
  for (d = 0; d < hn; d++)
    {
      if (hp_spline->marked)
	{
	  del_spline (hp_spline);
	  r = True;
	}
      hp_spline = hp_spline->next;
      if (!hp_spline)
	break;
    }
    if (addflag ==1) 
    	if (refmark==1){
    		addflag=0;
    		refmark=0;
    		refx=refy=0;
    		}
  if (r)
    {
      FreePix ();
      CreatePix ();
      Display_Mol ();
    }
  return (r);
}

void
get_center_marked (int *x, int *y)
/* get the geometric center of the marked fragment */
{
  struct data *hp_bond;
  struct dc *hp_atom;
  struct spline *hp_spline;
  int d;
  int xmax,ymax,xmin,ymin;
  
  xmin=1000000;
  ymin=1000000;
  xmax=-1000000;
  ymax=-1000000;
  
  if (hp->n || hp->nc || hp->nsp)
    {
      hp_atom = dac_root.next;
      for (d = 0; d < hp->nc; d++)
	{
	  if (hp_atom->marked == 1)
	    {
	      if (hp_atom->x > xmax)
		xmax = hp_atom->x;
	      if (hp_atom->y > ymax)
		ymax = hp_atom->y;
	      if (hp_atom->x < xmin)
		xmin = hp_atom->x;
	      if (hp_atom->y < ymin)
		ymin = hp_atom->y;
	    }
	  hp_atom = hp_atom->next;
	}

      hp_bond = da_root.next;
      for (d = 0; d < hp->n; d++)
	{
	  if (hp_bond->smarked == 1)
	    {
	      if (hp_bond->x > xmax)
		xmax = hp_bond->x;
	      if (hp_bond->y > ymax)
		ymax = hp_bond->y;
	      if (hp_bond->x < xmin)
		xmin = hp_bond->x;
	      if (hp_bond->y < ymin)
		ymin = hp_bond->y;
	      if (hp_bond->tx > xmax)
		xmax = hp_bond->tx;
	      if (hp_bond->ty > ymax)
		ymax = hp_bond->ty;
	      if (hp_bond->tx < xmin)
		xmin = hp_bond->tx;
	      if (hp_bond->ty < ymin)
		ymin = hp_bond->ty;
	    }
	  hp_bond = hp_bond->next;
	}
      hp_spline = sp_root.next;
      for (d = 0; d < hp->nsp; d++)
	{
	  if (hp_spline->marked)
	    {
	      if (hp_spline->x0 < xmin)
		xmin = hp_spline->x0;
	      if (hp_spline->x0 > xmax)
		xmax = hp_spline->x0;
	      if (hp_spline->y0 < ymin)
		ymin = hp_spline->y0;
	      if (hp_spline->y0 > ymax)
		ymax = hp_spline->y0;
	      if (hp_spline->x1 < xmin)
		xmin = hp_spline->x1;
	      if (hp_spline->x1 > xmax)
		xmax = hp_spline->x1;
	      if (hp_spline->y1 < ymin)
		ymin = hp_spline->y1;
	      if (hp_spline->y1 > ymax)
		ymax = hp_spline->y1;
	      if (hp_spline->x2 < xmin)
		xmin = hp_spline->x2;
	      if (hp_spline->x2 > xmax)
		xmax = hp_spline->x2;
	      if (hp_spline->y2 < ymin)
		ymin = hp_spline->y2;
	      if (hp_spline->y2 > ymax)
		ymax = hp_spline->y2;
	      if (hp_spline->x3 < xmin)
		xmin = hp_spline->x3;
	      if (hp_spline->x3 > xmax)
		xmax = hp_spline->x3;
	      if (hp_spline->y3 < ymin)
		ymin = hp_spline->y3;
	      if (hp_spline->y3 > ymax)
		ymax = hp_spline->y3;
	    }
	  hp_spline = hp_spline->next;
	}
    }
      *x = (xmin + xmax) / 2;
      *y = (ymin + ymax) / 2;
  return;
}

void
center_mol (void)
/* tries to center the drawing on the drawing area */
{
  struct data *hp_bond;
  struct dc *hp_atom;
  struct spline *hp_spline;
  int xmax = 0, ymax = 0;
  int xmin = head.width, ymin = head.height;
  int d;

  if (hp->n || hp->nc || hp->nsp)
    {

      hp_atom = dac_root.next;
      for (d = 0; d < hp->nc; d++)
	{
	  if (hp_atom->x > xmax)
	    xmax = hp_atom->x;
	  if (hp_atom->y > ymax)
	    ymax = hp_atom->y;
	  if (hp_atom->x < xmin)
	    xmin = hp_atom->x;
	  if (hp_atom->y < ymin)
	    ymin = hp_atom->y;
	  hp_atom = hp_atom->next;
	}

      hp_bond = da_root.next;
      for (d = 0; d < hp->n; d++)
	{
	  if (hp_bond->x > xmax)
	    xmax = hp_bond->x;
	  if (hp_bond->y > ymax)
	    ymax = hp_bond->y;
	  if (hp_bond->x < xmin)
	    xmin = hp_bond->x;
	  if (hp_bond->y < ymin)
	    ymin = hp_bond->y;
	  if (hp_bond->tx > xmax)
	    xmax = hp_bond->tx;
	  if (hp_bond->ty > ymax)
	    ymax = hp_bond->ty;
	  if (hp_bond->tx < xmin)
	    xmin = hp_bond->tx;
	  if (hp_bond->ty < ymin)
	    ymin = hp_bond->ty;
	  hp_bond = hp_bond->next;
	}
      hp_spline = sp_root.next;
      for (d = 0; d < hp->nsp; d++)
	{
	  if (hp_spline->x0 < xmin)
	    xmin = hp_spline->x0;
	  if (hp_spline->x0 > xmax)
	    xmax = hp_spline->x0;
	  if (hp_spline->y0 < ymin)
	    ymin = hp_spline->y0;
	  if (hp_spline->y0 > ymax)
	    ymax = hp_spline->y0;
	  if (hp_spline->x1 < xmin)
	    xmin = hp_spline->x1;
	  if (hp_spline->x1 > xmax)
	    xmax = hp_spline->x1;
	  if (hp_spline->y1 < ymin)
	    ymin = hp_spline->y1;
	  if (hp_spline->y1 > ymax)
	    ymax = hp_spline->y1;
	  if (hp_spline->x2 < xmin)
	    xmin = hp_spline->x2;
	  if (hp_spline->x2 > xmax)
	    xmax = hp_spline->x2;
	  if (hp_spline->y2 < ymin)
	    ymin = hp_spline->y2;
	  if (hp_spline->y2 > ymax)
	    ymax = hp_spline->y2;
	  if (hp_spline->x3 < xmin)
	    xmin = hp_spline->x3;
	  if (hp_spline->x3 > xmax)
	    xmax = hp_spline->x3;
	  if (hp_spline->y3 < ymin)
	    ymin = hp_spline->y3;
	  if (hp_spline->y3 > ymax)
	    ymax = hp_spline->y3;
	  hp_spline = hp_spline->next;
	}

      head.width = xmax + 400 - xmin;
      head.height = ymax + 400 - ymin;

      if (mark.flag)
	{
	  mark.x = mark.x + 200 - xmin;
	  mark.y = mark.y + 200 - ymin;
	}

      hp_atom = dac_root.next;
      for (d = 0; d < hp->nc; d++)
	{
	  hp_atom->x = hp_atom->x - xmin + 200;
	  hp_atom->y = hp_atom->y - ymin + 200;
	  hp_atom = hp_atom->next;
	}

      hp_bond = da_root.next;
      for (d = 0; d < hp->n; d++)
	{
	  hp_bond->x = hp_bond->x - xmin + 200;
	  hp_bond->y = hp_bond->y - ymin + 200;
	  hp_bond->tx = hp_bond->tx - xmin + 200;
	  hp_bond->ty = hp_bond->ty - ymin + 200;
	  hp_bond = hp_bond->next;
	}
    }
  if (hp->nsp)
    {
      hp_spline = sp_root.next;
      for (d = 0; d < hp->nsp; d++)
	{
	  hp_spline->x0 = hp_spline->x0 - xmin + 200;
	  hp_spline->y0 = hp_spline->y0 - ymin + 200;
	  hp_spline->x1 = hp_spline->x1 - xmin + 200;
	  hp_spline->y1 = hp_spline->y1 - ymin + 200;
	  hp_spline->x2 = hp_spline->x2 - xmin + 200;
	  hp_spline->y2 = hp_spline->y2 - ymin + 200;
	  hp_spline->x3 = hp_spline->x3 - xmin + 200;
	  hp_spline->y3 = hp_spline->y3 - ymin + 200;
	  hp_spline = hp_spline->next;
	  if (!hp_spline)
	    break;
	}
    }
}

void
cut_end (int *x, int *y, int tx, int ty, int r)
{
  int dx, dy, v;
  int qx, qy;
  float alpha;

  dx = tx - *x;
  dy = ty - *y;
  v = calc_vector (dx, dy);
  alpha = (float) asin ((float) dx / v);
  qx = (int) (r * sin (alpha));
  qy = (int) (r * cos (alpha));

  if (dx < 0 && dy < 0)
    {
      *x = *x + qx;
      *y = *y - qy;
    }

  if (dx == 0 && dy < 0)
    {
      *y = *y - r;
    }

  if (dx > 0 && dy < 0)
    {
      *x = *x + qx;
      *y = *y - qy;
    }

  if (dx < 0 && dy == 0)
    {
      *x = *x - r;
    }

  if (dx > 0 && dy == 0)
    {
      *x = *x + r;
    }

  if (dx < 0 && dy > 0)
    {
      *x = *x + qx;
      *y = *y + qy;
    }

  if (dx == 0 && dy > 0)
    {
      *y = *y + r;
    }

  if (dx > 0 && dy > 0)
    {
      *x = *x + qx;
      *y = *y + qy;
    }
}

struct xy_co *
bond_cut (int x, int y, int tx, int ty, int r)
{
  struct xy_co *coord = &xycoord[0];
  struct dc *hp_a;
  int d, i, n;
  char *hc;


  hp_a = dac_root.next;
  for (d = 0; d < hp->nc; d++)
    {
      hc = hp_a->c;
#ifdef GTK2
      n = (int)g_utf8_strlen (hp_a->c,-1);
      for (i = 0; i < n; i++)
	{
	  if (Extra_char (*hc,1))
	    {
	      hc++;
	      --n;
	    }
	  hc++;
	}
#else	
      n = (int)strlen (hp_a->c);
      for (i = 0; i < n; i++)
	{
	  if (Extra_char (*hc))
	    {
	      hc++;
	      --n;
	    }
	  hc++;
	}
#endif
      if (x == hp_a->x && y == hp_a->y)
	{
	  if (hp_a->direct == -10)
	    {
	      cut_end (&x, &y, tx, ty, r * n);
	    }
	  else
	    {
	      cut_end (&x, &y, tx, ty, r);
	    }
	}
      if (tx == hp_a->x && ty == hp_a->y)
	{
	  if (hp_a->direct == -10)
	    {
	      cut_end (&tx, &ty, x, y, r * n);
	    }
	  else
	    {
	      cut_end (&tx, &ty, x, y, r);
	    }
	}
      hp_a = hp_a->next;
    }
  coord->x = x;
  coord->y = y;
  coord->tx = tx;
  coord->ty = ty;
  return (coord);
}

int
move_pos (int x, int y, int tx, int ty)
/* updates the position of a given bond or label */
{
  struct data *hp_bond;
  struct dc *hp_atom;
  struct spline *hp_spline;
  int d, f = False;

  hp_bond = da_root.next;
  for (d = 0; d < hp->n; d++)
    {
      if (hp_bond->x == x && hp_bond->y == y)
	{
	  if (hp_bond->bond == 11)
	    {
	      hp_bond->tx = tx + (hp_bond->tx - hp_bond->x);
	      hp_bond->ty = ty + (hp_bond->ty - hp_bond->y);
	    }
	  hp_bond->x = tx;
	  hp_bond->y = ty;
	  f = True;
	}
      if (hp_bond->tx == x && hp_bond->ty == y)
	{
	  hp_bond->tx = tx;
	  hp_bond->ty = ty;
	  f = True;
	}
      hp_bond = hp_bond->next;
    }

  hp_atom = dac_root.next;
  for (d = 0; d < hp->nc; d++)
    {
      if (hp_atom->x == x && hp_atom->y == y)
	{
	  hp_atom->x = tx;
	  hp_atom->y = ty;
	  f = True;
	}
      hp_atom = hp_atom->next;
    }

  hp_spline = sp_root.next;
  for (d = 0; d < hp->nsp; d++)
    {
      if (hp_spline->x0 == x && hp_spline->y0 == y)
	{
	  hp_spline->x0 = tx;
	  hp_spline->y0 = ty;
	  f = True;
	}
      if (hp_spline->x1 == x && hp_spline->y1 == y)
	{
	  hp_spline->x1 = tx;
	  hp_spline->y1 = ty;
	  f = True;
	}
      if (hp_spline->x2 == x && hp_spline->y2 == y)
	{
	  hp_spline->x2 = tx;
	  hp_spline->y2 = ty;
	  f = True;
	}
      if (hp_spline->x3 == x && hp_spline->y3 == y)
	{
	  hp_spline->x3 = tx;
	  hp_spline->y3 = ty;
	  f = True;
	}

      hp_spline = hp_spline->next;
    }

	if ( !f) {
	hp_bond = select_vector(x,y, 1.);
	if (hp_bond) {
	      hp_bond->x += tx-x;
	      hp_bond->y += ty-y;
	      hp_bond->tx += tx-x;
	      hp_bond->ty += ty-y;
		f = True;
		}
	}
  return f;
}

int
partial_move (int dx, int dy)
/* updates the position of all selected bonds and labels */
{
  struct data *hp_bond;
  struct dc *hp_atom;
  struct spline *hp_spline;
  int d, f = False;

  hp_bond = da_root.next;
  for (d = 0; d < hp->n; d++)
    {
      if (hp_bond->smarked == 1)
	{
	  hp_bond->x = hp_bond->x + dx;
	  hp_bond->y = hp_bond->y + dy;
	  f = True;
	}
      if (hp_bond->tmarked == 1)
	{
	  hp_bond->tx = hp_bond->tx + dx;
	  hp_bond->ty = hp_bond->ty + dy;
	  f = True;
	}
      hp_bond = hp_bond->next;
    }

  hp_atom = dac_root.next;
  for (d = 0; d < hp->nc; d++)
    {
      if (hp_atom->marked == 1)
	{
	  hp_atom->x = hp_atom->x + dx;
	  hp_atom->y = hp_atom->y + dy;
	  f = True;
	}
      hp_atom = hp_atom->next;
    }

  if (hp->nsp)
    {
      hp_spline = sp_root.next;
      for (d = 0; d < hp->nsp; d++)
	{
	  if (hp_spline->marked)
	    {
	      hp_spline->x0 = hp_spline->x0 + dx;
	      hp_spline->y0 = hp_spline->y0 + dy;
	      hp_spline->x1 = hp_spline->x1 + dx;
	      hp_spline->y1 = hp_spline->y1 + dy;
	      hp_spline->x2 = hp_spline->x2 + dx;
	      hp_spline->y2 = hp_spline->y2 + dy;
	      hp_spline->x3 = hp_spline->x3 + dx;
	      hp_spline->y3 = hp_spline->y3 + dy;
	      f = True;
	    }
	  hp_spline = hp_spline->next;
	  if (!hp_spline)
	    break;
	}
    }
    if (addflag==1 && refmark ==1)
    			 {
    			refx+=dx;
    			refy+=dy;
    			}
  return f;
}

int
partial_rotate (/*int xcent, int ycent,*/ double rotsin, double rotcos)
/* rotates all marked bonds and labels by a given angle */
{
  struct data *hp_bond;
  struct dc *hp_atom;
  struct spline *hp_spline;
  int d, f = False;
  double tmp;
  int dtmp;
  double rx, ry;

  dtmp = 0;

  hp_bond = da_root.next;
  for (d = 0; d < hp->n; d++)
    {
      if (hp_bond->smarked == 1)
	{
	  rx = (double)tmpx[dtmp];
	  ry = (double)tmpy[dtmp++];
	  tmp = rx * rotcos - ry * rotsin;
	  ry = rx * rotsin + ry * rotcos;
	  tmp = tmp + copysign (0.5, tmp);
	  ry = ry + copysign (0.5, ry);
	  hp_bond->x = (int) tmp + xcent;
	  hp_bond->y = (int) ry + ycent;
	  f = True;
	}
      if (hp_bond->tmarked == 1)
	{
	  rx = (double)tmpx[dtmp];
	  ry = (double)tmpy[dtmp++];
	  tmp = rx * rotcos - ry * rotsin;
	  tmp = tmp + copysign (0.5, tmp);
	  ry = rx * rotsin + ry * rotcos;
	  ry = ry + copysign (0.5, ry);
	  hp_bond->tx = (int) tmp + xcent;
	  hp_bond->ty = (int) ry + ycent;
	  f = True;
	}
      hp_bond = hp_bond->next;
    }

  hp_atom = dac_root.next;
  for (d = 0; d < hp->nc; d++)
    {
      if (hp_atom->marked == 1)
	{
	  rx = (double)tmpx[dtmp];
	  ry = (double)tmpy[dtmp++];
	  tmp = rx * rotcos - ry * rotsin;
	  tmp = tmp + copysign (0.5, tmp);
	  ry = rx * rotsin + ry * rotcos;
	  ry = ry + copysign (0.5, ry);
	  hp_atom->x = (int) tmp + xcent;
	  hp_atom->y = (int) ry + ycent;
	  f = True;
	}
      hp_atom = hp_atom->next;
    }

  hp_spline = sp_root.next;
  for (d = 0; d < hp->nsp; d++)
    {
      if (hp_spline->marked == 1)
	{
	  rx = (double)tmpx[dtmp];
	  ry = (double)tmpy[dtmp++];
	  tmp = rx * rotcos - ry * rotsin;
	  tmp = tmp + copysign (0.5, tmp);
	  ry = rx * rotsin + ry * rotcos;
	  ry = ry + copysign (0.5, ry);
	  hp_spline->x0 = (int) tmp + xcent;
	  hp_spline->y0 = (int) ry + ycent;
	  rx = (double)tmpx[dtmp];
	  ry = (double)tmpy[dtmp++];
	  tmp = rx * rotcos - ry * rotsin;
	  tmp = tmp + copysign (0.5, tmp);
	  ry = rx * rotsin + ry * rotcos;
	  ry = ry + copysign (0.5, ry);
	  hp_spline->x1 = (int) tmp + xcent;
	  hp_spline->y1 = (int) ry + ycent;
	  rx = (double)tmpx[dtmp];
	  ry = (double)tmpy[dtmp++];
	  tmp = rx * rotcos - ry * rotsin;
	  tmp = tmp + copysign (0.5, tmp);
	  ry = rx * rotsin + ry * rotcos;
	  ry = ry + copysign (0.5, ry);
	  hp_spline->x2 = (int) tmp + xcent;
	  hp_spline->y2 = (int) ry + ycent;
	  rx = (double)tmpx[dtmp];
	  ry = (double)tmpy[dtmp++];
	  tmp = rx * rotcos - ry * rotsin;
	  tmp = tmp + copysign (0.5, tmp);
	  ry = rx * rotsin + ry * rotcos;
	  ry = ry + copysign (0.5, ry);
	  hp_spline->x3 = (int) tmp + xcent;
	  hp_spline->y3 = (int) ry + ycent;
	  f = True;
	}
      hp_spline = hp_spline->next;
    }
	if (addflag==1 && refmark==1){
	  rx = (double)tmpx[dtmp];
	  ry = (double)tmpy[dtmp++];
	  tmp = rx * rotcos - ry * rotsin;
	  tmp = tmp + copysign (0.5, tmp);
	  ry = rx * rotsin + ry * rotcos;
	  ry = ry + copysign (0.5, ry);
	  refx = (int) tmp + xcent;
	  refy = (int) ry + ycent;
	}
  return f;
}

void
flip_horiz ()
/** flips all currently marked bonds and labels horizontally 
    center line of the marker box serves as mirror plane **/
{
  struct data *hp_bond;
  struct dc *hp_atom;
  struct spline *hp_spline;
  int d, dummy;
  hp_bond = da_root.next;
  hp_atom = dac_root.next;
/*  xref = mark.x + mark.w / 2;*/

  if (hp->n == 0 && hp->nsp == 0)
    return;
  get_center_marked (&xref, &dummy);

  for (d = 0; d < hp->n; d++)
    {
      if (hp_bond->smarked || hp_bond->tmarked)
	{
	  hp_bond->x = hp_bond->x - 2 * (hp_bond->x - xref);
	  hp_bond->tx = hp_bond->tx - 2 * (hp_bond->tx - xref);
	  switch (hp_bond->bond)
	  {
	  	case 1:
	  		hp_bond->bond=2;
	  		break;
	  	case 2: 
	  		hp_bond->bond=1;
	  		break;
	  	case 14:
	  		hp_bond->bond=15;
	  		break;
	  	case 15:
	  		hp_bond->bond=14;
	  		break;
	  	default:
	  		break;
	  	}					 
	}
      hp_bond = hp_bond->next;
    }
  for (d = 0; d < hp->nc; d++)
    {
      if (hp_atom->marked)
	hp_atom->x = hp_atom->x - 2 * (hp_atom->x - xref);
      hp_atom = hp_atom->next;
    }

  if (hp->nsp)
    {
      hp_spline = sp_root.next;
      for (d = 0; d < hp->nsp; d++)
	{
	  if (hp_spline->marked)
	    {
	      hp_spline->x0 = hp_spline->x0 - 2 * (hp_spline->x0 - xref);
	      hp_spline->x1 = hp_spline->x1 - 2 * (hp_spline->x1 - xref);
	      hp_spline->x2 = hp_spline->x2 - 2 * (hp_spline->x2 - xref);
	      hp_spline->x3 = hp_spline->x3 - 2 * (hp_spline->x3 - xref);
	    }
	  hp_spline = hp_spline->next;
	  if (!hp_spline)
	    break;
	}
    }
    if (addflag==1 && refmark ==1 ) 
    		refx = refx - 2* (refx-xref);
	
  FreePix ();
  CreatePix ();
  Display_Mol ();
}

void
flip_vert ()
/** flips all currently marked bonds and labels vertically 
    center line of the marker box serves as mirror plane **/
{
  struct data *hp_bond;
  struct dc *hp_atom;
  struct spline *hp_spline;
  int d, dummy;

  if (hp->n == 0 && hp->nsp == 0)
    return;

  hp_bond = da_root.next;
  hp_atom = dac_root.next;
/*  yref = mark.y + mark.h / 2;*/
  get_center_marked (&dummy, &yref);

  for (d = 0; d < hp->n; d++)
    {
      if (hp_bond->smarked || hp_bond->tmarked)
	{
	  hp_bond->y = hp_bond->y - 2 * (hp_bond->y - yref);
	  hp_bond->ty = hp_bond->ty - 2 * (hp_bond->ty - yref);
	  switch (hp_bond->bond)
	  {
	  	case 1:
	  		hp_bond->bond=2;
	  		break;
	  	case 2: 
	  		hp_bond->bond=1;
	  		break;
	  	case 14:
	  		hp_bond->bond=15;
	  		break;
	  	case 15:
	  		hp_bond->bond=14;
	  		break;
	  	default:
	  		break;
	  	}					 
	}
      hp_bond = hp_bond->next;
    }
  for (d = 0; d < hp->nc; d++)
    {
      if (hp_atom->marked)
	hp_atom->y = hp_atom->y - 2 * (hp_atom->y - yref);
      hp_atom = hp_atom->next;
    }
  if (hp->nsp)
    {
      hp_spline = sp_root.next;
      for (d = 0; d < hp->nsp; d++)
	{
	  if (hp_spline->marked)
	    {
	      hp_spline->y0 = hp_spline->y0 - 2 * (hp_spline->y0 - yref);
	      hp_spline->y1 = hp_spline->y1 - 2 * (hp_spline->y1 - yref);
	      hp_spline->y2 = hp_spline->y2 - 2 * (hp_spline->y2 - yref);
	      hp_spline->y3 = hp_spline->y3 - 2 * (hp_spline->y3 - yref);
	    }
	  hp_spline = hp_spline->next;
	  if (!hp_spline)
	    break;
	}
    }

    if (addflag==1 && refmark ==1 )
    		refy = refy - 2* (refy-yref);

  FreePix ();
  CreatePix ();
  Display_Mol ();
}

void
copy_obj ()
/** creates a copy of the marked object, transfering the 
mark to the newly generated fragment. Dangling bonds, where one end is 
linked to something outside the box, are ignored. **/
{
  struct data *hp_bond;
  struct dc *hp_atom;
  struct spline *hp_spline;
  int d, n_end, nc_end, nsp_end;

  if (hp->n == 0 && hp->nc == 0 && hp->nsp == 0)
    return;

  hp_bond = da_root.next;
  hp_atom = dac_root.next;
  n_end = hp->n;
  nc_end = hp->nc;
  nsp_end = hp->nsp;

  for (d = 0; d < n_end; d++)
    {
      if (hp_bond->smarked && hp_bond->tmarked)
	add_struct (hp_bond->x , hp_bond->y ,
		    hp_bond->tx , hp_bond->ty , hp_bond->bond, 1, 1,hp_bond->decoration, hp_bond->color);
      hp_bond->smarked = hp_bond->tmarked = 0;
      hp_bond = hp_bond->next;
    }
  for (d = 0; d < nc_end; d++)
    {
      if (hp_atom->marked)
	add_char (hp_atom->x , hp_atom->y , hp_atom->c,
		  hp_atom->direct, 1, hp_atom->color,hp_atom->font,hp_atom->size);
      hp_atom->marked = 0;
      hp_atom = hp_atom->next;
    }

  hp_spline = sp_root.next;
  for (d = 0; d < nsp_end; d++)
    {
      if (hp_spline->marked)
	{
	  add_spline (hp_spline->x0 , hp_spline->y0 ,
		      hp_spline->x1 , hp_spline->y1 ,
		      hp_spline->x2 , hp_spline->y2 ,
		      hp_spline->x3 , hp_spline->y3 ,
		      hp_spline->type, 1, hp_spline->color);
	  hp_spline->marked = 0;
	}
      hp_spline = hp_spline->next;
    }

  FreePix ();
  CreatePix ();
  Display_Mol ();
}

int
partial_rescale (int updown, int aniso)
/* updates the position of all selected bonds and labels */
{
  struct data *hp_bond;
  struct dc *hp_atom;
  struct spline *hp_spline;
  double factor, factorx;
  int rx, ry, dtmp;
  int d, f = False;

  if (hp->n == 0 && hp->nsp == 0)
    return (f);
  if (updown < 0)
    factor = 1. - abs (updown) / 10.;
  else
    factor = 1. + updown / 10.;
  if (factor < 0.) return f; /* negative scale would cause strange inversions */
  if (aniso)
    factorx = 1.;
  else
    factorx = factor;

  dtmp = 0;

  hp_bond = da_root.next;
  for (d = 0; d < hp->n; d++)
    {
      if (hp_bond->smarked == 1)
	{
	  rx = tmpx[dtmp];
	  ry = tmpy[dtmp++];
	  hp_bond->x = (int) (rx * factorx) + xcent;
	  hp_bond->y = (int) (ry * factor) + ycent;
	  f = True;
	}
      if (hp_bond->tmarked == 1)
	{
	  rx = tmpx[dtmp];
	  ry = tmpy[dtmp++];
	  hp_bond->tx = (int) (rx * factorx) + xcent;
	  hp_bond->ty = (int) (ry * factor) + ycent;
	  f = True;
	}
      hp_bond = hp_bond->next;
    }

  hp_atom = dac_root.next;
  for (d = 0; d < hp->nc; d++)
    {
      if (hp_atom->marked == 1)
	{
	  rx = tmpx[dtmp];
	  ry = tmpy[dtmp++];
	  hp_atom->x = (int) (rx * factorx) + xcent;
	  hp_atom->y = (int) (ry * factor) + ycent;
	  if (factor < 0.9) hp_atom->size -=1 ;
	  if (factor < 0.6) hp_atom->size -=1 ;
	  if (factor < 0.3) hp_atom->size -=1 ;
	  if (factor > 1.1) hp_atom->size +=1 ;
	  if (factor > 1.3) hp_atom->size +=1 ;
	  if (factor > 1.6) hp_atom->size +=1 ;
	  if (hp_atom->size <0) hp_atom->size=0;
	  if (hp_atom->size >6) hp_atom->size=6;
	  f = True;
	}
      hp_atom = hp_atom->next;
    }

  hp_spline = sp_root.next;
  for (d = 0; d < hp->nsp; d++)
    {
      if (hp_spline->marked == 1)
	{
	  rx = tmpx[dtmp];
	  ry = tmpy[dtmp++];
	  hp_spline->x0 = (int) (rx * factorx) + xcent;
	  hp_spline->y0 = (int) (ry * factor) + ycent;
	  rx = tmpx[dtmp];
	  ry = tmpy[dtmp++];
	  hp_spline->x1 = (int) (rx * factorx) + xcent;
	  hp_spline->y1 = (int) (ry * factor) + ycent;
	  rx = tmpx[dtmp];
	  ry = tmpy[dtmp++];
	  hp_spline->x2 = (int) (rx * factorx) + xcent;
	  hp_spline->y2 = (int) (ry * factor) + ycent;
	  rx = tmpx[dtmp];
	  ry = tmpy[dtmp++];
	  hp_spline->x3 = (int) (rx * factorx) + xcent;
	  hp_spline->y3 = (int) (ry * factor) + ycent;
	  f = True;
	}
      hp_spline = hp_spline->next;
    }
	if (addflag == 1 && refmark == 1) {
	  rx = tmpx[dtmp];
	  ry = tmpy[dtmp++];
	  refx = (int) (rx * factorx) + xcent;
	  refy = (int) (ry * factor) + ycent;
	  }
	  
  return f;
}

void
tidy_mol ()
{
  int hn, hnc;
  int d, t;
  int r = 0;
  struct data *hp_bond, *tmp_bond;
  struct dc *hp_atom;

  if (hp->n == 0 && hp->nc == 0 && hp->nsp == 0)
    return;
  Unmark_all ();
  hp_bond = da_root.next;
  hn = hp->n;
  for (d = 0; d < hn; d++)	/* for all bonds */
    {
	if (hp_bond->bond == 11) continue; /* do not bother with circles */
	/* straigthen near-vertical or near-horizontal bonds */
	if (abs (hp_bond->y - hp_bond->ty) ==1) hp_bond->ty=hp_bond->y;
	if (abs (hp_bond->x - hp_bond->tx) ==1) hp_bond->tx=hp_bond->x;
	 
      tmp_bond = da_root.next;
      for (t = 0; t < d; t++)	/* check against all preceding bonds */
	{
	if (tmp_bond->bond == 11) continue;
	if (hp_bond->decoration || tmp_bond->decoration == 1) continue;
/* if startpoint or endpoint very close to another, snap to that */
	  if (abs (hp_bond->x - tmp_bond->x) < 5 &&
	      abs (hp_bond->y - tmp_bond->y) < 5)
	    {
	      hp_bond->x = tmp_bond->x;
	      hp_bond->y = tmp_bond->y;
	    }
	  if (abs (hp_bond->tx - tmp_bond->x) < 5 &&
	      abs (hp_bond->ty - tmp_bond->y) < 5)
	    {
	      hp_bond->tx = tmp_bond->x;
	      hp_bond->ty = tmp_bond->y;
	    }
	  if (abs (hp_bond->tx - tmp_bond->tx) < 5 &&
	      abs (hp_bond->ty - tmp_bond->ty) < 5)
	    {
	      hp_bond->tx = tmp_bond->tx;
	      hp_bond->ty = tmp_bond->ty;
	    }
	  if (abs (hp_bond->x - tmp_bond->tx) < 5 &&
	      abs (hp_bond->y - tmp_bond->ty) < 5)
	    {
	      hp_bond->x = tmp_bond->tx;
	      hp_bond->y = tmp_bond->ty;
	    }


/* if startpoint _and_ endpoint overlap another bond, mark for deletion */

	  if (abs (hp_bond->x - tmp_bond->x) < 5 &&
	      abs (hp_bond->y - tmp_bond->y) < 5 &&
	      abs (hp_bond->tx - tmp_bond->tx) < 5 &&
	      abs (hp_bond->ty - tmp_bond->ty) < 5)
	    {
	      if (hp_bond->bond <= tmp_bond->bond)
		hp_bond->smarked = hp_bond->tmarked = 1;
	      else
		tmp_bond->smarked = tmp_bond->tmarked = 1;
	    }
	  if (abs (hp_bond->tx - tmp_bond->x) < 5 &&
	      abs (hp_bond->ty - tmp_bond->y) < 5 &&
	      abs (hp_bond->x - tmp_bond->tx) < 5 &&
	      abs (hp_bond->y - tmp_bond->ty) < 5)
	    {
	      if (hp_bond->bond <= tmp_bond->bond)
		hp_bond->smarked = hp_bond->tmarked = 1;
	      else
		tmp_bond->smarked = tmp_bond->tmarked = 1;
	    }
	  tmp_bond = tmp_bond->next;
	}

      hp_atom = dac_root.next;
      hnc = hp->nc;
      for (t = 0; t < hnc; t++)
	{
/* if a label is close to either startpoint or endpoint, move it */
	  if (abs (hp_atom->x - hp_bond->x) < 5 &&
	      abs (hp_atom->y - hp_bond->y) < 5)
	    {
	      hp_atom->x = hp_bond->x;
	      hp_atom->y = hp_bond->y;
	    }
	  if (abs (hp_atom->x - hp_bond->tx) < 5 &&
	      abs (hp_atom->y - hp_bond->ty) < 5)
	    {
	      hp_atom->x = hp_bond->tx;
	      hp_atom->y = hp_bond->ty;
	    }
	  hp_atom = hp_atom->next;
	  if (!hp_atom)
	    break;
	}

      hp_bond = hp_bond->next;
      if (!hp_bond)
	break;
    }
  hp_bond = da_root.next;
  hn = hp->n;
  for (d = 0; d < hn; d++)
    {
      if (hp_bond->smarked && hp_bond->tmarked)
	{
	  del_struct (hp_bond);
	  r = True;
	}
      hp_bond = hp_bond->next;
      if (!hp_bond)
	break;
    }
  hp_atom = dac_root.next;
  hn = hp->nc;
  for (d = 0; d < hn; d++)
    {
      if (hp_atom->marked)
	{
	  del_char (hp_atom);
	  r = True;
	}
      hp_atom = hp_atom->next;
      if (!hp_atom)
	break;
    }
  FreePix ();
  CreatePix ();
  Display_Mol ();
  return;
}


void
add_bracket ()
{

  int gauge;
  if (importflag != 0)
    return;
  gauge = 10 + (boxgap * mark.h / 600 * mark.w / 600);
  add_struct (mark.x, mark.y, mark.x + gauge, mark.y, 0, 0, 0, 1, curpen);
  add_struct (mark.x, mark.y, mark.x, mark.y + mark.h, 0, 0, 0, 1, curpen);
  add_struct (mark.x, mark.y + mark.h, mark.x + gauge, mark.y + mark.h, 0, 0,
	      0, 1, curpen);
  add_struct (mark.x + mark.w - gauge, mark.y, mark.x + mark.w, mark.y, 0, 0,
	      0, 1, curpen);
  add_struct (mark.x + mark.w, mark.y, mark.x + mark.w, mark.y + mark.h, 0, 0,
	      0, 1, curpen);
  add_struct (mark.x + mark.w - gauge, mark.y + mark.h, mark.x + mark.w,
	      mark.y + mark.h, 0, 0, 0, 1, curpen);
  FreePix ();
  CreatePix ();
  Display_Mol ();
}

void
add_r_bracket ()
{

  int gauge;
  if (importflag != 0)
    return;
  gauge = 10 + (boxgap * mark.h / 600 * mark.w / 600);
  add_struct (mark.x, mark.y + gauge, mark.x, mark.y + mark.h - gauge, 0, 0,
	      0, 1, curpen);
  add_struct (mark.x + mark.w, mark.y + gauge, mark.x + mark.w,
	      mark.y + mark.h - gauge, 0, 0, 0, 1, curpen);
  add_spline (mark.x, mark.y + gauge, mark.x, mark.y + gauge / 2,
	      mark.x + gauge / 2, mark.y, mark.x + gauge, mark.y, 0, 0,curpen);
  add_spline (mark.x + mark.w - gauge, mark.y, mark.x + mark.w - gauge / 2,
	      mark.y, mark.x + mark.w, mark.y + gauge / 2, mark.x + mark.w,
	      mark.y + gauge, 0, 0,curpen);
  add_spline (mark.x, mark.y + mark.h - gauge, mark.x,
	      mark.y + mark.h - gauge / 2, mark.x + gauge / 2,
	      mark.y + mark.h, mark.x + gauge, mark.y + mark.h, 0, 0,curpen);
  add_spline (mark.x + mark.w - gauge, mark.y + mark.h,
	      mark.x + mark.w - gauge / 2, mark.y + mark.h, mark.x + mark.w,
	      mark.y + mark.h - gauge / 2, mark.x + mark.w,
	      mark.y + mark.h - gauge, 0, 0,curpen);
  FreePix ();
  CreatePix ();
  Display_Mol ();
}

void
add_r2_bracket ()
{

  int gauge;
  if (importflag != 0)
    return;
  gauge = 10 + (boxgap * mark.h / 600 * mark.w / 600);
  add_spline (mark.x+gauge,mark.y,  mark.x-gauge,mark.y+gauge/2,
              mark.x-gauge,mark.y+mark.h-gauge/2, mark.x+gauge,mark.y+mark.h,0,0,curpen);
  add_spline (mark.x+mark.w-gauge,mark.y,  mark.x+mark.w+gauge,mark.y+gauge/2,
              mark.x+mark.w+gauge,mark.y+mark.h-gauge/2, mark.x+mark.w-gauge,mark.y+mark.h,0,0,curpen);
  FreePix ();
  CreatePix ();
  Display_Mol ();
}

void
add_brace ()
{

  int gauge;
  if (importflag != 0)
    return;
  gauge = 10 + (boxgap * mark.h / 600 * mark.w / 600);
  add_spline (mark.x + gauge, mark.y, mark.x - gauge, mark.y, mark.x + gauge,
	      mark.y + mark.h / 2, mark.x - gauge, mark.y + mark.h / 2, 0, 0,curpen);
  add_spline (mark.x - gauge, mark.y + mark.h / 2, mark.x + gauge,
	      mark.y + mark.h / 2, mark.x - gauge, mark.y + mark.h,
	      mark.x + gauge, mark.y + mark.h, 0, 0,curpen);
  add_spline (mark.x + mark.w - gauge, mark.y, mark.x + mark.w + gauge,
	      mark.y, mark.x + mark.w - gauge, mark.y + mark.h / 2,
	      mark.x + mark.w + gauge, mark.y + mark.h / 2, 0, 0,curpen);
  add_spline (mark.x + mark.w + gauge, mark.y + mark.h / 2,
	      mark.x + mark.w - gauge, mark.y + mark.h / 2,
	      mark.x + mark.w + gauge, mark.y + mark.h,
	      mark.x + mark.w - gauge, mark.y + mark.h, 0, 0,curpen);
  FreePix ();
  CreatePix ();
  Display_Mol ();
}



void
add_box1 ()
{
  if (importflag != 0)
    return;

  add_struct (mark.x, mark.y, mark.x + mark.w, mark.y, 0, 0, 0, 1, curpen);
  add_struct (mark.x, mark.y, mark.x, mark.y + mark.h, 0, 0, 0, 1, curpen);
  add_struct (mark.x, mark.y + mark.h, mark.x + mark.w, mark.y + mark.h, 0, 0,
	      0, 1, curpen);
  add_struct (mark.x + mark.w, mark.y, mark.x + mark.w, mark.y + mark.h, 0, 0,
	      0, 1, curpen);
  FreePix ();
  CreatePix ();
  Display_Mol ();
}


void
add_box2 ()
{
  if (importflag != 0)
    return;

  add_struct (mark.x, mark.y, mark.x + mark.w, mark.y, 0, 0, 0, 1, curpen);
  add_struct (mark.x, mark.y, mark.x, mark.y + mark.h, 0, 0, 0, 1, curpen);
  add_struct (mark.x, mark.y + mark.h, mark.x + mark.w, mark.y + mark.h, 0, 0,
	      0, 1, curpen);
  add_struct (mark.x + mark.w, mark.y, mark.x + mark.w, mark.y + mark.h, 0, 0,
	      0, 1, curpen);
  add_struct (mark.x + mark.w + 2, mark.y + 3, mark.x + mark.w + 2,
	      mark.y + mark.h + 3, 0, 0, 0, 1, curpen);
  add_struct (mark.x + mark.w + 3, mark.y + mark.h + 2, mark.x + 3,
	      mark.y + mark.h + 2, 0, 0, 0, 1, curpen);
  FreePix ();
  CreatePix ();
  Display_Mol ();
}


void
add_box3 ()
{
  if (importflag != 0)
    return;

  add_struct (mark.x, mark.y, mark.x + mark.w, mark.y, 0, 0, 0, 1, curpen);
  add_struct (mark.x, mark.y, mark.x, mark.y + mark.h, 0, 0, 0, 1, curpen);
  add_struct (mark.x, mark.y + mark.h, mark.x + mark.w, mark.y + mark.h, 0, 0,
	      0, 1, curpen);
  add_struct (mark.x + mark.w, mark.y, mark.x + mark.w, mark.y + mark.h, 0, 0,
	      0, 1, curpen);
  add_struct (mark.x + mark.w + 5, mark.y + 8, mark.x + mark.w + 5,
	      mark.y + mark.h + 8, 10, 0, 0, 1, curpen);
  add_struct (mark.x + mark.w + 9, mark.y + mark.h + 5, mark.x + 9,
	      mark.y + mark.h + 5, 10, 0, 0, 1, curpen);
  FreePix ();
  CreatePix ();
  Display_Mol ();
}


void
add_box4 ()
{
  if (importflag != 0)
    return;

  add_struct (mark.x + boxgap, mark.y, mark.x + mark.w - boxgap, mark.y, 0, 0,
	      0, 1, curpen);
  add_struct (mark.x, mark.y + boxgap, mark.x, mark.y + mark.h - boxgap, 0, 0,
	      0, 1, curpen);
  add_struct (mark.x + boxgap, mark.y + mark.h, mark.x + mark.w - boxgap,
	      mark.y + mark.h, 0, 0, 0, 1, curpen);
  add_struct (mark.x + mark.w, mark.y + boxgap, mark.x + mark.w,
	      mark.y + mark.h - boxgap, 0, 0, 0, 1, curpen);
  add_spline (mark.x, mark.y + boxgap, mark.x, mark.y, mark.x, mark.y,
	      mark.x + boxgap, mark.y, 0, 0,curpen);
  add_spline (mark.x + mark.w - boxgap, mark.y, mark.x + mark.w, mark.y,
	      mark.x + mark.w, mark.y, mark.x + mark.w, mark.y + boxgap, 0,
	      0,curpen);
  add_spline (mark.x, mark.y + mark.h - boxgap, mark.x, mark.y + mark.h,
	      mark.x, mark.y + mark.h, mark.x + boxgap, mark.y + mark.h, 0,
	      0,curpen);
  add_spline (mark.x + mark.w - boxgap, mark.y + mark.h, mark.x + mark.w,
	      mark.y + mark.h, mark.x + mark.w, mark.y + mark.h,
	      mark.x + mark.w, mark.y + mark.h - boxgap, 0, 0,curpen);
  FreePix ();
  CreatePix ();
  Display_Mol ();
}


int
has_label(int x, int y)
/* checks if a given coordinate (i.e. bond-limit) has a label/atom on it */
{
  int d;
  struct dc *hp_atom;  
  hp_atom = dac_root.next;
  for (d = 0; d < hp->nc; d++)
    {
      if ( abs(hp_atom->x-x)<5  && abs(hp_atom->y-y)<5 )
 	return d;
      hp_atom = hp_atom->next;
    }
  return -1;
}
 
int
endpoint_connected(int x, int y, int tx, int ty)
/* checks if the endpoint of a bond has another bond on it */
{
  int d;
  struct data *hp_bond;
  hp_bond = da_root.next;
  for (d = 0; d < hp->n; d++) {
    if (hp_bond->x == tx && hp_bond->y == ty) {
      /* this bond starts at our endpoint */
      if (hp_bond->tx != x && hp_bond->ty != y)
        /* and its not 'our' bond, so return true */
        return 0;
    }
    if (hp_bond->tx == tx && hp_bond->ty == ty) {
      /* same as above, it ends at our endpoint this time though */
      if (hp_bond->x != x && hp_bond->y != y)
        return 0;
    }
    hp_bond = hp_bond->next;
  }
  return 1;
}


