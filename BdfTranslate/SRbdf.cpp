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
// SRbdf.cpp: implementation of the SRpostProcess routines specific to Nastran: f06 displacement input
//
//////////////////////////////////////////////////////////////////////

#include <search.h>
#include "SRmodel.h"
#include <chrono>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern SRmodel model;

bool SRinput::BdfInput()
{

	//faster version- minimizes passes through bdf file
	//1 pass to count, 1 pass to read everything. do not crop inside e.g. inputelement.
	//make an int array for unsup and shell. int unsupdnodes. store uid and type
	//after read everything, crop --> pack nodes
	//then process elements and pack, setnodeelementowners.
	//then process constraints and forces and pack.

	anyCoordsReferenceGrids = false;

	//count entities: nodes, elements, constraints, forces:
	//input coords, mats, and elprops because they need to be sorted before input elements and input nodes
	nnode = nelem = 0;
	int nforce = 0, nvol = 0, ntherm = 0, ncon = 0, nspcd = 0, numunsup = 0;
	SRstring tok;
	SRstring filename, line, basename, tail;

	TopToBulk();

	bool isComment = false;
	bool isMat = false;
	bool matNameWasRead = false;
	SRstring matname;

	//first pass through bdf file. read mats, elprops, and coordinates, count everything else.
	int linesRead = 0;
	while (1)
	{
		bool ret = model.inpFile.GetBdfLine(line, isComment, isMat, tok);
		linesRead++;
		if (isComment && linesRead < 10)
		{
			SRstring rtStr;
			if (line.LastChar(':')!= NULL)
			{
				line.Right(':', rtStr);
				tok = rtStr.Token();
				if (tok.CompareUseLength("Femap"))
					model.isNx = true;
			}
		}
		if (isMat)
		{
			matNameWasRead = true;
			matname = tok;
		}
		if (!ret)
			break;

		if (isComment)
			continue;
		else if (line.CompareUseLength("GRID"))
			nnode++;
		else if (line.CompareUseLength("CHEXA") || line.CompareUseLength("CPENTA") || line.CompareUseLength("CTETRA"))
		{
			if (nelem == 0)
			{
				//check for linear mesh:
				tok = line.BdfToken();//skip element label field (e.g chexa)
				tok = line.BdfToken();//skip eluid field
				tok = line.BdfToken();//skip elpropid field
				int nnodes = 0;
				int nuid;
				while (1)
				{
					if (!line.BdfRead(nuid))
						break;
					nnodes++;
				}
				if (nnodes != 10 && nnodes != 15 && nnodes != 20)
					model.linearMesh = true;
			}
			nelem++;
		}
		else if (line.CompareUseLength("FORCE"))
			nforce++;
		else if (line.CompareUseLength("PLOAD4"))
			nforce++;
		else if (line.CompareUseLength("GRAV") || line.CompareUseLength("RFORCE"))
			nvol++;
		else if (line.CompareUseLength("TEMP"))
			ntherm++;
		else if (line.CompareUseLength("SPCD"))
			nspcd++;
		else if (line.CompareUseLength("SPC1") || line.CompareUseLength("SPC"))
			ncon++;
		else if (line.CompareUseLength("CORD"))
			InputCoordinate(line);
		else if (line.CompareUseLength("MAT1"))
			InputMaterial(line, matNameWasRead, matname);
		else if (line.CompareUseLength("PSOLID"))
			InputElementProperty(line);
		else if (line.CompareUseLength("CTRI") || line.CompareUseLength("CQUAD") ||
			line.CompareUseLength("CB") || line.CompareUseLength("CR") || line.CompareUseLength("CS") ||
			line.CompareUseLength("CW") || line.CompareUseLength("CF") || line.CompareUseLength("Ci") ||
			line.CompareUseLength("RB") || line.CompareUseLength("RJ") || line.CompareUseLength("CELAS") ||
			line.CompareUseLength("MPC") || line.CompareUseLength("SUPORT") || line.CompareUseLength("BSURFS"))

		{
			numunsup++;
		}
	}

	SortOtherEntities();

	model.nodes.Allocate(nnode);
	//note: can't be sure there are not constraints on the grid cards, so conservatively add nnode to ncon:
	ncon += nnode;
	model.constraints.Allocate(ncon);
	model.enfds.Allocate(nspcd);
	model.elements.Allocate(nelem);
	model.forces.Allocate(nforce);
	model.volumeForces.Allocate(nvol);
	if (ntherm != 0)
		model.thermalForce = new SRthermalForce;
	model.unsups.Allocate(numunsup);

	//2nd pass through bdf file. read everything else.

	TopToBulk();

	int nline = 0;
	int numFaces = 0;
	while (1)
	{
		if (!model.inpFile.GetBdfLine(line, isComment, matNameWasRead, matname))
			break;
		if (isComment)
			continue;
		if (line.CompareUseLength("GRID"))
			InputNode(line);
		else if (line.CompareUseLength("CHEXA") || line.CompareUseLength("CPENTA") || line.CompareUseLength("CTETRA"))
			InputElement(line, numFaces);
		else if (line.CompareUseLength("FORCE") || line.CompareUseLength("PLOAD4") ||
			line.CompareUseLength("FORCE1") || line.CompareUseLength("FORCE2"))
			InputForce(line);
		if (line.CompareUseLength("SPCD"))
			InputEnfd(line);
		else if (line.CompareUseLength("SPC1") || line.CompareUseLength("SPC"))
			InputConstraint(line);
		else if (line.CompareUseLength("GRAV") || line.CompareUseLength("RFORCE"))
			InputVolumeForce(line);
		else if (line.CompareUseLength("TEMP"))
			InputThermal(line);
		else if (line.CompareUseLength("CTRI") || line.CompareUseLength("CQUAD") ||
			line.CompareUseLength("CB") || line.CompareUseLength("CR") || line.CompareUseLength("CS") ||
			line.CompareUseLength("CW") || line.CompareUseLength("CF") || line.CompareUseLength("Ci") ||
			line.CompareUseLength("RB") || line.CompareUseLength("RJ") || line.CompareUseLength("CELAS") ||
			line.CompareUseLength("MPC") || line.CompareUseLength("SUPORT") || line.CompareUseLength("BSURFS"))

		{
			inputUnsupported(line);
			model.anyUnsupportedElement = true;
		}
		nline++;
	}

	int numNodeDispsRead = 0;
	if (model.cropModelWithDispNodes)
	{
		SortNodes();
		//unsupported entities refer to nodes and element ids; finish filling them
		//for cropped model this has to be done now, because it will throw bsurf processing off
		SortElements();
		finishUnsup();

		//read the disp file to get the node ids that have disps. mark the nodes "havedisps".
		//Also output the .srs file so engine can read the disps.
		model.srrFile.Open(SRoutputMode);
		model.srrFile.PrintLine("displacements");
		model.nodeDispFile.Open(SRinputMode);
		model.nodeDispFile.GetLine(line); //skip header
		int uid;
		line.setTokSep(',');
		SRstring linesav;

		while (1)
		{
			if (!model.nodeDispFile.GetLine(line))
				break;
			linesav = line;
			if (!line.TokRead(uid))
				break;
			int nid = model.input.NodeFind(uid);
			if (nid == -1 || nid > nnode)
			{
				//possibly a node on unsupported entity e.g. shell, just skip it:
				continue;
			}
			model.srrFile.PrintLine(linesav.getStr());
			SRnode* node = model.GetNode(nid);
			node->hasDisp = true;
			numNodeDispsRead++;
		}
		if (numNodeDispsRead < nnode)
			model.partialDispFile = true;
		model.nodeDispFile.Close();

		//now can crop elements, only keep those for which at least one node has disp:
		cropElements();
		elemUidOffSet = -1;
		//packing elements will mess up elemUidOffSet and throw off searches.
		//just set elemUidOffSet to -1 to force
		//bsearch:
		SortElements();
		//fix mat active flags in case all elements removed that refer to a material:
		for (int m = 0; m < model.GetNumMaterials(); m++)
			model.GetMaterial(m)->active = false;
		for (int e = 0; e < model.GetNumElements(); e++)
		{
			int mid = model.GetElement(e)->matid;
			model.GetMaterial(mid)->active = true;
		}

		SetNodeElmentOwners();
		//delete orphan nodes, pack nodes and resort:
		int numNodesTotal = model.GetNumNodes();
		int numfreed = 0;
		for (int n = 0; n < numNodesTotal; n++)
		{
			if (model.GetNode(n)->isOrphan())
			{
				model.nodes.Free(n);
				numfreed++;
			}
		}
		if (numfreed > 0)
		{
			model.nodes.packNulls();
			//packing nodes will mess up nodeuidoffset and throw off searches.
			//just set nodeUidOffset to -1 to force
			//bsearch:
			nodeUidOffset = -1;
			SortNodes();
		}
	}
	else
	{
		SortNodes();
		SortElements();
		SetNodeElmentOwners();
		//unsupported entities refer to nodes and element ids; finish filling them:
		finishUnsup();
	}




	//forces, and constraints refer to nodes and element ids; finish filling them:
	finishForces();
	finishConstraints();

	model.inpFile.Close();

	return true;
}

