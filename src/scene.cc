// Scene.cc		Contains the loadScene and saveScene functions.

#include "platform.h"
#include "raytrace.h"
#include "vector.h"		// Vector-related objects and functions
#include "miscobj.h"		// Miscellaneous objects
#include "lights.h"		// Light objects
#include "textures.h"	// Texture objects
#include "object.h"		// Object abstract-class definition
#include "planar.h"		// Planar objects
#include "quadric.h"		// Quadric objects
#include "octree.h"		// Octree stuff (voxels, etc.)
#include "scene.h"		// Function headers & variables
#include <string.h>		// (ANSI)  for strchr in main()

extern int maxLevel, hres, vres, numberOfObjects, numberOfLights, numberOfTextures;
extern int display, storage, order, fov, threshold;
extern int bytes_per_pixel, supersample, startingline, numlines;
extern Object *objptr[MAXOBJ];
extern Light *lightptr[16];
extern Texture *textptr[256];
extern Ray camera;
extern Color color, acolor, backgroundColor, ambient;
extern Vector scrnx, scrny, firstray, up;
extern FP aspect, hdeflect, scalex, scaley;
extern int objtype[MAXOBJ];		// Object type codes
extern int textype[64];			// Texture type codes
extern int lightype[16];		// Light type codes


void loadScene(char *filename)
{
	int temp, textureType;
	Point location;
	Vector direction, scrni, scrnj;
	ifstream f1;

	f1.open(filename);
	if (!f1)
	{
		printf("Cannot open %s for input.\n", filename);
		exit(1);
	}

	f1.setf(ios::skipws);	// Set I/O stream flags (skip whitespace on input)

	f1 >> display;		// How to display the data
	f1 >> storage;		// Storage mode
	f1 >> order;		// Order of row computation (0 = top down)
	f1 >> hres;			// Number of pixels per line
	f1 >> vres;			// Number of lines per frame
	f1 >> threshold;	// The maximum number of objects per voxel.
	f1 >> startingline;	// Raster line to start rendering
	f1 >> supersample;	// Degree of supersampling
	f1 >> numlines;		// Number of raster lines to render
	f1 >> fov;			// Horizontal view angle of the camera
	f1 >> aspect;		// Aspect ratio of the image
	f1 >> location;		// The camera location
	f1 >> direction;	// The camera direction
	f1 >> ambient;		// The ambient light intensity
	f1 >> maxLevel;		// The maximum depth of the intersection tree
	f1 >> backgroundColor;	// The color of the background

	// The 'up' direction vector:
	up.init(0.0, 1.0, 0.0);

	// Initialize the camera:
	camera.init(location, direction);
	camera.unitize();

	// The camera's field of view, in radians:
	hdeflect = ((FP) fov) * DTOR;   // Convert to radians

	// Compute the vector (scrni) at a right angle to the camera direction, pointed to the right.
	if (vecnormcross(camera.direction, up, scrni) == 0.0)
	{
		printf("The view and up directions are identical!\n\n");
		exit(1);
	}

	// Compute the vector (scrnj) pointing up relative to the camera:
	vecnormcross(scrni, camera.direction, scrnj);

	scrnx = scrni * 2 * tan(hdeflect * 0.5) / hres;
	scrny = scrnj * 2 * tan(hdeflect * aspect * 0.5) / vres;

    // Firstray corresponds to the upper left pixel in the image.
	firstray = camera.direction + scrni * tan(hdeflect * 0.5)
	- scrnj * tan(hdeflect * aspect * 0.5);
	firstray.dx = firstray.dx + SIGMA;
	firstray.dy = firstray.dy + SIGMA;

	numberOfLights = 0;
	numberOfObjects = 0;

//	numberOfTextures = 1;	// 0 = no texture, for ray tracer
//	numberOfTextures = 0;	// for scenebuilder

	if ((display < 0) || (display > 4))
	{
		display = 0;		// Clamp display to the default "off" value.
	}

	if ((storage < 0) || (storage > 5))
	{
		storage = 0;		// Clamp storage to the default "off" value.
	}

	if ((storage == 2) || (storage == 1) || (storage == 5))
		bytes_per_pixel = 0x3;
	else
	if ((storage >= 3) || (storage <= 4))
		bytes_per_pixel = 0x4;

/*	Defined display codes:
	0: Compute the data only - do not display.
	1: Compute and display in VGA 16-color mode.
	2: Compute and display in SVGA 640 x 480 x 256-color mode.
	3: Compute and display via X11 window.
	4: No output at all.

	Defined storage codes:
	0: Do not store data.
	1: Store the full 24 bits of pixel data in raster format.
	2: Store 24-bit image in Sun rasterfile format.
	3: Store in 32-bit ABGR Sun rasterfile format.
	4: Write to stdout in ABGR packed-pixel format.
	5: Store 24-bit image in Windows BMP format.

	Defined supersampling codes:
	0: No supersampling
	1: 2 x 2 supersampling with jitter
	2: 3 x 3 supersampling with jitter

	Object types:
	0: Point light source
	1: Sphere
	2: Box
	3: Orthoplane
	4: Cylinder
	5: Quadric
	6: Directional light source
	7: Polygon
	8: Plane
	9: Ring
	255: Texture

	Texture types:

	0: No texture (flat color)
	1: Imagefile (Sun raster)
	2: Mandelbrot
	3: Tile
*/

	do
	{
		f1 >> temp;			// Read in the object type
		switch (temp)
		{
			case -1:
			{
				break;		// -1 means end of scene description.
			}
			case 0:			// Point light source
			{
				if (!(lightptr[numberOfLights] = new Plight()))
				{
					printf("\nInsufficient memory to allocate space for the %dth light.\n", numberOfLights);
					exit(1);
				}
				f1 >> *((Plight *)lightptr[numberOfLights]);
				lightype[numberOfLights] = 0;
				numberOfLights++;
				break;
			}
			case 1:			// sphere
			{
				if (!(objptr[numberOfObjects] = new Sphere()))
				{
					printf("\nInsufficient memory to allocate space for the %dth object (a sphere).\n", numberOfObjects);
					exit(1);
				}
				f1 >> *((Sphere *)objptr[numberOfObjects]);
				objtype[numberOfObjects] = 1;
				numberOfObjects++;
				break;
			}
			case 2:			// Box
			{
				if (!(objptr[numberOfObjects] = new Box()))
				{
					printf("\nInsufficient memory to allocate space for the %dth object (a box).\n", numberOfObjects);
					exit(1);
				}
				f1 >> *((Box *)objptr[numberOfObjects]);
				objtype[numberOfObjects] = 2;
				numberOfObjects++;
				break;
			}
			case 3:			// Orthoplane
			{
				if (!(objptr[numberOfObjects] = new Orthoplane()))
				{
					printf("\nInsufficient memory to allocate space for the %dth object (an orthoplane).\n", numberOfObjects);
					exit(1);
				}
				f1 >> *((Orthoplane *)objptr[numberOfObjects]);
				objtype[numberOfObjects] = 3;
				numberOfObjects++;
				break;
			}
			case 4:			// Cylinder
			{
				if (!(objptr[numberOfObjects] = new Cylinder()))
				{
					printf("\nInsufficient memory to allocate space for the %dth object (a cylinder).\n", numberOfObjects);
					exit(1);
				}
				f1 >> *((Cylinder *)objptr[numberOfObjects]);
				objtype[numberOfObjects] = 4;
				numberOfObjects++;
				break;
			}
			case 5:			// Quadric
			{
				if (!(objptr[numberOfObjects] = new Quadric()))
				{
					printf("\nInsufficient memory to allocate space for the %dth object (a quadric).\n", numberOfObjects);
					exit(1);
				}
				f1 >> *((Quadric *)objptr[numberOfObjects]);
				objtype[numberOfObjects] = 5;
				numberOfObjects++;
				break;
			}
			case 6:			// Directional light
			{
				if (!(lightptr[numberOfLights] = new Dlight()))
				{
					printf("\nInsufficient memory to allocate space for the %dth light.\n", numberOfLights);
					exit(1);
				}
				f1 >> *((Dlight *)lightptr[numberOfLights]);
				lightype[numberOfLights] = 6;
				numberOfLights++;
				break;
			}
			case 7:			// Polygon
			{
				if (!(objptr[numberOfObjects] = new Polygon()))
				{
					printf("\nInsufficient memory to allocate space for the %dth object (a polygon).\n", numberOfObjects);
					exit(1);
				}
				f1 >> *((Polygon *)objptr[numberOfObjects]);
				objtype[numberOfObjects] = 7;
				numberOfObjects++;
				break;
			}
			case 8:			// plane
			{
				if (!(objptr[numberOfObjects] = new Plane()))
				{
					printf("\nInsufficient memory to allocate space for the %dth object (a plane).\n", numberOfObjects);
					exit(1);
				}
				f1 >> *((Plane *)objptr[numberOfObjects]);
				objtype[numberOfObjects] = 8;
				numberOfObjects++;
				break;
			}
			case 9:			// Ring
			{
				if (!(objptr[numberOfObjects] = new Ring()))
				{
					printf("\nInsufficient memory to allocate space for the %dth object (a ring).\n", numberOfObjects);
					exit(1);
				}
				f1 >> *((Ring *)objptr[numberOfObjects]);
				objtype[numberOfObjects] = 9;
				numberOfObjects++;
				break;
			}
			case 255:		// A Texture
			{
				f1 >> textureType;
				switch(textureType)
				{
					case 1:	// Imagefile
					{
						if (!(textptr[numberOfTextures] = new Imagefile()))
						{
							printf("\nInsufficient memory to allocate space for the %dth object (an Imagefile texture).\n", numberOfTextures);
							exit(1);
						}
						f1 >> *((Imagefile *)textptr[numberOfTextures]);
						textype[numberOfTextures] = 1;
						numberOfTextures++;
						break;
					}
					case 2:	// Mandelbrot
					{
						if (!(textptr[numberOfTextures] = new Mandelbrot()))
						{
							printf("\nInsufficient memory to allocate space for the %dth object (a Mandelbrot texture).\n", numberOfTextures);
							exit(1);
						}
						f1 >> *((Mandelbrot *)textptr[numberOfTextures]);
						textype[numberOfTextures] = 2;
						numberOfTextures++;
						break;
					}
					case 3:	// Tile
					{
						if (!(textptr[numberOfTextures] = new Tile()))
						{
							printf("\nInsufficient memory to allocate space for the %dth object (a Tile texture).\n", numberOfTextures);
							exit(1);
						}
						f1 >> *((Tile *)textptr[numberOfTextures]);
						textype[numberOfTextures] = 3;
						numberOfTextures++;
						break;
					}
					default:
					{
						printf("There is an unrecognized texture code in the scene description file.  The code\n");
						printf("is: %d.  The object count is: %d.  The texture count is: %d.\n\n", textureType, numberOfObjects, numberOfTextures);
						printf("Aborting the program.\n");
						exit(1);
					}
				}
				break;
			}
			default:
			{
				printf("There is an unrecognized object code in the scene description file.  The code\n");
				printf("is: %d.  The object count is: %d.  The light count is: %d.\n\n", temp, numberOfObjects, numberOfLights);
				printf("Aborting the program.\n");
				exit(1);
			}
		}
		if (numberOfObjects > MAXOBJ)
		{
			printf("Error: the number of objects in the .sdf exceeded the allowed %d objects.\n", MAXOBJ);
			printf("Ignoring the remaining objects...\n");
			temp = -1;
		}
	}  while (temp != -1);
	f1.close();

	// Next, check each object for legal texture references...

	for (temp = 0; temp < numberOfObjects; temp++)
	{
		if (objptr[temp]->surface.texture >= numberOfTextures)
		{		// Then, the texture index is invalid - reset to no texture.
			printf("\nError in object %d:  The texture index (%d) is invalid (>= %d).\n", temp, objptr[temp]->surface.texture, numberOfTextures);
			printf("This object will be set to no texture.\n");
			objptr[temp]->surface.texture = 0;
		}
	}
}


