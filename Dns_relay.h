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

#define DEF_DNS_ADDRESS "10.3.9.4"	     //北邮DNS服务器地址
#define LOCAL_ADDRESS "10.128.229.120"		//本地DNS服务器地址
#define DNS_PORT 53						//进行DNS服务的53端口
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

//问题格式的： QTYPE,QCLASS
typedef struct _tagQuestion
{
	unsigned short type;
	unsigned short qclass;
}Question;

//用户使用的Question : QNAME,QTYPE,QCLASS
//QNAME 就是所请求的域名
typedef struct _tagSQuestion
{
	string		domain;
	Question	question;
}SQuestion;

//资源记录格式
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

//相应报文的 Answer(resource recording)
typedef struct _tagSAnswer
{
	string		domain;
	Resource	resource;
	string		result;//ip or 主机名
}SAnswer;


//dns缓存记录
typedef struct DNS_Record
{
	string IP;						//IP地址
	string domain;					//域名
}DNS_record;

//ID转换表结构,下标做ID
typedef struct IDChange
{
	unsigned short oldID;			//原有ID
	BOOL done;						//标记是否完成解析
	SOCKADDR_IN client;				//请求者套接字地址
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