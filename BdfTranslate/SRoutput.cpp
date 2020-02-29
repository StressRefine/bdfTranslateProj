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
// SRoutput.cpp: implementation of the SRoutput class.
//
//////////////////////////////////////////////////////////////////////



#include <stdlib.h>
#include <search.h>
#include "SRmodel.h"
#include "SRmachDep.h"
#include "SRoutput.h"

extern SRmodel model;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

void SRoutput::DoOutput()
{
	for (int i = 0; i < model.GetNumMaterials(); i++)
	{
		if (model.GetMaterial(i)->active)
			model.numactiveMat++;
	}
	model.mshFile.PrintLine("EntityCounts From BDF translate");
	model.mshFile.PrintLine("%d //nodes", model.GetNumNodes());
	model.mshFile.PrintLine("%d //elements", model.GetNumElements());
	model.mshFile.PrintLine("%d //materials", model.numactiveMat);
	model.mshFile.PrintLine("%d //coordinates", model.Coords.GetNum());
	model.mshFile.PrintLine("%d //nodal contraints", model.GetNumConstraints());
	model.mshFile.PrintLine("0 0 //multi face constraint groups, multi face constraints");
	model.mshFile.PrintLine("0 //breakout constraints");
	model.mshFile.PrintLine("0 //nodal breakout constraints");
	model.mshFile.PrintLine("%d //forces", model.GetNumForces());
	model.mshFile.PrintLine("0 0 //multi face force groups, multi face forces");
	model.mshFile.PrintLine("%d //volume forces", model.volumeForces.GetNum());
	model.mshFile.PrintLine("%d //nodesWithDisplacements", model.volumeForces.GetNum());
	OutputMaterials();
	OutputCoordinates();
	OutputNodes();
	OutputElements();
	OutputConstraints();
	OutputForces();
	OutputVolumeForces();
	OutputThermalForce();
	model.mshFile.Close();
}

void SRoutput::OutputNodes()
{
	model.mshFile.PrintLine("nodes");
	for (int i = 0; i < model.nodes.GetNum(); i++)
	{
		SRnode* node = model.GetNode(i);
		if (node->isOrphan())
			continue;
		if (node->unSupported)
			model.mshFile.PrintLine(" %d %lg %lg %lg unsupported", node->GetUserid(), node->pos.d[0], node->pos.d[1], node->pos.d[2]);
		else if (node->shellOrBeamNode)
			model.mshFile.PrintLine(" %d %lg %lg %lg shellOrBeamNode", node->GetUserid(), node->pos.d[0], node->pos.d[1], node->pos.d[2]);
		else if (node->bsurf)
			model.mshFile.PrintLine(" %d %lg %lg %lg onBsurf", node->GetUserid(), node->pos.d[0], node->pos.d[1], node->pos.d[2]);
		else
			model.mshFile.PrintLine(" %d %lg %lg %lg", node->GetUserid(), node->pos.d[0], node->pos.d[1], node->pos.d[2]);
	}
	model.mshFile.PrintLine("end nodes");

}
void SRoutput::OutputElements()
{
	model.mshFile.PrintLine("elements");
	for (int i = 0; i <model.elements.GetNum(); i++)
	{
		SRelement* elem = model.GetElement(i);
		model.mshFile.Print(" %d %s ", elem->GetUserid(), elem->matname.getStr());
		for (int n = 0; n < elem->GetNumNodes(); n++)
		{
			int id = elem->GetNodeId(n);
			int uid = model.GetNodeUid(id);
			model.mshFile.Print(" %d", uid);
		}
		model.mshFile.Print("\n");
	}
	model.mshFile.PrintLine("end elements");

}


void SRoutput::OutputConstraints()
{
	int n = model.constraints.GetNum();
	if (n == 0)
		return;
	model.mshFile.PrintLine("constraints");
	for (int i = 0; i < n; i++)
	{
		SRconstraint* con = model.GetConstraint(i);
		int nuid = con->entityId;
		model.mshFile.Print(" %d", nuid);
		for (int dof = 0; dof < 3; dof++)
		{
			if (con->IsConstrainedDof(dof))
				model.mshFile.Print(" %lg", con->getDisp(0, dof));
			else
				model.mshFile.Print(" -");
		}
		if (con->coordId != -1)
		{
			SRcoord* coord = model.GetCoord(con->coordId);
			model.mshFile.Print(" coord %s", coord->name.getStr());
		}

		model.mshFile.Print("\n");
	}
	model.mshFile.PrintLine("end constraints");
}


