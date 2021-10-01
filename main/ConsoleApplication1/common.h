#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "windows.h"
#include "string"
#include "vector"

//#define byte unsigned char


using namespace std;

class common
{
public:
	common();
	~common();
};

class MyTimer
{
public:
	MyTimer()
	{
		QueryPerformanceFrequency(&m_Frequency);
		Start();
	}
	void Start()
	{
		QueryPerformanceCounter(&m_StartCount);
	}
	double End()
	{
		LARGE_INTEGER CurrentCount;
		QueryPerformanceCounter(&CurrentCount);
		return double(CurrentCount.LowPart  - m_StartCount.LowPart) / (double)m_Frequency.LowPart;
	}
	void ShowNow()
	{
		LARGE_INTEGER CurrentCount;
		QueryPerformanceCounter(&CurrentCount);
		//cout << "Timer Count is:" << double(CurrentCount.LowPart  - m_StartCount.LowPart) / (double)m_Frequency.LowPart << endl;
	}
private:
	LARGE_INTEGER m_Frequency;
	LARGE_INTEGER m_StartCount;
};
void getFiles(const std::string & path, std::vector<std::string> & files); 
void getDirs(const std::string & path, std::vector<std::string> & files);
void split(const string& src,const string& separator, vector<string>& dest);
void split(vector<string>& result, string str, vector<char> delimiters);
string ReadAllFile(char* filename);
bool FileExists(string file);
string Trim(const string& str);
void TrimSpace(char* str);
string replace(const string& str, const string& src, const string& dest);
char* currentTime(char* ch);
int64_t time64();
double time_ms();
double time_us();

string IntToStr(int number);
int StrToInt(string s);
string GetCurrentDir();

char * strLeft(const char *src, unsigned int n);
char * strMid(const char *src, unsigned int startIdx, unsigned int n);
string strRight(const char *src, unsigned int n);
string BufToStr(unsigned char* p, int len);
int PathToInt(string path);
string IntToPath(int i);
int PathSubStringInt(int v, int pathLen);//ȡ���ַ���������߿�ʼ��ȡ �����Ϊint�����λ��
int GetIntPathLength(int i);
int GetFatherIntPath(int i);

//����·������ָʾ��ָ������CNF��ѯ�е�λ��i��j
struct PathWithNumber
{
	int path;
	int disjunctive_i;//from 0
	int conjunctive_j;//from 1
};

// 001
// 0011
// 00111 
// ������� 00111, ������ֲ���ȵģ��򷵻�0
//int GetOverlapPath(vector<int> paths);//��ȡ���
//int GetOverlapPath(vector<int> paths, vector<int>& lens, int& max);
int GetOverlapPath(vector<int> paths, vector<int>& lens, int& max, int& max_i);
int GetOverlapedLongestPath(vector<int> paths);//��ȡ��̵�
//PathWithNumber GetOverlapedLongestPath(vector<PathWithNumber> paths);
//PathWithNumber GetOverlapedLongestPath(vector<PathWithNumber> paths, vector<int>& lens);
int GetOverlapedLongestPath(vector<int>& paths, vector<int>& lens);
//PathWithNumber GetOverlapPath(vector<PathWithNumber> paths, vector<int>& lens, int& max);
bool getBit( int & input, const unsigned char n);
void setBit( int & input, const unsigned char n);
void clearBit( int & input, const unsigned char n);
void RandomArray(vector<int>& v, int count, int max);
void RandomArrayWithExclusion(vector<int>& v, int count, int max, vector<int> &except);
vector<string> ReadAllKeywords(string filename);
string rndStr(int len);

int rand32();
long long rand64();
void Log(const char* filename, const char* fmt, ...);
string Now();