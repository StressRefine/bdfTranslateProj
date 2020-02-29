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
// SRinput.cpp: implementation of the SRinput class.
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

static int SRuidDataCompareFunc(const void *v1, const void *v2);

SRinput::SRinput()
{
	nodeUidOffset = 0;
	elemUidOffSet = 0;
	CoordUidOffset = 0;
	MatUidOffset = 0;
	elPropUidOffset = 0;

	lastNodeUid = -1;
	lastNodeId = -1;
	lastCoordUid = -1;
	lastCoordId = -1;
	lastMatUid = -1;
	lastMatId = -1;
	lastElPropUid = -1;
	lastElPropId = -1;
	lastElemUid = -1;
	lastElemId = -1;
}

bool SRinput::Translate()
{
	//translate data from external source (bdf, etc) in model.inFile and store in model classes for output to model.mshfile
	//convert to SR .msh data, then write back out to .mshFile

	SRstring filename, line, basename, tail;

	//assign output, results, and combined results names; delete existing
	//files in case future opens will be for "append":
	tail = model.fileNameTail;
	basename = model.outdir;
	basename += slashStr;
	basename += tail;
	filename = basename;

	if(!model.inpFile.Open(SRinputMode))
	{
		const char *tmp = filename.LastChar(slashChar, true);
		SCREENPRINT(" bdf file not found: %s", tmp);
		OUTPRINT(" bdf file not found: %s", tmp);
		exit(0);
	}

	//coordinates, materials and element properties will be read in on first pass for efficiency.
	//they need to be stored and sorted for quick lookup during element input
	model.Coords.Allocate(MAXNCOORD);
	model.materials.Allocate(MAXNMAT);
	model.elProps.Allocate(MAXNMAT);

	if (!BdfInput())
		return false;

	//output:
	model.mshFile.Open(SRoutputMode);
	model.output.DoOutput();

	nodeUids.Free();
	elemUids.Free();
	elpropUids.Free();
	matUids.Free();
	coordUids.Free();
	return true;
}

int SRinput::GetMaterialId(SRstring &name)
{
	//look up the material id with "name"
	//return:
		//material number, -1 if not found

	SRmaterial* mat;
	for (int i = 0; i < model.materials.GetNum(); i++)
	{
		mat = model.GetMaterial(i);
		if (name.Compare(mat->name))
			return i;
	}
	return -1;
}

int SRinput::GetCoordId(SRstring &name)
{
	//look up the coordinate system id with "name"
	//return:
		//coordinate system number, -1 if not found

	SRcoord*     coord;
	for (int i = 0; i < model.Coords.GetNum(); i++)
	{
		coord = model.GetCoord(i);
		if (name == coord->name)
			return i;
	}
	return -1;
}

void SRinput::SortOtherEntities()
{
	SRuidData *nuid;
	//sort "coordUids" array in ascending order of uid
	int ncoord = model.Coords.GetNum();
	if (CoordUidOffset == -1)
	{
		if (ncoord > 0)
		{
			coordUids.Allocate(ncoord);
			for (int i = 0; i < ncoord; i++)
			{
				SRcoord* coord = model.GetCoord(i);
				nuid = coordUids.GetPointer(i);
				nuid->id = i;
				nuid->uid = coord->uid;
			}
			qsort(coordUids.GetVector(), ncoord, sizeof(SRuidData), SRuidDataCompareFunc);
		}
	}
	//sort "matUids" array in ascending order of uid
	if (MatUidOffset == -1)
	{
		int nmat = model.materials.GetNum();
		if (nmat > 0)
		{
			matUids.Allocate(nmat);
			for (int i = 0; i < nmat; i++)
			{
				SRmaterial* mat = model.GetMaterial(i);
				nuid = matUids.GetPointer(i);
				nuid->id = i;
				nuid->uid = mat->uid;
			}
			qsort(matUids.GetVector(), nmat, sizeof(SRuidData), SRuidDataCompareFunc);
		}
		else
			ERROREXIT; //model has to have at least one mat prop
	}
	//sort "elpropUids" array in ascending order of uid

	int nelprop = model.elProps.GetNum();
	for (int i = 0; i < nelprop; i++)
	{
		SRElProperty* prop = model.elProps.GetPointer(i);
		int muid = prop->matuid;
		int mid = MatFind(muid);
		prop->matid = mid;
	}
	if (elPropUidOffset == -1)
	{
		elpropUids.Allocate(nelprop);
		if (nelprop > 0)
		{
			for (int i = 0; i < nelprop; i++)
			{
				SRElProperty* prop = model.elProps.GetPointer(i);
				nuid = elpropUids.GetPointer(i);
				nuid->id = i;
				nuid->uid = prop->uid;
			}
			qsort(elpropUids.GetVector(), nelprop, sizeof(SRuidData), SRuidDataCompareFunc);
		}
		else
			ERROREXIT; //model has to have at least one el prop
	}
}

