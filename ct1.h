/*@ignore@  splint claims the structs are redefined - i do not think so */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/stat.h>
#include <gtk/gtk.h>

#ifndef MAXPATHLEN
#  ifdef PATH_MAX
#    define MAXPATHLEN PATH_MAX
#  else
#    define MAXPATHLEN 2048
#    define PATH_MAX MAXPATHLEN
#  endif
#endif

#define PIXMAPWIDTH 1600
#define PIXMAPHEIGHT 1200

#define MAXCL 101
#define Hex 1    /* define angletypes and textdirect. , DONT CHANGE THEM !*/
#define Pent 2
#define Pent_switch 3
#define Octa 4
#define No_angle 0
#define Left_Text 0 
#define Middle_Text -1
#define Right_Text -2
#define False 0
#define True 1
extern struct data {
            int x,y;
            int tx,ty;
	    int bond;
            int smarked,tmarked;
	    int decoration;
	    int color;
	    struct data *next;
	    struct data *prev; 
	    } da_root, *new;

extern struct dc {
	    int x,y;
	    char c[MAXCL];
	    int direct;
	    int marked;
            int color;
            int font;
	    int size;
	    struct dc *next;
	    struct dc *prev; 
  	  } dac_root, *new_c;

extern struct data_head {
	    int pix_width, pix_height;
	    int width, height;
	    } head;
  
extern struct new {
	   int x,y;
	   int tx,ty;
	   int startx,starty;
	   int n, nc;
	   int nsp;
	   } *hp; 

extern struct xy_co {
	    int x,y;
	    int tx,ty;
	    } xycoord[30];

extern struct marked {
	   int x,y;
	   int w, h;
	   int flag;
	   } mark; 

extern struct spline {
            int x0,y0;
            int x1,y1;
            int x2,y2;
            int x3,y3;
	    int type;
            int marked;
            int color;
	    struct spline *next;
	    struct spline *prev; 
	    } sp_root, *sp_new;

extern float size_factor; 
extern int zoom_factor;

extern int draw_angle;
extern int text_direct;
extern int font_type;
extern int line_type;
extern int modify;
extern int addflag;
extern int xbmflag;
extern int drawmode;
extern int xref,yref;
extern int papersize;
extern char *paper[11];
extern char *orientation[2];
extern int orient;
extern float printscale;
extern int printcmd;
extern char *printcommand[4];
extern char *queuename;
extern char datadir[PATH_MAX];
extern char datamask[PATH_MAX];
extern int figversion;
extern int importflag;
extern int pdbmode;

/*	chemproc.c	*/

extern struct data *select_vector(int mx, int my, float s_f);
extern void add_struct(int x,int y,int tx, int ty, int bond, int smarked, int tmarked, int decoration, int color);
extern int round_coord(int i, float step);
extern void del_char(struct dc *hpc);
extern void del_struct(struct data *hpc);
extern void del_spline(struct spline *);
extern void add_char(int ,int , char *, int , int, int, int, int);
extern struct dc *select_char(int mx, int my, float s_f);
extern struct xy_co *calc_angle(int x, int y, int ang_type, int angle, int);
extern struct xy_co *position(int x, int y, int ax, int ay, int ang_type);
extern void setup_data(void);
extern void clear_data(void);
extern int partial_delete(void);
extern void center_mol(void);
extern int move_pos(int x, int y, int tx, int ty);
extern int check_pos_rec(int x, int y, int rx, int ry, int rtx, int rty);
extern int partial_move(int dx, int dy);
extern int partial_rotate(double rotsin, double rotcos);
extern int partial_rescale(int updown, int aniso);

extern int calc_vector(int dx, int dy);
extern struct xy_co *multi_bonds(int mx, int my, int mtx, int mty, int r);
extern struct xy_co *center_double_bond(int mx,int my,int mtx, int mty, int r);
extern struct xy_co *intersect(int ax, int ay, int atx, int aty, int bx, int by, int btx, int bty);
extern struct xy_co *bond_cut(int x, int y, int tx, int ty, int r);
extern void cut_end (int *x, int *y, int tx, int ty, int r);
extern void get_center_marked (int*, int*);
extern void add_box1(void);
extern void add_box2(void);
extern void add_box3(void);
extern void add_box4(void);
extern void add_bracket(void);
extern void add_r_bracket(void);
extern void add_r2_bracket(void);
extern void add_brace(void);

 /*	graph.c	*/

extern void CreatePix(void );
extern void FreePix(void );
extern void CopyPlane(void );
extern void Display_Mol(void);
extern void Drawline( int , int, int, int , int, int );
extern void DrawWide( int, int, int, int, int,int);
extern void DrawStripe( int, int, int, int, int,int);
extern void DrawWedge( int, int, int, int, int,int);
extern void DrawDashedWedge( int, int, int, int, int,int);
extern void DrawArrow( int, int, int, int, int,int,int);
extern void DrawWiggly( int, int, int, int, int,int);
extern void Drawstring( int, int, char*, int, int, int, int, int, int);
extern void DrawCircle( int, int, int, int, int,int);
extern void DrawDotted( int , int, int, int , int, int );
extern void DrawAcross( int , int, int, int , int , int);
#ifdef GTK2
extern int Extra_char(gunichar c, int ha);
#else
extern int Extra_char(char c);
#endif
extern int Load_Font(void);
extern void Set_Line(int);

