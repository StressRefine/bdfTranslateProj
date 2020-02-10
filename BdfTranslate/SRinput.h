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
// SRinput.h: interface for the SRinput class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(SRINPUT_INCLUDED)
#define SRINPUT_INCLUDED

#include "SRfile.h"
#include "SRstring.h"

struct SRuidData
{
	int id;
	int uid;
};

class SRinput  
{
public:
	SRinput();
	bool Translate();
	void SortOtherEntities();
	void SortNodes();
	void SortElements();
	int GetCoordId(SRstring& name);
	int GetMaterialId(SRstring& name);
	void InputConstraint(SRstring& line);
	void InputCoordinate(SRstring& line);
	void InputVolumeForce(SRstring& line);
	void InputThermal(SRstring& line);
	void InputForce(SRstring& line);
	void InputElementProperty(SRstring& line);
	void InputMaterial(SRstring& line, bool matNameWasRead, SRstring& matname);
	void InputElement(SRstring& line, int& numFaces);
	void InputNode(SRstring& line);
	void InputEnfd(SRstring& line);
	void finishUnsup();
	void cropElements();
	void finishForces();
	void finishConstraints();
	int NodeFind(int uid);
	int CoordFind(int uid);
	int MatFind(int uid);
	int ElpropFind(int uid);
	int ElemFind(int uid);
	int findElemFace(bool needMidSide, SRelement* elem, int g1, int g2, int gout[8], double *fv = NULL);
	void CountEntities(int &num);
	void checkLcs(SRconstraint* con, int cid);
	void SetNodeElmentOwners();
	void inputUnsupported(SRstring &line);
	int findFaceNodes(SRelement* elem, int gidv[], int& nread, int gidFace[]);
	bool IsWedgeFaceQuad(SRelement* elem, int gid[]);
	void checkUnsupportedTouchesNonOrphan();

	//BDF Specific:
	void TopToBulk();
	bool BdfInput();

	SRvector <SRuidData> nodeUids;
	SRvector <SRuidData> coordUids;
	SRvector <SRuidData> matUids;
	SRvector <SRuidData> elpropUids;
	SRvector <SRuidData> elemUids;
	int nodeUidOffset;
	int CoordUidOffset;
	int MatUidOffset;
	int elPropUidOffset;
	int elemUidOffSet;
	int lastNodeUid;
	int lastNodeId;
	int lastCoordUid;
	int lastCoordId;
	int lastMatUid;
	int lastMatId;
	int lastElPropUid;
	int lastElPropId;
	int lastElemUid;
	int lastElemId;
	int nnode;
	int nelem;
//bdf specific
	bool anyCoordsReferenceGrids;
};

#endif // !defined(SRINPUT_INCLUDED)