void SRoutput::OutputForces()
{
	int n = model.forces.GetNum();
	if (n == 0)
		return;

	model.mshFile.PrintLine("forces");
	bool anyfaceForce = false;
	for (int i = 0; i < n; i++)
	{
		SRforce* force = model.GetForce(i);
		if (force->type == nodalForce)
		{
			int nuid = force->entityId;
			model.mshFile.Print(" %d", nuid);
			if (force->pressure)
				model.mshFile.PrintLine(" pressure %lg", force->GetForceVal(0, 0));
			else
			{
				if (force->coordId > 0)
				{
					int cid = model.input.CoordFind(force->coordId);
					SRcoord* coord = model.GetCoord(cid);
					model.mshFile.Print(" coord %s", coord->GetName());
				}
				else
					model.mshFile.Print(" gcs");
				for (int dof = 0; dof < 3; dof++)
					model.mshFile.Print(" %lg", force->GetForceVal(0, dof));
				model.mshFile.Print("\n");
			}
		}
		else if (force->type == faceForce)
			anyfaceForce = true;
	}
	model.mshFile.PrintLine("end forces");
	if (!anyfaceForce)
		return;
	model.mshFile.PrintLine("facePressures");
	for (int i = 0; i < n; i++)
	{
		SRforce* force = model.GetForce(i);
		if (force->type == faceForce)
		{
			if (force->pressure)
			{
				//facePressure, elid, n1, n2, n3, n4, p1, p2, p3, p4
				//elid = element that owns the face
				//n1, n2, n3, n4 = nodes at corner of face, n4 = 1 for tri
				//p1, p2, p3, p4 = pressures at corner of face, p4 omitted for tri face
				//p2, p3, p4 omitted for constant pressure
				int euid = force->entityId;
				model.mshFile.Print(" %d", euid);
				for (int n = 0; n < 4; n++)
					model.mshFile.Print(" %d", force->nv[n]);
				int nn = 3;
				if (force->nv[3] != -1)
					nn = 4;
				for (int n = 0; n < nn; n++)
					model.mshFile.Print(" %lg", force->forceVals.Get(n, 0));
				model.mshFile.Print("\n");
			}
		}
	}
	model.mshFile.PrintLine("end facePressures");
	model.mshFile.PrintLine("faceTractions");
	for (int i = 0; i < n; i++)
	{
		SRforce* force = model.GetForce(i);
		if (force->type == faceForce)
		{
			if (!force->pressure)
			{
				//faceTractions, elid, n1, n2, n3, n4
				//# t1, t2, t3, t4   (for dof 0)
				//# t1, t2, t3, t4   (for dof 1)
				//# t1, t2, t3, t4   (for dof 2)
				//elid = element that owns the face
				//n1, n2, n3, n4 = nodes at corner of face, n4 = 1 for tri
				int euid = force->entityId;
				model.mshFile.Print(" %d", euid);
				for (int n = 0; n < 4; n++)
					model.mshFile.Print(" %d", force->nv[n]);
				model.mshFile.PrintReturn();
				int nn = 3;
				if (force->nv[3] != -1)
					nn = 4;

				for (int dof = 0; dof < 3; dof++)
				{
					model.mshFile.Print("#");
					for (int n = 0; n < nn; n++)
						model.mshFile.Print(" %lg", force->forceVals.Get(n, dof));
					model.mshFile.Print("\n");
				}
			}
		}
	}
	model.mshFile.PrintLine("end faceTractions");
}

void SRoutput::OutputVolumeForces()
{
	int n = model.volumeForces.GetNum();
	if (n == 0)
		return;

	model.mshFile.PrintLine("volumeforces");
	for (int i = 0; i < n; i++)
	{
		SRvolumeForce* vol = model.volumeForces.GetPointer(i);
		if (vol->type == gravity)
			model.mshFile.PrintLine("gravity %lg %lg %lg", vol->g1, vol->g2, vol->g3);
		else
		{
			model.mshFile.Print("centrifugal %lg", vol->omega);
			for (int i = 0; i < 3; i++)
				model.mshFile.Print(" %lg", vol->axis.d[i]);
			for (int i = 0; i < 3; i++)
				model.mshFile.Print(" %lg", vol->origin.d[i]);
			model.mshFile.Print(" %lg", vol->alpha);
			model.mshFile.Print("\n");
		}
	}
	model.mshFile.PrintLine("end volumeforces");
}

