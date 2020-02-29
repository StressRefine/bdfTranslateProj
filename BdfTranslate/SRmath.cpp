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
// SRmath.cpp: implementation of the SRmath class.
//
//////////////////////////////////////////////////////////////////////

#include "SRmodel.h"

extern SRmodel model;

double SRmath::GetSvm(double stress[])
{
	//return svm of a stress tensor
	//input:
	//stress = stress tensor stored as vector
	//return:
	//svm = von Mise Stress


	double p, s11, s12, s13, s22, s23, s33, svm;
	p = ONETHIRD*(stress[0] + stress[1] + stress[2]);
	s11 = stress[0] - p;
	s22 = stress[1] - p;
	s33 = stress[2] - p;
	s12 = stress[3];
	s13 = stress[4];
	s23 = stress[5];
	svm = s11*s11 + s22*s22 + s33*s33 + 2.0*(s12*s12 + s13*s13 + s23*s23);
	svm = sqrt(1.5*svm);
	return svm;
}

double SRvec3::Dot(double* x, double* y)
{
	//dot product between 3d vectors x and y
	//input:
		//x, y = vectors
	//return:
		//dot product

	double dot = 0.0;
	for (int i = 0; i < 3; i++)
		dot += x[i] * y[i];
	return dot;
}

double SRvec3::TwoNorm(double* v)
{
	//calculate the two-norm of a 3d vector
	//input:
		//v = vector
	//return:
		//two-norm of v

	double vn = 0.0;
	for(int i = 0; i < 3; i++)
		vn += v[i] * v[i];
	vn = sqrt(vn);
	return vn;
}

void SRvec3::Copy(double v[])
{
	//copy this into a vector of doubles
	for (int dof = 0; dof < 3; dof++)
		v[dof] = d[dof];
}

void SRvec3::Zero(double* v)
{
	//zero a vec3

	for (int i = 0; i < 3; i++)
		v[i] = 0.0;
}

void SRvec3::Cross(double* a, double* b, double* c)
{
	//cross 2 vectors
	//input:
		//a,b = vectors
	//output:
		//c = a cross b

	c[0] = a[1] * b[2] - b[1] * a[2];
	c[1] = a[2] * b[0] - b[2] * a[0];
	c[2] = a[0] * b[1] - b[0] * a[1];
}

double SRvec3::Normalize(double* v)
{
	//normalize a 3d vector
	//input:
		//v = vector
	//return:
		//norm of v

	double norm = TwoNorm(v);
	int i;
	double d = 1.0/norm;
	for(i = 0; i < 3; i++)
		v[i] *= d;
	return norm;
}

void SRvec3::Rotate(SRmat33& R, bool transpose)
{
	//v = this vector, perform v = R*v, overriding contents of this vector
	//input:
		//R = 3x3 rotation matrix
		//transpose = true to use R-transpose instead of R, else false

	SRvec3 v2;
	v2.Copy(*(this));
	Rotate(R,v2,transpose);
	Copy(v2);
}

void SRvec3::Rotate(SRmat33& R, SRvec3& v2, bool transpose)
{
	//v = this vector, perform v2 = R*v
	//input:
		//R = 3x3 rotation matrix
		//transpose = true to use R-transpose instead of R, else false
	//output:
		//v2 = rotated vector stored as SRvec3

	Rotate(R, v2.d, transpose);
}

void SRvec3::Rotate(SRmat33& R, double v2[], bool transpose)
{
	//v = this vector, perform v2 = R*v
	//input:
		//R = 3x3 rotation matrix
		//transpose = true to use R-transpose instead of R, else false
	//output:
		//v2 = rotated vector 

	int i, j;
	for (i = 0; i < 3; i++)
	{
		v2[i] = 0.0;
		if (transpose)
		{
			for (j = 0; j < 3; j++)
				v2[i] += (R.rows[j].col[i] * d[j]);
		}
		else
		{
			for (j = 0; j < 3; j++)
				v2[i] += (R.rows[i].col[j] * d[j]);
		}
	}
}

void SRvec3::Subtract(SRvec3& v2, SRvec3& v3)
{
	//v = this, perform v3 = v - v2
	//input:
		//v2 = vector to subtract
	//output:
		//v3 = vector resulting from subtraction

	v3.d[0] = d[0] - v2.d[0];
	v3.d[1] = d[1] - v2.d[1];
	v3.d[2] = d[2] - v2.d[2];
}

