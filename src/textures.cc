//	Textures.cc  Contains all the texture function definitions.
//	Started on 11/25/1992

/* Texture types:
	0: No texture - object is the color stored in surface.color.
	1: Texture loaded from a disk file (Sun raster).
	2: Texture is a Mandelbrot zoom.
	3: Texture is a tiled surface.
*/

#include "raytrace.h"
#include "vector.h"
#include "miscobj.h"
#include "textures.h"
#include <string.h>

Texture::Texture(void)
{
}

void Imagefile::init(char *ifilename)
{
	strcpy(filename, ifilename);
}

Imagefile::Imagefile(void)
{
#ifdef RTMT
	mutex_init(&io_mutex, USYNC_THREAD, NULL);
#endif
}


Color Imagefile::getcolor(FP xx, FP yy)	// Read in a pixel
{
	int x, y;
	long pos;
	Color color;

	x = (int)((FP) hres * xx);
	y = (int)((FP) vres * (1.0 - yy));
	pos = offset + (y * hres + x) * pixsize;	// Locate the pixel
#ifdef MTRT
	mutex_lock(&io_mutex);
#endif
	fseek(infile, pos, 0x0);		// Go to the desired pixel
	fread(pixelptr, 0x1, pixsize, infile);	// Read the pixel
	if (truecolor == true)
	{
		if (rfile.ras_type != RT_STANDARD)
			color.init((FP) pixel[0] / 255.0, (FP) pixel[1] / 255.0,
			(FP) pixel[2] / 255.0);
		else
			color.init((FP) pixel[3] / 255.0, (FP) pixel[2] / 255.0,
			(FP) pixel[1] / 255.0);
	}
	else
		color = palette[pixel[0]];
#ifdef MTRT
	mutex_unlock(&io_mutex);
#endif
	return color;
}

istream& operator >> (istream& s, Imagefile& i)
{
	int x;

	s >> i.filename;
	i.rfileptr = (char *) &i.rfile;
	i.pixelptr = (char *) &i.pixel;
	i.inmapptr = (char *) &i.inmap;

	// First, open the image file:
	if ((i.infile = fopen(i.filename, "rb")) == NULL)
	{
		printf("\nThe file %s cannot be opened.  Exiting...\n\n", i.filename);
		exit(1);
	}

	// Read the rasterfile header
	fread(i.rfileptr, 0x4, 0x8, i.infile);

	if (i.rfile.ras_magic != RAS_MAGIC)
	{
		printf("\nWrong magic number in the image file - not a Sun raster!\n\n");
		exit(1);
	}

	i.hres = i.rfile.ras_width;
	i.vres = i.rfile.ras_height;

	if (i.rfile.ras_depth == 0x18)
	{
		i.truecolor = true;
		i.pixsize = 0x3;
	}
	else if (i.rfile.ras_depth == 0x20)
	{
		i.truecolor = true;
		i.pixsize = 0x4;
	}
	else if (i.rfile.ras_depth == 0x8)
	{
		i.truecolor = false;
		i.pixsize = 0x1;
	}

	if ((i.rfile.ras_type != RT_STANDARD) && (i.rfile.ras_type != RT_FORMAT_RGB))
	{
		printf("\nUnrecognized raster type: %d\n", i.rfile.ras_type);
		printf("Terminating...\n");
		exit(1);
	}
	
	if (i.rfile.ras_maptype > RMT_NONE)	// Read the palette
	{
		fread(i.inmapptr, i.pixsize, i.rfile.ras_maplength, i.infile);

		for (x = 0; x < (i.rfile.ras_maplength / 3); x++)
		{
			if (i.rfile.ras_type == RT_STANDARD)
			{	// Palette stored as all reds, all greens, then all blues, not rgbrgbrgb.
				i.palette[x].init((FP)i.inmap[x] / 255.0,
				(FP)i.inmap[x + i.rfile.ras_maplength / 3] / 255.0, (FP)i.inmap[x + i.rfile.ras_maplength / 3 * 2] / 255.0);
			}
			else
			{
				i.palette[x].init((FP)i.inmap[x + i.rfile.ras_maplength / 3 * 2] / 255.0,
				(FP)i.inmap[x + i.rfile.ras_maplength / 3] / 255.0, (FP)i.inmap[x] / 255.0);
			}
		}

/*
		for (i.x = 0; i.x < (i.rfile.ras_maplength / 3); i.x++)
		{
			if (i.rfile.ras_type == RT_STANDARD)
			{
				i.palette[i.x].init((FP)i.inmap[3][i.x] / 255.0,
				(FP)i.inmap[2][i.x] / 255.0, (FP)i.inmap[1][i.x] / 255.0);
			}
			else
			{
				i.palette[i.x].init((FP)i.inmap[0][i.x] / 255.0,
				(FP)i.inmap[1][i.x] / 255.0, (FP)i.inmap[2][i.x] / 255.0);
			}
		}
*/
	}

	i.offset = 0x20 + i.rfile.ras_maplength;	// The image beginning.
	return s;
}

