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
	SRfile();
	~SRfile(){Close();};
	static bool CreateDir(const char* name);
	static void Delete(const char* name);
	static bool Existcheck(const char* name);
	static bool Existcheck(SRstring& name);
	static bool PrintOutFileNoReturn(const char *fmt, ...);
	static bool PrintOutFile(const char *fmt, ...);
	static bool PrintOutFile();
	static bool Screenprint(const char *fmt, ...);
	static void OpenOutFile();
	static void CloseOutFile();
	bool VPrintLine(const char* fmt, va_list arglist);
	bool VPrint(const char* fmt, va_list arglist);
	void Delete();
	bool PrintReturn();
	bool Close();
	void ToTop();
	bool GetBdfLine(SRstring& line, bool& isComment, bool &isMat, SRstring& matname);
	bool GetLine(SRstring& line, bool noSlashN = true);
	bool Open(FileOpenMode mode, const char* name = NULL);
	bool Open(SRstring& fn, FileOpenMode mode);
	bool Print(const char* s, ...);
	bool PrintLine(const char* s, ...);
	void SetFileName(SRstring& name);

	SRstring tmpstr;
	FILE* fileptr;
	bool opened;
	SRstring filename;
	char linebuf[MAXLINELENGTH];
	bool bdfLineSaved;
	SRstring bdfLineSave;
};
#endif //if !(defined SRFILE_INCLUDED)