void SRvec3::Add(SRvec3& v2, SRvec3 &v3)
{
	//v = this, perform v3 = v + v2
	//input:
		//v2 = vector to add
	//output:
		//v3 = vector resulting from addition

	v3.d[0] = d[0] + v2.d[0];
	v3.d[1] = d[1] + v2.d[1];
	v3.d[2] = d[2] + v2.d[2];
}

double SRvec3::Distance(SRvec3& v2)
{
	//calculate distance from this vector to v2

	double dx, dy, dz;
	dx = v2.d[0] - d[0];
	dy = v2.d[1] - d[1];
	dz = v2.d[2] - d[2];
	return sqrt(dx*dx + dy*dy + dz*dz);
}

bool SRvec3::Equals(SRvec3& v2)
{
	//see if this vector equals v2

	SRvec3 dv;
	//dv = "this" - v2:
	Subtract(v2, dv);
	if (dv.Length() < TINY)
		return true;
	else
		return false;
}

void SRmat33::Assign(double m11, double m12, double m13, double m21, double m22, double m23, double m31, double m32, double m33)
{
	//assign a mat33 using components m11...
	rows[0].col[0] = m11;
	rows[0].col[1] = m12;
	rows[0].col[2] = m13;
	rows[1].col[0] = m21;
	rows[1].col[1] = m22;
	rows[1].col[2] = m23;
	rows[2].col[0] = m31;
	rows[2].col[1] = m32;
	rows[2].col[2] = m33;
}

void SRmat33::setIdentity()
{
	Zero();
	rows[0].col[0] = 1.0;
	rows[1].col[1] = 1.0;
	rows[2].col[2] = 1.0;
}


void SRmat33::Mult(SRmat33& b, SRmat33& c, bool transpose)
{
	// a ="this" matrix
	//if(transpose) then c = a*b-transpose else c = a times b

	int i, j, k;
	double ct;
	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
			ct = 0.0;
			if (transpose)
			{
				for (k = 0; k < 3; k++)
					ct += rows[i].col[k] * b.rows[j].col[k];
			}
			else
			{
				for (k = 0; k < 3; k++)
					ct += rows[i].col[k] * b.rows[k].col[j];
			}
			c.rows[i].col[j] = ct;
		}
	}
}

void SRmat33::Mult(SRvec3& vin, SRvec3& vout)
{
	//matrix-vector multiply
	// a ="this" matrix
	//input:
		//vin = vector to multiply by a
	//output:
		//vout = a*vin

	vout.Zero();
	int i;
	for (i = 0; i < 3; i++)
		vout.d[i] = vin.Dot(rows[i].col);
}

void SRmat33::Subtract(SRmat33& m2, SRmat33& m3)
{
	//m = this, perform m3 = m - m2
	//input:
		//m2 = matrix to subtract
	//output:
		//m3 = matrix resulting from subtraction

	int i, j;
	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
			m3.rows[i].col[j] = rows[i].col[j] - m2.rows[i].col[j];
	}
}

void SRmat33::Scale(double s)
{
	//scale this matrix by scalar s

	int i, j;
	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
			rows[i].col[j] *= s;
	}
}

void SRmat33::Copy(SRvec3& e1, SRvec3& e2, SRvec3& e3, bool columns)
{
	//copy vectors e1,e2,e3 into this matrix.
	//input:
		//e1,e2,e3 = 3-vectors
		//columns = true if e1,e2,e3 should be columns of matrix else false

	int i;
	if(columns)
	{
		for (i = 0; i < 3; i++)
		{
			rows[i].col[0] = e1.d[i];
			rows[i].col[1] = e2.d[i];
			rows[i].col[2] = e3.d[i];
		}
	}
	else
	{
		rows[0].Copy(e1.d);
		rows[1].Copy(e2.d);
		rows[2].Copy(e3.d);
	}
}

void SRmat33::vecCopy(int rc, SRvec3&v, bool copyRow)
{
	if (copyRow)
	{
		for (int i = 0; i < 3; i++)
			v.d[i] = rows[rc].col[i];
	}
	else
	{
		for (int i = 0; i < 3; i++)
			v.d[i] = rows[i].col[rc];
	}
}


SRmath::SRmath()
{
	identityMatrix.setIdentity();
}

double SRmath::Distance(SRvec3& p1, SRvec3& p2)
{
	//distance between two points

	return p1.Distance(p2);
}


double SRmat33::Determinant()
{
	//Determinant of this 3x3 matrix

	return rows[0].col[0] * (rows[1].col[1] * rows[2].col[2] - rows[2].col[1] * rows[1].col[2])
		- rows[1].col[0] * (rows[0].col[1] * rows[2].col[2] - rows[2].col[1] * rows[0].col[2])
		+ rows[2].col[0] * (rows[0].col[1] * rows[1].col[2] - rows[1].col[1] * rows[0].col[2]);
}

