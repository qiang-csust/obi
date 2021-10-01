// ConsoleApplication1.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#define _CRT_SECURE_NO_WARNINGS
#include "common.h"
#include <conio.h> //kbhit
#include <iostream>
#include <unordered_map>
#include "CPath64.h"
#include "myhash.h"
#include "myAES.h"
#include "ORAMtree.h"
#include "CInvertedIndex.h"
#include "CashSSE.h"
#include <vector>
#include <string>
using namespace std;


void test1()
{
	ORAMtree tree(5);
	tree.add("keyword", 99);
	tree.add("keyword", 100);
	tree.add("keyword", 101);
	tree.add("keyword2", 1);
	tree.add("keyword2", 2);

	vector<ull> v;
	tree.search("keyword", v);
	tree.search("keyword2", v);
	tree.search("keyword", v);
	tree.search("keyword", v);
	tree.dumpLevel();
}
void test2()
{
	ORAMtree tree(15);
	tree.add("keyword", 99);
	tree.add("keyword", 100);
	tree.add("keyword", 101);
	tree.add("keyword2", 1);
	tree.add("keyword2", 2);
	for (int i = 0; i < tree.P.capacity / 2; i++)
	{
		tree.add(IntToStr(i), i * 2);
	}

	vector<ull> v;
	tree.search("keyword", v);
	tree.search("keyword2", v);
	tree.search("keyword", v);
	tree.search("keyword", v);
}
void test3()
{
	ORAMtree tree(4,4,4);
	for (int i = 0; i < tree.P.capacity*4; i++)
	{
		tree.add(IntToStr(i), 100 + i, true);
	}
	vector<ull> v;
	tree.dump();
	tree.dumpStashUsage();
}
void testCorrectness()
{
	ORAMtree tree(10);
	int c = 0;
	for (int i = 0; i < tree.P.capacity; i++)
	{
		tree.add(IntToStr(i), 100 + i);
	}
	for (int i = 0; i < tree.P.capacity; i++)
	{
		vector<ull> ids;
		tree.search(IntToStr(i), ids,false);
		if (ids.size() == 0)
		{
			printf("\r\n error : i=%d", i);
			c++;
		}
		else
		{
			if (ids[0] != 100 + i)
			{
				printf("\r\n error id : i=%d ids[0]=%lld", i,ids[0]);
				c++;
			}
		}
	}
	if(c==0) printf("\r\n testing OK!");
	else
		printf("\r\n errors=%d", c);
}

