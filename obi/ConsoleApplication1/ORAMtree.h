#pragma once
#include "common.h"
#include "unordered_map"
#include "CPath64.h"
#include "myhash.h"
#include "myAES.h"
#include <vector>
#include <algorithm>

#define BLOCK_SIZE_B 32
#define Constant_Z 4  //ÿһ�����ڵ��е�Z��С
#define Constant_S 4  //ÿһ�����ڵ��е�S��С
#define DUMMY_VALUE 9999999
#define MAX_KEYWORD_LENGTH 31

#define ALGORITHM_PBEA 1
#define ALGORITHM_BOTTOM_TO_TOP 2
#define ALGORITHM_PATHORAM 3
using namespace std;
typedef unsigned long long ull;
struct IndexKey
{
	unsigned char buf[20];
};

//IndexKey toKey(string w, ulong i);   //F'(w||i)
struct DataBlock
{
	//unsigned long long data;//���ڴ洢�ļ���ʶ��
	unsigned long long data[BLOCK_SIZE_B];
};

struct Slot
{
	IndexKey key;// w,i�Ѿ����洢��
	DataBlock value;//�洢�ļ���ʶ����������Ϣ
	unsigned long long leaf;
};

struct TreeNode
{
	vector<Slot> slots;
	int level;
	string path;
};

struct ENode
{
	//vector<Slot> slots;
	vector<unsigned char> bytes;
	int level;
	string path;
};

struct KeywordInfo
{
	unsigned long long wLength;//�����й��ж��ٸ��ؼ���
	unsigned long long searchCounter;//��ǰ�ؼ��ʱ������Ĵ���
};

struct hash_func1
{
	size_t operator()(const IndexKey&addr) const
	{
		return *(size_t*)addr.buf;
	}
};
struct cmp_fun1 //�ȽϺ��� ==
{
	bool operator()(const IndexKey&a1, const IndexKey&a2) const
	{
		return memcmp(a1.buf, a2.buf, sizeof(a1)) == 0 ? true : false;
	}
};


typedef unordered_map<string, ENode> ENodes;
typedef unordered_map<string, TreeNode> TreeNodes;
typedef unordered_map<IndexKey, Slot, hash_func1, cmp_fun1> KeyValues;
//typedef multimap<unsigned long long, Slot, hash_func1, cmp_fun1> orderedMap;
//typedef map<unsigned long long, vector<Slot>> orderedMap;//��Ҷ��Ϊ���������Ի�������������
//typedef vector<TreeNode> Nodes;


//Ҷ�ӵ�LID���룬��Ҷ����߿�ʼ��һ��Ҷ��Ϊ0�����α��Ϊ1,, x_LID;

struct Param
{
	int Z = Constant_Z;// 4;// 4;//ÿ���ڵ���Z��slot
	int S = Constant_S;// 6;//ÿһ�������ڵ���S��slot
	int L = 32;//����Ĭ�ϸ߶�
	int n_root_exceed_S = 0;
	int Max_S_outofdate = 0;//����
	ull capacity = 0;//����
	ull w_block_count = 0;//�������ܹ��ģ������ظ��ģ�����ɾ���ģ�
	ull firstLeafID = 0;//
	ull leafCount = 0;
	int aesCounter1 = 0;
	int Blake2bCounter1 = 0;
	int shuffle_choice = ALGORITHM_BOTTOM_TO_TOP;//=ALGORITHM_BOTTOM_PBEA ��ʾ�����µķ��������㷨��=ALGORITHM_BOTTOM_TO_TOP��ʾ�����ϵķǷ����㷨
	int max_S = 0;//����stash size ������stash��
	int band_width = 0;//ͳ�ƴ���ʹ��
	int max_Stash_Evict = 1024; //stash��������ֵ����������
};
struct DebugParam
{
	ull PATH_pathObjInit;
	ull PATH_stashRemove;	
	double PATH_ORAM_chooseTime;
	ull BottomToTop_ObjInit;
	ull BottomToTop_stashRemove;
	double BottomToTop_removeTime;
	ull readBytes;
};
class ORAMtree
{
public:
	string privateKey = "1234567890123456"; //16λ
	Param P;
	DebugParam debug;
	unordered_map <string, KeywordInfo> localMap;
	unordered_map<IndexKey, Slot, hash_func1, cmp_fun1> stash;
	unordered_map<string, ENode> oram;

	ORAMtree(int m_L, int m_S, int m_Z);
	ORAMtree(int m_L);
	void init();
	void initAES();
//	void addBlock(string w, DataBlock block,bool bForceAdd=false);
	void addBlock(string w, DataBlock block, bool bForceAdd, bool bUseEvict=true);
	void Evict0(int times=1);
	void Evict(int times=1);
	ull addBlockLocal(string w, DataBlock block, bool bForceAdd);

public: //User;
	void add(string w, unsigned long long id,bool bForce=false);

	//void add(string w, vector<ulong> ids, bool bForce, bool bUseEvict);

	void add(string w, vector<ull> ids, bool bForce=false,bool bUseEvict=true);
	void insertBatch(string w, ull from, ull to);
	void insertBatchLocal(string w, ull from, ull to);
	void SetupLocal(unordered_map<string, vector<ull>>& inverted_index);
	void SetupOnline(unordered_map<string, vector<ull>>& inverted_index);
	//�ӵ�����������
	ull addLocal(string w, vector<ull> ids, bool bForce = false);

