//  raytrace.cc   Started 12/25/1990  J. O'Sullivan

#include "platform.h"
#include "raytrace.h"
#include "bmp.h"			// Windows BMP file object
#include "vector.h"		// Vector-related objects and functions
#include "miscobj.h"		// Miscellaneous objects
#include "lights.h"		// Light objects
#include "textures.h"	// Texture objects
#include "object.h"		// Object abstract-class declaration
#include "planar.h"		// Planar objects
#include "quadric.h"		// Quadric-related objects
#include "octree.h"		// Octree-related stuff (voxels, etc.)
#include "scene.h"		// LoadScene

#include <time.h>			// (ANSI)
#include <string.h>		// (ANSI)  for strchr in main()
#include <signal.h>		// For catching floating-point errors

#ifdef SUNOS
#include <floatingpoint.h>	// SIGFPE exception handler (ieee_handler)
#include "xplot/color.h"	// COLOR definition from Radiance
#include "xplot/standard.h"	// From radiance
extern "C" {
#include "xplot/driver.h"	// From radiance
}
#endif

#ifdef MSDOS
#include <dos.h>
#include <conio.h>
#include <float.h>			// For signal handler constant definitions
#endif

// ****  Function Headers  ****

void scan(char *outfilename);
void trace(Ray aray, Node *rootptr, FP weight, int level);
Color illumination(Point& poi, Vector& normal);
Color illuminate(Node *rootptr, FP weight);
void deleteTree(Node *nodeptr, Boolean root);
void catcher(int exceptionType, int exceptionError);

#ifdef SUNOS
extern "C" {
void devopen(char *dname);
}

char *devname = dev_default;		// For X11 display
struct driver *dev = NULL;			// For X11 display
COLOR tempcolor = {0.0,1.0,0.0};	// Used with X11 routines
#endif


#ifdef MSDOS
extern unsigned _stklen = 32768U;	// Increase the stack size for BC.
#endif

int maxLevel, hres, vres, numberOfObjects, numberOfLights, numberOfTextures;
int display, storage, order, fov;
int bytes_per_pixel, supersample, startingline, numlines;
Object *objptr[MAXOBJ];
Light *lightptr[16];
Texture *textptr[256];
Ray camera;
Color color, acolor, backgroundColor, ambient;
Vector scrnx, scrny, firstray, up;
Bmp bmp;
FP aspect, hdeflect, scalex, scaley, minlen2;
FILE *outfile;
rasterfile rfile;		// Declare an instance of the rasterfile header struct

int objtype[MAXOBJ];	// Object type codes
int textype[64];		// Texture type codes
int lightype[16];		// Light type codes
Boolean used_by_scenebuilder = false;

int threshold, numberOfVoxels = 0;
Voxel rootvoxel;
Boolean use_octree;

