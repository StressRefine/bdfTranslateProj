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
// SRforce.h: interface for the SRforce class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(SRFORCE_INCLUDED)
#define SRFORCE_INCLUDED

class SRforce;
class SRvolumeForce;
class SRelement;
class SRvec3;

enum SRforceType { nodalForce, faceForce };
enum SRvolumeForceType { gravity, centrifugal };

class SRforce
{
	friend class SRoutput;
	friend class SRinput;

public:
	SRforceType GetType(){ return type; };
	bool isPressure(){ return pressure; };
	bool isGcs(){ return coordId == -1; };
	int GetCoordId(){ return coordId; };
	int GetEntityId(){ return entityId; };
	double GetForceVal(int i, int j){ return forceVals.Get(i, j); };
	void Copy(SRforce& that, bool copyForceVals = true);

	SRforce()
	{
		type = nodalForce; pressure = false; coordId = -1; numDuplicates = 1; constant = false;
		for (int n = 0; n < 4; n++)
			nv[n] = -1;
	};
	int uid;
	SRstring coordname;
private:
	SRforceType type;
	int coordId;
	int entityId; //node id, or element id for pressure on faces
	int nv[4]; //nodes at corners of face for pressure on faces
	int numDuplicates;
	SRdoubleMatrix forceVals;
	bool pressure;
	bool constant;
};


class SRthermalForce
{
	friend class SRoutput;
	friend class SRinput;
public:
	double GetTemp(SRelement* elem, double r, double s, double t);
	bool CeTMult(SRmaterial* mat, double eTx, double eTy, double eTz, double ceT[]);
	void Process();
private:
	bool constantTemp;
	double temp;
};

class SRvolumeForce
{
	friend class SRoutput;
	friend class SRinput;

public:
	SRvolumeForce(){ g1 = g2 = g3 = 0.0; omega = 0.0; alpha = 0.0; }
	void GetForceValue(SRelement* elem, SRvec3& p, double val[]);
	SRvolumeForceType GetType(){ return type; };

private:
	SRvolumeForceType type;
	//for gravity loads:
	double g1, g2, g3;
	//for centrifugal loads:
	double omega;
	double alpha;
	SRvec3 axis;
	SRvec3 origin;
};

#endif // !defined(SRFORCE_INCLUDED)
