//	Raytrace.h  Miscellaneous stuff for raytrace

#ifndef _raytrace_h
#define _raytrace_h

#include <fstream.h>		// Standard C++ stream I/O functions
#include <math.h>			// (ANSI)
#include <stdio.h>		// (ANSI)
#include <stdlib.h>		// (ANSI)

#include "rstrfile.h"	// Sun Microsystems rasterfile definitions

#define FP double			// FP is the general floating-point type.
#define sigma 0.000001	// To compensate for precision errors.
#define sqr(a) ((a)*(a))
#define DTOR 0.0174532925199		// Convert degrees to radians
#define SIGMA 0.0000000000000001	// Used to prevent divide by zero
//#define PI 3.14159265358979323846264383
#define PIO2 1.570796327			// Pi divided by 2
#define max(a,b)	(((a)>(b))?(a):(b))
#define min(a,b)	(((a)<(b))?(a):(b))

#ifndef __GNUC__
// This statement causes g++ (and gcc) grief:
enum Boolean { false, true };		// False = 0; true = 1
#endif
#ifdef __GNUC__
typedef int Boolean;
#endif

#ifdef SUNOS	// Sun defines different codes in sys/ieeefp.h.
#define FPE_INEXACT	0
#define FPE_ZERODIVIDE	1
#define FPE_UNDERFLOW	2
#define FPE_OVERFLOW	3
#define FPE_INVALID	4
#define FPE_INTOVFLOW	127
#define FPE_INTDIV0	126
#define FPE_STACKFAULT	125
#define FPE_EXPLICITGEN 124
// 124 - 127 are just made-up - there are no equivalents in unix-land.
#endif	// SUNOS


#endif	// Of _raytrace_h
