// lights.h		Light object declarations.

#ifndef lights_h
#define lights_h

class Light		// An abstract class
{
	public:

	Point location;
	Color color;

	Light(void);
	
	// This pure virtual function is a placeholder for Light's children...
	
	virtual Color getillumination(Vector normal, Vector lightvector) = 0;
};


class Plight : public Light
{
	public:

	Plight(void);
	void init(Point ilocation, Color icolor);
	Color getillumination(Vector normal, Vector lightvector);
	friend void loadScene(void);
	friend Color illumination(Point& poi, Vector& normal);
	friend istream& operator >> (istream& s, Plight& l);
	friend ostream& operator << (ostream& s, Plight& l);
};

istream& operator >> (istream& s, Plight& l);
ostream& operator << (ostream& s, Plight& l);

class Dlight : public Light
{
	public:

	Vector direction;
	FP fov;

	Dlight(void);
	void init(Point ilocation, Vector idirection, FP ifov, Color icolor);
	Color getillumination(Vector normal, Vector lightvector);
	friend void loadScene(void);
	friend Color illumination(Point& poi, Vector& normal);
	friend istream& operator >> (istream& s, Dlight& l);
	friend ostream& operator << (ostream& s, Dlight& l);
};

istream& operator >> (istream& s, Dlight& l);
ostream& operator << (ostream& s, Dlight& l);

#endif	// Of lights_h
