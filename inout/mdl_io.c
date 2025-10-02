#include "inout_common.h"

int
import_mdl_mol (char *filename, int skip)
/* imports a MDL Molfile structure */
{
/*  int x, y, tx, ty;*/ /* the coordinates */
/*  int bond = 0; */ /* Chemtool bondstyle */
  float mdlfactor = 60.0;  /* conversion factor .cht <-> .mol */
  int mdloffset = 200;
  int label_direction = 0; /* Chemtool text direction */
  int d, e;         /* dummy variables */
  int v3=0;
  char line[255];
  char prop[41],*cfg;
  double pdbxmin = 100000., pdbxmax = -100000., pdbymin = 1000000., pdbymax =
    -100000., pdbzmin = 100000., pdbzmax = -100000.;

  /* the counts line, see ctfile.pdf for more information */
  int a;            /* number of atoms */
  int b;            /* number of bonds */
  /* not parsed right now: */
  int l;            /* number of atoms list */
  int f;            /* obsolete */
  int c;            /* chiral flag, 0=not chiral, 1=chiral */
  int s;            /* number of stext entries */
  int x_;           /* number of reaction components + 1 */
  int r;            /* number of reactants */
  int p;            /* number of products */
  int i;            /* number of intermediates */
  int m;            /* number of additional properties, 
                   no longer supported and set to 999 */
  char v[6];        /* ctab version, 'v2000' or 'v3000' */
  /* end of counts line */

  /* the atom block */

/*float xxx, yyy, zzz; */ /* the coordinates of the atom */
  char aaa[3];      /* the atomic symbol */
  /* not parsed right now: */
  int dd;           /* mass difference for isotopes */
  int ccc;          /* charge */
  int sss;          /* atom stereo parity */
  int hhh;          /* hydrogen count+1 */
  int bbb;          /* stereo care box */
  int vvv;          /* valence */
  int HHH;          /* H designator, redundant */
  int rrr;          /* reaction component type */
  int iii;          /* reaction component number */
  int mmm;          /* atom-atom mapping number */
  int nnn;          /* inversion/retention flag */
  int eee;          /* exact change flag */
  /* end atom block */

  /* the bond block */
  int atom1;        /* first atom number */
  int atom2;        /* second atom number */
  int tt;           /* bond type */
  int ss;           /* bond stereo */
  int xx;           /* not used */
  int rr;           /* bond topology, 0=eighter, 1=ring, 2=chain */
  int cc;           /* reaction center status */
  /* end bond block */


  char label[20];   /* a label defined in the .mol-file */
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

      atjust[pdbn] = -1;    /*default to centered labels */
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
    label_direction = 0;
      f = (int)strlen (label);
            if (f == 1)
                label_direction = -1;       /* center label if only one character */
      for (e = 0; label[e] != '\0'; e++)
    {
      if (isdigit (label[e]))
        {
          /* If there's a number at the beginning, assume right-
           * justified text: */
          if (e == 0)
                label_direction = -2;
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

    atjust[atom1 - 1] = (short) label_direction;

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
  float mdlfactor = 60.0;  /* conversion factor .cht <-> .mol */
  float previewscale;
  int label_direction = 0; /* Chemtool text direction */
  int d, e;            /* dummy variables */
  GdkRectangle update_rect;
  int v3=0;
  char prop[41],*cfg;
  char line[255];
  double pdbxmin = 100000., pdbxmax = -100000., pdbymin = 1000000., pdbymax =
    -100000.;

  /* the counts line, see ctfile.pdf for more information */
  int a;            /* number of atoms */
  int b;            /* number of bonds */
  /* not parsed right now: */
  int l;            /* number of atoms list */
  int f;            /* obsolete */
  int c;            /* chiral flag, 0=not chiral, 1=chiral */
  int s;            /* number of stext entries */
  int x_;           /* number of reaction components + 1 */
  int r;            /* number of reactants */
  int p;            /* number of products */
  int i;            /* number of intermediates */
  int m;            /* number of additional properties, 
                   no longer supported and set to 999 */
  char v[6];        /* ctab version, 'v2000' or 'v3000' */
  /* end of counts line */

  /* the atom block */

/*float xxx, yyy, zzz; */ /* the coordinates of the atom */
  char aaa[3];      /* the atomic symbol */
  /* not parsed right now: */
  int dd;           /* mass difference for isotopes */
  int ccc;          /* charge */
  int sss;          /* atom stereo parity */
  int hhh;          /* hydrogen count+1 */
  int bbb;          /* stereo care box */
  int vvv;          /* valence */
  int HHH;          /* H designator, redundant */
  int rrr;          /* reaction component type */
  int iii;          /* reaction component number */
  int mmm;          /* atom-atom mapping number */
  int nnn;          /* inversion/retention flag */
  int eee;          /* exact change flag */
  /* end atom block */

  /* the bond block */
  int atom1;        /* first atom number */
  int atom2;        /* second atom number */
  int tt;           /* bond type */
  int ss;           /* bond stereo */
  int xx;           /* not used */
  int rr;           /* bond topology, 0=eighter, 1=ring, 2=chain */
  int cc;           /* reaction center status */
  /* end bond block */


  char label[20];   /* a label defined in the .mol-file */
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
      atjust[pdbn] = -1;    /*default to centered labels */
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
    label_direction = 0;
      f = (int)strlen (label);
            if (f == 1)
                label_direction = -1;       /* center label if only one character */
      for (e = 0; label[e] != '\0'; e++)
    {
      if (isdigit (label[e]))
        {
          /* If there's a number at the beginning, assume right-
           * justified text: */
          if (e == 0)
            label_direction = -2;
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

    atjust[atom1 - 1] = (short) label_direction;

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
export_mdl_mol (FILE *fp, int topipe)
/* exports a MDL Molfile structure */
{
  float factor = 76.0;     /* conversion factor .cht <-> .mol */
  int d, e, f;         /* dummy variables */
  char aaa[3];         /* the atomic symbol */
  int atom1 = 0;       /* first atom number */
  int atom2 = 0;       /* second atom number */
  int tt;          /* bond type */
  int ss;          /* bond stereo */
  struct dc *hp_a;
  struct data *hp_b;
  int atoms[999][2];       /* coordinates of unique labels */
  int start_already_there;
  int end_already_there;
  int is_label;
  int labelcount;       /* number of unique atoms/labels */
  int omit;         /* number of decorative lines (boxes etc) */
  int hcount=0;         /* for explicit CH_3, CH_2, CH */
  
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
      tt = 2;    /*double*/ /* 7=double or aromatic */
      break;
    case 4:
      tt = 2;       /* double */
      break;
    case 3:
    case 18:
    case 19:    /* FIXME: does MOL handle quadruple bonds at all ?*/
      tt = 3;       /* triple */
      break;
    case 5:
    case 10:
      ss = 1;       /* stereo up */
      break;
    case 6:
    case 16:
      ss = 6;       /* stereo down */
      break;
    case 7:
      ss = 4;       /* stereo either */
      break;
    case 8:
    case 9:
    case 11:
/*    case 12: */ /* bond or not? */
    case 17:
      tt = 0;       /* assume no chemical bond */
      break;
    case 14:
    case 15:
      tt = 1;       /* 6 = single or aromatic */
      break;
    default:
      tt = 1;       /* single */
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
