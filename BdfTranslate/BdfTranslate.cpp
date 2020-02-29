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


// BdfTranslate.cpp : Defines the entry point for the console application.
//

#include <stdlib.h>
#include <vector>
#include "SRmodel.h"

using namespace std;

//One global instance of "model";
SRmodel model;

int main(int argc, char *argv[])
{
	SRfile ft;
	SRfile modelF;
	SRstring line, nametail, infoldername, outfoldername, inname, outname;

	if (argc > 1)
	{
		model.wkdir.Copy(argv[1]);
	}
	else
	{
		char buf[256];
		MDGETCWD(buf, sizeof(buf));
		SCREENPRINT(" _getcwd: %s\n", buf);
		model.wkdir = buf;
		model.wkdir += slashStr;
	}
	line = model.wkdir;
	line += "xlate_log.txt";
	model.logFile.SetFileName(line);
	model.logFile.Delete();
	model.logFile.Open(SRoutputMode);
	model.logFile.PrintLine("bdf translate log");
	model.logFile.PrintLine("wkdir: %s",model.wkdir.getStr());
	model.logFile.Close();
	SCREENPRINT("logfile %s\n", line.getStr());
	line = model.wkdir;
	line += "translateCmd.txt";
	if (!modelF.Open(line, SRinputMode))
	{
		SCREENPRINT("translateCmd file %s not found\n", line.getStr());
		ERROREXIT;
	}
	SRstring bdfFileName;
	modelF.GetLine(bdfFileName);
	bdfFileName.Left(slashChar, infoldername);
	bdfFileName.Right(slashChar, line);
	line.Left('.', model.fileNameTail);
	modelF.GetLine(outfoldername);
	SRstring dispFile;
	if (modelF.GetLine(dispFile))
	{
		if (SRfile::Existcheck(dispFile))
		{
			model.cropModelWithDispNodes = true;
			model.nodeDispFile.SetFileName(dispFile);
		}
	}

	outfoldername.Right(slashChar, model.SrFileNameTail);
	if (!model.inpFile.Existcheck(bdfFileName.getStr()))
	{
		//try .dat extension:
		bdfFileName.Left('.', line);
		bdfFileName.Copy(line);
		bdfFileName.Cat(".dat");
	}
	if (!model.inpFile.Existcheck(bdfFileName.getStr()))
	{
		SCREENPRINT("input file %s not found", bdfFileName.getStr());
		ERROREXIT;
	}
	model.inpFile.SetFileName(bdfFileName);

	modelF.Close();

	SRstring bdfdir;
	bdfFileName.Left(slashChar, bdfdir);
	line = model.wkdir;
	line += "xlate_status.txt";
	model.statFile.SetFileName(line);
	model.statFile.Delete();
	SCREENPRINT("statFile %s\n", line.getStr());

	line = bdfdir;
	line += slashStr;
	line += "out.txt";
	model.outputFile.SetFileName(line);
	model.outputFile.Delete();


	bdfFileName.Copy(outfoldername);
	SRfile::CreateDir(bdfFileName.getStr());
	model.outdir = bdfFileName;
	bdfFileName.Cat(slashStr);
	bdfFileName.Cat(model.SrFileNameTail);
	bdfFileName.Cat(".msh");
	model.mshFile.SetFileName(bdfFileName);
	model.mshFile.Delete();

	line = outfoldername;
	line.Cat(slashStr);
	line.Cat(model.fileNameTail);
	line.Cat(".srr");
	model.srrFile.SetFileName(line);

	SCREENPRINT(" Translating %s\n", model.fileNameTail.getStr());
	OUTPRINT(" Translating %s\n", model.fileNameTail.getStr());
	if (!model.input.Translate())
		exit(0);

	model.statFile.Open(SRoutputMode);
	model.statFile.PrintLine("translation successful model %s", model.fileNameTail.getStr());
	if (model.linearMesh)
		model.statFile.PrintLine("linear mesh");
	if (model.anyUnsupportedElement)
		model.statFile.PrintLine("unsupported elements encountered");
	if (model.partialDispFile)
		model.statFile.PrintLine("Partial Displacement File");
	if (model.isNx)
		model.statFile.PrintLine("NxNastran Model");
	model.statFile.Close();
	model.CleanUp();

	SCREENPRINT("SuccessFul Completion\n");
	OUTPRINT("SuccessFul Completion\n");
	model.outputFile.Delete();

	return 0;
}

