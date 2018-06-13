#include "stdafx.h"
#include "Multicast.h"
#include <io.h>
#include <fcntl.h>
//////////////////////////////////////////////////////////////////////////
// �ڴ�й¶�����Ҫ��ӵ�ͷ�ļ�
// #define _CRTDBG_MAP_ALLOC
// #include <stdlib.h>
// #include <crtdbg.h>
//////////////////////////////////////////////////////////////////////////
CMulticast::CMulticast()
	: m_pBmpTransData(NULL)
	, m_pBitMapInfo(NULL)
	, m_isFirst(false)
	, m_isMulticastEnd(false)
	, m_isInitBITMAPINFO(true)
	, m_socketMulticast(INVALID_SOCKET)
{
}

CMulticast::~CMulticast()
{
	CleanData();
}

/******************************************************************
Function	: CleanData
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:05
Return		: void
Desc		: �ͷ���Դ
******************************************************************/
void CMulticast::CleanData()
{
	DeletepBitMapInfo();

	DeletepBmpTransData();

	CloseSocketMulticast();
}


/******************************************************************
Function	: CloseSocketMulticast
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:05
Return		: void
Desc		: �ͷ�SOCKET��Դ
******************************************************************/
void CMulticast::CloseSocketMulticast()
{
	if (m_socketMulticast != INVALID_SOCKET)
	{
		closesocket(m_socketMulticast);
		m_socketMulticast = INVALID_SOCKET;
	}
}


/******************************************************************
Function	: DeletepBmpTransData
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:05
Return		: void
Desc		: �ͷ�new ���ڴ�,��ֹ�ڴ�й©
******************************************************************/
void CMulticast::DeletepBmpTransData()
{
	if (m_pBmpTransData != NULL)
	{
		delete[] m_pBmpTransData;
		m_pBmpTransData = NULL;
	}
}


/******************************************************************
Function	: DeletepBitMapInfo
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:04
Return		: void
Desc		: �ͷ�new ���ڴ�,��ֹ�ڴ�й©
******************************************************************/
void CMulticast::DeletepBitMapInfo()
{
	if (m_pBitMapInfo != NULL)
	{
		LocalFree(m_pBitMapInfo);
		m_pBitMapInfo = NULL;
	}
}


/******************************************************************
Function	: GetDeskScreeData
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:04
Return		: void
Desc		: ��������ͼ�����ݲ�����ѹ������
******************************************************************/
void CMulticast::GetDeskScreeData()
{
	CDC memDC; //����һ���ڴ滭��
	CBitmap bmp;
	BITMAP bitmap;
	GetDeskScreenDC(memDC, bmp, bitmap);
	SetCursorIcon(memDC);
	// �����ͼ��ԭʼ��������
	BYTE* pBmpOriginalData = new BYTE[m_imgTotalSize];
	// ��ͼ������ݿ�����pBmpOriginalData ��
	if (::GetDIBits(memDC.m_hDC, bmp, 0, bitmap.bmHeight,
		pBmpOriginalData, m_pBitMapInfo, DIB_RGB_COLORS) == 0)
	{
		AfxMessageBox(_T("GetDIBits Error"));
		delete[] pBmpOriginalData;
		pBmpOriginalData = NULL;
		LocalFree(m_pBitMapInfo);
		m_pBitMapInfo = NULL;
		return;
	}

	// ѹ����ͼ����
	CompressBmpData(pBmpOriginalData);

	delete[] pBmpOriginalData;
	pBmpOriginalData = NULL;
	DeleteDC(memDC);
	DeleteObject(bmp);
	//	_CrtDumpMemoryLeaks();
}


/******************************************************************
Function	: SetCursorIcon
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:04
Parameter	: memDC--CDC��
Return		: void
Desc		: ��������Ϣ�����Ƶ�����ͼ����
******************************************************************/
void CMulticast::SetCursorIcon(CDC &memDC)
{
	//��ȡ��ǰ��꼰��λ��
	HCURSOR hCursor = GetCursor();
	POINT ptCursor;
	GetCursorPos(&ptCursor);
	//��ȡ����ͼ������
	ICONINFO IconInfo;
	if (GetIconInfo(hCursor, &IconInfo))
	{
		ptCursor.x -= ((int)IconInfo.xHotspot);
		ptCursor.y -= ((int)IconInfo.yHotspot);
		if (IconInfo.hbmMask != NULL)
			DeleteObject(IconInfo.hbmMask);
		if (IconInfo.hbmColor != NULL)
			DeleteObject(IconInfo.hbmColor);
	}
	//�ڼ����豸�������ϻ����ù��
	DrawIconEx(
		memDC.m_hDC,         // handle to device context
		ptCursor.x, ptCursor.y,
		hCursor,         // handle to icon to draw
		0, 0,          // width of the icon
		0,           // index of frame in animated cursor
		NULL,          // handle to background brush
		DI_NORMAL | DI_COMPAT      // icon-drawing flags
		);
}


