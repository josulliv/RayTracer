// quadric.cc		Quadric objects and related code

#include "raytrace.h"
#include "vector.h"
#include "miscobj.h"
#include "textures.h"
#include "object.h"
#include "quadric.h"

extern Texture *textptr[64];
extern int x, y;
extern Boolean used_by_scenebuilder;


Sphere::Sphere(void)
{
}

void Sphere::init(Surface isurface, Point icenter, FP ira)
{
	surface = isurface;
	center = icenter;
	ra = ira;
}

Boolean Sphere::icheck(Ray& aray, Interdata& id)
{
	// Algorithm from Eric Haines in Ray Tracing. (the geometric method)

	FP l2, d, tca;
	Boolean outside = true;
	Vector oc = center - aray.origin;

	// tca is the distance from origin to closest approach to the sphere center
	tca = oc * aray.direction;
	l2 = oc * oc;			//  l2 is length^2 of the origin to the center vector.
	if (l2 <= ras)
		outside = false;	//  The ray origin is inside the sphere.
	if ((tca < sigma) && (outside == true))
		return false;		//  The ray won't intersect the sphere.
	d = ras + sqr(tca) - l2;
	if (d < sigma)
		return false;
	if (outside == true)
	{

		id.t = tca - sqrt(d);
		if (fabs(id.t) < sigma)
			id.t = tca + sqrt(d);
	}
	else
	{
		id.t = tca + sqrt(d);
		if (fabs(id.t) < sigma)
			return false;
	}
	id.poi = aray.getPoi(id.t);
	id.normal = id.poi - center;
	id.normal.unitize();
	return true;
}

// Note, on the intersect function, a reflected ray will be generated only if
// kspec is non-zero AND entering is true. But the trace function will trace
// a reflected ray if kspec is non-zero, regardless of entering.  FIX THIS!!!


void Sphere::intersect(Ray& aray, Node *nodeptr, Interdata& id)
{
//	This procedure computes the specularly reflected and transmitted rays
//	generated by an intersection of the incident ray on the sphere.

	Ray transmitted, reflected;
	Vector incident, a, s;
	FP ci, n, q;
	Boolean entering = nodeptr->entering;
	Surface tsurface;
	tsurface = surface;

	incident = aray.direction;

	if ((surface.kspec > 0.0) && (entering == true))	// Compute the reflected ray
	{
		reflected.init(id.poi, reflect(incident, id.normal));
	}
	
	if (surface.ktran > 0.0)
	{
		if (entering == true)
		{
			n = surface.in;		// Inverse of the solid's n
			ci = id.normal.neg() * incident;	// The cosine of the angle of incidence
			entering = false;			// Invert the entering flag
		}
		else
		{
			n = surface.n;		// The solid's index of refraction n
			ci = id.normal * incident;	// The cosine of the angle of incidence
			id.normal = id.normal.neg();	// Turn around normal
			entering = true;			// Invert the entering flag
		}
		s = ((id.normal * ci) + incident) * n;
		q = s * s;				// The magnitude^2 of s
		if (sqrt(q) < 1)			// No T.I.R.
		{
			q = sqrt(1 - q);
			transmitted.init(id.poi, s - (id.normal * q));
		}
		else					// T.I.R. occurs - compute the reflected ray
		{
			q = -2 * (id.poi * id.normal);
			a = q * id.normal;
			transmitted.init(id.poi, id.poi + a);
//			transmitted.init(id.poi,  ((-2 * (id.poi * id.normal)) * id.normal) + id.poi);
		}
	}

	// If the surface has a diffuse component and a texture, then
	// compute the point of intersection in spherical coordinates.
	if ((surface.kdiff > 0.0) && (surface.texture != 0))
	{
		FP phi, theta, u, v;
		Vector sp, se, xx;
		sp.init(0,1,0);
		se.init(0,0,1);
		xx.init(1,0,0);

		phi = acos(id.normal.neg() * sp);
		v = phi / 3.14159265358979;
		if ((v < sigma) || (v == 1.0))
		{
			u = 0.0;
		}
		else
		{
			theta = (se * id.normal) / sin(phi);
			if (theta < -0.99999999)
				theta = -0.99999999;	// Clamp to in-range value for acos.
			else if (theta > 0.99999999)
				theta = 0.99999999;		// Clamp to in-range value for acos.
			theta = acos(theta) / 6.28318530718;

			if ((xx * id.normal) > 0.0)
			{
				u = theta;
			}
			else
			{
				u = 1 - theta;
			}
		}	// Note: u & v vary from 0 - 1

		tsurface.color.init(textptr[surface.texture]->getcolor(u, v));
	}
	nodeptr->init(id.poi, id.normal, tsurface, transmitted, reflected, 0, 0, false, false, entering);
}


