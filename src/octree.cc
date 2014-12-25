// Octree.cc    Octree-related functions    J. O'Sullivan   9/27/1993

#include "raytrace.h"
#include "vector.h"
#include "miscobj.h"
#include "textures.h"
#include "object.h"
#include "planar.h"
#include "quadric.h"
#include "octree.h"

// Note: rootvoxel is ALWAYS empty - it never has any objects in it.
// It's always subdivided.

Object *checktree(Ray& ray, Interdata& idn)
{
	OctreeInterdata oid;
	Point point;

/*
	printf("\nEntering checktree.  Ray:\n");
	printf("%1.19e,%1.19e,%1.19e\n", ray.origin.x, ray.origin.y, ray.origin.z);
	printf("%1.19e,%1.19e,%1.19e\n", ray.direction.dx, ray.direction.dy, ray.direction.dz);
*/

	// First, are we inside or outside the world box?

	if (!rootvoxel.inside(ray.origin))
	{
		// We're outside the world box.  Intersect with the world box:

		if (rootvoxel.icheck(ray, oid) == true)
		{
			// Found an intersection with the world box.
			// Next, find a point inside the world:
			// First, determine which planes the poi is on.

			if ((fabs(oid.poin.z - rootvoxel.min.z) < OTSIGMA) ||
			(fabs(oid.poin.z - rootvoxel.max.z) < OTSIGMA))
            {
				if (ray.direction.dz < 0.0)
					oid.normaln.dz = -1.0;
				else
					oid.normaln.dz = 1.0;
            }
			if ((fabs(oid.poin.x - rootvoxel.min.x) < OTSIGMA) ||
			(fabs(oid.poin.x - rootvoxel.max.x) < OTSIGMA))
			{
                if (ray.direction.dx < 0.0)
					oid.normaln.dx = -1.0;
				else
					oid.normaln.dx = 1.0;
            }

			if ((fabs(oid.poin.y - rootvoxel.min.y) < OTSIGMA) ||
			(fabs(oid.poin.y - rootvoxel.max.y) < OTSIGMA))
			{
                if (ray.direction.dy < 0.0)
					oid.normaln.dy = -1.0;
				else
					oid.normaln.dy = 1.0;
			}

			// Compute a point inside the voxel.

			point = oid.poin + VtoP(oid.normaln * minlen2);

			return iterate(ray, findvoxel(&rootvoxel, point), idn);
		}
		else
			return NULL;	// No intersection with the world.
	}
	else
	{
		// We're inside the world box.  Find out which voxel we're in,
		// then call the recursive intersect function iterate.

		return iterate(ray, findvoxel(&rootvoxel, ray.origin), idn);
	}
}


Object *iterate(Ray& ray, Voxel *voxel, Interdata& idn)
{
	int n;
	Interdata id;
	OctreeInterdata oid;
	FP closest = 9999999999.0;  // Distance to the closest object
	Boolean iflag = false, intersection;
	Object *closeptr;   // pointer to the object closest to the camera
	Point point;

/*
	printf("\nEntering iterate.  Ray:\n");
	printf("%1.19e,%1.19e,%1.19e\n", ray.origin.x, ray.origin.y, ray.origin.z);
	printf("%1.19e,%1.19e,%1.19e\n", ray.direction.dx, ray.direction.dy, ray.direction.dz);
	printf("\nVoxel min and max:\n");
	printf("%1.19e,%1.19e,%1.19e\n", voxel->min.x, voxel->min.y, voxel->min.z);
	printf("%1.19e,%1.19e,%1.19e\n", voxel->max.x, voxel->max.y, voxel->max.z);
*/

	// First, check the ray for intersection with all objects in this voxel.
	// Store the intersection data in id, and return the pointer to the
	// object if there is an intersection.

	if (voxel->numberOfObjects > 0)	// Be sure this voxel isn't empty.
	{
		n = 0;
		do    // Loop through all intersected objects in the scene
		{
			do  // Loop through all objects until an intersection is found
			{
				intersection = (voxel->list[n])->icheck(ray, id);
				n++;
			}  while ((intersection == false) && (n < voxel->numberOfObjects));

			// If this intersection is closer than any other, and
			// if it is in the current voxel, then record it.

			if ((intersection == true) && (id.t < closest) &&
			(id.poi > voxel->min) && (id.poi < voxel->max))
			{
				closeptr = voxel->list[n-1];
				closest = id.t;
				idn = id;
				iflag = true;
			}
		}  while (n < voxel->numberOfObjects);
	}

	// If there is no intersection, compute the next voxel and iterate.
	// If the ray passes out of the world, return null.

	if (iflag == true)	// If there's an intersection, return.
		return closeptr;
	else
	{
		// No intersection, move to the next voxel.
		// To do this, compute the far poi:

		if (voxel->icheck(ray, oid) == false)
		{
			printf("Deep kimchee!\n");
			exit(1);
		}

		// determine which plane(s) it's on:

		if ((fabs(oid.poif.z - voxel->min.z) < OTSIGMA) ||
		(fabs(oid.poif.z - voxel->max.z) < OTSIGMA))
			if (ray.direction.dz < 0.0)
				oid.normalf.dz = -1.0;
			else
				oid.normalf.dz = 1.0;

		if ((fabs(oid.poif.x - voxel->min.x) < OTSIGMA) ||
		(fabs(oid.poif.x - voxel->max.x) < OTSIGMA))
			if (ray.direction.dx < 0.0)
				oid.normalf.dx = -1.0;
			else
				oid.normalf.dx = 1.0;

		if ((fabs(oid.poif.y - voxel->min.y) < OTSIGMA) ||
		(fabs(oid.poif.y - voxel->max.y) < OTSIGMA))
			if (ray.direction.dy < 0.0)
				oid.normalf.dy = -1.0;
			else
				oid.normalf.dy = 1.0;

		// Next, check if more than one ordinate is non-zero:

		closest = fabs(oid.normalf.dx) + fabs(oid.normalf.dy) + fabs(oid.normalf.dz);
		if (closest != 1.0)
		{
			printf("\nThe normal magnitude (%2.1f) is not 1.0 in iterate.\n", closest);
			exit(1);
		}

		// Compute a point inside the voxel.

		point = oid.poif + VtoP(oid.normalf * minlen2);

		if (rootvoxel.inside(point))
			return iterate(ray, findvoxel(&rootvoxel, point), idn);
		else
			return NULL;	// It left the world.
	}
}


