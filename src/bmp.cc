// bmp.cc	Member function definitions for the Bmp class

#include "bmp.h"
#include "raytrace.h"

void Bmp::writeheader(FILE *outfile)
{
	// First, write out the bitmap file header

	fwrite(&bfType1, 1, 1, outfile);
	fwrite(&bfType2, 1, 1, outfile);
	fwrite(&bfSize, 4, 1, outfile);
	fwrite(&bfReserved, 2, 1, outfile);
	fwrite(&bfReserved, 2, 1, outfile);
	fwrite(&bfOffBits, 4, 1, outfile);

	// Then write out the bitmap info header

	fwrite(&biSize, 4, 1, outfile);
	fwrite(&biWidth, 4, 1, outfile);
	fwrite(&biHeight, 4, 1, outfile);
	fwrite(&biPlanes, 2, 1, outfile);
	fwrite(&biBitCount, 2, 1, outfile);
	fwrite(&biCompression, 4, 1, outfile);
	fwrite(&biSizeImage, 4, 1, outfile);
	fwrite(&biXPelsPerMeter, 4, 1, outfile);
	fwrite(&biYPelsPerMeter, 4, 1, outfile);
	fwrite(&biClrUsed, 4, 1, outfile);
	fwrite(&biClrImportant, 4, 1, outfile);
}
