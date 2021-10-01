#pragma once

#include "common.h"
#include "unordered_map"
#include "CPath64.h"
#include "myhash.h"
#include "myAES.h"
#include <vector>
#include <algorithm>

//#include <unordered_map>

class CInvertedIndex
{

public:
	CInvertedIndex();
	void GetFileDistinctKeywords(string filename, vector<string>& keywords);
	void InsertFile(string filename);
	void LoadDirToInvertedIndex(string dir,unsigned long long maxsize=-1);
	string ReadLine(FILE* fp, char* buf);
	void Load(unsigned long long maxsize=-1, unsigned long long maxm=-1);
	void Save();
	unordered_map<string, vector<unsigned long long>> DB;
	unsigned long long N; //the number of (w,id) pairs
	unsigned long long m; //the number of keywords
	unsigned long long n; //the numebr of files;
	unordered_map<int,string> fileNames;
};

