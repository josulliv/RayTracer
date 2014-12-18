// quadric.h		Quadric-related object declarations.

#ifndef _quadric_h
#define _quadric_h


class Sphere : public Object
{
	public:

	FP ra, ras;
	Point center;

	Sphere(void);
	void init(Surface isurface, Point icenter, FP ira);
	Boolean icheck(Ray& aray, Interdata& id);
	void intersect(Ray& aray, Node *nodeptr, Interdata& id);
	Point getMin(void)
	{
		return Point(center.x - ra, center.y - ra, center.z - ra);
	}
	Point getMax(void)
	{
		return Point(center.x + ra, center.y + ra, center.z + ra);
	}
	Boolean voxelicheck(Point& vmin, Point& vmax);
	friend void loadScene(void);
	friend istream& operator >> (istream& s, Sphere& p);
	friend ostream& operator << (ostream& s, Sphere& p);
};

istream& operator >> (istream& s, Sphere& p);
ostream& operator << (ostream& s, Sphere& p);


class Cylinder : public Object
{
	public:

	FP ra, ras, h;
	Point base, end;	// The two center endpoints of the cylinder

	Cylinder(void);
	Boolean icheck(Ray& aray, Interdata& id);
	void intersect(Ray& aray, Node *nodeptr, Interdata& id);
	Point getMin(void)
	{
		return base;	// Note: getMin and getMax don't work!!!
	}
	Point getMax(void)
	{
		return end;
	}
	Boolean voxelicheck(Point& vmin, Point& vmax);
	friend void loadScene(void);
	friend istream& operator >> (istream& s, Cylinder& c);
	friend ostream& operator << (ostream& s, Cylinder& c);
};

istream& operator >> (istream& s, Cylinder& c);
ostream& operator << (ostream& s, Cylinder& c);


class Quadric : public Object
{
	public:

	FP a, b, c, d, e, f, g, h, i, j;

	Quadric(void);
	void init(Surface isurface, FP ia, FP ib, FP ic, FP id, FP ie, FP iif, FP ig, FP ih, FP ii, FP ij);
	Boolean icheck(Ray& aray, Interdata& id);
	void intersect(Ray& aray, Node *nodeptr, Interdata& id);
	Point getMin(void)
	{
		return Point(0, 0, 0);		// Note: getMin and getMax don't work!!!
	}
	Point getMax(void)
	{
		return Point(0, 0, 0);
	}
	Boolean voxelicheck(Point& vmin, Point& vmax);
	friend void loadScene(void);
	friend istream& operator >> (istream& s, Quadric& c);
	friend ostream& operator << (ostream& s, Quadric& c);
};

istream& operator >> (istream& s, Quadric& c);
ostream& operator << (ostream& s, Quadric& c);


#endif	// Of _quadric_h

