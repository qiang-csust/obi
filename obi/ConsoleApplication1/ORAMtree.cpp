#include "ORAMtree.h"
#include "myAES.h"

ORAMtree::ORAMtree(int m_L, int m_S, int m_Z)
{
	P.L = m_L;
	P.S = m_S;
	P.Z = m_Z;
	init();
}

ORAMtree::ORAMtree(int m_L)
{
	P.L = m_L;
	init();

}
void ORAMtree::init()
{
	P.capacity = pow(2, P.L);// +1;
	P.firstLeafID = pow(2, P.L - 1) -1;
	P.leafCount = pow(2, P.L - 1);
	ull mem = P.capacity * P.Z * 5;//�ܹ���Ԫ�ظ�������5 ��֤��ȡ�ٶ�
	ull maxMem = pow(2, 26);
	if (mem > oram.max_bucket_count()) mem = oram.max_bucket_count();//����һ���ֿռ�
	if (mem > maxMem) mem = maxMem;//�����˻�����ڴ����
	oram.reserve(mem);
	initAES();
}

void ORAMtree::initAES()
{
	myAES::SetPrivateKey((char*)"12345678");
}

//del Ҳ�� add ������һ����־����
void ORAMtree::addBlock(string w, DataBlock block,  bool bForceAdd,bool bUseEvict )
{
	KeywordInfo info = { 0 };
	if (!bForceAdd)
	{
		if (P.w_block_count > P.capacity)
		{
			printf("\r\n MAX reaches: max=%lld, w_block_count=%lld", P.capacity, P.w_block_count);
			return;
		}
	}
	P.w_block_count++;
	if (localMap.find(w) == localMap.end())
	{
		info.searchCounter = 0;
		info.wLength = 1;
		localMap[w] = info;
	}
	else
	{
		info = localMap[w];
		info.wLength += 1;
		localMap[w] = info;
	}
	vector<ull> leafs;
	ull tk = CPath64(0).RandomLeaf(P.L);//����һ���������Ҷ�ڵ�
	leafs.push_back(tk);
	ENodes enodes1 = ReadPath(tk);
	WriteIntoStash(enodes1);

	ull futuretk = SearchToken(w, info.wLength, info.searchCounter);//future token
	IndexKey key = toIndexKey(w, info.wLength);
	DataBlock data=block;
//	data.data[0] = id;
//	for (int i = 1; i < BLOCK_SIZE_B; i++)
//	{
//		data.data[i] = EMPTY_BLOCK_VALUE;//����
//	}
	DataBlock empty;
	ReadWriteFromStash("write", futuretk, key, data, empty);
	
	if (bUseEvict) //����д�뵽�Ʒ�����
	{
		ENodes enodes = RebuildPaths(leafs, 0);
		writePath(enodes);
	}
}
void ORAMtree::Evict0(int times)
{
	int stashSize = stash.size();
	if (stashSize == 0) return;
	int v = stashSize * times;//��ȡ��·���ĸ���
	vector<ull> leafs;
	for (int i = 0; i < v; i++)//
	{
		ull tk = CPath64(0).RandomLeaf(P.L);//����һ���������Ҷ�ڵ�
		leafs.push_back(tk);
	};
	for (int i = 0; i < leafs.size(); i++)
	{
		ENodes enodes1 = ReadPath(leafs[i]);
		WriteIntoStash(enodes1);
	};

	ENodes enodes = RebuildPaths(leafs, 0); //�㷨1 �ƺ������⣬Ŀǰ�����㷨0
	writePath(enodes);
}

void ORAMtree::Evict(int times)
{
	int stashSize = stash.size();
	if (stashSize == 0) return;
	int v = stashSize * times+6;//��ȡ��·���ĸ���
	vector<ull> leafs;
	unordered_map<ull, int> old;
	for (int i = 0; i < v; i++)//
	{
		ull tk = CPath64(0).RandomLeaf(P.L);//����һ���������Ҷ�ڵ�
		if (old[tk] == 1) continue;
		leafs.push_back(tk);
		old[tk] = 1;
	};
	for (int i = 0; i < leafs.size(); i++)
	{
		ENodes enodes1 = ReadPath(leafs[i]);
		WriteIntoStash(enodes1);
	};

	ENodes enodes = RebuildPaths(leafs,1); //�㷨1 �ƺ������⣬Ŀǰ�����㷨0
	writePath(enodes);
}

//��������һ���飬��д�뵽 �ƶ˵� ORAMtree֮��
ull ORAMtree::addBlockLocal(string w, DataBlock block, bool bForceAdd)
{
	KeywordInfo info = { 0 };
	if (!bForceAdd)
	{
		if (P.w_block_count > P.capacity)
		{
			printf("\r\n MAX reaches: max=%lld, w_block_count=%lld", P.capacity, P.w_block_count);
			return 0;
		}
	}
	P.w_block_count++;
	if (localMap.find(w) == localMap.end())
	{
		info.searchCounter = 0;
		info.wLength = 1;
		localMap[w] = info;
	}
	else
	{
		info = localMap[w];
		info.wLength += 1;
		localMap[w] = info;
	}
	vector<ull> leafs;
	ull tk = CPath64(0).RandomLeaf(P.L);//����һ���������Ҷ�ڵ�
	leafs.push_back(tk);
	ENodes enodes1 = ReadPath(tk);
	WriteIntoStash(enodes1);

	ull futuretk = SearchToken(w, info.wLength, info.searchCounter);//future token
	IndexKey key = toIndexKey(w, info.wLength);
	DataBlock data = block;
	//	data.data[0] = id;
	//	for (int i = 1; i < BLOCK_SIZE_B; i++)
	//	{
	//		data.data[i] = EMPTY_BLOCK_VALUE;//����
	//	}
	DataBlock empty;
	ReadWriteFromStash("write", futuretk, key, data, empty);

//  ע�⣺���ﲻд�뵽�ƶ˵ķ�����֮�С�
//	ENodes enodes = RebuildPaths(leafs);
//	writePath(enodes);
	return tk;//����һ��Ҷ��
}



void ORAMtree::add(string w, unsigned long long id,bool bForce)
{
		DataBlock data;
		data.data[0] = id;
		for (int i = 1; i < BLOCK_SIZE_B; i++)
		{
			data.data[i] = DUMMY_VALUE;//����
		}
		addBlock(w, data, bForce);
}
void ORAMtree::add(string w, vector<ull> ids,bool bForce,bool bUseEvict)//��������
{
	DataBlock data= EmptyBlock();
	int j = 0;
	for (int i = 0; i < ids.size(); i++)
	{
		data.data[j] = ids[i];
		j++;
		if (j == BLOCK_SIZE_B)
		{
			addBlock(w, data, bForce, bUseEvict);
			data = EmptyBlock();
			j = 0;
		}
	}
	if (!isEmptyBlock(data))
	{
		addBlock(w, data,false, bUseEvict);
	}
}
void ORAMtree::insertBatch(string w, ull from, ull to)//��������
{
	vector<ull> ids;
	for (ull i = from; i <= to; i++)
	{
		ids.push_back(i);
		if (ids.size() == BLOCK_SIZE_B)
		{
			add(w, ids, false);
			ids.clear();			
		}
	}
	if (ids.size() > 0)
	{
		add(w, ids, false);
		ids.clear();
	}
}
void ORAMtree::insertBatchLocal(string w, ull from, ull to)//��������
{
	vector<ull> ids;
	for (ull i = from; i <= to; i++)
	{
		ids.push_back(i);
		if (ids.size() == BLOCK_SIZE_B)
		{
			add(w, ids, false,false);;//������
			ids.clear();
		}
	}
	if (ids.size() > 0)
	{
		add(w, ids, false,false);//������
		ids.clear();
	}
}



//�ӵ�����������
void ORAMtree::SetupLocal(unordered_map<string,vector<ull>>& inverted_index)//��������
{
	ull tk = 0;
	vector<ull> leafs;
	for (auto& v : inverted_index)
	{
		string w = v.first;
		vector<ull> ids = v.second;
		DataBlock data = EmptyBlock();
		int j = 0;
		for (int i = 0; i < ids.size(); i++)
		{
			data.data[j] = ids[i];
			j++;
			if (j == BLOCK_SIZE_B)
			{
				tk=addBlockLocal(w, data, false);
				data = EmptyBlock();
				j = 0;
				leafs.push_back(tk);
			}
		}
		if (!isEmptyBlock(data))
		{
			tk=addBlockLocal(w, data,false);
			leafs.push_back(tk);
		}
	}
//	ENodes enodes = RebuildPaths(leafs);
	ENodes enodes = RebuildPaths_initial();
	writePath(enodes);
}
void ORAMtree::SetupOnline(unordered_map<string, vector<ull>>& inverted_index)//��������
{
	ull tk = 0;
	vector<ull> leafs;
	for (auto& v : inverted_index)
	{
		string w = v.first;
		vector<ull> ids = v.second;
		add(w, ids,false);
	}
	//	ENodes enodes = RebuildPaths(leafs);
}

ull ORAMtree::addLocal(string w, vector<ull> ids, bool bForce)
{
	DataBlock data = EmptyBlock();
	int j = 0;
	ull tk = 0;
	for (int i = 0; i < ids.size(); i++)
	{
		data.data[j] = ids[i];
		j++;
		if (j == BLOCK_SIZE_B)
		{
			tk=addBlockLocal(w, data, bForce);
			data = EmptyBlock();
			j = 0;
		}
	}
	if (!isEmptyBlock(data))
	{
		tk=addBlockLocal(w, data,bForce);
	}
	return tk;
}

//del Ҳ�� add ������һ����־����
void ORAMtree::del(string w, unsigned long long id)
{
	ull id1 = toDelIdentifer(id);
	add(w, id1);
}

