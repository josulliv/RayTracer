// Sphereflake: generates a scene description file for a fractal sphere object.
// J. O'Sullivan, 1/12/93

#include "../src/platform.h"
#include "../src/raytrace.h"
#include "../src/vector.h"
#include "../src/miscobj.h"
#include "../src/textures.h"
#include "../src/object.h"
#include "../src/quadric.h"
#include <string.h>

void sphereflake(FP radius, Point center, int level, Vector up, Vector mark, Boolean bottom);
void writeheader(void);


FP scale;
int x, numberOfObjects, depth;
Vector vector, vector1;
Point point, point1;
Color color;
Surface surface;
Sphere sphere;
Texture *textptr[1];
ofstream f1;
char filename[130], bufs[130];

// Note: Angle must be in radians!!!

//#define ANGLE 0.7854	// 45.0 degrees
#define ANGLE 0.9599	// 55.0 degrees

int main(int argc, char *argv[])
{
	if (argc > 1)
	{
		strcpy(bufs, argv[1]);
		sscanf(bufs, "%d", &depth);
		strcpy(filename, "sf");
		strcat(filename, bufs);
		strcat(filename, ".sdf");
	}
	else
	{
		strcpy(filename, "sf2.sdf");
		depth = 2;			// The number of levels of recursion
	}
	f1.open(filename);
	if (!f1)
	{
		printf("Cannot open sf.sdf for output.\n");
		exit(1);
	}
	writeheader();
	scale = 0.33;
	numberOfObjects = 2;	// The plane and the first sphere
	vector.init(0,1,0);		// The up vector
	vector1.init(0,0,-1.0);	// The mark vector
	point.init(0,-256.0,0);	// The sphere center
	color.init(0,0,1);
	surface.init(0, 1.0, 0, 0, 1, color);
	sphere.init(surface, point, 64.0);
	f1 << "1\n" << sphere;	// Write out the original sphere
	if (depth > 0)
		sphereflake(64.0, point, 1, vector, vector1, true);
	f1 << "-1\n-1\n-1\n";
	f1.close();
	printf("Number of objects: %d\n", numberOfObjects);

/*	Levels:    Number of Objects:
	4			9844
	3			1094
	2			122
	1			14
	0			2
*/
}


void sphereflake(FP radius, Point center, int level, Vector up, Vector mark, Boolean bottom)
{
	FP theta;
	int q;
	Vector vector, vector1, newmark;

	for (q = 0; q < 6; q++)	// Compute the equatorial subspheres
	{
		theta = (FP)q * 60.0 * DTOR;	// Convert degrees to radians
		vector = rotate(up, mark, theta);
		point = VtoP(center + (vector * (radius + radius * scale)));
		sphere.init(surface, point, radius * scale);
		f1 << "1\n" << sphere;	// Write out a subsphere
		numberOfObjects++;
		if (level < depth)
			sphereflake(radius * scale, point, level+1, vector, up, false);
	}
	vecnormcross(up, mark, vector);
	vector1 = rotate(vector, mark, ANGLE);
	for (q = 0; q < 3; q++)		// Compute the hemispherial subspheres
	{
		theta = ((FP)q * 120.0 + 60.0) * DTOR;
		vector = rotate(up, vector1, theta);
		point = VtoP(center + (vector * (radius + radius * scale)));
		sphere.init(surface, point, radius * scale);
		f1 << "1\n" << sphere;	// Write out a subsphere
		numberOfObjects++;
		point1 = VtoP(center + (up * ((radius + scale * radius) / cos(ANGLE))));
		newmark = point - point1;
		newmark.unitize();
		if (level < depth)
			sphereflake(radius * scale, point, level+1, vector, newmark, false);
	}

	if (bottom == true)
	{
		for (q = 0; q < 3; q++)		// Compute the hemispherial subspheres
		{
			theta = ((FP)q * 120.0 + 60.0) * DTOR;
			vector = rotate(up, vector1.neg(), theta);
			point = VtoP(center + (vector * (radius + radius * scale)));
			sphere.init(surface, point, radius * scale);
			f1 << "1\n" << sphere;	// Write out a subsphere
			numberOfObjects++;
			point1 = VtoP(center + (up.neg() * ((radius + scale * radius) / cos(ANGLE))));
			newmark = point - point1;
			newmark.unitize();
			if (level < depth)
				sphereflake(radius * scale, point, level+1, vector, newmark, false);
		}
	}
}
/*
Vector rotate(Vector up, Vector mark, FP theta)
{
	FP a, b, c, d, e, f, g, h, i, t, ct, st, xx, yy, zz;
	Vector temp;

	t = 1 - cos(theta);
	ct = cos(theta);
	st = sin(theta);

	a = t * up.dx * up.dx + ct;
	b = t * up.dx * up.dy + st * up.dz;
	c = t * up.dx * up.dz - st * up.dy;
	d = t * up.dx * up.dy - st * up.dz;
	e = t * up.dy * up.dy + ct;
	f = t * up.dy * up.dz + st * up.dx;
	g = t * up.dx * up.dz + st * up.dy;
	h = t * up.dy * up.dz - st * up.dx;
	i = t * up.dz * up.dz + ct;

	xx = mark.dx * a + mark.dy * b + mark.dz * c;
	yy = mark.dx * d + mark.dy * e + mark.dz * f;
	zz = mark.dx * g + mark.dy * h + mark.dz * i;
	temp.init(xx, yy, zz);

	return temp;
}
*/

void writeheader(void)
{
	f1 << "0\n5\n1\n";							// Display mode
	f1 << "1280\n1280\n0\n0\n0\n1280\n";		// Resolution
	f1 << "40\n1.0\n";							// FOV and aspect ratio
	f1 << "0\n-128.0\n-512.0\n0\n0\n1.0\n";		// Camera location & direction
	f1 << "140\n140\n140\n";					// Ambient light
	f1 << "8\n0\n0\n0\n";						// Maxlevel and background color
	f1 << "0\n-300\n1280\n-300\n46\n46\n46\n";	// The light
	f1 << "3\n0\n1\n0\n";						// The plane code & the normal
	f1 << "0\n1\n0\n0\n1\n0\n1\n0\n";			// The surface
	f1 << "511\n-1024\n-511.1\n0\n";			// D and min
	f1 << "1024\n-510.9\n2560\n";				// max
}

