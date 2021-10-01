#include "CashSSE.h"
#include <iostream>
#include "CInvertedIndex.h"


void testCMDCash()
{
	//	vb.SetIndexMode(true);//设置优化版本的最小索引
	string line;
	cout << "please input a keyword for search:" << endl;
	EII eii;
	while (getline(cin, line))//getline函数可以设置为getline(cin,line,' ')形式，那么就会以' '为分界来读取单词    
	{
		if (_strnicmp(line.c_str(), "-addrange", 9) == 0)//插入多个 (w,id) pair
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
			eii.insertBatch(k[1], atoi(k[2].c_str()), atoi(k[3].c_str()));
			double t2 = time_ms();
			printf("insertBatch OK; time=%f (ms)\r\n", t2 - t1);
			continue;
		}

		if (_strnicmp(line.c_str(), "-add", 4) == 0)//插入1个 (w,id) pair
		{
			vector<string> k;
			split(line, " ", k);
			if (k.size() != 3)
			{
				printf("size !=3;\r\n");
				continue;
			}
			vector<ull> ids;
			ids.push_back(atoi(k[2].c_str()));
			eii.Add(k[1], ids);
			printf("insertion OK;\r\n");
			continue;
		}


		if (_strnicmp(line.c_str(), "-save", 5) == 0)//保存索引
		{
			printf("save the index...\r\n");
			//eii.save();
			printf("index saving completed;\r\n");
			continue;
		}
		if (_strnicmp(line.c_str(), "-setup", 6) == 0)//
		{
			vector<string> k;
			split(line, " ", k);
			if (k.size() != 3)
			{
				printf("size !=3; \r\n e.g. -setup 10000000 3000\r\n");
				continue;
			}
			printf("Create index :%s ...\r\n", k[0]);
			CInvertedIndex g_index;
			double t1 = time_ms();
			g_index.Load(atoi(k[1].c_str()), atoi(k[2].c_str()));//载入一部分
			double t2 = time_ms();
			printf("completed!  %s time=%f\r\n", Now().c_str(), t2 - t1);
			printf("N=%lld m=%lld n=%lld \r\n", g_index.N, g_index.m, g_index.n);
			return;
			continue;
		}
		if (_strnicmp(line.c_str(), "-load", 5) == 0)//保存索引
		{
			printf("load the index...\r\n");
			//eii.load();
			printf("index loaded;\r\n");
			continue;
		}
		if (_strnicmp(line.c_str(), "-info", 5) == 0)//保存索引
		{
			//eii.info();
			continue;
		}
		if (_strnicmp(line.c_str(), "-exit", 5) == 0)//
		{
			//eii.info();
			printf("returned.\r\n");
			return;
		}


		int c1 = myhash::counter();
		vector<unsigned long long > results;
		string w = line;
		double time_start = time_ms();
		int aesCounter1 = myAES::counter();
		int Blake2bCounter1 = myhash::counter();
		eii.Search(w, results);
		int aesCounter2 = myAES::counter();
		int Blake2bCounter2 = myhash::counter();
		double time_end = time_ms();
		if (1)
		{
			printf("\r\n matched: count=%d time=%lf (ms) AES=%d Blake2b=%d\r\n", results.size(), time_end - time_start, aesCounter2 - aesCounter1, Blake2bCounter2 - Blake2bCounter1);
			int k = 0;
			for (auto r : results)
			{
				printf("id=%lld\r\n", r);
				if (k++ > 15)
				{
					printf(" \r\n ...");
					break;
				}
			}
		}
		cout << "Cash SSE: please input a keyword for search:\r\n" << endl;
		//cout << line << endl;
	}
}