Boolean Voxel::icheck(Ray& aray, OctreeInterdata& id)
{
	FP tn, tf, t1, t2, a;

	// This function determines if aray intersects with the voxel/box.
	// If so, it computes the point of intersection, and the distance.
	// From Ray Tracing.

	if ((fabs(aray.direction.dx) < 0.000001) && ((aray.origin.x < min.x - OTSIGMA) || (aray.origin.x > max.x + OTSIGMA)))
		return false;
	else
	{
		t1 = (min.x - aray.origin.x) / aray.direction.dx;
		t2 = (max.x - aray.origin.x) / aray.direction.dx;
		if (t1 > t2)
		{
			a = t1;
			t1 = t2;
			t2 = a;
		}
		tn = t1;
		tf = t2;
		if (tf < 0.0)
			return false;
	}

/*
	printf("\nEntering voxel::icheck:\n");
	printf("tn: %1.19e, tf: %1.19e\n", tn, tf);
*/

	if ((fabs(aray.direction.dy) < 0.000001) && ((aray.origin.y < min.y - OTSIGMA) || (aray.origin.y > max.y + OTSIGMA)))
		return false;
	else
	{
		t1 = (min.y - aray.origin.y) / aray.direction.dy;
		t2 = (max.y - aray.origin.y) / aray.direction.dy;
		if (t1 > t2)
		{
			a = t1;
			t1 = t2;
			t2 = a;
		}
		if (t1 > tn)
			tn = t1;
		if (t2 < tf)
			tf = t2;
		if ((tn > tf) || (tf < 0.0))
			return false;
	}

//	printf("tn: %1.19e, tf: %1.19e\n", tn, tf);

	if ((fabs(aray.direction.dz) < 0.000001) && ((aray.origin.z < min.z - OTSIGMA) || (aray.origin.z > max.z + OTSIGMA)))
		return false;
	else
	{
		t1 = (min.z - aray.origin.z) / aray.direction.dz;
		t2 = (max.z - aray.origin.z) / aray.direction.dz;
		if (t1 > t2)
		{
			a = t1;
			t1 = t2;
			t2 = a;
		}
		if (t1 > tn)
			tn = t1;
		if (t2 < tf)
			tf = t2;
		if ((tn > tf) || (tf < 0.0))
			return false;
	}

//	printf("tn: %1.19e, tf: %1.19e\n", tn, tf);

	id.tn = tn;
	id.tf = tf;
	id.poin = aray.getPoi(id.tn);
	id.poif = aray.getPoi(id.tf);

/*
	printf("Exiting voxel::icheck.  poif:\n");
	printf("%1.19e,%1.19e,%1.19e\n", id.poif.x, id.poif.y, id.poif.z);
*/

	return true;
}


