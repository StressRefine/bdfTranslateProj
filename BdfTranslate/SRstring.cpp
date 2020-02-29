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
// SRstring.cpp: implementation of the SRstring class.
//
//////////////////////////////////////////////////////////////////////

#include "SRmachDep.h"
#include "SRstring.h"
#include <sstream>  

using namespace std;

static char bdfBuf[18];

static string strBuf;

SRstring::SRstring(SRstring& s2)
{
	Copy(s2);
};

SRstring::SRstring(const char* s)
{
	Copy(s);
};

SRstring::SRstring()
{
	fresh = true;
	setTokSep(' ');
	tokNum = 0;
	bdfPointer = 0;
	bdfWidth = 8;
};


SRstring::~SRstring()
{
	Clear();
};

void SRstring::Clear()
{
	//clear a string for reuse
	str.erase();
	strSubs.clear();
};

const char* SRstring::getStr()
{
	return str.data();
}

const char* SRstring::LastChar(const char c, bool after)
{
	//LastChar finds last occurrence of character c. returns c and remainder of
	//string to right of c; returns NULL if c not found
	unsigned p = str.rfind(c);
	if (p < 0 || p >= str.size())
		return NULL;
	if (after)
		p++;
	strBuf = str.substr(p);
	return strBuf.c_str();
};

void SRstring::Copy(SRstring& s2)
{
	str.assign(s2.str);
	bdfWidth = s2.bdfWidth;
	fresh = s2.fresh;
	tokNum = 0;
	bdfPointer = 0;
	bdfWidth = s2.bdfWidth;
	setTokSep(s2.tokSep);
};

char SRstring::GetChar(int i)
{
	return str[i];
};

char SRstring::operator [] (int i)
{
	return GetChar(i);
};

const char* SRstring::FirstChar(char c)
{
	//FirstChar finds 1st occurrence of character c. returns c and remainder of
	//string to right of c; returns NULL if c not found
	int n = str.find(c);
	strBuf = str.substr(n);
	return strBuf.c_str();
};

void SRstring::Copy(const char *s, int n)
{
//Copy string s, overriding SRstring.str of "this"
	//input:
		//s = string
		//n = number of characters to copy. If n is 0, copy length of s
	Clear();
	if(s == NULL)
		return;
	fresh = true;
	str.assign(s);
	tokNum = 0;
	bdfPointer = 0;
	bdfWidth = 8;
	setTokSep(' ');
}

void SRstring::Cat(SRstring& s2)
{
	Cat(s2.getStr());
}

void SRstring::operator = (const char* s)
{
	Copy(s);
}
void SRstring::operator = (SRstring& s2)
{
	Copy(s2.getStr());
}

void SRstring::operator += (const char* s)
{
	Cat(s);
}

void SRstring::operator += (SRstring& s)
{
	Cat(s);
}

bool SRstring::Compare(SRstring& s2, int n)
{
	return Compare(s2.getStr(), n);
}

bool SRstring::CompareUseLength(SRstring& s2)
{
	return CompareUseLength(s2.getStr());
}

bool SRstring::operator == (const char* s2)
{
	return CompareUseLength(s2);
}

bool SRstring::operator != (const char* s2)
{
	return !Compare(s2);
}

bool SRstring::operator == (SRstring& s2)
{
	return CompareUseLength(s2);
}

void SRstring::Cat(const char *s)
{
//concatenate string s onto SRstring.str, reallocating space
	//input:
		//s = string
	if(str.length() == 0)
		Copy(s);
	else
		str.append(s);
	fresh = true;
}

bool SRstring::Compare(const char *s2, int n)
{
//see if s2 is same as str ("stricmp"); optional n chars ("strnicmp")
//ignoring case
	if (n == 0)
		return (str.compare(s2) == 0);
	else
		return (str.compare(0, n, s2) == 0);
}

const char *SRstring::Token()
{
if (strSubs.size() == 0)
	{
		tokNum = 0;
		//split the string using tokSep

		// stringstream class ss; 
		stringstream ss(str);

		string subfound;
		while (getline(ss, subfound, tokSep))
		{
			if(subfound.size() > 0)
				strSubs.push_back(subfound);
		}
	}
	if (tokNum >= (int) strSubs.size())
		return NULL;
	else
	{
		const char* retChars = strSubs[tokNum].c_str();
		tokNum++;
		return retChars;
	}
}

bool SRstring::TokRead(int &i)
{
//get next Token of this string, interpret as integer
	const char *s = Token();
	i = -1;
	if (s == NULL)
		return false;
	if (strIsBlank(s))
		return false;
	if (SSCANF(s, "%d", &i) > 0)
		return true;
	else
		return false;
}

bool SRstring::TokRead(double  &r)
{
	//get next Token of this string, interpret as double
	const char *s = Token();
	r = 0.0;
	if (strIsBlank(s))
		return false;
	if (s == NULL)
		return false;
	if (SSCANF(s, "%lg", &r) > 0)
		return true;
	else
		return false;
}

int SRstring::IntRead()
{
	int i = 0;
	SSCANF(getStr(), "%d", &i);
	return i;
}


double SRstring::RealRead()
{
	double r = 0.0;
	SSCANF(getStr(), "%lg", &r);
	return r;
}

