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
// SRmodel.h: interface for the SRmodel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(SRMODEL_INCLUDED)
#define SRMODEL_INCLUDED

#define MAXNMAT 100
#define MAXNCOORD 100
#define MAXNNODE 1000000
#define MAXNEL 1000000
#define MAXBREAKOUTS 100

enum SRstressComponent { xxComponent, yyComponent, zzComponent, xyComponent, xzComponent, yzComponent };

#include "SRmachDep.h"
#include "SRutil.h"
#include "SRmath.h"
#include "SRstring.h"
#include "SRfile.h"
#include "SRcoord.h"
#include "SRmaterial.h"
#include "SRconstraint.h"
#include "SRforce.h"
#include "SRnode.h"
#include "SRelement.h"
#include "SRinput.h"
#include "SRoutput.h"


#define ERROREXIT SRmodel::ErrorExit(__FILE__,__LINE__)

class SRmodel
{
	friend class SRoutput;
	friend class SRinput;


public:
	SRmodel();

	static void ErrorExit(const char* file, int line);

	void CleanUp(bool partial = false);

	void FindElemsAdjacentToBreakout();
	bool checkOrphanNode(int uid)
	{
		SRnode *node = GetNodeFromUid(uid);
		if (node == NULL)
			return true;
		else
			return node->isOrphan();
	};
	int GetNodeUid(int i){ return nodes.GetPointer(i)->userId; };

	void SetBB();

	int GetNumNodes(){ return nodes.GetNum(); };
	SRnode* GetNode(int i){ return nodes.GetPointer(i); };
	SRnode* GetNodeFromUid(int i)
	{
		int nid = input.NodeFind(i);
		if (nid < 0)
			return NULL;
		else
			return nodes.GetPointer(nid);
	};

	int GetNumElements() { return elements.GetNum(); };
	SRelement* GetElement(int i){ return elements.GetPointer(i); };

	int GetNumMaterials() { return materials.GetNum(); };
	SRmaterial* GetMaterial(int i){ return materials.GetPointer(i); };

	int GetNumConstraints(){ return constraints.GetNum(); };
	SRconstraint* GetConstraint(int i){ return constraints.GetPointer(i); };

	int GetNumForces(){ return forces.GetNum(); };
	SRforce* GetForce(int i){ return forces.GetPointer(i); };

	SRcoord* GetCoord(int i){ return Coords.GetPointer(i); };

	SRthermalForce* GetThermalForce() { return thermalForce; };

	void mapSetup();
	int GetElementLocalFacesLocalEdgeMidNodeNum(int lface, int lej, SRelementType type);

	//"read" routines for private data:
	double GetSize(){ return size; };
	bool IsAnyEnforcedDisplacement(){ return anyEnforcedDisplacement; };
	bool isNodeToFaceBCs() { return nodeToFaceBCs; };
	void SetNodeToFaceBCs(bool tf){ nodeToFaceBCs = tf; };

	//these should be set during input or from SRmodel routines and only accessed readonly elsewhere through query functions
	double size;
	bool anybricks;
	bool anywedges;
	bool anyEnforcedDisplacement;
	bool nodeToFaceBCs;
	int numFunctions;
	int maxNumElementFunctions;
	int numactiveMat;
	bool cropModelWithDispNodes;
	bool partialDispFile;

	SRpointerVector <SRnode> nodes;
	SRpointerVector <SRconstraint> constraints;
	SRpointerVector <SRenfd> enfds;
	SRpointerVector <SRunsup> unsups;
	SRpointerVector <SRcoord> Coords;
	SRpointerVector <SRmaterial> materials;
	SRpointerVector <SRElProperty> elProps;
	SRpointerVector <SRforce> forces;
	SRpointerVector <SRelement> elements;
	SRpointerVector <SRvolumeForce> volumeForces;
	SRthermalForce* thermalForce;

	//1 instance of each utility class:
	SRmath math;
	SRinput input;
	SRoutput output;

	SRstring wkdir;
	SRstring outdir;
	char stringBuffer[MAXLINELENGTH];
	SRfile outputFile;
	SRfile inpFile;
	SRfile logFile;
	SRfile mshFile;
	SRfile statFile;
	SRfile srrFile;
	SRfile nodeDispFile;
	SRstring fileNameTail;
	SRstring SrFileNameTail;
	int brickFaceLocalNodes[6][8];
	int tetFaceLocalNodes[4][6];
	int wedgeFaceLocalNodes[5][8];
	SRintVector breakoutElems;
	int numbreakoutElems;
	bool linearMesh;
	bool anyUnsupportedElement;
	int numnodalforces;
	int tetFaceLocalEdgeMidnodeNum[4][3];
	int brickFaceLocalEdgeMidnodeNum[6][4];
	int wedgeFaceLocalEdgeMidnodeNum[5][4];
	bool anyGeneralUnsupportedNode;
	bool anyShellOrBeamNode;
	bool isNx;
};

#endif //!defined(SRMODEL_INCLUDED)
