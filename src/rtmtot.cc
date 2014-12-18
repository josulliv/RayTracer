//  Multi-threaded ray tracer with octree-encoding		J. O'Sullivan

#include "platform.h"
#include "raytrace.h"
#include "vector.h"		// Vector-related objects and functions
#include "miscobj.h"	// Miscellaneous objects
#include "lights.h"		// Light objects
#include "textures.h"	// Texture objects
#include "object.h"	// Graphical objects
#include "planar.h"	// Planar objects
#include "quadric.h"	// Quadric-based objects
#include "octree.h"		// Octree-related stuff (voxels, etc.)
#include "scene.h"		// LoadScene

#include <time.h>		// (ANSI)
#include <string.h>		// (ANSI)  for strchr in main()
#include <synch.h>		// Threads synchronization
#include <thread.h>		// Threads include file
#include <errno.h>
#include <sys/lwp.h>
#include <unistd.h>		// Used by sysconf
#include <sunmath.h>	// SIGFPE exception handler (ieee_handler)

#ifdef XPLOT
#include "../xplot/color.h"		// COLOR definition from Radiance
#include "../xplot/standard.h"	// From radiance
extern "C" {
#include "../xplot/driver.h"	// From radiance
}
#endif

// ****  Function Headers  ****

void scan(void);
void *scanrow(void *row);
void trace(Ray aray, Node *rootptr, FP weight, int level);
Color illumination(Point& poi, Vector& normal);
Color illuminate(Node *rootptr, FP weight);
void deleteTree(Node *nodeptr, Boolean root);
void writefile(char *outfilename);

#ifdef XPLOT
extern "C" {
void devopen(char *dname);
}
#endif


// ****  Global variable declarations  ****

#ifdef XPLOT
char *devname = dev_default;		// For X11 display
struct driver *dev = NULL;			// For X11 display
COLOR tempcolor = {0.0,1.0,0.0};	// Used with X11 routines
#endif

thread_t thread[1280];		// The array of thread IDs
mutex_t display_lock;
time_t tstart, tend, *tloc;
int maxLevel, hres, vres, numberOfObjects, numberOfLights, numberOfTextures;
int display, storage, order, fov;
int bytes_per_pixel, supersample, startingline, numlines;
char *image;
Object *objptr[12500];
Light *lightptr[16];
Texture *textptr[256];
Ray camera;
Color color, acolor, backgroundColor, ambient;
Vector scrnx, scrny, firstray, up;
FP aspect, hdeflect, scalex, scaley, minlen2;
int objtype[12500];			// Object type codes
int textype[64];			// Texture type codes
int lightype[16];			// Light type codes
Boolean used_by_scenebuilder = false;
int threshold, numberOfVoxels = 0;
Voxel rootvoxel;
Boolean use_octree;

