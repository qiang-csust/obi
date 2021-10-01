#include "myAES.h"

#include <aes.h>
#include <math.h>


using namespace std;
using namespace CryptoPP;

AESEncryption aesEncryptor; //������ 
AESDecryption aesDecryptor;


unsigned char inBlock[AES::BLOCKSIZE] = "123456789"; //Ҫ���ܵ����ݿ�
unsigned char outBlock[AES::BLOCKSIZE]; //���ܺ�����Ŀ�
unsigned char xorBlock[AES::BLOCKSIZE]; //�����趨Ϊȫ��
int counter2 = 0;
myAES::myAES()
{

}

void myAES::SetPrivateKey(char* key)
{
	SetPrivateKey(key, strlen(key));
}
void myAES::SetPrivateKey(char * key, int len)
{
	unsigned char aesKey[AES::DEFAULT_KEYLENGTH] = { 0 }; //��Կ
	if (len > AES::DEFAULT_KEYLENGTH) len = AES::DEFAULT_KEYLENGTH;//ֻ������ô�� 16
	memcpy(aesKey, key, len);
	aesEncryptor.SetKey(aesKey, AES::DEFAULT_KEYLENGTH);
	aesDecryptor.SetKey(aesKey, AES::DEFAULT_KEYLENGTH);
}

//����Ϊ��� RCPA-secure
int counter = 0;
void myAES::Encrypt(char * input, int inputlen, char * output, int & outputlen)
{
	//inputlen += 4;//����һ��4�ֽڵļ��������Բ��������
	int groupsize = AES::BLOCKSIZE - 4;//ÿһ��������4���ֽڵ������
	int blocks = ceil((double)inputlen / groupsize);
	for (int i = 0; i < blocks; i++)
	{
		char* pInput = input + i * groupsize;
		if (i < blocks - 1)
		{
			memcpy(inBlock, pInput, groupsize);
			memcpy(inBlock + groupsize, (void*)&counter, 4);
		}
		else
		{
			memset(inBlock, 0, sizeof(inBlock));
			int left = inputlen - i*groupsize;
			memcpy(inBlock, pInput,left);
			memcpy(inBlock+ groupsize, (void*)&counter, 4);//����Ĳ��ֱ���0�����4���ֽڱ�����һ�������������������
		}
		//memset(xorBlock, 0, AES::BLOCKSIZE);
		//aesEncryptor.ProcessAndXorBlock((CryptoPP::byte*)inBlock, xorBlock, (CryptoPP::byte*)(output+ i * AES::BLOCKSIZE)); //����
		aesEncryptor.ProcessBlock((CryptoPP::byte*)inBlock, (CryptoPP::byte*)(output + i * AES::BLOCKSIZE)); //����
		counter2++;
	}
	outputlen = blocks * AES::BLOCKSIZE;
	
}

void myAES::Decrypt(char * input, int inputlen, char * output, int & outputlen)
{
	if (inputlen%AES::BLOCKSIZE != 0)
	{
		printf("error input!");
		return;
	}
	int blocks = inputlen / AES::BLOCKSIZE;
	int groupsize = AES::BLOCKSIZE - 4;
	outputlen = 0;
	char* temp = (char*)malloc(inputlen);
	for (int i = 0; i < blocks; i++)
	{
		char* pInput = input + i * AES::BLOCKSIZE;
		memcpy(inBlock, pInput, sizeof(inBlock));
		//memset(xorBlock, 0, AES::BLOCKSIZE);
		//aesDecryptor.ProcessAndXorBlock((CryptoPP::byte*)inBlock, xorBlock, (CryptoPP::byte*)(temp + i * AES::BLOCKSIZE)); //����
		aesDecryptor.ProcessBlock((CryptoPP::byte*)inBlock, (CryptoPP::byte*)outBlock); //


		memcpy(output + i * groupsize, outBlock, groupsize);
		outputlen += groupsize;//���ܺ������������ˣ���Ϊ��0���뵽BLOCKSIZE-4�ֽ���)	
		counter2++;
	}
	free(temp);

}

int myAES::counter()
{
	return counter2;
}




myAES::~myAES()
{
}
