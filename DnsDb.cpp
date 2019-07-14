
#include "DnsDb.h"
DnsDb::DnsDb() {}

DnsDb::~DnsDb()
{
	SQLDisconnect(hdbc);
	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
	SQLFreeHandle(SQL_HANDLE_ENV, henv);
}


void DnsDb::init_db(void)
{

	retcode = SQLAllocEnv(&henv);
	retcode = SQLAllocHandle(SQL_HANDLE_ENV, NULL, &henv);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {}
	else
	{
		printf("allocate henv fail!\n");
		system("pause");
		exit(0);
	}

	SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER);

	retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{

		SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);
		retcode = SQLConnect(hdbc, (SQLCHAR*)"myodbc", SQL_NTS, (SQLCHAR*)NULL, SQL_NTS, (SQLCHAR*)NULL, SQL_NTS);
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		{

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

vector<DNS_record_> DnsDb::select_(const char * domain)
{
	SQLCHAR szSelect[] = "SELECT * FROM mydnsrelay WHERE dominName='%s'";
	SQLCHAR ipAddr[IPLENGTH], dominName[DOMIN_LENGTH];
	char ip[IPLENGTH], domin_[DOMIN_LENGTH];

	SQLINTEGER s_len, c_len;
	SQLCHAR query[100];

	sprintf((char *)query, (char *)szSelect, domain);
	retcode = SQLExecDirect(hstmt, (SQLCHAR*)query, SQL_NTS);

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {}
	else
	{
		printf("select error!\n");
		system("pause");
		exit(0);
	}

	//DNS_record dns_records[MAX_RECORD_NUM];
	vector<DNS_record_> dns_records;
	DNS_record_ dns_record;
	//std::vector<DNS_record> dns_records;
	retcode = SQLBindCol(hstmt, 1, SQL_C_CHAR, ipAddr, IPLENGTH, &s_len);
	retcode = SQLBindCol(hstmt, 2, SQL_C_CHAR, dominName, DOMIN_LENGTH, &c_len);
	int i = 0;
	while (SQLFetch(hstmt) != SQL_NO_DATA)
	{

		//printf(" %d |     %s     |    %s\n", i, ipAddr, dominName);
		//memcpy(ip, (char*)ipAddr, sizeof((char*)ipAddr));
		//memcpy(domin_, (char*)dominName, sizeof((char*)dominName));
		strcpy(ip, (char*)ipAddr);
		strcpy(domin_, (char*)dominName);
		dns_record.domain = domin_;
		dns_record.IP = ip;
		dns_records.push_back(dns_record);
		i++;
	}
	for (auto it = dns_records.begin(); it != dns_records.end(); it++)
	{
		cout << (*it).IP.c_str() << " " << (*it).domain.c_str() << endl;
	}

	SQLCloseCursor(hstmt);
	cout << "dns records from database result :"<<dns_records.size()<<endl;
	return dns_records;
}

void DnsDb::update_record(const char* ip_addr, const char*domin_name)
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


