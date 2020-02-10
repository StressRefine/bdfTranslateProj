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
// SRconstraint.cpp: implementation of the SRconstraint class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SRmodel.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

extern SRmodel model;

SRconstraint::SRconstraint()
{
	type = nodalCon;
	for (int i = 0; i < 3; i++)
		constrainedDof[i] = 0;
	coordId = -1;
	breakoutElemUid = -1;
}

void SRconstraint::Clear()
{
	for (int i = 0; i < 3; i++)
	{
		constrainedDof[i] = 0;
	}
	coordId = -1;
	entityId = -1;
	coordId = -1;
	enforcedDisplacementData.Free();
}


void SRconstraint::Copy(SRconstraint& that)
{
	for (int i = 0; i < 3; i++)
		constrainedDof[i] = that.constrainedDof[i];
	if (!that.enforcedDisplacementData.isEmpty())
	{
		int n, m;
		that.enforcedDisplacementData.getSize(n, m);
		enforcedDisplacementData.Allocate(n,m);
		enforcedDisplacementData.Copy(that.enforcedDisplacementData);
	}
	entityId = that.entityId;
	type = that.type;
	coordId = that.coordId;
}

void SRconstraint::PlusAssign(SRconstraint& that)
{
	for (int i = 0; i < 3; i++)
		constrainedDof[i] += that.constrainedDof[i];
	if (!that.enforcedDisplacementData.isEmpty())
		enforcedDisplacementData.PlusAssign(that.enforcedDisplacementData);
}

void SRconstraint::ProcessFaceConstraint()
{
}

SRcoord* SRconstraint::GetCoord()
{
	//look up coordinate system associated with a constraint, if any
	//return:
	// pointer to coordinate system, NULL if none
	if (coordId == -1)
		return NULL;
	else
		return model.GetCoord(coordId);
}


void SRconstraint::PutEnforcedDisplacementData(int n, int dof, double val)
{
	if (enforcedDisplacementData.isEmpty())
		enforcedDisplacementData.Allocate(n + 1, 3);
	enforcedDisplacementData.Put(n, dof, val);
};

double SRconstraint::GetEnforcedDisp(int nodeNum, int dof)
{
	//look up enforced displacement value at a node of an edge or face constraint
	//input:
	//nodeNum = node number of corner of midside
	//dof = degree of freedom
	//return:
	//displacement value;
	if (enforcedDisplacementData.isEmpty())
		return 0.0;
	double disp = enforcedDisplacementData.Get(nodeNum, dof);

	return disp;
}
