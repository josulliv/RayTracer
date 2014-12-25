// planar.cc		Planar objects and related code

#include "raytrace.h"
#include "vector.h"
#include "miscobj.h"
#include "textures.h"
#include "object.h"
#include "planar.h"

extern Texture *textptr[64];
extern int x, y;
extern Boolean used_by_scenebuilder;

Object::Object(void)
{
}


Orthoplane::Orthoplane(void)
{
}

void Orthoplane::init(Vector inormal, Surface isurface, FP id, Point imin, Point imax)
{
	normal = inormal;
	surface = isurface;
	d = id;
	min = imin;
	max = imax;
}


Boolean Orthoplane::icheck(Ray& aray, Interdata& id)
{
/*
	This function determines if the ray "aray" intersects the
	plane.  If not, it returns false.  If intersection occurs, it
	computes the distance to the point of intersection (id.t) and
	the point of intersection (id.poi), and stores it in the class
	Interdata.
*/

	FP vd;

	vd = normal * aray.direction;
	if (fabs(vd) < SIGMA)
		return false;	// It's parallel or hits the plane edge-on.

	if (vd < 0)
		id.t = -(aray.origin * normal + d) / vd;
	else
		id.t = -(aray.origin * normal.neg() + d) / vd;

	if (id.t < 0.001)
		return false;	// The plane's in back of the ray

	id.poi = aray.getPoi(id.t);

	// Now we've established the ray intersects the plane at some point, and
	// have computed the point of intersection.  Now determine if the poi is
	// within the specified boundaries of this instance of plane.

	if ((id.poi << min) || (id.poi >> max))
		return false;
	else return true;
}


void Orthoplane::intersect(Ray& aray, Node *nodeptr, Interdata& id)
{
/*
	This method computes the reflected and transmitted vectors using the
	incident ray, the surface normal, and the surface characteristics.

	Since a plane has no thickness, the incident ray will be transmitted
	with no distortion or refraction (unless ktran = 0.0):

	Remember, change the light coming from the transmitted ray by the color of
	the plane.  Also change the di factor by the color of the plane.  I'm not
	sure what to do about the light from the reflected ray.  Perhaps change
	that by the plane color also.  Or should both an absorption and reflection
	spectrum be stored for each surface/material?  And can that spectrum be
	stored in a Color?  Also, generate and record weights for the transmitted
	ray, the reflected ray, and the di factor.
*/

	Surface tsurface = surface;
	FP xx, yy;

	if (surface.texture != 0)
	{
		// Planar inverse mapping: xx & yy range from 0 to 1.
		// The inverse mapping algorithm below requires an orthogonal plane:

		if (fabs(normal.dy) == 1.0)
		{
			xx = (id.poi.x - min.x) / (max.x - min.x);
			yy = (id.poi.z - min.z) / (max.z - min.z);
		}
		else
		if (fabs(normal.dz) == 1.0)
		{
			xx = (id.poi.x - min.x) / (max.x - min.x);
			yy = (id.poi.y - min.y) / (max.y - min.y);
		}
		else
		if (fabs(normal.dx) == 1.0)
		{
			xx = (id.poi.z - min.z) / (max.z - min.z);
			yy = (id.poi.y - min.y) / (max.y - min.y);
		}
		tsurface.color.init(textptr[tsurface.texture]->getcolor(xx, yy));
	}
	nodeptr->init(id.poi, normal, tsurface, Ray(id.poi, aray.direction),
	Ray(id.poi, reflect(aray.direction, normal)), 0, 0, false, false, false);
}


Boolean Orthoplane::voxelicheck(Point& vmin, Point& vmax)
{
	// Determine if the orthogonal plane intersects with the box.
	// First, a few quick tests.

	if (((max.x <= vmin.x) || (min.x >= vmax.x)) ||
	((max.y <= vmin.y) || (min.y >= vmax.y)) ||
	((max.z <= vmin.z) || (min.z >= vmax.z)))
		return false;
	else
		return true;
}


