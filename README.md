Thank you for deploying the OBI project!

Platform:

  Windows 10

The compiler:

  Visual Studio 2019 (v142)

Windows SDK:
  10.0

C++ Standard:

Preview - (/std:c++latest)

Other Package:
Crypto++. Thanks: https://www.cryptopp.com/

Data files：

   Enron Dataset：enron_mail_20150507.tgz. Thanks: https://www.cs.cmu.edu/~./enron/
   
   Full plain-text inverted index: the "enron" directory

Usage:
Run OBI.exe to enter a console.

Command-Line Commands:

 obi --create-inverted-index d:\\myDirectory
 
   #create an inverted index from a full directory containing plain-text files.
   
 obi --create-inverted-index d:\\myDirectory -save
 
   #save an inverted index
 obi --load-inverted-index
 
   #load an existing inverted index into memory.
   
 obi --create-inverted-index d:\\myDirectory --online-setup
 
   #online-setup an existing inverted index
   
 obi --set-height 10 --max-size 10000 --max-m 1000
 
   #load a part of the existing plain-text inverted index into memory with L=10, N=1000, and m=1000.
 obi -load 
 
   #load the full index

Console Commands:

  -testsearch 1000 keyword1 keyword2 keyword3  
  
  # search each keyword 1000 times.
  
  -testsearch 1000 keyword1 keyword2 keyword3  
  
  #search each keyword 1000 times
  
  -del w 100  
  
  #delete (w, 100)
  
  -addlocalrange w 100 200 
  
  #insert (w,100),(w,101),...(w,200) into the stash
  
  -addrange w 100 200  
  
  #insert (w,100),(w,101),...(w,200) into the index
  
  -dump 
  
  #dump the tree
  
 -add w 100  
 
  #add a (w,100) pair
  
  -L 10 
  
  #set the tree height
  
  -level  
  
  #dump the load factor of each level
  
  -path  
  
  #set to the PATH-ORAM eviction algorithm\r\n");
  
  -b  
  
  #set to the KNNEA eviction algorithm \r\n");
  
  -p  
  
  #set to the PBEA eviction algorithm \r\n");
  
  -loadstash 100000 1000 
  
  #load the inverted index to the local stash N=100000 m=1000
  
  -save  
  
  #save the oblivious index
  
  -load  
  
  #load the oblivious index
  
  -info  
  
  #show the information
  
//--------------------------------------------
		A C++ Demo using OBI:
		#include "common.h"
		#include "ORAMtree.h"
		#include <vector>
		#include <string>


		void testPerformance()
		{
			int L = 15; # The height of the tree
			int S = 4;  # The root-node size (here, S=Z)
			int Z = 4;  # The node size (i.e., the bucket size)
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
			testPerformance(); 
			getchar();
		}

//--------------------------------------------
		A C++ Demo using OMMAP:
		#include <vector>
		using namespace std;
		import qiang.csust.obi;

		void testOMMAP()
		{
			OMMAP tree(10, 4, 4);
			tree["info1"] = {"hello","world"};
			tree["info2"] = {"I","am", "OMMAP"};
			vector<IndexValue> v;
			tree.search("info1", v, true);
			tree.search("info2", v, true);
		}

		int main(int c,char* argv[])
		{
			testOMMAP();
			getchar();
		}













