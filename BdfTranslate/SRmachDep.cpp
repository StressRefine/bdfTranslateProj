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
// SRmachDep.cpp: implementation of the SRmachDep class.
//                Machine Dependent Functions
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <direct.h>
#include <time.h>
#include "SRString.h"
#include "SRmodel.h"
#ifdef _WINDOWS
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

static char buf[256];

bool SRmachDep::CreateDir(char *name)
{
	//create a directory with full path "name"
	//ttd: verify this is portable to non-windows
	int i = _mkdir(name);
	if (i == 0)
		return true;
	else
		return false;
	return false;
}


void SRmachDep::GetTime(SRstring &t)
{
	//return date and time in a string
	//note: this should be portable- time and ctime are standard c functions
	//put it in ifdef if problems on other platforms

	time_t ltime;
	time( &ltime );
	char buf[256];
	ctime_s(buf, 256, &ltime);
	t = buf;
#if 0
	//example:
	//Fri Apr 29 12:25:12 2001
	line.Token(); //skip day of week;
	line.Token(); //skip month;
	line.Token(); //skip day;
	t = line.Token();
#endif
}