istream& operator >> (istream& s, Orthoplane& p)
{
	s >> p.normal >> p.surface >> p.d >> p.min >> p.max;
	return s;
}

ostream& operator << (ostream& s, Orthoplane& p)
{
	s << p.normal << p.surface << p.d << "\n" << p.min << p.max;
	return s;
}



Plane::Plane(void)
{
}


void Plane::init(Surface isurface, Point ip0, Point ip1, Point ip2)
{
	Vector x, y;

	surface = isurface;
	p[0] = ip0;
	p[1] = ip1;
	p[2] = ip2;

	// Compute the fourth point:

	x = p[1] - p[0];
	d = x.unitizel();
	p[3] = p[2] + VtoP(x.neg() * d);

	// Next, compute the plane's normal and distance:

	x = p[1] - p[0];
	y = p[2] - p[0];
	vecnormcross(x, y, normal);	// Compute the plane's normal
	x.init(-p[0].x, -p[0].y, -p[0].z);
	d = x * normal;		// Compute the plane's distance
}


Boolean Plane::icheck(Ray& aray, Interdata& id)
{
/*
	This function determines if the ray "aray" intersects the
	plane.  If not, it returns false.  If intersection occurs, it
	computes the distance to the point of intersection (id.t) and
	the point of intersection (id.poi), and stores it in the class
	Interdata.
*/

	FP vd, uu, vv, up[4], vp[4];
	int x, nc, sh, nsh, a, b;

	vd = normal * aray.direction;
	if (fabs(vd) < SIGMA)
		return false;	// It's parallel or hits the plane edge-on.

	if (vd < 0)
		id.t = -(aray.origin * normal + d) / vd;
	else
		id.t = -(aray.origin * normal.neg() + d) / vd;

	if (id.t < 0.001)
		return false;	// The plane's in back of the ray

	id.poi = aray.getPoi(id.t);

	// Now we've established the ray intersects the plane at some point, and
	// have computed the point of intersection.  Now determine if the poi is
	// within the specified boundaries of this instance of plane.

	if (maxx == true)
	{
		uu = id.poi.y;
		vv = id.poi.z;
	}
	else if (maxy == true)
	{
		uu = id.poi.x;
		vv = id.poi.z;
	}
	else if (maxz == true)
	{
		uu = id.poi.x;
		vv = id.poi.y;
	}

	// Next, translate the polygon so origin = POI:

	for (x = 0; x < 4; x++)
	{
		up[x] = u[x] - uu;
		vp[x] = v[x] - vv;
	}

	// Next, set up the counters:

	nc = 0;
	if (vp[0] >= 0.0)
		sh = 1;
	else
		sh = -1;

	for (a = 0; a < 4; a++)
	{
		b = (a + 1) % 4;
		if (vp[b] < 0.0)
			nsh = -1;
		else
			nsh = 1;

		if (sh != nsh)
		{
			if ((up[a] >= 0.0) && (up[b] >= 0.0))
				nc++;	// The line crosses +up
			else if ((up[a] >= 0.0) || (up[b] >= 0.0))
			{
				if (up[a] - vp[a] * (up[b] - up[a])/(vp[b] - vp[a]) > 0.0)
					nc++;	// The line crosses +up
			}
			sh = nsh;
		}
	}

	return (Boolean)(nc % 2);	// If nc is odd, it's inside.
}


