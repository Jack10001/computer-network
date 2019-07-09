/*
*����DNS�������ĺ���Ӧ����
*/

#pragma once
#include"Dns_relay.h"

#pragma pack(2)//2�ֽڶ���
#define DNS_FLAG_QR			0x8000	//��ѯ(0) | ��Ӧ(1)
#define DNS_FLAG_OPCODE		0x7000	//��׼��ѯ(0) | �����ѯ(1) | ������״̬����(2) | ����(3-15)
#define DNS_FLAG_AA			0x0400	//Ȩ���ش�(1) | ��Ȩ���ش�(0), ��Ӧ����Ч
#define DNS_FLAG_TC			0x0200	//�ض�(1), ����λΪ1, ����ʹ��TCP��������
#define DNS_FLAG_RD			0x0100	//�����ݹ�
#define DNS_FLAG_RA			0x0080	//�ݹ����
#define DNS_FLAG_Z			0x0070	//����
#define DNS_FLAG_RCODE		0x000f	//û�д���(0) | ��ʽ����(1) | ����������(2) | ���ƴ���(3) | δʵ��(4) | �ܾ�(5) | ����(6-15)


typedef vector<string> StringList;
typedef vector<SAnswer*> AnswerList;
typedef unsigned short u_short;

enum { CNAME, A };

class DnsMessage
{
public:
	DnsMessage(char *recvbuf, int recv_len);
	~DnsMessage();

	bool	StartAnalysis();
	//dnsͷ��Ϣ
	bool	IsRequest();
	bool	IsResponce();
	u_short	GetDnsId();
	u_short	GetDnsFlag();
	u_short	GetQuestions();
	u_short	GetAnswers();
	u_short	GetAuthority();
	u_short	GetAdditional();
	//��ȡdns��־
	u_short	GetFlagQR();//��ѯ(0) | ��Ӧ(1)
	u_short	GetFlagOPCODE();//��׼��ѯ(0) | �����ѯ(1) | ������״̬����(2) | ����(3-15)
	u_short	GetFlagAA();//Ȩ���ش�(1) | ��Ȩ���ش�(0), ��Ӧ����Ч
	u_short	GetFlagTC();//�ض�(1), ����λΪ1, ����ʹ��TCP��������
	u_short	GetFlagRD();//�����ݹ�
	u_short	GetFlagRA();//�ݹ����
	u_short	GetFlagZ();//����
	u_short	GetFlagRCODE();//û�д���(0) | ��ʽ����(1) | ����������(2) | ���ƴ���(3) | δʵ��(4) | �ܾ�(5) | ����(6-15)

	//dns���� or ��Ӧ��Ϣ
	string  QuestionDomain();//��������
	StringList Get_CNAME_List();//CNAME����
	StringList Get_Ip_List();//IP����

	//log
	//void	PrintLog();

private:
	bool    GetDNSHeader(const DNS_HDR *dns);//��ȡdnsͷ��Ϣ
	void	GetDns_Question(const DNS_HDR *dns);//��ȡdns������Ϣ:Question
	void	GetDns_Response(const DNS_HDR *dns);//������ȡ����dns��Ӧ:Answer
	string	GetDomain(const char *first, const char *start, int *pLen = NULL);//��ȡ����,firstΪdns�׵�ַ��firstΪ������ȡ���׵�ַ
	void	Clear();

private:
	DNS_HDR	 m_DnsHdr;//dnsͷ��
	SQuestion m_Question;//������
	AnswerList m_AnswerList;//Ӧ������
	string m_strDnsBuf;//dns����/��Ӧ����
	DNS_HDR  *m_pDnsHdr;
};