void SRinput::TopToBulk()
{
	model.inpFile.ToTop();
	SRstring line, tok;
	//skip lines until "Begin Bulk"
	while (1)
	{
		if (!model.inpFile.GetLine(line))
			ERROREXIT;
		tok = line.Token();
		if (tok == "BEGIN")
		{
			tok = line.Token();
			if (tok == "BULK")
				break;
		}
	}
}

void SRinput::CountEntities(int &num)
{
	//count the entities currently being read in input file by counting 
	//lines (except blank or comment) until end is encountered

	SRstring line;
	bool isComment = false;
	bool matNameWasRead;
	SRstring matname;
	while (1)
	{
		if (!model.inpFile.GetBdfLine(line, isComment, matNameWasRead, matname))
			break;
		if (isComment)
			continue;
		if (line == "end")
			break;
		num++;
	}
}

void SRinput::InputNode(SRstring& line)
{
	//input nodes
	//spec:
	//GRID, ID, CoordId, x, y, z, outcoordid, constrained-dofs = 1 for x, 2 for y, 3 for z, 12 for x and y, omitted for unconstrained
	// CoordId omitted for gcs

	SRstring tok;
	int id = model.GetNumNodes();
	SRnode* node = model.nodes.Add();
	//skip "GRID" token
	line.BdfToken();
	SRstring condofs;
	int uid, coordid, dispCoorduid;
	double x, y, z;
	line.BdfRead(uid);
	if (id == 0)
		nodeUidOffset = uid;
	else if (nodeUidOffset != -1)
	{
		if (uid - nodeUidOffset != id)
			nodeUidOffset = -1;
	}
	line.BdfRead(coordid);
	line.BdfRead(x);
	line.BdfRead(y);
	line.BdfRead(z);
	line.BdfRead(dispCoorduid);
	condofs = line.BdfToken(false);
	if (coordid > 0)
	{
		if (anyCoordsReferenceGrids)
		{
			//rktd uncomment if need to support coord input that references grids, e.g. CORD1C
			//nodeCoordIds.Put(id, coordid;
		}
		else
		{
			SRvec3 pos;
			int id = CoordFind(coordid);
			SRcoord* coord = model.GetCoord(id);
			coord->GetPos(x, y, z, pos);
			x = pos.d[0];
			y = pos.d[1];
			z = pos.d[2];
		}
	}
	if (condofs.getLength() > 0 && !condofs.isBlank())
	{
		SRconstraint* con = model.constraints.Add();
		for (int i = 0; i < condofs.getLength(); i++)
		{
			char c = condofs.GetChar(i);
			if (c == '1')
				con->constrainedDof[0] = 1;
			else if (c == '2')
				con->constrainedDof[1] = 1;
			else if (c == '3')
				con->constrainedDof[2] = 1;
		}
		con->entityId = uid;
		con->uid = uid;
	}
	node->userId = uid;
	node->pos.Assign(x, y, z);
	if (dispCoorduid > 0)
	{
		int cid = CoordFind(dispCoorduid);
		node->dispCoordid = cid;
	}
}