void ORAMtree::search(string w, vector<unsigned long long>& ids, bool bDebug)
{
	KeywordInfo info = { 0 };
	ids.clear();
	P.aesCounter1 = myAES::counter();
	P.Blake2bCounter1 = myhash::counter();
	int n_root_exceed_S1 = P.n_root_exceed_S;
	double time_start = time_ms();
	if (localMap.find(w) == localMap.end())
	{
		printf("\r\n not found: %s \r\n", w.c_str());
		return;
	}
	else
	{
		info = localMap[w];
	}
	vector<ull> tokens;
	unordered_map<ull, ull> filter;
	if (info.wLength == 0)
	{
		printf("\r\n not found: %s \r\n", w.c_str());
		return;//�˹ؼ����Ѿ���ɾ��
	};
	for (int i = 1; i <= info.wLength; i++)
	{
		unsigned long long tk = SearchToken(w, i, info.searchCounter);
		if (filter[tk] == 1) continue;
		filter[tk] = 1;//ȥ���ظ�
		tokens.push_back(tk);
		CPath64 debugPath;
		debugPath.value = tk;
		string path = debugPath.ToString();
	}

	//	static ulong g_seed = 0; //���㷨�����⣬ûʲô��
	//	for (int i = 1; i <= stash.size() ; i++) //�˴������������������Ҷ�ӣ���������
	//	{
	//		ulong tk = SearchToken(string("seed:")+IntToStr(g_seed++)+string(":"), i, 0);//���������
	//		if (filter[tk] == 1) continue;
	//		filter[tk] = 1;//ȥ���ظ�
	//		tokens.push_back(tk);//�����ȡһ��·�������ݣ�Ԥ����ѯ��
	//	}
	ENodes enodes1 = ReadPaths(tokens);
	double time_transfer = time_ms();
	WriteIntoStash(enodes1);
	vector<ull> results;
	vector<ull> w_i_tokens;
	vector<IndexKey> w_i_keys;
	int oldSize = results.size();//0
	unordered_map<ull, int> resultKeywords;
	for (int i = 1; i <= info.wLength; i++)
	{
		IndexKey key = toIndexKey(w, i);
		w_i_keys.push_back(key);
		ull futuretk = SearchToken(w, i, info.searchCounter + 1);//��һ�η��ʵ�Ҷ��λ��
		w_i_tokens.push_back(futuretk);
		DataBlock input = { 0 };
		DataBlock output = { 0 };
		bool bExist = ReadWriteFromStash("read", futuretk, key, input, output);//��ȡ �������
		for (int j = 0; j < BLOCK_SIZE_B; j++)
		{
			ull id;
			if (output.data[j] == DUMMY_VALUE) continue;
			bool b = isDelIdentifer(output.data[j], id);
			if (b)
			{
				deleteFromVector(results, id);
				printf(" delete id=%ld \r\n", id);
				resultKeywords[id] = 0;//��־Ϊ��id�Ѿ�ɾ��
			}
			if (resultKeywords[id] != 1)//ȥ���ظ������ӵ�id����
			{
				if (!b)
				{
					resultKeywords[id] = 1;//��־Ϊ��id�Ѿ�����
					results.push_back(id);
				}
			}
		}
		//results.push_back(output.data);
	}
	int w_blockcount = 0;
	if (oldSize != results.size())//����ļ���ʶ����� ��Ҫ�ع��������ظ���identifer����־Ϊ�Ѿ�ɾ����identifier
	{
		int j = 0;
		for (int i = 1; i <= info.wLength; i++)
		{
			DataBlock output = { 0 };
			DataBlock input = EmptyBlock();
			for (int k = 0; k < BLOCK_SIZE_B; k++)
			{
				if (j >= results.size()) break;
				input.data[k] = results[j];
				j++;
			}
			if (!isEmptyBlock(input))
			{
				ReadWriteFromStash("write", w_i_tokens[i - 1], w_i_keys[i - 1], input, output);//�ع���Ľṹ��ɾ���ظ���identifier����ɾ����identifier
				w_blockcount++;
			}
			else
			{
				stash.erase(w_i_keys[i - 1]);//ɾ������Ŀ�
				break;
			}
		}
	}
//	if (w_blockcount != info.wLength)
//	{
//		printf(" some blocks are removed! =%d\r\n",info.wLength-w_blockcount);
//	}

	ENodes enodes2 = RebuildPaths(tokens,1);
	writePath(enodes2);
	double time_end = time_ms();

	info.searchCounter++;
	info.wLength = w_blockcount;//�����鳤��
	localMap[w] = info;
	ids = results;
	if(bDebug)
	{ 
		int aesCounter2 = myAES::counter();
		int Blake2bCounter2 = myhash::counter();
		int n_root_exceed_S2 = P.n_root_exceed_S;

		printf("\r\n matched: count=%d time=%lf (ms) AES=%d Blake2b=%d rootExceed=%d L=%d, S=%d Z=%d B=%d (bytes) U=%d bytes:%ld", results.size(),time_end-time_start, aesCounter2- P.aesCounter1, Blake2bCounter2- P.Blake2bCounter1, n_root_exceed_S2- n_root_exceed_S1, P.L, P.S, P.Z,sizeof(DataBlock), BLOCK_SIZE_B, debug.readBytes);
		int k = 0;
		for (auto r : results)
		{
			printf("\r\n id=%lld", r);
			if (k++ > 15)
			{
				printf(" \r\n ...");
				break;
			}
		}
	}
}

IndexKey ORAMtree::toIndexKey(string w, ull i)
{
	IndexKey k = { 0 };
	char buf[256] = { 0 };
	int len = w.length();
	if (w.length() > 64) len = 64;
	memcpy(buf, w.c_str(), len);
	memcpy(buf + len, (char*)&i, 8);
	myhash::Blake2b(buf, len + 8, (unsigned char*)k.buf);
	return k;
}

TreeNodes ORAMtree::RebuildPaths(TreeNodes nodes, vector<ull> leafs, string op, IndexKey key, DataBlock input_data, DataBlock & output_data)
{

	
	return TreeNodes();
}

bool ORAMtree::ReadWriteFromStash(string op, ull newLeaf, IndexKey key, DataBlock input_data, DataBlock & output_data)
{
	CPath64 p64;
	bool bExist = false;
	if (op == "write")
	{
		Slot s;
		s.key = key;
		s.value = input_data;
		s.leaf = newLeaf;
		stash[key] = s;
		bExist = true;
		//w_block_count++;//������һ�� ����
	}
	else
	{
		if (stash.find(key) == stash.end()) //�������һ���յ�key-value��Ҫ��Ҫ���룿
		{
		//	Slot s;
		//	s.key = key;
		//	s.value = input_data;
		//	s.leaf = newLeaf;
		//	stash[key] = s;   //û��Ҫ�����˰�
			bExist = false;
		}
		else
		{
			Slot s = stash[key];
			output_data = s.value;
			s.leaf = newLeaf;
			stash[key] = s;//�����µ�Ҷ�ӽڵ�
			bExist = true;
		}
	}
	return bExist;
}

bool LessSortByLeaf(Slot a, Slot b) { return (a.leaf < b.leaf); };

//���ֲ��� �������ʧ�ܣ�����һ�������
int ORAMtree::FindOneSlotFromSortedVector(vector<Slot>& slots, Slot tobeSearch)
{
	int i =0;
	int max = slots.size()-1;
	int min = 0;
//	if (slots.size() > 100)
//	{
//		i = 0;
//	}
	while (true)
	{
		i = (max + min) / 2;
		if (max < min)
		{
			return i;//����ʧ�� ѡ��һ�����������
		};
		if (slots[i].leaf == tobeSearch.leaf)
		{
			return i;
		}
		if (slots[i].leaf < tobeSearch.leaf)
		{
			min = i + 1;
		}
		if (slots[i].leaf > tobeSearch.leaf)
		{
		
			max = i - 1;
		}
	}
	return -1;
}



//���ֲ��� �������ʧ�ܣ�����һ�������
/*
int ORAMtree::FindOneSlotFromSortedMap(orderedMap& stashMap, Slot tobeSearch)
{
	int i = 0;
	int max = stashMap.size() - 1;
	int min = 0;
	//	if (slots.size() > 100)
	//	{
	//		i = 0;
	//	}
	orderedMap::iterator it = stashMap.find(tobeSearch.leaf);
	while (true)
	{
		i = (max + min) / 2;
		if (max < min)
		{
			return i;//����ʧ�� ѡ��һ�����������
		};
		if (slots[i].leaf == tobeSearch.leaf)
		{
			return i;
		}
		if (slots[i].leaf < tobeSearch.leaf)
		{
			min = i + 1;
		}
		if (slots[i].leaf > tobeSearch.leaf)
		{

			max = i - 1;
		}
	}
	return -1;
}
*/

void fastDeleteVector(vector<Slot>& ar, int i)
{
	// _ASSERT(i < ar.size());
	if (ar.size() - i - 1 > 0)
	{
		memcpy(&ar[i], &ar[i + 1], (ar.size() - i - 1) * sizeof(Slot));
	}
	ar.pop_back();
}

void ORAMtree::DeleteOneSlotFromVector(vector<Slot>& slots, Slot tobeDelete)
{
	int r = FindOneSlotFromSortedVector(slots,tobeDelete);
	int i = 0;
//	int j = slots.size()-1;
//	int div = 0;
	//if (r < 0) return;
	//int i = 0;
	while (1)
	{
		if (memcmp(slots[r + i].key.buf, tobeDelete.key.buf, sizeof(IndexKey)) == 0) 
		{
			r = r + i; break;
		}
		if (memcmp(slots[r - i].key.buf, tobeDelete.key.buf, sizeof(IndexKey)) == 0) 
		{
			r = r - i; break;
		}
		i++;
		if (r + i >= slots.size()) break;
		if (r - i < 0) break;
	}
	 
	if (r < 0) return;
	//for (int i = 0; i < slots.size(); i++)
	/*
	while(j>=i+1)
	{
		div = (i + j) / 2;
		if (memcmp(slots[div].key.buf,tobeDelete.key.buf,sizeof(IndexKey))==0)
		{
			r = div; break;
		}
		if (memcmp(slots[div].key.buf, tobeDelete.key.buf, sizeof(IndexKey)) > 0)
		{
			if (i != div)
			{
				i = div; break;
			}
			else
			{
				i = div + 1;
			}
		}
		else
		{
			if (j == div)
			{
				j = div - 1;
			}
			else
			{
				j = div; break;
			}
		}		
	}*/
//	int r = 0;
//	for (r=0;r<slots.size();r++)
//	{
//		if (memcmp(slots[r].key.buf, tobeDelete.key.buf, sizeof(IndexKey)) == 0)
//		{
//			break;
//		}
//	}

	int c = slots.size();
//	/**/
//
	for(int k=r;k<=c-2;k++)
	{
		slots[k] = slots[k+1];
	}
	slots.pop_back();	
	//fastDeleteVector(slots, r);
	//if (c - r - 1 > 0)
	//{
	//	memcpy(&slots[r], &slots[r + 1], (c - r - 1) * sizeof(Slot));
	//}
	//slots.pop_back();
	//slots.erase(slots.begin() + r);
}