void testCorrectness2()
{
	ORAMtree tree(10);
	int c = 0;
	for (int i = 0; i < tree.P.capacity/10; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			tree.add(IntToStr(i), 100 + i+j);
		}
	}
	//tree.dump();
	for (int i = 0; i < tree.P.capacity/10; i++)
	{
		vector<ull> ids;
		tree.search(IntToStr(i), ids, false);
		if (ids.size() != 10)
		{
			printf("\r\n error : i=%d", i);
			c++;
		}
		else
		{
			for (int j = 0; j < 10; j++)
			{
				if (ids[j] != 100 + i+j)
				{
					printf("\r\n error id : i=%d ids[%d]=%lld", i, j,ids[0]);
					c++;
				}
			}
		}
	}
	if (c == 0) printf("\r\n testing OK!");
	else
		printf("\r\n errors=%d", c);
}
//test deletions
void testCorrectness3()
{
	ORAMtree tree(12);
	int c = 0;
	vector<ull> ids;
	tree.add("a", 100);
	tree.add("a", 101);
	tree.add("a", 102);
	for (int i = 0; i < tree.P.capacity / 100; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			tree.add(IntToStr(i), 100 + i + j);
		}
	}
	for (int i = 0; i < tree.P.capacity / 100; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			tree.del(IntToStr(i), 100 + i + j);
		}
		tree.search(IntToStr(i), ids, false);
		if (ids.size() > 0)
		{
			printf("\r\n error : i=%d", i);
			c++;
		}
	}
	tree.del("a", 101);
	tree.search("a", ids);

	if (c == 0) printf("\r\n testing OK!");
	else
		printf("\r\n errors=%d", c);
}
void testCorrectness4()
{
	ORAMtree tree(12);
	int c = 0;
	vector<ull> ids= { 100,101,102,103,104 };
	tree.add("a", ids);
	tree.del("a", 103);
	tree.search("a", ids);

	if (c == 0) printf("\r\n testing OK!");
	else
		printf("\r\n errors=%d", c);
}
void testDistribution()
{
	double opCount = 0;
	for (int L = 7; L <= 25; L++)
	{
		ORAMtree tree(L,12,8);
		double t1 = time_ms();
		opCount = 0;
		for (int i = 0; i < tree.P.capacity/2; i++)
		{
			tree.add(IntToStr(i), 100 + i);
			opCount++;
		}
		double t2 = time_ms();
		vector<ull> ids;
		tree.search("1", ids);
		printf("\r\n L=%d S=%d Z=%d insertion time: %lf (ms)/%d blocks", L,tree.P.S,tree.P.Z, t2 - t1,opCount);
		Log("distribution.txt", "\r\n L=%d S=%d Z=%d B=%d (bytes) capacity=%lld op=%lf insertion time: %lf (ms) speed:%lf (entry/ms) insertion items:%lld exceedS:%d max_S:%d", L, tree.P.S, tree.P.Z, sizeof(DataBlock),tree.P.capacity, opCount, t2 - t1, opCount / (t2 - t1), tree.P.capacity, tree.P.n_root_exceed_S,tree.P.max_S);
		tree.dumpLevel();
	}
}
void testRootExceedS()
{
	ORAMtree tree(15,10,10);
	vector<ull> ids;
	ull total = 0;
	for (int i = 0; i < tree.P.capacity / 100; i++)
	{	
		for (int j = 0; j < 2; j++)
		{
			tree.add(IntToStr(i), i * 2+j); total++;
		}
	}
	for(int j=0;j<100;j++)
	for (int i = 0; i < tree.P.capacity; i++)
	{
		tree.search(IntToStr(i%(tree.P.capacity / 100)),ids,false);
	}
	//tree.dump();
	printf("\r\n Exceed s=%d total=%lld", tree.P.n_root_exceed_S, total);
}


