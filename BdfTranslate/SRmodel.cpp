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
// SRmodel.cpp: implementation of the SRmodel class.
//
//////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include "SRmodel.h"
#include "SRoutput.h"

#define ERRTOL 0.02

#define MINP 2
#define MAXP 8

//////////////////////////////////////////////////////////////////////
SRmodel::SRmodel()
{
	size = 0.0;
	anybricks = false;
	anywedges = false;
    anyEnforcedDisplacement = false;
	thermalForce = NULL;
	nodeToFaceBCs = false;
	numbreakoutElems = 0;
	linearMesh = false;
	anyUnsupportedElement = false;
	numnodalforces = 0;
	numactiveMat = 0;
	cropModelWithDispNodes = false;
	partialDispFile = false;
	anyGeneralUnsupportedNode = false;
	anyShellOrBeamNode = false;
	isNx = false;


	//mapping from local node numbers on faces to local node numbers
	//in tet element:
	int brickfacelnodetmp0[8] = { 0, 1, 2, 3, 8, 9, 10, 11 };
	int brickfacelnodetmp1[8] = { 4, 5, 6, 7, 12, 13, 14, 15 };
	int brickfacelnodetmp2[8] = { 0, 3, 7, 4, 11, 19, 15, 16 };
	int brickfacelnodetmp3[8] = { 1, 2, 6, 5, 9, 18, 13, 17 };
	int brickfacelnodetmp4[8] = { 0, 1, 5, 4, 8, 17, 12, 16 };
	int brickfacelnodetmp5[8] = { 3, 2, 6, 7, 10, 18, 14, 19 };
	int tetfacelnodetmp0[6] = { 1, 2, 3, 5, 9, 8 };
	int tetfacelnodetmp1[6] = { 0, 2, 3, 5, 9, 7 };
	int tetfacelnodetmp2[6] = { 0, 1, 3, 4, 8, 7 };
	int tetfacelnodetmp3[6] = { 0, 1, 2, 4, 5, 6 };
	int wedgefacelnodetmp0[8] = { 0, 1, 2, 6, 7, 8, -1, -1 };
	int wedgefacelnodetmp1[8] = { 3, 4, 5, 9, 10, 11, -1, -1 };
	int wedgefacelnodetmp2[8] = { 0, 1, 4, 3, 6, 13, 9, 12 };
	int wedgefacelnodetmp3[8] = { 1, 2, 5, 4, 7, 14, 10, 13 };
	int wedgefacelnodetmp4[8] = { 0, 2, 5, 3, 8, 14, 11, 12 };
	for (int i = 0; i < 4; i++)
	{
		brickFaceLocalNodes[0][i] = brickfacelnodetmp0[i];
		brickFaceLocalNodes[1][i] = brickfacelnodetmp1[i];
		brickFaceLocalNodes[2][i] = brickfacelnodetmp2[i];
		brickFaceLocalNodes[3][i] = brickfacelnodetmp3[i];
		brickFaceLocalNodes[4][i] = brickfacelnodetmp4[i];
		brickFaceLocalNodes[5][i] = brickfacelnodetmp5[i];
	}
	for (int i = 0; i < 3; i++)
	{
		tetFaceLocalNodes[0][i] = tetfacelnodetmp0[i];
		tetFaceLocalNodes[1][i] = tetfacelnodetmp1[i];
		tetFaceLocalNodes[2][i] = tetfacelnodetmp2[i];
		tetFaceLocalNodes[3][i] = tetfacelnodetmp3[i];
	}
	for (int i = 0; i < 4; i++)
	{
		wedgeFaceLocalNodes[0][i] = wedgefacelnodetmp0[i];
		wedgeFaceLocalNodes[1][i] = wedgefacelnodetmp1[i];
		wedgeFaceLocalNodes[2][i] = wedgefacelnodetmp2[i];
		wedgeFaceLocalNodes[3][i] = wedgefacelnodetmp3[i];
		wedgeFaceLocalNodes[4][i] = wedgefacelnodetmp4[i];
	}
	mapSetup();
};

void SRmodel::CleanUp(bool partial)
{
	nodes.Free();
	constraints.Free();
	Coords.Free();
	materials.Free();
	forces.Free();
	for (int e = 0; e < GetNumElements(); e++)
	{
		SRelement *elem = GetElement(e);
		elem->Cleanup();
	}
	elements.Free();
	volumeForces.Free();
	if (thermalForce != NULL)
	{
		DELETEMEMORY thermalForce;
		thermalForce = NULL;
	}

}

static bool errorExitCalled = false;

