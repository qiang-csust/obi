#pragma once
#include <string>
using namespace std;
class CPath64 //�ڵ������
{
public:
	CPath64();
	CPath64(long long node_value); //����ֵΪ�ڵ����
	CPath64 LeftChild();//��߽ڵ�
	CPath64 RightChild();//�ұ߽ڵ�
	CPath64 Sibiling();//�ֵܽڵ�
	CPath64 Father();//���׽ڵ�
	string ToString();//�õ���ǰ�ڵ�·��
	CPath64 Cross(long long leaf1, long long leaf2);//2��Ҷ�ӵĽ���ڵ� Ҷ�Ӳ��ýڵ����

	///height ���������߶ȣ�x_LIDΪҶ��(����߿�ʼ��0��ʼ)��l �ǵڼ��� ��root l=1��
	string P(long long x_LID, int l, int height);
	string AtLevelPath(int l);
	unsigned long long AtLevelPathFast(int l,int L);
	//�ڼ����·��
	void LoadPath(string path); //��·����ʽ����ڵ�
	void LoadLeaf(int level,long long leaf);//�Խڵ�+�� �ķ�ʽ����ڵ�
	unsigned long long MapToRandomLeaf(int height);
	unsigned long long RandomLeaf(int height);
	//ӳ�䵽һ�������Ҷ��
	
	unsigned long long GetLeaf();//��ȡ����߿�ʼ����ǰλ�õ�λ�ã�����ߵĽڵ�leaf=0
	unsigned long long value; //���ڵ�value=0 ����� 2i+1���ұ�2i+2
	
	int GetLevel();//��ȡ��ǰ�ڵ�ĸ߶� ��Root L=1��
	~CPath64();
};

//Number��0��ʼ��� ��ߺ���Ϊ2*i+1���ұߺ���Ϊ2*i+2
unsigned long long GetFatherNumber(unsigned long long node);
unsigned long long GetLeftChildNumber(unsigned long long node);//2*I+1
unsigned long long GetRightChildNumber(unsigned long long node);//2*I+2
string NumberToPath(unsigned long long node);
unsigned long long PathToNumber(string path);

