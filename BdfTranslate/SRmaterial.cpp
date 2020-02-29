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
// SRmaterial.cpp: implementation of the SRmaterial class.
//
//////////////////////////////////////////////////////////////////////

#include "SRutil.h"
#include "SRmodel.h"

void SRmaterial::IsoCreate(double et, double nut)
{
	//create material of type iso
	//input:
		//et = Young's modulus
		//nut = Poisson's ratio
	//note:
		//fills class variables E, nu, G, lambda, abd c11
	type = iso;
	E = et;
	nu = nut;
	G = E / (2.0*(1.0 + nu));
	lambda = 2.0*G*nu / (1.0 - 2.0*nu);
	c11 = lambda + 2.0*G;
};

double SRmaterial::MatScale()
{
	//determine a characteristic scale for this material
	//return:
		//scale = Young's modulus for isotropic element or equivalent value for anisotropic
	if (type == iso)
		return E;
	else if (type == ortho)
	{
		double pseudolam, pseudoG, PseudoGnu, lg;
		SRcij& cij = orthoCij;
		pseudolam = ONETHIRD*(cij.c12 + cij.c13 + cij.c23);
		pseudoG = ONETHIRD*(cij.c44 + cij.c55 + cij.c66);
		lg = pseudolam / pseudoG;
		PseudoGnu = 0.5*lg / (1.0 + lg);
		return pseudoG*2.0*(1.0 + PseudoGnu);
	}
	else if (type == genAniso)
	{
		double pseudolam, pseudoG, PseudoGnu, lg;
		SRgenAnisoCij& gcij = genAnisoCij;
		pseudolam = (gcij.c[0][1] + gcij.c[0][2] + gcij.c[1][2]);
		pseudoG = ONETHIRD*(gcij.c[3][3] + gcij.c[4][4] + gcij.c[5][5]);
		lg = pseudolam / pseudoG;
		PseudoGnu = 0.5*lg / (1.0 + lg);
		return pseudoG*2.0*(1.0 + PseudoGnu);
	}
	ERROREXIT;
	return 0;
}

void SRmaterial::printToFile(SRfile &f)
{
	f.Print(" %s", name.getStr());
	if (type == iso)
	{
		f.Print(" isotropic");
		f.PrintReturn();
		f.PrintLine(" %lg %lg //rho, alpha", rho, alphax);
		f.PrintLine(" %lg %lg //E, nu", E, nu);
	}
	else if (type == ortho)
	{
		f.Print(" orthotropic");
		f.PrintReturn();
		f.PrintLine(" %lg %lg %lg %lg //rho, alphax alphay alphaz", rho, alphax, alphay, alphaz);
		f.PrintLine(" %lg %lg %lg %lg %lg %lg %lg %lg %lg //c11,c12,c13,c22,c23,c33,c44,c55,c6",
			orthoCij.c11, orthoCij.c12, orthoCij.c13,
			orthoCij.c22, orthoCij.c23, orthoCij.c33,
			orthoCij.c44, orthoCij.c55, orthoCij.c66);
	}
	else if (type == ortho)
	{
		f.Print(" general");
		f.PrintReturn();
		f.PrintLine(" %lg %lg %lg %lg //rho, alphax alphay alphaz", rho, alphax, alphay, alphaz);
		for (int row = 0; row < 6; row++)
		{
			for (int col = 0; col < 6; col++)
				f.Print(" %lg", genAnisoCij.c[row][col]);
			f.PrintReturn();
		}
	}
}



bool SRgenAnisoCij::symCheck()
{
	//check if this material is symmetric
    for(int i = 0; i < 6; i++)
    {
        for(int j = 0; j < i; j++)
        {
            double diff = fabs(c[i][j] - c[j][i]);
            if(diff > TINY)
                return false;
        }
    }
    return true;
}