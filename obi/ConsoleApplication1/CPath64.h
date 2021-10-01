#pragma once
#include <string>
using namespace std;
class CPath64 //节点编码类
{
public:
	CPath64();
	CPath64(long long node_value); //输入值为节点编码
	CPath64 LeftChild();//左边节点
	CPath64 RightChild();//右边节点
	CPath64 Sibiling();//兄弟节点
	CPath64 Father();//父亲节点
	string ToString();//得到当前节点路径
	CPath64 Cross(long long leaf1, long long leaf2);//2个叶子的交叉节点 叶子采用节点编码

	///height 是树的最大高度，x_LID为叶子(从左边开始数0开始)，l 是第几级 （root l=1）
	string P(long long x_LID, int l, int height);
	string AtLevelPath(int l);
	unsigned long long AtLevelPathFast(int l,int L);
	//第几层的路径
	void LoadPath(string path); //以路径方式载入节点
	void LoadLeaf(int level,long long leaf);//以节点+层 的方式载入节点
	unsigned long long MapToRandomLeaf(int height);
	unsigned long long RandomLeaf(int height);
	//映射到一个随机的叶子
	
	unsigned long long GetLeaf();//读取从左边开始到当前位置的位置，最左边的节点leaf=0
	unsigned long long value; //根节点value=0 ，左边 2i+1，右边2i+2
	
	int GetLevel();//获取当前节点的高度 （Root L=1）
	~CPath64();
};

//Number从0开始编号 左边孩子为2*i+1，右边孩子为2*i+2
unsigned long long GetFatherNumber(unsigned long long node);
unsigned long long GetLeftChildNumber(unsigned long long node);//2*I+1
unsigned long long GetRightChildNumber(unsigned long long node);//2*I+2
string NumberToPath(unsigned long long node);
unsigned long long PathToNumber(string path);

