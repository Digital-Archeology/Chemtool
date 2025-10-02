#include "inout_common.h"

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
	    fprintf (fp, "%i\t%i\t%i\t%i\t%i\t%i\t%i\n",
	   	   hp_b->x, hp_b->y, hp_b->tx, hp_b->ty, hp_b->bond, hp_b->decoration,hp_b->color);
/*	  }else{
	    fprintf (fp, "%i\t%i\t%i\t%i\t%i\n",
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
	  fprintf (fp, "%i\t%i\t%s\t%i\t%i\t%i\t%i\n",
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
		   "%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\n",
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
    {			/* typo in versions < 1.1.2 */
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
    {			/* typo in versions < 1.1.2 */
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
			{
				fontsize = 0;
			}
		else
			{
				fontsize = 1;
			}

		res = fgetc (fp);
		if (res == EOF)
			{
				fclose (fp);
				return (2);
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
