#pragma once
#pragma  comment(lib, "Ws2_32.lib") 
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h> 
#include <windows.h> 
#include <time.h> 
#include <string>
#include<mutex>
#include<thread>
#include <iostream>
#include <fstream>
#include <iomanip>
#include<vector>
#include<string>

using namespace std;
#pragma warning(disable : 4996)

#define DEF_DNS_ADDRESS "10.3.9.4"	     //����DNS��������ַ
#define LOCAL_ADDRESS "127.0.0.1"		//����DNS��������ַ
#define DNS_PORT 53						//����DNS�����53�˿�
#define BUF_SIZE 512
#define LENGTH 65
#define AMOUNT 1000
#define NOTFOUND -1
#define IP_LENGTH 16+1
#define DOMIN_LENGTH 64+1

#pragma pack(2)//2�ֽڶ���
#define DNS_FLAG_QR			0x8000	//��ѯ(0) | ��Ӧ(1)
#define DNS_FLAG_OPCODE		0x7000	//��׼��ѯ(0) | �����ѯ(1) | ������״̬����(2) | ����(3-15)
#define DNS_FLAG_AA			0x0400	//Ȩ���ش�(1) | ��Ȩ���ش�(0), ��Ӧ����Ч
#define DNS_FLAG_TC			0x0200	//�ض�(1), ����λΪ1, ����ʹ��TCP��������
#define DNS_FLAG_RD			0x0100	//�����ݹ�
#define DNS_FLAG_RA			0x0080	//�ݹ����
#define DNS_FLAG_Z			0x0070	//����
#define DNS_FLAG_RCODE		0x000f	//û�д���(0) | ��ʽ����(1) | ����������(2) | ���ƴ���(3) | δʵ��(4) | �ܾ�(5) | ����(6-15)



//DNS�����ײ�

/*
ID: 2 bytes
FLAGS : 2 bytes
QDCOUNT : 2 bytes
ANCOUNT : 2 bytes
NSCOUNT : 2 bytes
ARCOUNT : 2 bytes
*/
typedef struct DNSHeader
{
	unsigned short ID;
	unsigned short Flags;
	unsigned short QuestNum;
	unsigned short AnswerNum;
	unsigned short AuthorNum;
	unsigned short AdditionNum;
} DNS_HDR;

//�����ʽ�ģ� QTYPE,QCLASS
typedef struct _tagQuestion
{
	unsigned short type;
	unsigned short qclass;
}Question;


//�û�ʹ�õ�Question : QNAME,QTYPE,QCLASS
//QNAME ���������������
typedef struct _tagSQuestion
{
	string		domain;
	Question	question;
}SQuestion;



//��Դ��¼��ʽ
/*
NAME 
TYPE
CLASS
TTL
RDLENGTH
RDATA
*/
typedef struct _tagResource
{
	unsigned short	type;
	unsigned short	qclass;
	unsigned int	ttl;
	unsigned short	len;
}Resource;

//��Ӧ���ĵ� Answer(resource recording)
typedef struct _tagSAnswer
{
	string		domain;
	Resource	resource; 
	string		result;//ip or ������
}SAnswer;



//dns�����¼
typedef struct DNS_record
{
	string IP;						//IP��ַ
	string domain;					//����
} DNS_record;


//IDת����ṹ,�±���ID
typedef struct IDChange
{
	unsigned short oldID;			//ԭ��ID
	BOOL done;						//����Ƿ���ɽ���
	SOCKADDR_IN client;				//�������׽��ֵ�ַ
} IDTransform;


DNS_record DNS_cache[AMOUNT];		//DNS�������ݽṹ
IDTransform IDTransTable[AMOUNT];	//IDת����

// vector<DNS_record> DNS_cache;
//vector<IDTransform> IDTranslate

int IDcount = 0;					//ת�����е���Ŀ����


char url[LENGTH];					//����


SYSTEMTIME sys;                     //ϵͳʱ��
int Day, Hour, Minute, Second, Milliseconds;//����ϵͳʱ��ı���


int DNS_cache_init(char* tablepath);

void GetUrl(char *recvbuf, int recvnum);

int IsFind(char* url, int num);

unsigned short RegisterNewID(unsigned short oID, SOCKADDR_IN temp, BOOL ifdone);
void showtime();
void DisplayInfo(unsigned short newID, int find);