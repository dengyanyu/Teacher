#include "stdafx.h"
#include "SocketCenter.h"


CSocketCenter::CSocketCenter()
// 	: m_screenSocketRecv(INVALID_SOCKET)
	: m_socketInit(INVALID_SOCKET)
// 	, m_diskDownLoadSocket(INVALID_SOCKET)
// 	, m_diskInfoSocket(INVALID_SOCKET)
{
}


CSocketCenter::~CSocketCenter()
{
	if (m_socketInit != INVALID_SOCKET)
	{
		closesocket(m_socketInit);
		m_socketInit = INVALID_SOCKET;
	}
}

/******************************************************************
Function	: Clean
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:26
Return		: void
Desc		: 清理
******************************************************************/
void CSocketCenter::Clean()
{
	::WSACleanup();
}


/******************************************************************
Function	: SendDataTCP
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:26
Parameter	: socket--SOCKET
Parameter	: buf--传输的数据的基地址
Parameter	: bytes--传输的数据的大小，buf中数据的大小
Return		: int--发送成功的数据的字节数
Desc		: 基于TCP 传输数据到学生机
******************************************************************/
int CSocketCenter::SendDataTCP(SOCKET socket, const char* buf, int bytes)
{
	const char *b = buf;
	while (bytes > 0)
	{
		int r = send(socket, b, bytes, 0);
// 		if (SOCKET_ERROR == r)
// 		{
// 			OutputError();
// 		}
		if (r < 0)
		{
			return r;
		}
		else if (r == 0)
		{
			break;
		}
		bytes -= r;
		b += r;
	}
	return b - (char*)buf;
}


/******************************************************************
Function	: OutputError
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:27
Return		: void
Desc		: 数据传输出错后输出的信息
******************************************************************/
void CSocketCenter::OutputError()
{
	int iError = WSAGetLastError();
	if (iError == WSAEWOULDBLOCK)
		AfxMessageBox(_T("recv failed with error : WSAEWOULDBLOCK\n"));
	else
	{
		CString str;
		str.Format(_T("recv failed with error: %ld\n"), iError);
		AfxMessageBox(str);
	}
}


/******************************************************************
Function	: RecvDataTCP
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:27
Parameter	: socket--SOCKET
Parameter	: buf--传输的数据的基地址  
Parameter	: bytes--传输的数据的大小，buf中数据的大小
Return		: int--传输成功的数据的大小
Desc		: 基于TCP接收来自学生机的数据
******************************************************************/
int CSocketCenter::RecvDataTCP(SOCKET socket, char* buf, int bytes)
{
	char *b = (char*)buf;
	while (bytes > 0)
	{
		int r = recv(socket, b, bytes, 0);
// 		if (SOCKET_ERROR == r)
// 		{
// 			OutputError();
// 		}
		if (r < 0)
		{
			return 0;
		}
		else if (r == 0)
		{
			break;
		}
		bytes = bytes - r;
		b = b + r;
	}
	return b - (char*)buf;
}


/******************************************************************
Function	: InitSocketInfo
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:27
Parameter	: port--监听的端口号
Parameter	: listenCount--最大监听的学生机的数量
Return		: bool--连接成功与否
Desc		: 初始化SOCKET 信息，之后可以建立连接
******************************************************************/
bool CSocketCenter::InitSocketInfo(int port, int listenCount)
{
	WSADATA wsaData;
	int err = ::WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (err != 0)
	{
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.                                  */
		return 0 ;
	}
	if (LOBYTE(wsaData.wVersion) != 2 ||
		HIBYTE(wsaData.wVersion) != 2)
	{
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.                                  */
		WSACleanup();
		return 0;
	}
//	SOCKET socketInit;
	m_socketInit = ::socket(AF_INET, SOCK_STREAM, 0);
	if (m_socketInit == SOCKET_ERROR)
	{
		AfxMessageBox(_T("创建连接失败"));
		//		return SOCKET_ERROR;
		return false;
	}

	SOCKADDR_IN saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_addr.S_un.S_addr = INADDR_ANY;
	saddr.sin_port = htons(port);

	int nRet;
	nRet = ::bind(m_socketInit, (SOCKADDR *)&saddr, sizeof(saddr));
	if (nRet == SOCKET_ERROR)
	{
		AfxMessageBox(_T("绑定端口失败"));
		return false;
	}

	nRet = listen(m_socketInit, listenCount);
	if (nRet == SOCKET_ERROR)
	{
		AfxMessageBox(_T("监听端口失败"));
		return false;
	}
	return true;
}


