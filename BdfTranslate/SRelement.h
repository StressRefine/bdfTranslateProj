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
// SRelement.h: interface for the SRelement class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(SRELEMENT_INCLUDED)
#define SRELEMENT_INCLUDED

enum SRelementType { tet, wedge, brick, undefined };

class SRelement
{
	friend class SRoutput;
	friend class SRinput;
	friend class SRmap;
public:
	void GetBrickFaceNodes(int lface, int& n1, int& n2, int& n3, int& n4);
	void GetWedgeFaceNodes(int lface, int& n1, int& n2, int& n3, int& n4);
	SRnode* GetNode(int localnodenum);
	int GetFaceNodes(bool needMidside, int lface, int n[]);
	void GetFaceNodes(int lface, int& n1, int& n2, int& n3, int& n4);
	void Create(SRelementType typet, int userid, int nnodes, int nodest[]);
	int GetUserid(){ return uid; };
	int GetId(){ return id; };
	SRelementType GetType(){ return type; };
	int GetNumNodes(){ return nodeUIds.GetNum(); };
	int GetNodeId(int i);
	int GetNumLocalFaces();
	int GetMaterialId(){ return matid; };
	void Cleanup();
	bool nodeDistCheck(SRvec3& pos, double radius);
	bool InsideBoundingBox(double xmin, double xmax, double ymin, double ymax, double zmin, double zmax);
	void SetNodeElmentOwners();
	int GetNumCorners();

	SRelement(){ type = tet; saveForBreakout = false; };
	~SRelement(){ Cleanup(); };
	int uid;
	int id;
	SRelementType type;
	SRintVector nodeUIds;
	SRstring matname;
	int matid;
	double size;
	int numNodes;
	bool saveForBreakout;
};

#endif //!defined(SRELEMENT_INCLUDED)
