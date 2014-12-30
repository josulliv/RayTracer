//		Textures.h  Contains all the texture object declarations.
//		Started on 11/25/1992, J. O'Sullivan

/* Texture types:
	0: No texture - object is the color stored in surface.color.
	1: Texture loaded from a disk file (Sun raster).
	2: Texture is a Mandelbrot zoom.
	3: Texture is a tiled surface.
*/

#ifndef _textures_h
#define _textures_h

#include "platform.h"
#include "rstrfile.h"	// Sun Microsystems rasterfile definitions

#ifdef MTRT
#include <thread.h>
#endif

class Texture		// An abstract class - the mother of all textures...
{
	public:

	int hres, vres;

	Texture(void);
	virtual Color getcolor(FP xx, FP yy) = 0;		// A pure virtual function
};

class Imagefile : public Texture
{
	protected:

#ifdef MTRT
	mutex_t io_mutex;
#endif
	unsigned char inmap[768];
	unsigned char pixel[4];
	char filename[130];
	int pixsize;
	long offset;
	FILE *infile;
	rasterfile rfile;
	char *inmapptr, *pixelptr, *rfileptr;
	Color palette[256];
	Boolean truecolor;

	public:

	Imagefile(void);
	void init(char *ifilename);
	Color getcolor(FP xx, FP yy);
	friend void loadScene(void);
	friend istream& operator >> (istream& s, Imagefile& i);
	friend ostream& operator << (ostream& s, Imagefile& i);
};

istream& operator >> (istream& s, Imagefile& i);
ostream& operator << (ostream& s, Imagefile& i);


class Mandelbrot : public Texture
{
	public:

	FP acorner, bcorner, side, gap;
	int type;
	Color palette[256];

	Mandelbrot(void);
	void init(int ihres, int ivres, int itype, FP iacorner, FP ibcorner, FP iside);
	Color getcolor(FP xx, FP yy);
	void setpalette(int type);
	friend void loadScene(void);
	friend istream& operator >> (istream& s, Mandelbrot& m);
	friend ostream& operator << (ostream& s, Mandelbrot& m);
};

istream& operator >> (istream& s, Mandelbrot& m);
ostream& operator << (ostream& s, Mandelbrot& m);

class Tile : public Texture
{
	public:

	Color oddcolor, evencolor;
	FP tilesize;

	Tile(void);
	void init(int ihres, int ivres, Color iodd, Color ieven, FP isize);
	Color getcolor(FP xx, FP yy);
	friend void loadScene(void);
	friend istream& operator >> (istream& s, Tile& t);
	friend ostream& operator << (ostream& s, Tile& t);
};

istream& operator >> (istream& s, Tile& t);
ostream& operator << (ostream& s, Tile& t);

#endif	// Of textures.h
