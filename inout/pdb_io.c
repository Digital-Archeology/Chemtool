#include "inout_common.h"

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
	int *atom_numbers = NULL;
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
	{		/*premature end */
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
	atom_numbers = realloc (atom_numbers, (i + 1) * sizeof (int));
	  atcode[i] = malloc (9 * sizeof (char));
	  atjust[i] = 0;
	res = sscanf (line, "%s %d %s %*6c %*6c %lf %lf %lf", code, &atom_numbers[i], atcode[i],
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
				if (atom_numbers[m] == at0) {
			at0=m+1;
			found=1;
			break;
			}
		}	
		for (m=0;m<=pdbn;m++){
				if (atom_numbers[m] == con[i]) {
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
free (atom_numbers);
atom_numbers = NULL;
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
    {			/* if no bonds exist, i.e. there were no CONECTs */
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
	      /* fall through */
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
	{		/* in MDL import, add all non-C labels */
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