static int brickBdftoSR[20] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 16, 17, 18, 19, 12, 13, 14, 15 };
static int wedgeBdftoSR[20] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 13, 14, 9, 10, 11 };
void SRinput::InputElement(SRstring& line, int& numFaces)
{
	//input elements
	// Spec:
	// CHEXA, CPENTA, or CTETRA followed by
	//eid, pid, g1,..,gn where n is 8 or 20 for hex, 6 or 15 for wedge (CPENTA), 4 or 10 for tet
	SRstring tok;
	int nnodes = 0;
	int id = model.elements.GetNum();
	SRelement* elem = model.elements.Add();
	SRstring lineSav = line;
	tok = line.BdfToken(false);
	if (tok == "CHEXA")
	{
		if (model.linearMesh)
			nnodes = 8;
		else
			nnodes = 20;
		elem->type = brick;
		model.anybricks = true;
		numFaces += 6;
	}
	else if (tok == "CPENTA")
	{
		if (model.linearMesh)
			nnodes = 6;
		else
			nnodes = 15;
		elem->type = wedge;
		model.anywedges = true;
		numFaces += 5;
	}
	else if (tok == "CTETRA")
	{
		if (model.linearMesh)
			nnodes = 4;
		else
			nnodes = 10;
		elem->type = tet;
		numFaces += 4;
	}
	int eid, pid, gid[20];
	line.BdfRead(eid);
	if (id == 0)
		elemUidOffSet = eid;
	else if (elemUidOffSet != -1)
	{
		if (eid - elemUidOffSet != id)
			elemUidOffSet = -1;
	}
	line.BdfRead(pid);
	for (int i = 0; i < nnodes; i++)
	{
		if (!line.BdfRead(gid[i]))
			ERROREXIT;//this can't happen unless mixed linear and quadratic mesh, not supported
	}
	elem->id = id;
	elem->uid = eid;
	elem->nodeUIds.Allocate(nnodes);
	if (elem->type == brick)
	{
		for (int i = 0; i < nnodes; i++)
		{
			int sri = brickBdftoSR[i];
			elem->nodeUIds.Put(i, gid[sri]);
		}
	}
	else if (elem->type == wedge)
	{
		for (int i = 0; i < nnodes; i++)
		{
			int sri = wedgeBdftoSR[i];
			elem->nodeUIds.Put(i, gid[sri]);
		}
	}
	else
	{
		for (int i = 0; i < nnodes; i++)
			elem->nodeUIds.Put(i, gid[i]);
	}

	id = ElpropFind(pid);
	if(id == -1)
		ERROREXIT;
	SRElProperty* elp = model.elProps.GetPointer(id);
	int mid = elp->matid;
	SRmaterial* mat = model.materials.GetPointer(mid);
#if 0
	if (!mat->active)
	{
		if (mid != 0)
			SCREENPRINT(" mid %d active. eluid: %d\n", mid, elem->uid);
	}
#endif
	mat->active = true;
	elem->matid = mid;
	elem->matname.Copy(mat->name);
}

void SRinput::InputElementProperty(SRstring& line)
{
	//input Element Property

	//Input Spec: 
	//PSOLID, pid, mat-uid, irrelevant...

	//skip PSOLID:
	line.BdfToken();
	int puid, muid;
	line.BdfRead(puid);
	int pid = model.elProps.GetNum();
	if (pid == 0)
		elPropUidOffset = puid;
	else if (elPropUidOffset != -1)
	{
		if (puid - elPropUidOffset != pid)
			elPropUidOffset = -1;
	}
	SRElProperty *prop = model.elProps.Add();
	prop->uid = puid;
	line.BdfRead(muid);
	prop->matuid = muid;

}
void SRinput::InputMaterial(SRstring& line, bool matNameWasRead, SRstring& matname)
{
	//input material
	//Spec:
	//MAT1,muid,E,G,nu,rho,alf,tref,GE,ST,SC,SS
	//GE = damping, ignore
	//tref = temperature reference for thermal loads
	//ST = allowable stress in tension
	//SC = allowable stress in tension
	//SS = allowable stress in tension

	//skip MAT1:
	SRstring lineSav = line;
	line.BdfToken();

	int mid = model.materials.GetNum();
	SRmaterial* mat = model.materials.Add();
	int muid;
	double E, G, nu, rho, alf, tref, GE, ST, SC, SS;
	E = G = nu = rho = alf = tref = GE = ST = SC = SS = 0.0;
	line.BdfRead(muid);
	if (mid == 0)
		MatUidOffset = muid;
	else if (MatUidOffset != -1)
	{
		if (muid - MatUidOffset != mid)
			MatUidOffset = -1;
	}
	mat->uid = muid;
	line.BdfRead(E);
	line.BdfRead(G);
	line.BdfRead(nu);
	line.BdfRead(rho);
	line.BdfRead(alf);
	line.BdfRead(tref);
	line.BdfRead(GE);
	line.BdfRead(ST);
	line.BdfRead(SC);
	line.BdfRead(SS);
	if (nu < TINY)
	{
		if (G > TINY)
			nu = -1.0 + E / (2.0*G);
	}
	char buf[100];
	if (matNameWasRead)
		SPRINTF(buf, "%d:%s", mid + 1, matname.getStr());
	else
		SPRINTF(buf, "MATERIAL%d", mid);
	mat->name.Copy(buf);
	mat->type = iso;
	mat->E = E;
	mat->nu = nu;
	mat->rho = rho;
	mat->alphax = alf;
	mat->tref = tref;
	//set allowable to the least of tensile, compressive, shear as long as they are assigned
	mat->allowableStress = BIG;
	bool assigned = false;
	if ((ST > TINY) && ST < mat->allowableStress)
	{
		mat->allowableStress = ST;
		assigned = true;
	}
	if ((SC > TINY) && SC < mat->allowableStress)
	{
		mat->allowableStress = SC;
		assigned = true;
	}
	if ((SS > TINY) && SS < mat->allowableStress)
	{
		mat->allowableStress = SS;
		assigned = true;
	}
	if (!assigned)
		mat->allowableStress = 0.0;
}

