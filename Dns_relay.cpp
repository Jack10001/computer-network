#include"Dns_relay.h"
#include"DnsDb.h"
#include"DnsMessage.h"

DnsDb dns_db;
int debugLevel = 0;
int IDcount = 0;					//ת�����е���Ŀ����

WSADATA wsaData;
SOCKET  socketServer, socketLocal;				 //����DNS���ⲿDNS�����׽���������
SOCKADDR_IN serverName, clientName, localName;	 //����DNS���ⲿDNS����������������׽��ֵ�ַ
char tablePath[100];
char outerDNS[16];

vector<DNS_record> DNS_cache;
vector<IDTransform> IDTransTable;

string url;
mutex Mutex;



//dns�����ʼ��
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
	infile.close();		//�ر��ļ�
	cout << "  ...OK" << endl;
	
}


//�������ж��Ƿ��ڱ����ҵ�DNS�����е��������ҵ������±�
int IsFind(int num)
{
	int find = NOTFOUND;
	string temp_url = url;
	string domain;
	for (int i = 0; i < num; i++) 
	{
		if (temp_url.compare(DNS_cache[i].domain) == 0) {	//�ҵ���һ�����±�
			find = i;
			break;
		}
	}
	//��� IP ��ַ�����,�����ҵ��ĵ�һ��find���±����������ж�,���뱣֤��ͬ  domain  �ļ�¼����һ��������ŵ�
	return find;
}

//������������IDת��Ϊ�µ�ID��������Ϣд��IDת������
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
	return (unsigned short)(IDcount - 1);	//�Ա����±���Ϊ�µ�ID
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
	
	//��ʼ���׽���
	if ((WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0)
	{
		return;
	}
	//�м�����������
	//���������׽����ļ�������
	socketLocal = socket(AF_INET, SOCK_DGRAM, 0);
	localName.sin_family = AF_INET;
	localName.sin_port = htons(DNS_PORT);
	localName.sin_addr.s_addr = inet_addr(LOCAL_ADDRESS);
	//�󶨱���DNS��������ַ
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

	//�м̷��������ͻ���
	//�������ⲿDNS�������ӵ��׽���������
	socketServer = socket(AF_INET, SOCK_DGRAM, 0);
	serverName.sin_family = AF_INET;
	serverName.sin_port = htons(DNS_PORT);
	serverName.sin_addr.s_addr = inet_addr(outerDNS);

}

void DisplayInfo(unsigned short newID, int find)
{
	//��ӡʱ��
	showtime();
	cout << "  ";

	//��ӡת�����µ�ID
	cout.setf(ios::left);
	cout << setiosflags(ios::left) << setw(4) << setfill(' ') << newID;
	cout << "    ";


	//�ڱ���û���ҵ�DNS�����е�����
	if (find == NOTFOUND)
	{
		//�м̹���
		cout.setf(ios::left);
		cout << setiosflags(ios::left) << setw(6) << setfill(' ') << "relay";
		cout << "    ";
		//��ӡ����
		cout.setf(ios::left);
		cout << setiosflags(ios::left) << setw(20) << setfill(' ') << url;
		cout << "    ";
		//��ӡIP
		cout.setf(ios::left);
		cout << setiosflags(ios::left) << setw(20) << setfill(' ') << endl;
	}

	//�ڱ����ҵ�DNS�����е�����
	else {
		if (DNS_cache[find].IP == "0.0.0.0")  //������վ����
		{
			//���ι���
			cout.setf(ios::left);
			cout << setiosflags(ios::left) << setw(6) << setfill(' ') << "illegal";
			cout << "    ";
			//��ӡ����(��*)
			cout.setf(ios::left);
			cout <<setiosflags(ios::left) << setw(19) << setfill(' ') << url;
			cout << "    ";
			//��ӡIP
			cout.setf(ios::left);
			cout << setiosflags(ios::left) << setw(20) << setfill(' ') << endl;
		}

		//�������Ϊ��ͨIP��ַ������ͻ����������ַ
		else {
			//����������
			cout.setf(ios::left);
			cout << setiosflags(ios::left) << setw(6) << setfill(' ') << "server";
			cout << "    ";
			//��ӡ����
			cout.setf(ios::left);
			cout << setiosflags(ios::left) << setw(19) << setfill(' ') << url;
			cout << "    ";
			//��ӡIP
			cout.setf(ios::left);
			cout << setiosflags(ios::left) << setw(20) << setfill(' ') << DNS_cache[find].IP << endl;
		}
	}
}

