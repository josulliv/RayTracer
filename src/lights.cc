// lights.cc	Light objects and related code

#include "raytrace.h"
#include "vector.h"
#include "miscobj.h"
#include "lights.h"

Light::Light(void)
{
}


// Point light source	************************************************

Plight::Plight(void)
{
}

void Plight::init(Point ilocation, Color icolor)
{
	location = ilocation;
	color = icolor;
}

Color Plight::getillumination(Vector normal, Vector lightvector)
{
	return color * (FP)(normal * lightvector);
}

istream& operator >> (istream& s, Plight& l)
{
	s >> l.location >> l.color;
	return s;
}

ostream& operator << (ostream& s, Plight& l)
{
	s << l.location << l.color;
	return s;
}


// Directional light source	************************************************

Dlight::Dlight(void)
{
}

void Dlight::init(Point ilocation, Vector idirection, FP ifov, Color icolor)
{
	location = ilocation;
	direction = idirection;
	fov = ifov;
	color = icolor;
}

Color Dlight::getillumination(Vector normal, Vector lightvector)
{
	FP theta;
	Color c;
	Vector vector_neg = lightvector.neg();

	theta = getangle(direction, vector_neg);	// Compute the angle.
	if (theta > fov)	// If the POI is outside the light cone
		return c;		// Return black (no color)
	else				// The POI is illuminated, but how much?
		return color * cos(theta * PIO2 / fov) * (FP)(normal * lightvector);
}

istream& operator >> (istream& s, Dlight& l)
{
	s >> l.location >> l.direction >> l.fov >> l.color;
	return s;
}

ostream& operator << (ostream& s, Dlight& l)
{
	s << l.location << l.direction << l.fov << "\n" << l.color;
	return s;
}