void Plane::intersect(Ray& aray, Node *nodeptr, Interdata& id)
{
/*
	This method computes the reflected and transmitted vectors using the
	incident ray, the surface normal, and the surface characteristics.

	Since a plane has no thickness, the incident ray will be transmitted
	with no distortion or refraction (unless ktran = 0.0):

	Remember, change the light coming from the transmitted ray by the color of
	the plane.  Also change the di factor by the color of the plane.  I'm not
	sure what to do about the light from the reflected ray.  Perhaps change
	that by the plane color also.  Or should both an absorption and reflection
	spectrum be stored for each surface/material?  And can that spectrum be
	stored in a Color?  Also, generate and record weights for the transmitted
	ray, the reflected ray, and the di factor.
*/

	Surface tsurface = surface;
	FP uu, vv;

	if (surface.texture != 0)
	{
		// Planar inverse mapping: uu & vv range from 0 to 1.

		uu = ((id.poi * nc) - du0) / (du1 - (id.poi * na));
		vv = ((id.poi * nb) - dv0) / (dv1 - (id.poi * na));
		tsurface.color.init(textptr[surface.texture]->getcolor(uu, vv));
	}

	nodeptr->init(id.poi, normal, tsurface, Ray(id.poi, aray.direction),
	Ray(id.poi, reflect(aray.direction, normal)), 0, 0, false, false, false);
}


Boolean Plane::voxelicheck(Point& vmin, Point& vmax)
{
	return false;
}


istream& operator >> (istream& s, Plane& p)
{
	Vector a, b, pa, pb, pc, pd, m;
	FP leglength;
	int x;

	s >> p.surface >> p.p[0] >> p.p[1] >> p.p[2];

	// Set up for inverse mapping...

	m = p.p[1] - p.p[0];
	leglength = m.unitizel();
	p.p[3] = p.p[2] + VtoP(m.neg() * leglength);

	pa = (p.p[0] - p.p[3]) + (p.p[2] - p.p[1]);
	pb = p.p[3] - p.p[0];
	pc = p.p[1] - p.p[0];
	pd.init(p.p[0].x, p.p[0].y, p.p[0].z);
	vecnormcross(pa, p.normal, p.na);
	vecnormcross(pc, p.normal, p.nc);
	p.du0 = p.nc * pd;
	p.du1 = (p.na * pd) + (p.nc * pb);
	vecnormcross(pb, p.normal, p.nb);
	p.dv0 = p.nb * pd;
	p.dv1 = (p.na * pd) + (p.nb * pc);

	// Set up for extents checking...
	// Next, compute the plane normal and distance from the first three vertexes:

	a = p.p[1] - p.p[0];
	b = p.p[2] - p.p[0];
	vecnormcross(a, b, p.normal);

	a.init(-p.p[0].x, -p.p[0].y, -p.p[0].z);
	p.d = a * p.normal;

	// Now, throw away the coordinate whose plane-equation magn. is greatest.

	if ((fabs(p.normal.dx) > fabs(p.normal.dy)) && (fabs(p.normal.dx) > fabs(p.normal.dz)))
	{
		// Then x is greatest

		p.maxx = true;
		p.maxy = false;
		p.maxz = false;
		for (x = 0; x < 4; x++)
		{
			p.u[x] = p.p[x].y;
			p.v[x] = p.p[x].z;
		}
	}
	else
	if (fabs(p.normal.dy) > fabs(p.normal.dz))
	{
		// Then y is greatest

		p.maxx = false;
		p.maxy = true;
		p.maxz = false;
		for (x = 0; x < 4; x++)
		{
			p.u[x] = p.p[x].x;
			p.v[x] = p.p[x].z;
		}
	}
	else
	{
		// Ok, then z is greatest!!!

		p.maxx = false;
		p.maxy = false;
		p.maxz = true;
		for (x = 0; x < 4; x++)
		{
			p.u[x] = p.p[x].x;
			p.v[x] = p.p[x].y;
		}
	}


	return s;
}

ostream& operator << (ostream& s, Plane& p)
{
	s << p.surface << p.p[0] << p.p[1] << p.p[2];
	return s;
}


Box::Box(void)
{
}

void Box::init(Surface isurface, Point imin, Point imax)
{
	surface = isurface;
	min = imin;
	max = imax;
}

