#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <odbcss.h>
#include<iostream>
#include<vector>
using namespace std;
#pragma warning(disable : 4996)

#define IPLENGTH 16+1
#define DOMIN_LENGTH 64+1

typedef struct DNS_Record_
{
	string IP;
	string domain;
}DNS_record_;


class DnsDb
{
public:
	DnsDb();
	~DnsDb();
	void init_db();
	void update_record(const char*ip_addr, const  char*domin_name);
	vector<DNS_record_> select_(const char * domain_name);//more than one ip address
private:
	SQLHENV henv;
	SQLHDBC hdbc;
	SQLHSTMT hstmt;
	RETCODE retcode;

	SQLCHAR sqlState[6], Msg[SQL_MAX_MESSAGE_LENGTH];
	SQLINTEGER NativeError;
	SQLSMALLINT  MsgLen;
	SQLRETURN rc2;
};

