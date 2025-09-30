/*
 *    functions for in and output
 */


#include "ct1.h"
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
extern GdkFont *font[7],*smallfont[7],*symbfont[7],*boldfont[7],*slfont[7];
#endif

static char babeloutp[4];

int
save_mol (FILE * fp, int partial)
/* saves the current structure to a file in native chemtool format */
{
  int d;
  int w, h;
  struct data *hp_b;
  struct dc *hp_a;
  struct spline *hp_sp;
  int mbonds = 0, mlabels = 0, msplines = 0;

#ifdef GTK2
  setlocale (LC_NUMERIC,"C");
#endif
  
/* get true extents of drawing*/
  w = 0;
  h = 0;
  hp_b = da_root.next;
  for (d = 0; d < hp->n; d++)
    {
      if (partial == 1 && hp_b->smarked == 0 && hp_b->tmarked == 0)
	{
	  hp_b = hp_b->next;
	}
      else
	{
	  if (partial)
	    mbonds++;
	  w = MAX (w, hp_b->x);
	  w = MAX (w, hp_b->tx);
	  h = MAX (h, hp_b->y);
	  h = MAX (h, hp_b->ty);
	  hp_b = hp_b->next;
	}
    }
  hp_a = dac_root.next;
  for (d = 0; d < hp->nc; d++)
    {
      if (partial && !hp_a->marked)
	{
	  hp_a = hp_a->next;
	}
      else
	{
	  if (partial)
	    mlabels++;
	  w = MAX (w, hp_a->x);
	  h = MAX (h, hp_a->y);
	  hp_a = hp_a->next;
	}
    }

  hp_sp = sp_root.next;
  for (d = 0; d < hp->nsp; d++)
    {
      if (partial && !hp_sp->marked)
	{
	  hp_sp = hp_sp->next;
	}
      else
	{
	  if (partial)
	    msplines++;
	  w = MAX (w, hp_sp->x0);
	  h = MAX (h, hp_sp->y0);
	  w = MAX (w, hp_sp->x1);
	  h = MAX (h, hp_sp->y1);
	  w = MAX (w, hp_sp->x2);
	  h = MAX (h, hp_sp->y2);
	  w = MAX (w, hp_sp->x3);
	  h = MAX (h, hp_sp->y3);
	  hp_sp = hp_sp->next;
	}
    }

  w = (int) (w * 1.1);
  h = (int) (h * 1.1);
  fprintf (fp, "Chemtool Version " VERSION "\n");

  fprintf (fp, "geometry %i %i\n", w, h);

  hp_b = da_root.next;
  if (partial)
    fprintf (fp, "bonds %i\n", mbonds);
  else
    fprintf (fp, "bonds %i\n", hp->n);
  for (d = 0; d < hp->n; d++)
    {
      if (partial && !hp_b->smarked && !hp_b->tmarked)
	{
	  hp_b = hp_b->next;
	}
      else
	{
/*	  if (hp_b->decoration == 1) {*/
	    fprintf (fp, "%i	%i	%i	%i	%i	%i	%i\n",
	  	   hp_b->x, hp_b->y, hp_b->tx, hp_b->ty, hp_b->bond, hp_b->decoration,hp_b->color);
/*	  }else{
	    fprintf (fp, "%i	%i	%i	%i	%i\n",
		   hp_b->x, hp_b->y, hp_b->tx, hp_b->ty, hp_b->bond);
	  }*/
	  hp_b = hp_b->next;
	}
    }

  hp_a = dac_root.next;
  if (partial)
    fprintf (fp, "atoms %i\n", mlabels);
  else
    fprintf (fp, "atoms %i\n", hp->nc);
  for (d = 0; d < hp->nc; d++)
    {
      if (partial && !hp_a->marked)
	{
	  hp_a = hp_a->next;
	}
      else
	{
	  fprintf (fp, "%i	%i	%s	%i	%i	%i	%i\n",
		   hp_a->x, hp_a->y, hp_a->c, hp_a->direct, hp_a->color,hp_a->font,hp_a->size);
	  hp_a = hp_a->next;
	}
    }

  hp_sp = sp_root.next;
  if (partial)
    fprintf (fp, "splines %i\n", msplines);
  else
    fprintf (fp, "splines %i\n", hp->nsp);
  for (d = 0; d < hp->nsp; d++)
    {
      if (partial && !hp_sp->marked)
	{
	  hp_sp = hp_sp->next;
	}
      else
	{
	  fprintf (fp,
		   "%i	%i	%i	%i	%i	%i	%i	%i	%i	%i\n",
		   hp_sp->x0, hp_sp->y0, hp_sp->x1, hp_sp->y1, hp_sp->x2,
		   hp_sp->y2, hp_sp->x3, hp_sp->y3, hp_sp->type,hp_sp->color);
	  hp_sp = hp_sp->next;
	}
    }
   
   if (addflag==1) {
   	fprintf (fp,"attach %i %i\n",refx,refy);
	}
   fprintf(fp,"scalefactors %f %d\n",bondlen_mm,zoom_factor);
/*  fclose (fp);*/
  return (0);
}

int
load_mol (char *filename)
/* loads a chemtool file, checking for correct header */
{
  int d, n, i;
  int x, y, tx, ty, x2, y2, x3, y3, b, deco, col, fnt, siz;
  int res;
  char str[255], str1[255];
  FILE *fp;
  char version[10];

#ifdef GTK2
  setlocale (LC_NUMERIC,"C");
#endif
  
  if (!strcmp(filename,"-")) fp=fdopen(0,"r");
    else
  if ((fp = fopen (filename, "r")) == NULL)
    return (1);

  res = fscanf (fp, "%s %s %s", str, str1, version);
  if (res != 3) 
    {
    fclose (fp);
    return (2);
    }  
  if (strcmp (str, "Chemtool") || strcmp (str1, "Version"))
    {
      fclose (fp);
      return (2);
    }

  clear_data ();

  res = fscanf (fp, "%s %i %i", str, &x, &y);
  if (res == 3 && !strcmp (str, "geometry"))
    {
      head.width = MAX (head.width, x);
      head.height = MAX (head.height, y);
    }

  res = fscanf (fp, "%s %i", str, &n);
  if (res == 2 && (!strcmp (str, "bonds") || !strcmp (str, "bounds")))
    {				/* typo in versions < 1.1.2 */
	res = fgetc(fp);
	if (res == EOF) {
          fclose (fp);
          return (2);
        }
      for (d = 0; d < n; d++)
	{
	if (!fgets(str,80,fp)) {
          fclose (fp);
          return (2);
        }
        i=sscanf (str, "%i %i %i %i %i %i %i", &x, &y, &tx, &ty, &b, &deco, &col);
	if (i == 7)
	add_struct (x, y, tx, ty, b, 0, 0, deco, col);
	else if (i == 6)  
	add_struct (x, y, tx, ty, b, 0, 0, deco, 0);
	else
	add_struct (x, y, tx, ty, b, 0, 0, 0, 0);
	}
    }

  res = fscanf (fp, "%s %i", str, &n);
  if (res == 2 && !strcmp (str, "atoms"))
    {
	res = fgetc(fp);
	if (res == EOF) {
          fclose (fp);
          return (2);
        }
      for (d = 0; d < n; d++)
	{
	if (!fgets(str,33+MAXCL,fp)){
          fclose (fp);
          return (2);
          }
	  i=sscanf (str, "%i %i %s %i %i %i %i", &x, &y, str, &b, &col, &fnt, &siz);
	  if (i==7)
	  	add_char (x, y, str, b, 0, col, fnt, siz);
	  else if (i == 6)
	  	add_char (x, y, str, b, 0, col, fnt, 3);
	  else if (i<5)
	  	add_char (x, y, str, b, 0, 0, 0, 3);
	  	else
		add_char (x, y, str, b, 0, col, 0, 3);
	}
    }

  res = fscanf (fp, "%s %i", str, &n);
  if (res == 2 && !strcmp (str, "splines"))
    {
    if (n > 0) {
      res = fgetc(fp);
      if (res == EOF) {
          fclose (fp);
          return (2);
          }
      for (d = 0; d < n; d++)
	{
	if (!fgets(str,80,fp)){
          fclose (fp);
          return (2);
          }
	  i=sscanf (str, "%i %i %i %i %i %i %i %i %i %i", &x, &y, &tx, &ty,
		  &x2, &y2, &x3, &y3, &b,&col);
	if (i<10)
		add_spline (x, y, tx, ty, x2, y2, x3, y3, b, 0, 0);
		else
		add_spline (x, y, tx, ty, x2, y2, x3, y3, b, 0, col);
	}
    }
  }
  res = fscanf (fp, "%s %s %i", str, str1,&y);
	if (res == 3 && !strcmp(str,"attach")){
			refx=atoi(str1);
			refy=y;
			addflag=1;
	} else if (res == 3 && !strcmp(str,"scalefactors")){
			double bondlen_new = atof(str1);
			if (fabs(bondlen_mm - bondlen_new)>1.e-3){
  			  size_factor *= bondlen_new/bondlen_mm;
  			  bondlen_mm = bondlen_new;
  			}  
			zoom_factor=y;
			Zoom(NULL,"2");
	}

  res = fscanf (fp, "%s %s %d", str, str1, &y);
	if (res == 3 && !strcmp(str,"scalefactors")){
			double bondlen_new = atof(str1);
			if (fabs(bondlen_mm - bondlen_new)>1.e-3){
  			  size_factor *= bondlen_new/bondlen_mm;
  			  bondlen_mm = bondlen_new;
  			}  
			zoom_factor=y;
			Zoom(NULL,"2");
	}
  fclose (fp);
  if (atof (version) > atof (VERSION))
    return (3);
  return (0);
}

int
load_preview (char *filename)
/* loads a chemtool file, checking for correct header */
{
  int d, n;
  int x, y, tx, ty, x2, y2, x3, y3, b, col, fnt;
  int res;
  float savedfactor;
  char str[255], str1[255];
  FILE *fp;
  char version[10];
  GdkRectangle update_rect;
  int fontsize;
  
#ifdef GTK2
  setlocale (LC_NUMERIC,"C");
#endif
    
  if ((fp = fopen (filename, "r")) == NULL)
    return (1);
  savedfactor = size_factor;
  res = fscanf (fp, "%s %s %s", str, str1, version);
  if (res != 3)
    {
      fclose (fp);
      return (2);
    }
  if (strcmp (str, "Chemtool") || strcmp (str1, "Version"))
    {
      fclose (fp);
      return (2);
    }

  res = fscanf (fp, "%s %i %i", str, &x, &y);
  if (res != 3)
    {
      fclose (fp);
      return (2);
    }
  size_factor = MIN (200. / x, 100. / y);

  res = fscanf (fp, "%s %i", str, &n);
  if (res == 2 && (!strcmp (str, "bonds") || !strcmp (str, "bounds")))
    {				/* typo in versions < 1.1.2 */
     res = fgetc(fp);
     if (res == EOF) {
       fclose(fp);
       return(2);
     }
	 for (d = 0; d < n; d++)
	{
	if (!fgets(str,80,fp)) {
          fclose(fp);
          return(2);
        }
	  res = sscanf (str, "%i %i %i %i %i %i", &x, &y, &tx, &ty, &b, &col);
	  if (res >= 5) draw_preview_bonds (x, y, tx, ty, b);
	}
    }

  res = fscanf (fp, "%s %i", str, &n);
  if (res == 2 && !strcmp (str, "atoms"))
    {
      if (size_factor < 0.15)
	fontsize=0;
      else
	fontsize=1;

	res = fgetc(fp);
        if (res == EOF) {
          fclose(fp);
          return(2);
        }

      for (d = 0; d < n; d++)
	{
	if (!fgets(str,33+MAXCL,fp)){
          fclose(fp);
          return(2);
          }
 	  res = sscanf (str, "%i %i %s %i %i %i", &x, &y, str, &b, &col,&fnt);
	  if (res >= 4) Drawstring (x, y, str, b, 0,0,0,1,fontsize);
	}
    }

  res = fscanf (fp, "%s %i", str, &n);
  if (res == 2 && !strcmp (str, "splines"))
    {
    if (n>0) {
    res = fgetc(fp);
     if (res == EOF) {
          fclose(fp);
          return(2);
          }
    
      for (d = 0; d < n; d++)
	{
	if (!fgets(str,80,fp)){
          fclose(fp);
          return(2);
          }
	  res = sscanf (str, "%i %i %i %i %i %i %i %i %i %i", &x, &y, &tx, &ty,
		  &x2, &y2, &x3, &y3, &b,&col);
	  if (res >= 9) Drawspline (x, y, tx, ty, x2, y2, x3, y3, b, 0,0);
	}
    }
  }  
  res = fscanf (fp, "%s %i %i", str, &xref,&yref);
	if (res == 3 && !strcmp(str,"attach")){
		draw_preview_bonds(xref-4,yref-4,xref+4,yref+4,0);
		draw_preview_bonds(xref-4,yref+4,xref+4,yref-4,0);
	}
  gdk_draw_pixmap (preview_area->window,
		   drawing_area->style->
		   fg_gc[GTK_WIDGET_STATE (drawing_area)], picture, 0, 0, 0,
		   0, 200, 100);
  update_rect.x = 0;
  update_rect.y = 0;
  update_rect.width = 200;
  update_rect.height = 100;
  gtk_widget_draw ((GtkWidget *) preview_area, &update_rect);
  size_factor = savedfactor;
  fclose (fp);
  return (0);
}

int
add_mol (char *filename)
/* loads a chemtool file and adds its contents at a given position */
{
  int d, n, i;
  int x, y, tx, ty, x2, y2, x3, y3, b, deco, col, fnt, siz;
  int res;
  int xdiff = 0, ydiff = 0;
  char str[255];
  FILE *fp;
  char version[10];

#ifdef GTK2
  setlocale (LC_NUMERIC,"C");
#endif
  
  if ((fp = fopen (filename, "r")) == NULL)
    return (1);

  res = fscanf (fp, "%s %s %s", str, str, version);
  if (res != 3 || atof (version) > atof (VERSION))
    {
      fclose (fp);
      return (2);
    }
  Unmark_all();

  res = fscanf (fp, "%s %i %i", str, &x, &y);
  if (res == 3 && !strcmp (str, "geometry"))
    {
      head.width += x;
      mark.w = x;
      head.height += y;
      mark.h = y;
      mark.flag = 1;
    }
  res = fscanf (fp, "%s %i", str, &n);
  if (res == 2 && (!strcmp (str, "bonds") || !strcmp (str, "bounds")))
    {
	res = fgetc(fp);
        if (res == EOF) {
          fclose(fp);
          return(2);
          }
      for (d = 0; d < n; d++)
	{
	  if (!fgets(str,80,fp)){
          fclose(fp);
          return(2);
          }
	  i= sscanf (str, "%i %i %i %i %i %i %i", &x, &y, &tx, &ty, &b, &deco, &col);

	if (i == 7)
	add_struct (x, y, tx, ty, b, 1, 1, deco, col);
	else if (i == 6)  
		add_struct (x, y, tx, ty, b, 1, 1, deco, 0);
	else
		add_struct (x, y, tx, ty, b, 1, 1, 0, 0);
	}
    }

  res = fscanf (fp, "%s %i", str, &n);
  if (res == 2 && !strcmp (str, "atoms"))
    {
    res = fgetc(fp);
      if (res == EOF) {    
          fclose(fp);
          return(2);
          }

      for (d = 0; d < n; d++)
	{
	if (!fgets(str,33+MAXCL,fp))
	{
          fclose(fp);
          return(2);
          }
	  i=sscanf (str, "%i %i %s %i %i %i %i", &x, &y, str, &b, &col, &fnt, &siz);
	  x = x + xdiff;
	  y = y + ydiff;
		if (i==7)
		add_char (x, y, str, b, 1, col, fnt, siz);
		else if (i==6)
		add_char (x, y, str, b, 1, col, fnt, 3);
	  	else if (i<5)
		add_char (x, y, str, b, 1, 0, 0, 3);
		else
		add_char (x, y, str, b, 1, col, 0, 3);
	}
    }

  res = fscanf (fp, "%s %i ", str, &n);
  if (res == 2 && !strcmp (str, "splines"))
    {
    if (n>0) {
    res = fgetc(fp);
      if (res == EOF) {    
          fclose(fp);
          return(2);
          }
      for (d = 0; d < n; d++)
	{
	if (!fgets(str,80,fp)) {
          fclose(fp);
          return(2);
          }
	  i=sscanf (str, "%i %i %i %i %i %i %i %i %i %i", &x, &y, &tx, &ty,
		  &x2, &y2, &x3, &y3, &b,&col);
	  	if (i<10)
	  	add_spline (x, y, tx, ty, x2, y2, x3, y3, b, 0,0);
		else
	  	add_spline (x, y, tx, ty, x2, y2, x3, y3, b, 0,col);
	}
    }
  }
  res = fscanf (fp, "%s %i %i", str, &xref,&yref);
  if (res == 3 && !strcmp (str, "attach")) {
  xref=refx-xref;
  yref=refy-yref;
  partial_move (xref,yref);
  }                                 

  fclose(fp);
  return (0);
}

int
import_pdb (char *filename)
{
  char line[255];
  int i, j,m;
  int res;
  int connect = 0;
  double dist;
  double pdbxmin = 100000., pdbxmax = -100000., pdbymin = 1000000., pdbymax =
    -100000., pdbzmin = 100000., pdbzmax = -100000.;
  char code[8];
  float pdbfactor = 50.;
  int pdboffset = 500;
  int at0, con[6];
  FILE *fp;
  int *atnum=NULL;
  int found=0;

#ifdef GTK2
  setlocale (LC_NUMERIC,"C");
#endif
  
  if ((fp = fopen (filename, "r")) == NULL)
    return (1);

  /* skip to the first atom or heteroatom record  */
  for (i = 0;; i++)
    {
      if (fgets (line, (int)sizeof (line), fp) == NULL)
	{
	  fclose (fp);
	  return (1);
	}
      if (!strncmp (line, "END", 3) || feof (fp))
	{			/*premature end */
	  fclose (fp);
	  return (1);
	}
      if (!strncmp (line, "ATOM", 4) || !strncmp (line, "HETATM", 6))
	break;
    }

  clear_data ();
  FreePix();
  i = -1;
  /* read in atom block data */
  while (!feof (fp))
    {
      if (!strncmp (line, "ATOM", 4) || !strncmp (line, "HETATM", 6))
	{
	  i++;
	  pdbx = realloc (pdbx, (i + 1) * sizeof (double));
	  pdby = realloc (pdby, (i + 1) * sizeof (double));
	  pdbz = realloc (pdbz, (i + 1) * sizeof (double));
	  atjust = realloc (atjust, (i + 1) * sizeof (short));
	  atcode = realloc (atcode, (i + 1) * sizeof (char *));
	  atnum = realloc (atnum,(i+1)*sizeof(int));
	  atcode[i] = malloc (9 * sizeof (char));
	  atjust[i] = 0;
	  res = sscanf (line, "%s %d %s %*6c %*6c %lf %lf %lf", code, &atnum[i],atcode[i],
		  &pdbx[i], &pdby[i], &pdbz[i]);
	  if (res < 6) { 
	    fclose (fp);
	    return (1);
          }
	  pdbxmin = MIN (pdbxmin, pdbx[i]);
	  pdbxmax = MAX (pdbxmax, pdbx[i]);

	  pdbymin = MIN (pdbymin, pdby[i]);
	  pdbymax = MAX (pdbymax, pdby[i]);

	  pdbzmin = MIN (pdbzmin, pdbz[i]);
	  pdbzmax = MAX (pdbzmax, pdbz[i]);
	}
      else
	break;
      if (!fgets (line, (int)sizeof (line), fp)) break;
    }
  pdbn = i;
  nbonds = -1;

  while (!feof (fp))
    {
      if (!strncmp (line, "CONECT", 6))
	{
	  connect = 1;
	  con[0] = con[1] = con[2] = con[3] = con[4] = con[5] = 0;
	  res = sscanf (line, "%*s %d %d %d %d %d %d %d", &at0, &con[0], &con[1], &con[2],
		  &con[3],&con[4],&con[5]);
	  for (i = 0; i < res; i++)
	    {
	      if (con[i] > 0)
		{
	        found=0;
		for (m=0;m<=pdbn;m++){
			if (atnum[m]==at0) {
				at0=m+1;
				found=1;
				break;
				}
			}	
		for (m=0;m<=pdbn;m++){
			if (atnum[m]==con[i]) {
				con[i]=m+1;
				found++;
				break;
				}
			}	
		if (found==2){
		  nbonds++;
		  bondfrom = realloc (bondfrom, (nbonds + 1) * sizeof (int));
		  bondto = realloc (bondto, (nbonds + 1) * sizeof (int));
		  bondtype =
		    realloc (bondtype, (nbonds + 1) * sizeof (short));
		  bondfrom[nbonds] = at0 - 1;
		  bondto[nbonds] = con[i] - 1;
		  bondtype[nbonds] = 0;
		  dist =
		    (pdbx[at0 - 1] - pdbx[con[i] - 1]) * (pdbx[at0 - 1] -
							  pdbx[con[i] - 1]) +
		    (pdby[at0 - 1] - pdby[con[i] - 1]) * (pdby[at0 - 1] -
							  pdby[con[i] - 1]) +
		    (pdbz[at0 - 1] - pdbz[con[i] - 1]) * (pdbz[at0 - 1] -
							  pdbz[con[i] - 1]);

		  if (dist < 1.45 * 1.45)
		    {
		      if (!strncmp (atcode[at0 - 1], "C", 1)
			  && !strncmp (atcode[con[i] - 1], "C", 1))
			bondtype[nbonds] = 4;
		    }
/*	fprintf(stderr,"CONECTED %d %d (%f)\n",at0,con[i],dist);*/
/*		  Drawline ((int) (pdbx[at0 - 1] * pdbfactor + pdboffset),
			    (int) (pdby[at0 - 1] * pdbfactor + pdboffset),
			    (int) (pdbx[con[i] - 1] * pdbfactor + pdboffset),
			    (int) (pdby[con[i] - 1] * pdbfactor + pdboffset),
			    1);*/
			}
		}
	    }
	}
      else
	{
	  break;
	}
      (void)fgets (line, (int)sizeof (line), fp);
    }

  fclose (fp);
free (atnum);
atnum=NULL;
  pdbxcent = (pdbxmax + pdbxmin) / 2.;
  pdbycent = (pdbymax + pdbymin) / 2.;
  pdbzcent = (pdbzmax + pdbzmin) / 2.;

  for (i = 0; i <= pdbn; i++)
    {
      pdbx[i] = pdbx[i] - pdbxcent;
      pdby[i] = pdby[i] - pdbycent;
      pdbz[i] = pdbz[i] - pdbzcent;
    }


  if (connect == 0)
    {				/* if no bonds exist, i.e. there were no CONECTs */
      for (i = 0; i <= pdbn; i++)
	{
	  for (j = i + 1; j <= pdbn; j++)
	    {
	      dist = (pdbx[i] - pdbx[j]) * (pdbx[i] - pdbx[j]) +
		(pdby[i] - pdby[j]) * (pdby[i] - pdby[j]) +
		(pdbz[i] - pdbz[j]) * (pdbz[i] - pdbz[j]);
	      if (dist < 1.58 * 1.58)
		{
		  nbonds++;
		  bondfrom = realloc (bondfrom, (nbonds + 1) * sizeof (int));
		  bondto = realloc (bondto, (nbonds + 1) * sizeof (int));
		  bondtype =
		    realloc (bondtype, (nbonds + 1) * sizeof (short));
		  bondfrom[nbonds] = i;
		  bondto[nbonds] = j;
		  bondtype[nbonds] = 0;
		  if (dist < 1.45 * 1.45)
		    {
		      if (!strncmp (atcode[i], "C", 1)
			  && !strncmp (atcode[j], "C", 1))
			bondtype[nbonds] = 4;
		    }
/*		  Drawline ((int) (pdbx[i] * pdbfactor + pdboffset),
			    (int) (pdby[i] * pdbfactor + pdboffset),
			    (int) (pdbx[j] * pdbfactor + pdboffset),
			    (int) (pdby[j] * pdbfactor + pdboffset), 1);*/
		}
	    }
	}
    }

/*  CopyPlane ();*/
  importflag = 1;
  importfactor = pdbfactor;
  importoffset = pdboffset;
  pdbrotate(0,0,0);
  return (0);
}


void
pdbrotate (int nx, int ny, int nz)
{

  double l, r, s, t, u, v, w, a, b, c, d, e, f, g, h, i;
  int ii, j;
  double x[3], xx, yy, zz;
  int dx, dy;
  double ang = 0.;
  GdkRectangle update_rect;

  if (nz != 2)
    {
      dy = nx - hp->x;
      dx = ny - hp->y;
      hp->x = nx;
      hp->y = ny;
      x[0] = (dx != 0 ? 1. : 0.);
      x[1] = (dy != 0 ? 1. : 0.);
      x[2] = (nz != 0 ? 1. : 0.);

      if (abs (dx) < abs (dy))
	{
	  dx = dy;
	  x[0] = 0.;
	  x[1] = 1.;
	}
      else
	{
	  x[0] = 1.;
	  x[1] = 0.;
	}
      ang = (float) dx / 2.;
      if (ang < -360.)
	ang = ang + 360.;
      if (ang > 360.)
	ang = ang - 360.;

      if (x[2] == 1)
	{
	  x[0] = 0.;
	  x[1] = 0.;
	}
/*fprintf(stderr,"rotate %f %f %f : %f\n",x[0],x[1],x[2],ang);*/

      l = sqrt (x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);
      r = x[0] / l;
      s = x[1] / l;
      t = x[2] / l;
      v = sin (ang * M_PI / 180.);
      u = cos (ang * M_PI / 180.);
      w = 1.0 - u;
      a = u + w * r * r;
      b = t * v + w * r * s;
      c = -s * v + w * r * t;
      d = -t * v + w * s * r;
      e = u + w * s * s;
      f = r * v + w * s * t;
      g = s * v + w * t * r;
      h = -r * v + w * t * s;
      i = u + w * t * t;
      for (j = 0; j <= pdbn; j++)
	{
	  xx = pdbx[j];
	  yy = pdby[j];
	  zz = pdbz[j];
	  pdbx[j] = a * xx + b * yy + c * zz;
	  pdby[j] = d * xx + e * yy + f * zz;
	  pdbz[j] = g * xx + h * yy + i * zz;
	}
    }
  FreePix ();
  for (ii = 0; ii <= nbonds; ii++)
    Drawline ((int) (pdbx[bondfrom[ii]] * importfactor + importoffset),
	      (int) (pdby[bondfrom[ii]] * importfactor + importoffset),
	      (int) (pdbx[bondto[ii]] * importfactor + importoffset),
	      (int) (pdby[bondto[ii]] * importfactor + importoffset), 1, 0);
  gdk_draw_pixmap (drawing_area->window,
		   drawing_area->style->
		   fg_gc[GTK_WIDGET_STATE (drawing_area)], picture, 0, 0, 0,
		   0, (gint)drawing_area->allocation.width,
		   (gint)drawing_area->allocation.height);
  update_rect.x = 0;
  update_rect.y = 0;
  update_rect.width = drawing_area->allocation.width;
  update_rect.height = drawing_area->allocation.height;
  gtk_widget_draw ((GtkWidget *) drawing_area, &update_rect);
}

void
pdbstore ()
{
  int i, j;
  char tmpstr[10];

  for (i = 0; i <= pdbn; i++)
    {
/*
      pdbx[i] = (pdbx[i] + pdbxcent) * importfactor + importoffset;
      pdby[i] = (pdby[i] + pdbycent) * importfactor + importoffset;
      pdbz[i] = (pdbz[i] + pdbzcent) * importfactor + importoffset;
*/
      pdbx[i] = pdbx[i] * importfactor + importoffset;
      pdby[i] = pdby[i] * importfactor + importoffset;
      pdbz[i] = pdbz[i] * importfactor + importoffset;

      if (importflag == 1)
	{
	  switch (pdbmode)
	    {
	    case 0:		/* add all labels */
	      add_char ((int) (pdbx[i]), (int) (pdby[i]), atcode[i],
			atjust[i], 0, 0, 0, zoom_factor);
	      break;
	    case 1:		/* add all non-H labels */
	      if (strncmp (atcode[i], "H", 1))
		add_char ((int) (pdbx[i]), (int) (pdby[i]), atcode[i],
			  atjust[i], 0, 0, 0, zoom_factor);
	      break;
	    case 3:		/* only non-H without numeric identifiers */
	      if (!strncmp (atcode[i], "H", 1))
		break;
	      /* fall through to case 2 otherwise */
	    case 2:		/* all labels, without the numeric part */
	      j = (int) strcspn (atcode[i], "1234567890");
	      strncpy (tmpstr, atcode[i], (size_t)j);
	      tmpstr[j] = '\0';
	      add_char ((int) (pdbx[i]), (int) (pdby[i]), tmpstr, atjust[i],
			0, 0, 0, zoom_factor);
	      break;
	    case 4:		/* no labels */
	    default:
	      break;
	    }
	}
      else
	{			/* in MDL import, add all non-C labels */
	  if (strncmp (atcode[i], "C", 1))
	    add_char ((int) (pdbx[i]), (int) (pdby[i]), atcode[i], atjust[i],
		      0,0,0, zoom_factor);
	}
    }
  for (i = 0; i <= nbonds; i++)
    add_struct ((int) (pdbx[bondfrom[i]]),
		(int) (pdby[bondfrom[i]]),
		(int) (pdbx[bondto[i]]),
		(int) (pdby[bondto[i]]), (int) bondtype[i], 0, 0, 0, 0);
  free (pdbx);
  free (pdby);
  free (pdbz);
  free (bondfrom);
  free (bondto);
  free (bondtype);
  free (atjust);
  for (i = 0; i <= pdbn; i++)
    free (atcode[i]);
  free (atcode);
  pdbx = NULL;
  pdby = NULL;
  pdbz = NULL;
  bondfrom = NULL;
  bondto = NULL;
  bondtype = NULL;
  atjust = NULL;
  atcode = NULL;
  importflag = 0;
  pdbn = 0;
  FreePix ();
  Display_Mol ();
}

