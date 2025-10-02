struct xfig_line {
	int	object;		/* (always 2) */
	int	sub_type;		/* (1: polyline */
				/*  2: box */
				/*  3: polygon */
				/*  4: arc-box) */
				/*  5: imported-picture bounding-box) */
	int	line_style;		/* (enumeration type) */
	int	thickness;		/* (1/80 inch) */
	int	pen_color;		/* (enumeration type, pen color) */
	int	fill_color;		/* (enumeration type, fill color) */
	int	depth;			/* (enumeration type) */
	int	pen_style;		/* (pen style, not used) */
	int	area_fill;		/* (enumeration type, -1 = no fill) */
	float	style_val;		/* (1/80 inch) */
	int	join_style;		/* (enumeration type) */
	int	cap_style;	/* (enumeration type, only used for POLYLINE) */
	int	radius;		/* (1/80 inch, radius of arc-boxes) */
	int	forward_arrow;		/* (0: off, 1: on) */
	int	backward_arrow;		/* (0: off, 1: on) */
	int	npoints;			/* (number of points in line) */

};

extern struct xfig_line figline;

struct xfig_text {
	int	object;		/* (always 4) */
	int	sub_type;		/* (0: Left justified */
				/*      1: Center justified */
				/*      2: Right justified) */
	int	color;			/* (enumeration type) */
	int	depth;			/* (enumeration type) */
	int	pen_style;		/* (enumeration , not used) */
	int	font;			/* (enumeration type) */
	int	font_size;		/* (font size in points) */
	float	angle;		/* (radians, the angle of the text) */
	int	font_flags;		/* (bit vector) */
	float	height;		/* (Fig units) */
	float	length;		/* (Fig units) */

};

extern struct xfig_text figtext;

struct xfig_ellipse {
	int	object_code;		/* (always 1) */
	int	sub_type;		/* (1: ellipse defined by radiuses */
				/*  2: ellipse defined by diameters */
				/*  3: circle defined by radius */
				/*  4: circle defined by diameter) */
	int	line_style;		/* (enumeration type) */
	int	thickness;		/* (1/80 inch) */
	int	pen_color;		/* (enumeration type, pen color) */
	int	fill_color;		/* (enumeration type, fill color) */
	int	depth;			/* (enumeration type) */
	int	pen_style;		/* (pen style, not used) */
	int	area_fill;		/* (enumeration type, -1 = no fill) */
	float	style_val;		/* (1/80 inch) */
	int	direction;		/* (always 1) */
	float	angle;		/* (radians, the angle of the x-axis) */
	int	center_x, center_y;	/* (Fig units) */
	int	radius_x, radius_y;	/* (Fig units) */
	int	start_x, start_y;	/* (Fig units; the 1st point entered) */
	int	end_x, end_y;		/* (Fig units;the last point entered) */

};

extern struct xfig_ellipse figellipse;
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <gdk/gdkx.h>
#include <X11/Xlib.h>

extern double pdbxcent,pdbycent,pdbzcent;

extern struct xy_co *bond_cut(int x, int y, int tx, int ty, int r);

extern int has_label (int, int);
