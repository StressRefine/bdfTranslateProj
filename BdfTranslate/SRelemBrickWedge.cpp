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
// SRelementBrickWedge.cpp: implementation of the SRelement class
//	for bricks and wedges
//
//////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "SRmodel.h"

extern SRmodel model;


void SRelement::GetWedgeFaceNodes(int lface, int &n1, int &n2, int &n3, int &n4)
{
	//get the global numbers for a local face of a wedge
	//input:
		//lface = local face number
	//output:
		//n1, n2, n3, n4 = node numbers
        //n4 == -1 for triangular face

	if (lface == 0)
	{
		//local face 1 = 1-2-3
		n1 = nodeUIds.Get(0);
		n2 = nodeUIds.Get(1);
		n3 = nodeUIds.Get(2);
		n4 = -1;
	}
	else if (lface == 1)
	{
		//local face 2 = 4-5-6
		n1 = nodeUIds.Get(3);
		n2 = nodeUIds.Get(4);
		n3 = nodeUIds.Get(5);
		n4 = -1;
	}
	else if (lface == 2)
	{
		//local face 3 = 1-2-5-4
		n1 = nodeUIds.Get(0);
		n2 = nodeUIds.Get(1);
		n3 = nodeUIds.Get(4);
		n4 = nodeUIds.Get(3);
	}
	else if (lface == 3)
	{
		//local face 4 = 2-3-6-5
		n1 = nodeUIds.Get(1);
		n2 = nodeUIds.Get(2);
		n3 = nodeUIds.Get(5);
		n4 = nodeUIds.Get(4);
	}
	else if (lface == 4)
	{
		//local face 5 = 1-3-6-4
		n1 = nodeUIds.Get(0);
		n2 = nodeUIds.Get(2);
		n3 = nodeUIds.Get(5);
		n4 = nodeUIds.Get(3);
	}
}

void SRelement::GetBrickFaceNodes(int lface, int &n1, int &n2, int &n3, int &n4)
{
	//get the global numbers for a local face of a brick
	//input:
		//lface  = local face number
	//output:
		//n1, n2, n3, n4 = node numbers
	if (lface == 0)
	{
		//local face 1 = 1-2-3-4
		n1 = nodeUIds.Get(0);
		n2 = nodeUIds.Get(1);
		n3 = nodeUIds.Get(2);
		n4 = nodeUIds.Get(3);
	}
	else if (lface == 1)
	{
		//local face 2 = 5-6-7-8
		n1 = nodeUIds.Get(4);
		n2 = nodeUIds.Get(5);
		n3 = nodeUIds.Get(6);
		n4 = nodeUIds.Get(7);
	}
	else if (lface == 2)
	{
		//local face 3 = 1-4-8-5
		n1 = nodeUIds.Get(0);
		n2 = nodeUIds.Get(3);
		n3 = nodeUIds.Get(7);
		n4 = nodeUIds.Get(4);
	}
	else if (lface == 3)
	{
		//local face 4 = 2-3-7-6
		n1 = nodeUIds.Get(1);
		n2 = nodeUIds.Get(2);
		n3 = nodeUIds.Get(6);
		n4 = nodeUIds.Get(5);
	}
	else if (lface == 4)
	{
		//local face 5 = 1-2-6-5
		n1 = nodeUIds.Get(0);
		n2 = nodeUIds.Get(1);
		n3 = nodeUIds.Get(5);
		n4 = nodeUIds.Get(4);
	}
	else if (lface == 5)
	{
		//local face 6 = 4-3-7-8
		n1 = nodeUIds.Get(3);
		n2 = nodeUIds.Get(2);
		n3 = nodeUIds.Get(6);
		n4 = nodeUIds.Get(7);
	}
}