	void del(string w, unsigned long long id);

	//����һ���ؼ���/�ĵ� ����id�����λ���ڱ�־�Ƿ������ӻ���ɾ����
	void search(string w, vector<unsigned long long>& ids, bool bDebug = true);
	//void del(string w, unsigned long long id);//ɾ��һ���ؼ���/�ĵ�


public://User
	//nodes ���м���������Ҷ�ӽڵ�
	//
	IndexKey toIndexKey(string w, ull i);   //F'(w||i)
	TreeNodes RebuildPaths(TreeNodes nodes,vector<ull> leafs,string op, IndexKey key, DataBlock input_data, DataBlock& output_data); //ϴ��
	bool ReadWriteFromStash(string op, ull newLeaf, IndexKey key, DataBlock input_data, DataBlock & output_data);
	int FindOneSlotFromSortedVector(vector<Slot>& slots, Slot tobeSearch);
	//int FindOneSlotFromSortedMap(orderedMap& stashMap, Slot tobeSearch);
	void DeleteOneSlotFromVector(vector<Slot>& slots, Slot tobeDelete);
	void FindZNearbySlotFromVector(vector<Slot>& slots, ull leaf, vector<Slot>& Z_results, int size);
	void evictPathsPATHORAM(unordered_map<string, TreeNode>& pathNodes, vector<unsigned long long> leafs);
	//void FindZNearbySlotFromMap(orderedMap& stashMap, ulong leaf, vector<Slot>& Z_results, int size);
	void evictPath(unordered_map<string, TreeNode>& pathNodes, vector<Slot> slots, ull leaf);
	//void evictPathOld(unordered_map<string, TreeNode>& pathNodes, vector<Slot> slots, ulong leaf);
	void sortByLeafDistance(vector<Slot>& slots, ull leaf);
	void evictPath1(unordered_map<string, TreeNode>& pathNodes, vector<Slot> slots, ull leaf);
	void evictPathAtLevel(unordered_map<string, TreeNode>& pathNodes, KeyValues& removed, vector<Slot> slots, ull leaf, int level);
	void evictPath3(unordered_map<string, TreeNode>& pathNodes, vector<Slot> slots, ull leaf);
	void evictPath2(unordered_map<string, TreeNode>& pathNodes, vector<Slot> slots, ull leaf);
	DataBlock EmptyBlock();
	bool isEmptyBlock(DataBlock & b);
	ull toDelIdentifer(ull value);
	bool isDelIdentifer(ull value, ull & output);//identifer�����һλ���ڱ�ʾ�Ƿ�ɾ�������಻��
	bool deleteFromVector(vector<ull>& data, ull tobeDeleted);
	void EncryptPaths(unordered_map<string, TreeNode>& pathNodes, ENodes & enodes);

	unsigned long long separationPoint(unsigned long long leaf1, unsigned long long leaf2);
	ENodes RebuildPaths_PBEA(vector<ull>& leafs, int evictOption);
	ENodes RebuildPaths_PartitionFirstOld(vector<ull>& leafs, int evictOption);
	//ENodes RebuildPaths_PartitionFirst(vector<ulong>& leafs);
	ENodes RebuildPathsOld(vector<ull>& leafs);
	ENodes RebuildPaths_StackedLocation(vector<ull>& leafs, int evictOption);
	ENodes RebuildPaths(vector<ull>& leafs,int option); //ϴ�� ���������Ѿ�����stash����
	ENodes RebuildPaths_nopartition(vector<ull>& leafs);
	void evict_Paths_KNNEA(unordered_map<string, TreeNode>& pathNodes, vector<Slot>& stashArray, vector<ull>& leafs);
	void evict_Paths_KNNEA(unordered_map<string, TreeNode>& pathNodes, vector<ull>& leafs);//��stash����������·����
	//ENodes RebuildPaths_nopartition_old(vector<ulong>& leafs);
	//ulong GetInitialPos(Slot& s);
	ENodes RebuildPaths_initial();//��ʼ��ʱ�� ���ع������������ݷ�Ҷ�ӣ�ʣ��Ĳ��ַű��ػ�����stash����
	ull SearchToken(string w, ull i, ull w_searchtimes); // F_K(w || i || w_searchtimes)  �������Ҷ�ӵ�λ��,
	TreeNode DecryptNode(ENode e);
	ENode EncryptNode(TreeNode d);
	TreeNodes DecryptNodes(ENodes e);
	ENodes EncryptNodes(TreeNodes d);
	void WriteIntoStash(ENodes enodes);
public:
	void LocalInsert(vector<Slot> blocks);
	//cloud
	ORAMtree();

	ENodes ReadPath(unsigned long long leaf);
	ENodes ReadPaths(vector<ull> leafs);
	void writePath(ENodes nodes);
	void dump();
	void info();
	void dumpdata();
	void save();
	void load();
	void dumpLevel();
	void dumpStashUsage();

	void DebugTestIsInPathOrInStash();
	
	~ORAMtree();
	int FindOneSlotFromSortedVector(vector<Slot>& slots, ull leaf);
};







