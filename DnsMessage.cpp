#include "DnsMessage.h"

DnsMessage::DnsMessage(char *recvbuf, int recvlen)
{
	//Clear();
	m_strDnsBuf = string(recvbuf, recvlen);//将接受到的报文转化为string
}

DnsMessage::~DnsMessage()
{}


//成功：内存搜索、转发
//失败：转发

bool DnsMessage::StartAnalysis()
{
	int dnsLen = m_strDnsBuf.size();
	m_pDnsHdr = (DNS_HDR *)m_strDnsBuf.c_str();

	//获取dns头信息
	GetDNSHeader(m_pDnsHdr);
	
	//获取dns请求头信息:Question

		GetDns_Question(m_pDnsHdr);

	//遍历获取所有dns响应头消息:Answer
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

	pDns += sizeof(DNS_HDR) + iSeek;//跳过dns头部和域名，指向Question
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
	//若是dns响应，才继续解析下去
	if (IsRequest()||!IsIpv4())
	{
		return;
	}
	Resource *pResource;
	SAnswer *pAnswer;
	char *pDns = (char *)dns;
	int iSeek=0;
	//header | question | answer | auth |addtional 
	//指针移动到Answer首地址
	pDns += sizeof(DNS_HDR);
	while (*pDns++);
	pDns += sizeof(Question);

	//pDns指向Answer首地址,返回的报文中包含多个ip地址
	//循环解析
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

		pDns += sizeof(Resource);//跳过Resource空间,指向存放IP地址处的内存空间
		
		pDns -= 2;
		//ipv4地址
		if (1 == pAnswer->resource.type)
		{ 

			struct in_addr * temp = (struct in_addr *)pDns;
			pAnswer->result = inet_ntoa(*temp);//将ip地址转换为字符串型
			pDns += 4;//移动四个字节的位置
		}
		//CNAME
		else if (5 == pAnswer->resource.type)//主机名
		{
			pAnswer->result = GetDomain((char *)dns, pDns, &iSeek);
			pDns += iSeek;
		}
		//添加到应答数组
		m_AnswerList.push_back(pAnswer);
	}
}


//获取请求报文中的域名
string DnsMessage::GetDomain(const char *first, const char *start, int *pLen)
{
	char *p = (char *)start;
	char *tmp;
	int iLen = 0;
	int iSeek = 0;
	string strDomain;

	//获取非压缩名称指针的内容
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

			//若遇到压缩指针，则循环遍历压缩指针所指的名称
			if (IsCompressedName(p, NULL))
			{
				break;
			}
			//若遇到空字符，则说明域名已经结束，直接返回
			if (!*p)
			{
				p++;//因为非压缩名称后面还有一个空字符
				if (pLen)
				{
					*pLen = p - start;
				}
				return strDomain;//返回
			}
		}
	}

	//获取压缩名称指针的内容
	tmp = p;//p已经改变
	do
	{
		if (IsCompressedName(tmp, &iSeek))
		{
			tmp = (char *)first + iSeek;//指向压缩名称

			while (1)
			{
				iLen = *tmp++;
				for (int i = 0; i < iLen; i++)
				{
					strDomain += *tmp++;
				}
				//若tmp+1还有内容，则添加'.'
				if (*tmp)
				{
					strDomain += '.';
				}

				//若遇到压缩指针，则循环遍历压缩指针所指的名称
				if (!*tmp || IsCompressedName(tmp, NULL))
				{
					break;
				}
			}
		}
	} while (*tmp);
	p += 2;//跳过压缩指针

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
			*seek = flag & 0x3fff;//剩下的14位就是压缩名称指针位移
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

