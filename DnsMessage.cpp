#include "DnsMessage.h"

DnsMessage::DnsMessage(char *recvbuf, int recvlen)
{
	Clear();
	m_strDnsBuf = string(recvbuf, recvlen);//�����ܵ��ı���ת��Ϊstring
}

DnsMessage::~DnsMessage()
{
	Clear();
}


//�ɹ����ڴ�������ת��
//ʧ�ܣ�ת��

bool DnsMessage::StartAnalysis()
{
	int dnsLen = m_strDnsBuf.size();
	m_pDnsHdr = (DNS_HDR *)m_strDnsBuf.c_str();

	//��ȡdnsͷ��Ϣ
	if (!GetDNSHeader(m_pDnsHdr))
	{
		return false;
	}
	//��ȡdns����ͷ��Ϣ:Question

	if (IsRequest())
	{
		GetDns_Question(m_pDnsHdr);
	}

	//������ȡ����dns��Ӧͷ��Ϣ:Answer
	if (IsResponce())
	{
		GetDns_Response(m_pDnsHdr);
	}
	return true;
}

bool DnsMessage::GetDNSHeader(const DNS_HDR *dns)
{
	m_DnsHdr.ID = ntohs(dns->ID);
	m_DnsHdr.Flags = ntohs(dns->Flags);
	m_DnsHdr.QuestNum = ntohs(dns->QuestNum);
	m_DnsHdr.AnswerNum = ntohs(dns->AnswerNum);
	m_DnsHdr.AuthorNum = ntohs(dns->AuthorNum);
	m_DnsHdr.AdditionNum = ntohs(dns->AdditionNum);

	return (1 == m_DnsHdr.QuestNum);
}




void DnsMessage::GetDns_Question(const DNS_HDR *dns)
{
	char *pDns = (char *)dns;
	string strDomain;
	int iSeek;
	Question *pQue;

	strDomain = GetDomain(pDns, pDns + sizeof(DNS_HDR), &iSeek);
	m_Question.domain = strDomain;

	pDns += sizeof(DNS_HDR) + iSeek;//����dnsͷ����������ָ��Question
	pQue = (Question *)pDns;
	m_Question.question.type = ntohs(pQue->type);
	m_Question.question.qclass = ntohs(pQue->qclass);
}

void DnsMessage::GetDns_Response(const DNS_HDR *dns)
{
	//����dns��Ӧ���ż���������ȥ
	Resource *pResource;
	SAnswer *pAnswer;
	char *pDns = (char *)dns;
	int iSeek;
	//header | question | answer | auth |addtional 
	//ָ���ƶ���Answer�׵�ַ
	pDns += sizeof(DNS_HDR);
	while (*pDns++);
	pDns += sizeof(Question);

	//pDnsָ��Answer�׵�ַ,���صı����а������ip��ַ
	//ѭ������
	for (int i = 0; i < m_DnsHdr.AnswerNum; i++)
	{
		pAnswer = new SAnswer;
		pAnswer->domain = GetDomain((char *)dns, pDns, &iSeek);
		pDns += iSeek;

		pResource = (Resource *)pDns;
		pDns += sizeof(Resource);//����Resource�ռ�

		pAnswer->resource.type = ntohs(pResource->type);
		pAnswer->resource.qclass = ntohs(pResource->qclass);
		pAnswer->resource.ttl = ntohl(pResource->ttl);
		pAnswer->resource.len = ntohs(pResource->len);

		//ipv4��ַ
		if (A == pAnswer->resource.type)
		{
			pAnswer->result = inet_ntoa(*(struct in_addr *)pDns);//��ip��ַת��Ϊ�ַ�����
			pDns += 4;
		}
		//CNAME
		else if (CNAME == pAnswer->resource.type)//������
		{
			pAnswer->result = GetDomain((char *)dns, pDns, &iSeek);
			pDns += iSeek;
		}
		//��ӵ�Ӧ������
		m_AnswerList.push_back(pAnswer);
	}
}


