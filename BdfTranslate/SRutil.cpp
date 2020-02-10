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
// SRutil.cpp: implementation of the SRutil class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SRmodel.h"

extern SRmodel model;

void SRutil::SRAssert(char *file, int line, bool expn)
{
#ifdef _DEBUG
	if (!expn)
		SRmodel::ErrorExit(file, line);
#endif
}

void SRutil::TimeStamp(bool debugOnly)
{
#ifndef _DEBUG
	if(debugOnly)
		return;
#endif
	SRstring line;
	SRmachDep::GetTime(line);
	OUTPRINT(line.str);
	OUTPRINT();
}

void SRutil::TimeStampToScreen()
{
	SRstring line;
	SRmachDep::GetTime(line);
	SCREENPRINT(line.str);
}

void SRintVector::PushBack(int v)
{
	SRintVector tmp;
	tmp.Copy(*this);
	num++;
	Allocate(num);
	for (int i = 0; i < tmp.GetNum(); i++)
		d[i] = tmp.d[i];
	d[num - 1] = v;
};



void SRdoubleVector::PushBack(double v)
{
	SRdoubleVector tmp;
	tmp.Copy(*this);
	num++;
	Allocate(num);
	for (int i = 0; i < tmp.GetNum(); i++)
		d[i] = tmp.d[i];
	d[num - 1] = v;
};

void SRdoubleMatrix::PlusAssign(SRdoubleMatrix& that)
{
	if (n==0)
		Allocate(that.n, that.m);
	else
	{
		if (n != that.n || m != that.m)
			ERROREXIT;
	}
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < m; j++)
			d[i][j] += that.d[i][j];
	}
}

void SRdoubleMatrix::Copy(SRdoubleMatrix& that)
{
	Allocate(that.n, that.m);
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < m; j++)
			d[i][j] = that.d[i][j];
	}
}

int SRIntCompareFunc(const void *v1, const void *v2)
{
	//compare function for sorting SRintVector
	return *((int *)v1) - *((int *)v2);
}

int SRintVector::Find(int intIn)
{
	//find an integer in this vector using binary search
	//input:
	//intIn = integer value
	//return:
	//location where intIn resides in this vector, -1 if not found
	//note:
	//Sort must be called before using this routine

	int* intOutPtr = (int *)bsearch(&intIn, d, num, sizeof(int), SRIntCompareFunc);
	if (intOutPtr == NULL)
		return -1;
	else
		return intOutPtr - d;
}

void SRintVector::Sort()
{
	//binary sort this vector
	qsort((void *)d, num, sizeof(int), SRIntCompareFunc);
}
