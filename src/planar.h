// planar.h		Planar object declarations.

#ifndef planar_h
#define planar_h


class Orthoplane : public Object
{
	public:

	FP d;			// The distance of the plane from the origin
	Point min, max;	// The minimum and maximum extents of the plane.

	Orthoplane(void);
	void init(Vector inormal, Surface isurface, FP id, Point imin, Point imax);
	Boolean icheck(Ray& aray, Interdata& id);
	void intersect(Ray& aray, Node *nodeptr, Interdata& id);
	Point getMin(void)
	{
		return min;
	}
	Point getMax(void)
	{
		return max;
	}
	Boolean voxelicheck(Point& vmin, Point& vmax);
	friend void loadScene(void);
	friend istream& operator >> (istream& s, Orthoplane& p);
	friend ostream& operator << (ostream& s, Orthoplane& p);
};

istream& operator >> (istream& s, Orthoplane& p);
ostream& operator << (ostream& s, Orthoplane& p);


class Plane : public Object
{
	public:

	FP d;			// The distance of the plane from the origin
	Point p[4];		// The four points to define the plane.
	Vector na, nb, nc;
	FP du0, du1, dv0, dv1;
	Boolean maxx, maxy, maxz;
	FP u[4], v[4];

	Plane(void);
	void init(Surface isurface, Point ip0, Point ip1, Point ip2);
	Boolean icheck(Ray& aray, Interdata& id);
	void intersect(Ray& aray, Node *nodeptr, Interdata& id);
	Point getMin(void)
	{
		Point a = p[0];

		if (p[1].x < a.x)
			a.x = p[1].x;
		if (p[1].y < a.y)
			a.y = p[1].y;
		if (p[1].z < a.z)
			a.z = p[1].z;

		if (p[2].x < a.x)
			a.x = p[2].x;
		if (p[2].y < a.y)
			a.y = p[2].y;
		if (p[2].z < a.z)
			a.z = p[2].z;

		if (p[3].x < a.x)
			a.x = p[3].x;
		if (p[3].y < a.y)
			a.y = p[3].y;
		if (p[3].z < a.z)
			a.z = p[3].z;

		return a;
	}
	Point getMax(void)
	{
		Point a = p[0];

		if (p[1].x > a.x)
			a.x = p[1].x;
		if (p[1].y > a.y)
			a.y = p[1].y;
		if (p[1].z > a.z)
			a.z = p[1].z;

		if (p[2].x > a.x)
			a.x = p[2].x;
		if (p[2].y > a.y)
			a.y = p[2].y;
		if (p[2].z > a.z)
			a.z = p[2].z;

		if (p[3].x > a.x)
			a.x = p[3].x;
		if (p[3].y > a.y)
			a.y = p[3].y;
		if (p[3].z > a.z)
			a.z = p[3].z;

		return a;
	}
	Boolean voxelicheck(Point& vmin, Point& vmax);
	friend void loadScene(void);
	friend istream& operator >> (istream& s, Plane& p);
	friend ostream& operator << (ostream& s, Plane& p);
};

istream& operator >> (istream& s, Plane& p);
ostream& operator << (ostream& s, Plane& p);


class Box : public Object
{
	public:

	Point min, max;

	Box(void);
	void init(Surface isurface, Point imin, Point imax);
	Boolean icheck(Ray& aray, Interdata& id);
	void intersect(Ray& aray, Node *nodeptr, Interdata& id);
	Point getMin(void)
	{
		return min;
	}
	Point getMax(void)
	{
		return max;
	}
	Boolean voxelicheck(Point& vmin, Point& vmax);
	friend void loadScene(void);
	friend istream& operator >> (istream& s, Box& p);
	friend ostream& operator << (ostream& s, Box& p);
};

istream& operator >> (istream& s, Box& p);
ostream& operator << (ostream& s, Box& p);


class Polygon : public Object
{
	public:

	Boolean maxx, maxy, maxz;	// True for the dominant axis in normal.
	FP d;			// The polygon's distance to origin.
	int vertexes;	// The number of vertexes in the polygon.
	FP *u, *v;		// Pointers to arrays containing the new vertexes.
	Point *vertex;	// A pointer to an array containing the original vertexes.
	Point min, max;	// The extents of the polygon, set during load.

	Polygon(void);
	void init(Surface isurface, int ivertexes);
	Boolean icheck(Ray& aray, Interdata& id);
	void intersect(Ray& aray, Node *nodeptr, Interdata& id);
	Point getMin(void)
	{
		return min;
	}
	Point getMax(void)
	{
		return max;
	}
	Boolean voxelicheck(Point& vmin, Point& vmax);
	friend void loadScene(void);
	friend istream& operator >> (istream& s, Polygon& p);
	friend ostream& operator << (ostream& s, Polygon& p);
};

istream& operator >> (istream& s, Polygon& p);
ostream& operator << (ostream& s, Polygon& p);


class Ring : public Object
{
	public:

	Point center;			// The center of the ring
	FP innerr, outerr, d;	// The inner and outer radiuses, and the distance

	Ring(void);
	void init(Surface isurface, Vector inormal, Point icenter, FP iinnerr, FP iouterr);
	Boolean icheck(Ray& aray, Interdata& id);
	void intersect(Ray& aray, Node *nodeptr, Interdata& id);
	Point getMin(void)
	{
		return center;
	}
	Point getMax(void)
	{
		return center;
	}
	Boolean voxelicheck(Point& vmin, Point& vmax);
	friend void loadScene(void);
	friend istream& operator >> (istream& s, Ring& p);
	friend ostream& operator << (ostream& s, Ring& p);
};

istream& operator >> (istream& s, Ring& p);
ostream& operator << (ostream& s, Ring& p);


#endif	// Of planar_h
