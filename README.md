# OBI
Thank you for deploying the OBI project!

> Platform:
> 
>   Windows 10
>   
> The compiler:
> 
>   Visual Studio 2019 (v142)
>   
> Windows SDK:
> 
>   10.0
>   
> C++ Standard:
> 
>   Preview - (/std:c++latest)
>   
> Other Package:
> 
>   Crypto++. Thanks: https://www.cryptopp.com/
>   
> Data files：
> 
>   Enron Dataset：enron_mail_20150507.tgz. Thanks: https://www.cs.cmu.edu/~./enron/
>   
> Memory required
> 
>   \>40GB for the whole Enron dataset
 
# Compiling the project

Install "Visual Studio 2019", and "Windows SDK 10.0". Set the compiler to C++ 20 Standard.

Unzip "obi-visual studio2019.zip" or clone https://github.com/qiang-csust/obi.git, open the solution file "obi.sln", and compile the project.

# Indexing the Enron dataset

Unzip "obi-enron.zip"

Copy "OBI.exe" to the current directory.

Download "enron_mail_20150507.tgz" from "https://www.cs.cmu.edu/~./enron/"

Copy the unzipped Enron dataset to "E:\OBindex\maildir"

Run "createindex_enron.bat" to create a full plain-text inverted index.

Run "search.bat" to enter an OBI console.

# Command-Line Commands:

 obi --create-inverted-index d:\\myDirectory
 
   #create a plain-text inverted index from a full directory containing plain-text files.
   
 obi --create-inverted-index d:\\myDirectory -save
 
   #create an oblivious inverted index and save the oblivious index
   
 obi --load-inverted-index
 
   #load an existing inverted index into memory.
   
 obi --create-inverted-index d:\\myDirectory --online-setup
 
   #online-setup an existing inverted index
   
 obi --load-inverted-index --set-height 23 --max-size 10000 --max-m 1000
 
   #load a part of the existing plain-text inverted index into memory with L=23, N=1000, and m=1000.
   
 obi -load 
 
   #load the full oblivious index
   
# Usage:
Run OBI.exe to enter a console.

Console Commands:

  -load  
  
  #load the oblivious index
  
 -add w 100  
 
  #add a keyword-identifier pair (w,100)
    
  -info  
  
  #show the index information
  
  -testsearch 1000 keyword1 keyword2 keyword3  
  
  #repeat the search 1000 times for each keyword
   
  -del w 100
  
  #delete (w, 100)
  
  -addlocalrange w 100 200 
  
  #insert (w,100),(w,101),...(w,200) into the stash
  
  -addrange w 100 200  
  
  #insert (w,100),(w,101),...(w,200) into the index
  
  -dump 
  
  #dump the tree 
  
  -L 10 
  
  #set the tree height
  
  -level  
  
  #dump the load factor of each level
  
  -path  
  
  #set to the PATH-ORAM eviction algorithm;
  
  -b  
  
  #set to the KNNEA eviction algorithm;
  
  -p  
  
  #set to the PBEA eviction algorithm;
  
  -loadstash 100000 1000 
  
  #load the inverted index to the local stash N=100000 m=1000
  
  -save  
  
  #save the oblivious index
  
  # Demo
--------------------------------------------

		// A C++ Demo using OBI:
		
		#include "common.h"
		
		#include "ORAMtree.h"
		
		#include <vector>
		
		#include <string>


		void testPerformance()
		{
			int L = 15; // The height of the tree
			int S = 4;  // The root-node size (here, S=Z)
			int Z = 4;  // The node size (i.e., the bucket size)
			vector<unsigned long long> ids;
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

--------------------------------------------

		// A C++ Demo using OMMAP:
		
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













