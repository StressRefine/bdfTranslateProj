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
// SRutil.h: interface for the SRutil class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(SRUTIL_INCLUDED)
#define SRUTIL_INCLUDED

#include <memory.h>
#include <vector>

using namespace std;


//temporary definitions to allow all new's and deletes to be
//easily found if later we decide to use a memory pool
#define ALLOCATEMEMORY new
#define DELETEMEMORY delete

#define SRASSERT(expn) SRutil::SRAssert(__FILE__,__LINE__,(expn))

class SRutil  
{
public:
	static void TimeStamp(bool debugOnly=false);
	static void SRutil::TimeStampToScreen();
	static void SRAssert(char* file, int line, bool expn);
	SRutil(){};
	~SRutil(){};
};

class SRintVector
{
public:
	void Sort();
	int Find(int intIn);
	bool isEmpty(){ return (d == NULL); };
	int GetNum(){ return num; };
	void Free()
	{
		if(num == 0)
			return;
		DELETEMEMORY d;
		d = NULL;
		num = 0;
	};
	void Allocate(int nt)
	{
		if(nt == 0)
			return;
		if(d != NULL)
			Free();
		num = nt;
		d = ALLOCATEMEMORY int[num];
		SRASSERT(d != NULL);
		Zero();
	};
	void PushBack(int v);
	void Zero(int nt = 0)
	{
		int n;
		if(nt == 0)
			n = num;
		else
            n = nt;
		for(int i = 0; i < n; i++)
			d[i] = 0;
	};
	void Set(int i)
	{
		for (int i = 0; i < num; i++)
			d[i] = i;
	};
	void Copy(SRintVector& v2, int numt = 0)
	{
		int len;
		if(numt == 0)
			len = v2.GetNum();
		else
			len = numt;
		if(num < len)
			Allocate(len);
		for(int i = 0; i < len; i++)
			d[i] = v2.d[i];
	};
	int* GetVector(){ SRASSERT(d != NULL); return d; };
#ifdef _DEBUG
	inline int Get(int i){ SRASSERT(i >= 0 && i < num); return d[i]; };
#else
	inline int Get(int i){ return d[i];};
#endif
#ifdef _DEBUG
	inline void Put(int i, int di){ SRASSERT(i >= 0 && i < num); d[i] = di; };
#else
	inline void Put(int i,int di){ d[i] = di; };
#endif
#ifdef _DEBUG
	inline void PlusAssign(int i, int di){ SRASSERT(i >= 0 && i < num); d[i] += di; };
#else
	inline void PlusAssign(int i, int di){ d[i] += di; };
#endif
	int operator [] (int i) { return Get(i); };

	SRintVector(int nt){ Allocate(nt); };
	SRintVector(){ num = 0; d = NULL; };
	~SRintVector()
    {
		Free();
	};

	int* d;
	int num;
};

class SRdoubleVector
{
public:
	bool isEmpty(){ return (d == NULL); };
	void EquateVector(double* v){ d = v; };
	int GetNum(){ return num; };
	void Copy(SRdoubleVector& v2){ Copy(v2.d, v2.GetNum()); };
	void PushBack(double v);
	void Free()
	{
		if(num == 0)
			return;
		DELETEMEMORY d;
		d = NULL;
		num = 0;
	};
	void Allocate(int nt)
	{
		if(nt == 0)
			return;
		if(d != NULL)
			Free();
		num = nt;
		d = ALLOCATEMEMORY double[num];
		SRASSERT(d != NULL);
		Zero();
	};
	void Zero(int nt = 0)
	{
		int n;
		if(nt == 0)
			n = num;
		else
			n = nt;
		for(int i = 0; i < n; i++)
			d[i] = 0.0;
	};
	void Set(double s)
	{
		for (int i = 0; i < num; i++)
			d[i] = s;
	};
	void Copy(double* v2, int len)
	{
		if(num < len)
			Allocate(len);
		for(int i = 0; i < len; i++)
			d[i] = v2[i];
	};
	double* GetVector(){ return d; };
#ifdef _DEBUG
	inline double Get(int i){ SRASSERT(i >= 0 && i < num); return d[i]; };
#else
	inline double Get(int i){ return d[i]; };
#endif
#ifdef _DEBUG
	inline void Put(int i, double di){ SRASSERT(i >= 0 && i < num); d[i] = di; };
#else
	inline void Put(int i, double di){ d[i] = di; };
#endif
#ifdef _DEBUG
	inline void PlusAssign(int i, double di){ SRASSERT(i >= 0 && i < num); d[i] += di; };
#else
	inline void PlusAssign(int i, double di){ d[i] += di; };
#endif
	double operator [] (int i) { return Get(i); };