//Ѱ�Һ� leaf ����� Z ��slot�����û��Z�����򷵻�ȫ����
void ORAMtree::FindZNearbySlotFromVector(vector<Slot>& slots, ull leaf, vector<Slot>& Z_results,int size)
{
	Slot s;
	s.leaf = leaf;
	int i = FindOneSlotFromSortedVector(slots, s);
	int count = 0;
	int j = 1;
	if (slots.size() <= size)
	{
		Z_results = slots;//����ȫ����
		return;
	}
	Z_results.push_back(slots[i]);
	count++;
	while (true)
	{
		if (count >= size) break;
		if (i - j > 0)
		{
			Z_results.push_back(slots[i - j]);
			count++;
		}
		if (count >= size) break;
		if (i + j < slots.size())
		{
			Z_results.push_back(slots[i + j]);
			count++;
		}
		j++;
		if (count >= size) break;
	}
}

//Ѱ�Һ� leaf ����� Z ��slot�����û��Z�����򷵻�ȫ����
//void ORAMtree::FindZNearbySlotFromMap(orderedMap& stashMap, ulong leaf, vector<Slot>& Z_results, int size)
//{
//	Slot s;
//	s.leaf = leaf;
	//int i = FindOneSlotFromSortedMap(slots, s);
	//int count = 0;
//	int j = 1;
//	if (slots.size() <= size)
//	{
//		Z_results = slots;//����ȫ����
//		return;
//	}
//	Z_results.push_back(slots[i]);
//	count++;
//		orderedMap::iterator it=stashMap.find(leaf);
//		if (it == stashMap.end()) return;
//		auto it1 = it;
//		for(int i=0;i<size/2;i++)
//		{
//			it++;
//			if (it == stashMap.end()) break; //�����ȡsize/2��
//			Z_results.push_back((*it).second);
//		}
//		for (int i = 0; i < size / 2; i++)
//		{
//			it1--;
//			if (it1 == stashMap.end()) break;//ǰ���ȡsize/2��
//			Z_results.push_back((*it1).second);
//		}
//}


void ORAMtree::evictPathsPATHORAM(unordered_map<string, TreeNode>& pathNodes, vector<unsigned long long> leafs)
{
	debug.PATH_pathObjInit = 0;

	for (int i = P.L; i >= 1; i--)
	{
		for (auto leaf : leafs)
		{
			CPath64 currentLeaf(leaf);
			TreeNode node;
			//string p1 = currentLeaf.AtLevelPath(i);
			ull p1long = currentLeaf.AtLevelPathFast(i, P.L);
			string p1 = NumberToPath(p1long);
			node = pathNodes[p1];//���pathNodes�в����� p1��������һ���յĽڵ� ռλ
			for (auto v=stash.begin();v!=stash.end();)
			{
				//int level = path64.Cross(leaf, v.second.leaf).GetLevel();	
				double t1 = time_ms();
				ull p2long = CPath64((*v).second.leaf).AtLevelPathFast(i,P.L); //���㷨Ч�ʲ��ߣ�
				double t2 = time_ms();
				debug.PATH_ORAM_chooseTime += t2 - t1;
				debug.PATH_pathObjInit++;
				if (p1long == p2long) //��Ϊ������Ľڵ���ܲ�������Ľڵ㣬
				{
					Slot s;
					s = (*v).second;
					if (node.slots.size() < P.Z) //root ��� S��
					{
						node.slots.push_back(s);
						v++;
						stash.erase(s.key);
						debug.PATH_stashRemove++;
					}
					else
					{
						v++;
						break;//�ڵ���
					}
					pathNodes[p1] = node;
				}
				else
				{
					v++;
				}
			}
		}
	}
}

void ORAMtree::evictPath(unordered_map<string, TreeNode>& pathNodes, vector<Slot> slots, ull leaf)
{
	CPath64 currentLeaf(leaf);
	KeyValues temp;
	for (int i = 0; i < slots.size(); i++)
	{
		temp[slots[i].key] = slots[i];
	}
	for (int i = P.L; i >= 1; i--)
	{
		TreeNode node;
		string p1 = currentLeaf.AtLevelPath(i);
		node = pathNodes[p1];//���pathNodes�в����� p1��������һ���յĽڵ� ռλ
		for (auto v : temp)
		{
			//	int level = path64.Cross(leaf, v.second.leaf).GetLevel();	
			string p2 = CPath64(v.second.leaf).AtLevelPath(i); //���㷨Ч�ʲ��ߣ�

			if (p1 == p2) //��Ϊ������Ľڵ���ܲ�������Ľڵ㣬
			{
				Slot s;
				s = v.second;			
				if (i == 1)
				{
				//	node.slots.push_back(s);//ȫ������ �����ж��� ���ڵ�
				//	if (node.slots.size() >= P.max_S)
				//	{
				//		P.max_S = node.slots.size();
				//	}
					if (node.slots.size() < P.S) //root ��� S��
					{
						node.slots.push_back(s);
					}
					else
					{
						break;//�ڵ���
					}
				}
				else
				{
					if (node.slots.size() < P.Z)
					{
						node.slots.push_back(s);
					}
					else
					{
						break;//�ڵ���
					}
				}
				pathNodes[p1] = node;
			}
		}
		for (Slot &v : node.slots)
		{
			temp.erase(v.key);
			stash.erase(v.key);//���Ѿ��������Ԫ�ش�stashɾ��
		}
	}
}

//��ȡ����ڵ� ���
unsigned long long cross(unsigned long long leaf1, unsigned long long leaf2)
{
	unsigned long long f1 = leaf1;
	unsigned long long f2 = leaf2;
	while (true)
	{
		if (f1 == f2)
		{
			return f1;
		}
		f1 = (f1 - 1) / 2;
		f2 = (f2 - 1) / 2;
		if (f1 == 0) break;
		if (f2 == 0) break;
	}
	return 0;
}
ull g_leaf = 0;
bool LessSortByLeaf2(Slot& a, Slot& b) 
{ 
	return cross(a.leaf, g_leaf) >= cross(b.leaf, g_leaf); //�ж��������
	//return fabs(a.leaf- g_leaf) < fabs(b.leaf- g_leaf); //������������ð취 ��Ϊ���
};//ѡ�����Ҷ�ӵ�

void ORAMtree::sortByLeafDistance(vector<Slot>& slots,ull leaf)
{
	//
	g_leaf = leaf;
	sort(slots.begin(), slots.end(), LessSortByLeaf2);
}
void ORAMtree::evictPath1(unordered_map<string, TreeNode>& pathNodes, vector<Slot> slots, ull leaf)
{
	CPath64 currentLeaf(leaf);

	//unordered_map<ulong, int> removed;
	//KeyValues removed;
	sortByLeafDistance(slots, leaf);//���տ���Ҷ�ӵ�˳�����������㷨
	//����ײ㿪ʼɨ��
	int j = 0;
	for (int i = P.L; i >= 1; i--)//����ײ㿪ʼ �����ϲ�
	{
		TreeNode node;
		string p1 = currentLeaf.AtLevelPath(i);
		node = pathNodes[p1];//���pathNodes�в����� p1��������һ���յĽڵ� ռλ
		node.path = p1;
		node.level = p1.length();
		for(int k=0;k<P.Z;k++) //ÿһ�㳢��Z������
		if(j < slots.size())//�����е�slot���� ������
		{
			//	int level = path64.Cross(leaf, v.second.leaf).GetLevel();
			Slot v = slots[j];
			string p2 = CPath64(v.leaf).AtLevelPath(i); //���㷨Ч�ʲ��ߣ�
		//	if (removed[v.key].leaf > 0) continue;//��ʾ�Ѿ�ɾ����//����Ҫ�ж��ˣ��˴��Ѿ���ʶ
			if (p1 == p2) //��Ϊ������Ľڵ���ܲ�������Ľڵ㣬
			{
				Slot s;
				s = v;
			//	if (i == 1)
			//	{
			//		node.slots.push_back(s);//ȫ������ �����ж��� ���ڵ�
			//		if (node.slots.size() >= P.max_S)
			//		{
			//			P.max_S = node.slots.size();
			//		}
			//		if (node.slots.size() < P.S)
			//		{
			//			node.slots.push_back(s);
			//			//removed[s.key] = s;
			//			j++;//�Ѿ�ʹ���˴˽ڵ�
			//		}
			//		else
			//		{
			//			break;//�ڵ���
			//		}
			//	}
			//	else
				//{
					if (node.slots.size() < P.Z)
					{
						node.slots.push_back(s);
						j++;//�Ѿ�ʹ����
						//removed[s.key] = s;
					}
					else
					{
						break;//�ڵ���
					}
				//}
			}
		}
		pathNodes[p1] = node;		
		for (Slot& v : node.slots)
		//for (auto & v : removed)
		{
			//slots.erase(&v);
			//removed[v.key] = v;//���Ϊɾ��
			stash.erase(v.key);//���Ѿ��������Ԫ�ش�stashɾ�� ȫ��
			//stash.erase(v.first);//ɾ�������Ѿ�������Ԫ�أ���stash
		}
	}

}