int
import_mdl_mol (char *filename, int skip)
/* imports a MDL Molfile structure */
{
/*  int x, y, tx, ty;*/ /* the coordinates */
/*  int bond = 0; */ /* Chemtool bondstyle */
  float mdlfactor = 60.0;	/* conversion factor .cht <-> .mol */
  int mdloffset = 200;
  int text_direct = 0;		/* Chemtool text direction */
  int d, e;			/* dummy variables */
  int v3=0;
  char line[255];
  char prop[41],*cfg;
  double pdbxmin = 100000., pdbxmax = -100000., pdbymin = 1000000., pdbymax =
    -100000., pdbzmin = 100000., pdbzmax = -100000.;

  /* the counts line, see ctfile.pdf for more information */
  int a;			/* number of atoms */
  int b;			/* number of bonds */
  /* not parsed right now: */
  int l;			/* number of atoms list */
  int f;			/* obsolete */
  int c;			/* chiral flag, 0=not chiral, 1=chiral */
  int s;			/* number of stext entries */
  int x_;			/* number of reaction components + 1 */
  int r;			/* number of reactants */
  int p;			/* number of products */
  int i;			/* number of intermediates */
  int m;			/* number of additional properties, 
				   no longer supported and set to 999 */
  char v[6];			/* ctab version, 'v2000' or 'v3000' */
  /* end of counts line */

  /* the atom block */

/*float xxx, yyy, zzz; */ /* the coordinates of the atom */
  char aaa[3];			/* the atomic symbol */
  /* not parsed right now: */
  int dd;			/* mass difference for isotopes */
  int ccc;			/* charge */
  int sss;			/* atom stereo parity */
  int hhh;			/* hydrogen count+1 */
  int bbb;			/* stereo care box */
  int vvv;			/* valence */
  int HHH;			/* H designator, redundant */
  int rrr;			/* reaction component type */
  int iii;			/* reaction component number */
  int mmm;			/* atom-atom mapping number */
  int nnn;			/* inversion/retention flag */
  int eee;			/* exact change flag */
  /* end atom block */

  /* the bond block */
  int atom1;			/* first atom number */
  int atom2;			/* second atom number */
  int tt;			/* bond type */
  int ss;			/* bond stereo */
  int xx;			/* not used */
  int rr;			/* bond topology, 0=eighter, 1=ring, 2=chain */
  int cc;			/* reaction center status */
  /* end bond block */


  char label[20];		/* a label defined in the .mol-file */
  char aa[4],bb[4];
  int res;
  FILE *fp;
  /*struct dc *hp_a; */

#ifdef GTK2
  setlocale (LC_NUMERIC,"C");
#endif
  
  if ((fp = fopen (filename, "r")) == NULL)
    return (1);

  if (skip >0 ) {
     d = 0;
     while ( d <skip) {
     if (! fgets(line, (int)sizeof(line),fp)) {
        fclose(fp);
        return(1);
     }
     if (!strncmp(line,"$$$$",4)) d++;
     }
  }   
     
     /* skip the first three lines (header) */
  for (d = 0; d < 3; d++)
    {
      if (!fgets (line, (int)sizeof (line), fp)) {
        fclose(fp);
        return(1);
      }  
    }

  /* parse the counts-line */
  if (!fgets (line, (int)sizeof (line), fp)) {
    fclose(fp);
    return(1);
  }  
  if (strstr(line,"V3000")) v3=1;

  if(v3==1) {
    if (!fgets(line,(int)sizeof(line),fp)) {
      fclose(fp);
      return(1);
    }  
    if (strncmp(line,"M V30 BEGIN CTAB",16)) {
      fclose(fp);
      return(1);
    }  
    if (!fgets(line,(int)sizeof(line),fp)) {
      fclose(fp);
      return(1);
    }  
    d = sscanf (line,"M V30 COUNTS %d %d",&a,&b);
    if (!fgets(line,(int)sizeof(line),fp)) {
      fclose(fp);
      return(1);
    }  
    if (strncmp(line,"M V30 BEGIN ATOM",16)) {
      fclose(fp);
      return(1);
    }  
  } else {  
    d = sscanf (line, "%c%c%c%c%c%c %i %i %i %i %i %i %i %i %i %s",
	      &aa[0],&aa[1],&aa[2],&bb[0],&bb[1],&bb[2], &l, &f, &c, &s, &x_, &r, &p, &i, &m, v);
    aa[3]='\0';
    bb[3]='\0';
    sscanf(aa,"%i",&a);
    sscanf(bb,"%i",&b);
    if (d == 0 || a <= 0 || b < 0 || a > 100000 || b > 100000) {
      fclose(fp);
      return (1);
    }
  }
  clear_data ();
  pdbn = -1;
  /* read in atom block data */
  for (d = 0; d < a; d++)
    {
      if (!fgets (line, (int)sizeof (line), fp)) {
        fclose(fp);
        return(1);
      }  
      pdbn++;
      pdbx = realloc (pdbx, (pdbn + 1) * sizeof (double));
      pdby = realloc (pdby, (pdbn + 1) * sizeof (double));
      pdbz = realloc (pdbz, (pdbn + 1) * sizeof (double));
      atjust = realloc (atjust, (pdbn + 1) * sizeof (short));
      atcode = realloc (atcode, (pdbn + 1) * sizeof (char *));
      atcode[pdbn] = malloc (9 * sizeof (char));
      if (v3==0) 
        res = sscanf (line, "%lf %lf %lf %s %i %i %i %i %i %i %i %i %i %i %i %i",
	      &pdbx[pdbn], &pdby[pdbn], &pdbz[pdbn], atcode[pdbn], &dd, &ccc,
	      &sss, &hhh, &bbb, &vvv, &HHH, &rrr, &iii, &mmm, &nnn, &eee);
      else
        res = sscanf (line, "M V30 %*d %s %lf %lf %lf",
                atcode[pdbn],&pdbx[pdbn], &pdby[pdbn], &pdbz[pdbn]);
      if (res <3 ) {
        fclose(fp);
        return(1);
      }  
      pdbxmin = MIN (pdbxmin, pdbx[d]);
      pdbxmax = MAX (pdbxmax, pdbx[d]);

      pdbymin = MIN (pdbymin, pdby[d]);
      pdbymax = MAX (pdbymax, pdby[d]);

      pdbzmin = MIN (pdbzmin, pdbz[d]);
      pdbzmax = MAX (pdbzmax, pdbz[d]);

      atjust[pdbn] = -1;	/*default to centered labels */
    }
  if (v3==1) {
    if (!fgets(line,(int)sizeof(line),fp)) {
      fclose(fp);
      return(1);
    }  
    if (strncmp(line,"M V30 END ATOM",14)) {
      fclose(fp);
      return(1);
    }
    if (!fgets(line,(int)sizeof(line),fp)) {
      fclose(fp);
      return(1);
    }
    if (strncmp(line,"M V30 BEGIN BOND",16)) {
      fclose(fp);
      return(1);
    }
  }  
     
  nbonds = -1;
  /* read in bond block data */
  for (d = 0; d < b; d++)
    {
      if (!fgets (line, (int)sizeof (line), fp)) {
        fclose(fp);
        return(1);
      }
      if (v3==1) {
        memset(prop,0,40);
        res = sscanf (line, "M V30 %*d %i %i %i %40c",
                       &tt,&atom1,&atom2,prop);
        if (res <2) {
          fclose(fp);
          return(1);
        }
        ss = 0;
        if ( (cfg=strstr(prop,"CFG=")) != NULL) {
          res = sscanf(cfg,"CFG=%d",&ss);
          if (ss == 3) ss = 6;
        }
      }else{        
      res = sscanf (line, "%c%c%c%c%c%c %i %i %i %i %i",
	      &aa[0],&aa[1],&aa[2], &bb[0], &bb[1], &bb[2], &tt, &ss, &xx, &rr, &cc);
	res = sscanf(aa,"%i",&atom1);
	res = sscanf(bb,"%i",&atom2);
      }
      nbonds++;
      bondfrom = realloc (bondfrom, (nbonds + 1) * sizeof (int));
      bondto = realloc (bondto, (nbonds + 1) * sizeof (int));
      bondtype = realloc (bondtype, (nbonds + 1) * sizeof (short));
      bondfrom[nbonds] = atom1 - 1;
      bondto[nbonds] = atom2 - 1;
      switch (tt)
	{
	case 1:   /* single */
	  bondtype[nbonds] = 0;
	  if (ss == 1)  /* stereo up */
	    bondtype[nbonds] = 5;
	  if (ss == 2 || ss == 4) /* either */
	    bondtype[nbonds] = 7;
	  if (ss == 6)           /* down */
	    bondtype[nbonds] = 6;
	  break;
	case 2:   /* double */
	  bondtype[nbonds] = 1;
	  break;
	case 3:
	  bondtype[nbonds] = 3;
	  break;
	default:
	  bondtype[nbonds] = 0;
	}


    }
  if (v3 == 1) {
      if (!fgets (line, (int)sizeof (line), fp)) {
        fclose(fp);
        return(1);
      }  
  } else {
  /* check for additional labels */
  res = fscanf (fp, "%s %s", aaa, line);
  while (res >0 && !strcmp (aaa, "A"))
    {
      atom1 = atoi (line);
      res = fscanf (fp, "%s", label);
      if (res == 0) break;
      /* found a label "label" at atom "atom1" */
      /* check for numbers, interpret as indices: */
      text_direct = 0;
      f = (int)strlen (label);
      if (f == 1)
	text_direct = -1;	/*center label if only one character */
      for (e = 0; label[e] != '\0'; e++)
	{
	  if (isdigit (label[e]))
	    {
	      /* If there's a number at the beginning, assume right-
	       * justified text: */
	      if (e == 0)
		text_direct = -2;
	      /* copy already processed string in auxiliary string 'line': */
	      strncpy (line, label, (size_t)e);
	      /* append a '_', the number and a NULL-string: */
	      line[e] = '_';
	      line[e + 1] = label[e];
	      line[e + 2] = '\0';
	      /* append the rest of the label-string to 'line': */
	      strcat (line, strrchr (label, label[e + 1]));
	      /* copy 'line' back to 'label' and add a NULL-string at the end: */
	      strcpy (label, line);
	      e++;
	      label[++f] = '\0';
	    }
	}
      strcpy (atcode[atom1 - 1], label);

      atjust[atom1 - 1] = (short)text_direct;

      res = fscanf (fp, "%s %s", aaa, line);
    }
  }
  /* check last line for "M  END" */
  if (strcmp(line,"END") && strcmp(line,"$$$$") ) {
 do   {
      if (!fgets (line,80,fp)){
        fprintf(stderr,"EOF\n");
        break;
        }
    } while (strncmp (line, "M  END",6) && strncmp(line,"$$$$",4) );
  if (strncmp (line, "M  END",6) && strncmp (line,"$$$$",4) )
    {
fprintf(stderr,"endless molfile???: %s\n",line);
      clear_data ();
      fclose (fp);
      return (2);
    }
  }
  fclose (fp);
  pdbxcent = (pdbxmax + pdbxmin) / 2.;
  pdbycent = (pdbymax + pdbymin) / 2.;
  pdbzcent = (pdbzmax + pdbzmin) / 2.;

  for (i = 0; i <= pdbn; i++)
    {
      pdbx[i] = pdbx[i] - pdbxcent;
      pdby[i] = pdby[i] - 2. * (pdby[i] - pdbycent);
      pdbz[i] = pdbz[i] - pdbzcent;
    }
  /* try to normalize bond lengths based on first bond */
  mdlfactor =
    mdlfactor / sqrt ((pdbx[bondfrom[0]] - pdbx[bondto[0]]) *
		      (pdbx[bondfrom[0]] - pdbx[bondto[0]]) +
		      (pdby[bondfrom[0]] -
		       pdby[bondto[0]]) * (pdby[bondfrom[0]] -
					   pdby[bondto[0]]) +
		      (pdbz[bondfrom[0]] -
		       pdbz[bondto[0]]) * (pdbz[bondfrom[0]] -
					   pdbz[bondto[0]]));

  for (i = 0; i <= nbonds; i++)
    Drawline ((int) (pdbx[bondfrom[i]] * mdlfactor + mdloffset),
	      (int) (pdby[bondfrom[i]] * mdlfactor + mdloffset),
	      (int) (pdbx[bondto[i]] * mdlfactor + mdloffset),
	      (int) (pdby[bondto[i]] * mdlfactor + mdloffset), 1, 0);
  CopyPlane ();
  importflag = 2;
  importfactor = mdlfactor;
  importoffset = mdloffset;
  return (0);
}

int
preview_mdl_mol (char *filename, int skip)
/* imports a MDL Molfile structure  into the preview widget*/
{
/*  int x, y, tx, ty;*/ /* the coordinates */
/*  int bond = 0; */ /* Chemtool bondstyle */
  float mdlfactor = 60.0;	/* conversion factor .cht <-> .mol */
  float previewscale;
  int text_direct = 0;		/* Chemtool text direction */
  int d, e;			/* dummy variables */
  GdkRectangle update_rect;
  int v3=0;
  char prop[41],*cfg;
  char line[255];
  double pdbxmin = 100000., pdbxmax = -100000., pdbymin = 1000000., pdbymax =
    -100000.;

  /* the counts line, see ctfile.pdf for more information */
  int a;			/* number of atoms */
  int b;			/* number of bonds */
  /* not parsed right now: */
  int l;			/* number of atoms list */
  int f;			/* obsolete */
  int c;			/* chiral flag, 0=not chiral, 1=chiral */
  int s;			/* number of stext entries */
  int x_;			/* number of reaction components + 1 */
  int r;			/* number of reactants */
  int p;			/* number of products */
  int i;			/* number of intermediates */
  int m;			/* number of additional properties, 
				   no longer supported and set to 999 */
  char v[6];			/* ctab version, 'v2000' or 'v3000' */
  /* end of counts line */

  /* the atom block */

/*float xxx, yyy, zzz; */ /* the coordinates of the atom */
  char aaa[3];			/* the atomic symbol */
  /* not parsed right now: */
  int dd;			/* mass difference for isotopes */
  int ccc;			/* charge */
  int sss;			/* atom stereo parity */
  int hhh;			/* hydrogen count+1 */
  int bbb;			/* stereo care box */
  int vvv;			/* valence */
  int HHH;			/* H designator, redundant */
  int rrr;			/* reaction component type */
  int iii;			/* reaction component number */
  int mmm;			/* atom-atom mapping number */
  int nnn;			/* inversion/retention flag */
  int eee;			/* exact change flag */
  /* end atom block */

  /* the bond block */
  int atom1;			/* first atom number */
  int atom2;			/* second atom number */
  int tt;			/* bond type */
  int ss;			/* bond stereo */
  int xx;			/* not used */
  int rr;			/* bond topology, 0=eighter, 1=ring, 2=chain */
  int cc;			/* reaction center status */
  /* end bond block */


  char label[20];		/* a label defined in the .mol-file */
  char aa[4],bb[4];
  int res;
  FILE *fp;
  /*struct dc *hp_a; */

#ifdef GTK2
  setlocale (LC_NUMERIC,"C");
#endif
  
  if ((fp = fopen (filename, "r")) == NULL)
    return (1);


  if (skip >0 ) {
     d = 0;
     while ( d <skip) {
     if (! fgets(line, (int)sizeof(line),fp)) {
     sdfindex--;
     fclose(fp);
     return(1);
     }
     if (!strncmp(line,"$$$$",4)) d++;
     }
  }   
     
     /* skip the first three lines (header) */
  for (d = 0; d < 3; d++)
    {
       if (!fgets (line, (int)sizeof (line), fp)) {
         fclose(fp);
         return(1);
       }
    }

  /* parse the counts-line */
  if (!fgets (line, (int)sizeof (line), fp)){
    fclose(fp);
    return(1);
  }
  if (strstr(line,"V3000")) v3=1;

  if(v3==1) {
    if(!fgets(line,(int)sizeof(line),fp)) {
      fclose(fp);
      return(1);
    }
    if (strncmp(line,"M V30 BEGIN CTAB",16)) {
      fclose(fp);
      return(1);
    }
    if (!fgets(line,(int)sizeof(line),fp)) {
      fclose(fp);
      return(1);
    }
    d = sscanf (line,"M V30 COUNTS %d %d",&a,&b);
    if(!fgets(line,(int)sizeof(line),fp)) {
      fclose(fp);
      return(1);
    }
    if (strncmp(line,"M V30 BEGIN ATOM",16)) {
      fclose(fp);
      return(1);
    }  
  } else {  
    d = sscanf (line, "%c%c%c%c%c%c %i %i %i %i %i %i %i %i %i %s",
                &aa[0],&aa[1],&aa[2],&bb[0],&bb[1],&bb[2], &l, &f, &c, &s, &x_, &r, &p, &i, &m, v);
    aa[3]='\0';
    bb[3]='\0';
    sscanf(aa,"%i",&a);
    sscanf(bb,"%i",&b);
    if (d == 0 || a <= 0 || b < 0 || a > 100000 || b > 100000) {
      fclose(fp);
      return (1);
    }  
  }
  clear_data ();
  pdbn = -1;
  /* read in atom block data */
  for (d = 0; d < a; d++)
    {
      if (!fgets (line, (int)sizeof (line), fp)){
        fclose(fp);
        return(1);
      }
      pdbn++;
      pdbx = realloc (pdbx, (pdbn + 1) * sizeof (double));
      pdby = realloc (pdby, (pdbn + 1) * sizeof (double));
      atjust = realloc (atjust, (pdbn + 1) * sizeof (short));
      atcode = realloc (atcode, (pdbn + 1) * sizeof (char *));
      atcode[pdbn] = malloc (9 * sizeof (char));
      if (v3==0) 
        res = sscanf (line, "%lf %lf %*f %s %i %i %i %i %i %i %i %i %i %i %i %i",
	      &pdbx[pdbn], &pdby[pdbn], atcode[pdbn], &dd, &ccc,
	      &sss, &hhh, &bbb, &vvv, &HHH, &rrr, &iii, &mmm, &nnn, &eee);
      else
        res= sscanf (line, "M V30 %*d %s %lf %lf",
                atcode[pdbn],&pdbx[pdbn], &pdby[pdbn]);

      if (res <3) {
        fclose(fp);
        return(1);
      }
      atjust[pdbn] = -1;	/*default to centered labels */
    }

  if (v3==1) {
    if (!fgets(line,(int)sizeof(line),fp)) {
      fclose(fp);
      return(1);
    }
    if (strncmp(line,"M V30 END ATOM",14)) {
      fclose(fp);
      return(1);
    }
    if (!fgets(line,(int)sizeof(line),fp)) {
      fclose(fp);
      return(1);
    }
    if (strncmp(line,"M V30 BEGIN BOND",16)) {
      fclose(fp);
      return(1);
    }  
  }  

  nbonds = -1;
  /* read in bond block data */
  for (d = 0; d < b; d++)
    {
      if (!fgets (line, (int)sizeof (line), fp)) {
        fclose(fp);
        return(1);
      }
      if (v3==1) {
        memset(prop,0,40);
        res = sscanf (line, "M V30 %*d %i %i %i %40c",
                       &tt,&atom1,&atom2,prop);
        ss = 0;
        if ( (cfg=strstr(prop,"CFG=")) != NULL) {
          res = sscanf(cfg,"CFG=%d",&ss);
          if (ss == 3) ss = 6;
        }
      }else{        
      res = sscanf (line, "%c%c%c%c%c%c %i %i %i %i %i",
	      &aa[0],&aa[1],&aa[2], &bb[0], &bb[1], &bb[2], &tt, &ss, &xx, &rr, &cc);
	res = sscanf(aa,"%i",&atom1);
	res = sscanf(bb,"%i",&atom2);
      }
      nbonds++;
      bondfrom = realloc (bondfrom, (nbonds + 1) * sizeof (int));
      bondto = realloc (bondto, (nbonds + 1) * sizeof (int));
      bondtype = realloc (bondtype, (nbonds + 1) * sizeof (short));
      bondfrom[nbonds] = atom1 - 1;
      bondto[nbonds] = atom2 - 1;
      switch (tt)
	{
	case 1:   /* single */
	  bondtype[nbonds] = 0;
	  if (ss == 1)  /* stereo up */
	    bondtype[nbonds] = 5;
	  if (ss == 2 || ss == 4) /* either */
	    bondtype[nbonds] = 7;
	  if (ss == 6)           /* down */
	    bondtype[nbonds] = 6;
	  break;
	case 2:
	  bondtype[nbonds] = 1;
	  break;
	case 3:
	  bondtype[nbonds] = 3;
	  break;
	default:
	  bondtype[nbonds] = 0;
	}


    }

  if (v3 == 1) {
      if (!fgets (line, (int)sizeof (line), fp)) {
        fclose(fp);
        return(1);
      }
  } else {
  /* check for additional labels */
  res = fscanf (fp, "%s %s", aaa, line);
  while (res >0 && !strcmp (aaa, "A"))
    {
      atom1 = atoi (line);
      res = fscanf (fp, "%s", label);
      if (res == 0) break;
      /* found a label "label" at atom "atom1" */
      /* check for numbers, interpret as indices: */
      text_direct = 0;
      f = (int)strlen (label);
      if (f == 1)
	text_direct = -1;	/*center label if only one character */
      for (e = 0; label[e] != '\0'; e++)
	{
	  if (isdigit (label[e]))
	    {
	      /* If there's a number at the beginning, assume right-
	       * justified text: */
	      if (e == 0)
		text_direct = -2;
	      /* copy already processed string in auxiliary string 'line': */
	      strncpy (line, label, (size_t)e);
	      /* append a '_', the number and a NULL-string: */
	      line[e] = '_';
	      line[e + 1] = label[e];
	      line[e + 2] = '\0';
	      /* append the rest of the label-string to 'line': */
	      strcat (line, strrchr (label, label[e + 1]));
	      /* copy 'line' back to 'label' and add a NULL-string at the end: */
	      strcpy (label, line);
	      e++;
	      label[++f] = '\0';
	    }
	}
      strcpy (atcode[atom1 - 1], label);

      atjust[atom1 - 1] = (short)text_direct;

      res = fscanf (fp, "%s %s", aaa, line);
    }
  }
  /* check last line for "M  END" */
  if (strcmp(line,"END") && strcmp(line,"$$$$") ) {
 do   {
      if (!fgets (line,80,fp)){
        fprintf(stderr,"EOF\n");
        break;
        }
    } while (strncmp (line, "M  END",6) && strncmp(line,"$$$$",4) );
  if (strncmp (line, "M  END",6) && strncmp (line,"$$$$",4) )
    {
fprintf(stderr,"endless molfile???: %s\n",line);
      clear_data ();
      fclose (fp);
      return (2);
    }
  }
  fclose (fp);

  /* try to normalize bond lengths based on first bond */
  mdlfactor =
    mdlfactor / sqrt ((pdbx[bondfrom[0]] - pdbx[bondto[0]]) *
		      (pdbx[bondfrom[0]] - pdbx[bondto[0]]) +
		      (pdby[bondfrom[0]] -
		       pdby[bondto[0]]) * (pdby[bondfrom[0]] -
					   pdby[bondto[0]]) );

  pdbxmin=10000.;
  pdbymin=10000.;
  pdbxmax=-10000.;
  pdbymax=-10000.;
        
  for (i = 0; i <= pdbn; i++) {
      pdbx[i] = pdbx[i]*mdlfactor;
      pdby[i] = pdby[i]*mdlfactor;
      
      pdbxmin = MIN (pdbxmin, pdbx[i]);
      pdbxmax = MAX (pdbxmax, pdbx[i]);
      pdbymin = MIN (pdbymin, pdby[i]);
      pdbymax = MAX (pdbymax, pdby[i]);
  }
 
/* center molecule and scale it to fit into the effective
   drawing area (300x150 plus small border) */
  
  pdbxcent = (pdbxmax + pdbxmin) / 2.;
  pdbycent = (pdbymax + pdbymin) / 2.;

  previewscale = MIN(280./(pdbxmax-pdbxmin),140./(pdbymax-pdbymin));
  
  for (i = 0; i <= pdbn; i++) {
     pdbx[i] = (pdbx[i]- pdbxcent)*previewscale+140.;
     pdby[i] = (pdby[i]- 2. * pdby[i] + pdbycent)*previewscale+75.;
  }
                        
  for (i = 0; i <= nbonds; i++)
    draw_preview_bonds ((int) pdbx[bondfrom[i]],
	      (int) pdby[bondfrom[i]],
	      (int) pdbx[bondto[i]],
	      (int) pdby[bondto[i]], 0);

  for (i = 0; i <= pdbn; i++)
     if (strcmp(atcode[i],"C") && strcmp(atcode[i],"H"))
       Drawstring ((int)pdbx[i], (int)pdby[i], atcode[i], atjust[i], 0,0,0,1, 1);
                         
  gdk_draw_pixmap (preview_area->window,
		   drawing_area->style->
		   fg_gc[GTK_WIDGET_STATE (drawing_area)], picture, 0, 0, 0,
		   0, 200, 100);
  update_rect.x = 0;
  update_rect.y = 0;
  update_rect.width = 200;
  update_rect.height = 100;
  gtk_widget_draw ((GtkWidget *) preview_area, &update_rect);
  return (0);
}

int
import_babel (char *filename)
/* imports a foreign file via BABEL as a MDL Molfile structure */
{
/*  int x, y, tx, ty;*/ /* the coordinates */
/*  int bond = 0; */ /* Chemtool bondstyle */
  float mdlfactor = 60.0;	/* conversion factor .cht <-> .mol */
  int mdloffset = 200;
  int text_direct = 0;		/* Chemtool text direction */
  int d, e;			/* dummy variables */
  char line[255];
  double pdbxmin = 100000., pdbxmax = -100000., pdbymin = 1000000., pdbymax =
    -100000., pdbzmin = 100000., pdbzmax = -100000.;

  /* the counts line, see ctfile.pdf for more information */
  int a;			/* number of atoms */
  int b;			/* number of bonds */
  /* not parsed right now: */
  int l;			/* number of atoms list */
  int f;			/* obsolete */
  int c;			/* chiral flag, 0=not chiral, 1=chiral */
  int s;			/* number of stext entries */
  int x_;			/* number of reaction components + 1 */
  int r;			/* number of reactants */
  int p;			/* number of products */
  int i;			/* number of intermediates */
  int m;			/* number of additional properties, 
				   no longer supported and set to 999 */
  char v[6];			/* ctab version, 'v2000' or 'v3000' */
  /* end of counts line */

  /* the atom block */

/*float xxx, yyy, zzz; */ /* the coordinates of the atom */
  char aaa[3];			/* the atomic symbol */
  /* not parsed right now: */
  int dd;			/* mass difference for isotopes */
  int ccc;			/* charge */
  int sss;			/* atom stereo parity */
  int hhh;			/* hydrogen count+1 */
  int bbb;			/* stereo care box */
  int vvv;			/* valence */
  int HHH;			/* H designator, redundant */
  int rrr;			/* reaction component type */
  int iii;			/* reaction component number */
  int mmm;			/* atom-atom mapping number */
  int nnn;			/* inversion/retention flag */
  int eee;			/* exact change flag */
  /* end atom block */

  /* the bond block */
  int atom1;			/* first atom number */
  int atom2;			/* second atom number */
  int tt;			/* bond type */
  int ss;			/* bond stereo */
  int xx;			/* not used */
  int rr;			/* bond topology, 0=eighter, 1=ring, 2=chain */
  int cc;			/* reaction center status */
  /* end bond block */

  int res;
  char label[20];		/* a label defined in the .mol-file */
  char aa[4],bb[4];
  FILE *fp;
  /*struct dc *hp_a; */

#ifdef GTK2
  setlocale(LC_NUMERIC,"C");
#endif
  
  if (!filename || !babel ) return (1); /* no filename or no input mode */
 
  snprintf(line,255,"babel -i%s %s -omdl %s",babel,filename,babeloutp);
/*  fprintf(stderr,"%s\n",line);*/
/*@ignore@ splint does not recognize popen */
   if ((fp = popen (line, "r")) == NULL)
/*@end@*/
    return (1);

  /* skip the first three lines (header) */
  for (d = 0; d < 3; d++)
    {
      if (!fgets (line, (int)sizeof (line), fp)) {
/*@ignore@*/
      pclose(fp);
/*@end@*/
      return(1);
      }
    }

  /* parse the counts-line */
  if (!fgets (line, (int)sizeof (line), fp)) {
/*@ignore@*/
      pclose(fp);
/*@end@*/
      return(1);
      }
  d = sscanf (line, "%c%c%c%c%c%c %i %i %i %i %i %i %i %i %i %s",
	      &aa[0],&aa[1],&aa[2], &bb[0], &bb[1], &bb[2], &l, &f, &c, &s, &x_, &r, &p, &i, &m, v);
  aa[3]=bb[3]='\0';
  sscanf(aa,"%i",&a);
  sscanf(bb,"%i",&b);
  if (d == 0 || a <= 0 || b < 0 || a > 100000 || b > 100000) {
/*@ignore@*/
    pclose(fp);
/*@end@*/
    return (1);
  }  
fprintf(stderr, "expecting na %d nb %d\n",a,b);
  clear_data ();
  pdbn = -1;
  /* read in atom block data */
  for (d = 0; d < a; d++)
    {
      if (!fgets (line, (int)sizeof (line), fp)) {
/*@ignore@*/
      pclose(fp);
/*@end@*/
      return(1);
      }
      pdbn++;
      pdbx = realloc (pdbx, (pdbn + 1) * sizeof (double));
      pdby = realloc (pdby, (pdbn + 1) * sizeof (double));
      pdbz = realloc (pdbz, (pdbn + 1) * sizeof (double));
      atjust = realloc (atjust, (pdbn + 1) * sizeof (short));
      atcode = realloc (atcode, (pdbn + 1) * sizeof (char *));
      atcode[pdbn] = malloc (9 * sizeof (char));
      res = sscanf (line, "%lf %lf %lf %s %i %i %i %i %i %i %i %i %i %i %i %i",
	      &pdbx[pdbn], &pdby[pdbn], &pdbz[pdbn], atcode[pdbn], &dd, &ccc,
	      &sss, &hhh, &bbb, &vvv, &HHH, &rrr, &iii, &mmm, &nnn, &eee);
      if (res <3) {
/*@ignore@*/
      pclose(fp);
/*@end@*/
      return(1);
      }

      pdbxmin = MIN (pdbxmin, pdbx[d]);
      pdbxmax = MAX (pdbxmax, pdbx[d]);

      pdbymin = MIN (pdbymin, pdby[d]);
      pdbymax = MAX (pdbymax, pdby[d]);

      pdbzmin = MIN (pdbzmin, pdbz[d]);
      pdbzmax = MAX (pdbzmax, pdbz[d]);

      atjust[pdbn] = -1;	/*default to centered labels */
    }
  nbonds = -1;
  /* read in bond block data */
  for (d = 0; d < b; d++)
    {
      if (!fgets (line, (int)sizeof (line), fp)) {
/*@ignore@*/
      pclose(fp);
/*@end@*/
      return(1);
      }

      res = sscanf (line, "%c%c%c%c%c%c %i %i %i %i %i",
	      &aa[0],&aa[1],&aa[2],&bb[0],&bb[1],&bb[2], &tt, &ss, &xx, &rr, &cc);
	res = sscanf(aa,"%i",&atom1);
	res = sscanf(bb,"%i",&atom2);
      nbonds++;
      bondfrom = realloc (bondfrom, (nbonds + 1) * sizeof (int));
      bondto = realloc (bondto, (nbonds + 1) * sizeof (int));
      bondtype = realloc (bondtype, (nbonds + 1) * sizeof (short));
      bondfrom[nbonds] = atom1 - 1;
      bondto[nbonds] = atom2 - 1;
      switch (tt)
	{
	case 1:
	  bondtype[nbonds] = 0;
	  if (ss == 1)
	    bondtype[nbonds] = 5;
	  if (ss == 6)
	    bondtype[nbonds] = 6;
	  break;
	case 2:
	  bondtype[nbonds] = 1;
	  break;
	case 3:
	  bondtype[nbonds] = 3;
	  break;
	default:
	  bondtype[nbonds] = 0;
	}


    }

  /* check for additional labels */
  res = fscanf (fp, "%s %s", aaa, line);
  while (res >0 && !strcmp (aaa, "A"))
    {
      atom1 = atoi (line);
      res = fscanf (fp, "%s", label);
      if (res <1) break;
      /* found a label "label" at atom "atom1" */
      /* check for numbers, interpret as indices: */
      text_direct = 0;
      f = (int)strlen (label);
      if (f == 1)
	text_direct = -1;	/*center label if only one character */
      for (e = 0; label[e] != '\0'; e++)
	{
	  if (isdigit (label[e]))
	    {
	      /* If there's a number at the beginning, assume right-
	       * justified text: */
	      if (e == 0)
		text_direct = -2;
	      /* copy already processed string in auxiliary string 'line': */
	      strncpy (line, label, (size_t)e);
	      /* append a '_', the number and a NULL-string: */
	      line[e] = '_';
	      line[e + 1] = label[e];
	      line[e + 2] = '\0';
	      /* append the rest of the label-string to 'line': */
	      strcat (line, strrchr (label, label[e + 1]));
	      /* copy 'line' back to 'label' and add a NULL-string at the end: */
	      strcpy (label, line);
	      e++;
	      label[++f] = '\0';
	    }
	}
      strcpy (atcode[atom1 - 1], label);
      atjust[atom1 - 1] = (short)text_direct;

      res = fscanf (fp, "%s %s", aaa, line);
    }

  /* check last line for "M  END" */
  while (strcmp (line, "END"))
    {
      if (fscanf (fp, "%s", line) == EOF)
	break;
      if (fscanf (fp, "%s %s", aaa, line) == EOF)
	break;
    }
  if (strcmp (aaa, "M") || strcmp (line, "END"))
    {
      clear_data ();
/*@ignore@ splint does not know pclose */
      pclose (fp);
/*@end@*/
      return (2);
    }

  pclose (fp);
  pdbxcent = (pdbxmax + pdbxmin) / 2.;
  pdbycent = (pdbymax + pdbymin) / 2.;
  pdbzcent = (pdbzmax + pdbzmin) / 2.;

  if (pdbn <=0) return (1); /* complain if no atoms */

  for (i = 0; i <= pdbn; i++)
    {
      pdbx[i] = pdbx[i] - pdbxcent;
      pdby[i] = pdby[i] - 2. * (pdby[i] - pdbycent);
      pdbz[i] = pdbz[i] - pdbzcent;
    }
  /* try to normalize bond lengths based on first bond */
  mdlfactor =
    mdlfactor / sqrt ((pdbx[bondfrom[0]] - pdbx[bondto[0]]) *
		      (pdbx[bondfrom[0]] - pdbx[bondto[0]]) +
		      (pdby[bondfrom[0]] -
		       pdby[bondto[0]]) * (pdby[bondfrom[0]] -
					   pdby[bondto[0]]) +
		      (pdbz[bondfrom[0]] -
		       pdbz[bondto[0]]) * (pdbz[bondfrom[0]] -
					   pdbz[bondto[0]]));

  for (i = 0; i <= nbonds; i++)
    Drawline ((int) (pdbx[bondfrom[i]] * mdlfactor + mdloffset),
	      (int) (pdby[bondfrom[i]] * mdlfactor + mdloffset),
	      (int) (pdbx[bondto[i]] * mdlfactor + mdloffset),
	      (int) (pdby[bondto[i]] * mdlfactor + mdloffset), 1, 0);
  CopyPlane ();
  importflag = 2;
  importfactor = mdlfactor;
  importoffset = mdloffset;
  return (0);
}