int main(int argc, char *argv[])
{
	char *ptr, bufs[130], outfilename[130];
	int x;
	time_t tstart, tend, tloc;

	if (argc == 1)
	{
		cout << "Auto-restarting has not been implemented yet.  Call this with an SDF filename!\n\n";
		exit(1);
	}

	strcpy(bufs, argv[1]);
	ptr = strchr(bufs, 0);
	if (ptr)
		*ptr = 0;	// Terminate the string at the first space.

	if (argc == 2)	// If there's only the .sdf filename, use default dest.
		strcpy(outfilename, bufs);

	if (argc == 3)	// If there are two parameters (the sdf & destination)
		strcpy(outfilename, argv[2]);

	strcat(bufs, ".sdf");		// Slap on an extension on the SDF file name

#ifdef SUNOS
	// Next, set up a SIGFPE handler (ingore divide by zero errors):
	signal(SIGFPE, (void (*)(int))catcher);
	signal(0x0e, (void (*)(int))catcher);		// Perhaps to catch other sigs.
#endif // SUNOS

	numberOfTextures = 1;	// 0 = no texture
	printf("Now loading the scene from the scene description file.\n\n");
	loadScene(bufs);

	if (threshold == 0)
		threshold = 16;		// Set the default threshold value.

	printf("Read in %d objects.\n", numberOfObjects);

	if (numberOfObjects > threshold)
	{
		use_octree = true;
		printf("Now building the octree...\n\n");

		tstart = time(&tloc);
		buildOctree();
		printf("Finished building the octree, which contains %d voxels.\n\n", numberOfVoxels);
		tend = time(&tloc);
		printf("Elapsed time: %ld seconds.\n\n", (tend - tstart));

		if (numberOfVoxels * threshold < numberOfObjects)
		{
			printf("Something's rotten in Denmark.  There are fewer objects in the octree\n");
			printf("than there are in the scene...\n\n");
		}
	}
	else
		use_octree = false;

	// Delete all the space used for storing the original polygon vertices:

	for (x = 0; x < numberOfObjects; x++)
	{
		if (objtype[x] == 7)	// If it's a polygon
			delete ((Polygon *)objptr[x])->vertex;
	}

	printf("Beginning the trace operation...\n\n");
	tstart = time(&tloc);

	scan(outfilename);

	tend = time(&tloc);
	printf("\n\nElapsed time: %ld seconds.\n\n", (tend - tstart));
	if (display == 3)
	{
		printf("Press any key to exit...\n");
		gets((char *)&bufs);
	}
}


