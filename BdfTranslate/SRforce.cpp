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
// SRforce.cpp: implementation of the SRforce class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SRmodel.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

extern SRmodel model;

void SRforce::Copy(SRforce& that, bool copyForceVals)
{
	type = that.type;
	pressure = that.isPressure();
	coordId = that.coordId;
	entityId = that.entityId;
	if (copyForceVals)
		forceVals.Copy(that.forceVals);
}


void SRvolumeForce::GetForceValue(SRelement *elem,SRvec3 &p,double val[])
{
    //return the value of a volume force in an element at position p
    //input:
        //elem = pointer to element
        //p = position
    //output
        //val = 3 dof force values at p
    //note:
		//p is only used for centrifugal, gravity is not dependent on position

	double rho, rhoOmega2;
	int mid = elem->GetMaterialId();
	SRmaterial *mat = model.GetMaterial(mid);
	rho = mat->GetRho();
	SRvec3 R, omR;
	if (type == gravity)
	{
		val[0] = rho*g1;
		val[1] = rho*g2;
		val[2] = rho*g3;
	}
	else if (type == centrifugal)
	{
		rhoOmega2 = rho*omega*omega;
		p.Subtract(origin, R);
		axis.Cross(R, omR);
		axis.Cross(omR, R);
		val[0] = -rhoOmega2*R.d[0];
		val[1] = -rhoOmega2*R.d[1];
		val[2] = -rhoOmega2*R.d[2];
	}
}

void SRthermalForce::Process()
{
}

bool SRthermalForce::CeTMult(SRmaterial *mat, double eTx, double eTy, double eTz, double ceT[])
{
	//multiply C-matrix times eT for this thermal force
	//input::
		//mat = pointer to material
		//eTx = alphax-T=thermal normal strain
		//eTy = alphay-T=thermal normal strain
		//eTz = alphaz-T=thermal normal strain
	//output:
		//ceT[6] = C*eT
	//return:
		//fullCeT indicates whether Ce[3,4,5] are 0
	double C[3][3],eT[3];
	int i,j;
	eT[0] = eTx;
	eT[1] = eTy;
	eT[2] = eTz;
	for(i = 0;i < 6; i++)
		ceT[i] = 0.0;
	if (mat->GetType() == iso)
	{
		double c11 = mat->c11;
		double lambda = mat->lambda;
		double G = mat->G;
		C[0][0] = c11;
		C[0][1] = lambda;
		C[0][2] = lambda;
		C[1][0] = lambda;
		C[1][1] = c11;
		C[1][2] = lambda;
		C[2][0] = lambda;
		C[2][1] = lambda;
		C[2][2] = c11;
	}
	else if (mat->GetType() == ortho)
	{
		C[0][0] = mat->orthoCij.c11;
		C[0][1] = mat->orthoCij.c12;
		C[0][2] = mat->orthoCij.c13;
		C[1][0] = mat->orthoCij.c12;
		C[1][1] = mat->orthoCij.c22;
		C[1][2] = mat->orthoCij.c23;
		C[2][0] = mat->orthoCij.c13;
		C[2][1] = mat->orthoCij.c23;
		C[2][2] = mat->orthoCij.c33;
	}
	else if (mat->GetType() == genAniso)
	{
		for(i = 0; i < 6; i++)
		{
			ceT[i] = 0.0;
			for(j = 0; j < 3; j++)
				ceT[i] += (mat->genAnisoCij.c[i][j] * eT[j]);
		}
		return true;
	}

	for(i = 0; i < 3; i++)
	{
		for(j = 0; j < 3; j++)
			ceT[i] += (C[i][j] * eT[j]);
	}
	return false;
}

double SRthermalForce::GetTemp(SRelement *elem, double r, double s, double t)
{
	//get temperature in an element due to this thermal force at natural coordinate point
	//input:
		//elem = pointer to element
		//r,s,t = natural coordinates
	//return:
		//temperature
	return 0.0;
}