Boolean Box::icheck(Ray& aray, Interdata& id)
{

	FP tn, tf, t1, t2, a;

//	This function determines if aray intersects with the box.  If it does, it
//	computes the point of intersection, and the distance.
//	From Ray Tracing.

	if ((fabs(aray.direction.dx) < 0.000001) && ((aray.origin.x < min.x) || (aray.origin.x > max.x)))
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
	if ((fabs(aray.direction.dy) < 0.000001) && ((aray.origin.y < min.y) || (aray.origin.y > max.y)))
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
	if ((fabs(aray.direction.dz) < 0.000001) && ((aray.origin.z < min.z) || (aray.origin.z > max.z)))
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
	id.t = tn;
	id.poi = aray.getPoi(id.t);
	return true;
}


void Box::intersect(Ray& aray, Node *nodeptr, Interdata& id)
{
	Ray transmitted;
	Vector incident = aray.direction;
	FP ci;

	if (fabs(id.poi.z - min.z) < 0.0000001)
		id.normal.init(0,0,-1.0);
	else
	if (fabs(id.poi.z - max.z) < 0.0000001)
		id.normal.init(0,0,1.0);
	else
	if (fabs(id.poi.x - min.x) < 0.0000001)
		id.normal.init(-1.0,0,0);
	else
	if (fabs(id.poi.x - max.x) < 0.0000001)
		id.normal.init(1.0,0,0);
	else
	if (fabs(id.poi.y - min.y) < 0.0000001)
		id.normal.init(0,-1.0,0);
	else
	if (fabs(id.poi.y - max.y) < 0.0000001)
		id.normal.init(0,1.0,0);

	// Next, compute the transmitted ray if there is one (if ktran > 0):

	if (surface.ktran > 0.0)
	{
		ci = id.normal * incident.neg();
		transmitted.init(id.poi, (incident * surface.in) + id.normal * (surface.in * ci - sqrt(1 + (sqr(surface.in) * (sqr(ci) - 1)))));
	}

	// If the surface has a diffuse component and a texture,
	// then compute the color:

	if (surface.texture != 0)
	{
	}

	nodeptr->init(id.poi, id.normal, surface, transmitted,
	Ray(id.poi, reflect(incident, id.normal)), 0, 0, false, false, false);
}


Boolean Box::voxelicheck(Point& vmin, Point& vmax)
{
	// If the box is inside the voxel, return true.
	// If the voxel is inside the box, return false.
	// This should be similar to the orthoplane function.

	if (((max.x <= vmin.x) || (min.x >= vmax.x)) ||
	((max.y <= vmin.y) || (min.y >= vmax.y)) ||
	((max.z <= vmin.z) || (min.z >= vmax.z)))
		return false;	// No intersection at all.

	if (((min.x < vmin.x) && (max.x > vmax.x)) &&
	((min.y < vmin.y) && (max.y > vmax.y)) &&
	((min.z < vmin.z) && (max.z > vmax.z)))
		return false;	// The voxel is inside the box.
	else
		return true;
}


istream& operator >> (istream& s, Box& p)
{
	s >> p.surface >> p.min >> p.max;
	return s;
}

ostream& operator << (ostream& s, Box& p)
{
	s << p.surface << p.min << p.max;
	return s;
}



Polygon::Polygon(void)
{
	vertexes = 0;
}


void Polygon::init(Surface isurface, int ivertexes)
{
	surface = isurface;
	vertexes = ivertexes;
}