int
export_mdl_mol (FILE *fp, int topipe)
/* exports a MDL Molfile structure */
{
  float factor = 76.0;		/* conversion factor .cht <-> .mol */
  int d, e, f;			/* dummy variables */
  char aaa[3];			/* the atomic symbol */
  int atom1 = 0;		/* first atom number */
  int atom2 = 0;		/* second atom number */
  int tt;			/* bond type */
  int ss;			/* bond stereo */
  struct dc *hp_a;
  struct data *hp_b;
  int atoms[999][2];		/* coordinates of unique labels */
  int start_already_there;
  int end_already_there;
  int is_label;
  int labelcount;		/* number of unique atoms/labels */
  int omit;			/* number of decorative lines (boxes etc) */
  int hcount=0;			/* for explicit CH_3, CH_2, CH */
  
#ifdef GTK2
  setlocale(LC_NUMERIC,"C");
#endif

  /* FIXME: Write some sane stuff in the header */
  fprintf (fp, "Molecule exported from chemtool\n");
  fprintf (fp, "\n");
  fprintf (fp, "\n");

  labelcount = 0;
  omit = 0;
  atoms[0][0]=atoms[0][1]=0;
  /* Get coordinates of all unique bond-delimiters 
   * and, more importantly, the number of unique atoms, 
   * chemtool only knows the number of bonds and the
   * number of non-carbon atoms :( */
  hp_b = da_root.next;
  for (d = 0; d < hp->n; d++)
    {
      if (!(mark.flag && (hp_b->smarked + hp_b->tmarked) == 0))
	{
	if (hp_b->decoration == 1) omit++;
	else {
	  /* Check for already parsed atoms on same coordinates */
	  start_already_there = 0;
	  end_already_there = 0;
	  for (f = 0; f <= labelcount; f++)
	    {
	      if (hp_b->x == atoms[f][0] && hp_b->y == atoms[f][1])
		{
		  start_already_there = 1;
		}
	      if (hp_b->tx == atoms[f][0] && hp_b->ty == atoms[f][1])
		{
		  end_already_there = 1;
		}
	    }
	  if (!start_already_there)
	    {
	      atoms[labelcount][0] = hp_b->x;
	      atoms[labelcount][1] = hp_b->y;
	      labelcount++;
	    }
	  if (!end_already_there)
	    {
	      atoms[labelcount][0] = hp_b->tx;
	      atoms[labelcount][1] = hp_b->ty;
	      labelcount++;
	    }
	  }
	}
      hp_b = hp_b->next;
    }

  /* The counts line: */
  fprintf (fp, "%3i%3i  0  0  0  0  0  0  0  0999 V2000\n", labelcount,
	   hp->n-omit);

  /* Now write down the labels.
   * start all over */
  for (d = 0; d < labelcount; d++)
    {
      /* Check for label on same position */
      hp_a = dac_root.next;
      is_label = 0;
      for (e = 0; e < hp->nc; e++)
	{
	  if (atoms[d][0] == hp_a->x && atoms[d][1] == hp_a->y)
	    {
	      is_label = 1;
	      break;
	    }
	  hp_a = hp_a->next;
	}
      if (is_label)
	{
	hcount = 0;
	  if (!strcmp(hp_a->c,"CH") || !strcmp(hp_a->c,"HC") )
	    {
	      hcount=1;
	      strcpy (aaa, "C");
	    } else
	  if (!strcmp(hp_a->c,"OH") || !strcmp(hp_a->c,"HO") )
	    {
	      hcount=1;
	      strcpy (aaa, "O");
	    } else
	  if (!strcmp(hp_a->c,"NH") || !strcmp(hp_a->c,"HN") )
	    {
	      hcount=1;
	      strcpy (aaa, "N");
	    } else
	  if (!strcmp(hp_a->c,"SH") || !strcmp(hp_a->c,"HS") )
	    {
	      hcount=1;
	      strcpy (aaa, "S");
	    } else
	  if (!strcmp(hp_a->c,"PH") || !strcmp(hp_a->c,"HP") )
	    {
	      hcount=1;
	      strcpy (aaa, "P");
	    } else
	  if ((int)strlen (hp_a->c) < 4)
	    {
	      strcpy (aaa, hp_a->c);
	    }
	  else if (!strncmp(hp_a->c,"CH_3",4) || !strncmp(hp_a->c,"H_3C",4))
	    { 
	      hcount = 3;
	      strcpy (aaa, "C");
	    }
	  else if ( !strncmp(hp_a->c,"CH_2",4) || !strncmp(hp_a->c,"H2_C",4))
	    {
	      hcount = 2;
	      strcpy (aaa, "C");
	    }
	  else if (!strncmp(hp_a->c,"NH_3",4) || !strncmp(hp_a->c,"H_3N",4))
	    { 
	      hcount = 3;
	      strcpy (aaa, "N");
	    }
	  else if ( !strncmp(hp_a->c,"NH_2",4) || !strncmp(hp_a->c,"H2_N",4))
	    {
	      hcount = 2;
	      strcpy (aaa, "N");
	    }
	  else
	    {
	    fprintf(stderr,"untranslatable label %s\n",hp_a->c);
	      strcpy (aaa, "X");
	    }
	}
      else
	{
	  strcpy (aaa, "C");
	}
      fprintf (fp,
	       "%10.4f%10.4f%10.4f %-3s 0  0  0  %1d  0  0  0  0  0  0  0  0\n",
	       (double) atoms[d][0] / factor, (double) (500.-atoms[d][1]) / factor,
	       0.0, aaa, hcount);
    }
  /* Now for the bonds */
  hp_b = da_root.next;
  for (d = 0; d < hp->n; d++)
    {
      if (hp_b->decoration == 0 && (!(mark.flag && (hp_b->smarked + hp_b->tmarked) == 0)))
	{
	  atom1 = 0;
	  atom2 = 0;
	  for (e = 0; e < labelcount; e++)
	    {
	      if (atoms[e][0] == hp_b->x && atoms[e][1] == hp_b->y)
		{
		  atom1 = e + 1;
		}
	      if (atoms[e][0] == hp_b->tx && atoms[e][1] == hp_b->ty)
		{
		  atom2 = e + 1;
		}
	    }
	}

      /* parse the bond type */
      tt = 1;
      ss = 0;
      switch (hp_b->bond)
	{
	case 1:
	case 2:
	  tt = 2;	/*double*/	/* 7=double or aromatic */
	  break;
	case 4:
	  tt = 2;		/* double */
	  break;
	case 3:
	case 18:
	case 19:       /* FIXME: does MOL handle quadruple bonds at all ?*/
	  tt = 3;		/* triple */
	  break;
	case 5:
	case 10:
	  ss = 1;		/* stereo up */
	  break;
	case 6:
	case 16:
	  ss = 6;		/* stereo down */
	  break;
	case 7:
	  ss = 4;		/* stereo either */
	  break;
	case 8:
	case 9:
	case 11:
/*    case 12: */ /* bond or not? */
	case 17:
	  tt = 0;		/* assume no chemical bond */
	  break;
	case 14:
	case 15:
	  tt = 1;		/* 6 = single or aromatic */
	  break;
	default:
	  tt = 1;		/* single */
	  break;
	}
      if (tt)
	{
	  fprintf (fp, "%3i%3i%3i%3i  0  0  0\n", atom1, atom2, tt, ss);
	}
      hp_b = hp_b->next;
    }
  fprintf (fp, "M  END\n");
  if (!topipe) fclose (fp);
  return 0;
}

int
export_svg (char *filename)
/* exports the current drawing to a Scaled Vector Graphics file */
{
  FILE *fp;
  int x, y, tx, ty, w, h;
  int x1,y1,x2,y2;
  float area;
  float factor;
  struct data *hp_b,*hp_bx;
  struct dc *hp_a;
  struct spline *hp_sp;
  struct xy_co *coord;
  int d, dd, i;
  int dy;
  int xbase, ybase, xside, yside, xend, yend, xlen, ylen;
  int shifted = 0;
  int unicodechar;
  int bond_already_tuned=0;
  static char svgcolor[7][8]={"#000000","#0000ff","#00ff00","#00ffff","#ff0000","#ff00ff",
  "#ffff00"};
 
#ifdef GTK2
  setlocale(LC_NUMERIC,"C");
#endif
 
  if ((fp = fopen (filename, "w")) == NULL)
    return (1);
  fprintf (fp,
	   "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n");
  fprintf (fp, "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.0//EN\"\n");
  fprintf (fp, "  \"http://www.w3.org/TR/SVG/DTD/svg10.dtd\">\n");

  if (mark.flag && mark.w != 0 && mark.h != 0)
    {
      w = mark.w * size_factor - 6;
      h = mark.h * size_factor - 6;
    }
  else
    {
/* get true extents of drawing*/
      w = 0;
      h = 0;
      hp_b = da_root.next;
      for (d = 0; d < hp->n; d++)
	{
	  w = MAX (w, hp_b->x);
	  w = MAX (w, hp_b->tx);
	  h = MAX (h, hp_b->y);
	  h = MAX (h, hp_b->ty);
	  hp_b = hp_b->next;
	}
      hp_a = dac_root.next;
      for (d = 0; d < hp->nc; d++)
	{
	  w = MAX (w, hp_a->x);
	  if (hp_a->direct > -2)
	    w = MAX (w, hp_a->x + (int)strlen (hp_a->c) * (6 + 2 * size_factor));
	  h = MAX (h, hp_a->y + 8);
	  hp_a = hp_a->next;
	}

      hp_sp = sp_root.next;
      for (d = 0; d < hp->nsp; d++)
	{
	  w = MAX (w, hp_sp->x0);
	  h = MAX (h, hp_sp->y0);
	  w = MAX (w, hp_sp->x1);
	  h = MAX (h, hp_sp->y1);
	  w = MAX (w, hp_sp->x2);
	  h = MAX (h, hp_sp->y2);
	  w = MAX (w, hp_sp->x3);
	  h = MAX (h, hp_sp->y3);
	  hp_sp = hp_sp->next;
	}
      w = (int) (w *  1.1) ;
      h = (int) (h *  1.1) ;
    }
  fprintf (fp,
	   "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\"\n");
  fprintf (fp,"     width=\"%dpx\" height=\"%dpx\" viewport-fill=\"%s\" >\n", w, h,bghexcolor);
  fprintf (fp, "<desc>\nCreated with Chemtool %s from file %s \n</desc>\n",
	   VERSION, filename);
  fprintf (fp, "<g>\n");
  hp_b = da_root.next;
  for (d = 0; d < hp->n; d++)
    {
      if (mark.flag && (hp_b->smarked + hp_b->tmarked) == 0)
	{
	}
      else
	{
	  coord = bond_cut (hp_b->x, hp_b->y, hp_b->tx, hp_b->ty, 12);


	  x = coord->x;
	  y = coord->y;
	  tx = coord->tx;
	  ty = coord->ty;
	if ( !use_whiteout) {
        int siz,j;
        float lfactor=((float)calc_vector(abs(tx-x),abs(ty-y))/(832.*size_factor));
        if (lfactor >1.) lfactor=1.; /* short bonds should not shrink as much, they might vanish completely */
                                     /* extra long bonds, however, need not leave more space around labels */
		i=has_label(hp_b->x,hp_b->y);
		if (i>=0) {
                 hp_a = dac_root.next;
                 for (j = 0; j < i; j++)hp_a=hp_a->next;
		 siz=2*hp_a->size;
                 if (y-ty==0&& hp_a->direct ==0) {
                   if (tx>x) {
                      int strl=0;
                      for (i=0;i<(int)strlen(hp_a->c);i++) {
                        if(hp_a->c[i] != '_' && hp_a->c[i] != '^' 
                        && hp_a->c[i] != '{' && hp_a->c[i] != '{')
                        strl++;
                      }
                      if (strl>1) x+=strl*(siz+1);
                   }     
//                   else
//                      x-=(strlen(hp_a->c)-1)*fzoom*(siz+1);
		 }
		int ox=x;
				x += (lfactor*(siz+1)/calc_vector(abs(tx-x),abs(ty-y))) *(tx-x);
				y += (lfactor*(siz+1)/calc_vector(abs(tx-ox),abs(ty-y))) *(ty-y);
				}
		i=has_label(hp_b->tx,hp_b->ty);
		if (i>=0) {
                 hp_a = dac_root.next;
                 for (j = 0; j < i; j++)hp_a=hp_a->next;
		 siz=2*hp_a->size;
                 if (y-ty==0 ){
                      int strl=0;
                      for (i=0;i<(int)strlen(hp_a->c);i++) {
                        if(hp_a->c[i] != '_' && hp_a->c[i] != '^' 
                        && hp_a->c[i] != '{' && hp_a->c[i] != '{')
                        strl++;
                      }
                   if ( hp_a->direct<-1) {
                      if (tx>x)
                        tx-=(strl-1)*(siz+1);
//                      else
//                       tx+=(strlen(hp_a->c)-1)*(siz+1);
                   }
                   else if ( hp_a->direct==0) {
//                      if (tx>x)
  //                      tx-=(strlen(hp_a->c)-1)*fzoom*(siz+1);
    //                  else
 if (tx<x)                      tx+=(strl-1)*(siz+1);
                   }
                }  
		int otx=tx;
				tx -= (lfactor*(siz+1)/calc_vector(abs(tx-x),abs(ty-y))) *(tx-x);
				ty -= (lfactor*(siz+1)/calc_vector(abs(otx-x),abs(ty-y))) *(ty-y);
				}
		}		


	  if (!hp_b->bond)
	    {
	      fprintf (fp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\"\n",
		       x, y, tx, ty);
	      fprintf (fp, "style=\"stroke-width:2; stroke:%s\" />\n",svgcolor[hp_b->color]);
	    }
	  if (hp_b->bond == 5)
	    {
    bond_already_tuned = 0;
  x1 = tx - (int) (0.1 * (float) (ty - y));
  y1 = ty + (int) (0.1 * (float) (tx - x));
  x2 = tx + (int) (0.1 * (float) (ty - y));
  y2 = ty - (int) (0.1 * (float) (tx - x));
factor=1.;
  hp_bx=da_root.next;
  for (dd = 0; dd < hp->n; dd++)
    {
    
      if (hp_bx->bond == 10)
	{
	  if (
	      (abs (hp_bx->x * factor - tx) < 3
	       && abs (hp_bx->y * factor - ty) < 3)
	      || (abs (hp_bx->tx * factor - tx) < 3
		  && abs (hp_bx->ty * factor - ty) < 3))
	    {
	      coord =
		center_double_bond (hp_bx->x, hp_bx->y, hp_bx->tx, hp_bx->ty, db_dist);
	      if (abs (hp_bx->x * factor - tx) < 3
		  && abs (hp_bx->y * factor - ty) < 3)
		{
		  x1 = coord->x * factor;
		  y1 = coord->y * factor;
		  coord++;
		  x2 = coord->x * factor;
		  y2 = coord->y * factor;
		}
	      else
		{
		  x1 = coord->tx * factor;
		  y1 = coord->ty * factor;
		  coord++;
		  x2 = coord->tx * factor;
		  y2 = coord->ty * factor;
		}
	      area =  (0.5 * abs (x * (y1 - y2)
				 + x1 * (y2 - y) + x2 * (y - y1)));

	      if (fabs (area) < 76. * factor)
		{
		  x1 = tx - (int) (0.05 * (float) (ty - y));
		  y1 = ty + (int) (0.05 * (float) (tx - x));
		  x2 = tx + (int) (0.05 * (float) (ty - y));
		  y2 = ty - (int) (0.05 * (float) (tx - x));
		}
	      else bond_already_tuned = 1;

	    } /* if connected to wide end of this wedge */
      } /* if adjoining bond is wide */
      if (hp_bx->bond == 0 && !bond_already_tuned) {
        if ((abs (hp_bx->x * factor - tx) < 3 && abs (hp_bx->y * factor - ty) < 3)
          ||(abs (hp_bx->tx * factor - tx) < 3 && abs (hp_bx->ty * factor - ty) < 3))

        /* let the wedge join smoothly alongside another bond */
	{
	  coord = intersect(x,y,x1,y1,hp_bx->x*factor,hp_bx->y*factor,
			    hp_bx->tx*factor,hp_bx->ty*factor);
	  coord->tx = coord->x;
	  coord->ty = coord->y;
	  coord = intersect(x,y,x2,y2,hp_bx->x*factor,hp_bx->y*factor,
			    hp_bx->tx*factor,hp_bx->ty*factor);
	  x1 = coord->tx; 
          y1 = coord->ty;
          x2 = coord->x;
          y2 = coord->y;

	  area = 0.5 * abs (x * (y1 - y2)
                                 + x1 * (y2 - y) + x2 * (y - y1));

          if (fabs (area) > 3300. * factor || fabs(area) < 1750. * factor)
            {
              x1 = tx - (int) (0.1 * (float) (ty - y));
              y1 = ty + (int) (0.1 * (float) (tx - x));
              x2 = tx + (int) (0.1 * (float) (ty - y));
              y2 = ty - (int) (0.1 * (float) (tx - x));
            }
        } /* if connected to wide end of this wedge */
      } /* if adjoining bond is single, and not already adjusted */
      hp_bx = hp_bx->next;
      } /* for dd */ 
	      fprintf (fp,
		       "<polygon style=\"fill:%s; stroke:%s; stroke-width:1\"\n",
		       svgcolor[hp_b->color],svgcolor[hp_b->color]);
	      fprintf (fp, "            points=\"%d,%d %d,%d %d,%d\" />\n", x,
		       y, x1,y1,x2,y2);
/*	      fprintf (fp, "            points=\"%d,%d %d,%d %d,%d\" />\n", x,
		       y, (int) (tx - 0.08 * (ty - y)),
		       (int) (ty + 0.08 * (tx - x)),
		       (int) (tx + 0.08 * (ty - y)),
		       (int) (ty - 0.08 * (tx - x)));
*/
	    }
	  if (hp_b->bond == 6)
	    {
	      for (i = 0; i < 8; i++)
		{
		  fprintf (fp,
			   "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\"\n",
			   (int) (x + 0.125 * i * (tx - x) -
				  0.01 * (ty - y) * i),
			   (int) (y + 0.125 * i * (ty - y) +
				  0.01 * (tx - x) * i),
			   (int) (x + 0.125 * i * (tx - x) +
				  0.01 * (ty - y) * i),
			   (int) (y + 0.125 * i * (ty - y) -
				  0.01 * (tx - x) * i));
		  fprintf (fp, "style=\"stroke-width:1; stroke:%s\" />\n",svgcolor[hp_b->color]);
		}
	    }
	  if (hp_b->bond == 7)
	    {
	    	i=MAX(abs(tx-x),abs(ty-y))/10;
	      fprintf (fp, "<path d=\"M %d,%d\n", x, y);
	      fprintf (fp, "A %d,%d %d %d %d %d %d\n",
		       i,i,0,0,0,x+(tx-x)/5,y+(ty - y)/5);
	      fprintf (fp, "A %d,%d %d %d %d %d %d\n", 
		       i,i,0,1,1,x+2*(tx-x)/5,y+2*(ty - y)/5);
	      fprintf (fp, "A %d,%d %d %d %d %d %d\n", 
		       i,i,0, 0,0,x+3*(tx-x)/5,y+3*(ty - y)/5);
	      fprintf (fp, "A %d,%d %d %d %d %d %d\n", 
		       i,i,0,1,1,x+4*(tx-x)/5,y+4*(ty - y)/5);
	      fprintf (fp, "A %d,%d %d %d %d %d %d\"\n", 
		       i,i,0,0,0,tx,ty);
	      fprintf (fp,
		       "style=\"stroke-width:2; stroke:%s; fill:none\" />\n",svgcolor[hp_b->color]);
	    }
	  if (hp_b->bond == 8)
	    {
	      fprintf (fp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\"\n",
		       x, y, (int)(x+0.8*(tx-x)), (int)(y+0.8*(ty-y)));
	      fprintf (fp, "style=\"stroke-width:2; stroke:%s\" />\n",svgcolor[hp_b->color]);
	      fprintf (fp,
		       "<polygon style=\"fill:%s; stroke:%s; stroke-width:1\"\n",svgcolor[hp_b->color],svgcolor[hp_b->color]);
	      fprintf (fp, "            points=\"%d,%d %d,%d %d,%d\" />\n",
		       (int) (x + 0.8 * (tx - x)), (int) (y + 0.8 * (ty - y)),
		       (int) (x + 0.8 * (tx - x) + 0.1 * (ty - y)),
		       (int) (y + 0.8 * (ty - y) - 0.1 * (tx - x)), tx, ty);
	    }
	  if (hp_b->bond == 9)
	    {
            int   xlen = tx - x;
            int   ylen = ty - y;
            float veclen = sqrt ((double)(xlen * xlen + ylen * ylen));
            float scalefact=64./veclen; /* keep arrowhead size constant (64=std length)*/
            int xbase = (int) (tx - 0.2 *xlen*scalefact);
            int ybase = (int) (ty - 0.2 *ylen*scalefact);
       
	      fprintf (fp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\"\n",
		       x, y, xbase, ybase);
/*x, y, (int)(x+0.8*(tx-x)), (int)(y+0.8*(ty-y)));*/
	      fprintf (fp, "style=\"stroke-width:2; stroke:%s\" />\n",svgcolor[hp_b->color]);
	      fprintf (fp,
		       "<polygon style=\"fill:%s; stroke:%s; stroke-width:1\"\n",svgcolor[hp_b->color],svgcolor[hp_b->color]);
	      fprintf (fp, "            points=\"%d,%d %d,%d %d,%d\" />\n",
tx, ty,(int)(xbase + 0.1 * ylen*scalefact),
(int)(ybase - 0.1 * xlen*scalefact) ,
(int)(xbase - 0.1 * ylen*scalefact),
(int)(ybase + 0.1 * xlen*scalefact) ); 
/*(int) (x + 0.8 * (tx - x) + 0.1 * (ty - y)),
(int) (y + 0.8 * (ty - y) - 0.1 * (tx - x)),
(int) (x + 0.8 * (tx - x) - 0.1 * (ty - y)),
(int) (y + 0.8 * (ty - y) + 0.1 * (tx - x)));*/
	    }
	  if (hp_b->bond == 11)
	    {
	      fprintf (fp, "<circle cx=\"%d\" cy=\"%d\" r=\"%d\"\n",
		       x, y, calc_vector (x - tx, y - ty));
	      fprintf (fp,
		       "style=\"fill:none; stroke-width:2; stroke:%s\" />\n",svgcolor[hp_b->color]);
	    }
	  if (hp_b->bond == 10)
	    {
	      fprintf (fp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\"\n",
		       x, y, tx, ty);
	      fprintf (fp, "style=\"stroke-width:10; stroke:%s\" />\n",svgcolor[hp_b->color]);
	    }

	  if (hp_b->bond == 12)
	    {
	      fprintf (fp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\"\n",
		       x, y, tx, ty);
	      fprintf (fp,
		       "style=\"stroke-width:2; stroke-dasharray:1,2; stroke:%s\" />\n",svgcolor[hp_b->color]);
	    }
	  if (hp_b->bond == 13)
	    {
	      fprintf (fp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\"\n",
		       x + (tx - x) / 4, y + (ty - y) / 4,
		       tx - (tx - x) / 4, ty - (ty - y) / 4);
	      fprintf (fp, "style=\"stroke-width:2; stroke:white\" />\n");
	      fprintf (fp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\"\n",
		       x, y, tx, ty);
	      fprintf (fp, "style=\"stroke-width:2; stroke:%s\" />\n",svgcolor[hp_b->color]);
	    }
	  if (hp_b->bond == 4)
	    {
	      coord = center_double_bond (x, y, tx, ty, db_dist);
	      fprintf (fp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\"\n",
		       coord->x, coord->y, coord->tx, coord->ty);
	      fprintf (fp, "style=\"stroke-width:2; stroke:%s\" />\n",svgcolor[hp_b->color]);
	      coord++;
	      fprintf (fp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\"\n",
		       coord->x, coord->y, coord->tx, coord->ty);
	      fprintf (fp, "style=\"stroke-width:2; stroke:%s\" />\n",svgcolor[hp_b->color]);
	    }
	  if (hp_b->bond == 1 || hp_b->bond == 3)
	    {
	      coord = multi_bonds (x, y, tx, ty, mb_dist);
	      fprintf (fp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\"\n",
		       x, y, tx, ty);
	      fprintf (fp, "style=\"stroke-width:2; stroke:%s\" />\n",svgcolor[hp_b->color]);
	      fprintf (fp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\"\n",
		       coord->x, coord->y, coord->tx, coord->ty);
	      fprintf (fp, "style=\"stroke-width:2; stroke:%s\" />\n",svgcolor[hp_b->color]);
	    }
	  if (hp_b->bond == 2 || hp_b->bond == 3)
	    {
	      coord = multi_bonds (tx, ty, x, y, mb_dist);
	      fprintf (fp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\"\n",
		       x, y, tx, ty);
	      fprintf (fp, "style=\"stroke-width:2; stroke:%s\" />\n",svgcolor[hp_b->color]);
	      fprintf (fp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\"\n",
		       coord->x, coord->y, coord->tx, coord->ty);
	      fprintf (fp, "style=\"stroke-width:2; stroke:%s\" />\n",svgcolor[hp_b->color]);
	    }
	  if (hp_b->bond == 14)
	    {
	      coord = multi_bonds (x, y, tx, ty, mb_dist);
	      fprintf (fp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\"\n",
		       x, y, tx, ty);
	      fprintf (fp, "style=\"stroke-width:2; stroke:%s\" />\n",svgcolor[hp_b->color]);
	      fprintf (fp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\"\n",
		       coord->x, coord->y, coord->tx, coord->ty);
	      fprintf (fp,
		       "style=\"stroke-width:2; stroke-dasharray:4,1; stroke:%s\" />\n",svgcolor[hp_b->color]);
	    }
	  if (hp_b->bond == 15)
	    {
	      coord = multi_bonds (tx, ty, x, y, mb_dist);
	      fprintf (fp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\"\n",
		       x, y, tx, ty);
	      fprintf (fp, "style=\"stroke-width:2; stroke:%s\" />\n",svgcolor[hp_b->color]);
	      fprintf (fp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\"\n",
		       coord->x, coord->y, coord->tx, coord->ty);
	      fprintf (fp,
		       "style=\"stroke-width:2; stroke-dasharray:4,1; stroke:%s\" />\n",svgcolor[hp_b->color]);
	    }
	  if (hp_b->bond == 16)
	    {
	      coord = center_double_bond (x, y, tx, ty, db_dist);
	      x = coord->x;
	      y = coord->y;
	      tx = coord->tx;
	      ty = coord->ty;
	      coord++;
	      for (i = 0; i < 10; i++) {
		fprintf (fp,
			 "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\"\n",
			 (int) (x + 0.1 * i * (tx - x)),
			 (int) (y + 0.1 * i * (ty - y)),
			 (int) (coord->x + 0.1 * i * (coord->tx - coord->x)),
			 (int) (coord->y + 0.1 * i * (coord->ty - coord->y)));
	      fprintf (fp, "style=\"stroke-width:2; stroke:%s\" />\n",svgcolor[hp_b->color]);
	     }
	    }
	  if (hp_b->bond == 18)
	    {
	      fprintf (fp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\"\n",
		       x, y, tx, ty);
	      fprintf (fp, "style=\"stroke-width:2; stroke:%s\" />\n",svgcolor[hp_b->color]);
	      coord = center_double_bond (x, y, tx, ty, db_dist+1);
	      fprintf (fp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\"\n",
		       coord->x, coord->y, coord->tx, coord->ty);
	      fprintf (fp, "style=\"stroke-width:2; stroke:%s\" />\n",svgcolor[hp_b->color]);
	      coord++;
	      fprintf (fp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\"\n",
		       coord->x, coord->y, coord->tx, coord->ty);
	      fprintf (fp, "style=\"stroke-width:2; stroke:%s\" />\n",svgcolor[hp_b->color]);
	    }

	  if (hp_b->bond == 19)
	    {
	int tmpx1,tmpx2,tmpy1,tmpy2,tmptx1,tmptx2,tmpty1,tmpty2;
	      coord = center_double_bond (x, y, tx, ty, db_dist+1);
	tmpx1=coord->x;
	tmpy1=coord->y;
	tmptx1=coord->tx;
	tmpty1=coord->ty;
	coord++;
	tmpx2=coord->x;
	tmpy2=coord->y;
	tmptx2=coord->tx;
	tmpty2=coord->ty;
	      coord = center_double_bond (tmpx1, tmpy1, tmptx1, tmpty1, db_dist-1);
	      fprintf (fp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\"\n",
		       coord->x, coord->y, coord->tx, coord->ty);
	      fprintf (fp, "style=\"stroke-width:2; stroke:%s\" />\n",svgcolor[hp_b->color]);
	      coord++;
	      fprintf (fp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\"\n",
		       coord->x, coord->y, coord->tx, coord->ty);
	      fprintf (fp, "style=\"stroke-width:2; stroke:%s\" />\n",svgcolor[hp_b->color]);
	      coord = center_double_bond (tmpx2, tmpy2, tmptx2, tmpty2, db_dist-1);
	      fprintf (fp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\"\n",
		       coord->x, coord->y, coord->tx, coord->ty);
	      fprintf (fp, "style=\"stroke-width:2; stroke:%s\" />\n",svgcolor[hp_b->color]);
	      coord++;
	      fprintf (fp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\"\n",
		       coord->x, coord->y, coord->tx, coord->ty);
	      fprintf (fp, "style=\"stroke-width:2; stroke:%s\" />\n",svgcolor[hp_b->color]);
	}	    
	}
      hp_b = hp_b->next;
    }

  hp_a = dac_root.next;
  for (d = 0; d < hp->nc; d++)
    {
      if (mark.flag && hp_a->marked == 0)
	{
	}
      else
	{
	 if (hp_a->font == 0 ) { 
	  switch (hp_a->direct)
	    {
	    case Middle_Text:
	      fprintf (fp,
		       "<text x=\"%d\" y=\"%d\" style=\"font-family:Helvetica; font-size:%dpt; fill:%s;",
		       hp_a->x - 2, hp_a->y + 8,12+hp_a->size*2,svgcolor[hp_a->color]);
	      fprintf (fp, "text-anchor:middle;\" > \n<tspan>\n");
	      break;
	    case Left_Text:
	      fprintf (fp,
		       "<text x=\"%d\" y=\"%d\" style=\"font-family:Helvetica; font-size:%dpt; fill:%s;",
		       hp_a->x - 8 , hp_a->y + 8,12+hp_a->size*2,svgcolor[hp_a->color]);
	      fprintf (fp, "text-anchor:start;\" > \n<tspan>\n");
	      break;
	    case Right_Text:
	      fprintf (fp,
		       "<text x=\"%d\" y=\"%d\" style=\"font-family:Helvetica; font-size:%dpt; fill:%s;",
		       hp_a->x + 6, hp_a->y + 8,12+hp_a->size*2,svgcolor[hp_a->color]);
	      fprintf (fp, "text-anchor:end;\" > \n<tspan>\n");
	      break;
	    default:
	      fprintf (stderr, "undefined text direction in svg output\n");
	      ;;
	    }
	} else {
	  switch (hp_a->direct)
	    {
	    case Middle_Text:
	      fprintf (fp,
		       "<text x=\"%d\" y=\"%d\" style=\"font-family:Times; font-size:%dpt; fill:%s;",
		       hp_a->x - 2, hp_a->y + 8,12+hp_a->size*2,svgcolor[hp_a->color]);
	      fprintf (fp, "text-anchor:middle;\" > \n<tspan>\n");
	      break;
	    case Left_Text:
	      fprintf (fp,
		       "<text x=\"%d\" y=\"%d\" style=\"font-family:Times; font-size:%dpt; fill:%s;",
		       hp_a->x - 8 , hp_a->y + 8,12+hp_a->size*2,svgcolor[hp_a->color]);
	      fprintf (fp, "text-anchor:start;\" > \n<tspan>\n");
	      break;
	    case Right_Text:
	      fprintf (fp,
		       "<text x=\"%d\" y=\"%d\" style=\"font-family:Times; font-size:%dpt; fill:%s;",
		       hp_a->x + 6, hp_a->y + 8,12+hp_a->size*2,svgcolor[hp_a->color]);
	      fprintf (fp, "text-anchor:end;\" > \n<tspan>\n");
	      break;
	    default:
	      fprintf (stderr, "undefined text direction in svg output\n");
	      ;;
	    }
	  }
	  for (i = 0; i < (int)strlen (hp_a->c); ++i)
	    {
	      if (hp_a->c[i] == '\\')
		hp_a->c[i] = ' ';
	      if (hp_a->c[i] == '@')
		{
			unicodechar=848+hp_a->c[++i];
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
			case 1031:										case 775: /* bullet */
				unicodechar=8226;
				break;
                        case 891: //oplus
                                unicodechar=8853;
                                break;
                        case 893: //ominus
                                unicodechar=8854;
                                break;			
			default:
				break;
			}				 
		  fprintf (fp, "&#%04d;", unicodechar);
		  /*FIXME: unicode greek does not always map to X11 Symbol, e.g. F is Z not Phi */
		}
	      else if (hp_a->c[i] == '#')
		{
		  fprintf (fp, "\n<tspan style=\"font-weight:bold;\">");
		  fprintf (fp, "%c", hp_a->c[++i]);
		  fprintf (fp, "</tspan>\n");
		}
	      else if (hp_a->c[i] == '|')
		{
		  fprintf (fp, "\n<tspan style=\"font-style:oblique;\">");
		  fprintf (fp, "%c", hp_a->c[++i]);
		  fprintf (fp, "</tspan>\n");
		}
	      else if (hp_a->c[i] == '_')
		{
		  dy=5+hp_a->size;
		  fprintf (fp, "</tspan>\n<tspan dy=\"%d\" style=\"font-size:%dpt;\">",  dy, 8+hp_a->size*2);
		  if (hp_a->c[i + 1] && hp_a->c[i + 1] == '{')
		    {
		      shifted = 1;
		      i++;
		    }
		  if (shifted == 1)
		    {
		      while (hp_a->c[i + 1] && hp_a->c[i + 1] != '}')
			fprintf (fp, "%c", hp_a->c[++i]);
		      shifted = 0;
		      i++;
		    }
		  else
		    {
		      fprintf (fp, "%c", hp_a->c[++i]);
		    }
		  fprintf (fp, "</tspan>\n<tspan dy=\"%d\">\n", -dy);
		}
	      else if (hp_a->c[i] == '^')
		{
		  dy=-2 * (2 + hp_a->size); /* BUG FIX 1 */
		  fprintf (fp, "</tspan>\n<tspan dy=\"%d\" style=\"font-size:%dpt;\">", dy, 8+hp_a->size*2);
		  if (hp_a->c[i + 1] && hp_a->c[i + 1] == '{')
		    {
		      shifted = 1;
		      i++;
		    }
		  if (shifted == 1)
		    {
		      while (hp_a->c[i + 1] && hp_a->c[i + 1] != '}')
			fprintf (fp, "%c", hp_a->c[++i]);
		      shifted = 0;
		      i++;
		    }
		  else
		    {
		      fprintf (fp, "%c", hp_a->c[++i]);
		    }
		  fprintf (fp, "</tspan>\n<tspan dy=\"%d\">\n", -dy);
		}
	      else
		{
#ifndef GTK2
		if ((unsigned char)hp_a->c[i] >128)
		  fprintf (fp, "&#%03d;",(unsigned char)hp_a->c[i]);
		  else
#endif
		     if (hp_a->c[i] == '<')
		  fprintf (fp, "&lt;");
		else if (hp_a->c[i] == '>')
		  fprintf (fp, "&gt;");
		else if (hp_a->c[i] == '&')
		  fprintf (fp, "&amp;");
		else
		  fprintf (fp, "%c", hp_a->c[i]);
		}
            if (hp_a->c[i]==' ') /* protect blanks again after output */
              hp_a->c[i]='\\';
	    }
	  fprintf (fp, "</tspan>\n</text>\n");
	}
      hp_a = hp_a->next;
    }

  hp_sp = sp_root.next;
  for (d = 0; d < hp->nsp; d++)
    {
      if (mark.flag == 1 && hp_sp->marked == 0)
	{
	}
      else
	{
	  fprintf (fp, "<path d=\"M %d,%d\n", hp_sp->x0, hp_sp->y0);
	  fprintf (fp, "C %d,%d %d,%d %d,%d\"\n", hp_sp->x1, hp_sp->y1,
		   hp_sp->x2, hp_sp->y2, hp_sp->x3, hp_sp->y3);
	  switch (hp_sp->type)
	    {
	    case 0:
	    case 1:
	    case 2:
	      fprintf (fp,
		       "style=\"stroke-width:2; stroke:%s; fill:none\">\n",svgcolor[hp_sp->color]);
	      break;
	      ;;
	    case -2:
	      fprintf (fp,
		       "style=\"stroke-width:2; stroke-dasharray:4,1; stroke:%s fill:none\">\n",svgcolor[hp_sp->color]);
	      break;
	      ;;
	    case -1:
	      fprintf (fp,
		       "style=\"stroke-width:2; stroke:%s; fill:solid\">\n",svgcolor[hp_sp->color]);
	    }
	  fprintf (fp, "</path>\n");

	  if (hp_sp->type > 0)
	    {
	      xbase = (int)
		(0.7 * 0.7 * 0.7 * (double) hp_sp->x3 + 3. * 0.7 * 0.7 * (1. -
									 0.7)
		* (double) hp_sp->x2 + 3. * 0.7 * (1. - 0.7) * (1. -
								0.7) *
		(double) hp_sp->x1 + (1. - 0.7) * (1. - 0.7) * (1. -
								0.7) *
		hp_sp->x0);
	      ybase = (int)
		(0.7 * 0.7 * 0.7 * (double) hp_sp->y3 + 3. * 0.7 * 0.7 * (1. -
									 0.7)
		* (double) hp_sp->y2 + 3. * 0.7 * (1. - 0.7) * (1. -
								0.7) *
		(double) hp_sp->y1 + (1. - 0.7) * (1. - 0.7) * (1. -
								0.7) *
		hp_sp->y0);


	      xlen = hp_sp->x3 - xbase;
	      ylen = hp_sp->y3 - ybase;

	      if (xlen != 0)
		xlen = (int) copysign (50., (double)xlen);
	      if (ylen != 0)
		ylen = (int) copysign (50., (double)ylen);

	      xside = (int) (xbase + 0.15 * ylen);
	      yside = (int) (ybase - 0.15 * xlen);

	      if (hp_sp->type == 1)
		{

		  xend = (int) (xbase - 0.15 * ylen);
		  yend = (int) (ybase + 0.15 * xlen);
		  x =
		    (xside - hp_sp->x0) * (xside - hp_sp->x0) + (yside -
								 hp_sp->y0) *
		    (yside - hp_sp->y0);
		  tx =
		    (xend - hp_sp->x0) * (xend - hp_sp->x0) + (yend -
							       hp_sp->y0) *
		    (yend - hp_sp->y0);
		  if (tx > x)
		    {
		      xside = xend;
		      yside = yend;
		    }
		  xend = xbase;	/*on baseline */
		  yend = ybase;
		}
	      else
		{

		  xend = (int) (xbase - 0.15 * ylen);
		  yend = (int) (ybase + 0.15 * xlen);
		}


	      fprintf (fp,
		       "<polygon style=\"fill:%s; stroke:%s; stroke-width:2\"\n",svgcolor[hp_sp->color],svgcolor[hp_sp->color]);
	      fprintf (fp, "            points=\"%d,%d %d,%d, %d,%d\" />\n",
		       hp_sp->x3, hp_sp->y3, xside, yside, xend, yend);
	    }
	}
      hp_sp = hp_sp->next;
    }

  fprintf (fp, "</g>\n</svg>\n");
  fclose (fp);
  return (0);
}