Voxel *findvoxel(Voxel *voxel, Point& point)
{
	// Return a pointer to the non-subdivided voxel containing point.
	// This must be called with a subdivided voxel.

	// First, determine which octant the point is in, and retrieve the
	// pointer to the voxel in that octant.  Recurse if that voxel is
	// subdivided, else return that voxel's pointer.

	if ((point.x > voxel->min.x) && (point.x < (voxel->min.x + voxel->size / 2)))
	{
		// Then it's on the left side
		if ((point.y > voxel->min.y) && (point.y < (voxel->min.y + voxel->size / 2)))
		{
			// Then it's on the bottom half
			if ((point.z > voxel->min.z) && (point.z < (voxel->min.z + voxel->size / 2)))
			{
				// Then it's in the front half (quadrant 2)
				if (voxel->childrenptr[2].subdivided == true)
					return findvoxel(&voxel->childrenptr[2], point);
				else
					return &voxel->childrenptr[2];
			}
			else
			{
				// It's in the rear half.
				if (voxel->childrenptr[6].subdivided == true)
					return findvoxel(&voxel->childrenptr[6], point);
				else
					return &voxel->childrenptr[6];
			}
		}
		else
		{
			// It's on the upper half
			if ((point.z > voxel->min.z) && (point.z < (voxel->min.z + voxel->size / 2)))
			{
				// Then it's in the front half
				if (voxel->childrenptr[0].subdivided == true)
					return findvoxel(&voxel->childrenptr[0], point);
				else
					return &voxel->childrenptr[0];
			}
			else
			{
				// It's in the rear half.
				if (voxel->childrenptr[4].subdivided == true)
					return findvoxel(&voxel->childrenptr[4], point);
				else
					return &voxel->childrenptr[4];
			}
		}
	}
	else
	{
		// It's on the right side
		if ((point.y > voxel->min.y) && (point.y < (voxel->min.y + voxel->size / 2)))
		{
			// Then it's on the bottom half
			if ((point.z > voxel->min.z) && (point.z < (voxel->min.z + voxel->size / 2)))
			{
				// Then it's in the front half
				if (voxel->childrenptr[3].subdivided == true)
					return findvoxel(&voxel->childrenptr[3], point);
				else
					return &voxel->childrenptr[3];
			}
			else
			{
				// It's in the rear half.
				if (voxel->childrenptr[7].subdivided == true)
					return findvoxel(&voxel->childrenptr[7], point);
				else
					return &voxel->childrenptr[7];
			}
		}
		else
		{
			// It's on the upper half
			if ((point.z > voxel->min.z) && (point.z < (voxel->min.z + voxel->size / 2)))
			{
				// Then it's in the front half
				if (voxel->childrenptr[1].subdivided == true)
					return findvoxel(&voxel->childrenptr[1], point);
				else
					return &voxel->childrenptr[1];
			}
			else
			{
				// It's in the rear half.
				if (voxel->childrenptr[5].subdivided == true)
					return findvoxel(&voxel->childrenptr[5], point);
				else
					return &voxel->childrenptr[5];
			}
		}
	}
}


void buildOctree(void)
{
	Point p, min, max;	// The extents of the world.
	int x;
	FP size;

	// First, determine the world extents.

	printf("\nDetermining the extents of the world...\n\n");

	min = objptr[0]->getMin();
	max = objptr[0]->getMax();
	for (x = 1; x < numberOfObjects; x++)
	{
		p = objptr[x]->getMin();
		if (p.x < min.x)
			min.x = p.x;
		if (p.y < min.y)
			min.y = p.y;
		if (p.z < min.z)
			min.z = p.z;

		p = objptr[x]->getMax();
		if (p.x > max.x)
			max.x = p.x;
		if (p.y > max.y)
			max.y = p.y;
		if (p.z > max.z)
			max.z = p.z;
	}
	// Add a little margin to the world voxel to prevent precision problems:
	min.init(min.x - 5.0, min.y - 5.0, min.z - 5.0);
	max.init(max.x + 5.0, max.y + 5.0, max.z + 5.0);

	// Next, establish the world voxel.

	size = max.x - min.x;
	if (size < max.y - min.y)
		size = max.y - min.y;
	if (size < max.z - min.z)
		size = max.z - min.z;

	rootvoxel.min = min;
	rootvoxel.max.init(min.x + size, min.y + size, min.z + size);
	rootvoxel.size = size;
	minlen2 = size / 2;

	printf("Finished determining the world extents.  The rootvoxel size is %f.\n\n", rootvoxel.size);

	// Next, allocate space for children.

	rootvoxel.subdivided = true;
	rootvoxel.numberOfObjects = 0;
	rootvoxel.childrenptr = new Voxel[8];
	numberOfVoxels = 9;	// The parent plus 8 children

	// Next, recurse to fill in the children.
	for (x = 0; x < 8; x++)
	{
		printf("Working on octant %d...\n", x);

		// First, fill in the appropriate fields in the voxel:
		rootvoxel.childrenptr[x].size = size / 2;
		setextents(x, size / 2, rootvoxel.childrenptr[x].min, rootvoxel.childrenptr[x].max, rootvoxel.min, rootvoxel.max);

		// Finally, recurse to fill in the voxel with objects (or not):
		voxelfill(&rootvoxel.childrenptr[x]);
	}
	printf("\n\n");
}


