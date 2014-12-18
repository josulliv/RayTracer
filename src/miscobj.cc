// Miscobjects.cc	Miscellaneous object's member function definitions.
// Color, Point, Surface, Ray, Node

#include "raytrace.h"
#include "vector.h"
#include "miscobj.h"


istream& operator >> (istream& s, Color& c)
{
	s >> c.r >> c.g >> c.b;
	return s;
}

ostream& operator << (ostream& s, Color& c)
{
	s << c.r << "\n" << c.g << "\n" << c.b << "\n";
	return s;
}

void Color::unitize(void)
{
	// This function unitizes the color.

	FP q = r;	// First extract the largest color component.
	if (q < g)
		q = g;
	if (q < b)
		q = b;

	if (q == 0.0)	// Prevent divide by zero!
		q = 1.0;

	r /= q;		// Then scale down the color components.
	g /= q;
	b /= q;
}

FP Color::unitizel(void)
{
	// This function unitizes the color and returns its intensity.

	FP q = r;	// First extract the largest color component.
	if (q < g)
		q = g;
	if (q < b)
		q = b;

	if (q == 0.0)	// Prevent divide by zero!
		q = 1.0;

	r /= q;		// Then scale down the color components.
	g /= q;
	b /= q;
	return q;
}

Color VtoC(Vector a)	// Converts a vector to a color.
{
	return Color(a.dx, a.dy, a.dz);
}


istream& operator >> (istream& s, Point& p)
{
	s >> p.x >> p.y >> p.z;
	return s;
}

ostream& operator << (ostream& s, Point& p)
{
	s << p.x << "\n" << p.y << "\n" << p.z << "\n";
	return s;
}


Point VtoP(Vector a)   // This function converts a vector to a point.
{
	return Point(a.dx, a.dy, a.dz);
}


istream& operator >> (istream& s, Surface& p)
{
	s >> p.texture >> p.kdiff >> p.kspec >> p.ktran >> p.n >> p.color;
	p.in = 1.0 / p.n;
	return s;
}

ostream& operator << (ostream& s, Surface& p)
{
	s << p.texture << "\n" << p.kdiff << "\n" << p.kspec << "\n" <<
	p.ktran << "\n" << p.n << "\n" << p.color;
	return s;
}


Ray::Ray(Point iorigin, FP idx, FP idy, FP idz)
{
	origin = iorigin;
	direction.init(idx, idy, idz);
	direction.unitize();
}

Ray::Ray(Point iorigin, Vector idirection)
{
	origin = iorigin;
	direction = idirection;
	direction.unitize();
}

Ray::Ray(void)
{
}

FP Ray::init(Point iorigin, Vector idirection)
{
	origin = iorigin;
	direction = idirection;
	return direction.unitizel();
}

FP Ray::init(Point iorigin, FP idx, FP idy, FP idz)
{
	origin = iorigin;
	direction.init(idx, idy, idz);
	return direction.unitizel();
}

istream& operator >> (istream& s, Ray& r)
{
	s >> r.origin >> r.direction;
	return s;
}

ostream& operator << (ostream& s, Ray& r)
{
	s << r.origin << r.direction;
	return s;
}


Node::Node(void)
{
	tflag = false;
	rflag = false;
	entering = true;
}

void Node::init(Point ipoi, Vector inormal, Surface isurface, Ray itr, Ray irr,
Node *itp, Node *irp, Boolean itflag, Boolean irflag, Boolean ienter)
{
	poi = ipoi;
	normal = inormal;
	surface = isurface;
	transmitted = itr;
	reflected = irr;
	tptr = itp;
	rptr = irp;
	tflag = itflag;
	rflag = irflag;
	entering = ienter;
}
