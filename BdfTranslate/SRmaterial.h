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
// SRmaterial.h: interface for the SRmaterial class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(SRMATERIAL_INCLUDED)
#define SRMATERIAL_INCLUDED

enum SRmaterialType {iso,ortho,genAniso};

struct SRcij
{
	double c11;
	double c12;
	double c13;
	double c22;
	double c23;
	double c33;
	double c44;
	double c55;
	double c66;
};

class SRgenAnisoCij
{
public:
	double c[6][6];
    bool symCheck();
};
class SRstring;

class SRmaterial
{

	friend class SRoutput;
	friend class SRinput;

	friend class SRelement;
	friend class SRthermalForce;

public:
	//functions:
	void IsoCreate(double et, double nut);
	const char* GetName(){ return name.getStr(); };
	SRmaterialType GetType(){ return type; };
	double GetRho(){ return rho; };
	double MatScale();
	void printToFile(SRfile &f);

	SRmaterial(){ type = iso; E = 0.0; nu = 0.0; rho = 0.0; active = false; };
	~SRmaterial(){};

	int id;
	int uid;
	bool active;
	SRstring name;
	SRmaterialType type;
	//density:
	double rho;
	//coefficient of thermal expansion:
	double alphax;
	double alphay;
	double alphaz;
	//elastic properties:
	double E;
	double nu;
	double lambda;
	double G;
	double c11; //=lambda+2G for iso
	SRcij orthoCij;
	SRgenAnisoCij genAnisoCij;
	double tref;
	double allowableStress;
};

class SRElProperty
{
public:
	int uid;
	int matid;
	int matuid;
};

#endif //!defined(SRMATERIAL_INCLUDED)