void voxelfill(Voxel *voxel)
{
	// Intersect this voxel with all objects.  If the number of intersected
	// objects exceeds the threshold, subdivide this voxel and recurse.

	int n;						// The number of objects intersecting this voxel.
	Boolean intersection;		// True for intersections
	Object **list;				// A pointer to an array of pointers.

	list = new Object *[threshold+1];	// A temporary list of intersected objects

	if (voxel->size / 2 < minlen2)
		minlen2 = voxel->size / 2;	// Keep track of the smallest voxel size / 2.

	// The following is code used in debugging voxelfill:

	if (voxel->size == 13127.85)
		n = 0;

	n = 0;
	voxel->numberOfObjects = 0;

	do    // Loop through all objects in the scene
	{
		intersection = objptr[n]->voxelicheck(voxel->min, voxel->max);
		if (intersection == true)
		{
			list[voxel->numberOfObjects] = objptr[n];
			voxel->numberOfObjects++;
		}
		n++;
	}  while ((n < numberOfObjects) && (voxel->numberOfObjects <= threshold));

	if (voxel->numberOfObjects > threshold)	// Then subdivide
	{
		voxel->subdivided = true;
		voxel->numberOfObjects = 0;
		voxel->childrenptr = new Voxel[8];
		numberOfVoxels += 8;

		// Next, recurse to fill in the children.
		for (n = 0; n < 8; n++)
		{
			// First, fill in the appropriate fields in the voxel:
			voxel->childrenptr[n].size = voxel->size / 2;
			setextents(n, voxel->size / 2, voxel->childrenptr[n].min, voxel->childrenptr[n].max, voxel->min, voxel->max);

			// Then, recurse to fill the voxel with objects (or not):

			voxelfill(&voxel->childrenptr[n]);
		}
	}
	else	// Allocate a list of object pointers and copy over.
	{
		voxel->subdivided = false;
		if (voxel->numberOfObjects > 0)
		{
			voxel->list = new Object*[voxel->numberOfObjects];
			for (n = 0; n < voxel->numberOfObjects; n++)
				voxel->list[n] = list[n];
		}
	}
	delete list;
}


void setextents(int x, FP size, Point& newmin, Point& newmax, Point& min, Point& max)
{
	switch(x)
	{
		case 0:		// Left side, upper-front voxel.
		{
			newmin.init(min.x, min.y + size, min.z);
			newmax.init(min.x + size, max.y, min.z + size);
			break;
		}
		case 1:		// Right side, upper-front voxel.
		{
			newmin.init(min.x + size, min.y + size, min.z);
			newmax.init(max.x, max.y, min.z + size);
			break;
		}
		case 2:		// Left side, lower-front voxel.
		{
			newmin.init(min.x, min.y, min.z);
			newmax.init(min.x + size, min.y + size, min.z + size);
			break;
		}
		case 3:		// Right side, lower-front voxel.
		{
			newmin.init(min.x + size, min.y, min.z);
			newmax.init(max.x, min.y + size, min.z + size);
			break;
		}
		case 4:		// Left side, upper-rear voxel.
		{
			newmin.init(min.x, min.y + size, min.z + size);
			newmax.init(min.x + size, max.y, max.z);
			break;
		}
		case 5:		// Right side, upper-rear voxel.
		{
			newmin.init(min.x + size, min.y + size, min.z + size);
			newmax.init(max.x, max.y, max.z);
			break;
		}
		case 6:		// Left side, lower-rear voxel.
		{
			newmin.init(min.x, min.y, min.z + size);
			newmax.init(min.x + size, min.y + size, max.z);
			break;
		}
		case 7:		// Right side, lower-rear voxel.
		{
			newmin.init(min.x + size, min.y, min.z + size);
			newmax.init(max.x, min.y + size, max.z);
			break;
		}
	}	// end of switch...
}