Boolean Polygon::icheck(Ray& aray, Interdata& id)
{
	// This algorithm from Eric Haines

	FP vd, uu, vv, up[256], vp[256];
	int x, nc, sh, nsh, a, b;

	vd = normal * aray.direction;
	if (fabs(vd) < SIGMA)
		return false;	// Ray hits edge-on
	id.t = -((aray.origin * normal) + d) / vd;
	if (id.t < SIGMA)
		return false;	// Intersects behind ray or at its origin.
	id.poi = aray.getPoi(id.t);

	// Ok, we've determined the ray intersects with the plane containing
	// the polygon.  Now, we'll see if it is in the polygon.

	if (maxx == true)
	{
		uu = id.poi.y;
		vv = id.poi.z;
	}
	else if (maxy == true)
	{
		uu = id.poi.x;
		vv = id.poi.z;
	}
	else if (maxz == true)
	{
		uu = id.poi.x;
		vv = id.poi.y;
	}

	// Next, translate the polygon so origin = POI:

	for (x = 0; x < vertexes; x++)
	{
		up[x] = u[x] - uu;
		vp[x] = v[x] - vv;
	}

	// Next, set up the counters:

	nc = 0;
	if (vp[0] >= 0.0)
		sh = 1;
	else
		sh = -1;

	for (a = 0; a < vertexes; a++)
	{
		b = (a + 1) % vertexes;
		if (vp[b] < 0.0)
			nsh = -1;
		else
			nsh = 1;

		if (sh != nsh)
		{
			if ((up[a] >= 0.0) && (up[b] >= 0.0))
				nc++;	// The line crosses +up
			else if ((up[a] >= 0.0) || (up[b] >= 0.0))
			{
				if (up[a] - vp[a] * (up[b] - up[a])/(vp[b] - vp[a]) > 0.0)
					nc++;	// The line crosses +up
			}
			sh = nsh;
		}
	}

	return (Boolean)(nc % 2);	// If nc is odd, it's inside.
}

/*
Boolean Polygon::icheck(Ray& aray, Interdata& id)
{
	// This algorithm from Didier Badouel in Graphics Gems I.

	FP vd, u0, u1, beta, alpha;
	Boolean inter;

	vd = normal * aray.direction;
	if (fabs(vd) < SIGMA)
		return false;	// Ray hits edge-on
	id.t = -((aray.origin * normal) + d) / vd;
	if (id.t < SIGMA)
		return false;	// Intersects behind ray or at its origin.
	id.poi = aray.getPoi(id.t);

	if (maxx == true)
	{
		u0 = id.poi.y - vertex[0].y;
		v0 = id.poi.z - vertex[0].z;
	}
	else if (maxy == true)
	{
		u0 = id.poi.x - vertex[0].x;
		v0 = id.poi.z - vertex[0].z;
	}
	else if (maxz == true)
	{
		u0 = id.poi.x - vertex[0].x;
		v0 = id.poi.y - vertex[0].y;
	}

	inter = false;
	i = 2;

	do
	{
		u1 = vertex[i-1].x - vertex[0].x;
		v1 = vertex[i-1].y - vertex[0].y;
		u2 = vertex[i].x - vertex[0].x;
		v2 = vertex[i].y - vertex[0].y;

		if (fabs(u1) < SIGMA)
		{
			beta = u0 / u2;
			if ((beta >= SIGMA) && (beta <= 1.0))
			{
				alpha = (v0 - beta * v2) / v1;
				inter = ((alpha >= SIGMA) && (alpha + beta) <= 1.0));
			}
		}
		else
		{
			beta = (v0 * u1 - u0 * v1) / (v2 * u1 - u2 * v1);
			if ((beta >= SIGMA) && (beta <= 1.0))
			{
				alpha = (u0 - beta * u2) / u1;
				inter = ((alpha >= SIGMA) && ((alpha + beta) <= 1.0));
			}
		}
	} while ((!inter) && (++i < vertexes));

	if (inter)
	{
		gamma = 1 - (alpha + beta);
		normal.init(???);
	}

}
*/


void Polygon::intersect(Ray& aray, Node *nodeptr, Interdata& id)
{
	nodeptr->init(id.poi, normal, surface, Ray(id.poi, aray.direction),
	Ray(id.poi, reflect(aray.direction, normal)), 0, 0, false, false, false);
}