void SRinput::InputForce(SRstring& line)
{
	//input forces
	// Spec: 
	// FORCE,loadcaseid,gid,cuid,mag,fx,fy,fz
	//OR
	//PLOAD3,lsid,p,(elid,gid1,gid2)
	//p = pressure
	//elid = element
	//gid1, gid2 = nodes on diag corners of elface
	//OR
	//PLOAD4,lsid, eluid, p1,p2,p3,p4,gid1,gid2,coordid,n1,n2,n3
	//p1..p4 = pressure at nodes of face for linearly varying loads. if any of p2,p3,p4 omitted, default to p1
	//gid1 = node on corner of face, gid2 = node on diag corner of quad face
	//for tri face of wedge, gid2 is omitted
	//for tri face of tet, gid2 is the node not on the face
	//n1,n2,n3 = direction vector
	//defaults to pressure if coordid and n1,n2,n3 omitted
	//else this is an lcs force in direction n1,n2,n3, but use p's to scale the mag
	SRforce* force;
	int lsid, gid, cuid;
	double mag, magIn;
	SRvec3 f;
	if (line.CompareUseLength("FORCE1"))
	{
		line.BdfToken(); //skip FORCE1
		line.BdfRead(lsid);//rktd: SR doesn't currentlty use this. write out when SR needs it.
		line.BdfRead(gid);
		line.BdfRead(magIn);
		int guid1, guid2;
		line.BdfRead(guid1);
		line.BdfRead(guid2);
		force = model.forces.Add();
		force->type = nodalForce;
		force->entityId = gid;
		force->uid = gid;
		force->nv[0] = guid1;
		force->nv[1] = guid2;
		force->forceVals.Allocate(1, 1);
		force->forceVals.Put(0, 0, magIn);
		model.numnodalforces++;
	}
	else if (line.CompareUseLength("FORCE"))
	{
		line.BdfToken(); //skip FORCE
		line.BdfRead(lsid);//rktd: SR doesn't currentlty use this. write out when SR needs it.
		line.BdfRead(gid);
		line.BdfRead(cuid);
		line.BdfRead(magIn);
		line.BdfRead(f.d[0]);
		line.BdfRead(f.d[1]);
		line.BdfRead(f.d[2]);
		double fmag = f.Normalize();
		mag = fmag* magIn;
		f.Scale(mag);
		force = model.forces.Add();
		force->type = nodalForce;
		force->entityId = gid;
		force->uid = gid;
		force->coordId = CoordFind(cuid);
		force->forceVals.Allocate(1, 3);
		force->forceVals.Put(0, 0, f.d[0]);
		force->forceVals.Put(0, 1, f.d[1]);
		force->forceVals.Put(0, 2, f.d[2]);
		model.numnodalforces++;
	}

	else if (line.CompareUseLength("PLOAD4"))
	{
		SRstring linesav = line;
		//check linesav for "thru", only applies to shells, not relevant to solid model:
		const char *s = linesav.LastChar('T');
		if (s != NULL)
		{
			SRstring tmp;
			tmp.Copy(s);
			if (tmp.CompareUseLength("THRU"))
				return;
		}

		line.BdfToken(); //skip PLOAD4
		line.BdfRead(lsid);
		int eluid;
		line.BdfRead(eluid);

		double pv[4];
		line.BdfRead(pv[0]);
		if (!line.BdfRead(pv[1]))
			pv[1] = pv[0];
		if (!line.BdfRead(pv[2]))
			pv[2] = pv[0];
		if (!line.BdfRead(pv[3]))
			pv[3] = pv[0];
		int g1, g2;
		line.BdfRead(g1);
		line.BdfRead(g2);
		int coorduid;
		line.BdfRead(coorduid);

		SRvec3 n;
		bool pressure = true;
		if (line.BdfRead(n.d[0]))
		{
			pressure = false;
			line.BdfRead(n.d[1]);
			line.BdfRead(n.d[2]);
		}
		force = model.forces.Add();
		if (coorduid > 0)
			force->coordId = CoordFind(coorduid);
		else
			force->coordId = -1;

		force->pressure = pressure;
		force->type = faceForce;
		force->entityId = eluid;
		if (pressure)
			force->forceVals.Allocate(4, 1);
		else
		{
			for (int i = 0; i < 2; i++)
				force->forceVals.Put(i, 1, n.d[i]);
		}
		for (int i = 0; i < 4; i++)
			force->forceVals.Put(i, 0, pv[i]);

		force->nv[0] = g1;
		force->nv[1] = g2;
	}
}

void SRinput::InputVolumeForce(SRstring& line)
{
	//input volume forces
	//Spec: 
	if (line.CompareUseLength("GRAV"))
	{
		// GRAV,loadcaseid,cid,mag,fx,fy,fz
		line.BdfToken(); //skip GRAV
		int lsid, cuid;
		line.BdfRead(lsid);//rktd: SR doesn't currently use this. write out when SR needs it.
		line.BdfRead(cuid);
		double mag;
		SRvec3 f;
		line.BdfRead(mag);
		line.BdfRead(f.d[0]);
		line.BdfRead(f.d[1]);
		line.BdfRead(f.d[2]);
		f.Scale(mag);
		SRvolumeForce* vol = model.volumeForces.Add();
		vol->type = gravity;
		if (cuid > 0)
		{
			int cid = CoordFind(cuid);
			SRcoord* coord = model.GetCoord(cid);
			SRvec3 p = coord->origin;
			coord->VecTransform(p, f);
		}
		vol->g1 = f.d[0];
		vol->g2 = f.d[1];
		vol->g3 = f.d[2];
	}
	else if (line.CompareUseLength("RFORCE"))
	{
		// RFORCE,loadcaseid,cid,grid,omega, r1,r2,r3, alpha
		line.BdfToken(); //skip FORCE
		int lsid, cuid, grid;
		line.BdfRead(lsid);//rktd: SR doesn't currently use this. write out when SR needs it.
		line.BdfRead(cuid);
		line.BdfRead(grid);
		SRvec3 origin, axis;
		int nid = NodeFind(grid);
		if (nid == -1)
		{
			//force refers to node not found in model, skip it:
			return;
		}
		origin.Copy(model.GetNode(nid)->pos);
		double alpha, omega;
		line.BdfRead(omega);
		omega *= TWOPI; //see MSC linear ug p; msc use rev/time so 2pi converts to rad/time
		line.BdfRead(axis.d[0]);
		line.BdfRead(axis.d[1]);
		line.BdfRead(axis.d[2]);
		line.TokRead(alpha);
		alpha *= TWOPI;
		if (cuid > 0)
		{
			int cid = CoordFind(cuid);
			SRcoord* coord = model.GetCoord(cid);
			SRvec3 p = coord->origin;
			coord->VecTransform(p, axis);
		}
		SRvolumeForce* vol = model.volumeForces.Add();
		vol->type = centrifugal;
		axis.PlusAssign(origin);
		vol->omega = omega;
		vol->alpha = alpha;
		vol->axis.Copy(axis);
		vol->origin.Copy(origin);
	}

}

