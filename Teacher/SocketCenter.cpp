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
Desc		: ����
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
Parameter	: buf--��������ݵĻ���ַ
Parameter	: bytes--��������ݵĴ�С��buf�����ݵĴ�С
Return		: int--���ͳɹ������ݵ��ֽ���
Desc		: ����TCP �������ݵ�ѧ����
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
Desc		: ���ݴ��������������Ϣ
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
Parameter	: buf--��������ݵĻ���ַ  
Parameter	: bytes--��������ݵĴ�С��buf�����ݵĴ�С
Return		: int--����ɹ������ݵĴ�С
Desc		: ����TCP��������ѧ����������
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
Parameter	: port--�����Ķ˿ں�
Parameter	: listenCount--��������ѧ����������
Return		: bool--���ӳɹ����
Desc		: ��ʼ��SOCKET ��Ϣ��֮����Խ�������
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
		AfxMessageBox(_T("��������ʧ��"));
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
		AfxMessageBox(_T("�󶨶˿�ʧ��"));
		return false;
	}

	nRet = listen(m_socketInit, listenCount);
	if (nRet == SOCKET_ERROR)
	{
		AfxMessageBox(_T("�����˿�ʧ��"));
		return false;
	}
	return true;
}


/******************************************************************
Function	: SendReadyInfo
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:28
Parameter	: socket--��ѧ�������ӳɹ���SOCKET
Parameter	: msgID--��������ͣ�������Ļ���
Return		: void
Desc		: ����������Ϣ��ѧ����
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
Desc		: ��ó�ʼ����SOCKET���������ڽӷ�����
******************************************************************/
SOCKET CSocketCenter::GetSocketInit()
{
	return m_socketInit;
}


/******************************************************************
Function	: InitMulticastSocket
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:28
Parameter	: port--�㲥��������Ϣ
Parameter	: multicastIp--�鲥IP
Parameter	: addr--sockaddr_in
Return		: SOCKET
Desc		: ��ʼ���㲥��SOCKET ��Ϣ
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
Parameter	: socket--�㲥SOCKET
Parameter	: sendBuf--Ҫ���͵����ݵĴ�С
Parameter	: sendBytes--���͵����ݵĴ�С��sendBufָ������ݵĴ�С
Parameter	: addr--sockaddr_in
Return		: int
Desc		: ����UDP �������ݵ�ѧ����
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
	//1.��ʼ��wsa  
	WSADATA wsaData;
	int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (ret != 0)
	{
		return false;
	}
	//2.��ȡ������  
	char hostname[256];
	ret = gethostname(hostname, sizeof(hostname));
	if (ret == SOCKET_ERROR)
	{
		return false;
	}
	//3.��ȡ����ip  
	HOSTENT* host = gethostbyname(hostname);
	if (host == NULL)
	{
		return false;
	}
	//4.ת��Ϊchar*����������  
	strcpy(ip, inet_ntoa(*(in_addr*)*host->h_addr_list));
	return true;
}*/
//�ṹ���¼ip��Ϣ



/******************************************************************
Function	: GetLocalIPs
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:30
Parameter	: ips--���صĶ��IP��ַ��
Parameter	: maxCnt--����ȡ����IP��ַ
Parameter	: cnt--ȡ�õ�IP��ַ����
Return		: bool
Desc		: ��ȡ���ip��ַ��Ϣ�б�
******************************************************************/
bool CSocketCenter::GetLocalIPs(IPInfo* ips, int maxCnt, int* cnt)
{
	//1.��ʼ��wsa  
	WSADATA wsaData;
	int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (ret != 0)
	{
		return false;
	}
	//2.��ȡ������  
	char hostname[256];
	ret = gethostname(hostname, sizeof(hostname));
	if (ret == SOCKET_ERROR)
	{
		return false;
	}
	//3.��ȡ����ip  
	HOSTENT* host = gethostbyname(hostname);
	if (host == NULL)
	{
		return false;
	}
	//4.���ת��Ϊchar*����������  
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
Desc		: ����IPѰַ������
******************************************************************/
bool CSocketCenter::doServer() {
	int m_nPort = IP_PORT;
	const int MAX_BUF_LEN = 255;

	SOCKET sClient;
	sockaddr_in clientAddr, bindAddr;
	WSADATA wsdata;

	//����SOCKET�⣬�汾Ϊ2.0
	WORD    wVer = MAKEWORD(2, 0);
	if (0 != WSAStartup(wVer, &wsdata))
	{
		//AfxMessageBox(L"Not Support Socket2.0");
		return false;
	}

	//��UDP��ʼ���׽���
	sClient = socket(AF_INET, SOCK_DGRAM, 0);
	//���ø��׽���Ϊ�㲥���ͣ�
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

	//�ж��ip��ַ��ʱ����������
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
				//�ж��ĸ�ip��һ���ε�
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
			// ��������   
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