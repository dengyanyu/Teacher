#include "stdafx.h"
#include "ScreenMonitorDlg.h"
#include "afxdialogex.h"
//#include "ItemData.h"

// CScreenDlg 对话框

IMPLEMENT_DYNAMIC(CScreenMonitorDlg, CDialogEx)

CScreenMonitorDlg::CScreenMonitorDlg(CWnd* pParent /*=NULL*/, int widthCount /*=0*/)
: CDialogEx(CScreenMonitorDlg::IDD, pParent)
, m_widthCount(widthCount)
, m_pBitMapInfo(NULL)
, m_pBmpCompressData(NULL)
{
}

CScreenMonitorDlg::~CScreenMonitorDlg()
{
	CleanData();
}

/******************************************************************
Function	: CleanData
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:47
Return		: void
Desc		: 清理
******************************************************************/
void CScreenMonitorDlg::CleanData()
{
	// 	if (m_screenSocket != INVALID_SOCKET)
	// 	{
	// 		closesocket(m_screenSocket);
	// 		m_screenSocket = NULL;
	// 	}
	DeletepBmpCompressData();
	DeletepBitMapInfo();
}


/******************************************************************
Function	: DeletepBitMapInfo
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:47
Return		: void
Desc		: 释放new 的内存
******************************************************************/
void CScreenMonitorDlg::DeletepBitMapInfo()
{
	if (m_pBitMapInfo != NULL)
	{
		LocalFree(m_pBitMapInfo);
		m_pBitMapInfo = NULL;
	}
}


/******************************************************************
Function	: DeletepBmpCompressData
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:48
Return		: void
Desc		: 释放new 的内存
******************************************************************/
void CScreenMonitorDlg::DeletepBmpCompressData()
{
	if (m_pBmpCompressData != NULL)
	{
		delete[] m_pBmpCompressData;
		m_pBmpCompressData = NULL;
	}
}

void CScreenMonitorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CScreenMonitorDlg, CDialogEx)
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	//	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()

// CScreenDlg 消息处理程序

void CScreenMonitorDlg::OnClose()
{
	// 发送消息给TeacherDlg 让它发送结束屏幕监控的消息给学生机
	::SendMessage(m_pParentWnd->m_hWnd, ID_MONITOREND, 0, 0);
	DestroyWindow();
	CDialogEx::OnClose();
}

// void CScreenMonitorDlg::SetScreenTranEnd(bool screenMonitorEnd)
// {
// 	m_isScreenMonitorEnd = screenMonitorEnd;
// }

