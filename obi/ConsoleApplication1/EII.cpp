#include "EII.h"
#include <search.h>
//密文倒排索引的封装

EID ID2EID(ull id)
{
	EID eid = { 0 };
	int len=0;
	myAES::Encrypt((char*)&id, 8, (char*)&eid, len);
	return eid;
};
ull EID2ID(EID eid)
{
	char buf[32] = { 0 };
	ull id;
	int len;
	myAES::Decrypt((char*)&eid, sizeof(EID), (char*)buf, len);
	id = *(ull*)buf;
	return id;
};
void H1(Byte20* mem, Byte20* key)
{
	//
	char buf[40] = { 0 };
	memcpy(buf, (void*)mem, 20);
	buf[20] = 1;
	myhash::Blake2b(buf, 21, (unsigned char*)key);
};
// H(F_K(w) ||1)
void H2(void* mem, Byte36* mask)
{
	//
	char buf[40] = { 0 };
	memcpy(buf, (void*)mem, 20);
	buf[20] = 2;
	myhash::Blake2b_36(buf, 21, (unsigned char*)mask);
};

EII::EII()
{
	//ptree = NULL;
}

EII::~EII()
{
//	delete ptree;
}

TOKEN EII::GenToken(string w)
{
	TOKEN tk;
		char buf[40] = { 0 };
		int len = w.length();
		if (len >= 20) len = 20;
		memcpy(buf,(void*)w.c_str(), len);
		buf[20] = 3;
		myhash::Blake2b(buf, 21, (unsigned char*)tk.buf);	
	return tk;
}

void XOR(unsigned char* in1, unsigned char* in2, int len)
{
	for (int i = 0; i < len; i++)
	{
		in1[i] = in1[i] ^ in2[i];
	}
}

void EII::WID_to_Key(string w,ull id , Mem20& key20)
{
	char mem[40] = { 0 };
	int len = w.length();
	if (len > 32) len = 32;
	*(ull*)(mem + 32) = id;
	//triplet t = WID_to_triplet(wid, 0);
	//t.v = 1;
	//unsigned char outp[20] = { 0 };
	myhash::Blake2b((char*)&mem, sizeof(mem), (unsigned char*)key20.buf);

}

//写入key-value: 
// H1(  F_K(w) ),      H2( F_K(w) ) XOR ( F_K(w||id1) || eid1)
// H1(  F_K(w||id1) ), H2( F_K(w||id1) ) XOR ( F_K(w||id2) || eid2)
// H1(  F_K(w||id2) ), H2( F_K(w||id2) ) XOR ( F_K(w||id3) || eid3)
// ....
Byte20 EII::WriteMap(EII_MAP& mp, Byte20 key,string w,ull id)
{
    //tree.
	Mem20 mem20NextKey;
	Byte20 b,indexKey;
	Byte36 b36Value,b36mask;
	H1(&key, &indexKey);
	H2(&key, &b36mask);
	WID_to_Key(w,id, mem20NextKey);
	EID eid= ID2EID(id);
	memcpy(b36Value.buf, mem20NextKey.buf, 20);
	memcpy(b36Value.buf + 20, eid.buf, 16);
	XOR(b36Value.buf, b36mask.buf, sizeof(b36Value));
	mp[indexKey] = b36Value;
	return *(Byte20*)&mem20NextKey;
}

Byte20 EII::ReadMap(EII_MAP& mp, Byte20& key, EID& eid)
{
	//tree.
	Mem20 mem20NextKey;
	Byte20 b, indexKey;
	Byte36 b36Value, b36mask;
	H1(&key, &indexKey);
	H2(&key, &b36mask);
	Byte20 empty = { 0 };
	if (map.find(indexKey) == map.end())
	{
		memset(eid.buf, 0, sizeof(eid));
		return empty;//返回空
	}
	b36Value = map[indexKey];
	XOR(b36Value.buf, b36mask.buf, sizeof(b36Value));

	memcpy(mem20NextKey.buf, b36Value.buf, 20);//生成nextkey
	memcpy(eid.buf, b36Value.buf+20, 16);	
	return *(Byte20*)&mem20NextKey; //nextKey 用于解密下一个节点，他是用当前的（w，id）加密构成，用于读取OUtree对应的叶子到根的路径
}