int
export_emf (char *filename)
/* exports the current drawing to an ECMA Enhanced Metafile  */

#ifndef EMF
{
  char com[255];
  FILE *xfile;
  int rval;
#if 1
  char tmpfile[512];
  FILE *fp;
  int fd;
#endif  

#ifdef GTK2
  setlocale(LC_NUMERIC,"C");
#endif

  if (figversion == 0)
    return (1);			/* cannot export without fig2dev */
  if ((int)strlen (filename))
    {
#if 0
	snprintf (com,255, "fig2dev  -L emf > %s", filename);
      xfile = popen (com, "w");
#else
      strcpy (tmpfile, "/tmp/chtXXXXXX");
/*@ignore@ splint does not recognize mkstemp */
      fd = mkstemp (tmpfile);
/*@end@*/
      if (fd == -1)
	return (1);
      fp = fdopen (fd, "w");
      rval = exfig (fp, 0);
      if (rval != 0) 
        return(rval);
      if (fclose (fp) != 0)
        return(1);
        
	snprintf (com,255, "fig2dev  -L emf %s> \"%s\"", tmpfile, filename);
      xfile = popen (com, "w");
#endif        
      if (pclose (xfile) != 0)
	return (1);
      return (rval);
    }
  else
    return (1);
}

#else  

{
  int x, y, tx, ty, w, h;
  struct data *hp_b;
  struct dc *hp_a;
  struct spline *hp_sp;
  struct xy_co *coord;
  int d, i;
  int xbase, ybase, xside, yside, xend, yend, xlen, ylen;
  int shifted = 0;
  
  int ha,fzoom,xpos,chl,nsub,nsup,rval,l;
  int csize,fontsize,offset,savedpos;
  LPPOINT dummy={0};
  POINT points[4];
  char description[30];
  HWND desktop = GetDesktopWindow();
  HDC dc= GetDC(desktop);
//    PCSTR description = "Created by\0chemtool 1.6.13\0";
    char *tmpstring=malloc(80*sizeof(char));
    HENHMETAFILE mfh;
    HDC metaDC  ;
    HPEN nopen=CreatePen(PS_SOLID,0,RGB(0xff,0xff,0xff));
    HPEN pen=CreatePen(PS_SOLID,1,RGB(0x00,0x00,0x00));
    HPEN widepen=CreatePen(PS_SOLID,5,RGB(0x00,0x00,0x00));
    HPEN dashedpen=CreatePen(PS_DASH,1,RGB(0x00,0x00,0x00));
    HPEN dottedpen=CreatePen(PS_DOT,1,RGB(0x00,0x00,0x00));
    HPEN whitepen=CreatePen(PS_SOLID,5,RGB(0xff,0xff,0xff));
    HBRUSH fill=CreateSolidBrush(RGB(0x00,0x00,0x00));
    HFONT normalfont=CreateFontA(-11,0,0,0,FW_MEDIUM,0,0,0,ANSI_CHARSET,
    		OUT_DEFAULT_PRECIS,CLIP_CHARACTER_PRECIS,DEFAULT_QUALITY,
    		DEFAULT_PITCH| FF_DONTCARE, "Helvetica");
    HFONT smallfont=CreateFontA(-8,0,0,0,FW_MEDIUM,0,0,0,ANSI_CHARSET,
    		OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,
    		DEFAULT_PITCH| FF_DONTCARE, "Helvetica");
    HFONT italicfont=CreateFontA(11,0,12,0,FW_MEDIUM,1,0,0,ANSI_CHARSET,
    		OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,
    		DEFAULT_PITCH| FF_DONTCARE, "Helvetica");
    HFONT smallitalicfont=CreateFontA(8,0,8,0,FW_MEDIUM,1,0,0,ANSI_CHARSET,
    		OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,
    		DEFAULT_PITCH| FF_DONTCARE, "Helvetica");
    HFONT boldfont=CreateFontA(11,0,12,0,FW_BOLD,0,0,0,ANSI_CHARSET,
    		OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,
    		DEFAULT_PITCH| FF_DONTCARE, "Helvetica");
    HFONT smallboldfont=CreateFontA(8,0,8,0,FW_BOLD,0,0,0,ANSI_CHARSET,
    		OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,
    		DEFAULT_PITCH| FF_DONTCARE, "Helvetica");
    HFONT symbolfont=CreateFontA(11,0,12,0,FW_MEDIUM,0,0,0,SYMBOL_CHARSET,
    		OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,
    		DEFAULT_PITCH| FF_DONTCARE, "Helvetica");
    HFONT smallsymbolfont=CreateFontA(8,0,8,0,FW_MEDIUM,0,0,0,SYMBOL_CHARSET,
    		OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,
    		DEFAULT_PITCH| FF_DONTCARE, "Helvetica");
    
    
  savedpos = 0;
  switch (zoom_factor)
    {
    case 0:
      fontsize = 8;		/*8 */
      fzoom = 6;		/*6,4 */
      break;
    case 1:
      fontsize = 10;		/*6? */
      fzoom = 7;		/*5? */
      break;
    case 2:
      fontsize = 12;		/*9? */
      fzoom = 8;		/*7? */
      break;
    case 3:
      fontsize = 16;		/*12? */
      fzoom = 9;		/*12,10? */
      break;
    case 4:
      fontsize = 18;
      fzoom = 10;
      break;
    default:
      fprintf (stderr, "no emf font parameters for zoom factor %d\n",
	       zoom_factor);
      rval = 2;
    }
    fzoom=size_factor;
  if (mark.flag && mark.w != 0 && mark.h != 0)
    {
      x = mark.x * size_factor + 3;
      y = mark.y * size_factor + 3;
      w = mark.w * size_factor - 6;
      h = mark.h * size_factor - 6;
    }
  else
    {
/* get true extents of drawing*/
      x = 10000;
      y = 10000;
      w = 0;
      h = 0;
      hp_b = da_root.next;
      for (d = 0; d < hp->n; d++)
	{
	  w = MAX (w, hp_b->x);
	  w = MAX (w, hp_b->tx);
	  h = MAX (h, hp_b->y);
	  h = MAX (h, hp_b->ty);
	  x = MIN (x, hp_b->x);
	  x = MIN (x, hp_b->tx);
	  y = MIN (y, hp_b->y);
	  y = MIN (y, hp_b->ty);
	  hp_b = hp_b->next;
	}
      hp_a = dac_root.next;
      for (d = 0; d < hp->nc; d++)
	{
	  w = MAX (w, hp_a->x);
	  if (hp_a->direct > -2)
	    w = MAX (w, (hp_a->x + (int)strlen (hp_a->c) * 30));
	  h = MAX (h, hp_a->y + 12);
	  x = MIN (x, hp_a->x);
	  if (hp_a->direct < 0)
	    x = MIN (x, (hp_a->x - (int)strlen (hp_a->c) * 30));
	  y = MIN (y, hp_a->y - 12);
	  hp_a = hp_a->next;
	}

      hp_sp = sp_root.next;
      for (d = 0; d < hp->nsp; d++)
	{
	  w = MAX (w, hp_sp->x0);
	  h = MAX (h, hp_sp->y0);
	  w = MAX (w, hp_sp->x1);
	  h = MAX (h, hp_sp->y1);
	  w = MAX (w, hp_sp->x2);
	  h = MAX (h, hp_sp->y2);
	  w = MAX (w, hp_sp->x3);
	  h = MAX (h, hp_sp->y3);
	  x = MIN (x, hp_sp->x0);
	  y = MIN (y, hp_sp->y0);
	  x = MIN (x, hp_sp->x1);
	  y = MIN (y, hp_sp->y1);
	  x = MIN (x, hp_sp->x2);
	  y = MIN (y, hp_sp->y2);
	  x = MIN (x, hp_sp->x3);
	  y = MIN (y, hp_sp->y3);
	  hp_sp = hp_sp->next;
	}
      x = x * size_factor/2 * 0.8;
      y = y * size_factor/2 * 0.8;
      w = (w-x) * size_factor/2 * 1.1 ;
      h = (h-y) * size_factor/2 * 1.1 ;
    }

#ifdef GTK2
    setlocale(LC_NUMERIC,"C");
#endif
    snprintf(description,30,"Created by\0chemtool %s\0",VERSION);

    metaDC = CreateEnhMetaFile (dc, filename, NULL, description);
    SetMapMode (dc, MM_ANISOTROPIC);
    SetViewportExtEx(dc,10,-10,NULL);
/* invisible box for image sizing */
    SelectObject(metaDC,nopen);
    MoveToEx(metaDC,x,h,NULL);
    LineTo(metaDC,w,h);
    LineTo(metaDC,w,y);
    LineTo(metaDC,x,y);
    LineTo(metaDC,x,h);
/* now do the actual drawing */
    SelectObject(metaDC,pen);
    SelectObject(metaDC,fill);
    SetTextColor(metaDC,RGB(0x00,0x00,0x00));
    SetTextAlign(metaDC,TA_LEFT+TA_BASELINE);
    SetBkColor(metaDC,RGB(0xff,0xff,0xff));
    SetBkMode(metaDC,OPAQUE);
  hp_b = da_root.next;
  for (d = 0; d < hp->n; d++)
    {
      if (mark.flag && (hp_b->smarked + hp_b->tmarked) == 0)
	{
	}
      else
	{
	  coord = bond_cut (hp_b->x, hp_b->y, hp_b->tx, hp_b->ty, 15);


	  x = coord->x;
	  y = coord->y;
	  tx = coord->tx;
	  ty = coord->ty;


	  if (!hp_b->bond)
	    {
	    MoveToEx (metaDC, x*size_factor/2,y*size_factor/2,dummy);
	    LineTo(metaDC,tx*size_factor/2,ty*size_factor/2);
	    }
	  if (hp_b->bond == 5)
	    {
	    	points[0].x=x*size_factor/2;
	    	points[0].y=y*size_factor/2;
	    	points[1].x=(int) (tx - 0.1 * (ty - y))*size_factor/2;
		points[1].y=(int) (ty + 0.1 * (tx - x))*size_factor/2;
		points[2].x=(int) (tx + 0.1 * (ty - y))*size_factor/2;
		points[2].y=(int) (ty - 0.1 * (tx - x))*size_factor/2;
		Polygon(metaDC,points,3);
	    }
	  if (hp_b->bond == 6)
	    {
	      for (i = 0; i < 10; i++)
		{
		MoveToEx(metaDC,
			   (int) ((x + 0.1 * i * (tx - x) -
				  0.01 * (ty - y) * i)*size_factor/2),
			   (int) ((y + 0.1 * i * (ty - y) +
				  0.01 * (tx - x) * i)*size_factor/2),dummy);
		LineTo(metaDC,
			   (int) ((x + 0.1 * i * (tx - x) +
				  0.01 * (ty - y) * i)*size_factor/2),
			   (int) ((y + 0.1 * i * (ty - y) -
				  0.01 * (tx - x) * i)*size_factor/2));
		}
	    }
	  if (hp_b->bond == 7)
	    {
	      coord = multi_bonds (x, y, tx, ty, 2*mb_dist);
	    Arc(metaDC,(int)(coord->x*size_factor/2),(int)(coord->ty*size_factor/2),
	    	       (int)((x+0.25*(tx-x))*size_factor/2),
	    	       (int)((y+0.25*(ty-y))*size_factor/2),
			(int)(coord->x*size_factor/2),(int)(coord->y*size_factor/2),
	    	       (int)((x+0.25*(tx-x))*size_factor/2),
	    	       (int)((y+0.25*(ty-y))*size_factor/2));
	      coord = multi_bonds (tx, ty, x, y, 2*mb_dist);
	    Arc(metaDC,(int)(coord->x*size_factor/2),(int)(coord->ty*size_factor/2),
	    	       (int)((x+0.5*(tx-x))*size_factor/2),
	    	       (int)((y+0.5*(ty-y))*size_factor/2),
			(int)(coord->x*size_factor/2),(int)(coord->y*size_factor/2),
	    	       (int)((x+0.5*(tx-x))*size_factor/2),
	    	       (int)((y+0.5*(ty-y))*size_factor/2));
	      coord = multi_bonds (x, y, tx, ty, 2*mb_dist);
	    Arc(metaDC,(int)(coord->x*size_factor/2),(int)(coord->ty*size_factor/2),
	    	       (int)((x+0.75*(tx-x))*size_factor/2),
	    	       (int)((y+0.75*(ty-y))*size_factor/2),
			(int)(coord->x*size_factor/2),(int)(coord->y*size_factor/2),
	    	       (int)((x+0.75*(tx-x))*size_factor/2),
	    	       (int)((y+0.75*(ty-y))*size_factor/2));
	      coord = multi_bonds (tx, ty, x, y, 2*mb_dist);
	    Arc(metaDC,(int)(coord->x*size_factor/2),(int)(coord->ty*size_factor/2),
	    	       (int)((x+1.*(tx-x))*size_factor/2),
	    	       (int)((y+1.*(ty-y))*size_factor/2),
			(int)(coord->x*size_factor/2),(int)(coord->y*size_factor/2),
	    	       (int)((x+1.*(tx-x))*size_factor/2),
	    	       (int)((y+1.*(ty-y))*size_factor/2));
	    }
	  if (hp_b->bond == 8)
	    {
	    MoveToEx (metaDC, x*size_factor/2,y*size_factor/2,dummy);
	    LineTo(metaDC,tx*size_factor/2,ty*size_factor/2);
		points[0].x=(int) (x + 0.8 * (tx - x))*size_factor/2;
		points[0].y=(int) (y + 0.8 * (ty - y))*size_factor/2;
		points[1].x=(int) (x + 0.8 * (tx - x) + 0.1 * (ty - y))*size_factor/2;
		points[1].y=(int) (y + 0.8 * (ty - y) - 0.1 * (tx - x))*size_factor/2;
		points[2].x=(int) (tx*size_factor/2);
		points[2].y=(int) (ty*size_factor/2);
		Polygon(metaDC,points,3);
	    }
	  if (hp_b->bond == 9)
	    {
	    MoveToEx (metaDC, x*size_factor/2,y*size_factor/2,dummy);
	    LineTo(metaDC,tx*size_factor/2,ty*size_factor/2);
		 points[0].x= (int) (tx*size_factor/2);
		 points[0].y= (int) (ty*size_factor/2);
		 points[1].x= (int) ((x + 0.8 * (tx - x) + 0.1 * (ty - y))*size_factor/2);
		 points[1].y= (int) ((y + 0.8 * (ty - y) - 0.1 * (tx - x))*size_factor/2);
		 points[2].x= (int) ((x + 0.8 * (tx - x) - 0.1 * (ty - y))*size_factor/2);
		 points[2].y= (int) ((y + 0.8 * (ty - y) + 0.1 * (tx - x))*size_factor/2);
	         Polygon(metaDC,points,3);
	    }
	  if (hp_b->bond == 11)
	    {
	    w=calc_vector(x-tx,y-ty);
		Ellipse(metaDC, (x-w)*size_factor/2, 
				(y-w)*size_factor/2,
				(x+w)*size_factor/2,
				(y+w)*size_factor/2);
	    }
	  if (hp_b->bond == 10)
	    {
    	SelectObject(metaDC,widepen);
	    MoveToEx (metaDC, x*size_factor/2,y*size_factor/2,dummy);
	    LineTo(metaDC,tx*size_factor/2,ty*size_factor/2);
	    SelectObject(metaDC,pen);
	    }

	  if (hp_b->bond == 12)
	    {
	SelectObject(metaDC,dottedpen);
	    MoveToEx (metaDC, x*size_factor/2,y*size_factor/2,dummy);
	    LineTo(metaDC,tx*size_factor/2,ty*size_factor/2);
    	SelectObject(metaDC,pen);
	    }
	  if (hp_b->bond == 13)
	    {
	SelectObject(metaDC,whitepen);
	    MoveToEx (metaDC, x*size_factor/2,y*size_factor/2,dummy);
	    LineTo(metaDC,tx*size_factor/2,ty*size_factor/2);
    	SelectObject(metaDC,pen);
	    MoveToEx (metaDC, x*size_factor/2,y*size_factor/2,dummy);
	    LineTo(metaDC,tx*size_factor/2,ty*size_factor/2);
	    }
	  if (hp_b->bond == 4)
	    {
	      coord = center_double_bond (x, y, tx, ty, db_dist);
	    MoveToEx (metaDC, coord->x*size_factor/2,coord->y*size_factor/2,dummy);
	    LineTo(metaDC,coord->tx*size_factor/2,coord->ty*size_factor/2);
	      coord++;
	    MoveToEx (metaDC, coord->x*size_factor/2,coord->y*size_factor/2,dummy);
	    LineTo(metaDC,coord->tx*size_factor/2,coord->ty*size_factor/2);
	    }
	  if (hp_b->bond == 1 || hp_b->bond == 3)
	    {
	      coord = multi_bonds (x, y, tx, ty, mb_dist);
	    MoveToEx (metaDC, x*size_factor/2,y*size_factor/2,dummy);
	    LineTo(metaDC,tx*size_factor/2,ty*size_factor/2);
	    MoveToEx (metaDC, coord->x*size_factor/2,coord->y*size_factor/2,dummy);
	    LineTo(metaDC,coord->tx*size_factor/2,coord->ty*size_factor/2);
	    }
	  if (hp_b->bond == 2 || hp_b->bond == 3)
	    {
	      coord = multi_bonds (tx, ty, x, y, mb_dist);
	    MoveToEx (metaDC, x*size_factor/2,y*size_factor/2,dummy);
	    LineTo(metaDC,tx*size_factor/2,ty*size_factor/2);
	    MoveToEx (metaDC, coord->x*size_factor/2,coord->y*size_factor/2,dummy);
	    LineTo(metaDC,coord->tx*size_factor/2,coord->ty*size_factor/2);
	    }
	  if (hp_b->bond == 14)
	    {
	      coord = multi_bonds (x, y, tx, ty, mb_dist);
	    MoveToEx (metaDC, x*size_factor/2,y*size_factor/2,dummy);
	    LineTo(metaDC,tx*size_factor/2,ty*size_factor/2);
    	SelectObject(metaDC,dashedpen);
	    MoveToEx (metaDC, coord->x*size_factor/2,coord->y*size_factor/2,dummy);
	    LineTo(metaDC,coord->tx*size_factor/2,coord->ty*size_factor/2);
    	SelectObject(metaDC,pen);
	    }
	  if (hp_b->bond == 15)
	    {
	      coord = multi_bonds (tx, ty, x, y, mb_dist);
	    MoveToEx (metaDC, x*size_factor/2,y*size_factor/2,dummy);
	    LineTo(metaDC,tx*size_factor/2,ty*size_factor/2);
    	SelectObject(metaDC,dashedpen);
	    MoveToEx (metaDC, coord->x*size_factor/2,coord->y*size_factor/2,dummy);
	    LineTo(metaDC,coord->tx*size_factor/2,coord->ty*size_factor/2);
    	SelectObject(metaDC,pen);
	    }
	  if (hp_b->bond == 16)
	    {
	      coord = center_double_bond (x, y, tx, ty, db_dist);
	      x = coord->x;
	      y = coord->y;
	      tx = coord->tx;
	      ty = coord->ty;
	      coord++;
	      for (i = 0; i < 10; i++){
	    MoveToEx (metaDC,(int) (x+0.1*i*(tx-x)*size_factor/2),
	    (int)(y+0.1*i*(ty-y)*size_factor/2),dummy);
	    LineTo(metaDC,
	    (int)(coord->x+0.1*i*(coord->tx-coord->x)*size_factor/2),
	    (int)(coord->y+0.1*i*(coord->ty-coord->y)*size_factor/2));
	    }
	}
       }
      hp_b = hp_b->next;
   }

  ha = 9 * fzoom;
  ha = 9 *size_factor;
  hp_a = dac_root.next;
  for (d = 0; d < hp->nc; d++)
    {
      if (mark.flag && hp_a->marked == 0)
	{
	}
      else
	{
	  switch (hp_a->direct)
	    {
	    case Middle_Text:
	      chl = 0;
	   SetTextAlign(metaDC,TA_CENTER+TA_BOTTOM);
 	      break;
	    case Left_Text:
	      chl = 9 * fzoom;
	   SetTextAlign(metaDC,TA_LEFT+TA_BOTTOM);
 	      break;
	    case Right_Text:
	      chl = -9 * fzoom;
	   SetTextAlign(metaDC,TA_RIGHT+TA_BOTTOM);
 	      break;
	    default:
	      fprintf (stderr, "undefined text direction in emf output\n");
	      rval = 3;
	      chl=0;
	      ;;
	    }
	  nsub = 0;
	  nsup = 0;
	  for (i = 0; i < (int)strlen (hp_a->c); ++i)
	    if (hp_a->c[i] == '\\')
	      hp_a->c[i] = ' ';
	  if (!strpbrk (hp_a->c, "@_^|#"))
	    {			/*no sub- or superscripts */
	      xpos = hp_a->x * fzoom;
	      csize = fontsize * fzoom * 1.4;
		SelectObject(metaDC,normalfont);
		TextOutA(metaDC,(int)((hp_a->x)*size_factor/2),(int)((hp_a->y+25)*size_factor/2),
		hp_a->c,(int)strlen(hp_a->c));
	    }	
	  else
	    {			/* special formatting needed, every character becomes a separate entity */
	      l = (int)strlen (hp_a->c);
	      l = 0;
	      for (i = 0; i < (int)strlen (hp_a->c); i++)
		{
		  switch (hp_a->c[i])
		    {
		    case '@':
		      break;
		    case '_':
		    case '^':
/*		  l -= .5; ??*/
		      break;
		    default:
		      l++;
		    }
		}
		fzoom=size_factor;
	      xpos = hp_a->x * size_factor - chl;
	      if (hp_a->direct == Right_Text) xpos=xpos-12*(l-2);
	      csize = fontsize * size_factor * 1.4;

		shifted=0;
	      for (i = 0; i < (int)strlen (hp_a->c); i++)
		{
		  offset = 0;
		SelectObject(metaDC,normalfont);
		  csize = fontsize * fzoom * 1.4;

		  if (shifted != 0)
		    {
		      if (hp_a->c[i] == '}')
			{
			  shifted = 0;
			  i++;
			  if (i >= (int)strlen (hp_a->c))
			    break;
			}
		      else
			{
			  offset = shifted;
			}
		    }
		  if (hp_a->c[i] == '_')
		    {
		      if (chl >= 0)
			csize = csize / 2;
		      offset = ha / 2;
		      nsub = nsub + 1;
		      if (nsub == 1)
			{
			  if (nsup > 0)
			    xpos = savedpos;
			  else
			    savedpos = xpos;
			}
		      nsup = 0;
		    }

		  if (hp_a->c[i] == '^')
		    {
		      offset = -ha / 2;
		      nsup = nsup + 1;
		      if (nsup == 1)
			{
			  if (nsub > 0)
			    xpos = savedpos;
			  else
			    savedpos = xpos;
			}
		      nsub = 0;
		    }
		  if (offset != 0)
		    {
		SelectObject(metaDC,smallfont);
/*		      if (chl < 0)
			xpos = xpos - csize / 2;
		      else
			xpos = xpos - 20;*/
			if (hp_a->direct==Right_Text)
			xpos=xpos-10;
		      if (shifted == 0)
			{
			  i++;
			  if (hp_a->c[i] == '{')
			    {
			      shifted = offset;
			      i++;
			    }
			}
		      if (i >= (int)strlen (hp_a->c))
			break;
		      if (hp_a->c[i] != '-' && hp_a->c[i] != '+')
		      offset = offset * 2;
		    }
		  else
		    {		/*reset sub- and superscript counters */
		      nsub = 0;
		      nsup = 0;
		    }

		  if (hp_a->c[i] == '@')
		    {
	SelectObject(metaDC,symbolfont);
	if (offset !=0) SelectObject(metaDC,smallsymbolfont);
		      i++;
		    }
		  if (hp_a->c[i] == '|')
		    {
	SelectObject(metaDC,italicfont);
	if (offset!=0) SelectObject(metaDC,smallitalicfont);
		      /*  csize = csize / 2; */
		      i++;
		    }
		  if (hp_a->c[i] == '#')
		    { 
	SelectObject(metaDC,boldfont);
	if (offset!=0) SelectObject(metaDC,smallboldfont);
		      i++;  
		    }
	sprintf(tmpstring,"%c",hp_a->c[i]);
	TextOutA(metaDC,(int)((xpos-10+abs(offset/2))/2),
			(int)((hp_a->y+25)*size_factor/2),
			tmpstring,(int)strlen(tmpstring));

		  xpos = xpos + 20;
		  if (zoom_factor < 2)
		    xpos = xpos + 10;

		}		/*for i */
	} /* normal or special formatting */	
	  for (i = 0; i < (int)strlen (hp_a->c); ++i)
	    if (hp_a->c[i] == ' ')
	      hp_a->c[i] = '\\';
    } /* if within current set*/
      hp_a = hp_a->next;
   } /* for all labels */

  hp_sp = sp_root.next;
  for (d = 0; d < hp->nsp; d++)
    {
      if (mark.flag == 1 && hp_sp->marked == 0)
	{
	}
      else
	{
	   points[0].x=hp_sp->x0 *size_factor/2;
	   points[0].y=hp_sp->y0 *size_factor/2;
	   points[1].x=hp_sp->x1 *size_factor/2;
	   points[1].y=hp_sp->y1 *size_factor/2;
	   points[2].x=hp_sp->x2 *size_factor/2;
	   points[2].y=hp_sp->y2 *size_factor/2;
	   points[3].x=hp_sp->x3 *size_factor/2;
	   points[3].y=hp_sp->y3 *size_factor/2;
	  switch (hp_sp->type)
	    {
	    case 0:
	    case 1:
	    case 2:
		PolyBezier(metaDC,points,4);
	      break;
	      ;;
	    case -2:
	    	SelectObject(metaDC,dashedpen);
		PolyBezier(metaDC,points,4);
    		SelectObject(metaDC,pen);
	      break;
	      ;;
	    case -1:
	    	SelectObject(metaDC,fill);
		PolyBezier(metaDC,points,4);
    		SelectObject(metaDC,pen);
	    }

	  if (hp_sp->type > 0)
	    {
	      xbase =
		0.7 * 0.7 * 0.7 * (double) hp_sp->x3 + 3. * 0.7 * 0.7 * (1. -
									 0.7)
		* (double) hp_sp->x2 + 3. * 0.7 * (1. - 0.7) * (1. -
								0.7) *
		(double) hp_sp->x1 + (1. - 0.7) * (1. - 0.7) * (1. -
								0.7) *
		hp_sp->x0;
	      ybase =
		0.7 * 0.7 * 0.7 * (double) hp_sp->y3 + 3. * 0.7 * 0.7 * (1. -
									 0.7)
		* (double) hp_sp->y2 + 3. * 0.7 * (1. - 0.7) * (1. -
								0.7) *
		(double) hp_sp->y1 + (1. - 0.7) * (1. - 0.7) * (1. -
								0.7) *
		hp_sp->y0;


	      xlen = hp_sp->x3 - xbase;
	      ylen = hp_sp->y3 - ybase;

	      if (xlen != 0)
		xlen = (int) copysign (50., xlen);
	      if (ylen != 0)
		ylen = (int) copysign (50., ylen);

	      xside = xbase + 0.15 * ylen;
	      yside = ybase - 0.15 * xlen;

	      if (hp_sp->type == 1)
		{

		  xend = xbase - 0.15 * ylen;
		  yend = ybase + 0.15 * xlen;
		  x =
		    (xside - hp_sp->x0) * (xside - hp_sp->x0) + (yside -
								 hp_sp->y0) *
		    (yside - hp_sp->y0);
		  tx =
		    (xend - hp_sp->x0) * (xend - hp_sp->x0) + (yend -
							       hp_sp->y0) *
		    (yend - hp_sp->y0);
		  if (tx > x)
		    {
		      xside = xend;
		      yside = yend;
		    }
		  xend = xbase;	/*on baseline */
		  yend = ybase;
		}
	      else
		{

		  xend = xbase - 0.15 * ylen;
		  yend = ybase + 0.15 * xlen;
		}

	    	SelectObject(metaDC,fill);
		points[0].x=hp_sp->x3*size_factor/2;
		points[0].y=hp_sp->y3*size_factor/2;
		points[1].x=xside*size_factor/2;
		points[1].y=yside*size_factor/2;
		points[2].x=xend*size_factor/2;
		points[2].y=yend*size_factor/2;
		Polygon(metaDC,points,3);
	    	SelectObject(metaDC,pen);

	    }
	}
      hp_sp = hp_sp->next;
    }

   mfh=CloseEnhMetaFile(metaDC);
   DeleteEnhMetaFile(mfh);
   DeleteDC(metaDC);
  return (0);
}
#endif

