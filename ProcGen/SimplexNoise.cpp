// SimplexNoise.cpp
// Contains functions to execute Simplex Noise

#include "SimplexNoise.h"

SimplexNoise::SimplexNoise()
{
	for (int i = 0; i < 512; i++)
	{
		perm[i] = p[i & 255];
	}
}

SimplexNoise::~SimplexNoise()
{
}

double SimplexNoise::noise(double xin, double yin)
{
	//int X, Y, Z;
	//X = (int)fastfloor(x) & 255;	// FIND UNIT CUBE THAT
	//Y = (int)fastfloor(y) & 255;	// CONTAINS POINT.
	//Z = (int)fastfloor(z) & 255;

	//x -= fastfloor(x);				// FIND RELATIVE X,Y,Z
	//y -= fastfloor(y);				// OF POINT IN CUBE.
	//z -= fastfloor(z);

	//double u, v, w;
	//u = fade(x);				// COMPUTE FADE CURVES
	//v = fade(y);				// FOR EACH OF X,Y,Z.
	//w = fade(z);

	//int A, AA, AB, B, BA, BB;
	//A = perm[X] + Y;				// HASH COORDINATES OF
	//AA = perm[A] + Z;				// THE 8 CUBE CORNERS,
	//AB = perm[A + 1] + Z;
	//B = perm[X + 1] + Y;
	//BA = perm[B] + Z;
	//BB = perm[B + 1] + Z;

	//return lerp(w, lerp(v, lerp(u, grad(perm[AA], x, y, z),	// AND ADD
	//	grad(perm[BA], x - 1, y, z)),							// BLENDED
	//	lerp(u, grad(perm[AB], x, y - 1, z),					// RESULTS
	//		grad(perm[BB], x - 1, y - 1, z))),					// FROM 8
	//	lerp(v, lerp(u, grad(perm[AA + 1], x, y, z - 1),		// CORNERS
	//		grad(perm[BA + 1], x - 1, y, z - 1)),				// OF CUBE
	//		lerp(u, grad(perm[AB + 1], x, y - 1, z - 1),
	//			grad(perm[BB + 1], x - 1, y - 1, z - 1))));

	double n0, n1, n2; // Noise contributions from the three corners
	// Skew the input space to determine which simplex cell we're in
	double F2 = 0.5*(sqrt(3.0) - 1.0);
	double s = (xin + yin)*F2; // Hairy factor for 2D
	int i = fastfloor(xin + s);
	int j = fastfloor(yin + s);

	double G2 = (3.0 - sqrt(3.0)) / 6.0;
	double t = (i + j)*G2;
	double X0 = i - t; // Unskew the cell origin back to (x,y) space
	double Y0 = j - t;
	double x0 = xin - X0; // The x,y distances from the cell origin
	double y0 = yin - Y0;

	// For the 2D case, the simplex shape is an equilateral triangle.
	// Determine which simplex we are in.
	int i1, j1; // Offsets for second (middle) corner of simplex in (i,j) coords
	if (x0 > y0) { i1 = 1; j1 = 0; } // lower triangle, XY order: (0,0)->(1,0)->(1,1)
	else { i1 = 0; j1 = 1; } // upper triangle, YX order: (0,0)->(0,1)->(1,1)

	// A step of (1,0) in (i,j) means a step of (1-c,-c) in (x,y), and
	// a step of (0,1) in (i,j) means a step of (-c,1-c) in (x,y), where
	// c = (3-sqrt(3))/6

	double x1 = x0 - i1 + G2; // Offsets for middle corner in (x,y) unskewed coords
	double y1 = y0 - j1 + G2;
	double x2 = x0 - 1.0 + 2.0 * G2; // Offsets for last corner in (x,y) unskewed coords
	double y2 = y0 - 1.0 + 2.0 * G2;

	// Work out the hashed gradient indices of the three simplex corners
	int ii = i & 255;
	int jj = j & 255;
	int gi0 = perm[ii + perm[jj]] % 12;
	int gi1 = perm[ii + i1 + perm[jj + j1]] % 12;
	int gi2 = perm[ii + 1 + perm[jj + 1]] % 12;

	// Calculate the contribution from the three corners
	double t0 = 0.5 - x0 * x0 - y0 * y0;
	if (t0 < 0) n0 = 0.0;
	else {
		t0 *= t0;
		n0 = t0 * t0 * dot(grad3[gi0], x0, y0); // (x,y) of grad3 used for 2D gradient
	}
	double t1 = 0.5 - x1 * x1 - y1 * y1;
	if (t1 < 0) n1 = 0.0;
	else {
		t1 *= t1;
		n1 = t1 * t1 * dot(grad3[gi1], x1, y1);
	}
	double t2 = 0.5 - x2 * x2 - y2 * y2;
	if (t2 < 0) n2 = 0.0;
	else {
		t2 *= t2;
		n2 = t2 * t2 * dot(grad3[gi2], x2, y2);
	}
	// Add contributions from each corner to get the final noise value.
	// The result is scaled to return values in the interval [-1,1].
	return 70.0 * (n0 + n1 + n2);
}

int SimplexNoise::fastfloor(double x)
{
	return (x > 0 ? (int)x : (int)x - 1);
}

double SimplexNoise::fade(double t)
{
	return (t * t * t * (t * (t * 6 - 15) + 10));
}

double SimplexNoise::lerp(double t, double a, double b)
{
	return (a + t * (b - a));
}

double SimplexNoise::grad(int hash, double x, double y, double z)
{
	int h = hash & 15;
	double u = h < 8 ? x : y;
	double v = h < 4 ? y : h == 12 || h == 14 ? x : z;

	return (((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v));
}