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
// SRconstraint.h: interface for the SRconstraint class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(SRCONSTRAINT_INCLUDED)
#define SRCONSTRAINT_INCLUDED

enum SRconstraintType{ nodalCon, faceCon, inactive};

class SRcoord;
class SRintVector;

class SRenfd
{
public:
	SRenfd()
	{
		for (int i = 0; i < 3; i++)
			condof[i] = false;
	}
	int nuid;
	bool condof[3];
	double enfdVal;
};

class SRunsup
{
public:
	SRunsup(){ isShellOrBeam = false; isBsurf = false; };
	friend class SRinput;
	bool isShellOrBeam;
	bool isBsurf;
	SRintVector gids;
};

class SRconstraint
{

	friend class SRoutput;
	friend class SRinput;

public:
	void ProcessFaceConstraint();
	void FillFaceEnforcedDispCoeffs(int dof, double *dispvec);
	bool IsConstrainedDof(int i){ return constrainedDof[i] != 0; };
	int GetEntityId(){ return entityId; };
	SRconstraintType GetType(){ return type; };
	SRcoord* GetCoord();
	bool hasEnforcedDisp(){	return !enforcedDisplacementData.isEmpty();	};
	int GetCoordId(){ return coordId; };
	bool isGcs(){ return (coordId == -1); };
	void Copy(SRconstraint& that);
	void PlusAssign(SRconstraint& that);
	void operator =(SRconstraint& c2){ Copy(c2); };
	double getDisp(int n, int d)
	{
		if (!enforcedDisplacementData.isEmpty())
			return enforcedDisplacementData.Get(n, d);
		else
			return 0.0;
	};
	void allocateEnforcedDisplacementData(int n){ enforcedDisplacementData.Allocate(n, 3); };
	void PutEnforcedDisplacementData(int n, int dof, double val);
	double GetEnforcedDisp(int nodeNum, int dof);
	int GetNumEnforcedDisp(){ return enforcedDisplacementData.getNumCols(); };
	void Clear();
	bool isBreakout(){ return (breakoutElemUid != -1); }

	SRconstraint();

	int uid;
	SRstring coordname;
	int constrainedDof[3];
	int entityId; //node or face
	SRconstraintType type;
	int coordId;
	SRdoubleMatrix enforcedDisplacementData;
	int breakoutElemUid;
};


#endif // !defined(SRCONSTRAINT_INCLUDED)
