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
// SRfile.cpp: implementation of the SRfile class
//
//////////////////////////////////////////////////////////////////////

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "SRmachDep.h"
#include "SRfile.h"
#include "SRmodel.h"


extern SRmodel model;

SRfile::SRfile()
{
	fileptr = NULL;
	opened = false;
	filename = "";
	bdfLineSaved = false;
}

bool SRfile::Open(FileOpenMode mode,const char *name)
{
    //open file "name"
    //input:
        //mode = SRinputMode, SRoutputMode, SRappendMode, SRoutbinaryMode, SRinbinaryMode, or SRinoutbinaryMode
    //return:
		//true if file was already opened else flalse
	if(opened)
		return false;

	if (name != NULL)
		filename = name;
	if (filename.getLength() == 0)
		return false;

	if (mode == SRinputMode)
	{
		//unsuccessful trying to open a non-existent file for reading
		if (!Existcheck(filename.getStr()))
			return false;
		FOPEN(fileptr,filename.getStr(), "r");
	}
	else if (mode == SRoutputMode)
	{
		//overwrite existing file for output mode:
		if (Existcheck(filename.getStr()))
			UNLINK(filename.getStr());
		FOPEN(fileptr,filename.getStr(), "w");
	}
	else if (mode == SRappendMode)
		FOPEN(fileptr,filename.getStr(), "a");
	else if (mode == SRoutbinaryMode)
		FOPEN(fileptr,filename.getStr(), "wb");
	else if (mode == SRinbinaryMode)
	{
		//unsuccessful trying to open a non-existent file for reading
		if (!Existcheck(filename.getStr()))
			return false;
		FOPEN(fileptr,filename.getStr(), "rb");
	}
	else if (mode == SRinoutbinaryMode)
		FOPEN(fileptr,filename.getStr(), "rb+");
	else
		return false;

	if(fileptr == NULL)
	{
		return false;
	}
	else
	{
		opened = true;
		return true;
	}
}

bool SRfile::GetBdfLine(SRstring& line, bool& isComment, bool &isMat, SRstring& matname)
{
	//get a line from a file. if the line contains continuations, read until done with continuations, concatting to this line

	if (bdfLineSaved)
	{
		line.Copy(bdfLineSave);
		bdfLineSaved = false;
	}
	else
	{
		if (!GetLine(line))
			return false;
	}
	if (line.CompareUseLength("ENDDATA"))
		return false;
	if (line.isBdfComment(isMat, matname))
	{
		isComment = true;
		return true;
	}
	else
		isComment = false;
	//check for csv:
	line.setTokSep(' ');
	if (line.LastChar(',') != NULL)
		line.setTokSep(',');

	//continuation check: next line starts with a $ for a comment, skip it, or +, *, ',', or blank:
	SRstring line2;
	bool firstContinue = true;
	while (1)
	{
		if (!GetLine(line2))
			return false;
		char c0 = line2.GetChar(0);
		if (c0 == '+' ||
			c0 == '*' ||
			c0 == ',' ||
			c0 == ' ')
		{
			if (!line.isCsv())
			{
				if (line2.getLength() > 72)
					line2.truncate(72);
				//continuation line, splice to 1st
				if (firstContinue)
					line.truncate(72); //strip trailing comment fields from 1st line
				line.Cat(line2.str.substr(8).c_str()); // the 8 skips the first 8 fields of the continue, they are for opt. comment
			}
			else
			{
				//skip the "," in line2:
				line.Cat(line2.str.substr(1).c_str());//!!ttd make Right fun
			}
			bdfLineSaved = false;
			firstContinue = false;
		}
		else
		{
			//next keyword encountered:
			bdfLineSave.Copy(line2);
			bdfLineSaved = true;
			break;
		}
	}

	//reset to default (small field):
	line.bdfWidth = 8;
	//check for large field:
	if (!line.isCsv())
	{
		line.bdfCheckLargeField();
		line.bdfPointer = 0;
	}
	return true;
}

void SRfile::ToTop()
{
	rewind(fileptr); bdfLineSaved = false;
}

bool SRfile::Open(SRstring& fn, FileOpenMode mode)
{
	return Open(mode, fn.getStr());
}

void SRfile::SetFileName(SRstring& name)
{
	filename = name;
}

bool SRfile::GetLine(SRstring &line,bool noSlashN)
{
    //get a line from a file
    //input:
        //noSlashN = true to not return the "\n" character at end of line else false
    //output:
        //line = the fetched line stored as SRstring
    //return
        //true if successful else false (e.g. EOF)
	line.Clear();
	char *tmp,c;
	int len;
	tmp = fgets(linebuf, MAXLINELENGTH, fileptr);
	if (tmp == NULL)
		return false;
	if(noSlashN)
	{
		len = strlen(tmp);;
		c = tmp[len-1];
		if(c == '\n')
			tmp[len-1] = '\0';
	}
	else
	{
		len = strlen(tmp);
		c = tmp[len-1];
		if(c != '\n')
		{
			//append \n if not already there:
			tmp[len] = '\n';
		}
	}
	line = tmp;
	return true;
}

