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
// SRcoord.cpp: implementation of the SRcoord class.
//
//////////////////////////////////////////////////////////////////////

#include "SRmodel.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

SRcoord::SRcoord()
{
	e1.Zero();
	e1.d[0] = 1.0;
	e2.Zero();
	e2.d[1] = 1.0;
	e3.Zero();
	e3.d[2] = 1.0;
	otherCoordid = -1;//only used where definition of one cs references another
}


void SRcoord::Copy(SRcoord& c2)
{
	//copy contents of coordinate system c2 to this one
	//input:
		//c2 = coordinate system 
	name = c2.name;
	type = c2.type;
	origin.Copy(c2.origin);
	e1 = c2.e1;
	e2 = c2.e2;
	e3 = c2.e3;
}

void SRcoord::CalculateBasisVectors(SRvec3& p, SRvec3 &e1l, SRvec3 &e2l, SRvec3 &e3l)
{
	//calculate basis vectors for an lcs
	//input:
		//p = position
	//output
		//e1l, e2l, e3l = vectors
	if (type == cartesian)
	{
		e1l.Copy(e1);
		e2l.Copy(e2);
		e3l.Copy(e3);
		return;
	}
	SRvec3 erho;
	p.Subtract(origin, erho);
	double d =	erho.Normalize();
	if (d < SMALL)
		ERROREXIT;
	if (type == spherical)
	{
		//local coordinates rho,theta,phi
		//local basis e1l, e2l, e3l = e-rho, e-theta, e-phi
		e1l.Copy(erho);
		erho.Cross(e3, e2l);
		//this now points in -e-theta, see Tuma p 55
		e2l.Scale(-1.0);
		d = e2l.Normalize();
		if (d < SMALL)
		{
			//position is along z axis, e2l and e3l are arbitrary
			e1l.Copy(erho);
			e2l.Copy(e1);
			e2l.Scale(-1.0);
			e2l.Copy(e2);
		}
		else
			e2l.Cross(e1l, e3l); //e-phi
	}
	else if (type == cylindrical)
	{
		//local coordinates rho,theta,z
		//local basis e1l, e2l, e3l = e-rho, e-theta, e-z
		//project erho into xy plane:
		SRvec3 erhoxy;
		erhoxy.Copy(erho);
		d = erho.Dot(e3);
		SRvec3 erhoz;
		erhoz.Copy(e3);
		erhoz.Scale(-d);
		erhoxy.PlusAssign(erhoz);
		d = erhoxy.Normalize();
		if (d < SMALL)
		{
			//position is along z axis, e2l and e3l are arbitrary
			e3l.Copy(erho);
			e1l.Copy(e2);
			e2l.Copy(e3);
		}
		else
		{
			e3l.Copy(e3);
			e1l.Copy(erhoxy);
			e3l.Cross(e1l, e2l);
			e2l.Normalize();
		}
	}
}

void SRcoord::Create(double x0, double y0, double z0)
{
	//Create a local cartesian coordinate system aligned with gcs
	//input:
		// x0, y0, z0 = origin

	origin.Assign(x0, y0, z0);
}

void SRcoord::Create(double x0, double y0, double z0, SRvec3 p1, SRvec3 p3)
{
	//Create a local cartesian coordinate system aligned with gcs
	//input:
		// x0, y0, z0 = origin
		// p1 = point on the local e1 axis
		// p3 = point on the local e3 axis
	origin.Assign(x0, y0, z0);
	p1.Subtract(origin, e1);
	double d = e1.Normalize();
	if (d < SMALL)
		ERROREXIT;
	p3.Subtract(origin, e3);
	d = e1.Normalize();
	if (d < SMALL)
		ERROREXIT;
	e3.Cross(e1, e2);
	if ((1.0 - e2.Length()) > SMALL)
		ERROREXIT; //e1 not normal to e3
}


