#pragma once
#pragma  comment(lib, "Ws2_32.lib") 

#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h> 
#include <windows.h> 
#include <time.h> 
#include <string>
#include<mutex>
#include<thread>
#include <fstream>
#include <iomanip>
#include<vector>
#include<string>
#include<set>
#include<map>
#include<iostream>
#include<algorithm>



using namespace std;
#pragma warning(disable : 4996)

#define DEF_DNS_ADDRESS "10.3.9.4"	     //����DNS��������ַ
#define LOCAL_ADDRESS "10.128.229.120"		//����DNS��������ַ
#define DNS_PORT 53						//����DNS�����53�˿�
#define BUF_SIZE 512
#define LENGTH 65
#define AMOUNT 1000
#define NOTFOUND -1


/*
HEADER--
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
	unsigned short	type;//2
	unsigned short	qclass;//2
	unsigned int	ttl;//4
	unsigned short	len;//2
}Resource;

//��Ӧ���ĵ� Answer(resource recording)
typedef struct _tagSAnswer
{
	string		domain;
	Resource	resource;
	string		result;//ip or ������
}SAnswer;


//dns�����¼
typedef struct DNS_Record
{
	string IP;						//IP��ַ
	string domain;					//����
}DNS_record;

//IDת����ṹ,�±���ID
typedef struct IDChange
{
	unsigned short oldID;			//ԭ��ID
	BOOL done;						//����Ƿ���ɽ���
	SOCKADDR_IN client;				//�������׽��ֵ�ַ
} IDTransform;



	void DNS_cache_init(const char* tablepath);
	//void GetUrl(char *recvbuf, int recvnum);
	//int IsFind(char* url, int num);
	int IsFind(int num);
	unsigned short RegisterNewID(unsigned short oID, SOCKADDR_IN temp, BOOL ifdone);
	void showtime();
	void DisplayInfo(unsigned short newID, int find);
	void init_socket();
	void init_CMD(int argc, char**argv);
	void update_DNS_cache_db(char *recvbuf, int recv_num);
	void update_dns_cache(vector<DNS_record>& records);
	
     void to_BUPT_DNSServer(char * recvbuf, int iRecv, int find);
   void relay_server(char*recvbuf, int iRecv, int find);