/*
Boolean Polygon::voxelicheck(Point& vmin, Point& vmax)
{
	// Returns true if the polygon intersects with, or is inside the
	// box specified by vmin, vmax.

	int x;
	Boolean iflag;
	Ray edge;
	Box voxel;
	Interdata id;
	FP edgelength;

	voxel.min = vmin;
	voxel.max = vmax;

	x = 0;
	do
	{
		edgelength = edge.init(vertex[x], vertex[x + 1] - vertex[x]);
		iflag = voxel.icheck(edge, id);
		if (id.t > edgelength)
			iflag = false;
		x++;
	}	while ((iflag == false) && (x < vertexes));

	if (iflag == false)
	{
		if ((vertex[0] > vmin) && (vertex[0] < vmax))
			return true;
		else
			return false;
	}
	else
		return true;
}
*/

// The following routine adapted from Radiance.


Boolean Polygon::voxelicheck(Point& vmin, Point& vmax)
{
	// Returns true if the polygon intersects with, or is inside the
	// box specified by vmin, vmax.

	Box voxel;
	Point v1, v2;
	int x, loc, vloc, i, j;
	Ray edge;
	FP edgelength, d1, d2;
	Boolean iflag;
	Interdata id;

	FP uu, vv, up[256], vp[256];
	int nc, sh, nsh, a, b;

	vloc = 0x3f;	// 42 OR 21 = 63

	// First, is there a vertex inside the voxel?
	for (i = 0; i < vertexes; i++)
	{
		loc = 0;
		if (vertex[i].x < vmin.x - SIGMA)
			loc |= 03 & 025;
		else if (vertex[i].x > vmax.x + SIGMA)
			loc |= 03 & 052;
		if (vertex[i].y < vmin.y - SIGMA)
			loc |= 014 & 025;
		else if (vertex[i].y > vmax.y + SIGMA)
			loc |= 014 & 052;
		if (vertex[i].z < vmin.z - SIGMA)
			loc |= 060 & 025;
		else if (vertex[i].z > vmax.z + SIGMA)
			loc |= 060 & 052;

		j = loc;
		if (j != 0)
			vloc &= j;
		else
			return true;	// There's a vertex inside the voxel.
	}

	if (vloc)	// All to one side...
		return false;

	voxel.min = vmin;
	voxel.max = vmax;

	// Next, see if an edge intersects the voxel.
	x = 0;
	do
	{
		edgelength = edge.init(vertex[x], vertex[x + 1] - vertex[x]);
		iflag = voxel.icheck(edge, id);
		if (id.t > edgelength)
			iflag = false;
		if (iflag == true)
			return true;
		x++;
	}	while (x < vertexes);

	// Next, see if the voxel cuts the plane.

	return true;

/*
	if (normal.dx > 0.0)
	{
		v1.x = vmin.x;
		v2.x = vmax.x;
	}
	else
	{
		v1.x = vmax.x;
		v2.x = vmin.x;
	}

	if (normal.dy > 0.0)
	{
		v1.y = vmin.y;
		v2.y = vmax.y;
	}
	else
	{
		v1.y = vmax.y;
		v2.y = vmin.y;
	}

	if (normal.dz > 0.0)
	{
		v1.z = vmin.z;
		v2.z = vmax.z;
	}
	else
	{
		v1.z = vmax.z;
		v2.z = vmin.z;
	}

	d1 = (v1 * normal) - d;
	d2 = (v2 * normal) - d;

//	if ((d1 > 1e-6) || (d2 < -1e-6))
//		return false;

	// Intersect face:

	v1.x = (v1.x * d2 - v2.x * d1) / (d2 - d1);
	v1.y = (v1.y * d2 - v2.y * d1) / (d2 - d1);
	v1.z = (v1.z * d2 - v2.z * d1) / (d2 - d1);

	// If point v1 is inside the polygon, return true.

	if (maxx == true)
	{
		uu = v1.y;
		vv = v1.z;
	}
	else if (maxy == true)
	{
		uu = v1.x;
		vv = v1.z;
	}
	else if (maxz == true)
	{
		uu = v1.x;
		vv = v1.y;
	}

	// Next, translate the polygon so origin = v1:

	for (x = 0; x < vertexes; x++)
	{
		up[x] = u[x] - uu;
		vp[x] = v[x] - vv;
	}

	// Next, set up the counters:

	nc = 0;
	if (vp[0] >= 0.0)
		sh = 1;
	else
		sh = -1;

	for (a = 0; a < vertexes; a++)
	{
		b = (a + 1) % vertexes;
		if (vp[b] < 0.0)
			nsh = -1;
		else
			nsh = 1;

		if (sh != nsh)
		{
			if ((up[a] >= 0.0) && (up[b] >= 0.0))
				nc++;	// The line crosses +up
			else if ((up[a] >= 0.0) || (up[b] >= 0.0))
			{
				if (up[a] - vp[a] * (up[b] - up[a])/(vp[b] - vp[a]) > 0.0)
					nc++;	// The line crosses +up
			}
			sh = nsh;
		}
	}

	return (Boolean)(nc % 2);	// If nc is odd, it's inside.
*/
}


