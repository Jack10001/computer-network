#include"dnsrelay.h"
#include"dns_db.h"

//dns缓存初始化
int DNS_cache_init(char *tablePath)
{
	cout << "Try to load table 'dnsrelay.txt'";
	int i = 0,pos = 0;
	string table[AMOUNT];
	ifstream infile;
	try
	{
		infile.open(tablePath,ios::in);
	}
	catch (const exception& e)
	{
		cerr << e.what() << endl;
		cerr << "Open " << tablePath << " error!" << endl;
		system("pause");
		exit(1);
	}
	while (getline(infile, table[i]) && i < AMOUNT)
		i++;

	if (i == AMOUNT - 1)
		cout << "The DNS table memory is full. " << endl;

	for (int j = 0; j < i - 1; j++) {
		pos = table[j].find(' ');
		if (pos > table[j].size())
			cout << "The record is not in a correct format. " << endl;
		else {
			DNS_cache[j].IP = table[j].substr(0, pos);
			DNS_cache[j].domain = table[j].substr(pos + 1);
		}
	}
	infile.close();		//关闭文件
	cout << "  ...OK" << endl;
	return i - 1;			//返回域名解析表中条目个数
}



//函数：获取DNS请求中的域名
//换一下


void GetUrl(char *recvbuf, int recvnum)
{
	char urlname[LENGTH];
	int i = 0, j, k = 0;

	memset(url, 0, LENGTH);
	memcpy(urlname, &(recvbuf[sizeof(DNS_HDR)]), recvnum - 16);	//获取请求报文中的域名表示

	int len = strlen(urlname);

	//域名转换
	while (i < len) {
		if (urlname[i] > 0 && urlname[i] <= 63)
			for (j = urlname[i], i++; j > 0; j--, i++, k++)
				url[k] = urlname[i];

		if (urlname[i] != 0) {
			url[k] = '.';
			k++;
		}
	}
	url[k] = '\0';
	cout << "URL:" << url << endl;
}


//函数：判断是否在表中找到DNS请求中的域名，找到返回下标
int IsFind(char* url, int num)
{
	int find = NOTFOUND;
	char* domain;

	for (int i = 0; i < num; i++) {
		domain = (char *)DNS_cache[i].domain.c_str();
		if (strcmp(domain, url) == 0) {	//找到
			find = i;
			break;
		}
	}

	return find;
}





//函数：将请求ID转换为新的ID，并将信息写入ID转换表中
unsigned short RegisterNewID(unsigned short oID, SOCKADDR_IN temp, BOOL ifdone)
{
	srand(time(NULL));
	IDTransTable[IDcount].oldID = oID;
	IDTransTable[IDcount].client = temp;
	IDTransTable[IDcount].done = ifdone;
	IDcount++;

	return (unsigned short)(IDcount - 1);	//以表中下标作为新的ID
}





void showtime() {
	char tmp[64];
	time_t t = time(NULL);
	tm *_tm = localtime(&t);
	int year = _tm->tm_year + 1900;
	int month = _tm->tm_mon + 1;
	int date = _tm->tm_mday;
	int hh = _tm->tm_hour;
	int mm = _tm->tm_min;
	int ss = _tm->tm_sec;
	sprintf(tmp, "%02d:%02d:%02d",hh, mm, ss);
	//return string(tmp);
	cout << tmp << '\t';
}






