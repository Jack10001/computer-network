/*
*解析DNS的请求报文和相应报文
*/

#pragma once
#include"Dns_relay.h"

#pragma pack(2)//2字节对齐
#define DNS_FLAG_QR			0x8000	//查询(0) | 响应(1)
#define DNS_FLAG_OPCODE		0x7000	//标准查询(0) | 反向查询(1) | 服务器状态请求(2) | 保留(3-15)
#define DNS_FLAG_AA			0x0400	//权威回答(1) | 非权威回答(0), 响应中有效
#define DNS_FLAG_TC			0x0200	//截断(1), 若该位为1, 将会使用TCP重新请求
#define DNS_FLAG_RD			0x0100	//期望递归
#define DNS_FLAG_RA			0x0080	//递归可用
#define DNS_FLAG_Z			0x0070	//保留
#define DNS_FLAG_RCODE		0x000f	//没有错误(0) | 格式错误(1) | 服务器故障(2) | 名称错误(3) | 未实现(4) | 拒绝(5) | 保留(6-15)


typedef vector<string> StringList;
typedef vector<SAnswer*> AnswerList;
typedef unsigned short u_short;

enum { CNAME=28, A=1 };

class DnsMessage
{
public:
	DnsMessage(char *recvbuf, int recv_len);
	~DnsMessage();

	bool	StartAnalysis();
	//dns头信息
	bool	IsRequest();
	bool	IsResponce();
	u_short	GetDnsId();
	u_short	GetDnsFlag();
	u_short	GetQuestions();
	u_short	GetAnswers();
	u_short	GetAuthority();
	u_short	GetAdditional();
	//获取dns标志
	u_short	GetFlagQR();//查询(0) | 响应(1)
	u_short	GetFlagOPCODE();//标准查询(0) | 反向查询(1) | 服务器状态请求(2) | 保留(3-15)
	u_short	GetFlagAA();//权威回答(1) | 非权威回答(0), 响应中有效
	u_short	GetFlagTC();//截断(1), 若该位为1, 将会使用TCP重新请求
	u_short	GetFlagRD();//期望递归
	u_short	GetFlagRA();//递归可用
	u_short	GetFlagZ();//保留
	u_short	GetFlagRCODE();//没有错误(0) | 格式错误(1) | 服务器故障(2) | 名称错误(3) | 未实现(4) | 拒绝(5) | 保留(6-15)

	//dns请求 or 响应信息
	string  QuestionDomain();//请求域名
	StringList Get_CNAME_List();//CNAME数组
	StringList Get_Ip_List();//IP数组

	bool IsCompressedName(const char *ptr, int *seek);
	bool IsIpv4();
	

	//log
	//void	PrintLog();

private:
    void    GetDNSHeader(const DNS_HDR *dns);//获取dns头信息
	void	GetDns_Question(const DNS_HDR *dns);//获取dns请求信息:Question
	void	GetDns_Response(const DNS_HDR *dns);//遍历获取所有dns响应:Answer
	string	GetDomain(const char *first, const char *start, int *pLen = NULL);//获取域名,first为dns首地址，first为域名读取的首地址
	void	Clear();

private:
	DNS_HDR	 m_DnsHdr;//dns头部
	SQuestion m_Question;//请求报文
	AnswerList m_AnswerList;//应答数组
	string m_strDnsBuf;//dns请求/响应缓存
	DNS_HDR  *m_pDnsHdr;
};

