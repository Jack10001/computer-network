#include"dnsrelay.h"
#include"dns_db.h"

//dns�����ʼ��
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
	infile.close();		//�ر��ļ�
	cout << "  ...OK" << endl;
	return i - 1;			//������������������Ŀ����
}



//��������ȡDNS�����е�����
//��һ��


void GetUrl(char *recvbuf, int recvnum)
{
	char urlname[LENGTH];
	int i = 0, j, k = 0;

	memset(url, 0, LENGTH);
	memcpy(urlname, &(recvbuf[sizeof(DNS_HDR)]), recvnum - 16);	//��ȡ�������е�������ʾ

	int len = strlen(urlname);

	//����ת��
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


//�������ж��Ƿ��ڱ����ҵ�DNS�����е��������ҵ������±�
int IsFind(char* url, int num)
{
	int find = NOTFOUND;
	char* domain;

	for (int i = 0; i < num; i++) {
		domain = (char *)DNS_cache[i].domain.c_str();
		if (strcmp(domain, url) == 0) {	//�ҵ�
			find = i;
			break;
		}
	}

	return find;
}





//������������IDת��Ϊ�µ�ID��������Ϣд��IDת������
unsigned short RegisterNewID(unsigned short oID, SOCKADDR_IN temp, BOOL ifdone)
{
	srand(time(NULL));
	IDTransTable[IDcount].oldID = oID;
	IDTransTable[IDcount].client = temp;
	IDTransTable[IDcount].done = ifdone;
	IDcount++;

	return (unsigned short)(IDcount - 1);	//�Ա����±���Ϊ�µ�ID
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
	//��ӡʱ��
	//GetLocalTime(&sys);
	/*if (sys.wMilliseconds >= Milliseconds)
	{
		cout << setiosflags(ios::right) << setw(7) << setfill(' ') << (((sys.wDay - Day) * 24 + sys.wHour - Hour) * 60 + sys.wMinute - Minute) * 60 + sys.wSecond - Second;//���ÿ��Ϊ7��right���뷽ʽ
		cout << '.' << setiosflags(ios::right) << setw(3) << setfill('0') << sys.wMilliseconds - Milliseconds;
	}
	else {
		cout << setiosflags(ios::right) << setw(7) << setfill(' ') << (((sys.wDay - Day) * 24 + sys.wHour - Hour) * 60 + sys.wMinute - Minute) * 60 + sys.wSecond - Second - 1;//���ÿ��Ϊ7��right���뷽ʽ
		cout << '.' << setiosflags(ios::right) << setw(3) << setfill('0') << 1000 + sys.wMilliseconds - Milliseconds;
	}*/
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
			cout << "*" << setiosflags(ios::left) << setw(19) << setfill(' ') << url;
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
			cout << "*" << setiosflags(ios::left) << setw(19) << setfill(' ') << url;
			cout << "    ";
			//��ӡIP
			cout.setf(ios::left);
			cout << setiosflags(ios::left) << setw(20) << setfill(' ') << DNS_cache[find].IP << endl;
		}
	}
}










