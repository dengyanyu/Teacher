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
Parameter	: socket--����ѧ����SOCKET 
Return		: void
Desc		: ������ѧ������¼��Ϣ
******************************************************************/
void CStuInfo::SendStuInfoReq(SOCKET socket)
{
	CSocketCenter mySocket;
	mySocket.SendReadyInfo(socket, STUDENTINFO);
}

/******************************************************************
Function	: GetStuInfo
Parameter	: socket--����ѧ����SOCKET
Return		: void
Desc		: ���ѧ���ĵ�¼��Ϣ
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
Parameter	: ��
Return		: CString
Desc		: ���ѧ���ĵ�¼����
******************************************************************/
CString CStuInfo::GetStuName() const
{
	return m_stuName;
}


/******************************************************************
Function	: GetStuID
Parameter	: ��
Return		: CString
Desc		: ���ѧ���ĵ�¼ID
******************************************************************/
CString CStuInfo::GetStuID() const
{
	return m_stuID;
}