BOOL CScreenMonitorDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO:  在此添加专用代码和/或调用基类
	// 屏蔽ESC 与ENTER 键
	if (WM_KEYDOWN == pMsg->message)
	{
		int nVirtKey = (int)pMsg->wParam;
		if (nVirtKey == VK_RETURN || nVirtKey == VK_ESCAPE)
		{
			return TRUE;
		}
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

void CScreenMonitorDlg::OnSize(UINT nType, int cx, int cy)
{
	//	CDialogEx::OnSize(nType, cx, cy);
	CDialogEx::OnSize(nType, cx, cy);

	//	GetClientRect(&m_rect);
	this->GetClientRect(&m_rect);
	m_blockWidth = m_rect.Width() / m_widthCount;
	m_blockHeight = m_rect.Height() / m_widthCount;
}

// void CScreenMonitorDlg::SetScreenData() //显示图像
// {
//
// }


/******************************************************************
Function	: ShowBmp
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:48
Parameter	: bmp--结构体，里边保存了学生机桌面截图的数据
Parameter	: itemOrder--学生机的顺序，取值为0-15而且是连续的
Return		: void
Desc		: 显示学生机的桌面
******************************************************************/
void CScreenMonitorDlg::ShowBmp(BMP* bmp, int itemOrder)
{
	m_pBmpCompressData = UnCompressData(bmp->pBMPINFO->bmiHeader.biSizeImage,
		bmp->bmpCompressSize, bmp->pBmpCompressData);

	int xDest = 0;
	int yDest = 0;
	DetermineShowRegion(itemOrder, xDest, yDest);

	HDC hdc = GetDC()->m_hDC;
	if (hdc != NULL)
	{
		::SetStretchBltMode(hdc, STRETCH_HALFTONE); //解决失真问题
		::StretchDIBits(hdc,
			xDest,
			yDest,
			m_blockWidth,
			m_blockHeight,
			0,
			0,
			bmp->pBMPINFO->bmiHeader.biWidth,
			bmp->pBMPINFO->bmiHeader.biHeight,
			m_pBmpCompressData, //位图数据
			bmp->pBMPINFO, //BITMAPINFO 位图信息头
			DIB_RGB_COLORS,
			SRCCOPY
			);
	}

	// 	for (uLongf i = 0; i < bmp->bmpCompressSize; i++)
	// 	{
	// 		delete bmp->pBmpCompressData[i];
	// 	}
	// 	BYTE* temp1 = bmp->pBmpCompressData;
	// 	delete[] temp1;
	// 	temp1 = NULL;
	//
	// 	BITMAPINFO* temp2 = bmp->pBMPINFO;
	// 	LocalFree(temp2);
	// 	temp2 = NULL;
	// 	m_pBitMapInfo = (BITMAPINFO*)LocalAlloc(LPTR, bmp->pBMPINFO->bmiHeader.biSize);
	// 	memcpy_s(m_pBitMapInfo, bmp->pBMPINFO->bmiHeader.biSize,
	// 		bmp->pBMPINFO, bmp->pBMPINFO->bmiHeader.biSize);
	//	std::unique_lock<std::mutex> lock(m_mtx);
	// 	if (false == isLButtonUp)
	// 	{
	//		CleanData();
	// 	}
	//	lock.unlock();

	delete[] m_pBmpCompressData;
	m_pBmpCompressData = NULL;
}

/******************************************************************
Function	: DetermineShowRegion
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:49
Parameter	: itemOrder--学生机在对话框中显示的顺序取值为0-15
Parameter	: xDest--显示在对话框的左上角的x坐标
Parameter	: yDest--显示在对话框的左上角的y坐标
Return		: void
Desc		: 分配每个学生机显示屏幕的位置
******************************************************************/
void CScreenMonitorDlg::DetermineShowRegion(int itemOrder, int &xDest, int &yDest)
{
	// 	int blockWidth = m_rect.Width() / m_widthCount;
	// 	int blockHeight = m_rect.Height() / m_widthCount;
	switch (m_widthCount)
	{
	case 1:
		OneRegion(itemOrder, xDest, yDest);
		break;
	case 2:
		FourRegion(itemOrder, xDest, yDest);
		break;
	case 3:
		NineRegion(itemOrder, xDest, yDest);
		break;
	case 4:
		SixteenRegion(itemOrder, xDest, yDest);
		break;
	}
}


/******************************************************************
Function	: SixteenRegion
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 13:30
Parameter	: itemOrder--学生机在对话框中显示的顺序取值为0-15
Parameter	: xDest--显示在对话框的左上角的x坐标
Parameter	: yDest--显示在对话框的左上角的y坐标
Return		: void
Desc		: 同时监控的学生机的数量少于或等于16台
******************************************************************/
void CScreenMonitorDlg::SixteenRegion(int itemOrder, int &xDest, int &yDest)
{
	NineRegion(itemOrder, xDest, yDest);
	if (3 * m_widthCount <= itemOrder
		&& itemOrder < 4 * m_widthCount)
	{
		xDest = (itemOrder - 3 * m_widthCount) * m_blockWidth;
		yDest = 3 * m_blockHeight;
	}
}


/******************************************************************
Function	: NineRegion
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 13:31
Parameter	: itemOrder--学生机在对话框中显示的顺序取值为0-8
Parameter	: xDest--显示在对话框的左上角的x坐标
Parameter	: yDest--显示在对话框的左上角的y坐标
Return		: void
Desc		: 同时监控的学生机的数量少于或等于9台
******************************************************************/
void CScreenMonitorDlg::NineRegion(int itemOrder, int &xDest, int &yDest)
{
	FourRegion(itemOrder, xDest, yDest);
	if (2 * m_widthCount <= itemOrder
		&& itemOrder < 3 * m_widthCount)
	{
		xDest = (itemOrder - 2 * m_widthCount) * m_blockWidth;
		yDest = 2 * m_blockHeight;
	}
}


/******************************************************************
Function	: FourRegion
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 13:32
Parameter	: itemOrder--学生机在对话框中显示的顺序取值为0-3
Parameter	: xDest--显示在对话框的左上角的y坐标
Parameter	: yDest--显示在对话框的左上角的x坐标
Return		: void
Desc		: 同时监控的学生机的数量少于或等于4台
******************************************************************/
void CScreenMonitorDlg::FourRegion(int itemOrder, int &xDest, int &yDest)
{
	OneRegion(itemOrder, xDest, yDest);
	if (m_widthCount <= itemOrder
		&& itemOrder < 2 * m_widthCount)
	{
		xDest = (itemOrder - m_widthCount) * m_blockWidth;
		yDest = m_blockHeight;
	}
}


/******************************************************************
Function	: OneRegion
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 13:32
Parameter	: itemOrder--学生机在对话框中显示的顺序取值为0
Parameter	: xDest--显示在对话框的左上角的x坐标
Parameter	: yDest--显示在对话框的左上角的y坐标
Return		: void
Desc		: 同时监控的学生机的数量等于1台
******************************************************************/
void CScreenMonitorDlg::OneRegion(int itemOrder, int &xDest, int &yDest)
{
	if (itemOrder < m_widthCount)
	{
		xDest = itemOrder * m_blockWidth;
		yDest = 0;
	}
}


/******************************************************************
Function	: UnCompressData
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 13:33
Parameter	: biSizeImage--压缩前的图像大小
Parameter	: bmpCompressSize--压缩后的图像大小
Parameter	: pBmpCompressData--压缩的图像数据
Return		: BYTE*
Desc		: 解压从学生机接收到的屏幕图像数据
******************************************************************/
BYTE* CScreenMonitorDlg::UnCompressData(uLongf biSizeImage, uLongf bmpCompressSize, BYTE* pBmpCompressData)
{
	uLongf unCompressDataLen = (uLongf)((biSizeImage + 12)*(100.1 / 100)) + 1;
	BYTE* pUnCompressData = new BYTE[unCompressDataLen];
	int err = uncompress(pUnCompressData, &unCompressDataLen,
		pBmpCompressData, bmpCompressSize);
	if (err != Z_OK)
	{
		CString str;
		str.Format(_T("uncompess error: %d"), err);
		MessageBox(str);
		exit(1);
	}

	BYTE* bmpShowData = new BYTE[unCompressDataLen];
	memcpy(bmpShowData, pUnCompressData, unCompressDataLen);

	delete[] pUnCompressData;
	pUnCompressData = NULL;

	return bmpShowData;
}

// BYTE* CScreenMonitorDlg::UnCompressData(uLongf biSizeImage, uLongf bmpCompressSize)
// {
// 	uLongf unCompressDataLen = (uLongf)((biSizeImage + 12)*(100.1 / 100)) + 1;
// 	BYTE* pUnCompressData = new BYTE[unCompressDataLen];
// 	int err = uncompress(pUnCompressData, &unCompressDataLen,
// 		m_pBmpCompressData, bmpCompressSize);
// 	if (err != Z_OK)
// 	{
// 		CString str;
// 		str.Format(_T("uncompess error: %d"), err);
// 		MessageBox(str);
// 		exit(1);
// 	}
// 
// 	BYTE* bmpShowData = new BYTE[unCompressDataLen];
// 	memcpy(bmpShowData, pUnCompressData, unCompressDataLen);
// 
// 	delete[] pUnCompressData;
// 	pUnCompressData = NULL;
// 
// 	return bmpShowData;
// }

BOOL CScreenMonitorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	return TRUE;  // return TRUE unless you set the focus to a control
}

void CScreenMonitorDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	if (nSBCode != SB_ENDSCROLL)
	{
		SCROLLINFO hStructure;
		GetScrollInfo(SB_HORZ, &hStructure);
		hStructure.nPos = nPos;
		SetScrollInfo(SB_HORZ, &hStructure);
	}

	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CScreenMonitorDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	if (nSBCode != SB_ENDSCROLL)
	{
		SCROLLINFO vStructure;
		GetScrollInfo(SB_VERT, &vStructure);
		vStructure.nPos = nPos;
		SetScrollInfo(SB_VERT, &vStructure);
	}

	CDialogEx::OnVScroll(nSBCode, nPos, pScrollBar);
}


/******************************************************************
Function	: FourRegionClk
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 13:33
Parameter	: point--鼠标坐标
Parameter	: itemOrder--学生机显示的顺序0-3
Return		: void
Desc		: 同时监控的学生机的数量少于等于4台，判断学生机点击的是哪个学生机
******************************************************************/
void CScreenMonitorDlg::FourRegionClk(CPoint point, int& itemOrder)
{
	if (0 < point.x && point.x < m_blockWidth)
	{
		FourRegionClk(point, itemOrder, 0);
	}
	else if (m_blockWidth < point.x && point.x < 2 * m_blockWidth)
	{
		FourRegionClk(point, itemOrder, 1);
	}
}


/******************************************************************
Function	: NineRegionClk
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 13:34
Parameter	: point--鼠标坐标
Parameter	: itemOrder--学生机显示的顺序0-8
Return		: void
Desc		: 同时监控的学生机的数量少于等于9台，判断点击的是哪个学生机
******************************************************************/
void CScreenMonitorDlg::NineRegionClk(CPoint point, int& itemOrder)
{
	if (0 < point.x && point.x < m_blockWidth)
	{
		NineRegionClk(point, itemOrder, 0);
	}
	else if (m_blockWidth < point.x && point.x < 2 * m_blockWidth)
	{
		NineRegionClk(point, itemOrder, 1);
	}
	else if (2 * m_blockWidth < point.x && point.x < 3 * m_blockWidth)
	{
		NineRegionClk(point, itemOrder, 2);
	}
}


