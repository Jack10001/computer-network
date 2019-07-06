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

#define DEF_DNS_ADDRESS "10.3.9.4"	     //北邮DNS服务器地址
#define LOCAL_ADDRESS "127.0.0.1"		//本地DNS服务器地址
#define DNS_PORT 53						//进行DNS服务的53端口
#define BUF_SIZE 512
#define LENGTH 65
#define AMOUNT 1000
#define NOTFOUND -1
#define IP_LENGTH 16+1
#define DOMIN_LENGTH 64+1

#pragma pack(2)//2字节对齐
#define DNS_FLAG_QR			0x8000	//查询(0) | 响应(1)
#define DNS_FLAG_OPCODE		0x7000	//标准查询(0) | 反向查询(1) | 服务器状态请求(2) | 保留(3-15)
#define DNS_FLAG_AA			0x0400	//权威回答(1) | 非权威回答(0), 响应中有效
#define DNS_FLAG_TC			0x0200	//截断(1), 若该位为1, 将会使用TCP重新请求
#define DNS_FLAG_RD			0x0100	//期望递归
#define DNS_FLAG_RA			0x0080	//递归可用
#define DNS_FLAG_Z			0x0070	//保留
#define DNS_FLAG_RCODE		0x000f	//没有错误(0) | 格式错误(1) | 服务器故障(2) | 名称错误(3) | 未实现(4) | 拒绝(5) | 保留(6-15)



//DNS报文首部

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
	unsigned short	type;
	unsigned short	qclass;
	unsigned int	ttl;
	unsigned short	len;
}Resource;

//相应报文的 Answer(resource recording)
typedef struct _tagSAnswer
{
	string		domain;
	Resource	resource; 
	string		result;//ip or 主机名
}SAnswer;



//dns缓存记录
typedef struct DNS_record
{
	string IP;						//IP地址
	string domain;					//域名
} DNS_record;


//ID转换表结构,下标做ID
typedef struct IDChange
{
	unsigned short oldID;			//原有ID
	BOOL done;						//标记是否完成解析
	SOCKADDR_IN client;				//请求者套接字地址
} IDTransform;


DNS_record DNS_cache[AMOUNT];		//DNS缓存数据结构
IDTransform IDTransTable[AMOUNT];	//ID转换表

// vector<DNS_record> DNS_cache;
//vector<IDTransform> IDTranslate

int IDcount = 0;					//转换表中的条目个数


char url[LENGTH];					//域名


SYSTEMTIME sys;                     //系统时间
int Day, Hour, Minute, Second, Milliseconds;//保存系统时间的变量


int DNS_cache_init(char* tablepath);

void GetUrl(char *recvbuf, int recvnum);

int IsFind(char* url, int num);

unsigned short RegisterNewID(unsigned short oID, SOCKADDR_IN temp, BOOL ifdone);
void showtime();
void DisplayInfo(unsigned short newID, int find);