void DisplayInfo(unsigned short newID, int find)
{
	//打印时间
	//GetLocalTime(&sys);
	/*if (sys.wMilliseconds >= Milliseconds)
	{
		cout << setiosflags(ios::right) << setw(7) << setfill(' ') << (((sys.wDay - Day) * 24 + sys.wHour - Hour) * 60 + sys.wMinute - Minute) * 60 + sys.wSecond - Second;//设置宽度为7，right对齐方式
		cout << '.' << setiosflags(ios::right) << setw(3) << setfill('0') << sys.wMilliseconds - Milliseconds;
	}
	else {
		cout << setiosflags(ios::right) << setw(7) << setfill(' ') << (((sys.wDay - Day) * 24 + sys.wHour - Hour) * 60 + sys.wMinute - Minute) * 60 + sys.wSecond - Second - 1;//设置宽度为7，right对齐方式
		cout << '.' << setiosflags(ios::right) << setw(3) << setfill('0') << 1000 + sys.wMilliseconds - Milliseconds;
	}*/
	showtime();
	cout << "  ";

	//打印转换后新的ID
	cout.setf(ios::left);
	cout << setiosflags(ios::left) << setw(4) << setfill(' ') << newID;
	cout << "    ";






	//在表中没有找到DNS请求中的域名
	if (find == NOTFOUND)
	{
		//中继功能
		cout.setf(ios::left);
		cout << setiosflags(ios::left) << setw(6) << setfill(' ') << "relay";
		cout << "    ";
		//打印域名
		cout.setf(ios::left);
		cout << setiosflags(ios::left) << setw(20) << setfill(' ') << url;
		cout << "    ";
		//打印IP
		cout.setf(ios::left);
		cout << setiosflags(ios::left) << setw(20) << setfill(' ') << endl;
	}

	//在表中找到DNS请求中的域名
	else {
		if (DNS_cache[find].IP == "0.0.0.0")  //不良网站拦截
		{
			//屏蔽功能
			cout.setf(ios::left);
			cout << setiosflags(ios::left) << setw(6) << setfill(' ') << "illegal";
			cout << "    ";
			//打印域名(加*)
			cout.setf(ios::left);
			cout << "*" << setiosflags(ios::left) << setw(19) << setfill(' ') << url;
			cout << "    ";
			//打印IP
			cout.setf(ios::left);
			cout << setiosflags(ios::left) << setw(20) << setfill(' ') << endl;
		}

		//检索结果为普通IP地址，则向客户返回这个地址
		else {
			//服务器功能
			cout.setf(ios::left);
			cout << setiosflags(ios::left) << setw(6) << setfill(' ') << "server";
			cout << "    ";
			//打印域名
			cout.setf(ios::left);
			cout << "*" << setiosflags(ios::left) << setw(19) << setfill(' ') << url;
			cout << "    ";
			//打印IP
			cout.setf(ios::left);
			cout << setiosflags(ios::left) << setw(20) << setfill(' ') << DNS_cache[find].IP << endl;
		}
	}
}