void SRcoord::Create(double x0, double y0, double z0, double alf, double bet, double gam)
{
	//Create a local coordinate system
	//input:
        // x0, yot, z0t = origin
        //if !gcsAligned: alf, bet, gam rotates from gcs to lcs orientation
		// (only 2 are needed, so e.g. if rotate about Z then Y', gam = 0)

	origin.Assign(x0, y0, z0);
	double ca, sa, cb, sb, cg, sg;
	sa = sin(alf);
	ca = sqrt(1.0 - sa*sa);
	sb = sin(bet);
	cb = sqrt(1.0 - sb*sb);
	sg = sin(gam);
	cg = sqrt(1.0 - sg*sg);
	//see Tuma, p 57
	e1.d[0] = ca*cb;
	e1.d[1] = sa*cb;
	e1.d[2] = -sa;
	e2.d[0] = -sa*cg + ca*sb*sg;
	e2.d[1] = ca*cg + sa*sb*sg;
	e2.d[2] = ca*sg;
	e3.d[0] = sa*sg + ca*sb*cg;
	e3.d[1] = -ca*sg + sa*sb*cg;
	e3.d[2] = ca*cg;
}

void SRcoord::GetPos(double &x, double &y, double &z, SRvec3& pos)
{
	//convert x, y, z in lcs to position in gcs
	//input:
	//x,y,z = lcs coordinates
	//output:
	//pos = gcs position
	if (type == cartesian)
	{
		pos.d[0] = x;
		pos.d[1] = y;
		pos.d[2] = z;
	}
	else if (type == cylindrical)
	{
		double r = x;
		double theta = y;
		double ct = cos(theta);
		double st = sqrt(1.0 - ct*ct);
		pos.d[0] = r*ct;
		pos.d[1] = r*st;
		pos.d[2] = z;
	}
	else if (type == spherical)
	{
		double r = x;
		double theta = y;
		double phi = z;
		double cphi = cos(phi);
		double sphi = sqrt(1.0 - cphi*cphi);
		double ct = cos(theta);
		double st = sqrt(1.0 - ct*ct);
		pos.d[0] = r*ct*sphi;
		pos.d[1] = r*st*sphi;
		pos.d[2] = r*cphi;
	}
	if (!gcsaligned)
	{
		double xt = pos.d[0];
		double yt = pos.d[1];
		double zt = pos.d[2];
		pos.d[0] = xt * e1.d[0] + yt * e2.d[0] + zt * e3.d[0];
		pos.d[1] = xt * e1.d[1] + yt * e2.d[1] + zt * e3.d[1];
		pos.d[2] = xt * e1.d[2] + yt * e2.d[2] + zt * e3.d[2];
	}

	pos.PlusAssign(origin);
}

void SRcoord::VecTransform(SRvec3 p, SRvec3& v)
{
	//transform an lcs vector to gcs
	SRvec3 vlcs, e1l, e2l, e3l;
	CalculateBasisVectors(p, e1l, e2l, e3l);
	vlcs.Copy(v);
	v.Zero();
	v.PlusAssign(e1l.d, vlcs.d[0]);
	v.PlusAssign(e2l.d, vlcs.d[1]);
	v.PlusAssign(e3l.d, vlcs.d[2]);
}

int SRcoord::checkParallelToGcs(int dof)
{
	if (gcsaligned)
		return dof;
	int pardof = -1;
	SRvec3 *edof;
	if (dof == 0)
		edof = &e1;
	else if (dof == 1)
		edof = &e2;
	else
		edof = &e3;
	for (int gcsdir = 0; gcsdir < 3; gcsdir++)
	{
		if (fabs(edof->d[gcsdir]) > (1.0 - SMALL))
		{
			pardof = gcsdir;
			break;
		}
	}
	return pardof;
}

void SRcoord::PrintToFile(SRfile& f)
{
	SRvec3 p1, p3;
	p1.Copy(origin);
	p1.PlusAssign(e1);
	p3.Copy(origin);
	p3.PlusAssign(e3);

	f.Print("%s", name.getStr());
	if (type == cartesian)
		f.Print(" cartesian");
	else if (type == spherical)
		f.Print(" spherical");
	else
		f.Print(" cylindrical");
	f.Print(" NotGcsAligned");
	f.PrintReturn();
	f.PrintLine("%lg %lg %lg //(origin)", origin.d[0], origin.d[1], origin.d[2]);
	f.PrintLine("%lg %lg %lg %lg %lg %lg //(/p1, p3 are points along local e1 and e3 axes)",
		p1.d[0], p1.d[1], p1.d[2], p3.d[0], p3.d[1], p3.d[2]);
}