void SRinput::SortNodes()
{
	SRuidData *nuid;
	//fill node uid vector and sort in ascending order of uid for faster
	//node-finding:
	if (nodeUidOffset == -1)
	{
		int n = model.nodes.GetNum();
		if (n == 0)
			ERROREXIT;
		nodeUids.Allocate(n);
		for (int i = 0; i < n; i++)
		{
			SRnode* node = model.GetNode(i);
			nuid = nodeUids.GetPointer(i);
			nuid->id = i;
			nuid->uid = node->userId;
		}
		qsort(nodeUids.GetVector(), n, sizeof(SRuidData), SRuidDataCompareFunc);
	}

}

void SRinput::SortElements()
{
	SRuidData *nuid;

	//fill elem uid vector and sort in ascending order of uid for faster
	//elem-finding:
	if (elemUidOffSet == -1)
	{
		int n = model.elements.GetNum();
		if (n == 0)
			ERROREXIT;
		elemUids.Allocate(n);
		for (int i = 0; i < n; i++)
		{
			SRelement* elem = model.GetElement(i);
			nuid = elemUids.GetPointer(i);
			nuid->id = i;
			nuid->uid = elem->uid;
		}
		qsort(elemUids.GetVector(), n, sizeof(SRuidData), SRuidDataCompareFunc);
	}
}


int SRinput::NodeFind(int uid)
{
	//find node with user Id uid
	//input:
	//uid = user id to match
	//return:
	//number of the node that matches uid, -1 if not found

	int id = -1;

	//binary search:
	if (nodeUidOffset != -1)
		return uid - nodeUidOffset;
	else if (uid == lastNodeUid)
		id = lastNodeId;
	else
	{
		SRuidData* nuid;
		SRuidData uidt;
		//"search key" has to be same data type expected by compare function see SRuidDataCompareFunc:
		uidt.uid = uid;
		nuid = (SRuidData *)bsearch(&uidt, nodeUids.GetVector(), nodeUids.GetNum(), sizeof(SRuidData), SRuidDataCompareFunc);
		if (nuid != NULL)
			id = nuid->id;
	}
	lastNodeId = id;
	lastNodeUid = uid;
	return id;
}


int SRinput::CoordFind(int uid)
{
	//find coord with user Id uid
	//input:
		//uid = user id to match
	//return:
		//number of the coord that matches uid, -1 if not found

	int id = -1;
	if (CoordUidOffset != -1)
		return uid - CoordUidOffset;
	else if (uid == lastCoordUid)
		id = lastCoordId;
	else
	{
		//binary search:
		
		SRuidData* nuid;
		SRuidData uidt;
		//"search key" has to be same data type expected by compare function see SRuidDataCompareFunc:
		uidt.uid = uid;
		nuid = (SRuidData *)bsearch(&uidt, coordUids.GetVector(), coordUids.GetNum(), sizeof(SRuidData), SRuidDataCompareFunc);
		if (nuid != NULL)
			id = nuid->id;
	}
	lastCoordId = id;
	lastCoordUid = uid;
	return id;
}
int SRinput::MatFind(int uid)
{
	//find material with user Id uid
	//input:
		//uid = user id to match
	//return:
		//number of the mat that matches uid, -1 if not found

	int id = -1;
	if (MatUidOffset != -1)
		return uid - MatUidOffset;
	else if (uid == lastMatUid)
		id = lastMatId;
	else
	{
		//binary search:
		
		SRuidData* nuid;
		SRuidData uidt;
		//"search key" has to be same data type expected by compare function see SRuidDataCompareFunc:
		uidt.uid = uid;
		nuid = (SRuidData *)bsearch(&uidt, matUids.GetVector(), matUids.GetNum(), sizeof(SRuidData), SRuidDataCompareFunc);
		if (nuid != NULL)
			id = nuid->id;
	}
	lastMatId = id;
	lastMatUid = uid;
	return id;

}
int SRinput::ElpropFind(int uid)
{
	//find elem. prop. with user Id uid
	//input:
		//uid = user id to match
	//return:
		//number of the elem. prop.  that matches uid, -1 if not found

	int id = -1;
	if (elPropUidOffset != -1)
		return uid - elPropUidOffset;
	else if (uid == lastElPropUid)
		id = lastElPropId;
	else
	{
		//binary search:
		SRuidData* nuid;
		SRuidData uidt;
		//"search key" has to be same data type expected by compare function see SRuidDataCompareFunc:
		uidt.uid = uid;
		nuid = (SRuidData *)bsearch(&uidt, elpropUids.GetVector(), elpropUids.GetNum(), sizeof(SRuidData), SRuidDataCompareFunc);
		if (nuid != NULL)
			id = nuid->id;
	}
	lastElPropId = id;
	lastElPropUid = uid;
	return id;
}

