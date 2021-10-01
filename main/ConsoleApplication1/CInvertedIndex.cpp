#include "CInvertedIndex.h"
//#include<iostream>

#define MAX_KEYWORD_LENGTH 31  //"最多31个字符"
CInvertedIndex::CInvertedIndex()
{

}

void CInvertedIndex::GetFileDistinctKeywords(string filename, vector<string>& keywords)
{
	string txt = ReadAllFile((char*)filename.c_str());
	if (txt.length() == 0) return;
	vector<string> dest;
	//split(txt, " ", dest);
	vector<char> v;
	v.push_back(' ');
	v.push_back('\r');
	v.push_back('\n');
	split(dest, txt, v);
	unordered_map<string, int> my_map;
	for (auto& vv : dest)
	{
		string v = Trim(vv);
		if (v.length() > MAX_KEYWORD_LENGTH) continue;
		if (v.length() == 0) continue;
		if (my_map.find(vv) == my_map.end())
		{
			my_map[vv] = 1;
			keywords.push_back(vv);
		}
	}
}

//插入一个文件到 倒排索引
void CInvertedIndex::InsertFile(string filename)
{
	vector<string> keywords;
	GetFileDistinctKeywords(filename, keywords);
	int id = fileNames.size() + 1;
	for (auto& w : keywords)
	{
		vector<unsigned long long> ids = DB[w];
		ids.push_back(id);
		DB[w] = ids;
		N++;
	}
	fileNames[id] = filename;//保存文件名
	id++;//文件编号
	n++;
}
void CInvertedIndex::LoadDirToInvertedIndex(string dir,unsigned long long maxsize)
{
	vector<string> results;
	getFiles(dir, results);
	//vector<fileInfo_TF_IDF> files;

	unsigned long long id = 1;
	N = 0;
	m = 0;
	n = 0;
	for (auto& f : results)
	{
		vector<string> keywords;
		GetFileDistinctKeywords(f, keywords);		
		for (auto& w : keywords)
		{
		//	vector<unsigned long long> ids = DB[w];
		//	ids.push_back(id);
		//	DB[w] = ids;
			if (N >= maxsize) break;
			DB[w].push_back(id);
			N++;

		}
		if (N >= maxsize) break;
		fileNames[id] = f;
		id++;//文件编号
		n++;
		//fileInfo_TF_IDF f;
		//f.filename = v;
		//f.keywords = keywords;
		//files.push_back(f);
	}
	m = DB.size();
}

string CInvertedIndex::ReadLine(FILE* fp,char* buf)
{
	int i = 0;
	while (!feof(fp))
	{
		fread(buf+i, 1, 1, fp);
		if (buf[i] == '\r') break;
		if (buf[i] == '\n') break;
		i++;
	}
	buf[i] = 0;
	return string(buf, i);
}
void CInvertedIndex::Load(unsigned long long maxsize,unsigned long long maxm)
{
	FILE* fp = fopen("inverted_index.txt", "rb");
	N = 0;
	m = 0;
	n = 0;
	DB.clear();
	char* buf = (char*)malloc(1024 * 1024*1024);
	unordered_map<unsigned long long, int> fileids;
	while (!feof(fp))
	{
		//char buf[1024] = { 0 };
		//int r=fscanf_s(fp, "%[^\n]", buf,sizeof(buf);
		//int r = fscanf(fp, "%s", buf);
		//if (r <= 0) break;
		string line = ReadLine(fp, buf);
		string v=line;
		if (v.size() == 0) break;
		vector<string> ss;
		split(v, " ",ss);
		if (ss.size() == 0) break;
		string w = ss[0];
		//if (w.length() > MAX_KEYWORD_LENGTH) continue;//不处理超长的字符串
		vector<unsigned long long> ids;
		if (N >= maxsize) break;
		for (int i = 1; i < ss.size(); i++)
		{
			unsigned long long id = atoi(ss[i].c_str());
			if (id <= 0) continue;
			ids.push_back(id);
			fileids[id] = 1;//此文档已经被使用
			N++;
			if (N >= maxsize) break;
		}
		DB[w] = ids;		
		m++;
		if (m >= maxm) break;//关键词足够了
		fgetc(fp);
	}
	fclose(fp);
	
	fileNames.clear();
	FILE* fp2 = fopen("fileNames.txt", "rb");//load all the filenames
	while (!feof(fp2))
	{
		char buf[1024] = { 0 };
		int r = fscanf(fp2, "%[^\n]", buf);
		if (r <= 0) break;
		string v = buf;
		vector<string> ss;
		split(v, " ", ss);
		if (ss.size() != 2) break;
		int id = atoi(ss[0].c_str());
		if(id>0) fileNames[id] = ss[1];
		fgetc(fp2);
	}
	fclose(fp2);
	/**/
	//n = fileNames.size();
	//m = DB.size();
	n = fileids.size();
	free(buf);
}

void CInvertedIndex::Save()
{
	FILE* fp = fopen("inverted_index.txt", "wb+");
	for (auto& v : DB)
	{
		string w = v.first;
		fprintf(fp,"%s ", w.c_str());
		for (auto id : v.second)
		{
			fprintf(fp, "%d ", id);
		}
		fprintf(fp, "\r\n");
	}
	fclose(fp);

	FILE* fp2 = fopen("fileNames.txt", "wb+");
	for (auto v : fileNames)
	{
		fprintf(fp2, "%d %s\r\n", v.first, v.second.c_str());
	}
	fclose(fp2);
}
