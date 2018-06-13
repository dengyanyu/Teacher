#include "stdafx.h"
#include "StuInfo.h"


CStuInfo::CStuInfo()
{
}


CStuInfo::~CStuInfo()
{
}


/******************************************************************
Function	: SendStuInfoReq
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:24
Parameter	: socket--连接学生机SOCKET 
Return		: void
Desc		: 向请求学生机登录信息
******************************************************************/
void CStuInfo::SendStuInfoReq(SOCKET socket)
{
	CSocketCenter mySocket;
	mySocket.SendReadyInfo(socket, STUDENTINFO);
}


/******************************************************************
Function	: GetStuInfo
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:25
Parameter	: socket--连接学生机SOCKET
Return		: void
Desc		: 获得学生的登录信息
******************************************************************/
void CStuInfo::GetStuInfo(SOCKET socket)
{
	STUINFODATA stuInfoData;
	memset(&stuInfoData, 0, sizeof(STUINFODATA));
	CSocketCenter mySocket;
	mySocket.RecvDataTCP(socket, (char*)&stuInfoData, sizeof(STUINFODATA));

	m_stuName.Format(_T("%s"), stuInfoData.m_stuName);

	m_stuID.Format(_T("%s"), stuInfoData.m_stuID);
}


/******************************************************************
Function	: GetStuName
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:25
Return		: CString
Desc		: 获得学生的登录名称
******************************************************************/
CString CStuInfo::GetStuName() const
{
	return m_stuName;
}


/******************************************************************
Function	: GetStuID
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:25
Return		: CString
Desc		: 获得学生的登录ID
******************************************************************/
CString CStuInfo::GetStuID() const
{
	return m_stuID;
}