//���ñ��취 ���һ��ǿ�Ʊ���
void ORAMtree::evictPathAtLevel(unordered_map<string, TreeNode>& pathNodes, KeyValues& removed, vector<Slot> slots, ull leaf,int level)
{
	CPath64 currentLeaf(leaf);

	//unordered_map<ulong, int> removed;
	//KeyValues removed;
	//sortByLeafDistance(slots, leaf);//���տ���Ҷ�ӵ�˳�����������㷨
	//KeyValues temp;

	//��level�㿪ʼɨ��
	int j = 0;
	int i = level;

		TreeNode node;
		string p1 = currentLeaf.AtLevelPath(i);
		node = pathNodes[p1];//���pathNodes�в����� p1��������һ���յĽڵ� ռλ
		node.path = p1;
		node.level = p1.length();

		//for (int k = 0; k < P.Z; k++) //ÿһ�㳢��Z������
			for (auto& v : slots)
				//if (j < slots.size())//�����е�slot���� ������
			{
				//if (temp.find(v.key) != temp.end()) continue;//�Ѿ�ʹ��
				//	int level = path64.Cross(leaf, v.second.leaf).GetLevel();
				//Slot v = slots[j];			
				string p2 = CPath64(v.leaf).AtLevelPath(i); //���㷨Ч�ʲ��ߣ�
			//	if (removed[v.key].leaf > 0) continue;//��ʾ�Ѿ�ɾ����//����Ҫ�ж��ˣ��˴��Ѿ���ʶ
				if (p1 == p2) //��Ϊ������Ľڵ���ܲ�������Ľڵ㣬
				{
					Slot s;
					s = v;
					//	if (i == 1)
					//	{
					//		node.slots.push_back(s);//ȫ������ �����ж��� ���ڵ�
					//		if (node.slots.size() >= P.max_S)
					//		{
					//			P.max_S = node.slots.size();
					//		}
					//		if (node.slots.size() < P.S)
					//		{
					//			node.slots.push_back(s);
					//			//removed[s.key] = s;
					//			j++;//�Ѿ�ʹ���˴˽ڵ�
					//		}
					//		else
					//		{
					//			break;//�ڵ���
					//		}
					//	}
					//	else
						//{
					if (node.slots.size() < P.Z)
					{
						if (removed.find(s.key) == removed.end())//���û�б�ɾ��
						{
							node.slots.push_back(s);
							//j++;//�Ѿ�ʹ����
							removed[s.key] = s;//���Ϊ�Ѿ�ɾ��
							//temp[s.key] = s;
							stash.erase(s.key);
						};
					}
					else
					{
						break;//�ڵ���
					}
					//}
				}
			}
		pathNodes[p1] = node;
		//for (Slot& v : node.slots)
			//for (auto & v : removed)
		//{
			//slots.erase(&v);
			//removed[v.key] = v;//���Ϊɾ��
			//stash.erase(v.key);//���Ѿ��������Ԫ�ش�stashɾ�� ȫ��
			//stash.erase(v.first);//ɾ�������Ѿ�������Ԫ�أ���stash
		//}


}


//���ñ��취 ÿһ�㶼ǿ�Ʊ���
void ORAMtree::evictPath3(unordered_map<string, TreeNode>& pathNodes, vector<Slot> slots, ull leaf)
{
	CPath64 currentLeaf(leaf);

	//unordered_map<ulong, int> removed;
	//KeyValues removed;
	//sortByLeafDistance(slots, leaf);//���տ���Ҷ�ӵ�˳�����������㷨
	KeyValues temp;

	//����ײ㿪ʼɨ��
	int j = 0;
	for (int i = P.L; i >= 1; i--)//����ײ㿪ʼ �����ϲ�
	{
		TreeNode node;
		string p1 = currentLeaf.AtLevelPath(i);
		node = pathNodes[p1];//���pathNodes�в����� p1��������һ���յĽڵ� ռλ
		node.path = p1;
		node.level = p1.length();
	
		for (int k = 0; k < P.Z; k++) //ÿһ�㳢��Z������
			for(auto& v: slots)
			//if (j < slots.size())//�����е�slot���� ������
			{
				if (temp.find(v.key)!=temp.end()) continue;//�Ѿ�ʹ��
				//	int level = path64.Cross(leaf, v.second.leaf).GetLevel();
				//Slot v = slots[j];			
				string p2 = CPath64(v.leaf).AtLevelPath(i); //���㷨Ч�ʲ��ߣ�
			//	if (removed[v.key].leaf > 0) continue;//��ʾ�Ѿ�ɾ����//����Ҫ�ж��ˣ��˴��Ѿ���ʶ
				if (p1 == p2) //��Ϊ������Ľڵ���ܲ�������Ľڵ㣬
				{
					Slot s;
					s = v;
					//	if (i == 1)
					//	{
					//		node.slots.push_back(s);//ȫ������ �����ж��� ���ڵ�
					//		if (node.slots.size() >= P.max_S)
					//		{
					//			P.max_S = node.slots.size();
					//		}
					//		if (node.slots.size() < P.S)
					//		{
					//			node.slots.push_back(s);
					//			//removed[s.key] = s;
					//			j++;//�Ѿ�ʹ���˴˽ڵ�
					//		}
					//		else
					//		{
					//			break;//�ڵ���
					//		}
					//	}
					//	else
						//{
					if (node.slots.size() < P.Z)
					{
						node.slots.push_back(s);
						//j++;//�Ѿ�ʹ����
						//removed[s.key] = s;
						temp[s.key] = s;
						stash.erase(s.key);
					}
					else
					{
						break;//�ڵ���
					}
					//}
				}
			}
		pathNodes[p1] = node;
		//for (Slot& v : node.slots)
			//for (auto & v : removed)
		//{
			//slots.erase(&v);
			//removed[v.key] = v;//���Ϊɾ��
			//stash.erase(v.key);//���Ѿ��������Ԫ�ش�stashɾ�� ȫ��
			//stash.erase(v.first);//ɾ�������Ѿ�������Ԫ�أ���stash
		//}
	}

}

//����㷨����ʱ��̫�� �ƺ���BUG
void ORAMtree::evictPath2(unordered_map<string, TreeNode>& pathNodes, vector<Slot> slots, ull leaf)
{
	CPath64 currentLeaf(leaf);
	KeyValues temp;
	//����slots�����Ѿ�������� ����Ҷ�Ӵ�С����
	for (int i = P.L; i >= 1; i--)
	{
		TreeNode node;
		vector<Slot> Z_results;
		if (i > 1)
		{
			FindZNearbySlotFromVector(slots, leaf, Z_results, P.Z);
		}
		else
		{
			Z_results = slots;//�����еĽڵ�������
		}
		for (auto v : Z_results)
		{
			//	int level = path64.Cross(leaf, v.second.leaf).GetLevel();
			string p1 = currentLeaf.AtLevelPath(i);
			string p2 = CPath64(v.leaf).AtLevelPath(i); //
			if (p1 == p2) //
			{
				Slot s;
				s = v;
				node = pathNodes[p1];
				if (i == 1)
				{
					//node.slots.push_back(s);//ȫ������ �����ж��� ���ڵ�
					if (node.slots.size() < P.S)
					{
						node.slots.push_back(s);
					}
					else
					{
						break;//�ڵ���
					}
				}
				else
				{
					if (node.slots.size() < P.Z)
					{
						node.slots.push_back(s);
					}
					else
					{
						break;//�ڵ���
					}
				}
				pathNodes[p1] = node;
			}
		}
		for (auto& v : node.slots)//ɾ���Ѿ�ʹ���˵Ŀ�
		{
			DeleteOneSlotFromVector(slots, v);//ɾ���㷨���ĵ�ʱ��̫��
			stash.erase(v.key);
		}
	}
}
DataBlock ORAMtree::EmptyBlock()
{
	DataBlock b;
	for (int i = 0; i < BLOCK_SIZE_B; i++)
	{
		b.data[i] = DUMMY_VALUE;
	}
	return b;
}
bool ORAMtree::isEmptyBlock(DataBlock& b)
{
	for (int i = 0; i < BLOCK_SIZE_B; i++)
	{
		if (b.data[i] != DUMMY_VALUE) return false;
	}
	return true;
}
ull ORAMtree::toDelIdentifer(ull value)
{
	ull mask1 = 0x8000000000000000;
	return  value | mask1;
}
bool ORAMtree::isDelIdentifer(ull value,ull& output)
{
	ull mask1 = 0x8000000000000000;
	ull mask2 = 0x7FFFFFFFFFFFFFFF;
	output = value;
	if ((value & mask1) == mask1)
	{
		output = value & mask2;
		return true;
	}
	return false;
}
bool ORAMtree::deleteFromVector(vector<ull>& data, ull tobeDeleted)
{
	vector<ull> r;
	for (int i = 0; i < data.size(); i++)
	{
		if (data[i] != tobeDeleted)
		{
			r.push_back(data[i]);
		}
	}
	if (r.size() == data.size()) return false;
	data = r;
	return true;
}
void ORAMtree::EncryptPaths(unordered_map<string, TreeNode>& pathNodes,ENodes &enodes)
{
	for (auto &v : pathNodes)
	{
		TreeNode node= v.second;
	//	w_block_count += node.slots.size();//�ǿյĿ����ӣ������յ��Ʒ������ϵ�w-block����
		if (v.first != "")
		{
			while (node.slots.size() < P.Z) //����
			{
				Slot empty = { 0 };
				empty.value= EmptyBlock();
				node.slots.push_back(empty);
			}
		}
		node.path = v.first;
		node.level = v.first.length()+1;
		if (node.level == 1) //����ʹ�� ����root�ڵ�������
		{
			while (node.slots.size() < P.S) //����
			{
				Slot empty = { 0 };
				empty.value = EmptyBlock();
				node.slots.push_back(empty);
			}

		//	if (node.slots.size() > P.S)
		//	{
		//		P.n_root_exceed_S++;
		//		if (node.slots.size() > P.Max_S)
		//		{
		//			P.Max_S = node.slots.size();//��������ֵ
		//		}
				//printf("\r\n exceed S root_size=%d", node.slots.size());
			//}
		}
		ENode enode = EncryptNode(node);
		enodes[node.path] = enode;		
	}

}