	SRdoubleVector(int nt){ Allocate(nt); };
	SRdoubleVector(){ num = 0; d = NULL; };
	~SRdoubleVector(){ Free(); };

	double* d;
	int num;
};

template <class gen>
class SRvector
{
public:
	bool isEmpty(){ return (d.size() == 0); };
	void Free()
	{
		if (!isEmpty())
			d.clear();
	};
	void Allocate(int nt)
	{
		d.resize(nt);
	};
	int GetNum(){ return d.size(); };
	inline gen* GetVector(){ return &d[0]; };
#ifdef _DEBUG
	inline gen *GetPointer(int i){ SRASSERT(i >= 0 && i < d.size()); return &d[i]; };
#else
	inline gen *GetPointer(int i){ return &d[i]; };
#endif
#ifdef _DEBUG
	gen& Get(int i){ SRASSERT(i >= 0 && i < d.size()); return d[i]; };
#else
	gen& Get(int i){ return d[i]; };
#endif
#ifdef _DEBUG
	inline void Put(int i, gen &di){
		SRASSERT(i >= 0 && i< d.size());
		d[i] = di;
	};
#else
	inline void Put(int i, gen &di){ d[i] = di; };
#endif
#ifdef _DEBUG
	inline void PlusAssign(int i, gen &di){ SRASSERT(i >= 0 && i<num); d[i] += di; };
#else
	inline void PlusAssign(int i, gen &di){ d[i] += di; };
#endif
	void pushBack(gen dt){ d.push_back(dt); };
	gen operator [] (int i) { return Get(i); };
	SRvector(int nt){ d.resize(0); Allocate(nt); };
	SRvector(){ d.resize(0); };
	~SRvector(){ Free(); };
	vector <gen> d;
};

//use SRpointerVector instead of SRvector for cases like faces where 
//have to conservatively allocate because only some extra pointers are wasted,
//not storage of extra structure

template <class gen>
class SRpointerVector
{
public:
	bool isEmpty(){ return (num == 0); };

	void ClearButNotFree()
	{
		if (isEmpty())
			return;
		for (int i = 0; i < num; i++)
			DELETEMEMORY d[i];
		num = 0;
	};

	void Free()
	{
		if (isEmpty())
			return;
		for (int i = 0; i < num; i++)
			DELETEMEMORY d[i];
		d.clear();
		num = 0;
	};
	void Free(int i)
	{
		DELETEMEMORY d[i];
		d[i] = NULL;
	};

	void Allocate(int nt)
	{
		if (nt == 0)
			return;
		d.resize(nt);
	};

	int GetNum(){ return num; };
	int GetLength(){ return d.size(); };
	gen* Add()
	{
		SRASSERT(num < d.size());
		if (d[num] == NULL)
			d[num] = ALLOCATEMEMORY gen();
		num++;
		return d[num - 1];
	};
#ifdef _DEBUG
	inline gen *GetPointer(int i){ SRASSERT(i >= 0 && i < num); return d[i]; };
#else
	inline gen *GetPointer(int i){ return d[i]; };
#endif

	void packNulls()
	{
		int npacked = 0;
		int i = 0;
		while (1)
		{
			if (i >= num)
				break;
			if (d[i] != NULL)
			{
				d[npacked] = d[i];
				npacked++;
				i++;
				continue;
			}
			while (1)
			{
				if (i >= num)
					break;
				if (d[i] == NULL)
				{
					i++;
					continue;
				}
				d[npacked] = d[i];
				npacked++;
				i++;
				break;
			}
		}
		if (npacked < num)
		{
			num = npacked;
			d.resize(num);
		}

	};


	SRpointerVector(){ num = 0; d.resize(0); };
	~SRpointerVector(){ Free(); };
	vector <gen *> d;
	int num;
};

class SRintMatrix
{
public:
	bool isEmpty(){ return (d == NULL); };
	void Zero()
	{
		for(int i = 0; i < n; i++)
		{
			for(int j = 0; j < m; j++)
				d[i][j] = 0;
		}
	}
	void Allocate(int nt, int mt)
	{
		if(nt == 0 || mt == 0)
			return;
		if(d != NULL)
			Free();
		n = nt;
		m = mt;
		d = ALLOCATEMEMORY int* [n];
		SRASSERT(d != NULL);
		for(int i = 0; i < n; i++)
		{
			d[i] = ALLOCATEMEMORY int[m];
			SRASSERT(d[i] != NULL);
		}
		Zero();
	};
	void Free()
	{
		if(d == NULL)
			return;
		for(int i = 0; i < n; i++)
			DELETEMEMORY d[i];
		DELETEMEMORY [n] d;
		d = NULL;
		n = m = 0;
	};
#ifdef _DEBUG
	inline int Get(int i,int j)
	{
		SRASSERT(i >= 0 && i < n); SRASSERT(j >= 0 && j < m); return d[i][j];
	};
#else
	inline int Get(int i, int j){ return d[i][j]; };
#endif
#ifdef _DEBUG
	inline void Put(int i,int j, int v)
	{
		SRASSERT(i >= 0 && i < n); SRASSERT(j >= 0 && j < m); d[i][j] = v;
	};
#else
	inline void Put(int i, int j, int v){ d[i][j] = v; };
#endif
#ifdef _DEBUG
	inline void PlusAssign(int i, int j, int v)
	{
		SRASSERT(i >= 0 && i < n); SRASSERT(j >= 0 && j < m); d[i][j] += v;
	};
#else
	inline void PlusAssign(int i, int j, int v){ d[i][j] += v; };
#endif
	void getSize(int& nt, int &mt){ nt = n; mt = m; };
	int getNumCols(){ return n; };
	int getNumRows(){ return m; };