int
export_bitmap (char *filename)
/* exports the current drawing to an X11 bitmap file */
{
  int x, y, w, h;
  struct data *hp_b;
  struct dc *hp_a;
  struct spline *hp_sp;
  int d;
  int retval;
  char xfile[512];
  Pixmap buffer;
  GdkPixmap *bitmap;
  GdkGC *bitmapgc;

  if (mark.flag && mark.w != 0 && mark.h != 0)
    {
fprintf (stderr, "marked x %d y %d w %d h %d\n",mark.x,mark.y,mark.w,mark.h);
      x = mark.x * size_factor + 3;
      y = mark.y * size_factor + 3;
      w = mark.w * size_factor - 6;
      h = mark.h * size_factor - 6;
    }
  else
    {
/* get true extents of drawing*/
      x = 10000;
      y = 10000;
      w = 0;
      h = 0;
      hp_b = da_root.next;
      for (d = 0; d < hp->n; d++)
	{
	  w = MAX (w, hp_b->x);
	  w = MAX (w, hp_b->tx);
	  h = MAX (h, hp_b->y);
	  h = MAX (h, hp_b->ty);
	  x = MIN (x, hp_b->x);
	  x = MIN (x, hp_b->tx);
	  y = MIN (y, hp_b->y);
	  y = MIN (y, hp_b->ty);
	  hp_b = hp_b->next;
	}
      hp_a = dac_root.next;
      for (d = 0; d < hp->nc; d++)
	{
	  w = MAX (w, hp_a->x);
	  if (hp_a->direct > -2)
	    w = MAX (w, hp_a->x + (int)strlen (hp_a->c) * (6 + 2 * size_factor));
	  h = MAX (h, hp_a->y + 8);
	  x = MIN (x, hp_a->x);
	  if (hp_a->direct < 0)
	    x = MIN (x, hp_a->x - (int)strlen (hp_a->c) * (6 + 2 * size_factor));
	  y = MIN (y, hp_a->y - 8);
	  hp_a = hp_a->next;
	}

      hp_sp = sp_root.next;
      for (d = 0; d < hp->nsp; d++)
	{
	  w = MAX (w, hp_sp->x0);
	  h = MAX (h, hp_sp->y0);
	  w = MAX (w, hp_sp->x1);
	  h = MAX (h, hp_sp->y1);
	  w = MAX (w, hp_sp->x2);
	  h = MAX (h, hp_sp->y2);
	  w = MAX (w, hp_sp->x3);
	  h = MAX (h, hp_sp->y3);
	  x = MIN (x, hp_sp->x0);
	  y = MIN (y, hp_sp->y0);
	  x = MIN (x, hp_sp->x1);
	  y = MIN (y, hp_sp->y1);
	  x = MIN (x, hp_sp->x2);
	  y = MIN (y, hp_sp->y2);
	  x = MIN (x, hp_sp->x3);
	  y = MIN (y, hp_sp->y3);
	  hp_sp = hp_sp->next;
	}
      x = (int) (x * size_factor * 0.9);
      y = (int) (y * size_factor * 0.9);
      w = (int) (w * size_factor * 1.1 - x);
      h = (int) (h * size_factor * 1.1 - y);

      if (x < 0)
	x = 0;
      if (y < 0)
	y = 0;
    }
  bitmap = gdk_pixmap_new (drawing_area->window, 1, 1, 1);
  bitmapgc = gdk_gc_new (bitmap);
  buffer = XCreatePixmap (GDK_WINDOW_XDISPLAY(picture), GDK_WINDOW_XWINDOW(picture),
			  (unsigned int)w, (unsigned int)h, 1);
  XCopyPlane (GDK_WINDOW_XDISPLAY(picture), GDK_WINDOW_XWINDOW(picture), buffer,
	      GDK_GC_XGC(bitmapgc), x, y, (unsigned int)w, (unsigned int)h, 0, 0, 1);

  if ((int)strlen (filename))
    {
      snprintf (xfile,512, "%s", filename);
      retval = XWriteBitmapFile (GDK_DISPLAY (), xfile, buffer, 
      (unsigned int)w, (unsigned int)h, -1, -1);
      gdk_pixmap_unref (bitmap);
      gdk_gc_unref (bitmapgc);
      XFreePixmap (GDK_DISPLAY (), buffer);
      return (retval);
    }
  return (0);
}


void
xfig_line (FILE * fp, int x, int y, int tx, int ty)
/* writes a single line bond in XFig notation */
{
  fprintf (fp, "%i %i %i %i %i %i %i %i %i %.3f %i %i %i %i %i %i\n",
	   figline.object, figline.sub_type, figline.line_style,
	   figline.thickness, figline.pen_color, figline.fill_color,
	   figline.depth, figline.pen_style, figline.area_fill,
	   figline.style_val, figline.join_style, figline.cap_style,
	   figline.radius, figline.forward_arrow, figline.backward_arrow,
	   figline.npoints);
  fprintf (fp, "%i %i %i %i\n", x, y, tx, ty);
}

void
xfig_wedge (FILE * fp, int x, int y, int tx, int ty, int factor, int latex)
/* writes a wedge definition in XFig notation */
{
  struct data *hpc;
  struct xy_co *coord;
  int d, x1, y1, x2, y2;
  int bond_already_tuned;
  float area;

  x1 = tx - (int) (0.1 * (float) (ty - y));
  y1 = ty + (int) (0.1 * (float) (tx - x));
  x2 = tx + (int) (0.1 * (float) (ty - y));
  y2 = ty - (int) (0.1 * (float) (tx - x));
  hpc = da_root.next;
  bond_already_tuned = 0;
  for (d = 0; d < hp->n; d++)
    {
      if (hpc->bond == 10)
	{
	  if (
	      (abs (hpc->x * factor - tx) < 3
	       && abs (hpc->y * factor - ty) < 3)
	      || (abs (hpc->tx * factor - tx) < 3
		  && abs (hpc->ty * factor - ty) < 3))
	    {
	      coord =
		center_double_bond (hpc->x, hpc->y, hpc->tx, hpc->ty, db_dist);
	      if (abs (hpc->x * factor - tx) < 3
		  && abs (hpc->y * factor - ty) < 3)
		{
		  x1 = coord->x * factor;
		  y1 = coord->y * factor;
		  coord++;
		  x2 = coord->x * factor;
		  y2 = coord->y * factor;
		}
	      else
		{
		  x1 = coord->tx * factor;
		  y1 = coord->ty * factor;
		  coord++;
		  x2 = coord->tx * factor;
		  y2 = coord->ty * factor;
		}
	      area = 0.5 * abs (x * (y1 - y2)
				 + x1 * (y2 - y) + x2 * (y - y1));

	      if (fabs (area) < 76. * factor)
		{
		  x1 = tx - (int) (0.05 * (float) (ty - y));
		  y1 = ty + (int) (0.05 * (float) (tx - x));
		  x2 = tx + (int) (0.05 * (float) (ty - y));
		  y2 = ty - (int) (0.05 * (float) (tx - x));
		}
	      else bond_already_tuned = 1;

	    }
	}
      if (hpc->bond == 0 && !bond_already_tuned) {
        if ((abs (hpc->x * factor - tx) < 3 && abs (hpc->y * factor - ty) < 3)
          ||(abs (hpc->tx * factor - tx) < 3 && abs (hpc->ty * factor - ty) < 3))

        /* let the wedge smooth along a another bond */
	{
	  coord = intersect(x,y,x1,y1,hpc->x*factor,hpc->y*factor,
			    hpc->tx*factor,hpc->ty*factor);
	  coord->tx = coord->x;
	  coord->ty = coord->y;
	  coord = intersect(x,y,x2,y2,hpc->x*factor,hpc->y*factor,
			    hpc->tx*factor,hpc->ty*factor);
	  x1 = coord->tx; 
          y1 = coord->ty;
          x2 = coord->x;
          y2 = coord->y;

	  area = 0.5 * abs (x * (y1 - y2)
                                 + x1 * (y2 - y) + x2 * (y - y1));

          if (fabs (area) > 3300. * factor || fabs(area) < 1750. * factor)
            {
              x1 = tx - (int) (0.1 * (float) (ty - y));
              y1 = ty + (int) (0.1 * (float) (tx - x));
              x2 = tx + (int) (0.1 * (float) (ty - y));
              y2 = ty - (int) (0.1 * (float) (tx - x));
            }
        }
      } 
      hpc = hpc->next;
    }

  if (!latex)
    {
      fprintf (fp, "%i %i %i %i %i %i %i %i %i %f %i %i %i %i %i %i\n",
	       figline.object, figline.sub_type, figline.line_style,
	       figline.thickness, figline.pen_color, figline.fill_color,
	       figline.depth, figline.pen_style, figline.area_fill + 21,
	       figline.style_val, figline.join_style, figline.cap_style,
	       figline.radius, figline.forward_arrow, figline.backward_arrow,
	       figline.npoints + 1);
      fprintf (fp, "%i %i %i %i %i %i\n", x, y, x1, y1, x2, y2);
    }
  else
    {
      for (d = 0; d < 10; d++)
	xfig_line (fp, x, y, x1 + d * (x2 - x1) / 10,
		   y1 + d * (y2 - y1) / 10);
    }

}

void
xfig_wiggly (FILE * fp, int x, int y, int tx, int ty)
/* writes a wavy bond in XFig notation */
{
  int arccode = 5;
  int arctype = 1;
  int direction = 1;
  float xlen, ylen, veclen, boxlen;
  int boxcorx, boxcory;
  int i;
  float center_x, center_y;
  int xstart, ystart, xmid, ymid, xend, yend;
  xlen = (float)(tx - x);
  ylen = (float)(ty - y);

  veclen = (float)calc_vector (abs (tx - x), abs (ty - y));
  boxlen = 0.2 * veclen;
  boxcorx = (int) ((1. - xlen / veclen) * boxlen / 2);
  boxcory = (int) ((1. - ylen / veclen) * boxlen / 2);

  for (i = 0; i < 5; i++)
    {
      direction = abs (direction - 1);
      center_x = x + (0.2 * i + .1) * xlen;
      center_y = y + (0.2 * i + .1) * ylen;
      xstart = (int) (x + 0.2 * i * xlen);
      ystart = (int) (y + 0.2 * i * ylen);
      xmid = (int) (center_x - boxcorx * pow (-1, (double)i));
      ymid = (int) (center_y - boxcory * pow (-1, (double)i));
      if (x > tx)
	{
	  xmid = (int) (center_x + boxcorx * pow (-1, (double)i));
	  ymid = (int) (center_y + boxcory * pow (-1, (double)i));
	}
      xend = (int) (x + 0.2 * (i + 1) * xlen);
      yend = (int) (y + 0.2 * (i + 1) * ylen);

      fprintf (fp,
	       "%i %i %i %i %i %i %i %i %i %f %i %i %i %i %f %f %i %i %i %i %i %i\n",
	       arccode, arctype, figline.line_style, figline.thickness,
	       figline.pen_color, figline.fill_color, figline.depth,
	       figline.pen_style, figline.area_fill, figline.style_val,
	       figline.cap_style, direction, figline.forward_arrow,
	       figline.backward_arrow, center_x, center_y, xstart, ystart,
	       xmid, ymid, xend, yend);
    }
}

void
xfig_arrow (FILE * fp, int x, int y, int tx, int ty, int type, int latex)
/* writes an arrow in XFig notation */
{
  float xlen, ylen;
  float headfact;
  int xbase, ybase, xside, yside, xend, yend;
  int i;
  float veclen;

  xlen = (float)(tx - x);
  ylen = (float)(ty - y);
  headfact = 0.8;
  xbase = x + headfact * xlen;
  ybase = y + headfact * ylen;

if (select_char (tx,ty,1) != NULL){
	veclen = (float)calc_vector(tx-x,ty-y);
        xbase = x+headfact*xlen-10*xlen/veclen;
        ybase = y+headfact*ylen-10*ylen/veclen;
}

  xside = (int) (xbase + 0.05 * ylen);
  yside = (int) (ybase - 0.05 * xlen);
  xend = (int) (xbase - type * (0.05 * ylen));	/*type=0 (half arrow) ends on baseline, */
  yend = (int) (ybase + type * (0.05 * xlen));	/*type=1 is a symmetrical arrowhead */


  if (type == 1)
    {
      /* symmetrical arrow */
      fprintf (fp, "%i %i %i %i %i %i %i %i %i %f %i %i %i %i %i %i\n",
	       figline.object, figline.sub_type, figline.line_style,
	       figline.thickness, figline.pen_color, figline.fill_color,
	       figline.depth, figline.pen_style, figline.area_fill,
	       figline.style_val, figline.join_style, figline.cap_style,
	       figline.radius, figline.forward_arrow + 1,
	       figline.backward_arrow, figline.npoints);
      fprintf (fp, "%i %i %f %f %f\n",	/* arrow attributes */
	       1,		/* arrow_type (triangle, as before) */
	       1,		/* arrow_style (1 == filled) */
	       2.00,		/* arrow_thickness (1/80inch) */
	       72.00,		/* arrow_width (fig units (?) ) */
	       114.00		/* arrow_height (fig units) */
	);
      fprintf (fp, "%i %i %i %i\n", x, y, tx, ty);
    }

  else
    {
      /* half arrow */
      /* the line */
      fprintf (fp, "%i %i %i %i %i %i %i %i %i %f %i %i %i %i %i %i\n",
	       figline.object, figline.sub_type, figline.line_style,
	       figline.thickness, figline.pen_color, figline.fill_color,
	       figline.depth, figline.pen_style, figline.area_fill,
	       figline.style_val, figline.join_style, figline.cap_style,
	       figline.radius, figline.forward_arrow, figline.backward_arrow,
	       figline.npoints);
      fprintf (fp, "%i %i %i %i\n", x, y, tx, ty);

/* and the arrowhead - a filled polyline */
      if (!latex)
	{
	  fprintf (fp, "%i %i %i %i %i %i %i %i %i %f %i %i %i %i %i %i\n",
		   figline.object, figline.sub_type, figline.line_style,
		   figline.thickness, figline.pen_color, figline.fill_color,
		   figline.depth, figline.pen_style, figline.area_fill + 21,
		   figline.style_val, figline.join_style, figline.cap_style,
		   figline.radius, figline.forward_arrow,
		   figline.backward_arrow, figline.npoints + 1);
	  fprintf (fp, "%i %i %i %i %i %i\n", tx, ty, xside, yside, xend,
		   yend);
	}
      else
	{
	  for (i = 0; i < 10; i++)
	    xfig_line (fp, tx, ty, xend + i * (xside - xend) / 10,
		       yend + i * (yside - yend) / 10);
	}

    }
}

void
xfig_circle (FILE * fp, int x, int y, int tx, int ty)
{
  int radius;
  float start_ang = 0.;
  int direction = 1;
radius = calc_vector (abs (tx - x), abs (ty - y));
  fprintf (fp, "1 3 %i %i %i %i %i %i %i %.4f %i %.4f %i %i %i %i %i %i %i %i\n",
	   figline.line_style,
	   figline.thickness, figline.pen_color, figline.fill_color,
	   figline.depth, figline.pen_style, figline.area_fill,
	   figline.style_val, direction,
	   start_ang, x, y, radius, radius, tx, ty, tx, ty);
}

void
xfig_spline (FILE * fp, int x0, int y0, int x1, int y1, int x2, int y2,
	     int x3, int y3, int type, int latex)
/* draws a spline curve, optionally with full of half arrowhead  */
{
  int xlen, ylen, xbase, ybase, flag;
  double px0, py0, px1, py1;
  int i;
  double t;
  double dist;
  int xside, yside, xend, yend;

  figline.npoints = 21;
  if (type == -2)
    {
      figline.line_style = 2;
      figline.style_val = 4;
    }
  if (type == -1)
    {
      figline.area_fill = 20;
    }
/*  if (type == 1)*/
/*    figline.npoints = 15;*/

  /* the line */
  fprintf (fp, "%i %i %i %i %i %i %i %i %i %f %i %i %i %i %i %i\n",
	   figline.object, figline.sub_type, figline.line_style,
	   figline.thickness, figline.pen_color, figline.fill_color,
	   figline.depth, figline.pen_style, figline.area_fill,
	   figline.style_val, figline.join_style, figline.cap_style,
	   figline.radius, figline.forward_arrow, figline.backward_arrow,
	   figline.npoints);

  fprintf (fp, "%i %i\n", x0, y0);

  px0 = (double)x0;
  py0 = (double)y0;
  xbase = 0;
  ybase = 0;
  flag = 0;
  
  for (i = 0; i < figline.npoints -1 ; i++)
    {
      t = (float) i / (figline.npoints -2 );
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



      fprintf (fp, "%i %i\n", (int) px1, (int) py1);

      if (type >= 0) {
	dist = (x3-px1) * (x3-px1) + (y3-py1) * (y3-py1) ;
        if (( flag == 0) && ( i == 18 || (i >= 15 && dist < 30000.) ) )
	  {
	    xbase = (int)px1;
	    ybase = (int)py1;

	      if (dist >30000.) {
		xbase += (x3-xbase)/2;
		ybase += (y3-ybase)/2;
	      }

	  flag = 1;
	}
      }
    }

  figline.line_style = 0;
  figline.style_val = 0;
  figline.area_fill = -1;
  figline.npoints = 2;

  if (type <= 0)
    return;

  xlen = x3 - xbase;
  ylen = y3 - ybase;

  if (xlen != 0)
    xlen = (int) copysign (50., (double)xlen);
  if (ylen != 0)
    ylen = (int) copysign (50., (double)ylen);

  xside = (int) (xbase + 0.5 * ylen);
  yside = (int) (ybase - 0.5 * xlen);

  if (type == 1)
    {

      xend = (int) (xbase - 0.5 * ylen);
      yend = (int) (ybase + 0.5 * xlen);
      px0 = (double)((xside - x0) * (xside - x0) + (yside - y0) * (yside - y0));
      px1 = (double)((xend - x0) * (xend - x0) + (yend - y0) * (yend - y0));
      if (px1 > px0)
	{
	  xside = xend;
	  yside = yend;
	}
      xend = xbase;		/*on baseline */
      yend = ybase;
    }
  else
    {

      xend = (int) (xbase - 0.5 * ylen);
      yend = (int) (ybase + 0.5 * xlen);
    }

/* add the arrowhead - a filled polyline */
  if (!latex)
    {
      fprintf (fp, "%i %i %i %i %i %i %i %i %i %f %i %i %i %i %i %i\n",
	       figline.object, figline.sub_type, figline.line_style,
	       figline.thickness, figline.pen_color, figline.fill_color,
	       figline.depth, figline.pen_style, figline.area_fill + 21,
	       figline.style_val, figline.join_style, figline.cap_style,
	       figline.radius, figline.forward_arrow, figline.backward_arrow,
	       figline.npoints + 1);
      fprintf (fp, "%i %i %i %i %i %i\n", (int) x3, (int) y3, xside, yside,
	       xend, yend);
    }
  else
    {
      for (i = 0; i < 10; i++)
	xfig_line (fp, x3, y3, xend + i * (xside - xend) / 10,
		   yend + i * (yside - yend) / 10);
    }

}