/******************************************************************
Function	: GetDeskScreenDC
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:04
Parameter	: memDC--CDC��
Parameter	: bmp--λͼ
Parameter	: bitmap--λͼ�ṹ��
Return		: void
Desc		: �������ͼ��
******************************************************************/
void CMulticast::GetDeskScreenDC(CDC &memDC, CBitmap &bmp, BITMAP& bitmap)
{
	CDC* pDeskDC = CWnd::GetDesktopWindow()->GetDC(); //��ȡ���滭������
	int width = GetSystemMetrics(SM_CXSCREEN); //��ȡ��Ļ�Ŀ��
	int height = GetSystemMetrics(SM_CYSCREEN); //��ȡ��Ļ�ĸ߶�
	memDC.CreateCompatibleDC(pDeskDC); //����һ�����ݵĻ���
	bmp.CreateCompatibleBitmap(pDeskDC, width, height); //��������λͼ
	memDC.SelectObject(&bmp); //ѡ��λͼ����
	bmp.GetBitmap(&bitmap);

	memDC.BitBlt(0, 0, width, height, pDeskDC, 0, 0, SRCCOPY);

	InitBITMAPINFO(bitmap, height, width);
	pDeskDC->DeleteDC();
}


/******************************************************************
Function	: CompressBmpData
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:03
Parameter	: pBmpOriginalData--����ͼ�������
Return		: void
Desc		: ʹ��ZLIB ��ѹ������ͼ������
******************************************************************/
void CMulticast::CompressBmpData(BYTE* pBmpOriginalData)
{
	BYTE* pCompressData = NULL;
	//	m_compressBmpDataLen = 0;
	// ��Ҫһ���㹻��Ŀռ�
	m_compressBmpDataLen = (uLongf)((m_imgTotalSize + 12)*(100.1 / 100)) + 1;

	pCompressData = new BYTE[m_compressBmpDataLen];
	// �����ݽ���ѹ�������浽pCompressData ��
	int err = compress(pCompressData, &m_compressBmpDataLen, pBmpOriginalData, m_imgTotalSize);

	if (err != Z_OK) {
		// 		InitConsoleWindow();
		// 		printf("compess error: %d", err);
		exit(1);
	}

	if (m_pBmpTransData != NULL)
	{
		delete[] m_pBmpTransData;
		m_pBmpTransData = NULL;
	}
	// ��ѹ��������ݱ��浽m_pBmpTransData ��
	m_pBmpTransData = new BYTE[m_compressBmpDataLen];
	memcpy(m_pBmpTransData, pCompressData, m_compressBmpDataLen);
	delete[] pCompressData;
	pCompressData = NULL;
}


/******************************************************************
Function	: InitBITMAPINFO
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:03
Parameter	: bitmap--ͼ��Ĵ�С����Ϣ
Parameter	: height--����ĸ�
Parameter	: width--����Ŀ�
Return		: void
Desc		: ��ʼ��ͼ�����Ϣͷ�ṹ�� BITMAPINFO
******************************************************************/
void CMulticast::InitBITMAPINFO(BITMAP &bitmap, int height, int width)
{
	m_imgTotalSize = bitmap.bmWidthBytes * bitmap.bmHeight;
	double paletteSize = 0; //��¼��ɫ���С
	if (bitmap.bmBitsPixel < 16) //�ж��Ƿ�Ϊ���ɫλͼ
	{
		//paletteSize = pow(2.0, (double)bitmap.bmBitsPixel*sizeof(RGBQUAD));
		paletteSize = (1 << bitmap.bmBitsPixel)*sizeof(RGBQUAD);
	}
	m_bmpHeadTotalSize = (int)paletteSize + sizeof(BITMAPINFO);

	m_pBitMapInfo = (BITMAPINFO*)LocalAlloc(LPTR, m_bmpHeadTotalSize);
	m_pBitMapInfo->bmiHeader.biBitCount = bitmap.bmBitsPixel;
	m_pBitMapInfo->bmiHeader.biClrImportant = 0;
	m_pBitMapInfo->bmiHeader.biCompression = 0;
	m_pBitMapInfo->bmiHeader.biHeight = height;
	m_pBitMapInfo->bmiHeader.biPlanes = bitmap.bmPlanes;
	m_pBitMapInfo->bmiHeader.biSize = m_bmpHeadTotalSize;//sizeof(BITMAPINFO);
	m_pBitMapInfo->bmiHeader.biSizeImage = m_imgTotalSize;
	m_pBitMapInfo->bmiHeader.biWidth = width;
	m_pBitMapInfo->bmiHeader.biXPelsPerMeter = 0;
	m_pBitMapInfo->bmiHeader.biYPelsPerMeter = 0;
}