/******************************************************************
Function	: SendReadyInfo
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:28
Parameter	: socket--与学生机连接成功的SOCKET
Parameter	: msgID--请求的类型，比如屏幕监控
Return		: void
Desc		: 发送请求消息到学生机
******************************************************************/
void CSocketCenter::SendReadyInfo(SOCKET socket, int msgID)
{
	MSGTYPE msgType;
	msgType.msgID = msgID;
	// 	msgType.flag = false;
	SendDataTCP(socket, (char*)&msgType, sizeof(MSGTYPE));
}


/******************************************************************
Function	: GetSocketInit
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:28
Return		: SOCKET
Desc		: 获得初始化的SOCKET，不是用于接发数据
******************************************************************/
SOCKET CSocketCenter::GetSocketInit()
{
	return m_socketInit;
}


/******************************************************************
Function	: InitMulticastSocket
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:28
Parameter	: port--广播监听的消息
Parameter	: multicastIp--组播IP
Parameter	: addr--sockaddr_in
Return		: SOCKET
Desc		: 初始化广播的SOCKET 信息
******************************************************************/
SOCKET CSocketCenter::InitMulticastSocket(int port, char* multicastIp, sockaddr_in& addr)
{
	WSAData wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("Error in WSAStartup");
	}

	int fd;

	/* create what looks like an ordinary UDP socket */
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("socket");
		exit(1);
	}

	/* set up destination address */
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(multicastIp);
	addr.sin_port = htons(port);

	return fd;
}


/******************************************************************
Function	: SendDataUDP
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:29
Parameter	: socket--广播SOCKET
Parameter	: sendBuf--要发送的数据的大小
Parameter	: sendBytes--发送的数据的大小，sendBuf指向的数据的大小
Parameter	: addr--sockaddr_in
Return		: int
Desc		: 基于UDP 发送数据到学生机
******************************************************************/
int CSocketCenter::SendDataUDP(SOCKET socket, const char* sendBuf, int sendBytes, sockaddr_in addr)
{
	int addrlen = sizeof(sockaddr_in);
	const char *pNeedToSend = sendBuf;
	while (sendBytes > 0)
	{
		int sendedAmount = sendto(socket, pNeedToSend, sendBytes, 0, 
			(sockaddr *)&addr, addrlen);
		if (sendedAmount < 0)
		{
			int err = GetLastError();
			CString str;
			str.Format(_T("Socket_Error: %d"), err);
			AfxMessageBox(str);
			return sendedAmount;
		}
		else if (sendedAmount == 0)
		{
			AfxMessageBox(_T("CSocketCenter::SendDataUDP"));
			break;
		}
		sendBytes = sendBytes - sendedAmount;
		pNeedToSend += sendedAmount;
	}
	return pNeedToSend - sendBuf;
}
/*
bool CSocketCenter::GetLocalIP(char* ip)
{
	//1.初始化wsa  
	WSADATA wsaData;
	int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (ret != 0)
	{
		return false;
	}
	//2.获取主机名  
	char hostname[256];
	ret = gethostname(hostname, sizeof(hostname));
	if (ret == SOCKET_ERROR)
	{
		return false;
	}
	//3.获取主机ip  
	HOSTENT* host = gethostbyname(hostname);
	if (host == NULL)
	{
		return false;
	}
	//4.转化为char*并拷贝返回  
	strcpy(ip, inet_ntoa(*(in_addr*)*host->h_addr_list));
	return true;
}*/
//结构体记录ip信息