void EII::Setup(InvertedIndexMap& db)
{
	ull N = 0;
	
	for (auto& p : db)
	{
		string w = p.first;
		TOKEN tk = GenToken(w);
		Byte20 b20 = *(Byte20*)&tk;
		for (auto id : p.second)
		{
			b20 = WriteMap( map, b20, p.first,id);
			N++;
		}
	}
}
void EII::Add(string w,vector<ull>& ids)
{
		TOKEN tk = GenToken(w);
		Byte20 b20 = *(Byte20*)&tk;
		for (auto id : ids)
		{
			b20 = WriteMap(map, b20, w, id);
			N++;
		}
}
void EII::insertBatch(string w, ull from, ull to)
{
	TOKEN tk = GenToken(w);
	Byte20 b20 = *(Byte20*)&tk;
	for (ull id=from;id<to;id++)
	{
		b20 = WriteMap(map, b20, w, id);
		N++;
	}
}


void EII::Search(string w,vector<ull>& ids)
{
	double time_1 = time_ms();
	TOKEN tk = GenToken(w); //客户端生成令牌
	Byte20 t = *(Byte20*)&tk;
	EID eid;
	vector <EID> resultIDs;
//	unordered_map<ull, OUENode> resultFromTree;
	//提交云查询
	while (*(ull*)&t.buf!=0)
	{
		t = ReadMap(map, t, eid);
		if (*(ull*)&eid.buf == 0) break;//为空
		resultIDs.push_back(eid);
//		NODE_ID leaf=ptree->Byte20_to_leaf(t.buf);
//		vector<OUENode> r=ptree->ReadPath(leaf);
//		for (auto ed : r)
//		{
//			CPath64 p;
//			p.LoadPath(ed.path);
//			ull id = p.value;
//			resultFromTree[id] = ed;
//		}
	}
	//返回到客户端，由客户端解密

	for (auto r : resultIDs)
	{
		ull id = EID2ID(r);
//		WID wid = { w,id };
		ids.push_back(id);
	}

//	debug("search_time.txt","\r\n results=%ld time=%lf(ms) stash=%d MaxRootStash=%d Z=%d L=%d N=%lld", results.size(),time_2-time_1,ptree->current_stash_size_used,ptree->max_stash_size_used, ptree->Z_Constant, ptree->L,N);
	//FILE* fp = fopen("experiment.txt", "ab+");
	//fprintf(fp,"\r\n %s results=%ld time=%lf(ms) stash=%d MaxRootStash=%d Z=%d L=%d N=%lld",Now().c_str(), results.size(), time_2 - time_1, ptree->current_stash_size_used, ptree->max_stash_size_used, ptree->Z_Constant, ptree->L,N);
	//fclose(fp);

}


//m是关键词个数，len是平均的DB（w）长度
void EII::test_ini(int m, int len,int Z)
{
	InvertedIndexMap ii;// = { {"a",{1,2,3,4,5,6,7}},{"b",{1,2}} };
//N=2^10  2^15  2^20
	for (int i = 0; i < m; i++)//关键词个数 ，对每一个关键词
	{
		string w = "w";
		w += IntToStr(i);
		for (int j = 0; j < len; j++)
		{
			ii[w].push_back(j);
		}
	}
	double t1 = time_ms();
	Setup(ii);
	double t2 = time_ms();
	//debug("\r\n init speed:%f pairs/s, time=%f (ms)", m * len / (t2 - t1)*1000.,t2-t1);
//	debug("experiment.txt", "\r\n init speed:%f pairs/s, time=%f (ms), N=%lld,L=%d,Z=%d,Max_leaf_size=%d", m * len / (t2 - t1) * 1000., t2 - t1,ptree->N,ptree->L, ptree->Z_Constant,ptree->Max_leaf_size);
}

//m是关键词个数，w1长度是1  w100的长度是100, w500的结果集长度是500
void EII::test_ini2(int m, int max_len, int Z)
{
	InvertedIndexMap ii;// = { {"a",{1,2,3,4,5,6,7}},{"b",{1,2}} };
//N=2^10  2^15  2^20
	for (int i = 1; i <= m; i++)//关键词个数 ，对每一个关键词
	{
		string w = "w";
		w += IntToStr(i);
		for (int j = 1; (j<=i) && (j <= max_len); j++)
		{
			ii[w].push_back(j);
		}
	}
	double t1 = time_ms();
	Setup(ii);
	double t2 = time_ms();
	//debug("\r\n init speed:%f pairs/s, time=%f (ms)", m * len / (t2 - t1)*1000.,t2-t1);
//	debug("experiment.txt", "\r\n init speed:%f pairs/s, time=%f (ms), N=%lld,L=%d,Z=%d,Max_leaf_size=%d", ptree->N / (t2 - t1) * 1000., t2 - t1, ptree->N, ptree->L, ptree->Z_Constant, ptree->Max_leaf_size);
}