void update_DNS_cache_db(char *recvbuf, int recv_num)
{
	//���������ص�DNS�����еĶ��ip
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
	//����dns��������ݿ�
	vector<DNS_record> dns_records;
	for (auto it = ip_list.begin(); it != ip_list.end(); it++)
	{
		record.IP = (*it);
		dns_records.push_back(record);	
	}
	
	//ɾ����ǰ��ģ����뵽����(DNS_cache���еĻ��Ͳ������DNS_cache�����ݿ�)
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
	//�������ݿ�
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
	//ɾ����ǰ��ģ����뵽����(DNS_cache���еĻ��Ͳ������DNS_cache�����ݿ�)
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


//�м̹���
void to_BUPT_DNSServer(char * recvbuf_temp, int iRecv, int find)
{
	int iLen_cli = sizeof(localName);
	int iSend = 0, recv_num;
	char recvBUF[BUF_SIZE];

	memcpy(recvBUF, recvbuf_temp, iRecv);

	//IDת�� ����2�ֽ�
	unsigned short *pID = (unsigned short *)malloc(sizeof(unsigned short));
	//��������ϵͳ�����ID
	memcpy(pID, recvBUF, sizeof(unsigned short));
	unsigned short nID = htons(RegisterNewID(ntohs(*pID), clientName, FALSE));
	//�µ�ID���±꣬�ӿ�����ٶ�
	memcpy(recvBUF, &nID, sizeof(unsigned short));

	free(pID);	//�ͷŶ�̬������ڴ�

	//Mutex.lock();
	cout << "nID :" << ntohs(nID) << endl;
	//��ӡ ʱ�� newID ���� ���� IP
	//DisplayInfo(ntohs(nID), find);
	//Mutex.unlock();


	//��IDת��֮�󣬽�ԭ�ȵ�����ԭ�ⲻ����ת����ȥ
	//��recvbufת����ָ�����ⲿDNS������
	iSend = sendto(socketServer, recvBUF, iRecv, 0, (SOCKADDR*)&serverName, sizeof(serverName));
	if (iSend == SOCKET_ERROR) {
		cout << "sendto Failed: " << WSAGetLastError() << endl;
	}
	else if (iSend == 0)
		return;

	//���������ⲿDNS����������Ӧ����
	recv_num = recvfrom(socketServer, recvBUF, BUF_SIZE, 0, (SOCKADDR*)&localName, &iLen_cli);

	//����DNS��������ݿ���
	//Mutex.lock();
	cout << "recv_num : "<<recv_num << endl;
		
	
	   //�������ݿ��DNS����
    	update_DNS_cache_db(recvBUF,recv_num);

		unsigned short *ppID = (unsigned short *)malloc(sizeof(unsigned short));

		//���� ID ת��
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
		//ID����������֮�󣬱���һ���̸߳�һ��ʱ�����ID�������������IDת��
		//????



		//�м̷������յ�����DNS������������ ��Ӧ���� ��ֱ�� ���� ����
		iSend = sendto(socketLocal, recvBUF, iRecv, 0, (SOCKADDR*)&clientName, sizeof(clientName));
		if (iSend == SOCKET_ERROR) {
			cout << "sendto Failed: " << WSAGetLastError() << endl;
		}
		else if (iSend == 0)
			return;
		free(ppID);	//�ͷŶ�̬������ڴ�
	
}

//����������
void relay_server(char * recvbuf,int iRecv,int find)
{
	char sendbuf[BUF_SIZE];
	int iSend = 0;

	//��ȡ�����ĵ�ID
	//unsigned short *pID = (unsigned short *)malloc(sizeof(unsigned short));
	//memcpy(pID, recvbuf, sizeof(unsigned short));
	//ת��ID
	//unsigned short nID = RegisterNewID(ntohs(*pID), clientName, FALSE);



	//��ӡ:ʱ�� newID ���� ���� IP
	Mutex.lock();
	//DisplayInfo(*pID, find);
	Mutex.unlock();
	//������Ӧ���� 12bytes ͷ��
	//FLAGS 1000 0001 1000 0000
	
	memcpy(sendbuf, recvbuf, iRecv);//�����ĺ���Ӧ��������ͬ�ĸ�ʽ

	unsigned short a = htons(0x8180);
	memcpy(&sendbuf[2], &a, sizeof(unsigned short));		//FLAGS


	//�޸�ANCOUNT �ҵ�IP��ַ ���������ip��ַ ???
	//����domain�ж�� ip ��ַ
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
		a = htons(0x0000);	//���ι��ܣ��ش���Ϊ0
	else
		a = htons(0x0001);	//���������ܣ��ش���Ϊ1
	//2ID|2FLAGS|2QDCOUNT|2ANCOUNT
	memcpy(&sendbuf[6], &a, sizeof(unsigned short));

	//��Ӧ���ĵ� ��Դ��¼�Ĺ���
	int curLen = 0;
	char answer[16];

	//NAME
	unsigned short Name = htons(0xc00c);//
	memcpy(answer, &Name, sizeof(unsigned short));
	curLen += sizeof(unsigned short);//2 bytes ��domin
	//TYPE
	unsigned short TypeA = htons(0x0001);//0000 0000 0000 0001
	memcpy(answer + curLen, &TypeA, sizeof(unsigned short));
	curLen += sizeof(unsigned short);// 2 bytes TYPE: ����IP��ַ�Ķ���A����
	//CLASS 
	unsigned short ClassA = htons(0x0001);//IN ��
	memcpy(answer + curLen, &ClassA, sizeof(unsigned short));
	curLen += sizeof(unsigned short);
	//TTL
	unsigned long timeLive = htonl(0x007b);//0000 0000 0111 1011
	memcpy(answer + curLen, &timeLive, sizeof(unsigned long));
	curLen += sizeof(unsigned long);
	//RDLENGTH
	unsigned short IPLen = htons(0x0004);//4bytes �� IP ��ַ
	memcpy(answer + curLen, &IPLen, sizeof(unsigned short));
	curLen += sizeof(unsigned short);
	//RDATA  ??? ���IP��ַ ???
	//�ҵ���� IP����ַ�����ҡ����ء�DNS_cache.count(find)
	//���ҵ��� ip+domin ��浽 ��һ�� ���ݽṹ multimap
	unsigned long IP = (unsigned long)inet_addr(DNS_cache[find].IP.c_str());
	memcpy(answer + curLen, &IP, sizeof(unsigned long));
	curLen += sizeof(unsigned long);

	curLen += iRecv;

	//�����ĺ���Ӧ���ֹ�ͬ���DNS��Ӧ���Ĵ���sendbuf
	memcpy(sendbuf + iRecv, answer, curLen);


	//����DNS��Ӧ����
	iSend = sendto(socketLocal, sendbuf, curLen, 0, (SOCKADDR*)&clientName, sizeof(clientName));
	if (iSend == SOCKET_ERROR) {
		cout << "sendto Failed: " << WSAGetLastError() << endl;
	}
	else if (iSend == 0)
		return;

	
}


int main(int argc, char** argv)
{
	//��ʼ�����ݿ�
	dns_db.init_db();
	
	char recvbuf[BUF_SIZE];
	int Len_cli, iRecv = 0;
	//��ʼ��������
	init_CMD(argc,argv);
	//��ʼ���׽���
	init_socket();
	DNS_cache_init(tablePath);
	cout << DNS_cache.size() << " names,occpy " << sizeof(DNS_record) * DNS_cache.size() << " bytes memory!\n\n\n" ;

	//��������ݿ��в�ѯ���ļ�¼
	vector<DNS_record> dns_records;
	DNS_record a_record;


	//����DNS�м̷������ľ������
	while (1)
	{
		Len_cli = sizeof(clientName);
		memset(recvbuf, 0, BUF_SIZE);

		//recvfrom ��������
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
		/*�����Կͻ��˵� ������ ���н���*/
		else
		{
			//��ȡ���ĵ�������Ϣ
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
			//��dns������в���
			int find = IsFind(DNS_cache.size());

			//Mutex.lock();
			cout << "find in DNS_cache : " << find << endl;
		//	Mutex.unlock();
			//������ �����������н��в��ң����û�ҵ�������������ݿ�����
			//������ݿ��д��ڣ���dns�����в����ڣ�����dns���棬�滻��ǰ���
			//��������ظ��û������Ƕ��ip��ַ��������ڹ���dns���ģ�
			//��DNS������û���ҵ�ip��ַ
			//Mutex.unlock();
			if (find == NOTFOUND)
			{
				const char * url_ = url.c_str();

				//�����ݿ��н��в���
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
					//1.���ݿ��в�ѯ�������ݱ�����뵽DNS������
					//Mutex.lock();
					//update_dns_cache(dns_records);
					//Mutex.unlock();

					//2.����DNS���ķ��ظ������û�

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
				//�����ݿ���û���ҵ���������м�
				//�м̹���,���صı�������������ip��ַ�������DNS��������ݿ���
				else
				{
					//1.������DNS������
					//2.��������
					//3.�����������а�����ip+domin
					//4.����������ip+domin ���뵽���ݿ���
					//5.����DNS���ķ��ظ��û�
					char recvbuf_temp[BUF_SIZE];
					memcpy(recvbuf_temp, recvbuf, BUF_SIZE);
					//thread t1(to_BUPT_DNSServer, recvbuf_temp, iRecv, find);
					//t1.detach();
					to_BUPT_DNSServer(recvbuf_temp, iRecv, find);
				}
			}
			//��dns�������ҵ�
			else
			{
				char recvbuf_temp[BUF_SIZE];
				memcpy(recvbuf_temp, recvbuf, BUF_SIZE);
				//thread t2(relay_server, recvbuf_temp, iRecv, find);
				//t2.detach();
			}
		}

	}
	closesocket(socketServer);	//�ر��׽���
	closesocket(socketLocal);
	WSACleanup();		
	return 0;
}