void SRmodel::ErrorExit(const char *file, int line)
{
	//print error messages and shut down
	//when fatal error occurs
	//input:
	//file = filename where error occurred
	//line = line number where error occurred

	if (!errorExitCalled)
	{
		errorExitCalled = true;
		SRstring s;
		s.Copy(file);
		const char *t = s.LastChar(slashChar);
		if (t != NULL)
		{
			SCREENPRINT("\nFatal Error\nFile: %s\nLine: %d\n", t + 1, line);
			OUTPRINT("\nFatal Error\nFile: %s\nLine: %d\n", t + 1, line);
		}
		else
		{
			SCREENPRINT("\nFatal Error\nFile: %s\nLine: %d\n", file, line);
			OUTPRINT("\nFatal Error\nFile: %s\nLine: %d\n", file, line);
		}
		//int iii = _gettchar();
		exit(0);
	}
	else
		exit(0);
}

void SRmodel::mapSetup()
{
	//mapping from local edge numbers on local faces of elements to element node numbers

	int brickfacelejtmp0[4] = { 8, 9, 10, 11 };
	int brickfacelejtmp1[4] = { 12, 13, 14, 15 };
	int brickfacelejtmp2[4] = { 11, 19, 15, 16 };
	int brickfacelejtmp3[4] = { 9, 18, 13, 17 };
	int brickfacelejtmp4[4] = { 8, 17, 12, 16 };
	int brickfacelejtmp5[4] = { 10, 18, 14, 19 };

	for (int i = 0; i < 4; i++)
	{
		brickFaceLocalEdgeMidnodeNum[0][i] = brickfacelejtmp0[i];
		brickFaceLocalEdgeMidnodeNum[1][i] = brickfacelejtmp1[i];
		brickFaceLocalEdgeMidnodeNum[2][i] = brickfacelejtmp2[i];
		brickFaceLocalEdgeMidnodeNum[3][i] = brickfacelejtmp3[i];
		brickFaceLocalEdgeMidnodeNum[4][i] = brickfacelejtmp4[i];
		brickFaceLocalEdgeMidnodeNum[5][i] = brickfacelejtmp5[i];
	}
	
	int tetfacelejtmp0[3] = { 5, 9, 8 };
	int tetfacelejtmp1[3] = { 6, 9, 7 };
	int tetfacelejtmp2[3] = { 4, 8, 7 };
	int tetfacelejtmp3[3] = { 4, 5, 6 };
	for (int i = 0; i < 3; i++)
	{
		tetFaceLocalEdgeMidnodeNum[0][i] = tetfacelejtmp0[i];
		tetFaceLocalEdgeMidnodeNum[1][i] = tetfacelejtmp1[i];
		tetFaceLocalEdgeMidnodeNum[2][i] = tetfacelejtmp2[i];
		tetFaceLocalEdgeMidnodeNum[3][i] = tetfacelejtmp3[i];
	}

	int wedgefacelejtmp0[4] = { 6, 7, 8, -1 };
	int wedgefacelejtmp1[4] = { 9, 10, 11, -1 };
	int wedgefacelejtmp2[4] = { 6, 13, 9, 12 };
	int wedgefacelejtmp3[4] = { 7, 14, 10, 13 };
	int wedgefacelejtmp4[4] = { 8, 14, 11, 12 };
	for (int i = 0; i < 4; i++)
	{
		wedgeFaceLocalEdgeMidnodeNum[0][i] = wedgefacelejtmp0[i];
		wedgeFaceLocalEdgeMidnodeNum[1][i] = wedgefacelejtmp1[i];
		wedgeFaceLocalEdgeMidnodeNum[2][i] = wedgefacelejtmp2[i];
		wedgeFaceLocalEdgeMidnodeNum[3][i] = wedgefacelejtmp3[i];
		wedgeFaceLocalEdgeMidnodeNum[4][i] = wedgefacelejtmp4[i];
	}
}

int SRmodel::GetElementLocalFacesLocalEdgeMidNodeNum(int lface, int lej, SRelementType type)
{
	if (type == brick)
		return brickFaceLocalEdgeMidnodeNum[lface][lej];
	else if (type == tet)
		return tetFaceLocalEdgeMidnodeNum[lface][lej];
	else if (type == wedge)
		return wedgeFaceLocalEdgeMidnodeNum[lface][lej];
	else
		return -1;
}

void SRmodel::SetBB()
{
	double x, y, z, xmin, xmax, ymin, ymax, zmin, zmax;
	//model bounding box:
	xmin = BIG;
	xmax = -BIG;
	ymin = BIG;
	ymax = -BIG;
	zmin = BIG;
	zmax = -BIG;
	for (int n = 0; n < GetNumNodes(); n++)
	{
		SRnode *node = GetNode(n);
		x = node->pos.d[0];
		y = node->pos.d[1];
		z = node->pos.d[2];
		if (x < xmin)
			xmin = x;
		if (x > xmax)
			xmax = x;
		if (y < ymin)
			ymin = y;
		if (y > ymax)
			ymax = y;
		if (z < zmin)
			zmin = z;
		if (z > zmax)
			zmax = z;
	}
	double dx, dy, dz;
	dx = xmax - xmin;
	dy = ymax - ymin;
	dz = zmax - zmin;
	size = sqrt(dx*dx + dy*dy + dz*dz);
}