//separation point
//���ؽ���ڵ㣬����ߺ��ӣ�������Ҷ�ӽڵ㣬��Ϊ�ָ���
unsigned long long ORAMtree::separationPoint(unsigned long long leaf1, unsigned long long leaf2)
{
	unsigned long long crossnode = cross(leaf1, leaf2);
	unsigned long long crossnodeLeftChild = crossnode * 2 +1;//left child
	unsigned long long MaxChildLeaf= crossnodeLeftChild;
	while (MaxChildLeaf < P.firstLeafID)
	{
		MaxChildLeaf = MaxChildLeaf * 2 + 2; // 
	}
	return MaxChildLeaf;
}
//�Ƚ����е�slot ��Ҷ��λ������Ȼ����leafs����
ENodes ORAMtree::RebuildPaths_PBEA(vector<ull>& leafs, int evictOption) //
{
	ENodes enodes;
	CPath64 path64;
	vector<Slot> stashArray;
	unordered_map<string, TreeNode> pathNodes;
	vector<Slot> slots;
	TreeNode d = { };
	for (auto& v : leafs)
	{
		CPath64 p(v);
		while (1)
		{
			string path = p.ToString();
			if (path == "") break;
			pathNodes[path] = d;//��ʼ��
			p = p.Father();
		}
	}
	for (auto& v : stash)
	{
		stashArray.push_back(v.second);
	};
	sort(stashArray.begin(), stashArray.end(), LessSortByLeaf);	//�Ƚ�stash�е����尴��Ҷ��λ�����򣬴�С����
	sort(leafs.begin(), leafs.end());
	ull div = 0;
	bool bComputeNextLeaf = false;
	if (leafs.size() == 1) div = pow(2, P.L) - 2;

	int k = 0;
	int PAGESIZE = 65536;
	int i = 1;
	vector<Slot> subStashArray;
	vector<ull> subLeafs;
	for (auto& v : stashArray)
	{
		div = this->P.firstLeafID + PAGESIZE * i;
		if (v.leaf >= div)
		{
			//div = separationPoint(leafs[k], leafs[k+1]);//�������� cross�ڵ���ӽڵ�ķ�ʽ
			i++;
			while ((k < leafs.size()) && (leafs[k] < div))
			{
				subLeafs.push_back(leafs[k]);
				k++;				
			}
			if (subLeafs.size() > 0)
			{
				evict_Paths_KNNEA(pathNodes, subStashArray, subLeafs);
			}
			subLeafs.clear();
			subStashArray.clear();
		}
		else
		{
			subStashArray.push_back(v);
		}
		
	}
	if (subStashArray.size() > 0)//�����Ȼ��������Ҫ����
	{
		while ((k < leafs.size()) && (leafs[k] <= div))
		{
			subLeafs.push_back(leafs[k]);
			k++;
		}
		if (subLeafs.size() > 0) evict_Paths_KNNEA(pathNodes, subStashArray, subLeafs);
		subLeafs.clear();
		subStashArray.clear();
	}
	evict_Paths_KNNEA(pathNodes, leafs);//��ʣ�������һ��
	EncryptPaths(pathNodes, enodes);
	return enodes;
}
//�Ƚ����е�slot ��Ҷ��λ������Ȼ����leafs���� ,�����ϵķָ�������û�н����ݲ��뵽�ײ������������㷨
ENodes ORAMtree::RebuildPaths_PartitionFirstOld(vector<ull>& leafs,int evictOption) //
{
	ENodes enodes;
	CPath64 path64;
	vector<Slot> stashArray;
	unordered_map<string, TreeNode> pathNodes;
	vector<Slot> slots;
	TreeNode d = { };
	for (auto& v : leafs)
	{
		CPath64 p(v);
		while (1)
		{
			string path = p.ToString();
			if (path == "") break;
			pathNodes[path] = d;//��ʼ��
			p = p.Father();
		}
	}
	for (auto& v : stash)
	{
		stashArray.push_back(v.second);
	};
	sort(stashArray.begin(), stashArray.end(), LessSortByLeaf);	//�Ƚ�stash�е����尴��Ҷ��λ�����򣬴�С����
	sort(leafs.begin(), leafs.end());
	ull div=0;
	bool bComputeNextLeaf = false;
	if (leafs.size() == 1) div = pow(2, P.L) - 2;
	else
		//div = (leafs[0] + leafs[1]) / 2;		
		div = separationPoint(leafs[0], leafs[1]);//�������� cross�ڵ���ӽڵ�ķ�ʽ ���ַ�ʽ��һЩ����Ҷ�ӵĽڵ㣬��һ�����Ա����뵽��ǰ·���У�����һЩԶ�Ľڵ���Ա����뵽��ǰ·����

	int k = 0;
	for (auto v : stashArray)
	{
		if (v.leaf > div)
		{
			if (evictOption == 0)
			{
				evictPath(pathNodes, slots, leafs[k]);
			}
			else
			{
				//evictPath1(pathNodes, slots, leafs[k]);
				evictPath3(pathNodes, slots, leafs[k]);
			}
			slots.clear();
			if (k>=leafs.size()-1) 
				div = pow(2, P.L) - 2;
			else
				div = (leafs[k] + leafs[k+1]) / 2;
				//div = separationPoint(leafs[k], leafs[k+1]);//�������� cross�ڵ���ӽڵ�ķ�ʽ
			k++;
		}
		slots.push_back(v);
	}
	if (slots.size() > 0)
	{
		if (evictOption == 0)
		{
			evictPath(pathNodes, slots, leafs[leafs.size() - 1]);//����Ҷ��
		}
		else
		{
			//evictPath1(pathNodes, slots, leafs[leafs.size() - 1]);//����Ҷ��
			evictPath3(pathNodes, slots, leafs[leafs.size() - 1]);//����Ҷ��
		}
	}
	if (stash.size() > 0)//�����Ȼ��������Ҫ����
	{
		evict_Paths_KNNEA(pathNodes, leafs);
		//evictPathsDFEA(pathNodes, leafs); //DFEA
	}
	EncryptPaths(pathNodes, enodes);
	return enodes;
}

//�Ƚ����е�slot ��Ҷ��λ������Ȼ����leafs���� �����ص���Ч�ʷǳ���
ENodes ORAMtree::RebuildPaths_StackedLocation(vector<ull>& leafs, int evictOption) //
{
	ENodes enodes;
	CPath64 path64;
	vector<Slot> stashArray;
	unordered_map<string, TreeNode> pathNodes;
	vector<Slot> slots;
	vector<vector<Slot>> toBeEvict;
	TreeNode d = { };
	for (auto v : leafs)
	{
		CPath64 p(v);
		while (1)
		{
			string path = p.ToString();
			if (path == "") break;
			pathNodes[path] = d;//��ʼ��
			p = p.Father();
		}
	}
	for (auto v : stash)
	{
		stashArray.push_back(v.second);
	};
	sort(stashArray.begin(), stashArray.end(), LessSortByLeaf);	//�Ƚ�stash�е����尴��Ҷ��λ�����򣬴�С����
	sort(leafs.begin(), leafs.end());
	ull div = 0;
	bool bComputeNextLeaf = false;

	int k = 0;
	vector<int> divs;
	int i = 0;
	for (i = 0; i < stashArray.size(); i++)
	{
		if (stashArray[i].leaf >= leafs[k])
		{
			divs.push_back(i);
			k++;
		}
	};
	while (divs.size() < leafs.size()) divs.push_back(i);

	for (auto& v : divs)
	{
		vector<Slot> s;
		for (int i=0;i<3*P.Z*P.L;i++) //�ұ����� Z*L
		{
			//
			if(v+i<stashArray.size())
			s.push_back(stashArray[v+i]);
		}
		for (int i = 1; i <= 3*P.Z * P.L; i++)//������� Z*L
		{
			//
			if(v-i>0) s.push_back(stashArray[v-i]);
		}
		toBeEvict.push_back(s);
	}

	KeyValues removed;
	for (int i = P.L; i >= 1; i--)
	{
		int k = 0;
		for (auto& v : toBeEvict)
			evictPathAtLevel(pathNodes, removed, v, leafs[k++], i);
	}
	EncryptPaths(pathNodes, enodes);
	return enodes;
}

ENodes ORAMtree::RebuildPaths(vector<ull>& leafs,int option) //
{
	if (P.shuffle_choice == ALGORITHM_PATHORAM)
	{
		unordered_map<string, TreeNode> nodes;
		evictPathsPATHORAM(nodes,leafs);
		ENodes enodes;
		EncryptPaths(nodes, enodes);
		return  enodes;
	}
	if (P.shuffle_choice == ALGORITHM_PBEA)
	{
		return RebuildPaths_PBEA(leafs,option);
		//return RebuildPaths_PBEA(leafs, option);
	}
	if (P.shuffle_choice == ALGORITHM_BOTTOM_TO_TOP)
	{
		return RebuildPaths_nopartition(leafs);
	}
}