void SRinput::InputThermal(SRstring& line)
{
	//input thermal load on node(s)
	//spec:
	//TEMP,lsid,(gid,T)
	line.BdfToken();//skip "TEMP"
	int lsid;
	line.BdfRead(lsid);
	while (1)
	{
		int guid;
		if (!line.BdfRead(guid))
			break;
		int nid = NodeFind(guid);
		if (nid == -1)
		{
			//force refers to node not found in model, skip it:
			return;
		}
		SRnode* node = model.GetNode(guid);
		double T;
		line.BdfRead(T);
		node->hasTemp = true;
		node->Temp = T;
	}
}


void SRinput::InputCoordinate(SRstring& line)
{
	//input local coordinate systems
	//Spec:
	//CORD2C, uid, refid, a1,a2,a3,b1,b2,b3,c1,c2,c3
	//CORD2R, uid, refid, a1,a2,a3,b1,b2,b3,c1,c2,c3
	//CORD2S, uid, refid, a1,a2,a3,b1,b2,b3,c1,c2,c3
	int id = model.Coords.GetNum();
	int uid, refid;

	SRcoord* coord = model.Coords.Add();
	if (line.CompareUseLength("CORD2C"))
	{
		line.BdfToken(); //skip CORD2c keyword
		coord->type = cylindrical;
	}
	else if (line.CompareUseLength("CORD2R"))
	{
		line.BdfToken(); //skip CORD2c keyword
		coord->type = cartesian;
	}
	else if (line.CompareUseLength("CORD2S"))
	{
		line.BdfToken(); //skip CORD2S keyword
		coord->type = spherical;
	}
	else
		ERROREXIT; //can't handle cord1's yet
	line.BdfRead(uid);
	coord->uid = uid;
	if (id == 0)
		CoordUidOffset = uid;
	else if (CoordUidOffset != -1)
	{
		if (uid - CoordUidOffset != id)
			CoordUidOffset = -1;
	}

	line.BdfRead(refid);
	if (refid > 0)
		coord->otherCoordid = refid;
	int dof;
	SRvec3 p13, p3;
	for (dof = 0; dof < 3; dof++)
		line.BdfRead(coord->origin.d[dof]);
	for (dof = 0; dof < 3; dof++)
		line.BdfRead(p3.d[dof]);
	for (dof = 0; dof < 3; dof++)
		line.BdfRead(p13.d[dof]);
	p13.MinusAssign(coord->origin);
	p3.Subtract(coord->origin, coord->e3);
	coord->e3.Normalize();
	coord->e3.Cross(p13, coord->e2);
	coord->e2.Normalize();
	coord->e2.Cross(coord->e3, coord->e1);
	char buf[100];
	SPRINTF(buf, "LCS%d", id);
	coord->name.Copy(buf);
	SRvec3 e1g, e3g;
	e1g.Assign(1.0, 0.0, 0.0);
	e3g.Assign(0.0, 0.0, 1.0);
	coord->gcsaligned = true;
	if (e1g.Dot(coord->e1) < (1.0 - SMALL))
		coord->gcsaligned = false;
	if (e3g.Dot(coord->e3) < (1.0 - SMALL))
		coord->gcsaligned = false;
}

void SRinput::InputConstraint(SRstring& line)
{
	//input constraints
	//spec:
	//SPC, setid, (gid, condof, enfd)
	//gid = node id
	//condof = 1 for x, 2 for y, 3 for z, 12 for x and y, omitted for unconstraine
	//enfd = disp value, applied to all dofs specified in condof
	//the 3 items in parm may be repeated up to 12x
	//OR
	//SPC1,setid,(condof,gid) -repeated
	SRstring condofs;
	int setid, gid;
	double enfd;

	if (line.CompareUseLength("SPC1"))
	{
		SRconstraint* con = model.constraints.Add();
		line.BdfToken();//skip SPC
		line.BdfRead(setid);
		while (1)
		{
			const char* buf = line.BdfToken(false);
			if (buf == NULL)
				break;
			condofs = buf;
			for (int i = 0; i < condofs.getLength(); i++)
			{
				char c = condofs.GetChar(i);
				if (c == '1')
					con->constrainedDof[0] = 1;
				else if (c == '2')
					con->constrainedDof[1] = 1;
				else if (c == '3')
					con->constrainedDof[2] = 1;
			}
			line.BdfRead(gid);

			con->entityId = gid;
			con->uid = gid;
		}
	}
	else if (line.CompareUseLength("SPC"))
	{
		//SPC, setid, (gid, condof, enfd)
		line.BdfToken();//skip SPC
		line.BdfRead(setid);
		while (1)
		{
			if (!line.BdfRead(gid))
				break;
			SRconstraint* con = model.constraints.Add();
			con->entityId = gid;
			con->uid = gid;
			condofs = line.BdfToken(false);
			for (int i = 0; i < condofs.getLength(); i++)
			{
				char c = condofs.GetChar(i);
				if (c == '1')
					con->constrainedDof[0] = 1;
				else if (c == '2')
					con->constrainedDof[1] = 1;
				else if (c == '3')
					con->constrainedDof[2] = 1;
			}
			if (line.BdfRead(enfd))
			{
				con->enforcedDisplacementData.Allocate(1, 3);
				for (int dof = 0; dof < 3; dof++)
				{
					if (con->constrainedDof[dof])
						con->enforcedDisplacementData.Put(0, dof, enfd);
				}
			}
			else
				break;
		}
	}
}

