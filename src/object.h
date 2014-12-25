// object.h		Declaration of the ancestral, abstract class Object.

#ifndef object_h
#define object_h


class Object       // An abstract class.
{
	public:

	Vector normal;
	Surface surface;

	virtual Boolean icheck(Ray& aray, Interdata& id) = 0;
	virtual void intersect(Ray& aray, Node  *nodeptr, Interdata& id) = 0;
	virtual Point getMin(void) = 0;
	virtual Point getMax(void) = 0;
	virtual Boolean voxelicheck(Point& vmin, Point& vmax) = 0;
};

#endif	// Of object_h