bool SRfile::Close()
{
    //close a file
    //return:
		//false if file was not opened or close is unsuccessful, else true

	if(!opened)
		return false;
	opened = false;
	if(fclose(fileptr) != 0)
		return false;
	fileptr = NULL;
	return true;
}


bool SRfile::Print(const char *fmt,...)
{
//print to file; layer over vfprintf
//input:
    //fmt = format string
    //... = variable input

	va_list arglist;
	va_start(arglist, fmt);
	int ret = vfprintf(fileptr, fmt, arglist);
	va_end(arglist);
	if(ret < 0)
		return false;
	else
		return true;
}

bool SRfile::VPrint(const char *fmt, va_list arglist)
{
	//print to file; layer over vfprintf
	//input:
	//fmt = format string
	//va_list = variable argument list

	int ret = 0;
	vprintf(fmt, arglist);
	ret = vfprintf(fileptr, fmt, arglist);
	va_end(arglist);
	if (ret < 0)
		return false;
	else
		return true;
}

bool SRfile::PrintLine(const char *fmt,...)
{
//print to file; layer over vfprintf. append \n
//input:
    //fmt = format string
    //... = variable input

	va_list arglist;
	va_start(arglist, fmt);
	bool ret = VPrintLine(fmt, arglist);
	va_end(arglist);
	return ret;
}

bool SRfile::VPrintLine(const char *fmt, va_list arglist)
{
//print to file; layer over vfprintf. append \n
//input:
    //fmt = format string
    //va_list = variable argument list

	int len = strlen(fmt);
	int ret = 0;
	if(fmt[len-1] != '\n')
	{
		STRCPY(linebuf, MAXLINELENGTH, fmt);
		STRCAT(linebuf, MAXLINELENGTH, "\n");
		ret = vfprintf(fileptr, linebuf, arglist);
	}
	else
	{
		ret = vfprintf(fileptr, fmt, arglist);
	}
	va_end(arglist);
	if(ret < 0)
		return false;
	else
		return true;
}

bool SRfile::PrintReturn()
{
    //print "\n" to a file
	int ret = fprintf(fileptr,"\n");
	if(ret < 0)
		return false;
	else
		return true;
}

//static:

bool SRfile::Existcheck(const char* name)
{
	//determine if file "name" exists
	//return true if it exists else false
#ifdef linux
	if (access(name, 0) != -1)
		return true;
	else
		return false;
#else
	if (_access(name, 0) != -1)
		return true;
	else
		return false;
#endif
}



bool SRfile::Existcheck(SRstring& name)
{
	return Existcheck(name.getStr());
}

void SRfile::Delete(const char *name)
{
    //delete file "name"
	if(Existcheck(name))
		UNLINK(name);
}

void SRfile::Delete()
{
    //delete this file
	if(opened)
		Close();
	SRfile::Delete(filename.getStr());
}

bool SRfile::PrintOutFile()
{
	//print blank line to out file

	SRfile* f = &model.outputFile;
	bool opened = f->opened;
	if (!opened)
	{
		if (!f->Open(SRappendMode))
			return false;
	}
	f->PrintReturn();
	if (!opened)
		f->Close();
	return true;
}

bool SRfile::PrintOutFile(const char *fmt, ...)
{
	//print to file using format fmt; append \n
	//input:
		//fmt = format string
		//... variable data to print
	//return:
		//true if successful else false

	SRfile* f = &model.outputFile;
	bool opened = f->opened;
	if (!opened)
	{
		if (!f->Open(SRappendMode))
			return false;
	}
	va_list arglist;
	va_start(arglist, fmt);
	bool ret = f->VPrintLine(fmt, arglist);
	if (!opened)
		f->Close();
	return ret;
}

bool SRfile::PrintOutFileNoReturn(const char *fmt, ...)
{
	//print to file using format fmt;
	//input:
	//fmt = format string
	//... variable data to print
	//return:
	//true if successful else false

	SRfile* f = &model.outputFile;
	bool opened = f->opened;
	if (!opened)
	{
		if (!f->Open(SRappendMode))
			return false;
	}
	va_list arglist;
	va_start(arglist, fmt);
	bool ret = f->VPrint(fmt, arglist);
	if (!opened)
		f->Close();
	return ret;
}

void SRfile::OpenOutFile()
{
	model.outputFile.Open(SRappendMode);
}
void SRfile::CloseOutFile()
{
	model.outputFile.Close();
}


bool SRfile::Screenprint(const char *fmt, ...)
{
	//print to cmd screen
	//input:
		//fmt = format string
		//... variable data to print
	//return:
		//true if successful else false
	va_list arglist;
	va_start(arglist, fmt);
	int ret = 0;
	ret = vprintf(fmt, arglist);
	return ret;
}

bool SRfile::CreateDir(const char *name)
{
    //create directory "name"
    //return:
		//true if successful else false

	bool ret;
	if(Existcheck(name))
		ret = true;
	else
		ret = SRmachDep::CreateDir(name);
	return ret;
}