bool SRstring::CompareUseLength(const char* s2)
{
//see if s2 is same as str ("strncmp") using length of s2
//as number of characters
	int n = strlen(s2);

	if(str.length() == 0)
	{
		if(n == 0)
			return true;
		else
			return false;
	}
	int ret = str.compare(0, n, s2);
	return (ret == 0) ;
}

void SRstring::Left(int n, SRstring &s2)
{
	//copy leftmost n characters of "this" to s2
	s2.Copy(getStr(), n);
}

void SRstring::Left(char c, SRstring &s2, bool last)
{
	//copy characters of "this" left of char c to s2
	//if last = true, then use last occurrence of c, else use 1st
	int n;
	if (last)
		n = str.rfind(c);
	else
		n = str.find(c);
	s2.Copy(str.substr(0, n).c_str());
}

void SRstring::Right(char c, SRstring &s2)
{
	//copy characters of "this" right of last occurrence of char c to s2
	int pos = str.rfind(c);
	if (pos >= 0)
	{
		s2.Copy(str.substr(pos + 1).c_str());
	}
}

bool SRstring::isCommentOrBlank()
{
	if (Compare("") || CompareUseLength("\\") || CompareUseLength("//") || CompareUseLength("$"))
		return true;
	else
		return false;

}

bool SRstring::isBlank()
{
	SRstring s2;
	s2 = getStr();
	s2.TrimWhiteSpace();
	if (s2.Compare(""))
		return true;
	else
		return false;
}


bool SRstring::isBdfComment(bool &isMat, SRstring& matname)
{
	isMat = false;
	if (str[0] == '$')
	{
		if (this->CompareUseLength("$ Femap with NX Nastran Material"))
		{
			isMat = true;
			this->Right(':', matname);
			matname.TrimWhiteSpace();
		}
		return true;
	}
	else
		return false;

}

const char* SRstring::BdfToken(bool skipOnly)
{
	if (str.size() == 0)
		return NULL;
	if (!isCsv())
	{
		int n = str.size() - bdfPointer;
		if (n <= 0)
			return NULL;
		int width = getBdfWidth();
		if (n > width)
			n = width;
		int pprev = bdfPointer;
		bdfPointer += width;
		if (skipOnly)
			return NULL;
		strBuf = str.substr(pprev, n);
		const char* s = strBuf.c_str();
		return s;
	
	}
	else
	{
		const char* s = Token();
		if (skipOnly)
			return NULL;
		STRCPY(bdfBuf, 17, s);
		return bdfBuf;
	}
}


bool SRstring::BdfRead(int &i)
{
	i = 0;
	const char* s = BdfToken(false);
	if (s == NULL)
		return false;
	if (strIsBlank(s))
		return false;
	if (SSCANF(s, "%d", &i) > 0)
		return true;
	else
		return false;
}

bool SRstring::BdfRead(double &r)
{
	r = 0.0;
	const char* s = BdfToken(false);
	if (s == NULL)
		return false;
	if (strIsBlank(s))
		return false;
	char buf[20];
	realStringCopy(buf, s, strlen(s));
	if (SSCANF(buf, "%lg", &r) > 0)
		return true;
	else
		return false;
}


bool SRstring::continueCheck()
{
	if (str.size() > 72)
	{
		char c = str[72];
		truncate(72);
		if (c == '+')
			return true;
	}
	return false;
}


int SRstring::FirstCharLocation(const char c)
{
	//returns 1st location of char c, -1 if not found
	unsigned pos = str.find(c);
	if (pos < str.size())
		return pos;
	else
		return -1;
}

void SRstring::bdfCheckLargeField()
{
	//check if this is large field line (there will be a '*' in locations 0-7)
	//if so, set bdfWidth
	int loc = FirstCharLocation('*');
	if (loc >= 0 && loc < 8)
		bdfWidth = 16;
}

int SRstring::getBdfWidth()
{
	if (bdfPointer > 7)
		return bdfWidth;
	else
		return 8;
}

void SRstring::truncate(int n)
{
	//truncate this field at char n
	str.resize(n);
}

void SRstring::TrimWhiteSpace()
{
	int k = 0;
	for (int i = 0; i < getLength(); i++)
	{
		if (str[i] != ' ')
		{
			str[k] = str[i];
			k++;
		}
	}
	str[k] = '\0';
	str.resize(k);
}

bool SRstring::strIsBlank(const char* str)
{
	int n = strlen(str);
	for (int i = 0; i < n; i++)
	{
		if (str[i] != ' ')
			return false;
	}
	return true;
}

void SRstring::setTokSep(const char sep)
{
	tokSep = sep;
}

bool SRstring::isCsv()
{
	return (tokSep == ',');
}

int SRstring::getLength()
{
	return str.size();
}

void SRstring::realStringCopy(char* dest, const char* src, int len)
{
	int ii = 0;
	bool afterDot = false;
	bool efound = false;
	for (int i = 0; i < len; i++)
	{
		char c = src[i];
		if (c == ' ')
			continue;
		if (c == 'e' || c == 'E')
			efound = true;
		if (afterDot && !efound)
		{
			if ((i != len) && (c == '+' || c == '-'))
			{
				dest[ii] = 'E';
				ii++;
			}
		}
		else if (c == '.')
			afterDot = true;
		dest[ii] = c;
		ii++;
	}
	dest[ii] = '\0';
}