istream& operator >> (istream& s, Polygon& p)
{
	int x;
	Vector a, b;

	// Load in the surface and the number of vertexes

	s >> p.surface >> p.vertexes;

	// Next, load in the polygon:

	p.vertex = new Point[p.vertexes + 1];
	s >> p.vertex[0];
	p.min = p.vertex[0];
	p.max = p.vertex[0];

	for (x = 1; x < p.vertexes; x++)
	{
		s >> p.vertex[x];
		if (p.vertex[x].x < p.min.x)
			p.min.x = p.vertex[x].x;
		if (p.vertex[x].y < p.min.y)
			p.min.y = p.vertex[x].y;
		if (p.vertex[x].z < p.min.z)
			p.min.z = p.vertex[x].z;
		if (p.vertex[x].x > p.max.x)
			p.max.x = p.vertex[x].x;
		if (p.vertex[x].y > p.max.y)
			p.max.y = p.vertex[x].y;
		if (p.vertex[x].z > p.max.z)
			p.max.z = p.vertex[x].z;
	}

	// To make the polygon-box algorithm cleaner:
	p.vertex[p.vertexes] = p.vertex[0];

	if (used_by_scenebuilder == false)
	{

		// Next, compute the plane normal and distance from the first three vertexes:

		a = p.vertex[1] - p.vertex[0];
		b = p.vertex[2] - p.vertex[0];
		vecnormcross(a, b, p.normal);

		a.init(-p.vertex[0].x, -p.vertex[0].y, -p.vertex[0].z);
		p.d = a * p.normal;

		// Allocate space for the two arrays:

		p.u = new FP[p.vertexes];
		p.v = new FP[p.vertexes];

		// Now, throw away the coordinate whose plane-equation magn. is greatest.

		if ((fabs(p.normal.dx) > fabs(p.normal.dy)) && (fabs(p.normal.dx) > fabs(p.normal.dz)))
		{
			// Then x is greatest

			p.maxx = true;
			p.maxy = false;
			p.maxz = false;
			for (x = 0; x < p.vertexes; x++)
			{
				p.u[x] = p.vertex[x].y;
				p.v[x] = p.vertex[x].z;
			}
		}
		else
		if (fabs(p.normal.dy) > fabs(p.normal.dz))
		{
			// Then y is greatest

			p.maxx = false;
			p.maxy = true;
			p.maxz = false;
			for (x = 0; x < p.vertexes; x++)
			{
				p.u[x] = p.vertex[x].x;
				p.v[x] = p.vertex[x].z;
			}
		}
		else
		{
			// Ok, then z is greatest!!!

			p.maxx = false;
			p.maxy = false;
			p.maxz = true;
			for (x = 0; x < p.vertexes; x++)
			{
				p.u[x] = p.vertex[x].x;
				p.v[x] = p.vertex[x].y;
			}
		}
	}
	return s;
}