void SRinput::InputEnfd(SRstring& line)
{
	int setid, gid;
	SRstring condofs;
	double enfdval;
	line.BdfToken();//skip SPC
	line.BdfRead(setid);
	line.BdfRead(gid);
	condofs = line.BdfToken(false);//condofs
	line.BdfRead(enfdval);
	SRenfd* enfd = model.enfds.Add();
	enfd->nuid = gid;
	enfd->enfdVal = enfdval;
	for (int i = 0; i < condofs.getLength(); i++)
	{
		char c = condofs.GetChar(i);
		if (c == '1')
			enfd->condof[0] = true;
		else if (c == '2')
			enfd->condof[1] = true;
		else if (c == '3')
			enfd->condof[2] = true;
	}
}

void SRinput::inputUnsupported(SRstring &line)
{
	/*
		else if (line.CompareUseLength("CTRI") || line.CompareUseLength("CQUAD") ||
			line.CompareUseLength("CB") || line.CompareUseLength("CR") || line.CompareUseLength("CS") ||
			line.CompareUseLength("CW") || line.CompareUseLength("CF") || line.CompareUseLength("Ci") ||
			line.CompareUseLength("RB") || line.CompareUseLength("RJ") || line.CompareUseLength("MPC"))
	*/
	SRstring lineSav = line;
	int nnodes = 0;
	int eid, pid, cid, gid[100000];//this is really high because # of nodes in mpcs/bsurfs is arbitrary
	SRstring tok;
	tok = line.BdfToken(false);
	int dof;
	SRunsup *unsup = model.unsups.Add();
	if (tok.CompareUseLength("CELAS1"))
	{
		nnodes = 2;
		line.BdfRead(eid);
		line.BdfRead(pid);
		line.BdfRead(gid[0]);
		line.BdfRead(cid);
		line.BdfRead(gid[1]);
	}
	if (tok.CompareUseLength("CELAS2"))
	{
		double stiff;
		nnodes = 2;
		line.BdfRead(eid);
		line.BdfRead(stiff);
		line.BdfRead(gid[0]);
		line.BdfRead(cid);
		line.BdfRead(gid[1]);
	}
	else if (tok.CompareUseLength("CBUSH"))
	{
		nnodes = 1;
		line.BdfRead(eid);
		line.BdfRead(pid);
		line.BdfRead(gid[0]);
		if (line.BdfRead(gid[1]))
			nnodes = 2;
	}
	else if (tok.CompareUseLength("CROD") || tok.CompareUseLength("CBAR") || tok.CompareUseLength("CBEAM") || tok.CompareUseLength("CBEND"))
	{
		unsup->isShellOrBeam = true;
		nnodes = 2;
		line.BdfRead(eid);
		line.BdfRead(pid);
		for (int i = 0; i < nnodes; i++)
			line.BdfRead(gid[i]);
	}

	else if (tok.CompareUseLength("CTRIA3") || tok.CompareUseLength("CTRIAR"))
	{
		unsup->isShellOrBeam = true;
		nnodes = 3;
		line.BdfRead(eid);
		line.BdfRead(pid);
		for (int i = 0; i < nnodes; i++)
			line.BdfRead(gid[i]);
	}
	else if (tok.CompareUseLength("CBEAM3"))
	{
		unsup->isShellOrBeam = true;
		nnodes = 3;
		line.BdfRead(eid);
		line.BdfRead(pid);
		for (int i = 0; i < nnodes; i++)
			line.BdfRead(gid[i]);
	}
	else if (tok.CompareUseLength("CTRIA6"))
	{
		unsup->isShellOrBeam = true;
		nnodes = 6;
		line.BdfRead(eid);
		line.BdfRead(pid);
		for (int i = 0; i < nnodes; i++)
			line.BdfRead(gid[i]);
	}
	else if (tok.CompareUseLength("CQUAD4") || tok.CompareUseLength("CQUADR") || tok.CompareUseLength("CSHEAR"))
	{
		if (!tok.CompareUseLength("CSHEAR"))
			unsup->isShellOrBeam = true;
		nnodes = 4;
		line.BdfRead(eid);
		line.BdfRead(pid);
		for (int i = 0; i < nnodes; i++)
			line.BdfRead(gid[i]);
	}
	else if (tok.CompareUseLength("CQUAD8"))
	{
		unsup->isShellOrBeam = true;
		nnodes = 8;
		line.BdfRead(eid);
		line.BdfRead(pid);
		for (int i = 0; i < nnodes; i++)
			line.BdfRead(gid[i]);
	}
	else if (tok.CompareUseLength("MPCD"))
	{
		nnodes = 2;
		line.BdfRead(eid);//this is really sid but not needed
		line.BdfRead(gid[0]);
		line.BdfRead(cid);
		line.BdfRead(gid[1]);
	}
	else if (tok.CompareUseLength("MPCY"))
	{
	}
	else if (tok.CompareUseLength("MPC"))
	{
		nnodes = 0;
		line.BdfRead(eid);//this is really sid but not needed
		line.BdfRead(gid[nnodes]);
		nnodes++;
		//skip 9 fields:
		bool done = false;
		for (int i = 0; i < 9; i++)
		{
			line.BdfToken();
			if (line.BdfToken(false) == NULL)
			{
				done = true;
				break;
			}
		}
		if (!done)
		{
			line.BdfRead(gid[nnodes]);
			nnodes++;
			line.BdfToken();
			line.BdfToken();
			if (line.BdfRead(gid[nnodes]))
			{
				nnodes++;
				//skip 6 fields:
				for (int i = 0; i < 6; i++)
				{
					if (line.BdfToken(false) == NULL)
					{
						done = true;
						break;
					}
				}
				if (!done)
				{
					line.BdfRead(gid[nnodes]);
					nnodes++;
				}
			}
		}
	}
	else if (tok.CompareUseLength("MPCD"))
	{
		double coeff;
		nnodes = 0;
		line.BdfRead(eid);//this is really sid but not needed
		while (1)
		{
			line.BdfRead(gid[nnodes]);
			line.BdfRead(pid);
			if (!line.BdfRead(coeff))
				break;
			nnodes++;
		}
	}
	else if (tok.CompareUseLength("RBE2"))
	{
		line.BdfRead(eid);
		line.BdfRead(gid[0]);
		line.BdfRead(dof);
		nnodes = 1;
		while (1)
		{
			const char* tmp = line.BdfToken(false);
			if (tmp == NULL)
				break;
			tok = tmp;
			if (tok.isBlank())
				break;
			int gidt;
			tok.BdfRead(gidt);
			int nid = NodeFind(gidt);
			if (nid < 0)
				break;
			gid[nnodes] = gidt;
			nnodes++;
		}
	}
	else if (tok.CompareUseLength("SUPORT"))
	{
		line.BdfRead(eid);//this is really sid but not needed
		for (int i = 0; i < 4; i++)
		{
			line.BdfRead(gid[i]);
			line.BdfRead(dof);
			nnodes = 4;
		}
	}
	else if (tok.CompareUseLength("BSURFS"))
	{
		//parse till not blank:
		unsup->isBsurf = true;
		int euid, guid;
		nnodes = 0;
		int bsurfNum;
		line.BdfRead(bsurfNum);
		//there are blank fields before 1st euid:
		while (1)
		{
			if (line.BdfRead(euid))
			{
				gid[nnodes] = euid;
				nnodes++;
				break;
			}
		}
		//remainder of line will be euids or guids, will sort out in finishunsup
		while (1)
		{
			if (!line.BdfRead(guid))
				break;
			gid[nnodes] = guid;
			nnodes++;
		}
	}

	unsup->gids.Allocate(nnodes);
	for (int i = 0; i < nnodes; i++)
		unsup->gids.Put(i, gid[i]);
}

