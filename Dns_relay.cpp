#include"Dns_relay.h"
#include"DnsDb.h"
#include"DnsMessage.h"

DnsDb dns_db;
int debugLevel = 0;
int IDcount = 0;					//转换表中的条目个数

WSADATA wsaData;
SOCKET  socketServer, socketLocal;				 //本地DNS和外部DNS两个套接字描述符
SOCKADDR_IN serverName, clientName, localName;	 //本地DNS、外部DNS和请求端三个网络套接字地址
char tablePath[100];
char outerDNS[16];

vector<DNS_record> DNS_cache;
vector<IDTransform> IDTransTable;

string url;
mutex Mutex;



//dns缓存初始化
void DNS_cache_init(const char *tablePath)
{
	cout << "Try to load table 'dnsrelay.txt'";
	int i = 0;
	unsigned int pos = 0;
	string table[AMOUNT];
	DNS_record dns_record;
	ifstream infile(tablePath,ios::in);
	if (!infile)
	{
		cerr << "...Open " << tablePath << " error!" << endl;
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
			dns_record.IP = table[j].substr(0, pos);
			dns_record.domain = table[j].substr(pos + 1);

			DNS_cache.push_back(dns_record);
		}
	}
	infile.close();		//关闭文件
	cout << "  ...OK" << endl;
	
}


//函数：判断是否在表中找到DNS请求中的域名，找到返回下标
int IsFind(int num)
{
	int find = NOTFOUND;
	string temp_url = url;
	string domain;
	for (int i = 0; i < num; i++) 
	{
		if (temp_url.compare(DNS_cache[i].domain) == 0) {	//找到第一个的下标
			find = i;
			break;
		}
	}
	//多个 IP 地址的情况,根据找到的第一个find的下标依次往后判断,必须保证相同  domain  的记录是在一起连续存放的
	return find;
}

//函数：将请求ID转换为新的ID，并将信息写入ID转换表中
unsigned short RegisterNewID(unsigned short oID, SOCKADDR_IN temp, BOOL ifdone)
{
	//srand(time(NULL));
	//Mutex.lock();
	IDTransform transform;
	transform.oldID = oID;
	transform.client = temp;
	transform.done = ifdone;
	IDTransTable.push_back(transform);
	IDcount++;
	cout << "IDcount : " << IDcount - 1 << endl;
	//Mutex.unlock();
	return (unsigned short)(IDcount - 1);	//以表中下标作为新的ID
}

void showtime() 
{
	char tmp[64];
	time_t t = time(NULL);
	tm *_tm = localtime(&t);
	int year = _tm->tm_year + 1900;
	int month = _tm->tm_mon + 1;
	int date = _tm->tm_mday;
	int hh = _tm->tm_hour;
	int mm = _tm->tm_min;
	int ss = _tm->tm_sec;
	sprintf(tmp, "%02d:%02d:%02d", hh, mm, ss);
	//return string(tmp);
	cout << tmp << '\t';
}

void init_CMD(int argc, char ** argv)
{
	if (argc == 1)
	{
		debugLevel = 0;
		strcpy(outerDNS, DEF_DNS_ADDRESS);
		strcpy(tablePath, "dnsrelay.txt");
	}
	else if (argc == 3)
	{
		string d = argv[1];
		if (d == "-d")
			debugLevel = 1;
		else
			debugLevel = 2;
		strcpy(outerDNS, argv[2]);
		strcpy(tablePath, "dnsrelay.txt");
	}
	else if (argc == 4)
	{
		string d = argv[1];
		if (d == "-d")
			debugLevel = 1;
		else
			debugLevel = 2;
		strcpy(outerDNS, argv[2]);
		strcpy(tablePath, argv[3]);
	}

}

void init_socket()
{
	
	//初始化套接字
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
	try
	{
		::bind(socketLocal, (SOCKADDR*)&localName, sizeof(localName));
	}
	catch (const exception& e)
	{
		cout << "Binding Port 53 failed." << endl;
		cout << "REASON :" << e.what() << endl;
		system("pause");
		exit(1);
	}
	cout << " ...OK!" << endl;

	//中继服务器做客户端
	//创建于外部DNS服务连接的套接字描述符
	socketServer = socket(AF_INET, SOCK_DGRAM, 0);
	serverName.sin_family = AF_INET;
	serverName.sin_port = htons(DNS_PORT);
	serverName.sin_addr.s_addr = inet_addr(outerDNS);

}

