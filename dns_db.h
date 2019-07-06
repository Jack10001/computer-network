#pragma once
#include"dnsrelay.h"

#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <odbcss.h>

#define MAX_RECORD_NUM  10
class dns_db
{

public:
	dns_db();
	~dns_db();
	void init_db();
	//void close_odbc();
	DNS_record *select_(char*domin_name);//·µ»Øip+dominµÄ¼ÇÂ¼
	void updata_record(char*ip_addr,char*domin_name);
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