int SRinput::findFaceNodes(SRelement* elem, int gidv[], int& nread, int gidFace[])
{
	if (elem->type == brick)
	{
		for (int i = 0; i < 4; i++)
		{
			gidFace[i] = gidv[nread];
			nread++;
		}
		return 4;
	}
	else if (elem->type == tet)
	{
		for (int i = 0; i < 3; i++)
		{
			gidFace[i] = gidv[nread];
			nread++;
		}
		return 3;
	}
	else if (elem->type == wedge)
	{
		for (int i = 0; i < 3; i++)
		{
			gidFace[i] = gidv[nread];
			nread++;
		}
		if (IsWedgeFaceQuad(elem, gidv))
		{
			gidFace[3] = gidv[nread];
			nread++;
			return 4;
		}
		else
			return 3;
	}
	return 0;
}


void SRinput::checkUnsupportedTouchesNonOrphan()
{
	//check if any general unsupported or beam/shell nodes touches a non-orphan node
	//this is necessary because there can be dupes
	//ttd!! this is N-squared, but hopefully there are not the many general unsupported or beam/shell nodes
	if (!model.anyGeneralUnsupportedNode && !model.anyShellOrBeamNode)
		return;

	int nnode = model.GetNumNodes();

	double distTol = RELSMALL*model.size;
	for (int n = 0; n < nnode; n++)
	{
		SRnode *node = model.GetNode(n);
		if (node->isOrphan() || node->shellOrBeamNode || node->unSupported)
			continue;
		for (int n2 = 0; n2 < nnode; n2++)
		{
			SRnode* node2 = model.GetNode(n2);
			if (node2->unSupported)
			{
				if (node2->pos.Distance(node->pos) < distTol)
					node->unSupported = true;
			}
			else if (node2->shellOrBeamNode)
			{
				if (node2->pos.Distance(node->pos) < distTol)
					node->shellOrBeamNode = true;
			}
		}
	}
}

void SRinput::cropElements()
{
	int numfreed = 0;
	int nel = model.GetNumElements();
	for (int e = 0; e < nel; e++)
	{
		SRelement* elem = model.GetElement(e);
		//check if any of the element's nodes have displacements.
		bool anyNodeWithDisp = false;
		int nnode = elem->GetNumNodes();
		for (int n = 0; n < nnode; n++)
		{
			SRnode* node = model.GetNodeFromUid(elem->nodeUIds.Get(n));
			if (node != NULL && node->hasDisp)
			{
				anyNodeWithDisp = true;
				break;
			}
		}
		if (!anyNodeWithDisp)
		{
			model.elements.Free(e);
			numfreed++;
		}
	}
	if (numfreed > 0)
	{
		model.elements.packNulls();
		//packing elements will mess up elemUidOffSet and throw off searches.
		//just set elemUidOffSet to -1 to force
		//bsearch:
		elemUidOffSet = -1;
	}
}