ostream& operator << (ostream& s, Imagefile& i)
{
	s << i.filename << "\n";
	return s;
}


Mandelbrot::Mandelbrot(void)
{
}

void Mandelbrot::init(int ihres, int ivres, int itype, FP iacorner, FP ibcorner, FP iside)
{
	hres = ihres;
	vres = ivres;
	type = itype;
	acorner = iacorner;
	bcorner = ibcorner;
	side = iside;
}

Color Mandelbrot::getcolor(FP xx, FP yy)
{
	FP a, b, ac, bc, sa, sb;
	int count, stor;

	ac = xx * hres * gap + acorner;
	bc = yy * vres * gap + bcorner;
	a = ac;
	b = bc;
	count = 0;
	stor = 0;
	do
	{
		sa = a * a;
		sb = b * b;
		stor = count;
		if ((sa + sb) > 4)
			stor = 255;
		b = (a + a) * b + bc;
		a = sa - sb + ac;
		count++;
	}  while (stor < 255);
	count--;
	return palette[count];
}

void Mandelbrot::setpalette(int ptype)
{
	int x;

	switch (ptype)
	{
		case 1:
			{
				for (x = 0; x < 256; x++)
					palette[x].init((FP)x / 255.0, (FP)x / 255.0, (FP)x / 255.0);
				break;
			}
		default :	// Also case 0
			{
				srand(1);
				for (x = 0; x < 256; x++)
					palette[x].init((FP)x / 255.0, 1.0 - ((FP)x / 255.0), (FP)rand() / 65536.0);
				break;
			}
	}
}

istream& operator >> (istream& s, Mandelbrot& m)
{
	s >> m.hres >> m.vres >> m.type >> m.acorner >> m.bcorner >> m.side;
	m.gap = m.side / (FP) m.vres;
	m.setpalette(m.type);
	return s;
}

ostream& operator << (ostream& s, Mandelbrot& m)
{
	s << m.hres << "\n" << m.vres << "\n" << m.type << "\n" << m.acorner
	<< "\n" << m.bcorner << "\n" << m.side << "\n";
	return s;
}

Tile::Tile()
{
}

void Tile::init(int ihres, int ivres, Color iodd, Color ieven, FP isize)
{
	hres = ihres;
	vres = ivres;
	oddcolor = iodd;
	evencolor = ieven;
	tilesize = isize;
}

Color Tile::getcolor(FP xx, FP yy)
{
	if ((((int)(xx * hres / tilesize) ^ (int)(yy * vres / tilesize)) & 1) == 1)
		return oddcolor;
	else
		return evencolor;
}


istream& operator >> (istream& s, Tile& t)
{
	s >> t.hres >> t.vres >> t.oddcolor >> t.evencolor >> t.tilesize;
	return s;
}

ostream& operator << (ostream& s, Tile& t)
{
	s << t.hres << "\n" << t.vres << "\n" << t.oddcolor << t.evencolor
	<< t.tilesize << "\n";
	return s;
}