void saveScene(char *filename)	// Write a scene to the file in filename.
{
	int temp = 0;
	ofstream f2;

	f2.open(filename);
	if (!f2)
	{
		printf("Cannot open %s for input.\n", filename);
		exit(1);
	}

	f2 << display << "\n";	// How to display the data
	f2 << storage << "\n";	// Storage mode
	f2 << order << "\n";	// Order of row computation (0 = top down)
	f2 << hres << "\n";		// Number of pixels per line
	f2 << vres << "\n";		// Number of lines per frame
	f2 << temp << "\n";		// Left-most x-coordinate - OBSOLETE
	f2 << temp << "\n";		// Right-most x-coordinate - OBSOLETE
	f2 << supersample << "\n";	// Degree of supersampling
	f2 << vres << "\n";		// Number of rows to compute
	f2 << fov << "\n";		// Horizontal view angle of the camera
	f2 << aspect << "\n";	// Aspect ratio of the image
	f2 << camera.origin;	// The camera location point
	f2 << camera.direction;	// The camera direction vector
	f2 << ambient;			// The ambient light color/intensity
	f2 << maxLevel << "\n";	// The maximum depth of the intersection tree
	f2 << backgroundColor;	// The color of the background

	// Write out the lights...
	for (temp = 0; temp < numberOfLights; temp++)
	{
		f2 << lightype[temp] << "\n";	// Write out the light type code...
		switch (lightype[temp])
		{
			case 0:		// Point light source
			{
				f2 << *((Plight *)lightptr[temp]);	// Write out the light...
				break;
			}
			case 6:		// Directional light source
			{
				f2 << *((Dlight *)lightptr[temp]);	// Write out the light...
				break;
			}
		}
	}

	// Write out the graphical objects...
	for (temp = 0; temp < numberOfObjects; temp++)
	{
		f2 << objtype[temp] << "\n";	// Write out the object type
		switch (objtype[temp])
		{
			case 1:			// sphere
			{
				f2 << *((Sphere *)objptr[temp]);
				break;
			}
			case 2:			// box
			{
				f2 << *((Box *)objptr[temp]);
				break;
			}
			case 3:			// orthoplane
			{
				f2 << *((Orthoplane *)objptr[temp]);
				break;
			}
			case 4:			// Cylinder
			{
				f2 << *((Cylinder *)objptr[temp]);
				break;
			}
			case 5:			// Quadric
			{
				f2 << *((Quadric *)objptr[temp]);
				break;
			}
			case 7:			// Polygon
			{
				f2 << *((Polygon *)objptr[temp]);
				break;
			}
			case 8:			// Plane
			{
				f2 << *((Plane *)objptr[temp]);
				break;
			}
			case 9:			// Ring
			{
				f2 << *((Ring *)objptr[temp]);
				break;
			}
			default:
			{
				cout << "Invalid object type code!\n";
				break;
			}
		}
	}

	// Write out the textures...
	for (temp = 0; temp < numberOfTextures; temp++)
	{
		f2 << "255\n";					// Indicates that a texture follows...
		f2 << textype[temp] << "\n";	// Write out the texture type code
		switch (textype[temp])			// Call the appropriate inserter overload...
		{
			case 1:							// Imagefile texture
			{
				f2 << *((Imagefile *)textptr[temp]);
				break;
			}
			case 2:						// Mandelbrot texture
			{
				f2 << *((Mandelbrot *)textptr[temp]);
				break;
			}
			case 3:						// Square tiles
			{
				f2 << *((Tile *)textptr[temp]);
				break;
			}
			default:
			{
				cout << "Unrecognized texture type!\n";
				break;
			}
		}
	}

	f2 << "-1\n-1\n-1\n\n";		// Terminate the SDF.
	f2.close();
}


