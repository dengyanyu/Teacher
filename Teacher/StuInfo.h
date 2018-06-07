#pragma once
#include "SocketCenter.h"
#include "Common.h"
class CStuInfo
{
public:
	CStuInfo();

	~CStuInfo();
	void SendStuInfoReq(SOCKET socket);

	void GetStuInfo(SOCKET socket);

	CString GetStuName() const;

	CString GetStuID() const;
private:
	// ѧ������
	CString m_stuName;
	// ѧ����ѧ��
	CString m_stuID;

};