void scan(char *outfilename)
{
	Node *rootptr;
	FP xp, yp, jx, jy;
	int pixel, x, y, yy, sx, sy;
	Color pcolor;
	Ray aray;
	char zero = 0x0;
	char *rfileptr, *rowptr;
	unsigned char row[8192];

	rfileptr = (char *) &rfile;
	rowptr = (char *) &row;

	if (!(rootptr = new Node))
	{
		printf("\nInsufficient memory to allocate space for the root node.\n");
		exit(1);
	}

	if (storage == 1)	// If the image data should be stored in raw 24-bit format
	{
		strcat(outfilename, ".rif");	// Append the extension
		if ((outfile = fopen(outfilename, "wb")) == NULL)
		{
			printf("\nThe file %s cannot be opened.  Exiting...\n\n", outfilename);
			exit(1);
		}
	}

	if ((storage == 2) || (storage == 3))	// Store in Sun rasterfile format
	{
		strcat(outfilename, ".rif");	// Append the extension
		if ((outfile = fopen(outfilename, "wb")) == NULL)
		{
			printf("\nThe file %s cannot be opened.  Exiting...\n\n", outfilename);
			exit(1);
		}
		rfile.ras_magic = RAS_MAGIC;
		rfile.ras_width = hres;
		rfile.ras_height = vres;
		rfile.ras_depth = bytes_per_pixel * 8;
		rfile.ras_length = hres * vres * bytes_per_pixel;
		rfile.ras_type = RT_STANDARD;
		rfile.ras_maptype = RMT_NONE;
		rfile.ras_maplength = 0x0;

		// Write the rfile structure - 8 words of 4 bytes each.
		fwrite(rfileptr, 0x4, 0x8, outfile);
	}

	if (storage == 5)	// Store in Windows BMP format
	{
		strcat(outfilename, ".bmp");	// Append the extension
		if ((outfile = fopen(outfilename, "wb")) == NULL)
		{
			printf("\nThe file %s cannot be opened.  Exiting...\n\n", outfilename);
			exit(1);
		}

		bmp.bfSize = (long) hres * (long) bytes_per_pixel * (long) vres + 54;

		bmp.biWidth = (long) hres;
		bmp.biHeight = (long) vres;
		bmp.biBitCount = 0x18;	// 24 bits per pixel

		// Write the header:
		bmp.writeheader(outfile);
	}

#ifdef SUNOS
	if (display == 3)	// If the X11 display option is selected
	{
		devopen(devname);
		(*dev->clear)(hres, vres);
		(*dev->flush)();
	}
#endif

	srand(1);
	for (y = startingline; y < (numlines - startingline); y++)
	{
		if (order == 0)
			yy = y - (vres / 2) + 1;
		else
			yy = (vres / 2) - y - 1;

		if ((display == 0) || (display == 3))
			printf("Row being computed: %d    \r", (vres - y - 1));

		yp = yy;

		pixel = 0;
		for (x = 0; x < hres; x++)
		{
			xp = x;

			if (y == 216)
				xp = x;

			if (supersample == 0)	// No supersampling
			{
				rootptr->entering = true;
				aray.init(camera.origin, firstray
				- (scrnx * xp) - (scrny * yp));
				trace(aray, rootptr, 1.0, 0);
				pcolor = illuminate(rootptr, 1.0);
				deleteTree(rootptr, true);
			}
			else if (supersample == 1)	// 4x supersampling
			{
				pcolor.init(0.0, 0.0, 0.0);
				for (sx = 0; sx < 2; sx++)	// Subpixel x, 0 - 1
				{
					for (sy = 0; sy < 2; sy++)	// Subpixel y, 0 - 1
					{
						rootptr->entering = true;

						// Next, add jitter to the ray direction.  Compute a
						// random number between 0.0 and half-pixel-size.

						// Pseudo-random #'s from -0.25 to 0.25:
						jx = ((FP)rand() / 65535.0) - 0.25;
						jy = ((FP)rand() / 65535.0) - 0.25;

						// Add the column number, a quarter pixel or .75 pixel,
						// and +- 0.25 pixel jitter:

						aray.init(camera.origin, firstray
						- (scrnx * (xp + 0.25 + jx + (FP)sx * 0.5))
						- (scrny * (yp + 0.25 + jy + (FP)sy * 0.5)));
						trace(aray, rootptr, 1.0, 0);
						pcolor = pcolor + illuminate (rootptr, 1.0);
						deleteTree(rootptr, true);
					}
				}
				pcolor.scale(4.0);		// Average the four subpixels...
			}
			else if (supersample == 2)	// 9x (3x3) supersampling w/ Bartlett window
			{
				pcolor.init(0.0, 0.0, 0.0);
				for (sx = 0; sx < 3; sx++)	// Subpixel x, 0 - 2
				{
					for (sy = 0; sy < 3; sy++)	// Subpixel y, 0 - 2
					{
						rootptr->entering = true;

						// Next, add jitter to the ray direction.  Compute a
						// random number between 0.0 and half-pixel-size.

						// # from -1/6 to 1/6:
						jx = ((FP)rand() / 98301.0) - 0.166666666667;
						jy = ((FP)rand() / 98301.0) - 0.166666666667;

						aray.init(camera.origin, firstray
						- (scrnx * (xp + 0.166666666666667 + jx + (FP)sx * 0.33333333333))
						- (scrny * (yp + 0.166666666666667 + jy + (FP)sy * 0.33333333333)));
						trace(aray, rootptr, 1.0, 0);

						if ((sx == 1) && (sy == 1))
							pcolor = pcolor + (illuminate(rootptr, 1.0) * 4.0);
						else if ((sx == 1) || (sy == 1))
							pcolor = pcolor + (illuminate(rootptr, 1.0) * 2.0);
						else
							pcolor = pcolor + illuminate(rootptr, 1.0);

						deleteTree(rootptr, true);
					}
				}
				pcolor.scale(16.0);		// Scale back down...
			}

			// Next, clamp color component values to 8 bits.
			if (pcolor.r > 255.0)
				pcolor.r = 255.0;
			if (pcolor.g > 255.0)
				pcolor.g = 255.0;
			if (pcolor.b > 255.0)
				pcolor.b = 255.0;

			if (bytes_per_pixel == 3)
			{
				row[pixel * 3] = (unsigned char) pcolor.b;
				row[pixel * 3 + 1] = (unsigned char) pcolor.g;
				row[pixel * 3 + 2] = (unsigned char) pcolor.r;
			}
			else
			{
				row[pixel * 4] = (unsigned char) 0x0;
				row[pixel * 4 + 1] = (unsigned char) pcolor.b;
				row[pixel * 4 + 2] = (unsigned char) pcolor.g;
				row[pixel * 4 + 3] = (unsigned char) pcolor.r;
			}

			pixel++;

#ifdef SUNOS
			if (display == 3)
			{
				tempcolor[0] = pcolor.r / 255.0;
				tempcolor[1] = pcolor.g / 255.0;
				tempcolor[2] = pcolor.b / 255.0;
				(*dev->paintr)(tempcolor, x, (vres - y - 1), x+1, (vres - y));
			}
#endif
		}
#ifdef SUNOS
		if (display == 3)	// If display option is selected
			(*dev->flush)();
#endif
		if (storage > 0)		// If storage is enabled
		{
			fwrite(rowptr, bytes_per_pixel, hres, outfile);	// Write out a row.
			if (storage == 5)		// Windows BMP files get special treatment...
            {
				if (((bytes_per_pixel * hres) % 4) != 0)	// Check for 32-bit alignment
					fwrite(&zero, 1, ((bytes_per_pixel * hres) % 4), outfile);
            }
			else {
				if (((bytes_per_pixel * hres) % 2) == 1)	// If the row width is odd
					fwrite(&zero, 1, 1, outfile);	// Write a zero to pad the row width.
			}
		}
	}
	if (storage > 0)
		fclose(outfile);
}