int
exfig (FILE * fp, int latex_flag)
/* export the current molecule in the format of Brian Smith' XFig program */
{
  struct data *hp_b;
  struct dc *hp_a;
  struct spline *hp_sp;
  struct xy_co *coord;

  int i;
  float l;
  int d, x, y, tx, ty;
  int n;
  gchar *hpacc,*hpacc_p=NULL;
#ifdef GTK2
  int tw;
  static PangoLayout *thelayout;
  gchar hp_ac;
  gchar *ccx=malloc(10*sizeof(gchar));
#endif
  int ulx, uly, lrx, lry;
  int ha, chl = 0;
  int fontsize = 12, fzoom = 10;
  int xpos, offset, fontshrink;
  int shifted = 0;
  int nsub, nsup, savedpos;
  float csize;
  char symbol[30];
  int rval = 0;
  float charscale; /* scale textlength, width according to zoom factor */
  int len;
  float step;
//  const int myfontheight[7]={8,8,9,14,12,14,24};
  const int myfontheight[7]={8,8,9,14,16,14,24};

#ifdef GTK2
  setlocale(LC_NUMERIC,"C");
#endif
  
  savedpos = 0;
      charscale = 1.05;
      fzoom=16.;
      
    fzoom *= size_factor;
    
  if (!fprintf (fp, "#FIG 3.2\n"))
    return (1);
  fprintf (fp, "Portrait\n");
  fprintf (fp, "Center\n");
  fprintf (fp, "Metric\n");
  fprintf (fp, "A4\n");
  fprintf (fp, "100.00\n");
  fprintf (fp, "Single\n");
  fprintf (fp, "0\n");
  fprintf (fp, "# generated by Chemtool " VERSION "\n");
  fprintf (fp, "1200 2\n");

  figline.object = 2;
  figline.sub_type = 1;
  figline.line_style = 0;
  figline.thickness = 2;
  figline.pen_color = 0;
  figline.fill_color = 7;
  figline.depth = 100;
  figline.pen_style = 0;
  figline.area_fill = -1;
  figline.style_val = 0.000;
  figline.join_style = 0;
  figline.cap_style = 0;
  figline.radius = -1;
  figline.forward_arrow = 0;
  figline.backward_arrow = 0;
  figline.npoints = 2;

  hp_b = da_root.next;
  for (d = 0; d < hp->n; d++)
    {
      if (mark.flag && (hp_b->smarked + hp_b->tmarked) == 0)
	{
	}
      else
	{
	figline.pen_color = hp_b->color;
	figline.fill_color = hp_b->color;
	  coord = bond_cut (hp_b->x, hp_b->y, hp_b->tx, hp_b->ty, 12);
	if(hp_b->bond==11)  coord = bond_cut (hp_b->x, hp_b->y, hp_b->tx, hp_b->ty, 0);
#if 0
	if(hp_b->bond==5 || hp_b->bond==6)  coord = bond_cut (hp_b->x, hp_b->y, hp_b->tx, hp_b->ty, 15);
#endif	
	  x = coord->x * fzoom;
	  y = coord->y * fzoom;
	  tx = coord->tx * fzoom;
	  ty = coord->ty * fzoom;

	if ( !use_whiteout) {
        int siz,j;
        float lfactor=((float)calc_vector(abs(tx-x),abs(ty-y))/(832.*size_factor));
        if (lfactor >1.) lfactor=1.; /* short bonds should not shrink as much, they might vanish completely */
                                     /* extra long bonds, however, need not leave more space around labels */
		i=has_label(hp_b->x,hp_b->y);
		if (i>=0) {
                 hp_a = dac_root.next;
                 for (j = 0; j < i; j++)hp_a=hp_a->next;
		 siz=2*hp_a->size;
         	if (zoom_factor <2) siz-= 2-zoom_factor;
	        if (zoom_factor >2) siz+= zoom_factor-2;
	        if (siz <0) siz=0;
	        if (siz >6) siz=6;

                 if (y-ty==0&& hp_a->direct ==0) {
                   if (tx>x) {
                      int strl=0;
                      for (i=0;i<(int)strlen(hp_a->c);i++) {
                        if(hp_a->c[i] != '_' && hp_a->c[i] != '^' 
                        && hp_a->c[i] != '{' && hp_a->c[i] != '{')
                        strl++;
                      }
                      if (strl>1) x+=(strl+1)*fzoom*(siz+1);
                      if (hp_a->c[strlen(hp_a->c)-2]=='_') {
                        x-=fzoom/2*(siz+1);
                        }
                      if (hp_a->c[strlen(hp_a->c)-1]=='}')
                        x-=fzoom/2*(siz+1);
                   }     
//                   else
//                      x-=(strlen(hp_a->c)-1)*fzoom*(siz+1);
		 }
		int ox=x;
//		fprintf(stderr,"size_factor=%f bondl %d lfactor %f\n",size_factor,calc_vector(abs(tx-x),abs(ty-y)),lfactor);
				x += (lfactor*fzoom*(siz+1)/calc_vector(abs(tx-x),abs(ty-y))) *(tx-x);
				y += (lfactor*fzoom*(siz+1)/calc_vector(abs(tx-ox),abs(ty-y))) *(ty-y);
				}
		i=has_label(hp_b->tx,hp_b->ty);
		if (i>=0) {
                 hp_a = dac_root.next;
                 for (j = 0; j < i; j++)hp_a=hp_a->next;
		 siz=2*hp_a->size;
                 if (y-ty==0 ){
                   if ( hp_a->direct<-1) {
                      if (tx>x)
                        tx-=(strlen(hp_a->c)-1)*fzoom*(siz+1);
//                          else
//                       tx+=(strlen(hp_a->c)-1)*fzoom*(siz+1);
                   }
                   else if ( hp_a->direct==0) {
//                      if (tx>x)
  //                      tx-=(strlen(hp_a->c)-1)*fzoom*(siz+1);
    //                  else
 if (tx<x)                      tx+=(strlen(hp_a->c)-1)*fzoom*(siz+1);
                   }
                }  
		int otx=tx;
				tx -= (lfactor*fzoom*(siz+1)/calc_vector(abs(tx-x),abs(ty-y))) *(tx-x);
				ty -= (lfactor*fzoom*(siz+1)/calc_vector(abs(otx-x),abs(ty-y))) *(ty-y);
				}
		}		

	  if (!hp_b->bond)
	    xfig_line (fp, x, y, tx, ty);
	  if (hp_b->bond == 5)
	    {
	      xfig_wedge (fp, x, y, tx, ty, fzoom, latex_flag);
	    }
	  if (hp_b->bond == 6)
	    {
		len=(int) (calc_vector(abs(tx-x),abs(ty-y))/(8.*fzoom));
		if (len <8 ) len=8;
		step=1./len;
	      for (i = 1; i < len; i=i+1)
		xfig_line (fp, (int) (x + step * i * (tx - x) - 0.01 * (ty - y) * i),
			   (int) (y + step * i * (ty - y) + 0.01 * (tx - x) * i),
			   (int) (x + step * i * (tx - x) + 0.01 * (ty - y) * i),
			   (int) (y + step * i * (ty - y) - 0.01 * (tx - x) * i));
	    }
	  if (hp_b->bond == 7)
	    xfig_wiggly (fp, x, y, tx, ty);
	  if (hp_b->bond == 8)
	    xfig_arrow (fp, x, y, tx, ty, 0, latex_flag);
	  if (hp_b->bond == 9)
	    xfig_arrow (fp, x, y, tx, ty, 1, latex_flag);
	  if (hp_b->bond == 11)
	    xfig_circle (fp, x, y, tx, ty);
	  if (hp_b->bond == 10)
	    {
	      figline.thickness = 6;	/* 4 ? */
	      xfig_line (fp, x, y, tx, ty);
	      figline.thickness = 2;	/*  1 ? */
	    }
	  if (hp_b->bond == 12)
	    {
	      figline.line_style = 2;
	      figline.style_val = 4;
	      xfig_line (fp, x, y, tx, ty);
	      figline.line_style = 0;
	      figline.style_val = 0;
	    }
	  if (hp_b->bond == 13)
	    {
	      figline.thickness = 6;
	      figline.pen_color = 7;
	      figline.depth = 85;
	      xfig_line (fp, x + (tx - x) / 8., y + (ty - y) / 8.,
			 tx - (tx - x) / 8., ty - (ty - y) / 8.);
	      figline.thickness = 2;
	      figline.pen_color = 0;
	      figline.depth = 80;
	      xfig_line (fp, x, y, tx, ty);
	      figline.depth = 100;
	    }
	  if (hp_b->bond == 4)
	    {
	      coord = center_double_bond (x, y, tx, ty, db_dist * fzoom);
	      xfig_line (fp, coord->x, coord->y, coord->tx, coord->ty);
	      coord++;
	      xfig_line (fp, coord->x, coord->y, coord->tx, coord->ty);
	    }
	  if (hp_b->bond == 1 || hp_b->bond == 3)
	    {
	      coord = multi_bonds (x, y, tx, ty, mb_dist * fzoom);
	      xfig_line (fp, x, y, tx, ty);
	      xfig_line (fp, coord->x, coord->y, coord->tx, coord->ty);
	    }
	  if (hp_b->bond == 2 || hp_b->bond == 3)
	    {
	      coord = multi_bonds (tx, ty, x, y, mb_dist * fzoom);
	      xfig_line (fp, x, y, tx, ty);
	      xfig_line (fp, coord->x, coord->y, coord->tx, coord->ty);
	    }
	  if (hp_b->bond == 14)
	    {
	      coord = multi_bonds (x, y, tx, ty, mb_dist * fzoom);
	      xfig_line (fp, x, y, tx, ty);
	      figline.line_style = 1;
	      figline.style_val = 2.;
	      xfig_line (fp, coord->x, coord->y, coord->tx, coord->ty);
	      figline.line_style = 0;
	      figline.style_val = 0.;
	    }
	  if (hp_b->bond == 15)
	    {
	      coord = multi_bonds (tx, ty, x, y, mb_dist * fzoom);
	      xfig_line (fp, x, y, tx, ty);
	      figline.line_style = 1;
	      figline.style_val = 2.;
	      xfig_line (fp, coord->x, coord->y, coord->tx, coord->ty);
	      figline.line_style = 0;
	      figline.style_val = 0.;
	    }
	  if (hp_b->bond == 16)
	    {
	      coord = center_double_bond (x, y, tx, ty, db_dist * fzoom);
	      x = coord->x;
	      y = coord->y;
	      tx = coord->tx;
	      ty = coord->ty;
	      coord++;
	      for (i = 0; i < 10; i++)
		xfig_line (fp, (int) (x + 0.1 * i * (tx - x)),
			   (int) (y + 0.1 * i * (ty - y)),
			   (int) (coord->x + 0.1 * i * (coord->tx - coord->x)),
			   (int) (coord->y + 0.1 * i * (coord->ty - coord->y)));
	    }
	    
	  if (hp_b->bond == 18)
	    {
	    xfig_line (fp, x, y, tx, ty);
	      coord = center_double_bond (x, y, tx, ty, (db_dist+1) * fzoom);
	      xfig_line (fp, coord->x, coord->y, coord->tx, coord->ty);
	      coord++;
	      xfig_line (fp, coord->x, coord->y, coord->tx, coord->ty);
	    }

	  if (hp_b->bond == 19)
	    {
		int tmpx1,tmpx2,tmpy1,tmpy2,tmptx1,tmptx2,tmpty1,tmpty2;
	      coord = center_double_bond (x, y, tx, ty, (db_dist+2) * fzoom);
	      tmpx1=coord->x;
	      tmpy1=coord->y;
	      tmptx1=coord->tx;
	      tmpty1=coord->ty;
	      coord++;
	      tmpx2=coord->x;
	      tmpy2=coord->y;
	      tmptx2=coord->tx;
	      tmpty2=coord->ty;
	      coord = center_double_bond (tmpx1,tmpy1,tmptx1,tmpty1,(db_dist-1)*fzoom);
	      xfig_line (fp, coord->x, coord->y, coord->tx, coord->ty);
	      coord++;
	      xfig_line (fp, coord->x, coord->y, coord->tx, coord->ty);
	      coord = center_double_bond (tmpx2,tmpy2,tmptx2,tmpty2,(db_dist-1)*fzoom);
	      xfig_line (fp, coord->x, coord->y, coord->tx, coord->ty);
	      coord++;
	      xfig_line (fp, coord->x, coord->y, coord->tx, coord->ty);
	    }
/*      hp_b = hp_b->next;*/
	}
      hp_b = hp_b->next;
    }
  figtext.object = 4;
  figtext.sub_type = 0;
  figtext.color = 0;
  figtext.depth = 90;
  figtext.pen_style = 0;
  figtext.font = 16;		/*16=helvetica*,12=courier */
  figtext.font_size = 10;
  figtext.angle = 0.000;
  if (!latex_flag)
  figtext.font_flags = 4; 
  else
  figtext.font_flags = 6;
  figtext.height = 180. *charscale;
  figtext.length = 170. *charscale;

  hp_a = dac_root.next;
  for (d = 0; d < hp->nc; d++)
    {
      if (mark.flag && hp_a->marked == 0)
	{
	}
      else
	{
	fontsize=hp_a->size;
	if (zoom_factor <2) fontsize-= 2-zoom_factor;
	if (zoom_factor >2) fontsize+= zoom_factor-2;
	if (fontsize <0) fontsize=0;
	if (fontsize >6) fontsize=6;
  switch (fontsize)
    {
    case 0:
      figtext.font_size = 8;		/*8 */
      charscale=1.3;
      break;
    case 1:
      figtext.font_size = 10;		/*6? */
      charscale = 1.25;
      break;
    case 2:
      figtext.font_size = 12;		/*9? */
      charscale = 0.6;
      break;
    case 3:
      figtext.font_size = 16;		/*12? */
      charscale = 0.75;
      break;
    case 4:
      figtext.font_size = 18;
      charscale = 0.6;
      break;
    case 5:
      figtext.font_size = 20;
      charscale = 0.4;
      break;
    case 6:
      figtext.font_size = 24;
      charscale = 0.2;
      break;
    default:
      fprintf (stderr, "no figfont parameters for fontsize %d\n",
	       fontsize);
      rval = 2;
    }

  switch (zoom_factor)
    {
    case 0:
      charscale=0.95;
      break;
    case 1:
      charscale = 0.80;
      break;
    case 2:
      charscale = 0.95;
      break;
    case 3:
      charscale = 0.85;
      break;
    case 4:
      charscale = 0.80;
      break;
}    
/*    fzoom = size_factor * 16. ;*/ /* 75dpi screen to 1200dpi FIG */

float cfzoom = fzoom * charscale / size_factor;

#ifdef GTK2
  if (!thelayout) thelayout = gtk_widget_create_pango_layout(drawing_area,"X");
              pango_layout_set_text(thelayout, "X", -1);
              pango_layout_set_font_description(thelayout,symbfont[fontsize]);
              pango_layout_get_pixel_size(thelayout, NULL, &ha);
//fprintf(stderr,"gtk pixel height %d\n",ha);
figtext.height=8*ha;
		ha = ha/100 *fzoom;
#else		
	ha=fzoom * gdk_char_height(font[fontsize],(gchar)'X');
figtext.height=8*ha;
#endif
int hax=0;
#ifdef GTK2
              pango_layout_get_pixel_size(thelayout, NULL, &hax);
#endif
	ha=(int)(fzoom/2. * myfontheight[fontsize]);
//	fprintf(stderr,"fixed ha=%d, calculated %d\n",ha,hax);
//	figtext.height = 180. *charscale;
//	figtext.length = 170. *charscale;
	figtext.color = hp_a->color;
	if (hp_a->font == 0)
		figtext.font = 16; /* Helvetica */
	else
		figtext.font = 0; /* Times */	
	  figtext.sub_type = abs (hp_a->direct);
	  switch (hp_a->direct)
	    {
	    case Middle_Text:
	      chl = 0;
	      break;
	    case Left_Text:
	      chl = 9 * fzoom;
	      break;
	    case Right_Text:
	      chl = -9 * fzoom;
	      break;
	    default:
	      fprintf (stderr, "undefined text direction in xfig output\n");
	      rval = 3;
	      ;;
	    }
          hpacc=strdup(hp_a->c);
	  nsub = 0;
	  nsup = 0;
	  if ( (int)strlen(hpacc) != g_utf8_strlen(hpacc,-1) ) {
	     gchar *q = g_strdup(hpacc);
	     gchar *c,*cc;
	     gchar *t = g_strdup(hpacc);
	     gsize pos,out;
	     long symb;
	     c=g_convert(hpacc,-1,"ISO8859-1","UTF-8",&pos,&out,NULL);
             if (!c) {
               q[0]='\0';
               do {
	       c=g_convert(t,-1,"ISO8859-1","UTF-8",&pos,&out,NULL);
	       if (!c) {
               cc=t+(long)pos;
               symb=g_utf8_get_char(cc);
               *cc='\0';
               strcat(q,t);
               switch(symb)  { 
                 case 923:
                   strcat(q,"@L");
                   break;
                 case 915:
                   strcat(q,"@G");
                   break;
                 case 916:
                 case 8710:
                   strcat(q,"@D");
                   break;
                 case 945:
                  strcat(q,"@a");
                  break;
                 case 946:
                  strcat(q,"@b");
                  break;
                 case 947:
                  strcat(q,"@g");
                  break;
                 case 948:
                  strcat(q,"@d");
                  break;
                 case 949:
                  strcat(q,"@e");
                 case 966:
                  strcat(q,"@f");
                  break;
                 case 967:
                  strcat(q,"@g");
                  break;
                 case 954:
                  strcat(q,"@k");
                  break;
                 case 955:
                  strcat(q,"@l");
                  break;
                 case 956:
                  strcat(q,"@m");
                  break;
                 case 957:
                  strcat(q,"@n");
                  break;
                 case 960:
                  strcat(q,"@p");
                  break;
                 case 961:
                  strcat(q,"@r");
                  break;
                 case 963:
                  strcat(q,"@s");
                  break;
                 case 964:
                  strcat(q,"@t");
                  break;
                 case 8853:
                  strcat(q,"@+");
                  break;
                 case 8854:
                  strcat(q,"@-");
                  break;
                 case 8901:
                  strcat(q,"@*");
                  break;
                default:
                  break;
               }
               *cc=' ';
               t=g_utf8_find_next_char(cc,NULL);
               }
               } while(!c);
               strcat(q,t);
               strcpy(hpacc,q);
             } 
        }
	  for (i = 0; i < (int)strlen (hpacc); ++i)
	    if (hpacc[i] == '\\')
	      hpacc[i] = ' ';


	  if (!strpbrk (hpacc, "@_^|#"))
	    {			/* no sub- or superscripts - can write entire label in one go */

              /* calculate start position depending on justification */
	      xpos = hp_a->x * fzoom;
	      csize =  (fontsize * fzoom /* * 1.4*/);
	      if (chl < 0)
		xpos -= (int)strlen (hpacc) * csize;
	      if (chl == 0)
		xpos -= (int)strlen (hpacc) * csize / 2;
	      ulx = xpos;
	      if (chl > 0)
		ulx = xpos - csize / 2;
	      /* if (chl == 0) ulx = xpos+ csize/2; */
	      if (chl < 0)
		ulx = xpos + csize;
	      uly = hp_a->y * fzoom - ha;
	      lrx = xpos + (int)strlen (hpacc) * csize - csize / 2;
		if (chl == 0) lrx += csize/2;
	      if (chl < 0)
		lrx = lrx + csize;
	      lry = hp_a->y * fzoom + ha;
	      
          /* draw compound object with white box under the text if desired */
	if (use_whiteout == 1) fprintf (fp, " 6 %i %i %i %i\n", ulx, uly, lrx, lry);
	
	      if (use_whiteout == 1 && !latex_flag)
		{
		  fprintf (fp, " 2 2 0 1 7 7 95 0 20 1 0 0 5 0 0 5\n");
		  fprintf (fp, "%i %i\n", ulx, uly);
		  fprintf (fp, "%i %i\n", lrx, uly);
		  fprintf (fp, "%i %i\n", lrx, lry);
		  fprintf (fp, "%i %i\n", ulx, lry);
		  fprintf (fp, "%i %i\n", ulx, uly);
		}

	      if (latex_flag) /* convert special shortcut characters to LaTeX commands */
		{
		int ii,jj;
		char latexstr[255];
		jj=0;
		for (ii=0;ii<(int)strlen(hpacc);ii++){
		if (hpacc[ii] == '°' ){
			    latexstr[jj++]='\0';
			    strcat(latexstr,"$^{\\\\circ}$");
			    jj += 10;
			    continue;
		}
		if (hpacc[ii] == (char)169 ){
			    latexstr[jj++]='\0';
			    strcat(latexstr,"$\\\\copyright$");
			    jj += 12;
			    continue;
		}
		if (hpacc[ii] == (char)174 ){
			    latexstr[jj++]='\0';
			    strcat(latexstr,"$\\\\textregistered$");
			    jj += 17;
			    continue;
		}
		if (hpacc[ii] == (char)177 ){
			    latexstr[jj++]='\0';
			    strcat(latexstr,"$\\\\pm$");
			    jj += 5;
			    continue;
		}
		if (hpacc[ii]=='%' || hpacc[ii]=='$') {
			latexstr[jj++]='\\';
			latexstr[jj++]='\\';
			}
		latexstr[jj++]=hpacc[ii];
		}
		latexstr[jj]='\0';

		  fprintf (fp,
			   "%i %i %i %i %i %i %d %.4f %i %f %f %i %i %s\\001\n",
			   figtext.object, figtext.sub_type, figtext.color,
			   figtext.depth, figtext.pen_style, figtext.font,
			   figtext.font_size, figtext.angle,
			   figtext.font_flags, figtext.height,
			   figtext.length * (int)strlen (latexstr),
			   hp_a->x * fzoom - chl, hp_a->y * fzoom + ha,
			   latexstr);
		}
	      else
		{
//{
float tlen=0.;
int cw;
#ifdef GTK2
gchar *c,*t;
gsize pos,out;
      c=g_convert(hpacc,-1,"ISO8859-1","UTF-8",&pos,&out,NULL);
        if (!c) {
        t=hpacc+(long)pos;
      c=g_convert_with_fallback(hpacc,-1,"ISO8859-1","UTF-8","?",NULL,NULL,NULL);
        }
#else
char *c = strdup(hpacc);		
#endif

#ifdef GTK2
gchar *cx;
  cx=g_locale_to_utf8(hpacc,-1,NULL,NULL,NULL);
  if (!cx) cx = g_convert(hpacc,-1,"UTF-8","ISO8859-1",NULL,NULL,NULL);
        if (!cx) {
          fprintf(stderr,"invalid character cx in chemtool file\n");
          return 1;
              }
  if (!thelayout) thelayout = gtk_widget_create_pango_layout(drawing_area,cx);
              pango_layout_set_text(thelayout, cx, -1);
              pango_layout_set_font_description(thelayout,font[fontsize]);
              pango_layout_get_pixel_size(thelayout, &cw, NULL);
tlen =cw;
#else
for (k=0;k<(int)strlen(hpacc);k++){
cw=gdk_char_width(font[fontsize],(gchar)hpacc[k]);
tlen+=1.3*cw;
if (hpacc[k]==' ') tlen+=1.2*cw;
}
#endif
		  fprintf (fp,
			   "%i %i %i %i %i %i %d %.4f %i %.2f %.2f %i %i %s\\001\n",
			   figtext.object, figtext.sub_type, figtext.color,
			   figtext.depth, figtext.pen_style, figtext.font,
			   figtext.font_size, figtext.angle,
			   figtext.font_flags, figtext.height,
//			   figtext.length * (int)strlen (hp_a->c),
                           11.4*tlen,
			   (int)(hp_a->x * fzoom - chl ),
			   hp_a->y * fzoom + ha,
			   c);
		}
	      if (use_whiteout==1) fprintf (fp, "-6\n");	/* close block (around whiteout and label) */
	    }

	  else
	    {			/* special formatting needed, every character becomes a separate entity */
	      figtext.sub_type = 1; /* have xfig center characters at the calculated position */
	      l = 0;
#ifdef GTK2
  if (!thelayout) thelayout = gtk_widget_create_pango_layout(drawing_area,hpacc);
  pango_layout_set_font_description(thelayout,font[fontsize]);
              n = (int) g_utf8_strlen(hpacc,-1);
#else
              n = (int)strlen (hpacc);
#endif
              hpacc_p=hpacc;
/* first loop - determine approximate length of label after processing */
/* for sizing the compound box and optional white rectangle under text */
	      for (i = 0; i < n; i++)
		{
#ifdef GTK2
                        if (i>0) hpacc=g_utf8_find_next_char(hpacc,NULL);
                        if (!hpacc) break;
                        g_utf8_strncpy(&hp_ac,hpacc,1);
#else
              hp_ac= hpacc[i];
#endif
		  switch (hp_ac)
		    {
		    case '@':
		    i++;
			if (hpacc[i]=='}' || hpacc[i]=='{' ) i++;
#ifdef GTK2
              if (i>0) hpacc=g_utf8_find_next_char(hpacc,NULL);
              if (!hpacc) break;
              g_utf8_strncpy(ccx,hpacc,1);
              pango_layout_set_text(thelayout, ccx, -1);
              pango_layout_set_font_description(thelayout,symbfont[fontsize]);
              pango_layout_get_pixel_size(thelayout, &tw, NULL);
		l+= tw*cfzoom;
#else		
		    l+=gdk_char_width(symbfont[fontsize],hpacc[i])*fzoom;
#endif
		      break;
		    case '#':
		    i++;
			if (hpacc[i]=='}' || hpacc[i]=='{' ) i++;
#ifdef GTK2
              if (i>0) hpacc=g_utf8_find_next_char(hpacc,NULL);
              if (!hpacc) break;
              hp_ac= g_utf8_get_char(hpacc);
              g_utf8_strncpy(ccx,hpacc,1);
              pango_layout_set_text(thelayout, ccx, -1);
              pango_layout_set_font_description(thelayout,boldfont[fontsize]);
              pango_layout_get_pixel_size(thelayout, &tw, NULL);
		l+= tw/2.*cfzoom;
#else
		    l+=gdk_char_width(boldfont[fontsize],hpacc[i])*fzoom;
#endif
		      break;
		    case '|':
		    i++;
#if !defined GTK2 
    			if (hpacc[i]=='}' || hpacc[i]=='{' ) i++;
#else
                        hpacc=g_utf8_find_next_char(hpacc,NULL);
                        if (!hpacc) break;
                        hp_ac= g_utf8_get_char(hpacc);
                        if (hp_ac=='}'|| hp_ac == '{') { 
                          i++;
                          hpacc=g_utf8_find_next_char(hpacc,NULL);
                          if (!hpacc) break;
                          hp_ac= g_utf8_get_char(hpacc);
                        }
#endif                        
#ifdef GTK2
              g_utf8_strncpy(ccx,hpacc,1);
              pango_layout_set_text(thelayout, ccx, -1);
              pango_layout_set_font_description(thelayout,slfont[fontsize]);
              pango_layout_get_pixel_size(thelayout, &tw, NULL);
		l+=tw/2.*cfzoom;
#else
		    l+=gdk_char_width(slfont[fontsize],hpacc[i])*fzoom;
#endif
		      break;
		    case '}':
		    case '{':
		      break;
		    case '_':
		    	nsub=1;
   			i++;
#if !defined GTK2 
    			if (hpacc[i]=='}' || hpacc[i]=='{' ) i++;
#else
                        hpacc=g_utf8_find_next_char(hpacc,NULL);
                        if (!hpacc) break;
                        hp_ac= g_utf8_get_char(hpacc);
                        if (hp_ac=='}'|| hp_ac == '{') { 
                          i++;
                          hpacc=g_utf8_find_next_char(hpacc,NULL);
                          if (!hpacc) break;
                          hp_ac= g_utf8_get_char(hpacc);
                        }
#endif                        
#ifdef GTK2
              g_utf8_strncpy(ccx,hpacc,1);
              pango_layout_set_text(thelayout, ccx, -1);
                        pango_layout_set_font_description(thelayout,smallfont[fontsize]);
                        pango_layout_get_pixel_size(thelayout, &tw, NULL);
                        l+=tw*cfzoom;
#else
                        l+=gdk_char_width(smallfont[fontsize],hpacc[i])*fzoom;
#endif
		      break;
		    case '^':
		    	nsup=1;
		    i++;
			if (hpacc[i]=='}' || hpacc[i]=='{' ) i++;
#ifdef GTK2
              hpacc=g_utf8_find_next_char(hpacc,NULL);
              if (!hpacc) break;
              hp_ac= g_utf8_get_char(hpacc);
              g_utf8_strncpy(ccx,hpacc,1);
              pango_layout_set_text(thelayout, ccx, -1);
                        pango_layout_set_font_description(thelayout,smallfont[fontsize]);
                        pango_layout_get_pixel_size(thelayout, &tw, NULL);
                        l+=tw*cfzoom;
#else
		        l+=gdk_char_width(smallfont[fontsize],hpacc[i])*fzoom;
#endif
		      break;
		    default:
#ifdef GTK2
              g_utf8_strncpy(ccx,hpacc,1);
              pango_layout_set_text(thelayout, ccx, -1);
              pango_layout_set_font_description(thelayout,font[fontsize]);
              pango_layout_get_pixel_size(thelayout, &tw, NULL);
		l+=tw*cfzoom;
#else
                l+=gdk_char_width(font[fontsize],hpacc[i])*fzoom;
#endif

		    }
		}

	      xpos = hp_a->x * fzoom ;
#ifdef GTK2
              g_utf8_strncpy(ccx,hpacc_p,1);
              pango_layout_set_text(thelayout, ccx, -1);
              pango_layout_set_font_description(thelayout,font[fontsize]);
              pango_layout_get_pixel_size(thelayout, &tw, NULL);
		csize= 1+tw *cfzoom;
#else
		csize = gdk_char_width(font[fontsize],hpacc[0])*fzoom*2;
#endif

	      if (chl < 0) {
                xpos = hp_a->x * fzoom - (l-csize);
              }
              
	      if (chl == 0)
		xpos = hp_a->x * fzoom - l / 2;
/* glue text into compound for easier editing in xfig */
	      ulx = xpos -csize/2;
	      uly = hp_a->y * fzoom - ha;
	      lrx = xpos + l ;
	      if (chl < 0)
		lrx = xpos + (l - 1);
	      lry = hp_a->y * fzoom + ha;
	      if (nsup != 0) uly -= ha; /* height adjustment for */
	      if (nsub != 0) lry += ha; /* sub- and superscript */
	      fprintf (fp, " 6 %i %i %i %i\n", ulx, uly, lrx, lry);
/*whiteout box*/
	      if (use_whiteout == 1 && !latex_flag)
		{
		  fprintf (fp, " 2 2 0 1 7 7 95 0 20 1 0 0 5 0 0 5\n");
		  fprintf (fp, "%i %i\n", ulx, uly);
		  fprintf (fp, "%i %i\n", lrx, uly);
		  fprintf (fp, "%i %i\n", lrx, lry);
		  fprintf (fp, "%i %i\n", ulx, lry);
		  fprintf (fp, "%i %i\n", ulx, uly);
		}
/* second loop - for each character, determine its width, draw it and advance the text position */

		nsub=nsup=0;
		hpacc=hpacc_p;

	      for (i = 0; i < n; i++)
		{
		csize=0.;
		  offset = 0;
		  fontshrink = 0;
		if (hp_a->font == 0)
		  figtext.font = 16;	/*Helvetica */
		  else
		  figtext.font = 0; /* Times */

               /*default: regular character */
#ifdef GTK2
          g_utf8_strncpy(&hp_ac,hpacc,1);
#else
              hp_ac= hpacc[i];
#endif

#ifdef GTK2
              if (i>0) hpacc=g_utf8_find_next_char(hpacc,NULL);
              if (!hpacc) {fprintf(stderr,"eos1\n");
              break;
              }
              hp_ac= g_utf8_get_char(hpacc);
              if (!hp_ac) {fprintf(stderr,"eos2\n");
              break;
              }
              g_utf8_strncpy(ccx,hpacc,1);
              pango_layout_set_text(thelayout, ccx, -1);
              pango_layout_set_font_description(thelayout,font[fontsize]);
              pango_layout_get_pixel_size(thelayout, &tw, NULL);

		csize= 1+ tw *cfzoom;      /* convert from screen font width to xfig character size */

#else
		csize = (1+gdk_char_width(font[fontsize],hp_ac)) * cfzoom;
#endif		  

		  if (shifted != 0)        /* if sub- or superscripting already in progress */
		    {
		      if (hp_ac == '}')  /* end of sub/superscript, reset flag and treat next character as normal */
			{
			  shifted = 0;
			  offset = 0;
			  i++;
			  if (i >= n) break;

#ifdef GTK2
              hpacc=g_utf8_find_next_char(hpacc,NULL);
              hp_ac= g_utf8_get_char(hpacc);
              g_utf8_strncpy(ccx,hpacc,1);
              pango_layout_set_text(thelayout, ccx, -1);
              pango_layout_set_font_description(thelayout,font[fontsize]);
              pango_layout_get_pixel_size(thelayout, &tw, NULL);
		csize= 1+ tw *cfzoom;
#else
		csize = (1+gdk_char_width(font[fontsize],hp_ac)) * fzoom;
#endif
			}
		      else            /* still sub- or superscripting */
			{       
			  offset = shifted;
#ifdef GTK2
              g_utf8_strncpy(ccx,hpacc,1);
              pango_layout_set_text(thelayout, ccx, -1);
              pango_layout_set_font_description(thelayout,smallfont[fontsize]);
              pango_layout_get_pixel_size(thelayout, &tw, NULL);
		csize= 1+ tw *cfzoom;
#else
		csize = (1+gdk_char_width(smallfont[fontsize],hp_ac)) * fzoom;
#endif
			}
		    }
		  
		  if (hp_ac == '_')
		    {
#ifdef GTK2
              hpacc=g_utf8_find_next_char(hpacc,NULL);
              hp_ac= g_utf8_get_char(hpacc);
              g_utf8_strncpy(ccx,hpacc,1);
              pango_layout_set_text(thelayout, ccx, -1);
              pango_layout_set_font_description(thelayout,smallfont[fontsize]);
              pango_layout_get_pixel_size(thelayout, &tw, NULL);
		csize= 1+ tw *cfzoom;
#else
		csize = (1+gdk_char_width(smallfont[fontsize],hpacc[i+1])) * fzoom;
#endif
		      offset = ha / 2;
		      nsub = nsub + 1;
		      if (nsub == 1)
			{
			  if (nsup > 0)
			    xpos = savedpos;
			  else
			    savedpos = xpos;
			}
		      nsup = 0;
		    }

		  if (hp_ac == '^')
		    {
		      offset = -ha;
/*		      offset = -ha / 2;*/
#ifdef GTK2
              hpacc=g_utf8_find_next_char(hpacc,NULL);
              hp_ac= g_utf8_get_char(hpacc);
              g_utf8_strncpy(ccx,hpacc,1);
              pango_layout_set_text(thelayout, ccx, -1);
              pango_layout_set_font_description(thelayout,smallfont[fontsize]);
              pango_layout_get_pixel_size(thelayout, &tw, NULL);
		csize= 1+ tw *cfzoom;
#else
		csize = (1+gdk_char_width(smallfont[fontsize],hpacc[i+1])) * fzoom;
#endif
		      nsup = nsup + 1;
		      if (nsup == 1)
			{
			  if (nsub > 0)
			    xpos = savedpos;
			  else
			    savedpos = xpos;
			}
		      nsub = 0;
		    }

		  if (offset != 0)
		    {
		      if (shifted == 0)
			{
			  i++;

			  if (hp_ac == '{')
			    {
			      shifted = offset;
			      i++;
#ifdef GTK2
              hpacc=g_utf8_find_next_char(hpacc,NULL);
              hp_ac= g_utf8_get_char(hpacc);
              g_utf8_strncpy(ccx,hpacc,1);
              pango_layout_set_text(thelayout, ccx, -1);
              pango_layout_set_font_description(thelayout,smallfont[fontsize]);
              pango_layout_get_pixel_size(thelayout, &tw, NULL);
		csize= 1+ tw *cfzoom;
#else
		csize = (1+gdk_char_width(smallfont[fontsize],hp_ac)) * fzoom;
#endif
			    }
			}

		      if (i >= n)
			break;
		      if (hp_ac != '-' && hp_ac != '+')
			fontshrink = 4;
		      offset = offset * 2;
/*			xpos -= csize*.75;*/
#if !defined(GTK2)			
xpos -= csize*.35;
#endif
		    }
		  else
		    {		/*reset sub- and superscript counters */
		      nsub = 0;
		      nsup = 0;
		    }

		  if (hp_ac == '@')
		    {
		      figtext.font = 32;
#ifdef GTK2
              hpacc=g_utf8_find_next_char(hpacc,NULL);
              hp_ac= g_utf8_get_char(hpacc);
              g_utf8_strncpy(ccx,hpacc,1);
              pango_layout_set_text(thelayout, ccx, -1);
              pango_layout_set_font_description(thelayout,symbfont[fontsize]);
              pango_layout_get_pixel_size(thelayout, &tw, NULL);
		csize= 1+ tw *cfzoom;
#else
		csize = (1+gdk_char_width(symbfont[fontsize],hpacc[i+1])) * fzoom;
		      i++;
#endif
		    }
		  if (hp_ac == '|')
		    {
			if (figtext.font <16)
			figtext.font = 1; /* Times Italic */
			else 
		      figtext.font = 17; /* Helvetica Oblique */
#ifdef GTK2
              hpacc=g_utf8_find_next_char(hpacc,NULL);
              hp_ac= g_utf8_get_char(hpacc);
              g_utf8_strncpy(ccx,hpacc,1);
              pango_layout_set_text(thelayout, ccx, -1);
              pango_layout_set_font_description(thelayout,slfont[fontsize]);
              pango_layout_get_pixel_size(thelayout, &tw, NULL);
		csize= 1+ tw *cfzoom;
#else
		csize = (1+2*gdk_char_width(slfont[fontsize],hpacc[i+1])) * fzoom;
#endif
		if (chl > 0) xpos-= csize;
		      i++;
		    }
		  if (hp_ac == '#')
		    { 
			if (figtext.font < 16)
			figtext.font = 2 ; /* Times Bold */
			else
		      figtext.font = 18; /* Helvetica Bold */
#ifdef GTK2
              hpacc=g_utf8_find_next_char(hpacc,NULL);
              hp_ac= g_utf8_get_char(hpacc);
              g_utf8_strncpy(ccx,hpacc,1);
              pango_layout_set_text(thelayout, ccx, -1);
              pango_layout_set_font_description(thelayout,boldfont[fontsize]);
              pango_layout_get_pixel_size(thelayout, &tw, NULL);
		csize= 1+ tw *cfzoom;
#else
		csize = (1+gdk_char_width(boldfont[fontsize],hpacc[i+1])) * fzoom;
#endif
		if (chl > 0) xpos-= csize;
		      i++;  
		    }

                  if (i>0)xpos += csize/2.; //advance to center of this character
                  
		  if (latex_flag)
		    {
#ifdef GTK2
		      if (figtext.font == 32  /* use TeX names for symbols */
			  || (unsigned char) hp_ac > 128 || hp_ac=='%' || hp_ac =='$'
			  || hp_ac=='+' || hp_ac=='-')
#else
		      if (figtext.font == 32  /* use TeX names for symbols */
			  || (unsigned char) hpacc[i] > 128 || hpacc[i]=='%' || hpacc[i]=='$'
			  || hpacc[i]=='+' || hpacc[i]=='-')
#endif
			{
#ifdef GTK2
			  switch ((unsigned char)hp_ac)
#else
			  switch ((unsigned char) hpacc[i])
#endif
			    {
			    case 'a':
			      strcpy (symbol, "$\\\\alpha$");
			      break;
			    case 'b':
			      strcpy (symbol, "$\\\\beta$");
			      break;
			    case 'c':
			      strcpy (symbol, "$\\\\chi$");
			      break;
			    case 'd':
			      strcpy (symbol, "$\\\\delta$");
			      break;
			    case 'e':
			      strcpy (symbol, "$\\\\varepsilon$");
			      break;
			    case 'f':
			      strcpy (symbol, "$\\\\phi$");
			      break;
			    case 'g':
			      strcpy (symbol, "$\\\\gamma$");
			      break;
			    case 'h':
			      strcpy (symbol, "$\\\\eta$");
			      break;
			    case 'i':
			      strcpy (symbol, "$\\\\iota$");
			      break;
			    case 'j':
			      strcpy (symbol, "$\\\\varphi$");
			      break;
			    case 'k':
			      strcpy (symbol, "$\\\\kappa$");
			      break;
			    case 'l':
			      strcpy (symbol, "$\\\\lambda$");
			      break;
			    case 'm':
			      strcpy (symbol, "$\\\\mu$");
			      break;
			    case 'n':
			      strcpy (symbol, "$\\\\nu$");
			      break;
			    case 'p':
			      strcpy (symbol, "$\\\\pi$");
			      break;
			    case 'q':
			      strcpy (symbol, "$\\\\theta$");
			      break;
			    case 'r':
			      strcpy (symbol, "$\\\\rho$");
			      break;
			    case 's':
			      strcpy (symbol, "$\\\\sigma$");
			      break;
			    case 't':
			      strcpy (symbol, "$\\\\tau$");
			      break;
			    case 'u':
			      strcpy (symbol, "$\\\\upsilon$");
			      break;
			    case 'v':
			      strcpy (symbol, "$\\\\varpi$");
			      break;
			    case 'w':
			      strcpy (symbol, "$\\\\omega$");
			      break;
			    case 'x':
			      strcpy (symbol, "$\\\\xi$");
			      break;
			    case 'y':
			      strcpy (symbol, "$\\\\psi$");
			      break;
			    case 'z':
			      strcpy (symbol, "$\\\\zeta$");
			      break;
			    case 'C':
			      strcpy (symbol, "X");
			      break;
			    case 'D':
			      strcpy (symbol, "$\\\\Delta$");
			      break;
			    case 'F':
			      strcpy (symbol, "$\\\\Phi$");
			      break;
			    case 'G':
			      strcpy (symbol, "$\\\\Gamma$");
			      break;
			    case 'J':
			      strcpy (symbol, "$\\\\vartheta$");
			      break;
			    case 'L':
			      strcpy (symbol, "$\\\\Lambda$");
			      break;
			    case 'P':
			      strcpy (symbol, "$\\\\Pi$");
			      break;
			    case 'Q':
			      strcpy (symbol, "$\\\\Theta$");
			      break;
			    case 'R':
			      strcpy (symbol, "P");
			      break;
			    case 'S':
			      strcpy (symbol, "$\\\\Sigma$");
			      break;
			    case 'U':
			      strcpy (symbol, "$\\\\Upsilon$");
			      break;
			    case 'V':
			      strcpy (symbol, "$\\\\varsigma$");
			      break;
			    case 'W':
			      strcpy (symbol, "$\\\\Omega$");
			      break;
			    case 'X':
			      strcpy (symbol, "$\\\\Xi$");
			      break;
			    case 'Y':
			      strcpy (symbol, "$\\\\Psi$");
			      break;
			    case (unsigned char) 173:
			      strcpy (symbol, "$\\\\uparrow$");
			      break;
			    case (unsigned char) 175:
			      strcpy (symbol, "$\\\\downarrow$");
			      break;
			    case (unsigned char) 174:
			      strcpy (symbol,"$\\\\textregistered$");
			      break;
			    case '%':
			      strcpy (symbol, "\\\\%");
			      break;
			    case '$':
			      strcpy (symbol, "\\\\$");
			      break;
			    case (unsigned char) 176:
			    	strcpy(symbol,
			    		"$^{\\\\circ}$");
			      break;
			    case (unsigned char) 177:
			    	strcpy(symbol,
			    		"$\\\\pm$");
			      break;
			    case (unsigned char) 169:
			    	strcpy(symbol,
			    		"$\\\\copyright$");
			      break;
			   case '+':
                              strcpy(symbol,"$\\oplus$");
                              break;   			    					       
			   case '-':
                              strcpy(symbol,"$\\ominus$");
                              break;   			    					       
			    default:
			      fprintf (stderr, "no translation for %d\n",
				       (unsigned char) hpacc[i]);
			      symbol[0] = hpacc[i];
			      symbol[1] = '\0';
			      break;
			    }
			  fprintf (fp,
				   "%i %i %i %i %i %i %d %.4f %i %f %f %i %i %s \\001\n",
				   figtext.object, figtext.sub_type,
				   figtext.color, figtext.depth,
				   figtext.pen_style, figtext.font,
				   figtext.font_size - fontshrink,
				   figtext.angle, figtext.font_flags,
				   figtext.height, figtext.length,
				   xpos,
				   hp_a->y * fzoom + ha + offset, symbol);
			}
		      else
			{
			  fprintf (fp,
				   "%i %i %i %i %i %i %d %.4f %i %f %f %i %i %c\\001\n",
				   figtext.object, figtext.sub_type,
				   figtext.color, figtext.depth,
				   figtext.pen_style, figtext.font,
				   figtext.font_size - fontshrink,
				   figtext.angle, figtext.font_flags,
				   figtext.height, figtext.length,
				   xpos,
				   hp_a->y * fzoom + ha + offset, hpacc[i]);
			}
                        }
		  else
		    {    /* not in LaTeX mode */
		      fprintf (fp,
			       "%i %i %i %i %i %i %d %.4f %i %f %f %i %i %c\\001\n",
			       figtext.object, figtext.sub_type,
			       figtext.color, figtext.depth,
			       figtext.pen_style, figtext.font,
			       figtext.font_size - fontshrink, figtext.angle,
			       figtext.font_flags, figtext.height,
			       (double)csize, xpos,
			       hp_a->y * fzoom + ha + offset, hp_ac);
			if ((hp_ac=='+' || hp_ac=='-') && figtext.font == 32) //ionic charge symbol: add its circle
			fprintf(fp,"1 3 0 2 0 7 50 -1 -1 0.000 1 0.0000 %i %i %i %i %i %i %i %i\n",
			xpos,hp_a->y*fzoom+ha/2-2+offset, (figtext.font_size-fontshrink)*6,      
                        (figtext.font_size-fontshrink)*6,xpos,hp_a->y*fzoom+ha/2-2+offset,	xpos+((figtext.font_size-fontshrink)*6), hp_a->y*fzoom+ha/2-2+offset);   
		    }

                xpos += csize/2.;  //advance from center to end of character box
		}		/*for i */

	      fprintf (fp, "-6\n");	/* close this compound */
	    }
#ifdef GTK2
              n = (int) g_utf8_strlen(hpacc,-1);
#else
              n = (int)strlen (hpacc);
#endif
	  for (i = 0; i < n; ++i)
	    if (hpacc[i] == ' ')
	      hpacc[i] = '\\';
	}
      if (hpacc_p) {
        free(hpacc_p);
        hpacc_p=NULL;
      }
      hp_a = hp_a->next;
    }
    free(ccx);
figline.pen_color = 0; /* reset after linedrawing */
  hp_sp = sp_root.next;
  for (d = 0; d < hp->nsp; d++)
    {
      if (mark.flag == 1 && hp_sp->marked == 0)
	{
	}
      else
	{
	figline.pen_color=hp_sp->color;
	figline.fill_color=hp_sp->color;
	  xfig_spline (fp, hp_sp->x0 * fzoom, hp_sp->y0 * fzoom,
		       hp_sp->x1 * fzoom, hp_sp->y1 * fzoom,
		       hp_sp->x2 * fzoom, hp_sp->y2 * fzoom,
		       hp_sp->x3 * fzoom, hp_sp->y3 * fzoom,
		       hp_sp->type, latex_flag);
	}
      hp_sp = hp_sp->next;
    }
  return (rval);
}

