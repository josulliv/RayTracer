// miscobj.h	Miscellaneous object class definitions
// (Color, Point, Surface, Ray, Node)

#ifndef miscobjects_h
#define miscobjects_h

class Color
{
	public:

	FP r, g, b;

	Color(void)
	{
		r = 0;
		g = 0;
		b = 0;
	}

	Color(FP ir, FP ig, FP ib)
	{
		r = ir;
		g = ig;
		b = ib;
	}

	void init(FP ir, FP ig, FP ib)
	{
		r = ir;
		g = ig;
		b = ib;
	}

	void init(Color a)
	{
		r = a.r;
		g = a.g;
		b = a.b;
	}

	void scale(FP q)
	{
		r /= q;
		g /= q;
		b /= q;
	}

	Color operator + (Color a)
	{
		return Color(r + a.r, g + a.g, b + a.b);
	}

	Color operator * (Color a)
	{
		return Color(r * a.r, g * a.g, b * a.b);
	}

	Color operator * (FP a)
	{
		return Color(r * a, g * a, b * a);
	}

	void unitize(void);
	FP unitizel(void);
	friend istream& operator >> (istream& s, Color& c);
	friend ostream& operator << (ostream& s, Color& c);
};

istream& operator >> (istream& s, Color& c);
ostream& operator << (ostream& s, Color& c);

Color VtoC(Vector a);	// Converts a vector to a color.


class Point
{
	public:

	FP x, y, z;

	Point(void)
	{
		x = 0;
		y = 0;
		z = 0;
	}
	Point(FP ix, FP iy, FP iz)
	{
		x = ix;
		y = iy;
		z = iz;
	}
	void init(FP ix, FP iy, FP iz)
	{
		x = ix;
		y = iy;
		z = iz;
	}
	Point operator + (Point a)	// (Point + Point) -> Point
	{
		return Point(x + a.x, y + a.y, z + a.z);
	}
	Vector operator - (Point a)	// (Point - Point) -> Vector
	{
		return Vector(x - a.x, y - a.y, z - a.z);
	}
	Vector operator + (Vector a)	// (Point + Vector) -> Vector
	{
		return Vector(x + a.dx, y + a.dy, z + a.dz);
	}
	FP operator * (Vector a)		// (Point DOT Vector) -> FP
	{
		return (x * a.dx + y * a.dy + z * a.dz);
	}
	FP operator / (Point a)		// Compute the distance between two points.
	{
		return sqrt(sqr(x - a.x) + sqr(y - a.y) + sqr(z - a.z));
	}

//	The next two functions do a component-wise comparison of the two points.
//	If all components of a are greater (or less) than the corresponding
//	components in the other argument, the function returns true, else false.

	Boolean operator << (Point a)
	{
		if ((x < a.x) || (y < a.y) || (z < a.z))
			return true;
		else
			return false;
	}
	Boolean operator >> (Point a)
	{
		if ((x > a.x) || (y > a.y) || (z > a.z))
			return true;
		else
			return false;
	}

	Boolean operator < (Point a)
	{
		if ((x < a.x) && (y < a.y) && (z < a.z))
			return true;
		else
			return false;
	}
	Boolean operator > (Point a)
	{
		if ((x > a.x) && (y > a.y) && (z > a.z))
			return true;
		else
			return false;
	}
	Boolean operator == (Point a)
	{
		if ((x == a.x) && (y == a.y) && (z == a.z))
			return true;
		else
			return false;
	}

	friend istream& operator >> (istream& s, Point& p);
	friend ostream& operator << (ostream& s, Point& p);
};

istream& operator >> (istream& s, Point& p);
ostream& operator << (ostream& s, Point& p);

Point VtoP(Vector a);   // This function converts a vector to a point.

class Surface
{
	// Diffuse & specular reflection coefficients, transmission coefficient,
	// and index of refraction:
	public:

	FP kdiff, kspec, ktran, n, in;
	Color color;    // Surface color.
	int texture;	// The texture number (0 = no texture).

	Surface(int itexture = 0, FP kd = 1, FP ks = 0, FP kt = 0, FP tn = 1)
	{
		texture = itexture;
		kdiff = kd;
		kspec = ks;
		ktran = kt;
		n = tn;
		in = 1.0 / tn;
	}
	void init(int itexture, FP kd, FP ks, FP kt, FP tn, Color& icolor)
	{
		texture = itexture;
		kdiff = kd;
		kspec = ks;
		ktran = kt;
		n = tn;
		in = 1.0 / tn;
		color = icolor;
	}
	friend istream& operator >> (istream& s, Surface& p);
	friend ostream& operator << (ostream& s, Surface& p);
};

istream& operator >> (istream& s, Surface& p);
ostream& operator << (ostream& s, Surface& p);


class Ray
{
	public:

	Point origin;
	Vector direction;

	Ray(Point iorigin, FP idx, FP idy, FP idz);
	Ray(Point iorigin, Vector idirection);
	Ray(void);
	FP init(Point iorigin, Vector idirection);
	FP init(Point iorigin, FP idx, FP idy, FP idz);
	Point getPoi(FP t)
	{
		return VtoP(origin + direction * t);
	}
	FP unitize(void)
	{
		// This function unitizes the vector and returns its length.
		return direction.unitizel();
	}
	friend FP unitize(void);
	friend istream& operator >> (istream& s, Ray& r);
	friend ostream& operator << (ostream& s, Ray& r);
};

istream& operator >> (istream& s, Ray& r);
ostream& operator << (ostream& s, Ray& r);


class Node
{
// This class contains the information in a node of the intersection/
// illumination tree for the current ray.

	public:

//	The light from the transmitted ray, the reflected ray, and the light
//	hitting the point of intersection, including light from the light sources,
//	ambient light, and any other light I can think of, and their weights:
	
	Point poi;	// 	The point of intersection:

// 	The surface normal at the point of intersection:

	Vector normal;

// 	The poi surface & material characteristics:

	Surface surface;

//	The pointers to the next two branches on the tree:

	Node *tptr;
	Node *rptr;

//	The reflected and transmitted rays:

	Ray transmitted, reflected;

//	Branch flags (true if the corresponding branch exists):

	Boolean tflag, rflag;

//	Entering/leaving flag (true if entering a solid, false otherwise).
//	It starts out true, because the ray begins in air (presumably):

	Boolean entering;

	public:

	Node(void);

	void init(Point ipoi, Vector inormal, Surface isurface, Ray itr,
	Ray irr, Node *itp, Node *irp, Boolean itflag, Boolean irflag,
	Boolean ienter);
};


class Interdata
{
	public:
	
	FP t;			// The distance to the poi.
	Point poi;		// The point of intersection
	Vector normal;	// The normal at the poi

	void init(FP it, Point ip, Vector in)
	{
		t = it;
		poi = ip;
		normal = in;
	}
};

#endif	// Of miscobjects_h
