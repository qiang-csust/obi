#pragma once
#include "common.h"
#include "unordered_map"
#include "CPath64.h"
#include "myhash.h"
#include "myAES.h"
#include <vector>
#include <algorithm>
//#include "OUtree.h"

typedef unsigned long long ull;

struct Byte20
{
	unsigned char buf[20];
};

struct Mem20
{
	unsigned char buf[20];
};

struct Byte28
{
	unsigned char buf[20];
	ull id;
};
struct Byte36
{
	unsigned char buf[36];
};

struct OUhash_func
{
	size_t operator()(const Byte20& addr) const
	{
		return *(size_t*)addr.buf;
	}
};

struct OUcmp_fun //比较函数 ==
{
	bool operator()(const Byte20& a1, const Byte20& a2) const
	{
		return memcmp(a1.buf, a2.buf, sizeof(a1)) == 0 ? true : false;
	}
};

//typedef unordered_map<string, ENode> ENodes;
//typedef unordered_map<string, TreeNode> TreeNodes;
//typedef unordered_map<OUIndexKey, Slot, OUhash_func, OUcmp_fun> KeyValues;

struct TOKEN
{
	char buf[20];
};

struct EID
{
	char buf[16];
};

// H(F_K(w)||0) 

void H1(Byte20* mem, Byte20* key);
// H(F_K(w) ||1)
void H2(void* mem, Byte36* mask);
EID ID2EID(ull id);
ull EID2ID(EID eid);


typedef unordered_map<Byte20, Byte36, OUhash_func, OUcmp_fun> EII_MAP;
typedef unordered_map<string, vector<ull>> InvertedIndexMap;//inverted index


class EII
{
public:
	EII();
	~EII();
	ull N = 0;

	//生成用户令牌
	TOKEN GenToken(string w);
	void WID_to_Key(string w, ull id, Mem20& key20);
	Byte20 WriteMap(EII_MAP& mp, Byte20 key, string w, ull id);
	Byte20 ReadMap(EII_MAP& mp, Byte20& key, EID& eid);
	void Setup(InvertedIndexMap& db);
	void Add(string w,vector<ull>& ids);
	void insertBatch(string w, ull from, ull to);
	void Search(string w, vector<ull>& ids);
	void Search(string w);
	void test_ini(int m, int len, int Z);
	void test_ini2(int m, int max_len, int Z);
	//void init(vector<WID> index);
	EII_MAP map;
};

