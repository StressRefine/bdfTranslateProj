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
// SRmachDep.h: interface for the SRmachDep class.
// MACHINE DEPENDENT FUNCTIONS
//
//////////////////////////////////////////////////////////////////////

#if !defined(SRMACHDEP_INCLUDED)
#define SRMACHDEP_INCLUDED

//MACHINE Dependency flags:

//Windows: ifdef _WINDOWS
#define SRBUFSIZE 8192 //ttd tune me

enum dlgfilemode{newmode,openmode,saveasmode};

#ifdef _WINDOWS
#endif

class SRstring;
class SRmachDep  
{
public:
	static void GetTime(SRstring &t);
	static bool CreateDir(char* name);
};

#endif // !defined(SRMACHDEP_INCLUDED)
