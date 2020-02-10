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
// SRnode.cpp: implementation of the SRnode class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SRmodel.h"

extern SRmodel model;

SRnode::SRnode()
{
	userId = -1;
	firstElementOwner = -1;
	forceId = -1;
	constraintId = -1;
	hasTemp = false;
	Temp = 0.0;
	dispCoordid = -1;
	unSupported = false;
	shellOrBeamNode = false;
	ismidside = false;
	hasDisp = false;
	bsurf = false;
}

void SRnode::Copy(SRnode& n2)
{
	userId = n2.userId;
	pos.Copy(n2.pos);
	firstElementOwner = n2.firstElementOwner;
	constraintId = n2.constraintId;
	forceId = n2.forceId;
}


SRforce* SRnode::GetForce()
{
	if (forceId == -1)
		return NULL;
	else
		return model.GetForce(forceId);
}

SRconstraint* SRnode::GetConstraint()
{
	if (constraintId == -1)
		return NULL;
	else
		return model.GetConstraint(constraintId);
}