int SRinput::ElemFind(int uid)
{
	//find elem with user Id uid
	//input:
		//uid = user id to match
	//return:
		//number of the elem. that matches uid, -1 if not found

	int id = -1;
	if (elemUidOffSet != -1)
	{
		id = uid - elemUidOffSet;
		if (id >= model.GetNumElements())
			id = -1;
	}
	else if (uid == lastElemUid)
	{
		id = lastElemId;
		if (id >= model.GetNumElements())
			id = -1;
	}
	else
	{
		//binary search:
		SRuidData* nuid;
		SRuidData uidt;
		//"search key" has to be same data type expected by compare function see SRuidDataCompareFunc:
		uidt.uid = uid;
		nuid = (SRuidData *)bsearch(&uidt, elemUids.GetVector(), elemUids.GetNum(), sizeof(SRuidData), SRuidDataCompareFunc);
		if (nuid != NULL)
			id = nuid->id;
	}
	lastElemId = id;
	lastElemUid = uid;
	return id;
}

void SRinput::checkLcs(SRconstraint* con, int cid)
{
	if (cid == -1)
		return;
	SRcoord* coord = model.GetCoord(cid);
	if (coord->type == cartesian && !con->hasEnforcedDisp())
	{
		//check if spc can be converted to equivalent gcs:
		int dof;
		int ncondof = 0;
		for (dof = 0; dof < 3; dof++)
		{
			if (con->constrainedDof[dof])
				ncondof++;
		}
		if (ncondof == 1)
		{
			//check if the constrained direction is parallel to a gcs direction:
			for (dof = 0; dof < 3; dof++)
			{
				if (con->constrainedDof[dof])
				{
					int pardof = coord->checkParallelToGcs(dof);
					if (pardof != -1)
					{
						con->constrainedDof[dof] = false;
						con->constrainedDof[pardof] = true;
						cid = -1;
						break;
					}
				}
			}
		}
		else if (ncondof == 2)
		{
			//check if the free direction is parallel to a gcs direction:
			for (dof = 0; dof < 3; dof++)
			{
				if (!con->constrainedDof[dof])
				{
					int pardof = coord->checkParallelToGcs(dof);
					if (pardof != -1)
					{
						for (int i = 0; i < 3; i++)
							con->constrainedDof[i] = true;
						con->constrainedDof[pardof] = false;
						cid = -1;
						break;
					}
				}
			}
		}
		else if (ncondof == 3)
		{
			//all 3 dofs constrained in lcs is equiv to all 3 in gcs:
			cid = -1;
		}
	}
	con->coordId = cid;
}

