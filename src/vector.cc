// Vector.cc	All Vector member functions and other related functions
// J. O'Sullivan

#include "raytrace.h"
#include "vector.h"

void Vector::unitize(void)
{
	// This function unitizes the vector.

	FP q;
	q = sqrt(dx*dx + dy*dy + dz*dz);
	if (q == 0.0)
		q = SIGMA;	// Prevent divide by zero
	dx /= q;
	dy /= q;
	dz /= q;
}

FP Vector::unitizel(void)
{
	// This function unitizes the vector and returns its length.

	FP q;
	q = sqrt(dx*dx + dy*dy + dz*dz);
	if (q == 0.0)
		q = SIGMA;	// Prevent divide by zero
	dx /= q;
	dy /= q;
	dz /= q;
	return q;
}

Vector Vector::unitizev(void)
{
	// This function unitizes the vector and returns it.

	FP q;
	q = sqrt(dx*dx + dy*dy + dz*dz);
	if (q == 0.0)
		q = SIGMA;	// Prevent divide by zero
	dx /= q;
	dy /= q;
	dz /= q;
	return Vector(dx, dy, dz);
}

FP vecnormcross(Vector& a, Vector& b, Vector& r)
{
	r.dx = a.dy * b.dz - a.dz * b.dy;
	r.dy = a.dz * b.dx - a.dx * b.dz;
	r.dz = a.dx * b.dy - a.dy * b.dx;
	return r.unitizel();
}

Vector reflect(Vector& incident, Vector& normal)
{
	return (incident + normal * (-2 * (normal * incident)));
}

istream& operator >> (istream& s, Vector& p)
{
	s >> p.dx >> p.dy >> p.dz;
	return s;
}

ostream& operator << (ostream& s, Vector& p)
{
	s << p.dx << "\n" << p.dy << "\n" << p.dz << "\n";
	return s;
}

Vector operator * (FP a, Vector& b)
{
	return Vector(b.dx * a, b.dy * a, b.dz * a);
}


Vector rotate(Vector axis, Vector mark, FP theta)
{
	FP t, ct, st, xx, yy, zz;
	Vector temp;

	t = 1 - cos(theta);
	ct = cos(theta);
	st = sin(theta);

	xx = mark.dx * (t * axis.dx * axis.dx + ct) + mark.dy * (t * axis.dx * axis.dy + st * axis.dz) + mark.dz * (t * axis.dx * axis.dz - st * axis.dy);
	
	yy = mark.dx * (t * axis.dx * axis.dy - st * axis.dz) + mark.dy * (t * axis.dy * axis.dy + ct) + mark.dz * (t * axis.dy * axis.dz + st * axis.dx);
	
	zz = mark.dx * (t * axis.dx * axis.dz + st * axis.dy) + mark.dy * (t * axis.dy * axis.dz - st * axis.dx) + mark.dz * (t * axis.dz * axis.dz + ct);
	
	temp.init(xx, yy, zz);

	return temp;
}

FP getangle(Vector& a, Vector& b)
{
	return acos((a.dx * b.dx + a.dy * b.dy + a.dz * b.dz) /
	(sqrt(sqr(a.dx) + sqr(a.dy) + sqr(a.dz)) *
	sqrt(sqr(b.dx) + sqr(b.dy) + sqr(b.dz))));
}