int main(int argc, char** argv)
{
	dns_db DNS_db;
	WSADATA wsaData;
	SOCKET  socketServer, socketLocal;				 //����DNS���ⲿDNS�����׽���������
	SOCKADDR_IN serverName, clientName, localName;	 //����DNS���ⲿDNS����������������׽��ֵ�ַ
	
	char sendbuf[BUF_SIZE];
	char recvbuf[BUF_SIZE];

	char tablePath[100];

	char outerDns[16];

	int iLen_cli, iSend, iRecv;
	
	int num;
	//-d ���Լ���Ϊ1
	//-dd ���Լ���Ϊ2

	if (argc == 1) {
		strcpy(outerDns, DEF_DNS_ADDRESS);
		strcpy(tablePath, "dnsrelay.txt");
	}

	else if (argc == 2) {
		strcpy(outerDns, argv[1]);//���Ǳ��ʵ�DNS������
		strcpy(tablePath, "dnsrelay.txt");
	}

	else if (argc == 3) {//�Ȳ��Ǳ��ʵ�DNS��������Ҳ����Ĭ�ϵ������ļ�
		strcpy(outerDns, argv[1]);
		strcpy(tablePath, argv[2]);
	}

	//init
	num = DNS_cache_init(tablePath);						//��ȡ����������

	for (int i = 0; i < AMOUNT; i++) {				//��ʼ��IDת����
		IDTransTable[i].oldID = 0;
		IDTransTable[i].done = FALSE;
		memset(&(IDTransTable[i].client), 0, sizeof(SOCKADDR_IN));
	}


	//����ϵͳ��ʱ��
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
	//�м�����������
	//���������׽����ļ�������
	socketLocal = socket(AF_INET, SOCK_DGRAM, 0);
	localName.sin_family = AF_INET;
	localName.sin_port = htons(DNS_PORT);
	localName.sin_addr.s_addr = inet_addr(LOCAL_ADDRESS);
	//�󶨱���DNS��������ַ
	cout << "Binding UDP Port 53";
	if (bind(socketLocal, (SOCKADDR*)&localName, sizeof(localName))) {
		cout << "Binding Port 53 failed." << endl;
		system("pause");
		exit(1);
	}
	else
		cout << " ...OK!"<<endl;
	
	//�м����ͻ���
	//�������ⲿDNS�������ӵ��׽���������
	socketServer = socket(AF_INET, SOCK_DGRAM, 0);
	serverName.sin_family = AF_INET;
	serverName.sin_port = htons(DNS_PORT);
	serverName.sin_addr.s_addr = inet_addr(outerDns);

	//����DNS�м̷������ľ������
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
			//��ȡ���ĵ�������Ϣ
			GetUrl(recvbuf, iRecv);				//��ȡ����


			int find = IsFind(url, num);		//�������������в���
			//������ �����������н��в��ң����û�ҵ�������������ݿ�����
			//������ݿ��д��ڣ���dns�����в����ڣ�����dns���棬�滻��ǰ���
			//��������ظ��û������Ƕ��ip��ַ��������ڹ���dns���ģ� ������Դ��¼������


			//��DNS������û���ҵ�ip��ַ
			if (find==NOTFOUND)
			{
			   DNS_record * dns_records=DNS_db.select_(url);
			   //���������߳�
			   //����ip
			   if (dns_records)
			   {
				   //���ݿ��в�ѯ�������ݱ�����뵽DNS������

			   }
			   //�м̹���,���صı�������������ip��ַ�������DNS��������ݿ���
			   else
			   {


			   }
			}





			
			//��������������û���ҵ���ͬ�����ݿ���Ҳ�����ڣ�������м̣������ʵ�DNS����������ID�������н��м�¼
			//���صĽ���������ݿ��dns����(���ѡһ���򼸸������滻)
			
		//	��������
			//�м̹���ͨ������һ���߳���ɣ����𷢺��գ�����֮��˭����֪ͨ�������������������ݿ⣬����DNS���棨�����л��������� ���͸��ͻ���
		// ????	
			
			
			if (find == NOTFOUND) {

				//IDת�� ����2�ֽ�
				unsigned short *pID = (unsigned short *)malloc(sizeof(unsigned short));

				//��������ϵͳ�����id
				memcpy(pID, recvbuf, sizeof(unsigned short));
				unsigned short nID = htons(RegisterNewID(ntohs(*pID), clientName, FALSE));

				//�µ�ID���±꣬�ӿ�����ٶ�
				memcpy(recvbuf, &nID, sizeof(unsigned short));

				//��ӡ ʱ�� newID ���� ���� IP
				DisplayInfo(ntohs(nID), find);

				//��recvbufת����ָ�����ⲿDNS������

				iSend = sendto(socketServer, recvbuf, iRecv, 0, (SOCKADDR*)&serverName, sizeof(serverName));
				if (iSend == SOCKET_ERROR) {
					cout << "sendto Failed: " << WSAGetLastError() << endl;
					continue;
				}
				else if (iSend == 0)
					break;

				free(pID);	//�ͷŶ�̬������ڴ�

				//����
				//���������ⲿDNS����������Ӧ����
				
				iRecv = recvfrom(socketServer, recvbuf, sizeof(recvbuf), 0, (SOCKADDR*)&clientName, &iLen_cli);

				//���յ��ı��Ľ��н��������е�����


				//IDת��
				pID = (unsigned short *)malloc(sizeof(unsigned short));
				memcpy(pID, recvbuf, sizeof(unsigned short));
				int m = ntohs(*pID);
				unsigned short oID = htons(IDTransTable[m].oldID);
				memcpy(recvbuf, &oID, sizeof(unsigned short));
				IDTransTable[m].done = TRUE;

				//��IDת�����л�ȡ����DNS�����ߵ���Ϣ
				clientName = IDTransTable[m].client;
				//????
				//ID����������֮�󣬱��������һ���̸߳�һ��ʱ�����ID�������������IDת��
				//????



				//��recvbufת���������ߴ�
				iSend = sendto(socketLocal, recvbuf, iRecv, 0, (SOCKADDR*)&clientName, sizeof(clientName));
				if (iSend == SOCKET_ERROR) {
					cout << "sendto Failed: " << WSAGetLastError() << endl;
					continue;
				}
				else if (iSend == 0)
					break;

				free(pID);	//�ͷŶ�̬������ڴ�
			}

			//���������������ҵ�,���� ���ص�DNS��
			else 
			{
				//��ȡ�����ĵ�ID
				unsigned short *pID = (unsigned short *)malloc(sizeof(unsigned short));
				memcpy(pID, recvbuf, sizeof(unsigned short));

				//ת��ID
				unsigned short nID = RegisterNewID(ntohs(*pID), clientName, FALSE);

				//��ӡ ʱ�� newID ���� ���� IP
				DisplayInfo(nID, find);

				//������Ӧ���ķ���
				memcpy(sendbuf, recvbuf, iRecv);						//����������
				unsigned short a = htons(0x8180);
				memcpy(&sendbuf[2], &a, sizeof(unsigned short));		//�޸ı�־��

				//�޸Ļش�����
				if (strcmp(DNS_cache[find].IP.c_str(), "0.0.0.0") == 0)
					a = htons(0x0000);	//���ι��ܣ��ش���Ϊ0
				else
					a = htons(0x0001);	//���������ܣ��ش���Ϊ1
				memcpy(&sendbuf[6], &a, sizeof(unsigned short));
				int curLen = 0;

				//����DNS��Ӧ����
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

				//�����ĺ���Ӧ���ֹ�ͬ���DNS��Ӧ���Ĵ���sendbuf
				memcpy(sendbuf + iRecv, answer, curLen);

				//����DNS��Ӧ����
				iSend = sendto(socketLocal, sendbuf, curLen, 0, (SOCKADDR*)&clientName, sizeof(clientName));
				if (iSend == SOCKET_ERROR) {
					cout << "sendto Failed: " << WSAGetLastError() << endl;
					continue;
				}
				else if (iSend == 0)
					break;

				free(pID);		//�ͷŶ�̬������ڴ�
			}
		}
	}

	closesocket(socketServer);	//�ر��׽���
	closesocket(socketLocal);
	WSACleanup();				//�ͷ�ws2_32.dll��̬���ӿ��ʼ��ʱ�������Դ

	return 0;
}
