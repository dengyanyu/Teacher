#pragma once
#include "Common.h"

typedef struct tagIPInfo
{
	char ip[30];
}IPInfo;

class CSocketCenter
{
public:
	CSocketCenter();
	~CSocketCenter();
	void Clean();

	int SendDataTCP(SOCKET socket, const char* buf, int bytes);

	void OutputError();

	int RecvDataTCP(SOCKET socket, char* buf, int bytes);

	SOCKET SocketAccept(int port, int listenCount);

	bool InitSocketInfo(int port, int listenCount);

	SOCKET GetSocketInit();

	SOCKET InitMulticastSocket(int port, char* multicastIp, sockaddr_in& addr);
	
	int SendDataUDP(SOCKET socket, const char* sendBuf, int sendBytes, sockaddr_in addr);
	
	void SendReadyInfo(SOCKET socket, int msgID);

	//bool GetLocalIP(char* ip);
	bool GetLocalIPs(IPInfo* ips, int maxCnt, int* cnt);

	bool doServer();
private:
	SOCKET m_socketInit;
};

