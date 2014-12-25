//  Vector.h   The Vector class declaration       J. O'Sullivan  12/25/1990

#ifndef _vector_h
#define _vector_h

#include "raytrace.h"

class Vector
{
	public:

	FP dx, dy, dz;

	Vector(FP ix = 0, FP iy = 0, FP iz = 0)
	{
		dx = ix;
		dy = iy;
		dz = iz;
	}
	void init(FP ix, FP iy, FP iz)
	{
		dx = ix;
		dy = iy;
		dz = iz;
	}
	Vector operator * (FP a)	// vector * scalar  (V * s)
	{
		return Vector(dx * a, dy * a, dz * a);
	}
	FP operator * (Vector a)	// vector DOT vector (26% faster than macro)
	{
		return (dx * a.dx + dy * a.dy + dz * a.dz);
	}
	Vector operator + (Vector a)	// vector + vector
	{
		return Vector(dx + a.dx, dy + a.dy, dz + a.dz);
	}
	Vector operator - (Vector a)	// vector - vector
	{
		return Vector(dx - a.dx, dy - a.dy, dz - a.dz);
	}
	Vector operator / (FP a)	// vector / scalar
	{
		return Vector(dx / a, dy / a, dz / a);
	}
	Vector neg(void)		// negate(vector)
	{
		return Vector(-dx, -dy, -dz);
	}
	FP length(void)			// The vector's length
	{
		return (sqrt(dx*dx + dy*dy + dz*dz));
	}
	FP length2(void)		// The vector's length squared
	{
		return (dx*dx + dy*dy + dz*dz);
	}

	void unitize(void);
	FP unitizel(void);
	Vector unitizev(void);
	friend Vector operator * (FP a, Vector& b);
	friend istream& operator >> (istream& s, Vector& p);
	friend ostream& operator << (ostream& s, Vector& p);
};

istream& operator >> (istream& s, Vector& p);
ostream& operator << (ostream& s, Vector& p);
Vector operator * (FP a, Vector& b);
FP vecnormcross(Vector& a, Vector& b, Vector& r);
Vector reflect(Vector& incident, Vector& normal);
Vector rotate(Vector axis, Vector mark, FP theta);
FP getangle(Vector& a, Vector& b);

#endif	// of _vector_h
