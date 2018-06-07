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
	// 学生姓名
	CString m_stuName;
	// 学生的学号
	CString m_stuID;

};