//���������Ѿ�����stash���ˣ�׼��ϴ�� ���㷨���ȶ����еĽڵ����򣬴ӵײ�������,��ǰ���㷨 KNNEA
ENodes ORAMtree::RebuildPaths_nopartition(vector<ull>& leafs) //
{
	ENodes enodes;
	CPath64 path64;
	vector<Slot> stashArray;
	stashArray.reserve(stash.size());
	for (auto v : stash)
	{
		stashArray.push_back(v.second);
	};
	sort(stashArray.begin(), stashArray.end(), LessSortByLeaf);	//�Ƚ�stash�е����尴��Ҷ��λ�����򣬴�С����
	debug.BottomToTop_removeTime = 0;
	for (int i = P.L; i >= 1; i--)
	{
		for (auto leaf : leafs)
		{
			TreeNode node;
			node.slots.clear();
			node.level = 0;
			node.path = "";
			ENode enode;
			CPath64 currentLeaf(leaf);
			string path = currentLeaf.AtLevelPath(i);
			if (enodes.find(path) != enodes.end()) continue;//�Ѿ��������ڵ�������� ȥ���ظ�
			vector<Slot> Z_results;
			//if (i > 1)
			//{
			FindZNearbySlotFromVector(stashArray, leaf, Z_results, P.Z*2+1);//���Ҹ�Z��Ԫ�� ����ֻ��Z��Ԫ��
			//}
			//else
			//{
			//	Z_results = stashArray;//�����еĽڵ�������
			//}
			for (auto& v : Z_results)
			{
			//	int level = path64.Cross(leaf, v.second.leaf).GetLevel();
				//string p1 = CPath64(leaf).AtLevelPath(i);
				//string p2 = CPath64(v.leaf).AtLevelPath(i);
				ull p1long=  CPath64(leaf).AtLevelPathFast(i,P.L);
				ull p2long=  CPath64(v.leaf).AtLevelPathFast(i, P.L);
				if (p1long==p2long)
				//if(p1==p2)
				{
					Slot s;
					s = v;					
				//	if (i == 1)
				//	{
				//		if (node.slots.size() < P.S)
				//		{
				//			node.slots.push_back(s);
				//		}
				//		else
				//		{
				//			break;//�ڵ���
				//		}
				//	}
				//	else
				//	{
						if (node.slots.size() < P.Z)
						{
							node.slots.push_back(s);
						}
						else
						{
							break;//�ڵ���
						}
				//	}
				}
			}
			int max = P.Z;
			//if (i == 1) max = P.S;
			double t1 = time_ms();
			for (auto v : node.slots)//ɾ���Ѿ�ʹ���˵Ŀ�
			{							
				DeleteOneSlotFromVector(stashArray, v);//ɾ���㷨���ĵ�ʱ��̫��
				debug.BottomToTop_stashRemove++;
				stash.erase(v.key);
			}
			double t2 = time_ms();
			debug.BottomToTop_removeTime += (t2 - t1);
			while (node.slots.size() < max) //����
			{
				Slot empty = { 0 };
				empty.value = EmptyBlock();
				node.slots.push_back(empty);
			}
			node.path = path;// currentLeaf.AtLevelPath(i);
			node.level = i;
			if (i == 1)
			{
				//if (node.slots.size() > P.S)
				//{
			//		P.n_root_exceed_S++;
			//		if (node.slots.size()> P.Max_S)
			//		{
			//			P.Max_S = node.slots.size();//��������ֵ
			//		}
					//printf("\r\n exceed S root_size=%d", node.slots.size());
			//	}
			}
			enode = EncryptNode(node);
			enodes[node.path] = enode;
		}
	}
	return enodes;
}

//KNNEA �㷨 ��������
void ORAMtree::evict_Paths_KNNEA(unordered_map<string, TreeNode>& pathNodes, vector<Slot>& stashArray, vector<ull>& leafs) //
{
	//ENodes enodes;
	CPath64 path64;
//	vector<Slot> stashArray;
//	for (auto v : stash)
//	{
//		stashArray.push_back(v.second);
//	};
	sort(stashArray.begin(), stashArray.end(), LessSortByLeaf);	//�Ƚ�stash�е����尴��Ҷ��λ�����򣬴�С����
	for (int i = P.L; i >= 1; i--)
	{
		for (auto leaf : leafs)
		{
			TreeNode node;
			//node.slots.clear();
			//node.level = 0;
			//node.path = "";
			//node=
			//ENode enode;
			CPath64 currentLeaf(leaf);
			string path = currentLeaf.AtLevelPath(i);
			//if (enodes.find(path) != enodes.end()) continue;//�Ѿ��������ڵ�������� ȥ���ظ�
			node = pathNodes[path];//��ȡ
			vector<Slot> Z_results;
			if (i > 1)
			{
				FindZNearbySlotFromVector(stashArray, leaf, Z_results, P.Z * 2);//���Ҹ�Z��Ԫ�� ����ֻ��Z��Ԫ��
			}
			else
			{
				Z_results = stashArray;//�����еĽڵ�������
			}
			for (auto& v : Z_results)
			{
				//	int level = path64.Cross(leaf, v.second.leaf).GetLevel();
				//string p1 = CPath64(leaf).AtLevelPath(i);
				//string p2 = CPath64(v.leaf).AtLevelPath(i);
				//if (p1 == p2)
				ull p1long = CPath64(leaf).AtLevelPathFast(i, P.L);
				ull p2long = CPath64(v.leaf).AtLevelPathFast(i, P.L);
				if (p1long == p2long)
				{
					//Slot s;
					//s = v;
					if (node.slots.size() < P.Z)
					{
						node.slots.push_back(v);
						DeleteOneSlotFromVector(stashArray, v);//ɾ���㷨���ĵ�ʱ��̫��
						stash.erase(v.key);//ֱ��ɾ��
					}
					else
					{
						break;//�ڵ���
					}
				}
			}
			//int max = P.Z;
			//if (i == 1) max = P.S;
			//for (auto v : node.slots)//ɾ���Ѿ�ʹ���˵Ŀ�
			//{
			//	DeleteOneSlotFromVector(stashArray, v);//ɾ���㷨���ĵ�ʱ��̫��
			//	stash.erase(v.key);
			//}
			//while (node.slots.size() < max) //���� �������� ���������ܵ�ʱ��ȥ����
			//{
			//	Slot empty = { 0 };
			//	empty.value = EmptyBlock();
			//	node.slots.push_back(empty);
			//}
			node.path = currentLeaf.AtLevelPath(i);
			node.level = i;
			if (i == 1)
			{
				//if (node.slots.size() > P.S)
				//{
			//		P.n_root_exceed_S++;
			//		if (node.slots.size()> P.Max_S)
			//		{
			//			P.Max_S = node.slots.size();//��������ֵ
			//		}
					//printf("\r\n exceed S root_size=%d", node.slots.size());
			//	}
			}
			//enode = EncryptNode(node);
			//enodes[node.path] = enode;
			pathNodes[node.path] = node;//����
		}
	}
}

//KNNEA �㷨
void ORAMtree::evict_Paths_KNNEA(unordered_map<string,TreeNode>& pathNodes,vector<ull>& leafs) //
{
	//ENodes enodes;
	CPath64 path64;
	vector<Slot> stashArray;
	for (auto v : stash)
	{
		stashArray.push_back(v.second);
	};
	sort(stashArray.begin(), stashArray.end(), LessSortByLeaf);	//�Ƚ�stash�е����尴��Ҷ��λ�����򣬴�С����
	for (int i = P.L; i >= 1; i--)
	{
		for (auto leaf : leafs)
		{
			TreeNode node;
			//node.slots.clear();
			//node.level = 0;
			//node.path = "";
			//node=
			//ENode enode;
			CPath64 currentLeaf(leaf);
			string path = currentLeaf.AtLevelPath(i);
			//if (enodes.find(path) != enodes.end()) continue;//�Ѿ��������ڵ�������� ȥ���ظ�
			node = pathNodes[path];//��ȡ
			vector<Slot> Z_results;
			FindZNearbySlotFromVector(stashArray, leaf, Z_results, P.Z * 2);//���Ҹ�Z��Ԫ�� ����ֻ��Z��Ԫ��	
			for (auto& v : Z_results)
			{
				//	int level = path64.Cross(leaf, v.second.leaf).GetLevel();
				//string p1 = CPath64(leaf).AtLevelPath(i);
				//string p2 = CPath64(v.leaf).AtLevelPath(i);
				//if (p1 == p2)
				ull p1long = CPath64(leaf).AtLevelPathFast(i, P.L);
				ull p2long = CPath64(v.leaf).AtLevelPathFast(i, P.L);
				if (p1long == p2long)
				{
					//Slot s;
					//s = v;
						if (node.slots.size() < P.Z)
						{
							node.slots.push_back(v);
							DeleteOneSlotFromVector(stashArray, v);//ɾ���㷨���ĵ�ʱ��̫��
							stash.erase(v.key);//ֱ��ɾ��
						}
						else
						{
							break;//�ڵ���
						}
				}
			}
			//int max = P.Z;
			//if (i == 1) max = P.S;
			//for (auto v : node.slots)//ɾ���Ѿ�ʹ���˵Ŀ�
			//{
			//	DeleteOneSlotFromVector(stashArray, v);//ɾ���㷨���ĵ�ʱ��̫��
			//	stash.erase(v.key);
			//}
		//	while (node.slots.size() < max) //���� //��ʱ������
		//	{
		//		Slot empty = { 0 };
		//		empty.value = EmptyBlock();
		//		node.slots.push_back(empty);
		//	}
			node.path = currentLeaf.AtLevelPath(i);
			node.level = i;
			if (i == 1)
			{
				//if (node.slots.size() > P.S)
				//{
			//		P.n_root_exceed_S++;
			//		if (node.slots.size()> P.Max_S)
			//		{
			//			P.Max_S = node.slots.size();//��������ֵ
			//		}
					//printf("\r\n exceed S root_size=%d", node.slots.size());
			//	}
			}
			//enode = EncryptNode(node);
			//enodes[node.path] = enode;
			pathNodes[node.path] = node;//����
		}
	}
}