void DisplayInfo(unsigned short newID, int find)
{
	//打印时间
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
			cout <<setiosflags(ios::left) << setw(19) << setfill(' ') << url;
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
			cout << setiosflags(ios::left) << setw(19) << setfill(' ') << url;
			cout << "    ";
			//打印IP
			cout.setf(ios::left);
			cout << setiosflags(ios::left) << setw(20) << setfill(' ') << DNS_cache[find].IP << endl;
		}
	}
}

void update_DNS_cache_db(char *recvbuf, int recv_num)
{
	//解析出返回的DNS报文中的多个ip
	DnsMessage dns_msg(recvbuf,recv_num);
	if (!dns_msg.StartAnalysis())
	{
		cout << "analysi error" << endl;
		return;
	}
	string domain = dns_msg.QuestionDomain();
	vector<string> ip_list = dns_msg.Get_Ip_List();
	DNS_record record;
	record.domain = domain;
	//更新dns缓存和数据库
	vector<DNS_record> dns_records;
	for (auto it = ip_list.begin(); it != ip_list.end(); it++)
	{
		record.IP = (*it);
		dns_records.push_back(record);	
	}
	
	//删除最前面的，插入到后面(DNS_cache中有的话就不会更新DNS_cache和数据库)
	DNS_cache.insert(DNS_cache.end(), dns_records.begin(), dns_records.end());
	string ip = DNS_cache[0].IP;
	vector<DNS_record>::iterator temp;
	for (auto it = DNS_cache.begin(); it != DNS_cache.end(); it++)
	{
		if (ip.compare((*it).IP) != 0)
		{
			temp = it;
			break;
		}
	}
	DNS_cache.erase(DNS_cache.begin(), temp);
	//更新数据库
	const char* pdomain = domain.c_str();
	for (auto it = ip_list.begin(); it != ip_list.end(); it++)
	{
		const char * ip = (*it).c_str();
       //cout << "ip" << *ip << ",domain" << *pdomain << endl;
		dns_db.update_record(ip,pdomain);

	}
}


void update_dns_cache(vector<DNS_record>& records)
{
	DNS_cache.insert(DNS_cache.end(), records.begin(), records.end());
	//删除最前面的，插入到后面(DNS_cache中有的话就不会更新DNS_cache和数据库)
	string ip = DNS_cache[0].IP;
	vector<DNS_record>::iterator temp;
	for (auto it = DNS_cache.begin(); it != DNS_cache.end(); it++)
	{
		if (ip.compare((*it).IP)!=0)
		{
			temp = it;
			break;
		}
	}
	DNS_cache.erase(DNS_cache.begin(),temp);

}


//中继功能
void to_BUPT_DNSServer(char * recvbuf_temp, int iRecv, int find)
{
	int iLen_cli = sizeof(localName);
	int iSend = 0, recv_num;
	char recvBUF[BUF_SIZE];

	memcpy(recvBUF, recvbuf_temp, iRecv);

	//ID转换 长度2字节
	unsigned short *pID = (unsigned short *)malloc(sizeof(unsigned short));
	//解析的是系统分配的ID
	memcpy(pID, recvBUF, sizeof(unsigned short));
	unsigned short nID = htons(RegisterNewID(ntohs(*pID), clientName, FALSE));
	//新的ID是下标，加快查找速度
	memcpy(recvBUF, &nID, sizeof(unsigned short));

	free(pID);	//释放动态分配的内存

	//Mutex.lock();
	cout << "nID :" << ntohs(nID) << endl;
	//打印 时间 newID 功能 域名 IP
	//DisplayInfo(ntohs(nID), find);
	//Mutex.unlock();


	//做ID转换之后，将原先的内容原封不动的转发出去
	//把recvbuf转发至指定的外部DNS服务器
	iSend = sendto(socketServer, recvBUF, iRecv, 0, (SOCKADDR*)&serverName, sizeof(serverName));
	if (iSend == SOCKET_ERROR) {
		cout << "sendto Failed: " << WSAGetLastError() << endl;
	}
	else if (iSend == 0)
		return;

	//接收来自外部DNS服务器的响应报文
	recv_num = recvfrom(socketServer, recvBUF, BUF_SIZE, 0, (SOCKADDR*)&localName, &iLen_cli);

	//更新DNS缓存和数据库中
	//Mutex.lock();
	cout << "recv_num : "<<recv_num << endl;
		
	
	   //更新数据库和DNS缓存
    	update_DNS_cache_db(recvBUF,recv_num);

		unsigned short *ppID = (unsigned short *)malloc(sizeof(unsigned short));

		//进行 ID 转换
		ppID = (unsigned short *)malloc(sizeof(unsigned short));
		memcpy(ppID, recvBUF, sizeof(unsigned short));
		int m = ntohs(*ppID);
		unsigned short oID = htons(IDTransTable[m].oldID);
		memcpy(recvBUF, &oID, sizeof(unsigned short));

		//Mutex.lock();
		cout << "ppID :" << m << endl << endl;
		IDTransTable[m].done = TRUE;
		clientName = IDTransTable[m].client;
		//Mutex.unlock();
		//????
		//ID解析表处理完之后，必须一个线程隔一段时间清除ID解析表中做完的ID转换
		//????



		//中继服务器收到北邮DNS服务器发来的 相应报文 ，直接 发给 请求方
		iSend = sendto(socketLocal, recvBUF, iRecv, 0, (SOCKADDR*)&clientName, sizeof(clientName));
		if (iSend == SOCKET_ERROR) {
			cout << "sendto Failed: " << WSAGetLastError() << endl;
		}
		else if (iSend == 0)
			return;
		free(ppID);	//释放动态分配的内存
	
}