int
export_xfig (char *filename)
/* initiate exporting of the current molecule in plain XFig format */
{
  FILE *xfile;
  int rval;

  if ((int)strlen (filename))
    {
      xfile = fopen (filename, "w");
      if (!xfile)
	return (1);
      rval = exfig (xfile, 0);
      if (fclose (xfile) != 0)
	{
	  perror ("could not close fig file");
	  return (1);
	}
      return (rval);
    }
  return (1);
}

int
export_latex_pic (char *filename, float scale)
/* export molecule as a pictex file by writing a temporary XFig file and
   postprocessing that with fig2dev */
{
  char com[255];
  FILE *xfile;
  int rval;

  if (figversion == 0)
    return (1);			/* cannot export without fig2dev */

  if ((int)strlen (filename))
    {
      snprintf (com,255, "fig2dev -Lpictex -m %f > \"%s\"", scale, filename);
      xfile = popen (com, "w");
      if (!xfile)
	return (1);
      rval = exfig (xfile, 1);
      if (pclose (xfile) != 0)
	return (1);
      return (rval);
    }
  else
    return (1);
}

int
export_ps_pic (char *filename, float scale)
/* export molecule as a postscript file by writing a temporary XFig file and
   postprocessing that with fig2dev */
{
  char com[255];
  FILE *xfile;
  int rval;
  char *epso[4] = {" ","-A","-T","-C"};
  char intl[3];
  
#ifdef GTK2
  setlocale (LC_NUMERIC,"C");
#endif

  if (figversion == 0)
    return (1);			/* cannot export without fig2dev */

  if (use_intlchars)
     strcpy(intl,"-j");
  else
     strcpy(intl,"");
   
  if ((int)strlen (filename))
    {
      if (figversion == 1)
	snprintf (com,255, "fig2dev %s -Lps -m %f > \"%s\"", intl, scale, filename);
      else if (figversion == 2)
	snprintf (com,255, "fig2dev %s -L eps -m %f > \"%s\"", intl, scale, filename);
      else
	snprintf (com,255, "fig2dev %s -L eps %s -m %f -g \\%s> \"%s\"", intl,epso[epsoption], scale, bghexcolor, filename);
      xfile = popen (com, "w");
      if (!xfile)
	return (1);
      rval = exfig (xfile, 0);
      if (rval != 0) 
        return (rval);
      rval=pclose(xfile);
      if (rval != 0)
	return (1);
      else	
        return (rval);
    }
  else
    return (1);
}

int
export_png_pic (char *filename, float scale)
/* export molecule as a png file by writing a temporary XFig file and
   postprocessing that with fig2dev */
{
  char com[255];
  FILE *xfile;
  int rval;
  char intl[3];
  
#ifdef GTK2
  setlocale (LC_NUMERIC,"C");
#endif

  if (figversion == 0)
    return (1);			/* cannot export without fig2dev */

  if (use_intlchars)
     strcpy(intl,"-j");
  else
     strcpy(intl,"");
   
  if ((int)strlen (filename))
    {
      if (figversion == 1)
	snprintf (com,255, "fig2dev %s -Lpng -m %f > \"%s\"", intl, scale, filename);
      else if (figversion == 2) 
	snprintf (com,255, "fig2dev %s -L png -S 4 -F -m %f > \"%s\"", intl, scale, filename);
      else
	snprintf (com,255, "fig2dev %s -L png -g \\%s -S 4 -F -m %f > \"%s\"", intl, bghexcolor, scale, filename);
      xfile = popen (com, "w");
      if (!xfile)
	return (1);
      rval = exfig (xfile, 0);
      if (rval != 0) 
        return (rval);
      rval=pclose(xfile);
      if (rval != 0)
	return (1);
      else	
        return (rval);
    }
  else
    return (1);
}

int
print_ps_pic ()
/* Print molecule to a postscript printer by creating an XFig datastream and
    postprocessing that with fig2dev */
{
  char com[255];
  int rval;
  FILE *xfile;
  char intl[3];

#ifdef GTK2
  setlocale (LC_NUMERIC,"C");
#endif

  if (figversion == 0)
    return (1);			/* cannot print without fig2dev */

  if (use_intlchars)
     strcpy(intl,"-j");
  else
     strcpy(intl,"");

  if (figversion == 1)
    snprintf (com,255, "fig2dev %s -L ps -z %s -P %s -e -m %f  | %s%s",
	     intl,paper[papersize], orientation[orient], printscale,
	     printcommand[printcmd], queuename);
  else if (figversion == 2)
    snprintf (com,255, "fig2dev %s -L ps -z %s  %s -e -m %f  | %s%s",
	     intl,paper[papersize], orientation[orient], printscale,
	     printcommand[printcmd], queuename);
  else
    snprintf (com,255, "fig2dev %s -L ps -g \\%s -z %s  %s -e -m %f  | %s%s",
	     intl,bghexcolor,paper[papersize], orientation[orient], printscale,
	     printcommand[printcmd], queuename);
  xfile = popen (com, "w");
  if (!xfile)
    return (1);
  rval = exfig (xfile, 0);
      if (rval != 0) 
        return (rval);
      rval=pclose(xfile);
      if (rval != 0)
	return (1);
      else	
        return (rval);
}

int
export_fw (char *filename)
/* save file and postprocess it with cht */
{
  char com[255];
  char xfile[512];
  int rval;
  FILE *chtpipe;
  FILE *fp;
  int fd;

  if ((int)strlen (filename))
    {
      strcpy (xfile, "/tmp/chtXXXXXX");
/*@ignore@ splint does not recognize mkstemp */
      fd = mkstemp (xfile);
/*@end@*/
      if (fd == -1)
	return 1;
      snprintf (com,255, "cht %s", xfile);
      fp = fdopen (fd, "w");
      save_mol (fp, mark.flag);
      fclose (fp);
      chtpipe = popen (com, "r");
      if (!chtpipe) return (1);
      rval = fscanf (chtpipe, "%20s %20s %20s %64s", formula, weight, eweight,
	      compos);
      rval = pclose (chtpipe);
      unlink (xfile);
      return (rval);
    }
  else
    return (1);
}

int
readrc ()
{
  FILE *fp;
  int i;
  char key[20], value[100];
  char line[80];
  gchar *filename;
  char *epso[4] = { "None","EPSI","TIFFm","TIFFc" };

  filename = g_malloc (PATH_MAX + 1);
  filename =
    strncat (strncpy (filename, g_get_home_dir (), PATH_MAX), "/.chemtoolrc",
	     PATH_MAX);
  fp = fopen (filename, "r");
  if (!fp)
    {
      g_free (filename);
      return 1;
    }
  else
    {
      while (!feof (fp))
	{
	  if (fgets (line, (int)sizeof (line), fp)) {
	  i = sscanf (line, "%s %s", key, value);
	  if (i < 2)
	    continue;

	  if (!strcmp (key, "orientation"))
	    {
	      if (!strcmp (value, "portrait"))
		orient = 0;
	      else
		orient = 1;
	    }
	  if (!strcmp (key, "papersize"))
	    {
	      for (i = 0; i < 11; i++)
		if (!strcmp (value, paper[i]))
		  papersize = i;
	    }
	  if (!strcmp (key, "printcommand"))
	    {
	      for (i = 0; i < (int)strlen (value); i++)
		if (value[i] == '\\')
		  value[i] = ' ';
	      for (i = 0; i < 3; i++)
		if (!strcmp (value, printcommand[i]))
		  printcmd = i;
	    }
	  if (!strcmp (key, "printer"))
	    strcpy (queuename, value);
	  if (!strcmp (key, "printscale"))
	    printscale = atof (value);
	  if (!strcmp (key, "epsoption"))
	      for (i = 0; i < 4; i++) {
		if (!strcmp (value, epso[i]))
		  epsoption = i;
	    }
	  if (!strcmp (key, "whiteout"))
	      use_whiteout = atoi (value);  
	  if (!strcmp (key, "intlchars"))
	      use_intlchars = atoi (value);  
	  if (!strcmp (key, "datadir"))
	    strcpy (datadir, value);
	  if (!strcmp (key, "extension"))
	    strcpy (datamask, value);
	  if (!strcmp (key, "bondlength")){
	    bondlen_mm= atof(value);
	    if (bondlen_mm != 0 && bondlen_mm != 10.668) size_factor*=bondlen_mm/10.668;
	    }
	  if (!strcmp (key, "double_separation")){
	    db_dist = atoi(value);
	    if (db_dist <= 0 || db_dist >100) db_dist=4;
	    mb_dist = (int) ( ( (float)db_dist )* 2.5 ); 
	    }
	  if (!strcmp (key, "background")){
	  	i=sscanf(value,"(%d,%d,%d)",&bgred,&bggreen,&bgblue);
	  	if (i<3) bgred=bggreen=bgblue=65535;
	  	}
	  }
	}
      fclose (fp);
      g_free (filename);
      return 0;
    }
}

int
writerc ()
{
  FILE *fp;
  char *pcomm[4] = { "lpr\\-P", "lp\\-d", "kprinter -d","gtklp -d"};
  char *ori[2] = { "portrait", "landscape" };
  char *epso[4] = { "None","EPSI","TIFFm","TIFFc" };
  gchar *filename;

  filename = g_malloc (PATH_MAX + 1);
  filename =
    strncat (strncpy (filename, g_get_home_dir (), PATH_MAX), "/.chemtoolrc",
	     PATH_MAX);
  fp = fopen (filename, "w");
  if (!fp)
    {
      g_free (filename);
      return 1;
    }
  else
    {
      fprintf (fp, "datadir %s\n", datadir);
      fprintf (fp, "extension %s\n", datamask);
      fprintf (fp, "papersize %s\n", paper[papersize]);
      fprintf (fp, "orientation %s\n", ori[orient]);
      fprintf (fp, "printscale %f\n", printscale);
      fprintf (fp, "printcommand %s\n", pcomm[printcmd]);
      fprintf (fp, "printer %s\n", queuename);
      fprintf (fp, "bondlength %6.4f\n", bondlen_mm);
      fprintf (fp, "double_separation %d\n", db_dist);
      fprintf (fp, "epsoption %s\n", epso[epsoption]);
      fprintf (fp, "whiteout %d\n", use_whiteout);
      fprintf (fp, "intlchars %d\n", use_intlchars);
      fprintf (fp, "background (%d,%d,%d)\n", bgred,bggreen,bgblue);
      fclose (fp);
      g_free (filename);
      return 0;
    }
}

void
check_fig2dev ()
{
  char cmd[20];
  FILE *xfile;
  float version;
  float ref=3.2;
  int pl;
  int rval;

#ifdef GTK2
  setlocale (LC_NUMERIC,"C");
#endif

  snprintf (cmd,20, "fig2dev -V");
  xfile = popen (cmd, "r");
  rval = fscanf (xfile, "%*s %*s %f %*s %d", &version, &pl);
  rval = pclose (xfile);
  if (rval != 0)
    figversion = 0;		/* fig2dev not found */
  else
    {
      if (version < ref || (version == ref && pl < 3))
	figversion = 1;		/* "old" fig2dev, needs -Lps */
      else
	figversion = 2;		/* new fig2dev, knows -L eps */
    if (version > ref || (version == ref && pl >3))
        figversion = 3;		/* 3.2.4 can generate previews in eps file */
     }	
}
void
check_babel ()
{
  char cmd[20];
  char data[80];
    FILE *xfile;
/*  float version;*/
  int rval;
  int start=0;
  int obmajor=0,obminor=0,obrel=0;
  
  babelin=-1;
  babelout=-1;
  memset(data,'\0',80);
  snprintf (cmd,20, "babel 2>/dev/null");
  xfile = popen (cmd, "r");
  if (!xfile)return;
  
  if (!fgets (data,80,xfile)){
  pclose(xfile);
  return;
  }
  if (!strncmp(data,"Open",4)) { /* OpenBabel 1.x */
	  strcpy(babeloutp,"--");
	  sscanf(data,"%*s %*s %d.%d.%d",&obmajor,&obminor,&obrel);
	if (obmajor*10000+obminor*10+obrel >= 11001) { 
		rval=pclose(xfile);
		snprintf(cmd,20,"babel -H");
		xfile = popen (cmd, "r");
		}
  } else if (!strncmp(data,"No output",9)) { /* OpenBabel 2.x */
          strcpy(babeloutp,"--");
          rval=pclose(xfile);
          snprintf(cmd,20,"babel -H");
          xfile = popen (cmd, "r");
 }        
  	else				     /* original babel 1.6 */
  	  strcpy(babeloutp,"CON");
  
  
  
  while (!feof(xfile)) {
   if (fgets (data,80,xfile)) {
    if (!strncmp(data,"Currently",9)|| !strncmp(data,"The following",13)) {
    start=1;
    break;
    }
   } 
  }
  if (start == 0) {
    rval = pclose(xfile);
    snprintf(cmd,20,"babel -L formats");
    xfile = popen (cmd, "r");
    start = 1;
  }
  babelin=-1;
  inmode=NULL;
  intype=NULL;
  babelout=-1;
  outmode=NULL;
  outtype=NULL;
  if (start==1){
    while (!feof(xfile)){
      (void)fgets (data, 80, xfile);
       if (feof(xfile)) 
         break;
  if (!strncmp(data,"Currently",9)|| !strncmp(data,"See further",11)) {
  babelin--;
  babelout--;
  break;
  }else if (strstr(data,"Write-only")==NULL) {
  babelin++;
	  inmode = realloc (inmode, (babelin + 1) * sizeof (char *));
	  inmode[babelin] = malloc (9 * sizeof (char));
 	  intype = realloc (intype, (babelin + 1) * sizeof (char *));
	  intype[babelin] = malloc (39 * sizeof (char));
 sscanf(data,"%s -- %36[a-zA-Z0-9 -]",inmode[babelin],intype[babelin]);
  }
  if (strstr(data,"Read-only")==NULL) {
  babelout++;
	  outmode = realloc (outmode, (babelout + 1) * sizeof (char *));
	  outmode[babelout] = malloc (10 * sizeof (char));
 	  outtype = realloc (outtype, (babelout + 1) * sizeof (char *));
	  outtype[babelout] = malloc (39 * sizeof (char));
 sscanf(data,"%s -- %36[a-zA-Z0-9 -]",outmode[babelout],outtype[babelout]);
  }
  }
  }
  rval = pclose (xfile);
  if (rval != 0 && babelin <=0 )
	fprintf(stderr,_("Consider installing Babel/OpenBabel for file format conversions...\n"));    
#if 0
  else
    {
	for (start=0;start<=babelin;start++)
	fprintf(stderr,"BABEL input %s : %s\n",inmode[start],intype[start]);
	for (start=0;start<=babelout;start++)
	fprintf(stderr,"BABEL output %s : %s\n",outmode[start],outtype[start]);
    }
#endif    
}

