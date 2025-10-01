#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#undef __GNUC__
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/stat.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <gtk/gtk.h>
#pragma GCC diagnostic pop

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
#define BONDTYPES 22
#define BONDCOLORS 8

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
struct data {
            int x,y;
            int tx,ty;
	    int bond;
            int smarked,tmarked;
	    int decoration;
	    int color;
	    struct data *next;
	    struct data *prev; 
	    } da_root, *new
	    ;

struct dc {
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

struct data_head {
	    int pix_width, pix_height;
	    int width, height;
	    } head;
  
struct new {
	   int x,y;
	   int tx,ty;
	   int startx,starty;
	   int n, nc;
	   int nsp; 
	   } *hp; 

struct xy_co {
	    int x,y;
	    int tx,ty;
	    } xycoord[30];

struct marked {
	   int x,y;
	   int w, h;
	   int flag;
	   } mark; 

struct spline {
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

float size_factor; 
int zoom_factor;

int draw_angle;
int text_direct;
int font_type;
int line_type;
int modify;
int addflag;
int xbmflag;
int drawmode;
int xref,yref;
char *paper[11]={"A4","A3","A2","A1","A0","Letter",
		"Legal","Ledger","C","D","E"};
int papersize;
char *orientation[2]={"-p 1","-l 1"};
int orient;
float printscale;
int printcmd;
char *printcommand[4]={"lpr -P","lp -d","kprinter -d","gtklp -d"};
char *queuename;
char datadir[PATH_MAX];
char datamask[PATH_MAX];
int figversion;
int importflag;
int pdbmode;

void Clear(void);

/*	chemproc.c	*/

extern struct data *select_vector(int mx, int my, float s_f);
extern void add_struct(int x,int y,int tx, int ty, int bond, int smarked, int tmarked, int decoration, int color);
extern int round_coord(int i, float step);
extern void del_char(struct dc *hpc);
extern void del_struct(struct data *hpc);
extern void add_char(int x,int y, char *c, int direct, int marked, int color, int font, int size);
extern struct dc *select_char(int mx, int my, float s_f);
extern struct xy_co *calc_angle(int x, int y, int ang_type, int angle, int factor);
extern struct xy_co *position(int x, int y, int ax, int ay, int ang_type);
extern void setup_data(void);
extern void clear_data(void);
extern int partial_delete(int x, int y, int tx, int ty);
extern void center_mol(void);
extern int move_pos(int x, int y, int tx, int ty);
extern int check_pos_rec(int x, int y, int rx, int ry, int rtx, int rty);
extern int partial_move(int x, int y, int tx, int ty, int dx, int dy);
extern int partial_rotate(/*int xcent, int ycent,*/ double rotsin, double rotcos);

extern int calc_vector(int dx, int dy);
extern struct xy_co *multi_bonds(int mx, int my, int mtx, int mty, int r);
extern struct xy_co *center_double_bond(int mx,int my,int mtx, int mty, int r);
extern struct xy_co *intersect(int ax, int ay, int atx, int aty, int bx, int by, int btx, int bty);
struct xy_co *bond_cut(int x, int y, int tx, int ty, int r);
extern void add_box1(void);
extern void add_box2(void);
extern void add_box3(void);
extern void add_box4(void);

extern void Unmark_all(void);
extern void get_center_marked (int*, int*);
/*	graph.c	*/

extern void CreatePix(void);
extern void FreePix(void );
extern void CopyPlane(void );
extern void Display_Mol(void);
extern void Drawline( int , int, int, int , int, int );
extern void DrawWide( int, int, int, int, int,int);
extern void DrawWedge( int, int, int, int, int,int);
extern void DrawDashedWedge( int, int, int, int, int,int);
extern void DrawArrow( int, int, int, int, int,int,int);
extern void DrawWiggly( int, int, int, int, int,int);
extern void Drawstring( int, int, char*, int, int, int, int, int, int);
extern int Extra_char(char c);
extern int Load_Font(void);
extern void Set_Line(int);

/*	draw.c	*/

extern void Add_double(int, int);
extern void Add_vector(int);
extern void Del_vector( int, int);
extern void Set_vector( int, int,int);
extern void Put_vector( int, int);
extern void Invert_vector(int, int);
extern void Add_atom( int, int);
extern void Add_number( int, int);
extern void Fetch_atom( int, int);
extern void Del_atom( int, int);
extern void Set_start_rec( int, int);
extern void Put_rec(int, int);
extern void Del_rec(void);
extern void Mark_rec(int);
extern void Mark_atom(int, int);
extern void Center(void);
extern void Put_pmove(int, int,int);
extern void Set_pmove(int, int);
extern void Put_protate( int, int, int);
extern void Put_pscale( int, int, int);
extern void Add_ring (int, int, int);

/*	dialog.c	*/

extern void Load(void);
extern void Add(void);
extern void Save(void);
extern void Quit(void);
void Zoom(GtkWidget*, gpointer);
void Change_Text(GtkWidget*, gpointer);
void Change_Font(GtkWidget*);
void Change_Angle(GtkWidget*, gpointer);
extern void flip_horiz(void);
extern void flip_vert(void);
extern void copy_obj(void);
/*	inout.c	*/

extern int save_mol(FILE *fp, int partial);
extern int load_mol(char *filename);
extern int import_mdl_mol(char *filename,int skip);
extern int import_pdb(char *filename);
extern int import_babel(char *filename);
extern int add_mol(char *filename);
extern int export_bitmap(char *filename);
extern int export_xfig(char *filename);
extern int export_latex_pic(char *filename, float scalefactor);
extern int export_svg(char *filename);
extern int export_latex_pic(char *filename, float scalefactor);
extern int export_ps_pic(char *filename, float scalefactor);
extern int export_png_pic(char *filename, float scalefactor);
extern int print_ps_pic(void);
extern int export_fw(char *filename);
extern int export_mdl_mol(FILE *fp, int topipe);
extern int export_emf(char *filename);
extern int export_sxd(char *filename);
extern int export_babel(char *filename);
extern int export_asy(char *filename);
extern int readrc(void);
extern int writerc(void);
extern void pdbrotate(int,int,int);

#ifdef LIBUNDO
extern int undo_get_undo_count();
extern int undo_get_redo_count();
extern int undo_undo();
extern int undo_undo();
extern int undo_redo();
extern int undo_snapshot();
extern int undo_new(char*);
extern int undo_destroy();
extern int undo_set_memory_limit(size_t memmax);
#endif

GdkPixmap *picture,*savedpicture,*preview;
GtkWidget *textbox;
int xcent,ycent;
int *tmpx,*tmpy;
char formula[51],weight[21],eweight[21],compos[65];
  
void yesnodialog (void (*)(void));
void CheckAndClear(void);
void yesnodialog2 (void (*)(void),char*);
void do_fw(void);
void Grid(GtkWidget*);
void change_color(GtkWidget*, gpointer);
void set_bond (GtkWidget* , gpointer);
void Bondmode (GtkWidget*);
void Movemode (GtkWidget*);
void Markmode (GtkWidget*);
void Rotatemode (GtkWidget*);
void Rescalemode (GtkWidget*);
void show_or_raise (GtkWidget*);
void Add_template (GtkWidget*, gpointer);
void exp_mode (GtkWidget*, gpointer);
void do_export (GtkWidget*,GtkFileSelection*);
void really_export(void);
void file_ok_sel(GtkWidget*,GtkFileSelection*);
void reallysave(void);
void CheckAndLoad(void);
int CheckAndQuit(void);
void Set_bondtype(int, int, int);
void tidy_mol(void);
void do_undo(void);
void do_redo(void);
int newpaper(GtkWidget*,gpointer);
int neworient(GtkWidget*,gpointer);
int newprcmd(GtkWidget*,gpointer);
void Set_Textmode(GtkWidget*,GdkEvent*);
void Splinemode(GtkWidget*);
void Import(void);
void Import_PDB(void);
void SaveAs(void);
void Export(void);
int options_dialog_ok(GtkWidget*, gpointer);
void prepare_options_dialog(void);
int print_setup_menu_activate(GtkWidget*, gpointer);
int babelcmd(GtkWidget*, gpointer);
void Import_Babel(void);
void Export_Babel(void);
void pdb_mode(GtkWidget*, gpointer);
void sdf_mode(GtkWidget*, gpointer);

extern void add_bracket(void);
extern void add_r_bracket(void);
extern void add_r2_bracket(void);
extern void add_brace(void);

void add_spline(int, int, int, int, int, int, int, int, int, int, int);
void show_boxmenu(void);
void show_penmenu(void);
void print_ps(void);
void setup_printer(GtkWidget*,gpointer);
void do_complete(void);
void check_fig2dev(void);
extern void check_babel(void);
extern void check_fig2sxd(void);
void ct_crash(int);
void restore_picture(void);
void getpreview(GtkWidget*, gint, gint,  GdkEventButton*, gpointer);
void set_icon(GtkWidget*);
extern int load_preview(char*);
extern int preview_mdl_mol(char*, int);
extern void pdbstore(void);
double *pdbx, *pdby,*pdbz;
int *bondfrom,*bondto;
short *bondtype,*atjust;
char **atcode;
int pdbn=0;
int nbonds=0;
float importfactor=50.;
int importoffset=500;

extern char **intype;
extern char **inmode;
extern int babelin;
extern char **outtype;
extern char **outmode;
extern int babelout;
char* babel;
GdkGC *mygc[8],*background_gc,*hlgc;
int curpen=0;
int gridtype=0;
int gridx=0;
int gridy=0;
int serif_flag=0;
int refx=0,refy=0,refmark=0;
double bondlen_mm=10.668;
int db_dist=4;
int mb_dist=10;
int epsoption, use_whiteout,use_intlchars;
int bgred,bggreen,bgblue;
char bghexcolor[10];
int curfontsize=3;

extern int undo_malloc(size_t);
int atnum;
int draw_ok=0;
int sdfindex=0;
int have_fig2sxd;
