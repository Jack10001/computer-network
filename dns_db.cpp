#include "dns_db.h"
#include<iostream>
dns_db::dns_db()
{
	init_db();
}

dns_db::~dns_db()
{
	SQLDisconnect(hdbc);
	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
	SQLFreeHandle(SQL_HANDLE_ENV, henv);
}


void dns_db::init_db(void)
{
	//分配环境句柄
	retcode = SQLAllocEnv(&henv);
	retcode = SQLAllocHandle(SQL_HANDLE_ENV, NULL, &henv);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {}
	else
	{
		printf("allocate henv fail!\n");
		system("pause");
		exit(0);
	}
	//设置环境句柄属性
	SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER);
	//分配一个连接句柄
	retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{

		//连接数据源
		SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);
		retcode = SQLConnect(hdbc, (SQLCHAR*)"myodbc", SQL_NTS, (SQLCHAR*)NULL, SQL_NTS, (SQLCHAR*)NULL, SQL_NTS);
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		{
			//连接完毕
			//分配语句句柄
			retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {}
			else
			{
				printf("allocate hstmt fail!\n");
				system("pause");
				exit(0);
			}
		}
		else
		{
			printf("connect data source fail!\n");
			system("pause");
			exit(0);
		}
	}
	else
	{
		printf("allocate hdbc fail!\n");
		system("pause");
		exit(0);
	}
}

DNS_record * dns_db::select_(char * domin)
{
	SQLCHAR szSelect[] = "SELECT * FROM　mydnsrelay WHERE dominName='%s'";
	SQLCHAR ipAddr[IP_LENGTH], dominName[DOMIN_LENGTH];
	char ip[IP_LENGTH], domin[DOMIN_LENGTH];

	SQLINTEGER s_len, c_len;
	SQLCHAR query[100];

	sprintf((char *)query, (char *)szSelect, ipAddr);
	retcode = SQLExecDirect(hstmt, (SQLCHAR*)query, SQL_NTS);

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {}
	else
	{
		printf("select error!\n");
		system("pause");
		exit(0);
	}
	//获取结果集,用函数SQLBindCol绑定列，然后用函数SQLFetch遍历记录
	
	 DNS_record dns_records[MAX_RECORD_NUM];
	//std::vector<DNS_record> dns_records;
	retcode = SQLBindCol(hstmt, 1, SQL_C_CHAR, ipAddr, IP_LENGTH, &s_len);
	retcode = SQLBindCol(hstmt, 2, SQL_C_CHAR, dominName, DOMIN_LENGTH, &c_len);
	printf("---------------------------------------------------\n");
	printf(" count     class_id        course_id\n");
	printf("---------------------------------------------------\n");
	int i = 0;
	while (SQLFetch(hstmt) != SQL_NO_DATA)
	{
		//对数据进行打包发送
		printf(" %d |     %s     |    %s\n", i, ipAddr, dominName);
		memcpy(ip, (char*)ipAddr, sizeof((char*)ipAddr));
		memcpy(domin, (char*)dominName, sizeof((char*)dominName));
		dns_records[i].IP = ip;
		dns_records[i].domain = domin;
		i++;
	}

	SQLCloseCursor(hstmt);
	if (i == 0) return NULL;
	else
		return dns_records;
}

void dns_db::updata_record(char* ip_addr,char*domin_name)
{
	SQLCHAR szInsert[] = "INSERT INTO mydnsrelay(ipAddr,dominName) VALUES('%s','%s')";
	SQLSMALLINT i;
	SQLCHAR query[100];

	sprintf((char *)query, (char *)szInsert, ip_addr, domin_name);
	retcode = SQLExecDirect(hstmt, (SQLCHAR*)query, SQL_NTS);
	
	if ((retcode == SQL_SUCCESS_WITH_INFO) || (retcode == SQL_ERROR))
	{
		i = 1;
		while ((rc2 = SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, i, sqlState, &NativeError, Msg, sizeof(Msg), &MsgLen)) != SQL_NO_DATA)
		{
			std::cout << "row : " << i << std::endl;
			std::cout << "sqlState : " << sqlState << std::endl;
			std::cout << "NativeError : " << NativeError << std::endl;
			std::cout << "Msg : " << Msg << std::endl;
			i++;
		}
	}
	SQLCloseCursor(hstmt);
}


