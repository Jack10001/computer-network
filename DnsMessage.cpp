#include "DnsMessage.h"

DnsMessage::DnsMessage(char *recvbuf, int recvlen)
{
	//Clear();
	m_strDnsBuf = string(recvbuf, recvlen);//�����ܵ��ı���ת��Ϊstring
}

DnsMessage::~DnsMessage()
{}


//�ɹ����ڴ�������ת��
//ʧ�ܣ�ת��

bool DnsMessage::StartAnalysis()
{
	int dnsLen = m_strDnsBuf.size();
	m_pDnsHdr = (DNS_HDR *)m_strDnsBuf.c_str();

	//��ȡdnsͷ��Ϣ
	GetDNSHeader(m_pDnsHdr);
	
	//��ȡdns����ͷ��Ϣ:Question

		GetDns_Question(m_pDnsHdr);

	//������ȡ����dns��Ӧͷ��Ϣ:Answer
		GetDns_Response(m_pDnsHdr);
	return true;
}

void DnsMessage::GetDNSHeader(const DNS_HDR *dns)
{
	m_DnsHdr.ID = ntohs(dns->ID);
	m_DnsHdr.Flags = ntohs(dns->Flags);
	m_DnsHdr.QuestNum = ntohs(dns->QuestNum);
	m_DnsHdr.AnswerNum = ntohs(dns->AnswerNum);
	m_DnsHdr.AuthorNum = ntohs(dns->AuthorNum);
	m_DnsHdr.AdditionNum = ntohs(dns->AdditionNum);

	//return (1 == m_DnsHdr.QuestNum);
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

bool DnsMessage::IsIpv4()
{
	return (1 == m_Question.question.type);
}

void DnsMessage::GetDns_Response(const DNS_HDR *dns)
{
	//����dns��Ӧ���ż���������ȥ
	if (IsRequest()||!IsIpv4())
	{
		return;
	}
	Resource *pResource;
	SAnswer *pAnswer;
	char *pDns = (char *)dns;
	int iSeek=0;
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
		

		pAnswer->resource.type = ntohs(pResource->type);
		pAnswer->resource.qclass = ntohs(pResource->qclass);
		pAnswer->resource.ttl = ntohl(pResource->ttl);
		pAnswer->resource.len = ntohs(pResource->len);

		pDns += sizeof(Resource);//����Resource�ռ�,ָ����IP��ַ�����ڴ�ռ�
		
		pDns -= 2;
		//ipv4��ַ
		if (1 == pAnswer->resource.type)
		{ 

			struct in_addr * temp = (struct in_addr *)pDns;
			pAnswer->result = inet_ntoa(*temp);//��ip��ַת��Ϊ�ַ�����
			pDns += 4;//�ƶ��ĸ��ֽڵ�λ��
		}
		//CNAME
		else if (5 == pAnswer->resource.type)//������
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

	//��ȡ��ѹ������ָ�������
	if (!IsCompressedName(p, NULL))
	{
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

			//������ѹ��ָ�룬��ѭ������ѹ��ָ����ָ������
			if (IsCompressedName(p, NULL))
			{
				break;
			}
			//���������ַ�����˵�������Ѿ�������ֱ�ӷ���
			if (!*p)
			{
				p++;//��Ϊ��ѹ�����ƺ��滹��һ�����ַ�
				if (pLen)
				{
					*pLen = p - start;
				}
				return strDomain;//����
			}
		}
	}

	//��ȡѹ������ָ�������
	tmp = p;//p�Ѿ��ı�
	do
	{
		if (IsCompressedName(tmp, &iSeek))
		{
			tmp = (char *)first + iSeek;//ָ��ѹ������

			while (1)
			{
				iLen = *tmp++;
				for (int i = 0; i < iLen; i++)
				{
					strDomain += *tmp++;
				}
				//��tmp+1�������ݣ������'.'
				if (*tmp)
				{
					strDomain += '.';
				}

				//������ѹ��ָ�룬��ѭ������ѹ��ָ����ָ������
				if (!*tmp || IsCompressedName(tmp, NULL))
				{
					break;
				}
			}
		}
	} while (*tmp);
	p += 2;//����ѹ��ָ��

	if (pLen)
	{
		*pLen = p - start;
	}
	return strDomain;
}

bool DnsMessage::IsCompressedName(const char *ptr, int *seek)
{
	USHORT flag = ntohs(*(USHORT *)ptr);
	//0xc00c
	if (flag & 0xc000)
	{
		if (seek)
		{
			*seek = flag & 0x3fff;//ʣ�µ�14λ����ѹ������ָ��λ��
		}
		return true;
	}
	return false;
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
		if (1 == pAnswer->resource.type)
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