/******************************************************************
Function	: SixteenRegionClk
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 13:34
Parameter	: point--鼠标坐标
Parameter	: itemOrder--学生机显示的顺序0-15
Return		: void
Desc		: 同时监控的学生机的数量少于等于16台，判断点击的是哪个学生机
******************************************************************/
void CScreenMonitorDlg::SixteenRegionClk(CPoint point, int& itemOrder)
{
	if (0 < point.x && point.x < m_blockWidth)
	{
		SixteenRegionClk(point, itemOrder, 0);
	}
	else if (m_blockWidth < point.x && point.x < 2 * m_blockWidth)
	{
		SixteenRegionClk(point, itemOrder, 1);
	}
	else if (2 * m_blockWidth < point.x && point.x < 3 * m_blockWidth)
	{
		SixteenRegionClk(point, itemOrder, 2);
	}
	else if (3 * m_blockWidth < point.x && point.y < 4 * m_blockWidth)
	{
		SixteenRegionClk(point, itemOrder, 3);
	}
}


/******************************************************************
Function	: FourRegionClk
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 13:35
Parameter	: point--鼠标点击的位置
Parameter	: itemOrder--学生机在对话框上显示的顺序
Parameter	: offset--偏移第一列的位置取值为0或1
Return		: void
Desc		: 当offset = 0，点击的学生机是0或2，当offset = 1 点击的学生机是1 或3
******************************************************************/
void CScreenMonitorDlg::FourRegionClk(CPoint point, int& itemOrder, int offset)
{
	if (0 < point.y && point.y < m_blockHeight)
	{
		itemOrder = 0 * m_widthCount + offset;
	}
	else if (m_blockHeight < point.y && point.y < 2 * m_blockHeight)
	{
		itemOrder = 1 * m_widthCount + offset;
	}
}


/******************************************************************
Function	: NineRegionClk
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 13:35
Parameter	: point--鼠标点击的位置
Parameter	: itemOrder--学生机在对话框上显示的顺序
Parameter	: offset--偏移第一列的位置取值为0、1或2
Return		: void
Desc		: 当offset = 0 点击的学生机是0、3或6，
			  当offset = 1 点击的学生机是1、4或7
			  当offset = 2 点击的学生机是2、5或8
******************************************************************/
void CScreenMonitorDlg::NineRegionClk(CPoint point, int& itemOrder, int offset)
{
	FourRegionClk(point, itemOrder, offset);
	if (2 * m_blockHeight < point.y && point.y < 3 * m_blockHeight)
	{
		itemOrder = 2 * m_widthCount + offset;
	}
}


/******************************************************************
Function	: SixteenRegionClk
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 13:36
Parameter	: point--鼠标点击的位置
Parameter	: itemOrder--学生机在对话框上显示的顺序
Parameter	: offset--偏移第一列的位置取值为0、1或2
Return		: void
Desc		: 当offset = 0 点击的学生机是0、4、8或12，
			  当offset = 1 点击的学生机是1、5、9或13
			  当offset = 2 点击的学生机是2、6、10或14
			  当offset = 3 点击的学生机是3、7、11或15
******************************************************************/
void CScreenMonitorDlg::SixteenRegionClk(CPoint point, int& itemOrder, int offset)
{
	NineRegionClk(point, itemOrder, offset);
	if (3 * m_blockHeight < point.y && 4 * m_blockHeight)
	{
		itemOrder = 3 * m_widthCount + offset;
	}
}


/******************************************************************
Function	: OnLButtonDblClk
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 13:36
Parameter	: nFlags
Parameter	: point
Return		: void
Desc		: 双击消息，被双击学生机实现1对1屏幕监控
******************************************************************/
void CScreenMonitorDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	int itemOrder = 0;
	switch (m_widthCount)
	{
	case 1:
		break;
	case 2:
		FourRegionClk(point, itemOrder);
		break;
	case 3:
		NineRegionClk(point, itemOrder);
		break;
	case 4:
		SixteenRegionClk(point, itemOrder);
		break;
	default:
		break;
	}
	if (m_widthCount != 1)
	{
		// 发送消息给TeacherDlg 对话框，让它处理后续的事情
		::SendMessage(m_pParentWnd->m_hWnd, ID_ONESTUINSCREEN, 0, (LPARAM)&itemOrder);
	}
	CDialogEx::OnLButtonDblClk(nFlags, point);
}