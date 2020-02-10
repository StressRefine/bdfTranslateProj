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
// SRfile.h: interface for the SRfile class.
//
//////////////////////////////////////////////////////////////////////

#if !(defined SRFILE_INCLUDED)
#define SRFILE_INCLUDED

#include "SRstring.h"
#include "SRutil.h"

#define MAXLINELENGTH 1024

#define OUTOPEN SRfile::OpenOutFile
#define OUTCLOSE SRfile::CloseOutFile
#define OUTPRINT SRfile::PrintOutFile
#define OUTPRINTNORET SRfile::PrintOutFileNoReturn
#define SCREENPRINT SRfile::Screenprint

enum FileOpenMode{ SRinputMode, SRoutputMode, SRappendMode, SRoutbinaryMode, SRinbinaryMode, SRinoutbinaryMode };

class SRfile
{
	friend class SRoutput;
	friend class SRinput;
public:
	static bool CreateDir(char* name);
	static void GetCurrentDir(SRstring& dir);
	static void Delete(char* name);
	static bool Existcheck(char* name);
	static bool Existcheck(SRstring& name){return Existcheck(name.str);};
	static bool PrintOutFileNoReturn(char *fmt, ...);
	static bool PrintOutFile(char *fmt, ...);
	static bool PrintOutFile();
	static bool Screenprint(char *fmt, ...);
	static void OpenOutFile();
	static void CloseOutFile();
	bool VPrintLine(char* fmt, va_list arglist);
	bool VPrint(char* fmt, va_list arglist);
	bool SeekBinary(int pos, bool intArg = false);
	bool ReadBinary(int n, void* v, bool intArg = false);
	bool WriteBinary(int n, void* v, bool intArg = false);
	void Delete();
	bool PrintReturn();
	bool Close();
	void ToTop(){ rewind(fileptr); bdfLineSaved = false; };
	bool GetBdfLine(SRstring& line, bool& isComment, bool &isMat, SRstring& matname);
	bool GetLine(SRstring& line, bool noSlashN = true);
	bool Open(FileOpenMode mode, char* name = NULL);
	bool Open(SRstring& fn, FileOpenMode mode){ return Open(mode, fn.str); };
	bool Print(char* s, ...);
	bool PrintLine(char* s, ...);
	void SetFileName(SRstring& name){ filename = name; };
	int GetFilePos(){ return filePos; };

	SRfile(){ fileptr = NULL; opened = false; filename = ""; bdfLineSaved = false; };
	~SRfile(){Close();};

	SRstring tmpstr;
	FILE* fileptr;
	bool opened;
	SRstring filename;
	char linebuf[MAXLINELENGTH];
	int filePos;
	bool bdfLineSaved;
	SRstring bdfLineSave;
};
#endif //if !(defined SRFILE_INCLUDED)