void main(int argc, char *argv[])
{
	char *ptr, bufs[130], outfilename[130];
	int x, y;

	if (argc == 1)
	{
		cout << "Auto-restarting has not been implemented yet.  Call this with an SDF filename!\n\n";
		exit(1);
	}

	strcpy(bufs, argv[1]);
	ptr = strchr(bufs, 0);
	if (ptr)
	{
		*ptr = 0;	// Terminate the string at the first space.  In other words,
	}				// lop off all but the first parameter.

	if (argc == 2)	// If there's only the .sdf filename, use default dest.
	{
		strcpy(outfilename, bufs);
		strcat(outfilename, ".rif");	// Append the extension
	}

	if (argc == 3)	// If there are two parameters (the sdf & destination)
	{
		strcpy(outfilename, argv[2]);
	}

	strcat(bufs, ".sdf");	// Slap on an extension on the SDF file name

	// Next, set up a SIGFPE handler (ingore divide by zero errors):

	ieee_handler("clear", "common", NULL);

	numberOfTextures = 1;	// 0 = no texture
	printf("Now loading the scene from the scene description file.\n\n");
	loadScene(bufs);
	
	if (threshold == 0)
		threshold == 16;		// Set the default threshold...

	printf("Read in %d objects.\n", numberOfObjects);

	if (numberOfObjects > threshold)
	{
		use_octree = true;
		printf("Now building the octree...\n\n");

		tstart = time(tloc);
		buildOctree();
		printf("Finished building the octree, which contains %d voxels.\n\n", numberOfVoxels);
		tend = time(tloc);
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

	// Next, allocate memory for the image:
	
	image = new char[hres * vres * bytes_per_pixel];

#ifdef XPLOT
	if (display == 3)		// If the X11 display option is selected
	{
		devopen(devname);
		(*dev->clear)(hres, vres);
		(*dev->flush)();
	}
#endif

	if (display != 4)
		printf("Beginning the trace operation...\n\n");

	mutex_init(&display_lock, USYNC_THREAD, NULL);
	
	// Set the thread concurrency level to the number of processors:
	thr_setconcurrency((int)sysconf(_SC_NPROCESSORS_ONLN));

	tstart = time(tloc);
	scan();
	tend = time(tloc);
	mutex_destroy(&display_lock);

	if (display != 4)
		printf("\n\nElapsed time: %ld seconds.\n\n", (tend - tstart));
	if ((storage == 2) || (storage == 3))
		writefile(outfilename);
	if (display == 3)
	{
		printf("Press any key to exit...\n");
//		gets((char *)&bufs);
	}
}


void scan(void)
{
	int x, y;

	if (display != 4)
		printf("\nCreating threads...\n");

	for (x = 0; x < 4; x++)
	{
		for (y = 0; y < vres / 4; y++)
		{
			thr_create(NULL, NULL, scanrow, (void *)(y * 4 + x), NULL, &thread[y * 4 + x]);
		}

		if (display != 4)
		{
			printf("\nFinished creating thread group %d of 4...\n", x + 1);
			printf("\nWaiting for threads to complete...\n");
		}

		for (y = 0; y < vres/4; y++)
		{
			thr_join(thread[y * 4 + x], NULL, NULL);
		}
	}
}

void *scanrow(void *row)
{
	Node *rootptr;
	FP xp, yp, jx, jy;
	int x, yy, sx, sy, rowi;
	Color color;
	Ray aray;

	if (!(rootptr = new Node))
	{
		printf("\nInsufficient memory to allocate space for the root node.\n");
		thr_exit((void *)1);
	}

	rowi = (int)row;
	if (order == 0)
		yy = rowi - (vres / 2) + 1;
	else
		yy = (vres / 2) - rowi - 1;
	yp = yy;

	for (x = 0; x < hres; x++)
	{
		xp = x;
		
		if (supersample == 0)	// No supersampling
		{
			rootptr->entering = true;
			aray.init(camera.origin, firstray
			- (scrnx * xp) - (scrny * yp));
			trace(aray, rootptr, 1.0, 0);
			color = illuminate(rootptr, 1.0);
			deleteTree(rootptr, true);
		}
		else if (supersample == 1)	// 4x supersampling
		{
			color.init(0.0, 0.0, 0.0);
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
					color = color + illuminate (rootptr, 1.0);
					deleteTree(rootptr, true);
				}
			}
			color.scale(4.0);		// Average the four subpixels...
		}
		else if (supersample == 2)	// 9x (3x3) supersampling w/ Bartlett window
		{
			color.init(0.0, 0.0, 0.0);
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
/*
					// This is just supersampling, no jitter:

					aray.init(camera.origin, firstray
					- (scrnx * (xp + 0.33333333 * (FP)sx))
					- (scrny * (yp + 0.33333333 * (FP)sy)));
*/
					trace(aray, rootptr, 1.0, 0);

					// Implement the Bartlett window:

					if ((sx == 1) && (sy == 1))
						color = color + (illuminate(rootptr, 1.0) * 4.0);
					else if ((sx == 1) || (sy == 1))
						color = color + (illuminate(rootptr, 1.0) * 2.0);
					else
						color = color + illuminate(rootptr, 1.0);

					deleteTree(rootptr, true);
				}
			}
			color.scale(16.0);		// Scale back down...
		}

		if (color.r > 255.0)	// Clamp to socially acceptable values...
			color.r = 255.0;
		if (color.g > 255.0)
			color.g = 255.0;
		if (color.b > 255.0)
			color.b = 255.0;

		if (bytes_per_pixel == 4)	// For ABGR (32-bit) format
		{
			image[rowi * hres * bytes_per_pixel + x * 4] = (unsigned char)0x0;
			image[rowi * hres * bytes_per_pixel + x * 4 + 1] = (unsigned char)color.b;
			image[rowi * hres * bytes_per_pixel + x * 4 + 2] = (unsigned char)color.g;
			image[rowi * hres * bytes_per_pixel + x * 4 + 3] = (unsigned char)color.r;
		}
		else if (bytes_per_pixel == 3)	// For BGR format
		{
			image[rowi * hres * bytes_per_pixel + x * 3] = (unsigned char) color.b;
			image[rowi * hres * bytes_per_pixel + x * 3 + 1] = (unsigned char) color.g;
			image[rowi * hres * bytes_per_pixel + x * 3 + 2] = (unsigned char) color.r;
		}

#ifdef XPLOT
		if (display == 3)	// If X-win display is enabled
		{
			mutex_lock(&display_lock);
			tempcolor[0] = color.r / 255.0;
			tempcolor[1] = color.g / 255.0;
			tempcolor[2] = color.b / 255.0;
			(*dev->paintr)(tempcolor, x, (vres - rowi - 1), x+1, (vres - rowi));
			mutex_unlock(&display_lock);
		}
#endif
	}
#ifdef XPLOT
	if (display == 3)       // If display option is selected
	{
		mutex_lock(&display_lock);
		(*dev->flush)();
		mutex_unlock(&display_lock);
	}
#endif
	return ((void *)0);
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
				closeptr = objptr[n-1];
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
		o = 0;
		if (use_octree == false)
		{
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


void writefile(char *outfilename)
{
	char zero = 0;
	int y;
	FILE *outfile;
	rasterfile rfile;	// Declare an instance of the rasterfile header struct
	char *rfileptr;
	rfileptr = (char *) &rfile;

	if ((outfile = fopen(outfilename, "wb")) == NULL)
	{
		printf("\nThe file %s cannot be opened.  Exiting...\n\n", outfilename);
		exit(1);
	}
	rfile.ras_magic = RAS_MAGIC;
	rfile.ras_width = hres;
	rfile.ras_height = vres;
	rfile.ras_depth = 0x8 * bytes_per_pixel;
	rfile.ras_length = hres * vres * bytes_per_pixel;
	rfile.ras_type = RT_STANDARD;
	rfile.ras_maptype = RMT_NONE;
	rfile.ras_maplength = 0x0;
	fwrite(rfileptr, 0x4, 0x8, outfile);	// Write the rfile structure - 8 words of 4 bytes each.

	for (y = 0; y < vres; y++)
	{
		// Write out a row of pixels:
		fwrite((char *)&image[y * hres * bytes_per_pixel], bytes_per_pixel, hres, outfile);

		// If the row width is odd, write a zero to pad the row length
		// to a multiple of 16 bits:

		if (((bytes_per_pixel * hres) % 2) == 1)
			fwrite(&zero, 1, 1, outfile);
	}
	fclose(outfile);
}