Boolean Sphere::voxelicheck(Point& vmin, Point& vmax)
{
	FP dmax = 0, dmin = 0, a, b;

	a = sqr(center.x - vmin.x);
	b = sqr(center.x - vmax.x);
	dmax += max(a, b);
	if (center.x < vmin.x)
		dmin += a;
	else
	if (center.x > vmax.x)
		dmin += b;

	a = sqr(center.y - vmin.y);
	b = sqr(center.y - vmax.y);
	dmax += max(a, b);
	if (center.y < vmin.y)
		dmin += a;
	else
	if (center.y > vmax.y)
		dmin += b;

	a = sqr(center.z - vmin.z);
	b = sqr(center.z - vmax.z);
	dmax += max(a, b);
	if (center.z < vmin.z)
		dmin += a;
	else
	if (center.z > vmax.z)
		dmin += b;

	if ((dmin <= ras) && (ras <= dmax))
		return true;
	else
		return false;
}


istream& operator >> (istream& s, Sphere& p)
{
	s >> p.surface >> p.center >> p.ra;
	p.ras = sqr(p.ra);		// Compute the radius squared.
	return s;
}

ostream& operator << (ostream& s, Sphere& p)
{
	s << p.surface << p.center << p.ra << "\n";
	return s;
}



Cylinder::Cylinder(void)
{
	ra = 16.0;
	ras = 256.0;
	h = 0.0;
	base.init(0.0, 0.0, 0.0);
	end.init(0.0, 100.0, 0.0);
}

Boolean Cylinder::icheck(Ray& aray, Interdata& id)
{
	Ray newray;
	Point center;

//	newray.origin = aray.origin - base;		// Translate the ray

	id.poi = aray.getPoi(id.t);

	// Compute a point along the axis of the cylinder directly
	// beneath the POI, for use in computing the normal.

	id.normal = id.poi - center;
	id.normal.unitize();
	return true;
}

void Cylinder::intersect(Ray& aray, Node *nodeptr, Interdata& id)
{
//	This procedure computes the specularly reflected and transmitted rays
//	generated by an intersection of the incident ray on the cylinder.

	Ray transmitted, reflected;
	Vector incident, a, s;
	FP ci, n, q;
	Boolean entering = nodeptr->entering;

	incident = aray.direction;

	if ((surface.kspec > 0.0) && (entering == true))	// Compute the reflected ray
	{
		reflected.init(id.poi, reflect(incident, id.normal));
	}
	if (surface.ktran > 0.0)
	{
		if (entering == true)
		{
			n = surface.in;			// Inverse of the solid's n
			ci = id.normal.neg() * incident;	// The cosine of the angle of incidence
			entering = false;				// Invert the entering flag
		}
		else
		{
			n = surface.n;			// The solid's index of refraction n
			ci = id.normal * incident;		// The cosine of the angle of incidence
			id.normal = id.normal.neg();		// Turn around normal
			entering = true;				// Invert the entering flag
		}
		s = ((id.normal * ci) + incident) * n;
		q = s * s;				// The magnitude^2 of s
		if (sqrt(q) < 1)			// No T.I.R.
		{
			q = sqrt(1 - q);
			transmitted.init(id.poi, s - (id.normal * q));
		}
		else					// T.I.R. occurs - compute the reflected ray
		{
			q = -2 * (id.poi * id.normal);
			a = q * id.normal;
			transmitted.init(id.poi, id.poi + a);
		}
	}

	// If the surface has a diffuse component and a texture, then
	// compute the point of intersection in cylindrical coordinates.
	if ((surface.kdiff > 0.0) && (surface.texture != 0))
	{
		FP u, v;	// Note: u & v vary from 0 - 1

		v = id.poi.z / h;
		u = acos(id.poi.x / ra) / 6.28318530718;
		if (id.poi.y < sigma)
		{
			u = 1 - u;
		}
		surface.color.init(textptr[surface.texture]->getcolor(u, v));
	}
	nodeptr->init(id.poi, id.normal, surface, transmitted, reflected, 0, 0, false, false, entering);
}


Boolean Cylinder::voxelicheck(Point& vmin, Point& vmax)
{
	return false;
}


istream& operator >> (istream& s, Cylinder& c)
{
	s >> c.surface >> c.base >> c.end >> c.ra;
	c.ras = sqr(c.ra);		// Compute the radius squared.
	c.h = c.base / c.end;	// Compute the cylinder's height.
	return s;
}

ostream& operator << (ostream& s, Cylinder& c)
{
	s << c.surface << c.base << c.end << c.ra << "\n";
	return s;
}


Quadric::Quadric(void)
{
	a = 0;
	b = 0;
	c = 0;
	d = 0;
	e = 0;
	f = 0;
	g = 0;
	h = 0;
	i = 0;
	j = 0;
}