/******************************************************************
Function	: SendScreenBmpData
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:02
Parameter	: multicastSocket--�㲥SOCKET
Parameter	: addr--SOCKADDR_IN�鲥������������ʱ��Ҫʹ��
Return		: void
Desc		: ������Ļͼ�����ݵ�ѧ����
******************************************************************/
void CMulticast::SendScreenBmpData(SOCKET multicastSocket, SOCKADDR_IN addr)
{
	MULTICASTDATA multicastData;
	int count = InitBmpData(multicastData);

	for (int i = 0; i < count; i++)
	{
		SetBmpData(multicastData, i, count);

		m_mySocket.SendDataUDP(multicastSocket, (char*)&multicastData,
			sizeof(MULTICASTDATA), addr);
	}
}


/******************************************************************
Function	: SetBmpData
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:02
Parameter	: multicastData--��Ҫ����ֵ��MULTICASTDATA �ṹ�� 
Parameter	: i--������ͼ��ĵڼ��δ���
Parameter	: count--������ͼ����Ҫ������Ĵ���
Return		: void
Desc		: ������Ļͼ�����ݵ�ѧ����
******************************************************************/
void CMulticast::SetBmpData(MULTICASTDATA &multicastData, int i, int count)
{
	memset(multicastData.transData, 0, MULTICAST_TRANS_SIZE);

	UINT beginPos = i * MULTICAST_TRANS_SIZE;
	multicastData.beginPos = beginPos;
	multicastData.ID = i;
	if (i == count - 1) // ���һ�η�������
	{
		multicastData.isShow = true;
		multicastData.infoType = 2;
		memcpy_s(multicastData.transData, MULTICAST_TRANS_SIZE,
			m_pBmpTransData + beginPos, m_compressBmpDataLen - beginPos);
		m_isFirst = false;
	}
	else
	{
		memcpy_s(multicastData.transData, MULTICAST_TRANS_SIZE,
			m_pBmpTransData + beginPos, MULTICAST_TRANS_SIZE);
	}
}


/******************************************************************
Function	: InitBmpData
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:00
Parameter	: multicastData--����ͼ�����ݵĽṹ��
Return		: int--һ������ͼ����Ҫ������Ĵ���
Desc		: Ϊ����ͼ��ṹ��BMPDATA��ʼ��һЩ��Ҫ��Ϣ
******************************************************************/
int CMulticast::InitBmpData(MULTICASTDATA &multicastData)
{
	memset(&multicastData, 0, sizeof(MULTICASTDATA));
	//ͼ����Ϣͷ
	memcpy(&multicastData.bmpHeadInfo, m_pBitMapInfo, m_bmpHeadTotalSize);
	multicastData.bmpCompressSize = m_compressBmpDataLen;
	multicastData.isShow = false;
	// ����ͼ������
	int count = int(ceil(double(m_compressBmpDataLen) / MULTICAST_TRANS_SIZE));

	//		multicastData.isShow = false;
	//	UINT beginPos;
	multicastData.infoType = 1;
	return count;
}


/******************************************************************
Function	: SendScreenData
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018/06/13
Return		: void
Desc		: ���Ϸ������ݵ�ѧ����
******************************************************************/
void CMulticast::SendScreenData()
{
	SOCKADDR_IN addr;
	SOCKET socketMulticast = m_mySocket.InitMulticastSocket(MULTICAST_TRANS_PORT,
		MULTICAST_IP, addr);

	while (false == m_isMulticastEnd)
	{
		GetDeskScreeData();
		//		SendBmpHeaderInfo(socketMsg);
		SendScreenBmpData(socketMulticast, addr);

		Sleep(5);
		//AfxMessageBox(_T("1"));
	}
}


/******************************************************************
Function	: SetIsMulticastEnd
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018/06/13
Parameter	: IsMulticastEnd--true�����̣߳�falseֹͣ�߳�
Return		: void
Desc		: ����ͼ�����ݵ�ѧ�������߳��Ƿ�����ı�־
******************************************************************/
void CMulticast::SetIsMulticastEnd(bool IsMulticastEnd)
{
	m_isMulticastEnd = IsMulticastEnd;
}