//�����ʼ��Ҷ��λ��
//ulong ORAMtree::GetInitialPos(Slot& s)
//{
//	return firstLeafID + (*(ulong*)&s.key) % leafCount;
//}
//��ʼ���ķ�ʽ����һ���� ��Ĭ���ǲ���Ҷ�ӽڵ㣬���Ҷ�ӽڵ����˾�����stash֮�У���
//ÿһ��treenode �� Z�� Slot
ENodes ORAMtree::RebuildPaths_initial() //
{
	ENodes enodes;
	CPath64 path64;
	//vector<Slot> stashArray;
	KeyValues tempStash;//reserved 
	//for (auto v : stash)
	//{
//		stashArray.push_back(v.second);
		//eNodes=
	//}
	unordered_map<string, TreeNode> pathNodes;
//	for (auto v : stash)
//	{
//		stashArray.push_back(v.second);
//	};
// 
//	sort(stashArray.begin(), stashArray.end(), LessSortByLeaf);	//�Ƚ�stash�е����尴��Ҷ��λ�����򣬴�С����
//	for (int i = L; i >= 1; i--)
//	{	 
		for (auto& v : stash)
		{
			//ulong leaf = GetInitialPos(v.second);			
			ull leaf = v.second.leaf;// SearchToken(w, 0, info.searchCounter);
			CPath64 currentLeaf(leaf);			
			string path = currentLeaf.ToString(); //currentLeaf.AtLevelPath(L);
			TreeNode node = pathNodes[path];
			node.path = path;
			if(node.slots.size() < P.Z)
			{
				node.slots.push_back(v.second);
			}
			else
			{
				//node.slots.push_back(v.second);
				tempStash[v.first] = v.second;//������û�б�����ɹ����ű��ػ�����
			}		
			pathNodes[path] = node;
		}
		for (auto& v : pathNodes)
		{
			TreeNode node = v.second;			
			while (node.slots.size() < P.Z) //����
			{
				Slot empty = { 0 };
				empty.value = EmptyBlock();
				node.slots.push_back(empty);
			}			
			ENode enode = EncryptNode(node);
			enodes[node.path] = enode;
		}
		stash = tempStash;//����ԭ����
	//}
	//*/
	return enodes;
}
//���������Ѿ�����stash���ˣ�׼��ϴ�� //���㷨
ENodes ORAMtree::RebuildPathsOld(vector<ull>& leafs)
{
	ENodes enodes;
	CPath64 path64;	
	for (int i = P.L; i >= 1; i--)
	{
		for (auto leaf : leafs)
		{
			TreeNode node;
			ENode enode;
			CPath64 currentLeaf(leaf);
			string path = currentLeaf.AtLevelPath(i);
			if (enodes.find(path) != enodes.end()) continue;//�Ѿ��������ڵ�������� ȥ���ظ�
			for (auto v : stash)
			{
				//	if (v.second.value.data == 99)
				//	{
				//		printf("\r\n debug");
				//	}
				//	int level = path64.Cross(leaf, v.second.leaf).GetLevel();
				string p1 = CPath64(leaf).AtLevelPath(i);
				string p2 = CPath64(v.second.leaf).AtLevelPath(i);
				if (p1 == p2)
				{
					Slot s;
					s = v.second;
					if (i == 1)
					{
						if (node.slots.size() < P.S)
						{
							node.slots.push_back(s);
						}
						else
						{
							break;//�ڵ���
						}
					}
					else
					{
						if (node.slots.size() < P.Z)
						{
							node.slots.push_back(s);
						}
						else
						{
							break;//�ڵ���
						}
					}
				}
			}
			int max = P.Z;
			if (i == 1) max = P.S;
			for (auto v : node.slots)//ɾ���Ѿ�ʹ���˵Ŀ�
			{
				stash.erase(v.key);
			}
			while (node.slots.size() < max) //����
			{
				Slot empty = { 0 };
				empty.value=EmptyBlock();
				node.slots.push_back(empty);
			}
			node.path = currentLeaf.AtLevelPath(i);
			node.level = i;
			if (i == 1)
			{
				if (node.slots.size() > P.S)
				{
					P.n_root_exceed_S++;
					//printf("\r\n exceed S root_size=%d", node.slots.size());
				}
			}
			enode = EncryptNode(node);
			enodes[node.path] = enode;
		}
	}
	return enodes;
}

ull ORAMtree::SearchToken(string w, ull i, ull w_searchtimes)
{
	unsigned char output[20] = { 0 };
	char input[128] = { 0 };
	int len = w.length();
	if (w.length() > 64)
	{
		len = 64;
	};
	memcpy(input, (char*)&i, 8);
	memcpy(input +8, w.c_str(), len);
	memcpy(input + len+8, (char*)&w_searchtimes, 8);
	memcpy(input + len+8+8, privateKey.c_str(), privateKey.length());
	int total = 8+len  + 8+ privateKey.length(); //i  w  w_searchtimes
	myhash::Blake2b(input, total, output);
	ull leafid = *(ull *)&output;
	ull M = pow(2, P.L-1);
	leafid = leafid % M;//����߿�ʼ����Ҷ�ӱ��
	CPath64 p;
	p.LoadLeaf(P.L, leafid);
	return p.value;//����Ҷ�ӽڵ��ţ�����Ҷ�ӱ�ţ�
}

TreeNode ORAMtree::DecryptNode(ENode e)
{
	TreeNode d;

	int insize = e.bytes.size();
	char* m = (char*)malloc(insize);
	for (int i = 0; i < insize; i++)
	{
		m[i] = e.bytes[i];
	}
	char* plaintext = (char*)malloc(insize);
	int outsize=0;
	int sizem = sizeof(Slot);
	myAES::Decrypt(m, insize, plaintext, outsize);
	int outBlock = outsize / sizeof(Slot);//

	for (int i = 0; i < outBlock; i++)
	{
		if (d.slots.size() < outBlock)
		{
			Slot r = { 0 };
			d.slots.push_back(r);//�������
		}
		memcpy(&d.slots[i], plaintext +  i * sizeof(Slot), sizeof(Slot));
	}
	free(m);
	free(plaintext);
	return d;
}

ENode ORAMtree::EncryptNode(TreeNode d)
{
	ENode ed;
	ed.level = d.level;
	ed.path = d.path;
	
	//vector<byte> b;
	Slot s = { 0 };
	if (d.slots.size() == 0)
	{
		for (int i = 0; i < P.Z; i++)
		{
			d.slots.push_back(s);
		}
	}
	int insize = d.slots.size() * sizeof(Slot);
	int outsize=0;

	char* m = (char*)malloc(insize);
	char* output = (char*)malloc(insize * 2);
	for (int i = 0; i < d.slots.size(); i++)
	{
		memcpy(m + i * sizeof(Slot), (char*)&d.slots[i], sizeof(Slot));
	}
	myAES::Encrypt(m, insize, output, outsize);
	for (int i = 0; i < outsize; i++)
	{
		ed.bytes.push_back(output[i]);
	}
	free(output);

	return ed;
}

TreeNodes ORAMtree::DecryptNodes(ENodes e)
{
	TreeNodes t;
	for (auto v : e)
	{
		t[v.first] = DecryptNode(v.second);
	}
	return t;
}

ENodes ORAMtree::EncryptNodes(TreeNodes d)
{
	ENodes temp;
	for (auto v : d)
	{
		auto value = EncryptNode(v.second);
		temp[v.first] = value;
	}
	return temp;
}

void ORAMtree::WriteIntoStash(ENodes enodes)
{
	for (auto v : enodes)
	{
		TreeNode d = DecryptNode(v.second);
		for (auto& s : d.slots)
		{
			if (isEmptyBlock(s.value)) continue;//�����ݿ�
			stash[s.key] = s; //�����Ƿ���ڸ���ԭ�������ݵĿ��ܣ�
		//	w_block_count--;//�������ϵ� �ǿ����ݿ鱻ȡ������
		}
	}
}

//ֱ�ӽ���ʼ��������д��stash
void ORAMtree::LocalInsert(vector<Slot> blocks)
{
	for (auto& v : blocks)
	{
		stash[v.key] = v;
	}
}

ORAMtree::ORAMtree()
{
	//
}

ENodes ORAMtree::ReadPath(unsigned long long leaf)
{
	CPath64 p(leaf);
	ENodes nodes;
	while(true)
	{
		string path = p.ToString();
		if (oram.find(path) == oram.end())
		{
			ENode empty;
			empty.path = path;
			empty.level = p.GetLevel();
			nodes[path] = empty;
			if (p.value == 0) break;
			p = p.Father();
			continue;
		}
		ENode n=oram[path];
		nodes[path] = n;
		if (p.value == 0) break;
		p=p.Father();		
	}		
	return nodes;
}

ENodes ORAMtree::ReadPaths(vector<ull> leafs)
{
	ENodes nodes;
	debug.readBytes = 0;
	for (auto le : leafs)
	{
		CPath64 p(le);
		while (true)
		{
			string path = p.ToString();
			if (oram.find(path) == oram.end())
			{
				ENode empty;
				empty.path = path;
				empty.level = p.GetLevel();
				nodes[path] = empty;//����սڵ�
				if (p.value == 0) break;
				p = p.Father();
				continue;
			}
			ENode n = oram[path];
		//	ENode empty0;
		//	oram[path] = empty0;//����
			nodes[path] = n;
			debug.readBytes += n.bytes.size() + 8;
			if (p.value == 0) break;
			p = p.Father();
		}
	}
	return nodes;
}

void ORAMtree::writePath(ENodes nodes)
{
	for (auto d : nodes)
	{
		oram[d.first] = d.second;
	}
	for (auto v : nodes)
	{
		P.band_width += v.second.bytes.size() + 8;//8���ֽ����ڱ�ʾ node idenfier
	}
	if (P.max_S < stash.size()) P.max_S = stash.size();
}

