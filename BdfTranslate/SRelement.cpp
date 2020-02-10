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
// SRelement.cpp: implementation of the SRelement class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SRmodel.h"

extern SRmodel model;

void SRelement::Create(SRelementType typet, int useridt, int nnodes, int nodest[])
{
	//Create element. store nodes and material in element arrays
	//input:
		//typet = element type
		//useridt = user ID
		//nodest = vector of node ids

	type = typet;
	uid = useridt;
	nodeUIds.Allocate(nnodes);

	SRnode* node;
	for (int i = 0; i < nnodes; i++)
		nodeUIds.Put(i, nodest[i]);
}

int SRelement::GetFaceNodes(bool needMidside, int lface, int nv[])
{
	int i, ln, nn;
	if (type == brick)
	{
		nn = 4;
		if (needMidside)
			nn *= 2;
		for (i = 0; i < nn; i++)
		{
			ln = model.brickFaceLocalNodes[lface][i];
			nv[i] = nodeUIds.Get(ln);
		}
	}
	if (type == tet)
	{
		nn = 3;
		if (needMidside)
			nn *= 2;
		for (i = 0; i < nn; i++)
		{
			ln = model.tetFaceLocalNodes[lface][i];
			nv[i] = nodeUIds.Get(ln);
		}
	}
	if (type == wedge)
	{
		if (lface < 2)
			nn = 3;
		else
			nn = 4;
		if (needMidside)
			nn *= 2;
		for (i = 0; i < nn; i++)
		{
			ln = model.wedgeFaceLocalNodes[lface][i];
			nv[i] = nodeUIds.Get(ln);
		}
	}
	if (needMidside)
		nn /= 2;
	return nn;
}
void SRelement::GetFaceNodes(int lface, int &n1, int &n2, int &n3, int &n4)
{
	//get the node numbers at the corners of a local face of an element.
	//this is for use before global face assignment, so this cannot be looked
	//up with localFaces.
	//input:
		//lface = local face number
	//output:
		//n1,n2,n3,n4 = nodes at corners of face; n4 = -1 for tri face
	if (type == tet)
	{
		if (lface == 0)
		{
			//local face 1 = 2-3-4 (face for which L1=0)
			n1 = nodeUIds.Get(1);
			n2 = nodeUIds.Get(2);
			n3 = nodeUIds.Get(3);
		}
		else if (lface == 1)
		{
			//local face 2 = 1-3-4 (face for which L2=0)
			n1 = nodeUIds.Get(0);
			n2 = nodeUIds.Get(2);
			n3 = nodeUIds.Get(3);
		}
		else if (lface == 2)
		{
			//local face 3 = 1-2-4 (face for which L3=0)
			n1 = nodeUIds.Get(0);
			n2 = nodeUIds.Get(1);
			n3 = nodeUIds.Get(3);
		}
		else if (lface == 3)
		{
			//local face 4 = 1-2-3 (face for which L4=0)
			n1 = nodeUIds.Get(0);
			n2 = nodeUIds.Get(1);
			n3 = nodeUIds.Get(2);
		}
		n4 = -1;
	}
	else if (type == wedge)
	{
		GetWedgeFaceNodes(lface, n1, n2, n3, n4);
	}
	else
	{
		GetBrickFaceNodes(lface, n1, n2, n3, n4);
	}
}


SRnode* SRelement::GetNode(int localnodenum)
{
	//get the pointer to the global node corresponding to local node number
	//input:
		//localnodenum = local number in element
	//return:
		//pointer to the global node

	int uid = nodeUIds.Get(localnodenum);
	return model.GetNodeFromUid(uid);
}




void SRelement::Cleanup()
{
	nodeUIds.Free();
}

bool SRelement::nodeDistCheck(SRvec3& pos, double radius)
{
	//check the distance between all nodes of the element and a position.
	//input:
	//pos = position
	//radius = test radius
	//return
	//true if all nodes are within radius of pos, else false

	for (int n = 0; n < nodeUIds.GetNum(); n++)
	{
		int uid = nodeUIds.Get(n);
		SRnode* node = model.GetNodeFromUid(uid);
		double d = pos.Distance(node->pos);
		if (d > radius)
			return false;
	}
	return true;
}

bool SRelement::InsideBoundingBox(double xmin, double xmax, double ymin, double ymax, double zmin, double zmax)
{
	//check whether all nodes of the element are withn a bounding box
	//input:
	//xmin, xmax, ymin, ymax, zmin, zmax = bounding box
	//return
	//true if all nodes are within bounding box, else false

	for (int n = 0; n < nodeUIds.GetNum(); n++)
	{
		int uid = nodeUIds.Get(n);
		SRnode* node = model.GetNodeFromUid(uid);
		double x = node->pos.d[0];
		double y = node->pos.d[1];
		double z = node->pos.d[2];
		if (x < xmin || x > xmax || y < ymin || y > ymax || z < zmin || z > zmax)
			return false;
	}
	return true;
}

int SRelement::GetNodeId(int i)
{
	int uid = nodeUIds.Get(i);
	return model.input.NodeFind(uid);
}

void SRelement::SetNodeElmentOwners()
{
	for (int i = 0; i < nodeUIds.GetNum(); i++)
	{
		int nuid = nodeUIds.Get(i);
		SRnode* node = model.GetNodeFromUid(nuid);
		node->firstElementOwner = id;
	}
}
int SRelement::GetNumLocalFaces()
{
	if (type == brick)
		return 6;
	else if (type == tet)
		return 4;
	else if (type == wedge)
		return 5;
	else
		return 0;
}

int SRelement::GetNumCorners()
{
	if (type == brick)
		return 8;
	else if (type == tet)
		return 4;
	else
		return 6;
}