void trace(Ray aray, Node *nodeptr, FP weight, int level)
{
	FP closest = 9999999999.0;  // Distance to the closest object
	Boolean iflag = false, temp;
	Node *tnodeptr, *rnodeptr;
	Object *closeptr;   // pointer to the object closest to the camera
	int n;
	Interdata id, idn;

	if (use_octree == true)
		closeptr = checktree(aray, idn);
	else
	{
		n = 0;
		do    // Loop through all intersected objects in the scene
		{
			do  // Loop through all objects until an intersection is found
			{
				temp = objptr[n]->icheck(aray, id);
				n++;
			}  while ((temp == false) && (n < numberOfObjects));

			// If this intersection is closer than any other, then record it.

			if ((temp == true) && (id.t < closest))
			{
				closeptr = objptr [n-1];
				closest = id.t;
				idn = id;
				iflag = true;
			}
		}  while (n < numberOfObjects);
	}

	if (((use_octree == true) && (closeptr == NULL)) ||
	((use_octree == false) && (iflag == false)))
	{	// No intersections - color it background.
		nodeptr->tflag = false;
		nodeptr->rflag = false;
		nodeptr->surface.init(0, 1.0, 0.0, 0.0, 1.0, backgroundColor);
		return;
	}
	else	// Get the intersection data
		closeptr->intersect(aray, nodeptr, idn);

	Color c = nodeptr->surface.color;	// c = the surface color computed by intersect
	Color d = illumination(nodeptr->poi, nodeptr->normal);	// d is the light from the various sources
	nodeptr->surface.color.init((ambient + d) * c);		// Compute the final point color

	if (level >= maxLevel)
	{
		nodeptr->tflag = false;
		nodeptr->rflag = false;
		nodeptr->entering = false;
		return;
	}

	// Next, compute the weight of the transmitted ray.  If it is still
	// significant, allocate a node and trace the transmitted ray.

	if (weight * nodeptr->surface.ktran > 0.05)
	{
		if (!(tnodeptr = new Node))
		{
			printf("\nInsufficient memory to allocate a transmitted ray node.\n");
			exit(1);
		}
		nodeptr->tptr = tnodeptr;	// Store the pointer to the trasmitted's node
		nodeptr->tflag = true;		// Indicate that tptr is valid.
		tnodeptr->entering = nodeptr->entering;
		trace(nodeptr->transmitted, tnodeptr, weight * nodeptr->surface.ktran, level+1);
	}
	else
		nodeptr->tflag = false;

	// Next, compute the weight of the reflected ray.  If it is still
	// significant, allocate a node and trace the reflected ray.

	if (weight * nodeptr->surface.kspec > 0.05)
	{
		if (!(rnodeptr = new Node))
		{
			printf("\nInsufficient memory to allocate a reflected ray node.\n");
			exit(1);
		}
		nodeptr->rptr = rnodeptr;	// Store the pointer to the trasmitted's node
		nodeptr->rflag = true;		// Indicate that rptr is valid.
		trace(nodeptr->reflected, rnodeptr, weight * nodeptr->surface.kspec, level+1);
	}
	else
		nodeptr->rflag = false;
}