void ORAMtree::dumpLevel()
{
	printf("\r\n ---------------------------");
		unsigned long long size = 0;
	
		unordered_map<int, int> levelNonempty;
		unordered_map<int, int> levelsEmpty;
		unordered_map<int, int> levelsDummy;
		ull all=0;
		ull dummy = 0;
		ull real = 0;
		for (auto t : oram)
		{
			ENode ed = t.second;
			TreeNode d = DecryptNode(ed);
			CPath64 p;
			p.LoadPath(t.first);
			//printf("\r\n level=%lld path=%s [", p.GetLevel(), t.first.c_str());
			int level = p.GetLevel();
			for (int j = 0; j < d.slots.size(); j++)
			{
				for (int k = 0;k < BLOCK_SIZE_B; k++)
				{
					if (d.slots[j].value.data[k]== DUMMY_VALUE)
					{
						//printf("()");
						levelsDummy[level]++;
						all++;
						dummy++;
					}
					else
					{
						levelNonempty[level]++;
						all++;
						real++;
						//Slot s = d.slots[j];
						//printf("(%.2x%.2x %.2x%.2x, %.lld, leaf=%lld leafpath=%s)",s.key.buf[0], s.key.buf[1],s.key.buf[2],s.key.buf[3],s.value.data,s.leaf,CPath64(s.leaf).ToString().c_str());
					}
				}
			}
		}

		int rootBlocks = DecryptNode(oram[""]).slots.size();
		for (int i = 1; i <= P.L; i++)
		{
			int levelAllValues = pow(2, i - 1) * P.Z*BLOCK_SIZE_B;
			if (i == 1) levelAllValues = P.S* BLOCK_SIZE_B;// 
			double rate = (double)levelNonempty[i] / (double)levelAllValues;
			int c = levelAllValues - levelsDummy[i] - levelNonempty[i];
			if (c < 0) c = 0;
			printf("\r\n Level %d, empty:%d dummy:%d real:%d rate:%lf%% ", i, c, levelsDummy[i], levelNonempty[i], rate*100);
		}
		printf("\r\n all values:%ld real=%ld dummy=%ld max stash:%d L=%d S=%d Z=%d rootStash=%d(blocks)", all, real,dummy,P.max_S, P.L, P.S, P.Z, rootBlocks);
}
void ORAMtree::dumpStashUsage()
{
//	int rootBlocks = DecryptNode(oram[""]).slots.size();
//	int s = 0;
//	if (rootBlocks > P.S)
//	{
//		s = rootBlocks - P.S;
//	}
	string str;
	switch(P.shuffle_choice)
	{
		case ALGORITHM_PBEA: str = "ALGORITHM_PBEA"; break;
		case ALGORITHM_BOTTOM_TO_TOP:str = "BOTTOM_TO_TOP"; break;
		case ALGORITHM_PATHORAM:str = "ALGORITHM_PATHORAM"; break;
	}
	
	printf("\r\n L=%d S=%d Z=%d Stash=%d(blocks) max_stash=%d(blocks) max_stash_evict=%d EvictionAlogrithm: %s \r\n", P.L, P.S, P.Z, stash.size(), P.max_S,P.max_Stash_Evict, str.c_str());
}
void ORAMtree::DebugTestIsInPathOrInStash()
{

}
void ORAMtree::dump()
{
	unsigned long long size = 0;
	printf("\r\n//-----------------------------------------");
	for (auto t : oram)
	{
		ENode ed = t.second;
		TreeNode d = DecryptNode(ed);
		CPath64 p;
		p.LoadPath(t.first);
		printf("\r\n path=%s [",  t.first.c_str());
		for (int j = 0; j < d.slots.size(); j++)
		{
			if (isEmptyBlock(d.slots[j].value))
			{
				printf("()");
				continue;
			}
			else
			{
				Slot s = d.slots[j];
				printf("(%.2x%.2x %.2x%.2x, %.lld, leaf=%lld leafpath=%s)", s.key.buf[0], s.key.buf[1], s.key.buf[2], s.key.buf[3], s.value.data[0], s.leaf, CPath64(s.leaf).ToString().c_str());
			}
		}
		printf("]");
	}
}
void ORAMtree::info()
{
	unsigned long long size = 0;
	printf("\r\n//-----------------------------------------");
	unsigned long long  emptyBlockCount = 0;
	unsigned long long nonemptyBlockCount = 0;
	unsigned long long dummy = 0,real=0;
	for (auto& t : oram)
	{
		ENode ed = t.second;
		TreeNode d = DecryptNode(ed);
	//	CPath64 p;
	//	p.LoadPath(t.first);
		//printf("\r\n path=%s [", t.first.c_str());
		for(auto& s: d.slots)
		{
			if (isEmptyBlock(s.value))
			{
			//	printf("()");
				emptyBlockCount++;
				continue;
			}
			else
			{
				nonemptyBlockCount++;

				for (int k = 0; k < BLOCK_SIZE_B; k++)
				{
					if (s.value.data[k] == DUMMY_VALUE)
					{
						dummy++;
					}
					else
					{
						real++;
					}
				}
				//printf("(%.2x%.2x %.2x%.2x, %.lld, leaf=%lld leafpath=%s)", s.key.buf[0], s.key.buf[1], s.key.buf[2], s.key.buf[3], s.value.data[0], s.leaf, CPath64(s.leaf).ToString().c_str());
			}
		}
		//printf("]");
	}
	for (auto&s : stash)
	{
		for (int k=0;k< BLOCK_SIZE_B;k++)
		{
			if (s.second.value.data[k] == DUMMY_VALUE)
			{
				dummy++;
			}
			else
			{
				real++;
			}
		}
	}
	printf("emptyBlock:%ld nonemptyBlock:%ld real value+stash:%ld dummy value:%ld \r\n", emptyBlockCount, nonemptyBlockCount, real, dummy);
}
void ORAMtree::dumpdata()
{
	unsigned long long size = 0;
	printf("\r\n//-----------------------------------------\r\n");
	for (auto t : oram)
	{
		ENode ed = t.second;
		TreeNode d = DecryptNode(ed);
		CPath64 p;
		p.LoadPath(t.first);
		printf("\r\n path=%s [", t.first.c_str());
		for (int j = 0; j < d.slots.size(); j++)
		{
			if (isEmptyBlock(d.slots[j].value))
			{
				printf("()");
				continue;
			}
			else
			{
				Slot s = d.slots[j];
				printf("(%.2x%.2x %.2x%.2x,leaf=%lld leafpath=%s data:", s.key.buf[0], s.key.buf[1], s.key.buf[2], s.key.buf[3], s.leaf, CPath64(s.leaf).ToString().c_str());
				for (int k = 0; k < sizeof(s.value.data)/8; k++)
				{
					printf("%lld ", s.value.data[k]);
				}
				printf(")");
			}
		}
		printf("]");
	}
	printf("\r\n Stash:{");
	for (auto v : stash)
	{
		Slot s = v.second;
		printf("(%.2x%.2x %.2x%.2x,leaf=%lld leafpath=%s data:", s.key.buf[0], s.key.buf[1], s.key.buf[2], s.key.buf[3], s.leaf, CPath64(s.leaf).ToString().c_str());
		for (int k = 0; k < sizeof(s.value.data) / 8; k++)
		{
			printf("%lld ", s.value.data[k]);
		}
		printf(")");
	}
	printf("}\r\n");
}
void ORAMtree::save()
{
	//
	FILE* fp1 = fopen("p.bin", "wb+");
	fwrite(&P, sizeof(P), 1, fp1);
	fclose(fp1);

	FILE* fp2 = fopen("KeywordInfo.bin", "wb+");
	for (auto& v : localMap)
	{
		string w = v.first;
		char str[MAX_KEYWORD_LENGTH+1] = { 0 };// �����ַ��������31 ���ֽ�
		int c = w.length();
		if (c > MAX_KEYWORD_LENGTH) c = MAX_KEYWORD_LENGTH;
		strncpy(str, w.c_str(), c);
		//strcpy_s(str, MAX_KEYWORD_LENGTH+1, w.c_str());
		fwrite(str, sizeof(str), 1, fp2);//д���ַ���
		fwrite(&v.second, sizeof(KeywordInfo), 1, fp2);
	}
	fclose(fp2);

	FILE* fp3 = fopen("stash.bin", "wb+");//�洢��ջ����Ϣ
	for (auto& v : stash)
	{
		IndexKey w = v.first;
		fwrite(&v.first, sizeof(v.first), 1, fp3);//
		fwrite(&v.second, sizeof(v.second), 1, fp3);
	}
	fclose(fp3);

	FILE* fp4 = fopen("oram.bin", "wb+");//
	for (auto& v : oram)
	{
		string path = v.first;
		CPath64 p;
		p.LoadPath(path);
		
		fwrite(&p.value, sizeof(p.value), 1, fp4);//
		int len = v.second.bytes.size();
		fwrite(&len, sizeof(len), 1, fp4);//
		for(int i=0;i<len;i++)	fwrite(&v.second.bytes[i], 1, 1, fp4);
		fwrite(&v.second.level, sizeof(v.second.level), 1,fp4);		
	}
	fclose(fp4);
}

void ORAMtree::load()
{
	FILE* fp1 = fopen("p.bin", "rb");
	if (fp1)
	{
		fread(&P, sizeof(P), 1, fp1);
		fclose(fp1);
	}
	init();
	FILE* fp2 = fopen("KeywordInfo.bin", "rb");
	if (fp2)
	{
		localMap.clear();
		while (!feof(fp2))
		{
			char str[MAX_KEYWORD_LENGTH + 1] = { 0 };// �����ַ��������31 ���ֽ�
			fread(str, sizeof(str), 1, fp2);//д���ַ���
			KeywordInfo info;
			fread(&info, sizeof(KeywordInfo), 1, fp2);
			localMap[str] = info;
		}
		fclose(fp2);
	};
	FILE* fp3 = fopen("stash.bin", "rb");//�洢��ջ����Ϣ
	if (fp3)
	{
		stash.clear();
		while (!feof(fp3))
		{
			IndexKey key;
			Slot s;
			fread(&key, sizeof(key), 1, fp3);//
			fread(&s, sizeof(s), 1, fp3);
			stash[key] = s;
		}
		fclose(fp3);
	}
	
	FILE* fp4 = fopen("oram.bin", "rb");//
	if (fp4)
	{
		oram.clear();
		while (!feof(fp4))
		{
			ENode enode;
			string path;
			CPath64 p;
			ull id;
			fread(&id, sizeof(id), 1, fp4);//
			p.value = id;
			path = p.ToString();
			int len = 0;
			fread(&len, sizeof(len), 1, fp4);//
			int i = 0;
			while (!feof(fp4))
			{
				unsigned char b;
				fread(&b, 1, 1, fp4);
				enode.bytes.push_back(b);
				i++;
				if (i == len) break;
				
			}
			fread(&enode.level, sizeof(enode.level), 1, fp4);
			enode.path = path;
			oram[path] = enode;
		}
		fclose(fp4);
	};
	/**/
}


ORAMtree::~ORAMtree()
{
}