int SRinput::findElemFace(bool needMidSide, SRelement* elem, int g1, int g2, int gout[8], double *pv)
{
	//for a quad face, find all for nodes for face that matches diag corners g1,g2
	int ncorner = -1;
	int nv[8];
	int nn;
	if (elem->type == brick)
	{
		for (int lf = 0; lf < 6; lf++)
		{
			nn = elem->GetFaceNodes(needMidSide, lf, nv);
			for (int i = 0; i < nn; i++)
			{
				if (g1 == nv[i])
				{
					for (int j = 0; j < nn; j++)
					{
						if (g2 == nv[j])
						{
							ncorner = nn;
							if (needMidSide)
								nn *= 2;
							for (int k = 0; k < nn * 2; k++)
								gout[k] = nv[k];
							break;
						}
					}
				}
				if (ncorner != -1)
					break;
			}
		}
	}
	else if (elem->type == wedge)
	{
		if (g2 == -1)
		{
			//search for match on tri face:
			for (int lf = 0; lf < 2; lf++)
			{
				nn = elem->GetFaceNodes(needMidSide, lf, nv);
				for (int i = 0; i < nn; i++)
				{
					if (g1 == nv[i])
					{
						ncorner = 3;
						if (needMidSide)
							nn *= 2;
						for (int k = 0; k < nn * 2; k++)
							gout[k] = nv[k];
						break;
					}
				}
			}
		}
		else
		{
			//search for match on quad face:
			for (int lf = 2; lf < 5; lf++)
			{
				nn = elem->GetFaceNodes(needMidSide, lf, nv);
				for (int i = 0; i < nn; i++)
				{
					if (g1 == nv[i])
					{
						for (int j = 0; j < nn; j++)
						{
							if (g2 == nv[j])
							{
								ncorner = nn;
								if (needMidSide)
									nn *= 2;
								for (int k = 0; k < nn * 2; k++)
									gout[k] = nv[k];
								break;
							}
						}
					}
					if (ncorner != -1)
						break;
				}
			}
		}
	}
	else if (elem->type == tet)
	{
		for (int lf = 0; lf < 4; lf++)
		{
			nn = elem->GetFaceNodes(needMidSide, lf, nv);
			for (int i = 0; i < 3; i++)
			{
				if (g1 == nv[i])
				{
					int g2NotOnFace = true;
					for (int j = 0; j < 3; j++)
					{
						if (g2 == nv[j])
						{
							g2NotOnFace = false;
							break;
						}
					}
					if (g2NotOnFace)
					{
						ncorner = 3;
						if (needMidSide)
							nn *= 2;
						for (int k = 0; k < nn; k++)
							gout[k] = nv[k];
						gout[6] = -1;
						break;
					}

				}
				if (ncorner != -1)
					break;
			}
		}
	}

	if (pv != NULL)
	{
		//interpolate on face to get pv of midnodes:
		if (ncorner == 3)
		{
			//tri face
			pv[3] = 0.5*(pv[0] + pv[1]);
			pv[4] = 0.5*(pv[1] + pv[2]);
			pv[5] = 0.5*(pv[0] + pv[2]);
		}
		else
		{
			//quad face
			pv[4] = 0.5*(pv[0] + pv[1]);
			pv[5] = 0.5*(pv[1] + pv[2]);
			pv[6] = 0.5*(pv[3] + pv[2]);
			pv[7] = 0.5*(pv[0] + pv[3]);
		}
	}
	return ncorner;
}

void SRinput::SetNodeElmentOwners()
{
	for (int e = 0; e < model.GetNumElements(); e++)
	{
		SRelement* elem = model.GetElement(e);
		elem->SetNodeElmentOwners();
	}
}

bool SRinput::IsWedgeFaceQuad(SRelement* elem, int gid[])
{
	//from 1st 3 nodes of a wedge face, find local face and see if it is a quad
	int nv[4];
	for (int lf = 0; lf < 5; lf++)
	{
		int nn = elem->GetFaceNodes(false, lf, nv);
		int nmatches = 0;
		for (int j = 0; j < 3; j++)
		{
			for (int i = 0; i < nn; i++)
			{
				if (gid[j] == nv[i])
				{
					nmatches++;
					if (nmatches == 3)
						break;
				}
			}
			if (nmatches == 3)
			{
				if (lf < 2)
					return false;
				else
					return true;
			}
		}
	}
	return false;

}


static int SRuidDataCompareFunc(const void *v1, const void *v2)
{
	//compare function for sorting and searching of node userids
	//input:
	//v1 = pointer to uid data for node 1
	//v2 = pointer to uid data for node 2
	//return:
	//-1 if node1 uid is less than node2 uid
	//0  if node1 uid is equal to node2 uid
	//+1 if node1 uid is greater than node2 uid

	SRuidData* node1 = (SRuidData*)v1;
	SRuidData* node2 = (SRuidData*)v2;
	if (node1->uid < node2->uid)
		return -1;
	else if (node1->uid == node2->uid)
		return 0;
	else
		return 1;
}