	SRintMatrix(){ d = NULL; };
	SRintMatrix(int n, int m){ Allocate(n, m); };
	~SRintMatrix(){ Free(); };
private:
	int n,m;
	int** d;
};

class SRdoubleMatrix
{
public:
	bool isEmpty(){ return (d == NULL); };
	void Allocate(int nt, int mt)
	{
		if(nt == 0 || mt == 0)
			return;
		if(d != NULL)
			Free();
		n = nt;
		m = mt;
		d = ALLOCATEMEMORY double* [n];
		SRASSERT(d != NULL);
		for(int i = 0; i < n; i++)
		{
			d[i] = ALLOCATEMEMORY double[m];
			SRASSERT(d[i] != NULL);
		}
        Zero();
	};
	void Free()
	{
		if(d == NULL)
			return;
		for(int i = 0; i < n; i++)
			DELETEMEMORY d[i];
		DELETEMEMORY [n] d;
		d = NULL;
		n = m = 0;
	};
	void Zero()
	{
		for(int i = 0; i < n; i++)
		{
			for(int j = 0; j < m; j++)
				d[i][j] = 0.0;
		}
	};
	void Scale(int i, double s)
	{
		for (int j = 0; j < 3; j++)
			d[i][j] *= s;
	}

	void Scale(int i, int j, double s)
	{
		d[i][j] *= s;
	}

#ifdef _DEBUG
	inline double Get(int i,int j)
	{
		SRASSERT(i >= 0 && i < n); SRASSERT(j >= 0 && j < m); return d[i][j];
	};
#else
	inline double Get(int i, int j){ return d[i][j]; };
#endif
#ifdef _DEBUG
	inline void Put(int i, int j ,double v)
	{
		SRASSERT(i >= 0 && i < n); SRASSERT(j >= 0 && j < m); d[i][j] = v;
	};
#else
	inline void Put(int i, int j ,double v){ d[i][j] = v; };
#endif
#ifdef _DEBUG
	inline void PlusAssign(int i, int j, double v)
	{
		SRASSERT(i >= 0 && i < n); SRASSERT(j >= 0 && j < m); d[i][j] += v;
	};
#else
	inline void PlusAssign(int i, int j ,double v){ d[i][j] += v; };
#endif
	void getSize(int& nt, int &mt){ nt = n; mt = m; };
	int getNumCols(){ return n; };
	int getNumRows(){ return m; };
	void Copy(SRdoubleMatrix& that);
	void PlusAssign(SRdoubleMatrix& that);

	SRdoubleMatrix(){ n = 0; m = 0; d = NULL; };
	SRdoubleMatrix(int n, int m){ Allocate(n, m); };
	~SRdoubleMatrix(){ Free(); };
	int n,m;
	double** d;
};

template <class gen>
class SRmatrix
{
public:
	bool isEmpty(){ return (d == NULL); };
	void Allocate(int nt, int mt)
	{
		n = nt;
		m = mt;
		if(d != NULL)
			Free();
		d = ALLOCATEMEMORY gen* [n];
		SRASSERT(d != NULL);
		for(int i = 0; i < n; i++)
		{
			d[i] = ALLOCATEMEMORY gen[m];
			SRASSERT(d[i] != NULL);
		}
	};
	void Free()
	{
		if(d == NULL)
			return;
		for (int i = 0; i < n; i++)
			DELETEMEMORY d[i];
		DELETEMEMORY [n] d;
		d = NULL;
		n = m = 0;
	};

	SRmatrix(){ n = 0; m = 0; d = NULL; };
	SRmatrix(int n, int m){ Allocate(n, m); };
	~SRmatrix(){ Free(); };

	int n, m;
	gen** d;
};

#endif //!defined(SRUTIL_INCLUDED)