/******************************************************************
Function	: GetLocalIPs
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:30
Parameter	: ips--返回的多个IP地址表
Parameter	: maxCnt--最多获取多少IP地址
Parameter	: cnt--取得的IP地址数量
Return		: bool
Desc		: 获取多个ip地址信息列表
******************************************************************/
bool CSocketCenter::GetLocalIPs(IPInfo* ips, int maxCnt, int* cnt)
{
	//1.初始化wsa  
	WSADATA wsaData;
	int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (ret != 0)
	{
		return false;
	}
	//2.获取主机名  
	char hostname[256];
	ret = gethostname(hostname, sizeof(hostname));
	if (ret == SOCKET_ERROR)
	{
		return false;
	}
	//3.获取主机ip  
	HOSTENT* host = gethostbyname(hostname);
	if (host == NULL)
	{
		return false;
	}
	//4.逐个转化为char*并拷贝返回  
	*cnt = host->h_length<maxCnt ? host->h_length : maxCnt;
	for (int i = 0; i<*cnt; i++)
	{
		in_addr* addr = (in_addr*)*host->h_addr_list;
		strcpy(ips[i].ip, inet_ntoa(addr[i]));
	}
	return true;
}

/******************************************************************
Function	: doServer
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:31
Return		: bool
Desc		: 启动IP寻址服务器
******************************************************************/
bool CSocketCenter::doServer() {
	int m_nPort = IP_PORT;
	const int MAX_BUF_LEN = 255;

	SOCKET sClient;
	sockaddr_in clientAddr, bindAddr;
	WSADATA wsdata;

	//启动SOCKET库，版本为2.0
	WORD    wVer = MAKEWORD(2, 0);
	if (0 != WSAStartup(wVer, &wsdata))
	{
		//AfxMessageBox(L"Not Support Socket2.0");
		return false;
	}

	//用UDP初始化套接字
	sClient = socket(AF_INET, SOCK_DGRAM, 0);
	//设置该套接字为广播类型，
	BOOL optval = TRUE;
	bindAddr.sin_family = AF_INET;
	bindAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	bindAddr.sin_port = htons(m_nPort);
	setsockopt(sClient, SOL_SOCKET, SO_BROADCAST, (char FAR *)&optval, sizeof(optval));
	bind(sClient, (sockaddr *)&bindAddr, sizeof(sockaddr_in));

	int nAddrLen = sizeof(SOCKADDR);
	char buf[256] = { 0 };
	int fromlength = sizeof(SOCKADDR);
	/*
	char ipaddr[30] = { 0 };

	char buff[MAX_BUF_LEN] = "";
	if (GetLocalIP(ipaddr))
	{
		sprintf(buff, "%s", ipaddr);
	}
	else
	{
		sprintf(buff, "%s", "");
	}*/

	//有多个ip地址的时候，这样调用
	IPInfo ips[10];
	int len1 = 0;
	GetLocalIPs(ips, 10,&len1);

	while (true)
	{
		int nRet = recvfrom(sClient, buf, 256, 0, (struct sockaddr FAR *)&clientAddr, (int FAR *)&fromlength);
		if (SOCKET_ERROR != nRet)
		{
			char    *pIPAddr = inet_ntoa(clientAddr.sin_addr);
			if (NULL != pIPAddr)
			{
				WCHAR    wzIPBuffer[32] = { 0 };
				printf("clientAddr: %s\n", pIPAddr);
				printf("receive command: %s\n", buf);
			}
			if (strcmp(buf, GET_HOST_COMMAND) != 0)
			{
				continue;
			}
			bool que = false;
			char a2[10], b2[10], c2[10], d2[10];
			char buff[MAX_BUF_LEN] = "";
			
			sscanf(pIPAddr, "%[^.].%[^.].%[^.].%[^.]", a2, b2, c2, d2);

			for (int i = 0; i < len1; i ++) {
				//判断哪个ip是一个段的
				sprintf(buff, "%s", ips[i].ip);
				char a1[10], b1[10], c1[10], d1[10];
				sscanf(buff, "%[^.].%[^.].%[^.].%[^.]", a1, b1, c1, d1);
				if (strcmp(a1,a2)==0 && strcmp(b1, b2) == 0 && strcmp(c1, c2) == 0) {
					que = true;
					break;
				}

			}
			if (!que) {
				continue;
			}
			// 发送数据   
			int nSendSize = sendto(sClient, buff, strlen(buff), 0, (SOCKADDR*)&clientAddr, nAddrLen);
			if (SOCKET_ERROR == nSendSize)
			{
				int err = WSAGetLastError();
				printf("\"sendto\" error!, error code is %d\n", err);
				return false;
			}

		}
		else
		{
			//AfxMessageBox(L"Recv UDP Failed");
		}

		Sleep(1000);
	}

	closesocket(sClient);
	return true;
}