void testefficiency()
{
	int L = 16;
	int S = 4;
	int Z = 4;
	vector<ull> ids;
	ORAMtree tree(L, S, Z);
	double t1 = time_ms();
	tree.add("10", 88);
	tree.add("10", 99);
	tree.add("10", 100);
	tree.del("10", 88);
	for (int i = 0; i < tree.P.capacity*Z*0.7; i++)
	{
		tree.add(IntToStr(i), 100 + i,true);
	}
	for (int i = 0; i < tree.P.capacity*Z*0.7 ; i++)
	{
		tree.search(IntToStr(i), ids,false);
	}
	tree.dumpLevel();

	tree.search("10", ids);
	tree.search("10", ids);
	tree.search("10", ids);
	//tree.search("abc",)
}
void exp1_loadfactor()
{
	int L = 14;//10 12 14
	int S = 6;
	int Z = 6;
	vector<ull> ids;
	ORAMtree tree(L, S, Z);
	double t1 = time_ms();
	double beta =0;
	ull c = tree.P.capacity*Z*0.05;
	for (int i = 0; i < tree.P.capacity*Z; i++)
	{
		if ((i%c)==0)
		{
			beta = (double)i/(tree.P.capacity*Z);
			printf("\r\n beta=%lf", beta);
			tree.dumpStashUsage();
			printf("\r\n %lf---------------------", beta);

		}
		tree.add(IntToStr(i), 100 + i, true);

	}
	tree.dumpStashUsage();
}
void exp2_stashsize()
{
	int L = 0;
	int S = 0;
	int Z = 0;
	vector<ull> ids;
	for (L = 10; L <= 20; L+=2)
	{
		for (Z = 4; Z <= 8; Z+=2)
		{
			S = Z;
			ORAMtree tree(L, S, Z);
			double t1 = time_ms();

			double beta = 0;
			for (int i = 0; i < tree.P.capacity; i++) //
			{
				tree.add(IntToStr(i), 100 + i, true);
			}
			tree.dumpStashUsage();			
		}
	}

}
void exp3_distribution()
{
	double opCount = 0;
	for (int L = 16; L <= 16; L++)
	{
		ORAMtree tree(L, 6, 6);
		double t1 = time_ms();
		opCount = 0;
		for (int i = 0; i < tree.P.capacity; i++)
		{
			tree.add(IntToStr(i), 100 + i);
			opCount++;
		}
		double t2 = time_ms();
		Log("distribution.txt", "\r\n L=%d S=%d Z=%d B=%d (bytes) capacity=%lld op=%lf insertion time: %lf (ms) speed:%lf (entry/ms) insertion items:%lld exceedS:%d Max_S:%d", L, tree.P.S, tree.P.Z, sizeof(DataBlock), tree.P.capacity, opCount, t2 - t1, opCount / (t2 - t1), tree.P.capacity, tree.P.n_root_exceed_S, tree.P.max_S);
		printf("\r\n L=%d S=%d Z=%d B=%d (bytes) capacity=%lld op=%lf insertion time: %lf (ms) speed:%lf (entry/ms) insertion items:%lld exceedS:%d Max_S:%d", L, tree.P.S, tree.P.Z, sizeof(DataBlock), tree.P.capacity, opCount, t2 - t1, opCount / (t2 - t1), tree.P.capacity, tree.P.n_root_exceed_S, tree.P.max_S);

		//tree.dump();
		tree.dumpLevel();
	}
}
void exp4_bandwidth()
{
	vector<ull> ids;
	//int L = 16;


	double t1 = time_ms();
	for (int L = 10; L <= 32; L+=2)
	{
		ORAMtree tree(L, 4, 4);
		double t2 = time_ms();
		tree.P.band_width = 0;
		tree.add(IntToStr(L) +"100", 12345);
		printf("\r\n addition:band_width=%d(bytes) u=%d L=%d Z=%d", tree.P.band_width, BLOCK_SIZE_B, tree.P.L, tree.P.Z);

		tree.P.band_width = 0;
		tree.del(IntToStr(L) + "100", 12345);
		printf("\r\n lazy deletion:band_width=%d(bytes) u=%d L=%d Z=%d", tree.P.band_width, BLOCK_SIZE_B, tree.P.L, tree.P.Z);

	}
	printf("\r\n--------------------------");
	for (int L = 10; L <= 32; L += 2)
	{
		ORAMtree tree(L, 4, 4);
		for (int i = 0; i < 1; i++)
		{
			tree.add("a1", 1000);
		}
		tree.P.band_width = 0;
		tree.search("a1", ids,false); //r_w=1
		printf("\r\n r_w=1 search:band_width=%d(bytes) u=%d L=%d Z=%d", tree.P.band_width, BLOCK_SIZE_B, tree.P.L, tree.P.Z);

		for (int i = 0; i < 25; i++)
		{
			tree.add("a2", 1000 + i);
		}
		tree.P.band_width = 0;
		tree.search("a2", ids,false); //r_w=50
		printf("\r\n r_w=25 search:band_width=%d(bytes) u=%d L=%d Z=%d", tree.P.band_width, BLOCK_SIZE_B, tree.P.L, tree.P.Z);

		for (int i = 0; i < 50; i++)
		{
			tree.add("a3", i + 1000);
		}
		tree.P.band_width = 0;
		tree.search("a3", ids,false); //r_w=5000
		printf("\r\n r_w=50 search:band_width=%d(bytes) u=%d L=%d Z=%d", tree.P.band_width, BLOCK_SIZE_B, tree.P.L, tree.P.Z);
	}
}