//��ȡ�������е�����
string DnsMessage::GetDomain(const char *first, const char *start, int *pLen)
{
	char *p = (char *)start;
	char *tmp;
	int iLen = 0;
	int iSeek = 0;
	string strDomain;

	while (1)
	{
		iLen = *p++;
		for (int i = 0; i < iLen; i++)
		{
			strDomain += *p++;
		}
		if (*p)
		{
			strDomain += '.';
		}
		//���������ַ�����˵�������Ѿ�������ֱ�ӷ���
		if (!*p)
		{
			p++;
			if (pLen)
			{
				*pLen = p - start;
			}
			return strDomain;//����
		}
	}
	if (pLen)
	{
		*pLen = p - start;
	}
	return strDomain;
}




StringList DnsMessage::Get_CNAME_List()
{
	StringList strList;

	SAnswer *pAnswer;
	for (AnswerList::iterator it = m_AnswerList.begin(); it != m_AnswerList.end(); ++it)
	{
		pAnswer = *it;
		if (CNAME == pAnswer->resource.type)
		{
			strList.push_back(pAnswer->result);
		}
	}

	return strList;
}

StringList DnsMessage::Get_Ip_List()
{
	StringList strList;

	SAnswer *pAnswer;
	for (AnswerList::iterator it = m_AnswerList.begin(); it != m_AnswerList.end(); ++it)
	{
		pAnswer = *it;
		if (A == pAnswer->resource.type)
		{
			strList.push_back(pAnswer->result);
		}
	}

	return strList;
}


bool DnsMessage::IsRequest()
{
	return (0 == (m_DnsHdr.Flags & DNS_FLAG_QR));
}

bool DnsMessage::IsResponce()
{
	return (1 == (m_DnsHdr.Flags & DNS_FLAG_QR));
}

USHORT DnsMessage::GetDnsId()
{
	return m_DnsHdr.ID;
}

USHORT DnsMessage::GetDnsFlag()
{
	return m_DnsHdr.Flags;
}

USHORT DnsMessage::GetQuestions()
{
	return m_DnsHdr.QuestNum;
}

USHORT DnsMessage::GetAnswers()
{
	return m_DnsHdr.AnswerNum;
}

USHORT DnsMessage::GetAuthority()
{
	return m_DnsHdr.AuthorNum;
}

USHORT DnsMessage::GetAdditional()
{
	return m_DnsHdr.AdditionNum;
}



USHORT DnsMessage::GetFlagQR()
{
	return m_DnsHdr.Flags & DNS_FLAG_QR;
}

USHORT DnsMessage::GetFlagOPCODE()
{
	return m_DnsHdr.Flags & DNS_FLAG_OPCODE;
}

USHORT DnsMessage::GetFlagAA()
{
	return m_DnsHdr.Flags & DNS_FLAG_AA;
}

USHORT DnsMessage::GetFlagTC()
{
	return m_DnsHdr.Flags & DNS_FLAG_TC;
}

USHORT DnsMessage::GetFlagRD()
{
	return m_DnsHdr.Flags & DNS_FLAG_RD;
}

USHORT DnsMessage::GetFlagRA()
{
	return m_DnsHdr.Flags & DNS_FLAG_RA;
}

USHORT DnsMessage::GetFlagZ()
{
	return m_DnsHdr.Flags & DNS_FLAG_Z;
}

USHORT DnsMessage::GetFlagRCODE()
{
	return m_DnsHdr.Flags & DNS_FLAG_RCODE;
}

/*
bool DnsMessage::IsIpv4()
{
	return (A == m_Question.question.type);
}

bool DnsMessage::IsIpv6()
{
	return (AAAA == m_Question.question.type);
}

*/

string DnsMessage::QuestionDomain()
{
	return m_Question.domain;
}


void DnsMessage::Clear()
{
	memset(&m_DnsHdr, 0, sizeof(DNS_HDR));
	delete m_pDnsHdr;
	for (AnswerList::iterator it = m_AnswerList.begin(); it != m_AnswerList.end(); ++it)
	{
		delete *it;
	}
	m_AnswerList.clear();
}

