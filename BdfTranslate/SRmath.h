/*
Copyright (c) 2020 Richard King

BdfTranslate is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

BdfTranslate is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

The terms of the GNU General Public License are explained in the file COPYING.txt,
also available at <https://www.gnu.org/licenses/>
*/

//////////////////////////////////////////////////////////////////////
//
// SRmath.h: interface for the SRmath class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(SRMATH_INCLUDED)
#define SRMATH_INCLUDED

#include <math.h>

#define MATHMIN(x,y) ((x<y)?x:y)
#define MATHMAX(x,y) ((x>y)?x:y)
    
//value of p2 basis function at midedge (needed for handling quadratic constraints):
#define FDSTEP 1.e-6
#define BIG 1.e20
#define RELSMALL 1.e-7
#define SMALL 1.e-12
#define TINY 1.e-20
#define SQRT3 1.732050807568877
#define SQRT3OVER2 0.866025403784438
#define SQRT3OVER4 0.433012701892219
#define SQRT3OVER3 0.5773502691896257
#define SQRT3OVER6 0.2886751345948128
#define SQRT2 1.414213562373095
#define SQRT2OVER2 0.707106781186547
#define SQRT6OVER6 0.408248290463862
#define SQRT6OVER12 0.204124145231931
#define SQRT3OVERSQRT8  0.6123724356957945
#define SQRT8OVER8 0.353553390593273
#define PI 3.141592653589793
#define PIOVER2 1.570796326794897
#define ONETHIRD 0.3333333333333333
#define TWOSQRTTWOTHIRDS 1.632993161855452
#define TWOPI 6.283185307



class SRmat33;

class SRvec3
{
public:
	bool Equals(SRvec3& v2);
	void Assign(double v1, double v2, double v3){ d[0] = v1; d[1] = v2; d[2] = v3; };
	void Assign(double v[]){ d[0] = v[0]; d[1] = v[1]; d[2] = v[2]; };
	void PlusAssign(double v[]){ d[0] += v[0]; d[1] += v[1]; d[2] += v[2]; };
	void PlusAssign(SRvec3& v){ PlusAssign(v.d); };
	void MinusAssign(double v[]){ d[0] -= v[0]; d[1] -= v[1]; d[2] -= v[2]; };
	void MinusAssign(SRvec3& v){ MinusAssign(v.d); };
	void PlusAssign(double v[], double scale){ d[0] += scale*v[0]; d[1] += scale*v[1]; d[2] += scale*v[2]; };
	void Copy(SRvec3& v2){ Assign(v2.d); };
	void Copy(double v[]);
	bool operator ==(SRvec3& v2){ return Equals(v2); };
	void operator =(SRvec3& v2){ Copy(v2); };
	void operator = (double v[]){ Assign(v); };
	void operator += (double v[]){ PlusAssign(v); };
	void operator += (SRvec3& v){ PlusAssign(v); };
	double Distance(SRvec3& v2);
	void Add(SRvec3& v2, SRvec3& v3);
	void Subtract(SRvec3& v2, SRvec3& v3);
	void Cross(SRvec3& v2, SRvec3& v3){ Cross(d, v2.d, v3.d); };
	double Normalize(){ return Normalize(d); };
	void Zero(){ d[0] = 0.0; d[1] = 0.0; d[2] = 0.0; };
	double Length(){ return TwoNorm(d); };
	void Rotate(SRmat33& R, bool transpose = false);
	void Rotate(SRmat33& R, SRvec3& v2, bool transpose = false);
	void Rotate(SRmat33& R, double v2[], bool transpose = false);
	static double Normalize(double* v);
	static void Cross(double* a, double* b, double* c);
	static void Zero(double* v);
	static double TwoNorm(double* v);
	static double Dot(double* x, double* y);
	double Dot(SRvec3& v2){ return Dot( d, v2.d); };
	double Dot(double* x){ return Dot( d, x); };
	void Scale(double s)
	{
		for (int i = 0; i < 3; i++)
			d[i] *= s;
	};
	SRvec3(){ Zero(); };
	SRvec3(double v[]){ Assign(v); };
	SRvec3(double v1, double v2, double v3){ Assign(v1, v2, v3); };
	~SRvec3(){};
	double d[3];
};

class SRrow3
{
public:
	void Zero(){for(int i = 0; i < 3; i++) col[i] = 0.0;};
	void operator +=(SRrow3& r2)
	{
		for (int i = 0; i <3; i++)
			col[i] += r2.col[i];
	};
	void Copy(double v[])
	{
		for (int i = 0; i < 3; i++)
			col[i] = v[i];
	};
	void Copy(SRrow3& r2)
	{
		for(int i = 0; i < 3; i++)
			col[i] = r2.col[i];
	};
	void operator =(SRrow3& r2){Copy(r2);};

	SRrow3(){Zero();};
	~SRrow3(){};

	double col[3];
};

class SRmat33
{
public:
	double Invert();
	void Transpose(SRmat33& mout);
	void Assign(double m11, double m12, double m13, double m21, double m22, double m23, double m31, double m32, double m33);
	bool Solve(double b[], double x[]);
	double Determinant();
	void Copy(SRvec3& e1, SRvec3& e2, SRvec3& e3, bool columns = true);
	void vecCopy(int rc, SRvec3&v, bool copyRow);
	void Scale(double s);
	void Subtract(SRmat33& m2, SRmat33& m3);
	void Mult(SRvec3& vin, SRvec3& vout);
	void Mult(SRmat33& b, SRmat33& c, bool transpose = false);
	void setIdentity();
	void Zero()
	{
		for (int i = 0; i < 3; i++)
			rows[i].Zero();
	};
	void operator +=(SRmat33& m2)
	{
		for (int i = 0; i < 3; i++)
			rows[i] += m2.rows[i];
	};
	void Copy(SRmat33& m2)
	{
		for (int i = 0; i < 3; i++)
			rows[i] = m2.rows[i];
	};
	void operator =(SRmat33& m2){ Copy(m2); };

	SRmat33(double m11, double m12, double m13, double m21, double m22, double m23, double m31, double m32, double m33)
	{
		Assign(m11, m12, m13, m21, m22, m23, m31, m32, m33);
	};
	SRmat33(){};
	~SRmat33(){};

	SRrow3 rows[3];
};



class SRelement;
class SRmath  
{
public:
	SRmath();
	double Vector2Norm(int n, double a[]);
	double VectorDot(int n, double a[], double b[]);
	void VectorCopy(int n, double a[], double b[]);
	int Round(double d);
	double Distance(SRvec3& p1, SRvec3& p2);
	int Sign(double x){ return (x>0.0) ? 1 : -1; };
	double Max(double x, double y){ return (x>y) ? x : y; };
	double Min(double x, double y){ return (x<y) ? x : y; };
	inline int Max(int x, int y){ return (x>y) ? x : y; };
	inline int Min(int x, int y){ return (x<y) ? x : y; };
	inline bool Even(int x){ return (((x / 2) * 2) == x) ? true : false; };
	inline bool Odd(int x){ return (((x / 2) * 2) != x) ? true : false; };
	inline bool Equal(double x, double y){ return (fabs(fabs(x) - fabs(y))<TINY) ? true : false; };
	double GetSvm(double stress[]);
	SRmat33& getIdentityMatrix(){ return identityMatrix; };
private:
	SRmat33 identityMatrix;
};


#endif //!defined(SRMATH_INCLUDED)
