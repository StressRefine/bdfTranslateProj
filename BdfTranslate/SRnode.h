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
// SRnode.h: interface for the SRnode class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(SRNODE_INCLUDED)
#define SRNODE_INCLUDED

class SRvec3;
class SRoutputData;

class SRnode  
{
	friend class SRoutput;
	friend class SRinput;


public:
	SRnode();
	void Copy(SRnode& n2);
	void Create(int useridt, double xt, double yt, double zt){ userId = useridt; pos.Assign(xt, yt, zt); };
	int GetUserid(){ return userId; };
	void SetUserid(int id){ userId = id; };
	SRvec3& Position(){ return pos; };
	double GetXyz(int i){ return pos.d[i]; };
	bool isOrphan() { return (firstElementOwner == -1); };
	int GetFirstElementOwner() { return firstElementOwner; };
	SRforce* GetForce();
	SRconstraint* GetConstraint();
	void SetConstraintId(int i){ constraintId = i; };

	bool unSupported;
	bool shellOrBeamNode;
	bool bsurf;
	int firstElementOwner;
	int userId; //user original nodes numbers in case non-contiguous
	SRvec3 pos;
	SRvec3 disp;
	int constraintId;
	int forceId;
	bool hasTemp;
	double Temp;
	int dispCoordid;
	bool ismidside;
	bool hasDisp;
};

#endif //!defined(SRNODE_INCLUDED)