//服务器功能
void relay_server(char * recvbuf,int iRecv,int find)
{
	char sendbuf[BUF_SIZE];
	int iSend = 0;

	//获取请求报文的ID
	//unsigned short *pID = (unsigned short *)malloc(sizeof(unsigned short));
	//memcpy(pID, recvbuf, sizeof(unsigned short));
	//转换ID
	//unsigned short nID = RegisterNewID(ntohs(*pID), clientName, FALSE);



	//打印:时间 newID 功能 域名 IP
	Mutex.lock();
	//DisplayInfo(*pID, find);
	Mutex.unlock();
	//构造响应报文 12bytes 头部
	//FLAGS 1000 0001 1000 0000
	
	memcpy(sendbuf, recvbuf, iRecv);//请求报文和响应报文有相同的格式

	unsigned short a = htons(0x8180);
	memcpy(&sendbuf[2], &a, sizeof(unsigned short));		//FLAGS


	//修改ANCOUNT 找到IP地址 ？？？多个ip地址 ???
	//根据domain有多个 ip 地址
	vector<DNS_record> results;
	DNS_record dns_record;
	string result = DNS_cache[find].domain;
	dns_record.domain = result;
	for (unsigned int i = 0; i < DNS_cache.size(); ++i)
	{
		if (DNS_cache[i].domain == result)
		{
			dns_record.IP= DNS_cache[i].IP;
			results.push_back(dns_record);

		}
		else break;
		
	}
	int result_num = results.size();
	if (strcmp(DNS_cache[find].IP.c_str(), "0.0.0.0") == 0)
		a = htons(0x0000);	//屏蔽功能：回答数为0
	else
		a = htons(0x0001);	//服务器功能：回答数为1
	//2ID|2FLAGS|2QDCOUNT|2ANCOUNT
	memcpy(&sendbuf[6], &a, sizeof(unsigned short));

	//响应报文的 资源记录的构造
	int curLen = 0;
	char answer[16];

	//NAME
	unsigned short Name = htons(0xc00c);//
	memcpy(answer, &Name, sizeof(unsigned short));
	curLen += sizeof(unsigned short);//2 bytes 的domin
	//TYPE
	unsigned short TypeA = htons(0x0001);//0000 0000 0000 0001
	memcpy(answer + curLen, &TypeA, sizeof(unsigned short));
	curLen += sizeof(unsigned short);// 2 bytes TYPE: 请求IP地址的都是A类型
	//CLASS 
	unsigned short ClassA = htons(0x0001);//IN 类
	memcpy(answer + curLen, &ClassA, sizeof(unsigned short));
	curLen += sizeof(unsigned short);
	//TTL
	unsigned long timeLive = htonl(0x007b);//0000 0000 0111 1011
	memcpy(answer + curLen, &timeLive, sizeof(unsigned long));
	curLen += sizeof(unsigned long);
	//RDLENGTH
	unsigned short IPLen = htons(0x0004);//4bytes 的 IP 地址
	memcpy(answer + curLen, &IPLen, sizeof(unsigned short));
	curLen += sizeof(unsigned short);
	//RDATA  ??? 多个IP地址 ???
	//找到多个 IP　地址　并且　返回　DNS_cache.count(find)
	//把找到的 ip+domin 另存到 另一个 数据结构 multimap
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
	}
	else if (iSend == 0)
		return;

	
}