void exp5_insertion_speed()
{
	// BLOCK_SIZE_B 1 32
	int Z = 6;// 4 6
	vector<ull> ids;
	//int L = 16;

	double t1 = time_ms();
	int counter = 0;
	for (int L = 10; L <= 32; L += 2)
	{
		ORAMtree tree(L, Z, Z);
		counter = 0;
		double t1 = time_ms();
			for (int i = 0; i < tree.P.capacity / 2; i++)
			{
				tree.add(IntToStr(i), 100 + i);
				counter++;
				if (counter > 1000) break;
			}
		double t2 = time_ms();
		
		tree.P.band_width = 0;
		tree.add(IntToStr(L) + "100", 12345);
		printf("\r\n insertion speed %lf (pairs/ms) u=%d L=%d Z=%d", counter*BLOCK_SIZE_B /(t2-t1), BLOCK_SIZE_B, tree.P.L, tree.P.Z);
	}
}
void exp6_search_time()
{
	// BLOCK_SIZE_B (u) 1 32 64 128
	int Z = 4;// 4 6
	vector<ull> ids;
	int r = 200;//1 50 100 150 200
	//int L = 16;

	double t1 = time_ms();
	int counter = 0;
	for (int L = 10; L <= 32; L += 2)
	{
		ORAMtree tree(L, Z, Z);
		counter = 0;
				
		for (int i = 0; i < r; i++)
		{
			tree.add("100", 100 + i);
			counter++;
			if (counter > 1000) break;
		}
		
		tree.P.band_width = 0;
		double t1 = time_ms();
		tree.search("100", ids,false);
		double t2 = time_ms();
		printf("\r\n search time %lf (ms) r=%d u=%d L=%d Z=%d", (t2 - t1), r,BLOCK_SIZE_B, tree.P.L, tree.P.Z);
	}
}

void testInvertedIndex()
{
	CInvertedIndex index;
	index.LoadDirToInvertedIndex("E:\\OBindex\\test");
	printf("N=%lld m=%lld n=%lld \r\n", index.N, index.m, index.n);
	index.InsertFile("E:\\OBindex\\test2\\ee.txt");
	printf("N=%lld m=%lld n=%lld \r\n", index.N, index.m, index.n);
	getchar();
//	index.Save();
	index.Load();
	printf("N=%lld m=%lld n=%lld \r\n", index.N, index.m, index.n);
	getchar();
}
void testLocalSetup()
{
	CInvertedIndex index;
	index.LoadDirToInvertedIndex("E:\\OBindex\\test");
	ORAMtree r(10);
	r.SetupLocal(index.DB);
	r.dump();
	vector<ull> ids;
	r.search("a",ids,true);
	//printf("stash=%d\r\n", r.stash.size());
	r.dumpStashUsage();
	getchar();
}
CInvertedIndex g_index;


void ShowHelp()
{
	printf("-cash  # enter the cash SSE.\r\n");
	printf(" -testsearch 1000 keyword1 keyword2 keyword3  # search each keyword 1000 times.\r\n");
	printf(" -testsearch 1000 keyword1 keyword2 keyword3  # search each keyword 1000 times; watch the stash.\r\n");
	printf("-del w 100  #delete (w, 100)\r\n ");
	printf("-addlocalrange w 100 200 #insert (w,100),(w,101),...(w,200) into the stash\r\n");
	printf("-addrange w 100 200  #insert (w,100),(w,101),...(w,200) into the index\r\n");
	printf("-dump #dump the tree\r\n");
	printf("-add w 100  #add a (w,100) pair\r\n");
	printf("-L #set the tree height\r\n");
	printf("-level  #dump the load factor of each level\r\n");
	printf("-path  #set to the PATH-ORAM eviction algorithm\r\n");
	printf("-b  #set to the KNNEA eviction algorithm \r\n");
	printf("-loadstash 100000 1000 #load the inverted index to the local stash N=100000 m=1000\r\n");
	printf("-save  #save the oblivious index\r\n");
	printf("-load  #load the oblivious index\r\n");
	printf("-info  #show the information\r\n");

};