int main(int argc, char** argv)
{
	dns_db DNS_db;
	WSADATA wsaData;
	SOCKET  socketServer, socketLocal;				 //本地DNS和外部DNS两个套接字描述符
	SOCKADDR_IN serverName, clientName, localName;	 //本地DNS、外部DNS和请求端三个网络套接字地址
	
	char sendbuf[BUF_SIZE];
	char recvbuf[BUF_SIZE];

	char tablePath[100];

	char outerDns[16];

	int iLen_cli, iSend, iRecv;
	
	int num;
	//-d 调试级别为1
	//-dd 调试级别为2

	if (argc == 1) {
		strcpy(outerDns, DEF_DNS_ADDRESS);
		strcpy(tablePath, "dnsrelay.txt");
	}

	else if (argc == 2) {
		strcpy(outerDns, argv[1]);//不是北邮的DNS服务器
		strcpy(tablePath, "dnsrelay.txt");
	}

	else if (argc == 3) {//既不是北邮的DNS服务器，也不是默认的配置文件
		strcpy(outerDns, argv[1]);
		strcpy(tablePath, argv[2]);
	}

	//init
	num = DNS_cache_init(tablePath);						//获取域名解析表

	for (int i = 0; i < AMOUNT; i++) {				//初始化ID转换表
		IDTransTable[i].oldID = 0;
		IDTransTable[i].done = FALSE;
		memset(&(IDTransTable[i].client), 0, sizeof(SOCKADDR_IN));
	}


	//保存系统的时间
	GetLocalTime(&sys);
	Day = sys.wDay;
	Hour = sys.wHour;
	Minute = sys.wMinute;
	Second = sys.wSecond;
	Milliseconds = sys.wMilliseconds;


	

	if ((WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0)
	{
		return;
	}
	//中继做服务器端
	//创建本地套接字文件描述符
	socketLocal = socket(AF_INET, SOCK_DGRAM, 0);
	localName.sin_family = AF_INET;
	localName.sin_port = htons(DNS_PORT);
	localName.sin_addr.s_addr = inet_addr(LOCAL_ADDRESS);
	//绑定本地DNS服务器地址
	cout << "Binding UDP Port 53";
	if (bind(socketLocal, (SOCKADDR*)&localName, sizeof(localName))) {
		cout << "Binding Port 53 failed." << endl;
		system("pause");
		exit(1);
	}
	else
		cout << " ...OK!"<<endl;
	
	//中继做客户端
	//创建于外部DNS服务连接的套接字描述符
	socketServer = socket(AF_INET, SOCK_DGRAM, 0);
	serverName.sin_family = AF_INET;
	serverName.sin_port = htons(DNS_PORT);
	serverName.sin_addr.s_addr = inet_addr(outerDns);

	//本地DNS中继服务器的具体操作
	while (1) 
	{
		iLen_cli = sizeof(clientName);
		memset(recvbuf, 0, BUF_SIZE);
		iRecv = recvfrom(socketLocal, recvbuf, sizeof(recvbuf), 0, (SOCKADDR*)&clientName, &iLen_cli);

		if (iRecv == SOCKET_ERROR) 
		{
			cout << "Recvfrom Failed: " << WSAGetLastError() << endl;
			continue;
		}
		else if (iRecv == 0) 
		{
			break;
		}
		else 
		{
			//获取报文的所有信息
			GetUrl(recvbuf, iRecv);				//获取域名


			int find = IsFind(url, num);		//在域名解析表中查找
			//首先在 域名解析表中进行查找，如果没找到，则进行向数据库请求
			//如果数据库中存在，而dns缓存中不存在，更新dns缓存，替换最前面的
			//将结果返回给用户，考虑多个ip地址的情况，在构造dns报文， 增加资源记录的数量


			//在DNS缓存中没有找到ip地址
			if (find==NOTFOUND)
			{
			   DNS_record * dns_records=DNS_db.select_(url);
			   //开启两个线程
			   //返回ip
			   if (dns_records)
			   {
				   //数据库中查询到的内容必须放入到DNS缓存中

			   }
			   //中继功能,返回的报文中所包含的ip地址必须存入DNS缓存和数据库中
			   else
			   {


			   }
			}





			
			//在域名解析表中没有找到，同是数据库中也不存在，则进行中继，请求北邮的DNS服务器，在ID解析表中进行记录
			//返回的结果存入数据库和dns缓存(随机选一个或几个进行替换)
			
		//	？？？？
			//中继功能通过创建一个线程完成，负责发和收，收完之后谁负责通知？？？？？，更新数据库，更新DNS缓存（必须有互斥锁）， 发送给客户端
		// ????	
			
			
			if (find == NOTFOUND) {

				//ID转换 长度2字节
				unsigned short *pID = (unsigned short *)malloc(sizeof(unsigned short));

				//解析的是系统分配的id
				memcpy(pID, recvbuf, sizeof(unsigned short));
				unsigned short nID = htons(RegisterNewID(ntohs(*pID), clientName, FALSE));

				//新的ID是下标，加快查找速度
				memcpy(recvbuf, &nID, sizeof(unsigned short));

				//打印 时间 newID 功能 域名 IP
				DisplayInfo(ntohs(nID), find);

				//把recvbuf转发至指定的外部DNS服务器

				iSend = sendto(socketServer, recvbuf, iRecv, 0, (SOCKADDR*)&serverName, sizeof(serverName));
				if (iSend == SOCKET_ERROR) {
					cout << "sendto Failed: " << WSAGetLastError() << endl;
					continue;
				}
				else if (iSend == 0)
					break;

				free(pID);	//释放动态分配的内存

				//阻塞
				//接收来自外部DNS服务器的响应报文
				
				iRecv = recvfrom(socketServer, recvbuf, sizeof(recvbuf), 0, (SOCKADDR*)&clientName, &iLen_cli);

				//对收到的报文进行解析出所有的内容


				//ID转换
				pID = (unsigned short *)malloc(sizeof(unsigned short));
				memcpy(pID, recvbuf, sizeof(unsigned short));
				int m = ntohs(*pID);
				unsigned short oID = htons(IDTransTable[m].oldID);
				memcpy(recvbuf, &oID, sizeof(unsigned short));
				IDTransTable[m].done = TRUE;

				//从ID转换表中获取发出DNS请求者的信息
				clientName = IDTransTable[m].client;
				//????
				//ID解析表处理完之后，必须清楚，一个线程隔一段时间清除ID解析表中做完的ID转换
				//????



				//把recvbuf转发至请求者处
				iSend = sendto(socketLocal, recvbuf, iRecv, 0, (SOCKADDR*)&clientName, sizeof(clientName));
				if (iSend == SOCKET_ERROR) {
					cout << "sendto Failed: " << WSAGetLastError() << endl;
					continue;
				}
				else if (iSend == 0)
					break;

				free(pID);	//释放动态分配的内存
			}

			//在域名解析表中找到,构造 返回的DNS包
			else 
			{
				//获取请求报文的ID
				unsigned short *pID = (unsigned short *)malloc(sizeof(unsigned short));
				memcpy(pID, recvbuf, sizeof(unsigned short));

				//转换ID
				unsigned short nID = RegisterNewID(ntohs(*pID), clientName, FALSE);

				//打印 时间 newID 功能 域名 IP
				DisplayInfo(nID, find);

				//构造响应报文返回
				memcpy(sendbuf, recvbuf, iRecv);						//拷贝请求报文
				unsigned short a = htons(0x8180);
				memcpy(&sendbuf[2], &a, sizeof(unsigned short));		//修改标志域

				//修改回答数域
				if (strcmp(DNS_cache[find].IP.c_str(), "0.0.0.0") == 0)
					a = htons(0x0000);	//屏蔽功能：回答数为0
				else
					a = htons(0x0001);	//服务器功能：回答数为1
				memcpy(&sendbuf[6], &a, sizeof(unsigned short));
				int curLen = 0;

				//构造DNS响应部分
				char answer[16];
				unsigned short Name = htons(0xc00c);
				memcpy(answer, &Name, sizeof(unsigned short));
				curLen += sizeof(unsigned short);

				unsigned short TypeA = htons(0x0001);
				memcpy(answer + curLen, &TypeA, sizeof(unsigned short));
				curLen += sizeof(unsigned short);

				unsigned short ClassA = htons(0x0001);
				memcpy(answer + curLen, &ClassA, sizeof(unsigned short));
				curLen += sizeof(unsigned short);

				unsigned long timeLive = htonl(0x7b);
				memcpy(answer + curLen, &timeLive, sizeof(unsigned long));
				curLen += sizeof(unsigned long);

				unsigned short IPLen = htons(0x0004);
				memcpy(answer + curLen, &IPLen, sizeof(unsigned short));
				curLen += sizeof(unsigned short);

				unsigned long IP = (unsigned long)inet_addr(DNS_cache[find].IP.c_str());
				memcpy(answer + curLen, &IP, sizeof(unsigned long));
				curLen += sizeof(unsigned long);
				curLen += iRecv;

				//请求报文和响应部分共同组成DNS响应报文存入sendbuf
				memcpy(sendbuf + iRecv, answer, curLen);

				//发送DNS响应报文
				iSend = sendto(socketLocal, sendbuf, curLen, 0, (SOCKADDR*)&clientName, sizeof(clientName));
				if (iSend == SOCKET_ERROR) {
					cout << "sendto Failed: " << WSAGetLastError() << endl;
					continue;
				}
				else if (iSend == 0)
					break;

				free(pID);		//释放动态分配的内存
			}
		}
	}

	closesocket(socketServer);	//关闭套接字
	closesocket(socketLocal);
	WSACleanup();				//释放ws2_32.dll动态链接库初始化时分配的资源

	return 0;
}
