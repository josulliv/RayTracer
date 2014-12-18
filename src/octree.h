// Octree.h		The voxel class, octree functions	J. O'Sullivan	9/27/1993

#ifndef octree_h
#define octree_h

#define OTSIGMA 0.000000001
#include "platform.h"

extern FP minlen2;
extern Object *objptr[MAXOBJ];
extern int numberOfObjects, numberOfVoxels, threshold;
extern Boolean use_octree;

class OctreeInterdata
{
	public:
	
	FP tn, tf;				// The distance to the poi.
	Point poin, poif;			// The point of intersection
	Vector normaln, normalf;	// The normal at the poi

	void init(FP itn, FP itf, Point ipn, Point ipf, Vector inn, Vector inf)
	{
		tn = itn;
		tf = itf;
		poin = ipn;
		poif = ipf;
		normaln = inn;
		normalf = inf;
	}
};

class Voxel
{
	public:

	Point min, max;			// Extents of this voxel
	FP size;				// Size of the voxel
	Boolean subdivided;		// True if this voxel has children
	int numberOfObjects;		// The number of objects in this voxel
	Voxel *childrenptr;		// Pointer to the children voxels
	Object **list;			// Pointer to the list of objects in this voxel

	Boolean inside(Point& point)	// True if point's inside voxel.
	{
		if ((point > min) && (point < max))
			return true;
		else
			return false;
	}
	Boolean icheck(Ray& aray, OctreeInterdata& id);
};

extern Voxel rootvoxel;

Object *checktree(Ray& ray, Interdata& idn);
Object *iterate(Ray& ray, Voxel *voxel, Interdata& id);
Voxel *findvoxel(Voxel *voxel, Point& point);
void buildOctree(void);
void voxelfill(Voxel *voxel);
void setextents(int x, FP size, Point& newmin, Point& newmax, Point& min, Point& max);

#endif	// Of octree_h