void testCMD(ORAMtree &ORAMtree)
{
	string line;
	cout << "please input a keyword for search:" << endl;
	while (getline(cin, line))  
	{
		vector<string> k;
		split(line, " ", k);
		if (_strcmpi(k[0].c_str(), "-help") == 0)
		{
			ShowHelp();
			continue;
		}
		if (_strcmpi(k[0].c_str(), "-cash") == 0)
		{
			printf("cash et al. 's SSE \r\n");
			testCMDCash();
			continue;
		}
		if (_strcmpi(k[0].c_str(), "-testsearch") == 0)//stash
		{
			vector<string> k;
			split(line, " ", k);
			double t1 = time_ms();
			if (k.size() < 3)
			{
				printf("size <3;\r\n");
				printf("eg: -testsearch 1000 keyword1 keyword2 keyword3");
				continue;
			}
			int times = atoi(k[1].c_str());
			vector<ull> ids;
			for (int i = 0; i < times; i++)
			{
				for (int x = 2; x < k.size(); x++)
				{
					if (_kbhit()) {
						i = times; break;
					};
					double t3 = time_ms();
					ORAMtree.search(k[x], ids, false);
					double t4 = time_ms();
					Log("exp.txt", "【%d】 search %s, results=%d, time=%lf, stash=%d\r\n",i, k[x].c_str(), ids.size(), t4-t3,ORAMtree.stash.size());
					if (ORAMtree.stash.size() > ORAMtree.P.max_Stash_Evict)
					{
						double t5 = time_ms();
						ORAMtree.Evict();//
						double t6 = time_ms();
						Log("exp.txt", "evict  algorithm=%d time=%lf,stash=%d\r\n", ORAMtree.P.shuffle_choice,t6-t5, ORAMtree.stash.size());
					}
				}
			}
			//ORAMtree.insertBatch(k[1], atoi(k[2].c_str()), atoi(k[3].c_str()));
			double t2 = time_ms();
			ORAMtree.dumpStashUsage();
			Log("exp.txt","testsearch OK; time=%f (ms)\r\n", t2 - t1);
			continue;
		}
		if (_strcmpi(k[0].c_str(), "-del") == 0)//插入1个 (w,id) pair
		{
			vector<string> k;
			split(line, " ", k);
			if (k.size() != 3)
			{
				printf("size !=3;\r\n");
				continue;
			}
			ORAMtree.del(k[1], atoi(k[2].c_str()));
			printf("deletion OK;\r\n");
			continue;
		}
		if (_strcmpi(k[0].c_str(), "-addlocalrange") == 0)//插入多个 (w,id) pair
		{
			vector<string> k;
			split(line, " ", k);
			double t1 = time_ms();
			if (k.size() != 4)
			{
				printf("size !=4;\r\n");
				printf("eg: -addlocalrange w 100 200 # insertion (w,100),(w,101),...(w,200)\r\n");
				continue;
			}
			ORAMtree.insertBatchLocal(k[1], atoi(k[2].c_str()), atoi(k[3].c_str()));
			double t2 = time_ms();
			printf("insertBatchLocal OK; time=%f (ms)\r\n", t2 - t1);
			continue;
		}
		if (_strcmpi(k[0].c_str(), "-addrange") == 0)//插入多个 (w,id) pair
		{
			vector<string> k;
			split(line, " ", k);
			double t1 = time_ms();
			if (k.size() != 4)
			{
				printf("size !=4;\r\n");
				printf("eg: -addrange w 100 200 # insertion (w,100),(w,101),...(w,200)\r\n");
				continue;
			}
			ORAMtree.insertBatch(k[1], atoi(k[2].c_str()), atoi(k[3].c_str()));
			double t2 = time_ms();
			printf("insertBatch OK; time=%f (ms)\r\n", t2 - t1);
			continue;
		}
		if (_strcmpi(k[0].c_str(), "-dump") == 0)//插入1个 (w,id) pair
		{
			ORAMtree.dumpdata();
			printf("dump OK;\r\n");
			continue;
		}
		if (_strcmpi(k[0].c_str(), "-add") == 0)//插入1个 (w,id) pair
		{
			vector<string> k;
			split(line, " ", k);
			if (k.size() != 3)
			{
				printf("size !=3;\r\n");
				continue;
			}
			ORAMtree.add(k[1], atoi(k[2].c_str()));
			printf("insertion OK;\r\n");
			continue;
		}
		if (_strcmpi(k[0].c_str(), "-L") == 0)//
		{
			vector<string> k;
			split(line, " ", k);
			if (k.size() != 2)
			{
				printf("size !=2;\r\n");
				continue;
			}
			ORAMtree.P.L = atoi(k[1].c_str());
			ORAMtree.init();
			ORAMtree.dumpStashUsage();
			continue;
		}
		if (_strcmpi(k[0].c_str(), "-level") == 0)//
		{
			ORAMtree.dumpLevel();
			printf("\r\n dump OK;\r\n");
			continue;
		}
		if (_strcmpi(k[0].c_str(), "-setmaxstash") == 0)//
		{
			vector<string> k;
			split(line, " ", k);
			if (k.size() != 2)
			{
				printf("size !=2;\r\n");
				continue;
			}
			ORAMtree.P.max_Stash_Evict = atoi(k[1].c_str());
			ORAMtree.dumpStashUsage();
			continue;
		}
		//
		if (_strcmpi(k[0].c_str(), "-path") == 0)// pathORAM eviction
		{
			vector<string> k;
			split(line, " ", k);
			ORAMtree.P.shuffle_choice = ALGORITHM_PATHORAM;//老算法 不分区
			ORAMtree.debug.PATH_ORAM_chooseTime = 0;
			double t1 = time_ms();
			if (k.size() == 1)
			{
				ORAMtree.Evict0();
			}
			else
			{
				ORAMtree.Evict0(atoi(k[1].c_str()));//读取并驱除 stash*L*L*倍数 的路径
			}
			double t2 = time_ms();
			printf("non-partition Eviction OK! time=%lf (ms) candidate time=%lf (ms)\r\n", t2 - t1,ORAMtree.debug.PATH_ORAM_chooseTime);
			ORAMtree.dumpStashUsage();
			continue;
		}
		if (_strcmpi(k[0].c_str(), "-b") == 0)// bottom_to_top
		{
			vector<string> k;
			split(line, " ", k);
			ORAMtree.P.shuffle_choice = ALGORITHM_BOTTOM_TO_TOP;//老算法 不分区
			double t1 = time_ms();
			if (k.size() == 1)
			{
				ORAMtree.Evict0();
			}
			else
			{
				ORAMtree.Evict0(atoi(k[1].c_str()));//读取并驱除 stash*L*L*倍数 的路径
			}
			double t2 = time_ms();
			printf("non-partition Eviction OK! time=%lf(ms) deletion Time=%lf(ms)\r\n",t2-t1,ORAMtree.debug.BottomToTop_removeTime);
			ORAMtree.dumpStashUsage();
			continue;
		}
		if (_strcmpi(k[0].c_str(), "-p") == 0)//-pbea
		{
			vector<string> k;
			split(line, " ", k);
			ORAMtree.P.shuffle_choice = ALGORITHM_PBEA;//新算法 分区
			double t1 = time_ms();
			if (k.size() == 1)
			{
				ORAMtree.Evict();
			}
			else
			{
				ORAMtree.Evict(atoi(k[1].c_str()));//读取并驱除 stash*L*L*倍数 的路径
			}
			double t2 = time_ms();
			printf("partition Eviction OK! time=%lf\r\n",t2-t1);
			ORAMtree.dumpStashUsage();
			continue;
		}
		if (_strcmpi(k[0].c_str(), "-loadstash") == 0)
		{
			//vector<string> k;
			//split(line, " ", k);
			if (k.size() != 3)
			{
				printf("e.g., -loadstash 100000 1000 \r\n");
				continue;
			}
			ull N = atoi(k[1].c_str());
			ull m = atoi(k[2].c_str());
			printf("Load index local N=%ld m=%ld...\r\n",N,m);

			double t1 = time_ms();
			if (g_index.DB.size() == 0)
			{
				g_index.Load(N, m);
			}
			else
			{
				printf("already loaded...\r\n");
			}
			double t2 = time_ms();
			printf("loadlocal completed:  time=%f...\r\n",  t2 - t1);
			printf("N=%lld m=%lld n=%lld \r\n", g_index.N, g_index.m, g_index.n);
			double t11 = time_ms();
			//ORAMtree.insertBatchLocal(k[1], atoi(k[2].c_str()), atoi(k[3].c_str()));
			for (auto& v : g_index.DB)
			{
				ORAMtree.add(v.first, v.second, false, false);//不驱逐
			}
			double t22 = time_ms();
			printf("insertion ORAMtree completed:  time=%f...\r\n", t22 - t11);
			continue;
		}

		if (_strcmpi(k[0].c_str(), "-save") == 0)//
		{
			printf("save the index...\r\n");
			ORAMtree.save();
			printf("index saving completed;\r\n");
			continue;
		}
		if (_strcmpi(k[0].c_str(), "-load") == 0)//
		{
			printf("load the index...\r\n");
			ORAMtree.load();
			printf("index loaded;\r\n");
			continue;
		}
		if (_strcmpi(k[0].c_str(), "-info") == 0)//
		{
			ORAMtree.dumpStashUsage();
			ORAMtree.info();
			continue;
		}

			
		int c1 = myhash::counter();
		vector<unsigned long long > ids;
		string w = line;
		ORAMtree.search(w, ids, true);
		int c2 = myhash::counter();
		printf("\r\n pseudo-random %d\r\n\r\n", c2 - c1);
		ORAMtree.dumpStashUsage();

		cout << "please input a keyword for search:" << endl;
		//cout << line << endl;
	}
}