ostream& operator << (ostream& s, Polygon& p)
{
	int x;
	Point point;

	s << p.surface << p.vertexes << "\n";
	if (used_by_scenebuilder == true)
	{
		for (x = 0; x < p.vertexes; x++)
			s << p.vertex[x];
	}
	else
	{
		for (x = 0; x < p.vertexes; x++)
			s << point;
	}

	return s;
}


Ring::Ring(void)
{
}


void Ring::init(Surface isurface, Vector inormal, Point icenter, FP iinnerr, FP iouterr)
{
	Vector x;

	normal = inormal;
	surface = isurface;
	center = icenter;
	innerr = iinnerr;
	outerr = iouterr;

	// Next, compute the plane's normal and distance:

	x.init(-center.x, -center.y, -center.z);
	d = x * normal;		// Compute the plane's distance
}


Boolean Ring::icheck(Ray& aray, Interdata& id)
{
/*
	This function determines if the ray "aray" intersects the plane
	and is within the ring specified by inerr and outerr.  If not,
	it returns false.  If intersection occurs, it computes the
	distance to the point of intersection (id.t) and the point of
	intersection (id.poi), and stores it in the class Interdata.
*/

	FP vd, poid;

	vd = normal * aray.direction;
	if (fabs(vd) < SIGMA)
		return false;	// It's parallel or hits the plane edge-on.

	if (vd < 0)
		id.t = -(aray.origin * normal + d) / vd;
	else
		id.t = -(aray.origin * normal.neg() + d) / vd;

	if (id.t < 0.001)
		return false;	// The plane's in back of the ray

	id.poi = aray.getPoi(id.t);

	// Now we've established the ray intersects the plane at some point, and
	// have computed the point of intersection.  Now determine if the poi is
	// within the specified ring.

	poid = sqrt(sqr(id.poi.x) + sqr(id.poi.y) + sqr(id.poi.z));
	if ((poid >= innerr) && (poid <= outerr))
		return true;
	else
		return false;
}


void Ring::intersect(Ray& aray, Node *nodeptr, Interdata& id)
{
/*
	This method computes the reflected and transmitted vectors using the
	incident ray, the surface normal, and the surface characteristics.

	Since a plane has no thickness, the incident ray will be transmitted
	with no distortion or refraction (unless ktran = 0.0):

	Remember, change the light coming from the transmitted ray by the color of
	the plane.  Also change the di factor by the color of the plane.  I'm not
	sure what to do about the light from the reflected ray.  Perhaps change
	that by the plane color also.  Or should both an absorption and reflection
	spectrum be stored for each surface/material?  And can that spectrum be
	stored in a Color?  Also, generate and record weights for the transmitted
	ray, the reflected ray, and the di factor.
*/

	Surface tsurface = surface;
//	FP uu, vv;

/*	// No inverse mapping yet.
	if (surface.texture != 0)
	{
		// Planar inverse mapping: uu & vv range from 0 to 1.

		uu = ((id.poi * nc) - du0) / (du1 - (id.poi * na));
		vv = ((id.poi * nb) - dv0) / (dv1 - (id.poi * na));
		tsurface.color.init(textptr[surface.texture]->getcolor(uu, vv));
	}
*/

	nodeptr->init(id.poi, normal, tsurface, Ray(id.poi, aray.direction),
	Ray(id.poi, reflect(aray.direction, normal)), 0, 0, false, false, false);
}


Boolean Ring::voxelicheck(Point& vmin, Point& vmax)
{
	return false;
}


istream& operator >> (istream& s, Ring& p)
{
	s >> p.surface >> p.normal >> p.center >> p.innerr >> p.outerr;
	return s;
}

ostream& operator << (ostream& s, Ring& p)
{
	s << p.surface << p.normal << p.center << p.innerr << "\n" << p.outerr << "\n";
	return s;
}