/*	draw.c	*/

extern void Add_double( int, int);
extern void Add_vector(int);
extern void Add_ring(int, int, int);
extern void Del_vector( int, int);
extern void Set_vector( int, int,int);
extern void Put_vector( int, int);
extern void Invert_vector( int, int);
extern void Add_atom( int, int);
extern void Add_number( int, int);
extern void Fetch_atom( int, int);
extern void Del_atom( int, int);
extern void Set_start_rec( int, int);
extern void Put_rec( int, int);
extern void Del_rec(void);
extern void Mark_rec(int);
extern void Mark_atom( int, int);
extern void Center(void);
extern void Put_pmove( int, int,int);
extern void Set_pmove( int, int);
extern void Put_protate( int, int, int);
extern void Set_pscale ( int, int);
extern void Put_pscale( int, int,int);
extern void Unmark_all (void);
extern void Set_bondtype(int, int, int);

/*	dialog.c	*/

extern void Load(void);
extern void Add(void);
extern void Save(void);
extern void Quit(void);
extern void Zoom(GtkWidget*, gpointer);
extern void Change_Text(GtkWidget*, gpointer);
extern void Change_Angle(GtkWidget*, gpointer);
extern void flip_horiz(void);
extern void flip_vert(void);
extern void copy_obj(void);
/*	inout.c	*/

extern int save_mol(FILE *fp, int partial);
extern int load_mol(char *filename);
extern int add_mol(char *filename);
extern int export_bitmap(char *filename);
extern int export_xfig(char *filename);
extern int export_svg(char *filename);
extern int export_mdl_mol(FILE *fp, int topipe);
extern int load_preview(char *filename);
extern void pdbstore(void);
extern int preview_mdl_mol(char *filename, int skip);
extern int import_babel(char *filename);
extern int export_emf(char *filename);
extern int export_png_pic(char *filename, float scalefactor);
extern int import_mdl_mol(char *filename,int skip);
extern int import_pdb(char *filename);
extern int export_latex_pic(char *filename,float scalefactor);
extern int export_ps_pic(char *filename, float scalefactor);
extern int print_ps_pic(void);
extern void xfig_line (FILE*, int, int, int, int); 
extern void xfig_wedge (FILE*, int, int, int, int, int, int); 
extern void xfig_wiggly (FILE*, int, int, int, int);
extern void xfig_arrow (FILE*, int, int, int, int, int, int); 
extern void xfig_circle (FILE*, int, int, int, int);
extern void xfig_spline (FILE *, int , int , int , int , int , int , int , int , int, int);
extern int exfig(FILE*, int);
extern int export_fw(char*);
extern int export_sxd(char*);
extern int export_babel(char*);
extern int readrc(void);
extern int writerc(void);
extern void check_babel(void);
extern int export_asy(char *filename);
extern void check_fig2dev(void);
extern void check_fig2sxd(void);

#ifdef LIBUNDO
extern void undo_free(void*);
extern void* undo_malloc(size_t);
extern int undo_snapshot();
#endif

extern GdkPixmap *picture;
extern GtkWidget *drawing_area,*preview_area;
extern GtkWidget *textbox;
extern int xcent,ycent;
extern int *tmpx,*tmpy;
extern char formula[51],weight[21],eweight[21],compos[65];
extern void DrawDashed( int , int, int, int , int, int );
extern void tidy_mol(void);
extern void Drawspline (int, int, int, int, int, int, int, int, int, int, int);
extern void add_spline (int, int, int, int, int, int, int, int, int, int, int);
extern void draw_preview_bonds(int, int, int, int, int);
extern void pdbrotate(int,int,int);
extern double *pdbx,*pdby,*pdbz;
extern int *bondfrom,*bondto;
extern short *bondtype,*atjust;
extern char **atcode;
extern int pdbn;
extern int nbonds;
extern float importfactor;
extern int importoffset;

extern char **intype;
extern char **inmode;
extern int babelin;
extern char **outtype;
extern char **outmode;
extern int babelout;
extern char *babel;
extern GdkGC *mygc[8],*background_gc,*hlgc;
extern int curpen;
extern int gridtype;
extern int gridx,gridy;
extern int serif_flag;
extern int refx,refy,refmark;
extern double bondlen_mm;
extern int db_dist;
extern int mb_dist;
extern int epsoption, use_whiteout,use_intlchars;
extern int bgred,bggreen,bgblue;
extern char bghexcolor[10];
extern int curfontsize;
extern int atnum;
extern int draw_ok;
extern int sdfindex;
extern int have_fig2sxd;
extern int has_label(int,int);
/*@end@*/
