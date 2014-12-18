// BMP.H  Header file for Windows bitmap files
// This is the 2.x (Win16) version.

#ifndef bmp_h
#define bmp_h

#include <stdio.h>

class Bmp
{
	public:

	char bfType1, bfType2;		// File type = must be 0x424d
	long bfSize;			// File size in 32-bit words
	int bfReserved;			// Reserved - must be 0x0.
	long bfOffBits;			// Offset in bytes to image.

	long biSize;			// Number of bytes required by the header (0x40)
	long biWidth;			// Bitmap width in pixels
	long biHeight;			// Bitmap height in pixels
	int biPlanes;			// Must be set to 0x1.
	int biBitCount;			// Number of bits per pixel (1, 4, 8, 24)
	long biCompression;		// 0x0 for uncompressed
	long biSizeImage;			// Size (in bytes) of the image
	long biXPelsPerMeter;
	long biYPelsPerMeter;
	long biClrUsed;			// Number of the colors in the palette used.
	long biClrImportant;		// Number of those colors that are important.

	Bmp(void)
	{
		bfType1 = 0x42;
		bfType2 = 0x4d;
		bfSize = 0x36;
		bfReserved = 0x0;
		bfOffBits = 0x36;

		biSize = 0x28;		// 9 longs (32 bits) + 2 ints (16 bits)
		biWidth = 0x0;
		biHeight = 0x0;
		biPlanes = 0x1;
		biBitCount = 0x18;	// 24 bits per pixel
		biCompression = 0x0;
		biSizeImage = 0x0;
		biXPelsPerMeter = 0x0;
		biYPelsPerMeter=0x0;
		biClrUsed = 0x0;
		biClrImportant = 0x0;
	}

	void writeheader(FILE *outfile);
};

#endif