bool SRmat33::Solve(double b[], double x[])
{
	//solve Ax = b for x where A is this mat33, using Cramer's rule

	double d, dx;
	double colsave[3];
	int i, j;
	d = Determinant();
	if (fabs(d) < TINY)
		return false;
	d = 1.0 / d;
	for (j = 0; j < 3; j++)
	{
		for (i = 0; i < 3; i++)
		{
			colsave[i] = rows[i].col[j];
			rows[i].col[j] = b[i];
		}
		dx = Determinant();
		x[j] = dx*d;
		for (i = 0; i < 3; i++)
			rows[i].col[j] = colsave[i];
	}
	return true;
	//tuning: could save some ops (< 1/3) by sharing co-factor mults between determinant calculation
	//and solution for x[0]. but code is then much less readable (see old stress refine source)
	//only do it this routine scores high in profile
}

int SRmath::Round(double d)
{
	//round off a double to an int

	double r;
	int i= (int) d;
	r = d-i;
	if(r < 0.5)
		return i;
	else
		return i + 1;
}



void SRmat33::Transpose(SRmat33& mout)
{
	//transpose this matrix
	//output:
		//mout = transpose
	int i, j;
	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
			mout.rows[i].col[j] = rows[j].col[i];
	}
}

double SRmat33::Invert()
{
	//invert this matrix
	//note:
		//this matrix is overwritten with its inverse

	double det, inv11, inv12, inv13, inv21, inv22, inv23, inv31, inv32, inv33;
	inv11 = rows[1].col[1] * rows[2].col[2] - rows[2].col[1] * rows[1].col[2];//co-factor of 11
	inv21 = rows[2].col[0] * rows[1].col[2] - rows[1].col[0] * rows[2].col[2];//co-factor of 12
	inv31 = rows[1].col[0] * rows[2].col[1] - rows[2].col[0] * rows[1].col[1];//co-factor of 13
	det = rows[0].col[0] * inv11 + rows[0].col[1] * inv21 + rows[0].col[2] * inv31;
	if(fabs(det) < TINY)
		ERROREXIT;
	double detm1 = 1.0 / det;
	inv12 = rows[2].col[1] * rows[0].col[2] - rows[0].col[1] * rows[2].col[2];//co-factor of 21
	inv13 = rows[0].col[1] * rows[1].col[2] - rows[1].col[1] * rows[0].col[2];//co-factor of 31
	inv22 = rows[0].col[0] * rows[2].col[2] - rows[2].col[0] * rows[0].col[2];//co-factor of 22
	inv23 = rows[1].col[0] * rows[0].col[2] - rows[0].col[0] * rows[1].col[2];//co-factor of 32
	inv32 = rows[2].col[0] * rows[0].col[1] - rows[0].col[0] * rows[2].col[1];//co-factor of 23
	inv33 = rows[0].col[0] * rows[1].col[1] - rows[1].col[0] * rows[0].col[1];//co-factor of 33
	rows[0].col[0] = inv11*detm1;
	rows[1].col[0] = inv21*detm1;
	rows[2].col[0] = inv31*detm1;
	rows[0].col[1] = inv12*detm1;
	rows[1].col[1] = inv22*detm1;
	rows[2].col[1] = inv32*detm1;
	rows[0].col[2] = inv13*detm1;
	rows[1].col[2] = inv23*detm1;
	rows[2].col[2] = inv33*detm1;
	return det;
}

void SRmath::VectorCopy(int n, double a[], double b[])
{
	//copy double vector a to vector b
	//input:
		//a  = vector
		//n = length
	//output:
		//b = copy of a

	for(int i = 0; i < n; i++)
		b[i] = a[i];
}

double SRmath::VectorDot(int n, double a[], double b[])
{
	//dot product of double vector a and vector b
	//input:
		//a,b  = vectors
		//n = length
	//return:
		//a dot b

	double d = 0.0;
	for(int i = 0; i < n; i++)
		d += (b[i] * a[i]);
	return d;
}

double SRmath::Vector2Norm(int n, double a[])
{
	//two-norm of double vector a
	//input:
		//a  = vector
		//n = length
	//return:
		//two-norm of a

	double d = 0.0;
	for(int i = 0; i < n; i++)
		d += (a[i] * a[i]);
	d = sqrt(d);
	return d;
}