int
export_sxd (char *filename)
/* export molecule as a pictex file by writing a temporary XFig file and
   postprocessing that with fig2sxd */
{
  char com[255];
  FILE *xfile;
  int rval;

  if (have_fig2sxd == 0)
    return (1);			/* cannot export without fig2sxd */

  if ((int)strlen (filename))
    {
      snprintf (com,255, "fig2sxd - \"%s\"", filename);
      xfile = popen (com, "w");
      if (!xfile)
	return (1);
      rval = exfig (xfile, 0);
      if (pclose (xfile) < 0)
	return (1);
      return (rval);
    }
  else
    return (1);
}

void
check_fig2sxd ()
{
  char cmd[20];
  FILE *xfile;
  char myname[10];

  have_fig2sxd = 0;
 
  snprintf (cmd,20, "fig2sxd 2>&1");
  xfile = popen (cmd, "r");
  if (!xfile) return;
  
  fscanf (xfile, "%s", myname);
  pclose (xfile);
  if (!strncmp(myname,"fig2sxd",7)) have_fig2sxd = 1;    
}

int
export_babel (char *filename)
/* export molecule by writing a temporary molfile and
   postprocessing that with (open)babel, adding explicit hydrogens 
   if necessary */
{
  char com[255];
  FILE *xfile;
  int rval;

  if ((int)strlen (filename))
    {
      snprintf (com,255, "babel -h -imol %s -o%s \"%s\"", babeloutp, babel, filename);
      xfile = popen (com, "w");
      if (!xfile)
	return (1);
      rval = export_mdl_mol (xfile, 1);
      if (pclose (xfile) < 0)
	return (1);
      return (rval);
    }
  else
    return (1);
}

int
export_asy (char *filename)
/* exports the current drawing to an input file for Asymptote */
{
  FILE *fp;
  int x, y, tx, ty, w, h;
  int x1,y1,x2,y2;
  float area;
  float factor;
  struct data *hp_b,*hp_bx;
  struct dc *hp_a;
  struct spline *hp_sp;
  struct xy_co *coord;
  int d, i, dd;
  int xbase, ybase, xside, yside, xend, yend, xlen, ylen;
  int unicodechar;
  int bond_already_tuned=0;
  static char asycolor[7][8]={"black","blue","green","cyan","red","magenta",
  "yellow"};
 
#ifdef GTK2
  setlocale(LC_NUMERIC,"C");
#endif
 
  if ((fp = fopen (filename, "w")) == NULL)
    return (1);

  if (mark.flag && mark.w != 0 && mark.h != 0)
    {
      w = mark.w * size_factor - 6;
      h = mark.h * size_factor - 6;
    }
  else
    {
/* get true extents of drawing*/
      w = 0;
      h = 0;
      hp_b = da_root.next;
      for (d = 0; d < hp->n; d++)
	{
	  w = MAX (w, hp_b->x);
	  w = MAX (w, hp_b->tx);
	  h = MAX (h, hp_b->y);
	  h = MAX (h, hp_b->ty);
	  hp_b = hp_b->next;
	}
      hp_a = dac_root.next;
      for (d = 0; d < hp->nc; d++)
	{
	  w = MAX (w, hp_a->x);
	  if (hp_a->direct > -2)
	    w = MAX (w, hp_a->x + (int)strlen (hp_a->c) * (6 + 2 * size_factor));
	  h = MAX (h, hp_a->y + 8);
	  hp_a = hp_a->next;
	}

      hp_sp = sp_root.next;
      for (d = 0; d < hp->nsp; d++)
	{
	  w = MAX (w, hp_sp->x0);
	  h = MAX (h, hp_sp->y0);
	  w = MAX (w, hp_sp->x1);
	  h = MAX (h, hp_sp->y1);
	  w = MAX (w, hp_sp->x2);
	  h = MAX (h, hp_sp->y2);
	  w = MAX (w, hp_sp->x3);
	  h = MAX (h, hp_sp->y3);
	  hp_sp = hp_sp->next;
	}
      w = (int) (w *  1.1) ;
      h = (int) (h *  1.1) ;
    }
//  fprintf (fp, "size (%d,%d)\n\n",w,h);

fprintf(fp,"import fontsize;\n");
fprintf(fp,"defaultpen(Helvetica());\n");
fprintf(fp,"picture pic;\n");
fprintf(fp,"unitsize(pic,mm);\n");


//fprintf(fp,"size (%d,0);\n",w/2);
  
  hp_b = da_root.next;
  for (d = 0; d < hp->n; d++)
    {
      if (mark.flag && (hp_b->smarked + hp_b->tmarked) == 0)
	{
	}
      else
	{
	  coord = bond_cut (hp_b->x, hp_b->y, hp_b->tx, hp_b->ty, 12);


	  x = coord->x;
	  y = coord->y;
	  tx = coord->tx;
	  ty = coord->ty;
	if ( !use_whiteout) {
        int siz,j;
        float lfactor=((float)calc_vector(abs(tx-x),abs(ty-y))/(832.*size_factor));
        if (lfactor >1.) lfactor=1.; /* short bonds should not shrink as much, they might vanish completely */
                                     /* extra long bonds, however, need not leave more space around labels */
		i=has_label(hp_b->x,hp_b->y);
		if (i>=0) {
                 hp_a = dac_root.next;
                 for (j = 0; j < i; j++)hp_a=hp_a->next;
		 siz=2*hp_a->size;
                 if (y-ty==0&& hp_a->direct ==0) {
                   if (tx>x) {
                      int strl=0;
                      for (i=0;i<(int)strlen(hp_a->c);i++) {
                        if(hp_a->c[i] != '_' && hp_a->c[i] != '^' 
                        && hp_a->c[i] != '{' && hp_a->c[i] != '{')
                        strl++;
                      }
                      if (strl>1) x+=strl*(siz+1);
                   }     
//                   else
//                      x-=(strlen(hp_a->c)-1)*fzoom*(siz+1);
		 }
		int ox=x;
				x += (lfactor*(siz+1)/calc_vector(abs(tx-x),abs(ty-y))) *(tx-x);
				y += (lfactor*(siz+1)/calc_vector(abs(tx-ox),abs(ty-y))) *(ty-y);
				}
		i=has_label(hp_b->tx,hp_b->ty);
		if (i>=0) {
                 hp_a = dac_root.next;
                 for (j = 0; j < i; j++)hp_a=hp_a->next;
		 siz=2*hp_a->size;
                 if (y-ty==0 ){
                      int strl=0;
                      for (i=0;i<(int)strlen(hp_a->c);i++) {
                        if(hp_a->c[i] != '_' && hp_a->c[i] != '^' 
                        && hp_a->c[i] != '{' && hp_a->c[i] != '{')
                        strl++;
                      }
                   if ( hp_a->direct<-1) {
                      if (tx>x)
                        tx-=(strl-1)*(siz+1);
//                      else
//                       tx+=(strlen(hp_a->c)-1)*(siz+1);
                   }
                   else if ( hp_a->direct==0) {
//                      if (tx>x)
  //                      tx-=(strlen(hp_a->c)-1)*fzoom*(siz+1);
    //                  else
 if (tx<x)                      tx+=(strl-1)*(siz+1);
                   }
                }  
		int otx=tx;
				tx -= (lfactor*(siz+1)/calc_vector(abs(tx-x),abs(ty-y))) *(tx-x);
				ty -= (lfactor*(siz+1)/calc_vector(abs(otx-x),abs(ty-y))) *(ty-y);
				}
		}		


	  if (!hp_b->bond)
	    {
	      fprintf (fp, "draw(pic, (%d,%d)--(%d,%d),%s);\n", x, h-y, tx, h-ty,asycolor[hp_b->color]);
	    }
	  if (hp_b->bond == 5)
	    {
    bond_already_tuned = 0;
  x1 = tx - (int) (0.1 * (float) (ty - y));
  y1 = ty + (int) (0.1 * (float) (tx - x));
  x2 = tx + (int) (0.1 * (float) (ty - y));
  y2 = ty - (int) (0.1 * (float) (tx - x));
factor=1.;
  hp_bx=da_root.next;
  for (dd = 0; dd < hp->n; dd++)
    {
    
      if (hp_bx->bond == 10)
	{
	  if (
	      (abs (hp_bx->x * factor - tx) < 3
	       && abs (hp_bx->y * factor - ty) < 3)
	      || (abs (hp_bx->tx * factor - tx) < 3
		  && abs (hp_bx->ty * factor - ty) < 3))
	    {
	      coord =
		center_double_bond (hp_bx->x, hp_bx->y, hp_bx->tx, hp_bx->ty, db_dist);
	      if (abs (hp_bx->x * factor - tx) < 3
		  && abs (hp_bx->y * factor - ty) < 3)
		{
		  x1 = coord->x * factor;
		  y1 = coord->y * factor;
		  coord++;
		  x2 = coord->x * factor;
		  y2 = coord->y * factor;
		}
	      else
		{
		  x1 = coord->tx * factor;
		  y1 = coord->ty * factor;
		  coord++;
		  x2 = coord->tx * factor;
		  y2 = coord->ty * factor;
		}
	      area =  (0.5 * abs (x * (y1 - y2)
				 + x1 * (y2 - y) + x2 * (y - y1)));

	      if (fabs (area) < 76. * factor)
		{
		  x1 = tx - (int) (0.05 * (float) (ty - y));
		  y1 = ty + (int) (0.05 * (float) (tx - x));
		  x2 = tx + (int) (0.05 * (float) (ty - y));
		  y2 = ty - (int) (0.05 * (float) (tx - x));
		}
	      else bond_already_tuned = 1;

	    } /* if connected to wide end of this wedge */
      } /* if adjoining bond is wide */
      if (hp_bx->bond == 0 && !bond_already_tuned) {
        if ((abs (hp_bx->x * factor - tx) < 3 && abs (hp_bx->y * factor - ty) < 3)
          ||(abs (hp_bx->tx * factor - tx) < 3 && abs (hp_bx->ty * factor - ty) < 3))

        /* let the wedge join smoothly alongside another bond */
	{
	  coord = intersect(x,y,x1,y1,hp_bx->x*factor,hp_bx->y*factor,
			    hp_bx->tx*factor,hp_bx->ty*factor);
	  coord->tx = coord->x;
	  coord->ty = coord->y;
	  coord = intersect(x,y,x2,y2,hp_bx->x*factor,hp_bx->y*factor,
			    hp_bx->tx*factor,hp_bx->ty*factor);
	  x1 = coord->tx; 
          y1 = coord->ty;
          x2 = coord->x;
          y2 = coord->y;

	  area = 0.5 * abs (x * (y1 - y2)
                                 + x1 * (y2 - y) + x2 * (y - y1));

          if (fabs (area) > 3300. * factor || fabs(area) < 1750. * factor)
            {
              x1 = tx - (int) (0.1 * (float) (ty - y));
              y1 = ty + (int) (0.1 * (float) (tx - x));
              x2 = tx + (int) (0.1 * (float) (ty - y));
              y2 = ty - (int) (0.1 * (float) (tx - x));
            }
        } /* if connected to wide end of this wedge */
      } /* if adjoining bond is single, and not already adjusted */
      hp_bx = hp_bx->next;
      } /* for dd */ 
	      fprintf (fp, "fill(pic, (%d,%d)--(%d,%d)--(%d,%d)--cycle,%s);\n",
		       x, h-y, x1,h-y1,x2,h-y2,asycolor[hp_b->color]);
/*	      fprintf (fp, "            points=\"%d,%d %d,%d %d,%d\" />\n", x,
		       y, (int) (tx - 0.08 * (ty - y)),
		       (int) (ty + 0.08 * (tx - x)),
		       (int) (tx + 0.08 * (ty - y)),
		       (int) (ty - 0.08 * (tx - x)));
*/
	    }
	  if (hp_b->bond == 6)
	    {
	      for (i = 0; i < 8; i++)
		{
	      fprintf (fp, "draw(pic, (%d,%d)--(%d,%d),%s);\n", 
			   (int) (x + 0.125 * i * (tx - x) -
				  0.01 * (ty - y) * i),
			   (int) (h-(y + 0.125 * i * (ty - y) +
				  0.01 * (tx - x) * i)),
			   (int) (x + 0.125 * i * (tx - x) +
				  0.01 * (ty - y) * i),
			   (int) (h-(y + 0.125 * i * (ty - y) -
				  0.01 * (tx - x) * i)),
                           asycolor[hp_b->color]);
		}
	    }
	  if (hp_b->bond == 7)
	    {
	    fprintf(fp, "draw (pic, arc((%d,%d),(%d,%d),(%d,%d),false),%s);\n",
	            x+(tx-x)/10,h-(y+(ty-y)/10), x,h-y, 
                    x+(tx-x)/5,h-(y+(ty-y)/5), asycolor[hp_b->color]);
            fprintf(fp, "draw (pic, arc((%d,%d),(%d,%d),(%d,%d),true),%s);\n",
                    x+3*(tx-x)/10,h-(y+3*(ty-y)/10), x+(tx-x)/5,h-(y+(ty-y)/5),
                    x+2*(tx-x)/5,h-(y+2*(ty-y)/5), asycolor[hp_b->color]);
            fprintf(fp, "draw (pic, arc((%d,%d),(%d,%d),(%d,%d),false),%s);\n",
                    x+5*(tx-x)/10,h-(y+5*(ty-y)/10), x+2*(tx-x)/5,h-(y+2*(ty-y)/5),
                    x+3*(tx-x)/5,h-(y+3*(ty-y)/5),asycolor[hp_b->color]);
            fprintf(fp, "draw (pic, arc((%d,%d),(%d,%d),(%d,%d),true),%s);\n",
                    x+7*(tx-x)/10,h-(y+7*(ty-y)/10),x+3*(tx-x)/5,h-(y+3*(ty-y)/5),
                    x+4*(tx-x)/5,h-(y+4*(ty-y)/5), asycolor[hp_b->color]);
            fprintf(fp, "draw (pic, arc((%d,%d),(%d,%d),(%d,%d),false),%s);\n",
                    x+9*(tx-x)/10,h-(y+9*(ty-y)/10),x+4*(tx-x)/5,h-(y+4*(ty-y)/5),
                    tx,h-ty, asycolor[hp_b->color]);
	    }
	  if (hp_b->bond == 8)
	    {
	      fprintf (fp, "draw(pic, (%d,%d)--(%d,%d),%s);\n", 
		       x, h-y, (int)(x+0.8*(tx-x)), (int)(h-(y+0.8*(ty-y))),asycolor[hp_b->color]);
	      fprintf (fp, "fill(pic, (%d,%d)--(%d,%d)--(%d,%d)--cycle,%s);\n",
		       (int) (x + 0.8 * (tx - x)), (int) (h-(y + 0.8 * (ty - y))),
		       (int) (x + 0.8 * (tx - x) + 0.1 * (ty - y)),
		       (int) (h-(y + 0.8 * (ty - y) - 0.1 * (tx - x))), tx, h-ty,
		       asycolor[hp_b->color]);
	    }
	  if (hp_b->bond == 9)
	    {
            int   xlen = tx - x;
            int   ylen = ty - y;
            float veclen = sqrt ((double)(xlen * xlen + ylen * ylen));
            float scalefact=64./veclen; /* keep arrowhead size constant (64=std length)*/
            int xbase = (int) (tx - 0.2 *xlen*scalefact);
            int ybase = (int) (ty - 0.2 *ylen*scalefact);
       
	      fprintf (fp, "draw(pic, (%d,%d)--(%d,%d),%s);\n", 
		       x, h-y, xbase, h-ybase, asycolor[hp_b->color]);
	      fprintf (fp, "fill(pic, (%d,%d)--(%d,%d)--(%d,%d)--cycle,%s);\n",
                      tx, h-ty,(int)(xbase + 0.1 * ylen*scalefact),
                      (int)(h-(ybase - 0.1 * xlen*scalefact)) ,
                      (int)(xbase - 0.1 * ylen*scalefact),
                      (int)(h-(ybase + 0.1 * xlen*scalefact)), asycolor[hp_b->color] ); 
/*(int) (x + 0.8 * (tx - x) + 0.1 * (ty - y)),
(int) (y + 0.8 * (ty - y) - 0.1 * (tx - x)),
(int) (x + 0.8 * (tx - x) - 0.1 * (ty - y)),
(int) (y + 0.8 * (ty - y) + 0.1 * (tx - x)));*/
	    }
	  if (hp_b->bond == 11)
	    {
	      fprintf (fp, "draw(pic,circle((%d,%d),%d.),%s);\n",
		       x, h-y, calc_vector (x - tx, y - ty),asycolor[hp_b->color]);
	    }
	  if (hp_b->bond == 10)
	    {
	      fprintf (fp, "draw(pic, (%d,%d)--(%d,%d),%s+squarecap+linewidth(10));\n", 
		       x, h-y, tx, h-ty, asycolor[hp_b->color]);
	    }

	  if (hp_b->bond == 12)
	    {
	      fprintf (fp, "draw(pic, (%d,%d)--(%d,%d),%s+dotted);\n",
		       x, h-y, tx, h-ty, asycolor[hp_b->color]);
	    }
	  if (hp_b->bond == 13)
	    {
	      fprintf (fp, "draw(pic, (%d,%d)--(%d,%d),rgb(%.3f,%.3f,%.3f)+squarecap+linewidth(3));\n", 
		       x + (tx - x) / 4, h-(y + (ty - y) / 4),
		       tx - (tx - x) / 4, h-(ty - (ty - y) / 4), bgred/65535.,bggreen/65535.,bgblue/65535.);
	      fprintf (fp, "draw(pic, (%d,%d)--(%d,%d),%s);\n", 
		       x, h-y, tx, h-ty, asycolor[hp_b->color]);
	    }
	  if (hp_b->bond == 4)
	    {
	      coord = center_double_bond (x, y, tx, ty, db_dist);
	      fprintf (fp, "draw(pic, (%d,%d)--(%d,%d),%s);\n", 
		       coord->x, h-coord->y, coord->tx, h-coord->ty, asycolor[hp_b->color]);
	      coord++;
	      fprintf (fp, "draw(pic, (%d,%d)--(%d,%d),%s);\n", 
		       coord->x, h-coord->y, coord->tx, h-coord->ty, asycolor[hp_b->color]);
	    }
	  if (hp_b->bond == 1 || hp_b->bond == 3)
	    {
	      coord = multi_bonds (x, y, tx, ty, mb_dist);
	      fprintf (fp, "draw(pic, (%d,%d)--(%d,%d),%s);\n", 
		       x, h-y, tx, h-ty, asycolor[hp_b->color]);
	      fprintf (fp, "draw(pic, (%d,%d)--(%d,%d),%s);\n", 
		       coord->x, h-coord->y, coord->tx, h-coord->ty, asycolor[hp_b->color]);
	    }
	  if (hp_b->bond == 2 || hp_b->bond == 3)
	    {
	      coord = multi_bonds (tx, ty, x, y, mb_dist);
	      fprintf (fp, "draw(pic, (%d,%d)--(%d,%d), %s);\n",
		       x, h-y, tx, h-ty, asycolor[hp_b->color]);
	      fprintf (fp, "draw(pic, (%d,%d)--(%d,%d), %s);\n",
		       coord->x, h-coord->y, coord->tx, h-coord->ty,asycolor[hp_b->color]);
	    }
	  if (hp_b->bond == 14)
	    {
	      coord = multi_bonds (x, y, tx, ty, mb_dist);
	      fprintf (fp, "draw(pic, (%d,%d)--(%d,%d), %s);\n",
		       x, h-y, tx, h-ty, asycolor[hp_b->color]);
	      fprintf (fp, "draw(pic, (%d,%d)--(%d,%d), %s+longdashed);\n",
		       coord->x, h-coord->y, coord->tx, h-coord->ty, asycolor[hp_b->color]);
	    }
	  if (hp_b->bond == 15)
	    {
	      coord = multi_bonds (tx, ty, x, y, mb_dist);
	      fprintf (fp, "draw(pic, (%d,%d)--(%d,%d), %s);\n",
		       x, h-y, tx, h-ty, asycolor[hp_b->color]);
	      fprintf (fp, "draw(pic, (%d,%d)--(%d,%d), %s+longdashed);\n",
		       coord->x, h-coord->y, coord->tx, h-coord->ty, asycolor[hp_b->color]);
	    }
	  if (hp_b->bond == 16)
	    {
	      coord = center_double_bond (x, y, tx, ty, db_dist);
	      x = coord->x;
	      y = coord->y;
	      tx = coord->tx;
	      ty = coord->ty;
	      coord++;
	      for (i = 0; i < 10; i++) {
	      fprintf (fp, "draw(pic, (%d,%d)--(%d,%d), %s);\n",
			 (int) (x + 0.1 * i * (tx - x)),
			 (int) (h-(y + 0.1 * i * (ty - y))),
			 (int) (coord->x + 0.1 * i * (coord->tx - coord->x)),
			 (int) (h-(coord->y + 0.1 * i * (coord->ty - coord->y))),
			 asycolor[hp_b->color]);
	     }
	    }
	  if (hp_b->bond == 18)
	    {
	      fprintf (fp, "draw(pic, (%d,%d)--(%d,%d), %s);\n",
		       x, h-y, tx, h-ty, asycolor[hp_b->color]);
	      coord = center_double_bond (x, y, tx, ty, db_dist+1);
	      fprintf (fp, "draw(pic, (%d,%d)--(%d,%d), %s);\n",
		       coord->x, h-coord->y, coord->tx, h-coord->ty, asycolor[hp_b->color]);
	      coord++;
	      fprintf (fp, "draw(pic, (%d,%d)--(%d,%d), %s);\n",
		       coord->x, h-coord->y, coord->tx, h-coord->ty, asycolor[hp_b->color]);
	    }

	  if (hp_b->bond == 19)
	    {
	int tmpx1,tmpx2,tmpy1,tmpy2,tmptx1,tmptx2,tmpty1,tmpty2;
	      coord = center_double_bond (x, y, tx, ty, db_dist+1);
	tmpx1=coord->x;
	tmpy1=coord->y;
	tmptx1=coord->tx;
	tmpty1=coord->ty;
	coord++;
	tmpx2=coord->x;
	tmpy2=coord->y;
	tmptx2=coord->tx;
	tmpty2=coord->ty;
	      coord = center_double_bond (tmpx1, tmpy1, tmptx1, tmpty1, db_dist-1);
	      fprintf (fp, "draw(pic, (%d,%d)--(%d,%d), %s);\n",
		       coord->x, h-coord->y, coord->tx, h-coord->ty, asycolor[hp_b->color]);
	      coord++;
	      fprintf (fp, "draw(pic, (%d,%d)--(%d,%d), %s);\n",
		       coord->x, h-coord->y, coord->tx, h-coord->ty, asycolor[hp_b->color]);
	      coord = center_double_bond (tmpx2, tmpy2, tmptx2, tmpty2, db_dist-1);
	      fprintf (fp, "draw(pic, (%d,%d)--(%d,%d), %s);\n",
		       coord->x, h-coord->y, coord->tx, h-coord->ty, asycolor[hp_b->color]);
	      coord++;
	      fprintf (fp, "draw(pic, (%d,%d)--(%d,%d), %s);\n",
		       coord->x, h-coord->y, coord->tx, h-coord->ty, asycolor[hp_b->color]);
	}	    
	}
      hp_b = hp_b->next;
    }

  hp_a = dac_root.next;
  for (d = 0; d < hp->nc; d++)
    {
      if (mark.flag && hp_a->marked == 0)
	{
	}
      else
	{
	  fprintf(fp,"frame f;\n");
          fprintf (fp,"label(f,\"$\\mathrm{"); 

	  for (i = 0; i < (int)strlen (hp_a->c); ++i)
	    {
	      if (hp_a->c[i] == '\\')
		hp_a->c[i] = ' ';
	      if (hp_a->c[i] == '@')
		{
			unicodechar=848+hp_a->c[++i];
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
			case 1031:										
			case 775: /* bullet */
				unicodechar=8226;
				break;
                        case 891: //oplus
                                unicodechar=8853;
                                break;
                        case 893: //ominus
                                unicodechar=8854;
                                break;			
			default:
				break;
			}				 
		  fprintf (fp, "&#%04d;", unicodechar);
		  /*FIXME: unicode greek does not always map to X11 Symbol, e.g. F is Z not Phi */
		}
	      else if (hp_a->c[i] == '#')
		{
		  fprintf (fp, "{\\bf ");
		  fprintf (fp, "%c", hp_a->c[++i]);
		  fprintf (fp, "}");
		}
	      else if (hp_a->c[i] == '|')
		{
		  fprintf (fp, "{\\it");
		  fprintf (fp, "%c", hp_a->c[++i]);
		  fprintf (fp, "}");
		}
		else
		  fprintf (fp, "%c", hp_a->c[i]);
		}
            if (hp_a->c[i]==' ') /* protect blanks again after output */
              hp_a->c[i]='\\';
          }
          fprintf( fp,"}$\",NoAlign,%s);\n",asycolor[hp_a->color]);
          switch (hp_a->direct) {
          case 0:
     	    fprintf (fp, "add(pic,f,(%d%s,%d));\n",hp_a->x,"+(max(f).x-min(f).x)/2.5",h-hp_a->y);
            break;
          case -1:  
            fprintf (fp, "add(pic,f,(%d,%d));\n",hp_a->x,h-hp_a->y);
            break;
          case -2:  
            fprintf (fp, "add(pic,f,(%d%s,%d));\n",hp_a->x,"-(max(f).x-min(f).x)/4",h-hp_a->y);
            break;
          }
      hp_a = hp_a->next;
    }

  hp_sp = sp_root.next;
  for (d = 0; d < hp->nsp; d++)
    {
      if (mark.flag == 1 && hp_sp->marked == 0)
	{
	}
      else
	{
	if (hp_sp->type == -1)
	  fprintf (fp, "fill (pic, (%d,%d)..", hp_sp->x0, h-hp_sp->y0);
        else
	  fprintf (fp, "draw (pic, (%d,%d)..", hp_sp->x0, h-hp_sp->y0);
	  fprintf (fp, "controls (%d,%d) and (%d,%d)..(%d,%d)", hp_sp->x1, h-hp_sp->y1,
		   hp_sp->x2, h-hp_sp->y2, hp_sp->x3, h-hp_sp->y3);
/*
	  fprintf (fp, "<path d=\"M %d,%d\n", hp_sp->x0, h-hp_sp->y0);
	  fprintf (fp, "C %d,%d %d,%d %d,%d\"\n", hp_sp->x1, h-hp_sp->y1,
		   hp_sp->x2, h-hp_sp->y2, hp_sp->x3, h-hp_sp->y3);
*/
	  switch (hp_sp->type)
	    {
	    case 0:
	    case 1:
	    case 2:
	      fprintf (fp,
		       ",%s);\n",asycolor[hp_sp->color]);
	      break;
	      ;;
	    case -2:
	      fprintf (fp,
		       ",%s+longdashed);\n",asycolor[hp_sp->color]);
	      break;
	      ;;
	    case -1:
	      fprintf (fp,
		       "--cycle,%s);\n",asycolor[hp_sp->color]);
	    }

	  if (hp_sp->type > 0)
	    {
	      xbase = (int)
		(0.7 * 0.7 * 0.7 * (double) hp_sp->x3 + 3. * 0.7 * 0.7 * (1. -
									 0.7)
		* (double) hp_sp->x2 + 3. * 0.7 * (1. - 0.7) * (1. -
								0.7) *
		(double) hp_sp->x1 + (1. - 0.7) * (1. - 0.7) * (1. -
								0.7) *
		hp_sp->x0);
	      ybase = (int)
		(0.7 * 0.7 * 0.7 * (double) hp_sp->y3 + 3. * 0.7 * 0.7 * (1. -
									 0.7)
		* (double) hp_sp->y2 + 3. * 0.7 * (1. - 0.7) * (1. -
								0.7) *
		(double) hp_sp->y1 + (1. - 0.7) * (1. - 0.7) * (1. -
								0.7) *
		hp_sp->y0);


	      xlen = hp_sp->x3 - xbase;
	      ylen = hp_sp->y3 - ybase;

	      if (xlen != 0)
		xlen = (int) copysign (50., (double)xlen);
	      if (ylen != 0)
		ylen = (int) copysign (50., (double)ylen);

	      xside = (int) (xbase + 0.15 * ylen);
	      yside = (int) (ybase - 0.15 * xlen);

	      if (hp_sp->type == 1)
		{

		  xend = (int) (xbase - 0.15 * ylen);
		  yend = (int) (ybase + 0.15 * xlen);
		  x =
		    (xside - hp_sp->x0) * (xside - hp_sp->x0) + (yside -
								 hp_sp->y0) *
		    (yside - hp_sp->y0);
		  tx =
		    (xend - hp_sp->x0) * (xend - hp_sp->x0) + (yend -
							       hp_sp->y0) *
		    (yend - hp_sp->y0);
		  if (tx > x)
		    {
		      xside = xend;
		      yside = yend;
		    }
		  xend = xbase;	/*on baseline */
		  yend = ybase;
		}
	      else
		{

		  xend = (int) (xbase - 0.15 * ylen);
		  yend = (int) (ybase + 0.15 * xlen);
		}


	      fprintf (fp, "fill (pic, (%d,%d)--(%d,%d)--(%d,%d)--cycle,%s);\n",
		       hp_sp->x3, h-hp_sp->y3, xside, h-yside, xend, h-yend, asycolor[hp_sp->color]);
	    }
	}
      hp_sp = hp_sp->next;
    }
  fprintf(fp,"add(pic);\n");
  if (strcmp(bghexcolor,"#ffffff"))
      fprintf(fp,"shipout(bbox(pic,2mm,Fill(rgb %.3f,%.3f,%.3f)));\n",bgred/65535.,bggreen/65535.,bgblue/65535.); 
  fclose (fp);
  return (0);
}