void ProgramStart(int argc, char* argv[])
{
	string src;
	string des;
	int height = 22;
	bool bSetHeight = false;
	ull max_size = -1;//最大N; 
	ull max_m = -1;//最大的m 总关键词数
	bool bOnlineSetup = false;
	printf("\r\n Usage:\r\n", max_size);
	printf("-save  #save the index\r\n", max_size);
	printf("-load #load the index\r\n", max_size);
	printf("-add keyword 100  #add a keyword-identifier pair (keyword,100)\r\n", max_size);
	printf("-info #the index information\r\n");
	printf("-help #show helps\r\n");
	printf(" %s \r\n", Now().c_str());
	printf("\r\n");
	for (int i = 0; i < argc; i++)
	{
		if (strcmp(argv[i], "--max-size") == 0)
		{
			if (i + 1 < argc)
			{
				src = argv[i + 1];
				max_size = atoi(src.c_str());
				printf("max-size: %lld\r\n", max_size);
			}
			i += 1;			
		}
		if (strcmp(argv[i], "--max-m") == 0)
		{
			if (i + 1 < argc)
			{
				src = argv[i + 1];
				max_m = atoi(src.c_str());
				//printf("max-m: %lld\r\n", max_m);
			}
			i += 1;
		}
	}
	for (int i = 0; i < argc; i++)
	{
		if (strcmp(argv[i], "--create-inverted-index") == 0)
		{
			printf("Create index :%s ...\r\n",argv[i]);
			double t1 = time_ms();
			if (i + 1 < argc)
			{
				src = argv[i + 1];
				g_index.LoadDirToInvertedIndex(src, max_size);
			}
			i += 1;
			g_index.Save();
			double t2 = time_ms();
			printf("completed!  %s time=%f\r\n", Now().c_str(),t2-t1);
			printf("N=%lld m=%lld n=%lld \r\n", g_index.N, g_index.m, g_index.n);
			return;
		}
		if (strcmp(argv[i], "--set-height") == 0)
		{
			if (i + 1 < argc)
			{
				src = argv[i + 1];
				height = atoi(src.c_str());
				bSetHeight = true;
			}
			i += 1;
			g_index.Save();
		}
		if (strcmp(argv[i], "--load-inverted-index") == 0)
		{
			printf("Load index: %s ...\r\n", argv[i]);
			if (max_m != -1)
			{
				printf("max_m=%lld \r\n", max_m);
			}
			double t1 = time_ms();
			g_index.Load(max_size, max_m);
			double t2 = time_ms();
			printf("completed: %s  time=%f...\r\n", argv[i],t2-t1);
			printf("N=%lld m=%lld n=%lld \r\n", g_index.N, g_index.m, g_index.n);
		}
		if (strcmp(argv[i], "--online-setup") == 0)
		{
			bOnlineSetup = true;
		}
		
	}
	if (!bSetHeight)
	{
		if (g_index.DB.size() > 0)
		{		
			printf("set height=%d \r\n", height);
			height = log2(g_index.N) + 1; //设置高度
		}
	}
	ORAMtree ORAMtree(height); //BLOCK_SIZE_B
	printf("ORAMtree Z=%d L=%d\r\n", ORAMtree.P.Z,height);
	if (g_index.DB.size() > 0)
	{
		printf("inserting all the values...\r\n");
		double t1 = time_ms();
		if (bOnlineSetup)
		{
			printf("Online Setup ...\r\n");
			ORAMtree.SetupOnline(g_index.DB);
		}
		else
		{
			printf("Offline Setup ...\r\n");
			ORAMtree.SetupLocal(g_index.DB);
		};
		double t2 = time_ms();		
		printf("insertion complete time=%lf (ms) m=%lld\r\n",t2-t1,g_index.DB.size());
	}
	for (int i = 0; i < argc; i++)
	{
		if (strcmp(argv[i], "-save") == 0)
		{
			printf("Save oblivious index :%s ...\r\n", argv[i]);
			double t1 = time_ms();
			ORAMtree.save();
			double t2 = time_ms();
			printf("completed!  %s time=%f\r\n", Now().c_str(), t2 - t1);
			printf("N=%d m=%d n=%lld \r\n", g_index.N, g_index.m, g_index.n);
			return;
		}
		if (strcmp(argv[i], "-load") == 0)
		{
			printf("Load oblivious index: %s ...\r\n", argv[i]);
			double t1 = time_ms();
			ORAMtree.load();
			double t2 = time_ms();
			printf("completed: %s  time=%f...\r\n", argv[i], t2 - t1);
			printf("N=%lld m=%lld n=%lld \r\n", g_index.N, g_index.m, g_index.n);
		}
	}

	testCMD(ORAMtree);
}

void testPerformance()
{
	int L = 15;
	int S = 4;
	int Z = 4;
	vector<ull> ids;
	for (int i = 15; i < 23; i++)
	{
		ORAMtree tree(i, S, Z);

		for (int j = 0; j < 1024; j++)
		{
			ids.push_back(j);
		}
		tree.add("w", ids);
		tree.search("w", ids,true);
	}
}

int main(int c,char* argv[])
{
	//testLocalSetup(); getchar();
	//testPerformance(); getchar(); 
	printf("OBI v1.0, programmed by zhiqiang wu, wzq@csust.edu.cn\r\n\r\n");
	printf("Eg:\r\n");
	printf("obi --create-inverted-index d:\\myDirectory\r\n");
	printf("obi --load-inverted-index \r\n");
	printf("obi --create-inverted-index d:\\myDirectory -save\r\n");
	printf("obi  --create-inverted-index d:\\myDirectory --online-setup\r\n");// online create index
	printf("obi --set-height 10 --max-size 10000 --max-m 1000\r\n");
	printf("obi -load #load the index\r\n");
	printf("\r\n\r\n");
	ProgramStart(c, argv);
	getchar();
}