Color illumination(Point& poi, Vector& normal)
{
	Color c;
	int l, o;
	FP lt;
	Ray aray;
	Boolean blocked, hit;
	Interdata id;
	Object *closeptr;

	for (l = 0; l < numberOfLights; l++)	// For every light, add its contribution
	{
		lt = aray.init(poi, lightptr[l]->location - poi);  // A ray pointing to the light
		blocked = false;
		if (use_octree == false)
		{
			o = 0;
			do
			{
				do		// for every object, check for light obscuration
				{
					hit = objptr[o]->icheck(aray, id);
					o++;
				}  while ((hit == false) && (o < numberOfObjects));

				if ((hit == true) && (id.t < lt))
					blocked = true;	// An object is between the light and the poi.
			}  while ((blocked == false) && (o < numberOfObjects));
		}
		else
		{
			closeptr = checktree(aray, id);
			// If an object is hit, and it's closer than the light
			if ((closeptr != NULL) && (id.t < lt))
				blocked = true;
		}

		if (blocked == false)	// If there are no objects blocking the light
		{
			// Add this light's color & intensity:
			c = c + lightptr[l]->getillumination(normal, aray.direction);
		}
	}
	return c;
}


Color illuminate(Node *nodeptr, FP weight)
{
	// This procedure traverses the intersection tree and computes the color
	// of the pixel.

	Color color1, color2;

	if (nodeptr->tflag == true)
	{
		color1 = illuminate(nodeptr->tptr, weight * nodeptr->surface.ktran);
		color2 = color1 * weight;
	}

	if (nodeptr->rflag == true)
	{
		color1 = illuminate(nodeptr->rptr, weight * nodeptr->surface.kspec);
		color2 = color2 + color1 * weight;
	}

	return (color2 + (nodeptr->surface.color * nodeptr->surface.kdiff));
}


void deleteTree(Node *nodeptr, Boolean root)
{
	if (nodeptr->tflag == true)
	{
		deleteTree(nodeptr->tptr, false);
	}

	if (nodeptr->rflag == true)
	{
		deleteTree(nodeptr->rptr, false);
	}

	if (root == false)  delete nodeptr;
}

#ifdef SUNOS
void catcher(int exceptionType, int exceptionError)
{
	if (exceptionType == SIGFPE)
	{
		signal(SIGFPE, (void (*)(int))catcher);	// Re-install signal handler
		switch (exceptionError)
		{
			case FPE_ZERODIVIDE:
				printf("Caught a floating-point divide-by-zero...\n");
				break;
			case FPE_INTOVFLOW:
				printf("Caught an integer overflow...\n");
				break;
			case FPE_INTDIV0:
				printf("Caught an integer divide-by-zero...\n");
				break;
			case FPE_OVERFLOW:
				printf("Caught a numeric overflow...\n");
				break;
			case FPE_UNDERFLOW:
				printf("Caught a numeric underflow...\n");
				break;
			case FPE_INEXACT:
				printf("Caught a precision exception (inexact result)...\n");
				break;
			case FPE_EXPLICITGEN:
				printf("Caught a user-raised exception...\n");
				break;
			case FPE_STACKFAULT:
				printf("Caught a stack overflow or underflow, exiting...\n");
				exit(1);
				break;
			case FPE_INVALID:
				printf("Caught an invalid operation, exiting...\n");
				exit(1);
				break;
			default:
				printf("Unknown exception %d.\n", exceptionError);
				break;
		}
	}
	else
	if (exceptionType == 0x0e)
	{
		printf("Exception 14, error code %d.  Exiting...\n", exceptionError);
		exit(1);
	}
}
#endif // SUNOS