void SRoutput::OutputMaterials()
{
	//spec: name type
	//type = isotropic, orthotropic or general
	//if iso: alpha then E,nu
	//if ortho: alphax, alphay, alphaz, then c11,c12,c13,c22,c23,c33,c44,c55,c66
	//if general: alphax, alphay, alphaz, then full cij matrix, 36 constants,6 per line
	//(must be symmetric)

	model.mshFile.PrintLine("materials");
	for (int i = 0; i < model.GetNumMaterials(); i++)
	{
		SRmaterial* mat = model.materials.GetPointer(i);
		if (!mat->active)
			continue;
		model.mshFile.PrintLine("%s iso", mat->name.getStr());
		model.mshFile.PrintLine("%lg %lg %lg %lg //rho alpha tref allowable", mat->rho, mat->alphax, mat->tref, mat->allowableStress);
		model.mshFile.PrintLine("%lg %lg //E nu", mat->E, mat->nu);
	}
	model.mshFile.PrintLine("end materials");
}
void SRoutput::OutputCoordinates()
{
	//Spec: name type "NotGcsAligned"
	//type = cartesian,spherical,cylindrical
	//x0,y0,z0 (origin)
	//if NotGcsAligned:
	//p1, p3 are points along local e1 and e3 axes
	int n = model.Coords.GetNum();
	if (n == 0)
		return;

	model.mshFile.PrintLine("Coordinate Systems");
	SRstring typeStr;
	for (int i = 0; i < n; i++)
	{
		SRcoord* coord = model.Coords.GetPointer(i);
		if (coord->type == cartesian)
			typeStr.Copy("cartesian");
		else if (coord->type == cylindrical)
			typeStr.Copy("cylindrical");
		if (coord->type == spherical)
			typeStr.Copy("spherical");
		if (coord->gcsaligned)
			model.mshFile.PrintLine("%s %s", coord->name.getStr(), typeStr.getStr());
		else
			model.mshFile.PrintLine("%s %s NotGcsAligned", coord->name.getStr(), typeStr.getStr());
		model.mshFile.PrintLine(" %lg %lg %lg", coord->origin.d[0], coord->origin.d[1], coord->origin.d[2]);
		if (!coord->gcsaligned)
		{
			SRvec3 p1 = coord->origin;
			p1 += coord->e1;
			SRvec3 p3 = coord->origin;
			p3 += coord->e3;
			model.mshFile.Print(" %lg %lg %lg", p1.d[0], p1.d[1], p1.d[2]);
			model.mshFile.PrintLine(" %lg %lg %lg", p3.d[0], p3.d[1], p3.d[2]);
		}
	}
	model.mshFile.PrintLine("end Coordinate Systems");
}


void SRoutput::OutputThermalForce()
{
	SRthermalForce* tf = model.thermalForce;
	if (tf == NULL)
		return;

	model.mshFile.PrintLine("Thermal Force");
	//catch case all nodes loaded with same temp, convert to constant
	bool constant = true;
	double T0 = 0.0;
	if (model.GetNode(0)->hasTemp)
	{
		T0 = model.GetNode(0)->Temp;
		double fT0 = fabs(T0);
		for (int i = 1; i < model.GetNumNodes(); i++)
		{
			SRnode* node = model.GetNode(i);
			if (!node->hasTemp)
			{
				constant = false;
				break;
			}
			double diff = fabs(node->Temp - T0);
			if(diff > RELSMALL*fT0)
			{
				constant = false;
				break;
			}
		}
	}
	else
		constant = false;

	if(constant)
		model.mshFile.PrintLine("constant %lg", T0);
	else
	{
		model.mshFile.PrintLine("variable");
		for (int i = 0; i < model.GetNumNodes(); i++)
		{
			SRnode* node = model.GetNode(i);
			if (node->hasTemp)
				model.mshFile.PrintLine("%d %lg", node->GetUserid(), node->Temp);
		}
	}
	model.mshFile.PrintLine("end Thermal Force");
}