void SRinput::finishForces()
{
	int numfreed = 0;
	for (int f = 0; f < model.forces.GetNum(); f++)
	{
		SRforce* force = model.GetForce(f);
		if (force->type == nodalForce)
		{
			if (force->nv[0] == -1)
			{
				//force
				int gid = force->entityId;
				if (model.checkOrphanNode(gid))
				{
					model.forces.Free(f);
					numfreed++;
					continue;
				}
				int cid = force->coordId;
				if (cid > 0)
				{
					//transform to gcs:
					SRcoord* coord = model.GetCoord(cid);
					int nid = NodeFind(gid);
					SRvec3 f;
					for (int d = 0; d < 3; d++)
						f.d[d] = force->forceVals.Get(0, d);
					coord->VecTransform(model.GetNode(nid)->Position(), f);
					force->coordId = -1;//already transformed, so set gcs
				}
			}
			else
			{
				//force1
				int guid1 = force->nv[0];
				int guid2 = force->nv[1];
				if (model.checkOrphanNode(guid1) || model.checkOrphanNode(guid2))
				{
					//force refers to nodes not in mesh, may have been cropped. skip it
					model.forces.Free(f);
					numfreed++;
					continue;
				}
				int g1 = NodeFind(guid1);
				int g2 = NodeFind(guid2);
				double magIn = force->forceVals.Get(0, 0);
				force->forceVals.Free();
				SRvec3 p1;
				p1.Copy(model.GetNode(g1)->pos);
				SRvec3 f;
				f.Copy(model.GetNode(g2)->pos);
				f.MinusAssign(p1);
				f.Normalize();
				f.Scale(magIn);
				force->forceVals.Allocate(1, 3);
				force->forceVals.Put(0, 0, f.d[0]);
				force->forceVals.Put(0, 1, f.d[1]);
				force->forceVals.Put(0, 2, f.d[2]);
			}
		}
		else
		{
			//face force
			int eluid = force->entityId;
			int eid = ElemFind(eluid);
			if (eid == -1)
			{
				//element may have been cropped or is a shell. skip the force
				model.forces.Free(f);
				numfreed++;
				continue;

			}
			SRelement* elem = model.GetElement(eid);
			int g1,g2, gout[8];

			g1 = force->nv[0];
			g2 = force->nv[1];
			int ncorner = findElemFace(false, elem, g1, g2, gout);
			if (ncorner == -1)
			{
				//force refers to element nodes not in mesh, may have been cropped. skip it
				model.forces.Free(f);
				numfreed++;
				continue;
			}

			double pv[4];
			for (int n = 0; n < 4; n++)
				pv[n] = force->forceVals.Get(n, 0);
			if (force->pressure)
			{
				//ttd remove this limitation: pass the info to engine and let it handle it.
				//can't support variable pload4 unless bring in edge, face, and mapping.
				//see spec, pvs are assigned by using rh rule and outward normal to element
				//check for constant:
				double fp0 = fabs(pv[0]);
				for (int i = 0; i < ncorner; i++)
				{
					double diff = fabs(pv[i] - pv[0]);
					if (diff > fp0*RELSMALL)
					{
						model.forces.Free(f);
						numfreed++;
						continue;
						//ttd warning
					}
				}
				for (int i = 0; i < ncorner; i++)
					force->nv[i] = gout[i];
				if (ncorner == 3)
					force->nv[3] = -1;
				force->forceVals.Allocate(ncorner, 1);
				for (int i = 0; i < ncorner; i++)
					force->forceVals.Put(i, 0, pv[i]);
			}
			else
			{
				force->pressure = false;
				SRvec3 nv;
				nv.d[0] = force->forceVals.Get(0, 1);
				nv.d[1] = force->forceVals.Get(1, 1);
				force->forceVals.Free();
				force->forceVals.Allocate(ncorner, 3);
				for (int i = 0; i < ncorner; i++)
				{
					//use coordid to rotate N if nec
					if (force->coordId != -1)
					{
						SRcoord* coord = model.GetCoord(force->coordId);
						if (model.checkOrphanNode(gout[i]))
						{
							//force refers to node not found in model, skip it:
							model.forces.Free(f);
							numfreed++;
							continue;
						}
						int nodeid = NodeFind(gout[i]);
						SRvec3 p = model.GetNode(nodeid)->Position();
						coord->VecTransform(p, nv);
						force->coordId = -1;//already transformed so set as gcs
					}
					nv.Normalize();
					nv.Scale(pv[i]);
					for (int dof = 0; dof < 3; dof++)
						force->forceVals.Put(i, dof, nv.d[dof]);
				}
			}
		}
	}
	if (numfreed > 0)
		model.forces.packNulls();
}

void SRinput::finishConstraints()
{
	int numfreed = 0;
	for (int c = 0; c < model.GetNumConstraints(); c++)
	{
		SRconstraint* con = model.GetConstraint(c);
		int gid = con->entityId;
		if (model.checkOrphanNode(gid))
		{
			//constraint refers to node not in model. may have been cropped
			model.constraints.Free(c);
			numfreed++;
			continue;
		}
		int nid = NodeFind(gid);
		SRnode* node = model.GetNode(nid);
		node->constraintId = c;
		//check for coordid associated to node:
		checkLcs(con, node->dispCoordid);
	}
	for (int e = 0; e < model.enfds.GetNum(); e++)
	{
		SRenfd *enfd = model.enfds.GetPointer(e);
		int gid = enfd->nuid;
		int nid = NodeFind(gid);
		//constraint refers to node not in model. may have been cropped
		if (nid == -1)
			continue;
		SRnode* node = model.GetNode(nid);
		if (node == NULL)
			continue;
		int cid = node->constraintId;
		if (cid == -1)
			continue; //enfd is applied to unconstrained node
		SRconstraint* con = model.GetConstraint(cid);
		if (!con->hasEnforcedDisp())
			con->enforcedDisplacementData.Allocate(1, 3);
		//constraint may be fixed in more dofs than are enforced. get enforced dofs
		//from enfd->condofs, not con->constrainedDof
		for (int dof = 0; dof < 3; dof++)
		{
			if (enfd->condof[dof])
				con->enforcedDisplacementData.Put(0, dof, enfd->enfdVal);
		}
	}
	model.enfds.Free();
	if (numfreed > 0)
		model.constraints.packNulls();
}

void SRinput::finishUnsup()
{
	for (int u = 0; u < model.unsups.GetNum(); u++)
	{
		SRunsup *unsup = model.unsups.GetPointer(u);
		int nnodes = unsup->gids.GetNum();
		if (unsup->isBsurf)
		{
			int nread = 0;
			int gidFace[4];
			while (1)
			{
				int eluid = unsup->gids.Get(nread);
				nread++;
				int eid = ElemFind(eluid);
				if (eid != -1)
				{
					SRelement* elem = model.GetElement(eid);
					int nfaceGids = findFaceNodes(elem, unsup->gids.d, nread, gidFace);
					for (int n = 0; n < nfaceGids; n++)
					{
						int nid = NodeFind(gidFace[n]);
						//some entities refer to scalar points not grid points, so nid will come up -1:
						if (nid != -1)
						{
							SRnode* node = model.GetNode(nid);
							node->bsurf = true;
						}
					}
					if (nread >= nnodes)
						break;
				}
				else
					ERROREXIT;
			}
		}
		else
		{
			for (int i = 0; i < nnodes; i++)
			{
				int gid = unsup->gids.Get(i);
				int nid = NodeFind(gid);
				//some entities refer to scalar points not grid points, so nid will come up -1:
				if (nid != -1)
				{
					SRnode* node = model.GetNode(nid);
					if (unsup->isShellOrBeam)
					{
						node->shellOrBeamNode = true;
						model.anyShellOrBeamNode = true;
					}
					else
					{
						//mark general unsupported, only if it's not already a shell or beam node or a bsurf node
						//(they take precedence);
						if (!node->bsurf && !node->shellOrBeamNode)
						{
							node->unSupported = true;
							model.anyGeneralUnsupportedNode = true;
						}
					}
				}
			}
		}
	}
	model.unsups.Free();
}






