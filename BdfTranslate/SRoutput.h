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
// SRoutput.h: interface for the SRoutput class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(SROUTPUT_INCLUDED)
#define SROUTPUT_INCLUDED

#include "SRfile.h"
#include "SRstring.h"

class SRdoubleMatrix;
class SRintVector;

class SRoutput  
{
public:
	void DoOutput();
	void OutputNodes();
	void OutputElements();
	void OutputConstraints();
	void OutputForces();
	void OutputVolumeForces();
	void OutputThermalForce();
	void OutputMaterials();
	void OutputCoordinates();
	void printNodalForce(int nodeUid, SRforce* force, SRfile&f);
	SRdoubleMatrix nodalStress;
	SRintVector nodeCount;
	double svmmax;
	int maxsvmnodeid;
	SRintVector matMaxnodeids;
	SRdoubleVector matSvmMax;



};

#endif // !defined(SROUTPUT_INCLUDED)