int main(int argc, char** argv)
{
	//初始化数据库
	dns_db.init_db();
	
	char recvbuf[BUF_SIZE];
	int Len_cli, iRecv = 0;
	//初始化命令行
	init_CMD(argc,argv);
	//初始化套接字
	init_socket();
	DNS_cache_init(tablePath);
	cout << DNS_cache.size() << " names,occpy " << sizeof(DNS_record) * DNS_cache.size() << " bytes memory!\n\n\n" ;

	//保存从数据库中查询到的记录
	vector<DNS_record> dns_records;
	DNS_record a_record;


	//本地DNS中继服务器的具体操作
	while (1)
	{
		Len_cli = sizeof(clientName);
		memset(recvbuf, 0, BUF_SIZE);

		//recvfrom 阻塞调用
		iRecv = recvfrom(socketLocal, recvbuf, sizeof(recvbuf), 0, (SOCKADDR*)&clientName,&Len_cli);
		if (iRecv == SOCKET_ERROR)
		{
			cout << "Recvfrom Failed:  " << WSAGetLastError() << endl;
			continue;
		}
		else if (iRecv == 0)
		{
			break;
		}
		/*对来自客户端的 请求报文 进行解析*/
		else
		{
			//获取报文的所有信息
			//Mutex.lock();
			DnsMessage dns_msg(recvbuf, iRecv);
			if (!dns_msg.StartAnalysis())
			{
				cout << "dns_mag analysis error!\n";
				system("pause");
				exit(1);
			}
			//url=(char*)dns_msg.QuestionDomain().c_str();
			url = dns_msg.QuestionDomain();
			//Mutex.lock();
			cout << "question url:" << url << endl;
			//Mutex.unlock();
			//在dns缓存表中查找
			int find = IsFind(DNS_cache.size());

			//Mutex.lock();
			cout << "find in DNS_cache : " << find << endl;
		//	Mutex.unlock();
			//首先在 域名解析表中进行查找，如果没找到，则进行向数据库请求
			//如果数据库中存在，而dns缓存中不存在，更新dns缓存，替换最前面的
			//将结果返回给用户，考虑多个ip地址的情况，在构造dns报文，
			//在DNS缓存中没有找到ip地址
			//Mutex.unlock();
			if (find == NOTFOUND)
			{
				const char * url_ = url.c_str();

				//在数据库中进行查找
			//	Mutex.lock();
				vector<DNS_record_> dns_records_ = dns_db.select_(url_);
				

				if (dns_records_.size() != 0)
				{
					for (auto it = dns_records_.begin(); it != dns_records_.end(); it++)
					{
						a_record.domain = (*it).domain;
						a_record.IP = (*it).IP;
						dns_records.push_back(a_record);
					}
					//1.数据库中查询到的内容必须放入到DNS缓存中
					//Mutex.lock();
					//update_dns_cache(dns_records);
					//Mutex.unlock();

					//2.构造DNS报文返回给请求用户

					find = IsFind(DNS_cache.size());
					//Mutex.lock();
					cout << "find in database :" << find << endl;
					//Mutex.unlock();

				//	Mutex.unlock();
					//dns_relay.relay_server(recvbuf,iRecv,find);
					char recvbuf_temp[BUF_SIZE];
					memcpy(recvbuf_temp, recvbuf, BUF_SIZE);
					//thread t(relay_server, recvbuf_temp, iRecv, find);
					//t.detach();
				}
				//在数据库中没有找到，则进行中继
				//中继功能,返回的报文中所包含的ip地址必须存入DNS缓存和数据库中
				else
				{
					//1.请求北邮DNS服务器
					//2.阻塞返回
					//3.解析处返回中包含的ip+domin
					//4.将解析出的ip+domin 存入到数据库中
					//5.构造DNS报文返回给用户
					char recvbuf_temp[BUF_SIZE];
					memcpy(recvbuf_temp, recvbuf, BUF_SIZE);
					//thread t1(to_BUPT_DNSServer, recvbuf_temp, iRecv, find);
					//t1.detach();
					to_BUPT_DNSServer(recvbuf_temp, iRecv, find);
				}
			}
			//在dns缓存中找到
			else
			{
				char recvbuf_temp[BUF_SIZE];
				memcpy(recvbuf_temp, recvbuf, BUF_SIZE);
				//thread t2(relay_server, recvbuf_temp, iRecv, find);
				//t2.detach();
			}
		}

	}
	closesocket(socketServer);	//关闭套接字
	closesocket(socketLocal);
	WSACleanup();		
	return 0;
}


