#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char**argv)
/* dumps a chemtool drawing file in the format used in templates.h */
{
  int d, nb,na,ns;
  int w,h;
  int x[50], y[50] , tx[50], ty[50], xl[50],yl[50],dl[50],b[50];
  int xs1[50], ys1[50] , xs2[50], ys2[50], xs3[50],ys3[50],xs4[50],ys4[50],ts[50];
  int xr,yr;
  char tl[50][50];
  char str[255], str1[255];
  int refx,refy;
  FILE *fpin,*fpout;
  char version[10];
  char filename[100];
  
  if (argc==2) strcpy(filename,argv[1]);

  if ((fpin = fopen (filename, "r")) == NULL)
    return (1);
    strcat(filename,".tmpl");
  if ((fpout = fopen (filename, "w")) == NULL)
    return (1);

  fscanf (fpin, "%s %s %s", str, str1, version);
  if (strcmp (str, "Chemtool") || strcmp (str1, "Version"))
    exit(2);
fprintf(stderr,"%s %s %s\n",str,str1,version); 
  fscanf (fpin, "%s %i %i", str, &w,&h);

  fscanf (fpin, "%s %i", str, &nb);
  if (!strcmp (str, "bonds") || !strcmp (str, "bounds"))
    {				/* typo in versions < 1.1.2 */
	fprintf(stderr,"%d bonds\n",nb);
fgetc(fpin);
      for (d = 0; d < nb; d++)
	{
	(void)fgets(str,80,fpin);
	  sscanf (str, "%i %i %i %i %i", &x[d]  , &y[d], &tx[d], &ty[d] , &b[d]);
	}
    }
fprintf(stderr,"bonds ok\n"); 
  fscanf (fpin, "%s %i", str, &na);
  if (!strcmp (str, "atoms"))
    {
    fprintf(stderr,"%d atoms\n",na);
    fgetc(fpin);
      for (d = 0; d < na; d++)
	{
	(void)fgets(str,80,fpin);
	  sscanf (str, "%i %i %s %i", &xl[d], &yl[d], tl[d], &dl[d]);
	}
    }
fprintf(stderr,"atoms ok\n");

  fscanf (fpin, "%s %i", str, &ns);
  if (!strcmp (str, "splines"))
    {
    if (ns>0) fgetc(fpin);
      for (d = 0; d < ns; d++)
	{
	(void)fgets(str,80,fpin);
	  sscanf (str, "%i %i %i %i %i %i %i %i %i", xs1[d], ys1[d], 
	  xs2[d], ys2[d],xs3[d],ys3[d],xs4[d],ys4[d],ts[d]);
	}
    }
      fscanf (fpin, "%s %i %i", str, &xr,&yr);
              if (!strcmp(str,"attach")){
                       refx=xr;
                       refy=yr;
              }
                                                                                              

  fclose (fpin);
	fprintf(fpout,"bonds:%d\n",nb);
	fprintf(fpout,"atoms:%d\n",na);
	fprintf(fpout,"splines:%d\n",ns);
if (nb>0){
	fprintf(fpout,"{");
	for (d=0;d<nb-1;d++)
	fprintf(fpout,"%d,",x[d]);
	fprintf(fpout,"%d}\n",x[nb-1]);
	fprintf(fpout,"{");
	for (d=0;d<nb-1;d++)
	fprintf(fpout,"%d,",y[d]);
	fprintf(fpout,"%d}\n",y[nb-1]);
	fprintf(fpout,"{");
	for (d=0;d<nb-1;d++)
	fprintf(fpout,"%d,",tx[d]);
	fprintf(fpout,"%d}\n",tx[nb-1]);
	fprintf(fpout,"{");
	for (d=0;d<nb-1;d++)
	fprintf(fpout,"%d,",ty[d]);
	fprintf(fpout,"%d}\n",ty[nb-1]);
	fprintf(fpout,"{");
	for (d=0;d<nb-1;d++)
	fprintf(fpout,"%d,",b[d]);
	fprintf(fpout,"%d}\n",b[nb-1]);
}

if (na>0){	
	fprintf(fpout,"{");
	for (d=0;d<na-1;d++)
	fprintf(fpout,"%d,",xl[d]);
	fprintf(fpout,"%d}\n",xl[na-1]);
	fprintf(fpout,"{");
	for (d=0;d<na-1;d++)
	fprintf(fpout,"%d,",yl[d]);
	fprintf(fpout,"%d}\n",yl[na-1]);
	fprintf(fpout,"{");
	for (d=0;d<na-1;d++)
	fprintf(fpout,"%d,",dl[d]);
	fprintf(fpout,"%d}\n",dl[na-1]);
	fprintf(fpout,"{");
	for (d=0;d<na-1;d++)
	fprintf(fpout,"\"%s\",",tl[d]);
	fprintf(fpout,"\"%s\"}\n",tl[na-1]);
}
if (ns>0){	
	fprintf(fpout,"{");
	for (d=0;d<ns-1;d++)
	fprintf(fpout,"%d,",xs1[d]);
	fprintf(fpout,"%d}\n",xs1[ns-1]);
	fprintf(fpout,"{");
	for (d=0;d<ns-1;d++)
	fprintf(fpout,"%d,",ys1[d]);
	fprintf(fpout,"%d}\n",ys1[ns-1]);
	fprintf(fpout,"{");
	for (d=0;d<ns-1;d++)
	fprintf(fpout,"%d,",xs2[d]);
	fprintf(fpout,"%d}\n",xs2[ns-1]);
	fprintf(fpout,"{");
	for (d=0;d<ns-1;d++)
	fprintf(fpout,"\"%s\",",ys2[d]);
	fprintf(fpout,"\"%s\"}\n",ys2[ns-1]);
	fprintf(fpout,"{");
	for (d=0;d<ns-1;d++)
	fprintf(fpout,"%d,",xs3[d]);
	fprintf(fpout,"%d}\n",xs3[ns-1]);
	fprintf(fpout,"{");
	for (d=0;d<ns-1;d++)
	fprintf(fpout,"%d,",ys3[d]);
	fprintf(fpout,"%d}\n",ys3[ns-1]);
	fprintf(fpout,"{");
	for (d=0;d<ns-1;d++)
	fprintf(fpout,"%d,",xs4[d]);
	fprintf(fpout,"%d}\n",xs4[ns-1]);
	fprintf(fpout,"{");
	for (d=0;d<ns-1;d++)
	fprintf(fpout,"\"%s\",",ys4[d]);
	fprintf(fpout,"\"%s\"}\n",ys4[ns-1]);
	fprintf(fpout,"{");
	for (d=0;d<ns-1;d++)
	fprintf(fpout,"\"%s\",",ts[d]);
	fprintf(fpout,"\"%s\"}\n",ts[ns-1]);
}
	fprintf(fpout,"attach %d %d\n",refx,refy);
fclose(fpout);
exit(0);
}