void Quadric::init(Surface isurface, FP ia, FP ib, FP ic, FP id, FP ie, FP iif, FP ig, FP ih, FP ii, FP ij)
{
	surface = isurface;
	a = ia;
	b = ib;
	c = ic;
	d = id;
	e = ie;
	f = iif;
	g = ig;
	h = ih;
	i = ii;
	j = ij;
}

Boolean Quadric::icheck(Ray& aray, Interdata& id)
{
	FP aq, bq, cq, t0, t1, dis;
	FP xo = aray.origin.x;
	FP yo = aray.origin.y;
	FP zo = aray.origin.z - 100.0;
	FP xd = aray.direction.dx;
	FP yd = aray.direction.dy;
	FP zd = aray.direction.dz;

	aq = a * sqr(xd) + 2 * b * xd * yd + 2 * c * xd * zd
	+ e * sqr(yd) + 2 * f * yd * zd + h * sqr(zd);

	bq = 2 * (a * xo * xd + b * (xo * yd + xd * yo) + c * (xo * zd + xd * zo)
	+ d * xd + e * yo * yd + f * (yo * zd + yd * zo) + g * yd + h * zo * zd + i * zd);

	cq = a * sqr(xo) + 2 * b * xo * yo + 2 * c * xo * zo + 2 * d * xo +
	e * sqr(yo) + 2 * f * yo * zo + 2 * g * yo + h * sqr(zo) + 2 * i * zo + j;

	if (fabs(aq) > sigma)	// If aq != 0
	{
		dis = sqr(bq) - 4 * aq * cq;
		if (dis < sigma)
		{
			return false;
		}
		t0 = (-bq - sqrt(dis)) / (2 * aq);
		t1 = (-bq + sqrt(dis)) / (2 * aq);
		if (t0 > sigma)	// The smallest positive t is the distance.
		{
			id.t = t0;
			if (t1 < t0)
			{
				id.t = t1;
			}
		}
		else
			id.t = t1;
	}
	else	// If aq = 0
	{
		id.t = -cq / bq;
	}
	if (id.t < sigma)
		return false;	// Compensate for FP errors...
	id.poi = aray.getPoi(id.t);
	id.normal.dx = a * id.poi.x + b * id.poi.y + c * id.poi.z + d;
	id.normal.dy = b * id.poi.x + e * id.poi.y + f * id.poi.z + g;
	id.normal.dz = c * id.poi.x + f * id.poi.y + h * id.poi.z + i;
	id.normal.unitize();
	if (id.normal * aray.direction > 0)
	{	// Then reverse the normal
		id.normal.neg();
	}
	return true;
}

void Quadric::intersect(Ray& aray, Node *nodeptr, Interdata& id)
{
	Ray transmitted, reflected;
	Vector incident, x, s;
	FP ci, n, q;
	Boolean entering = nodeptr->entering;

	if ((surface.kspec > 0.0) && (entering == true))	// Compute the reflected ray
	{
		reflected.init(id.poi, reflect(aray.direction, id.normal));
	}

	if (surface.ktran > 0.0)
	{
		if (entering == true)
		{
			n = surface.in;							// Inverse of the solid's n
			ci = id.normal.neg() * aray.direction;	// The cosine of the angle of incidence
			entering = false;							// Invert the entering flag
		}
		else
		{
			n = surface.n;						// The solid's index of refraction n
			ci = id.normal * aray.direction;	// The cosine of the angle of incidence
			id.normal = id.normal.neg();			// Turn around normal
			entering = true;					// Invert the entering flag
		}
		s = ((id.normal * ci) + aray.direction) * n;
		q = s * s;			// The magnitude^2 of s
		if (sqrt(q) < 1)	// No T.I.R.
		{
			q = sqrt(1 - q);
			transmitted.init(id.poi, s - (id.normal * q));
		}
		else			// T.I.R. occurs - compute the reflected ray
		{
			q = -2 * (id.poi * id.normal);
			x = q * id.normal;
			transmitted.init(id.poi, id.poi + x);
		}
	}

//	if (surface.kdiff > 0.0){}	// I don't know how to inverse map a quadric!

	nodeptr->init(id.poi, id.normal, surface, transmitted, reflected, 0, 0, false, false, entering);
}


Boolean Quadric::voxelicheck(Point& vmin, Point& vmax)
{
	return false;
}


istream& operator >> (istream& s, Quadric& c)
{
	s >> c.surface >> c.a >> c.b >> c.c >> c.d >> c.e >> c.f >> c.g >> c.h >>
	c.i >> c.j;
	return s;
}

ostream& operator << (ostream& s, Quadric& c)
{
	s << c.surface << c.a << "\n" << c.b << "\n" << c.c << "\n" << c.d << "\n"
	<< c.e << "\n" << c.f << "\n" << c.g << "\n" << c.h << "\n" << c.i << "\n"
	<< c.j << "\n";
	return s;
}

