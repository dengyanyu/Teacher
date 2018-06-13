#include "stdafx.h"
#include "Teacher.h"
#include "TeacherDlg.h"
#include "afxdialogex.h"
#include "ItemData.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTeacherDlg 对话框



CTeacherDlg::CTeacherDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CTeacherDlg::IDD, pParent)
	, m_socketMsg(INVALID_SOCKET)
	, m_pScreenDlg(NULL)
	, m_oneStuScreenDlg(NULL)
	, m_pMulticast(NULL)
	, m_isStopMulticast(false)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	memset(m_item, -1, sizeof(int)*MAX_MONITOR);
}

void CTeacherDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list);
}

BEGIN_MESSAGE_MAP(CTeacherDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(ID_STUDENTLONGIN, OnAddStudent)
	ON_MESSAGE(ID_STUDENTLOGINOUT, OnLoginOutStudent)
	ON_MESSAGE(ID_SETBMPDATA, OnSetBmpCompressData)
	ON_MESSAGE(ID_MONITOREND, OnEndMonitor)
	ON_MESSAGE(ID_ONESTUINSCREEN, OnCreatOneStudentDlg)
	ON_MESSAGE(ID_SETONESTUBMPDATA, OnSetOneStuBmpCompressData)
	ON_MESSAGE(ID_ONESTUINSCREENEND, OnEndOneStudentMonitor)
	ON_COMMAND(ID_MULTICAST, OnMulticast)
	ON_COMMAND(ID_MULTICAST_STOP, OnMulticastStop)
	ON_COMMAND(ID_SCREENMONITOR, OnBeginScreenMonitor)
ON_WM_DESTROY()
ON_NOTIFY(NM_CLICK, IDC_LIST1, &CTeacherDlg::OnNMClickList1)
ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CTeacherDlg::OnNMDblclkList1)
END_MESSAGE_MAP()


// CTeacherDlg 消息处理程序

BOOL CTeacherDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码
	SetMainDlgSytle();
	// 创建一个线程用于监听服务端上线SOCKET
	::CloseHandle(CreateThread(0, 0, OnIPListen, (LPVOID)this, 0, NULL));
	::CloseHandle(CreateThread(0, 0, OnMsgListen, (LPVOID)this, 0, NULL));

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}


void CTeacherDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	CDialogEx::OnSysCommand(nID, lParam);
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。
void CTeacherDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CTeacherDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


/******************************************************************
Function	: OnMsgListen
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:07
Parameter	: self--CTeacherDlg类
Return		: DWORD WINAPI
Desc		: 监听学生机登录线程的运行方法
******************************************************************/
DWORD WINAPI CTeacherDlg::OnMsgListen(LPVOID self)
{
	CTeacherDlg* teacherDlg = (CTeacherDlg*)self;
	teacherDlg->ListenStudentLogin();
	return 0;
}

DWORD WINAPI CTeacherDlg::OnIPListen(LPVOID self)
{
	CTeacherDlg* teacherDlg = (CTeacherDlg*)self;
	teacherDlg->ListenIPGet();
	return 0;
}

bool CTeacherDlg::ListenIPGet()
{
	CSocketCenter mySocket;
	mySocket.doServer();
	return true;
}


/******************************************************************
Function	: ListenStudentLogin
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:07
Return		: bool
Desc		: 监听学生登录并产生唯一的ID
******************************************************************/
bool CTeacherDlg::ListenStudentLogin()
{
	CSocketCenter mySocket;
	mySocket.InitSocketInfo(LISTEN_PORT, MAX_LISTEN);
	//	SOCKET m_servSocket;
	SOCKADDR_IN Seraddr;
	int Seraddrsize = sizeof(Seraddr);

	while (true)
	{
		m_socketMsg = accept(mySocket.GetSocketInit(), (sockaddr *)&Seraddr, &Seraddrsize);
		if (m_socketMsg != INVALID_SOCKET)
		{
			SetLoginStudentID();
		}
	}
	return true;
}


/******************************************************************
Function	: SetLoginStudentID
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:07
Return		: void
Desc		: 给登录学生机分配唯一的ID
******************************************************************/
void CTeacherDlg::SetLoginStudentID()
{
	int id;
	CItemData *itemData = NULL;
	int Count = m_list.GetItemCount();
	if (Count == 0)
	{
		id = 0;
	}
	else
	{
		id = 0;
		for (int i = 0; i < Count; i++)
		{
			itemData = (CItemData*)m_list.GetItemData(i);
			if (itemData->m_id == id)
			{
				i = 0;
				id = id + 1;
				continue;
			}
		}
	}
	itemData = new CItemData(id, m_hWnd, m_socketMsg);
	// 监听消息的线程运行
	itemData->Run();
}


/******************************************************************
Function	: SetMainDlgSytle
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:08
Return		: void
Desc		: 设置教师机的主界面
******************************************************************/
void CTeacherDlg::SetMainDlgSytle()
{
	LoadBitBmp();

	CreatToolBar();

	CreateStatusBar();

	InsertColumn();

	GetClientRect(&m_rect);
}



/******************************************************************
Function	: InsertColumn
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:08
Return		: void
Desc		: 为ListControl 插入列头
******************************************************************/
void CTeacherDlg::InsertColumn()
{
	// 设置List Control 风格为整行选中，网格，带有复选框（控件的View属性值要设置为Report）
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	// 给List Control 添加列
	m_list.InsertColumn(0, _T("姓名"), LVCFMT_CENTER, 150);
	m_list.InsertColumn(1, _T("姓名"), LVCFMT_CENTER, 150);
	m_list.InsertColumn(2, _T("学号"), LVCFMT_CENTER, 150);
	m_list.DeleteColumn(0);
}


/******************************************************************
Function	: CreateStatusBar
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:08
Return		: void
Desc		: 创建状态栏
******************************************************************/
void CTeacherDlg::CreateStatusBar()
{
	// 底部状态栏的创建
	BOOL hbar = m_statusBar.Create(this);
	// 状态栏的ID
	CRect ct;
	GetClientRect(&ct);
	UINT b[2] = { 1009, 1010 };
	hbar = m_statusBar.SetIndicators(b, 2);
	m_statusBar.SetPaneInfo(0, b[0], SBPS_NORMAL, (int)(ct.Width() * 0.5));
	m_statusBar.SetPaneInfo(1, b[1], SBPS_NORMAL, (int)(ct.Width() * 0.5));
	m_statusBar.SetPaneText(0, _T("作者：石蕴金"));
	m_statusBar.SetPaneText(1, _T("在线主机：0 台"));
	// 显示工具条与状态条
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
}


/******************************************************************
Function	: CreatToolBar
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:08
Return		: void
Desc		: 创建工具栏
******************************************************************/
void CTeacherDlg::CreatToolBar()
{
	// 设置每一个工具栏的ID，之后要添加消息响应
	const UINT t[3] = { 1001, 1003, 1002 };
	m_toolBar.CreateEx(this);
	m_toolBar.SetButtons(t, 3);
	m_toolBar.SetSizes(CSize(60, 56), CSize(24, 24));
	// 工具栏添加文字
	m_toolBar.SetButtonText(0, _T("开始投影"));
	m_toolBar.SetButtonText(1, _T("停止投影"));
	m_toolBar.SetButtonText(2, _T("屏幕监控"));

	// 工具栏添加图片
	m_toolBar.GetToolBarCtrl().SetImageList(&m_imagelist);
	m_toolBar.GetToolBarCtrl().EnableButton(ID_MULTICAST_STOP, false);
}


/******************************************************************
Function	: LoadBitBmp
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:08
Return		: void
Desc		: 加载位图资源
******************************************************************/
void CTeacherDlg::LoadBitBmp()
{
	m_imagelist.Create(32, 32, ILC_COLOR24 | ILC_MASK, 1, 1);
	CBitmap bmp;
	bmp.LoadBitmapW(IDB_MULTICAST);
	m_imagelist.Add(&bmp, RGB(255, 255, 255));
	bmp.DeleteObject();
	bmp.LoadBitmapW(IDB_MULTICAST);
	m_imagelist.Add(&bmp, RGB(255, 255, 255));
	bmp.DeleteObject();
	bmp.LoadBitmapW(IDB_SCREEN);
	m_imagelist.Add(&bmp, RGB(255, 255, 255));
	bmp.DeleteObject();
}


/******************************************************************
Function	: OnAddStudent
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:09
Parameter	: wparam--响应消息时传入的数值，这里是CItemData类对象
Parameter	: lparam--响应消息时传入的数值，这里没有使用
Return		: LRESULT
Desc		: 响应ID_STUDENTLOGIN 消息
******************************************************************/
LRESULT CTeacherDlg::OnAddStudent(WPARAM wparam, LPARAM lparam)
{
	CItemData *t = (CItemData*)wparam;
	int i = m_list.GetItemCount();
	m_list.InsertItem(i, t->m_stuInfo.GetStuName(), 0);
	m_list.SetItemText(i, 1, t->m_stuInfo.GetStuID());
	m_list.SetItemData(i, (DWORD)t);
	ShowStudentInfo();
	if (true == m_isStopMulticast)
	{
		CItemData* itemData = (CItemData*)m_list.GetItemData(i);
		itemData->BeginMulticast();
	}
	return 0;
}


/******************************************************************
Function	: ShowStudentInfo
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:09
Return		: void
Desc		: 显示登录学生机的信息
******************************************************************/
void  CTeacherDlg::ShowStudentInfo()
{
	int c = m_list.GetItemCount();
	CString online;
	TCHAR* szText;
	online.Format(_T("在线主机：%d 台"), c);
	szText = online.GetBuffer(online.GetLength());
	::SendMessageW(m_statusBar.m_hWnd, SB_SETTEXTW, (WPARAM)1, (LPARAM)szText);
	online.ReleaseBuffer();
}


/******************************************************************
Function	: OnLoginOutStudent
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:09
Parameter	: wparam--响应消息时传入的数值，这里是登录学生的ID 值
Parameter	: lparam--响应消息时传入的数值，这里没有使用
Return		: LRESULT
Desc		: 登录学生下线，响应ID_STUDENTLOGINOUT 消息
******************************************************************/
LRESULT CTeacherDlg::OnLoginOutStudent(WPARAM wparam, LPARAM lparam)
{
	CItemData* t;
	for (int i = 0; i < m_list.GetItemCount(); i++)
	{
		t = (CItemData*)m_list.GetItemData(i);
		if (t->m_id == (int)wparam)
		{
			::closesocket(t->GetSocket());
			delete t;
			m_list.DeleteItem(i);
		}
	}
	ShowStudentInfo();
	return 0;
}



/******************************************************************
Function	: OnSetBmpCompressData
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:09
Parameter	: wparam--响应消息时传入的数值，这里是BMP结构体对象
Parameter	: lparam--响应消息时传入的数值，这里没有使用
Return		: LRESULT
Desc		: 接收来自学生机的屏幕图像数据ID_SETBMPDATA 消息
******************************************************************/
LRESULT CTeacherDlg::OnSetBmpCompressData(WPARAM wparam, LPARAM lparam)
{
	BMP* bmp = (BMP*)wparam;
	int* itemOrder = (int*)lparam;
	m_pScreenDlg->ShowBmp(bmp, *itemOrder);
	return 0;
}


/******************************************************************
Function	: OnEndMonitor
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:10
Parameter	: wparam--响应消息时传入的数值，这里没有使用
Parameter	: lparam--响应消息时传入的数值，这里没有使用
Return		: LRESULT
Desc		: 结束对学生机的监控响应ID_MONITOREND
******************************************************************/
LRESULT CTeacherDlg::OnEndMonitor(WPARAM wparam, LPARAM lparam)
{
	for (int i = 0; i < MAX_MONITOR; i++)
	{
		if (-1 != m_item[i])
		{
			CItemData* itemData = (CItemData*)m_list.GetItemData(m_item[i]);
			if(itemData)
				itemData->EndScreenMonitor();
		}
		else if (-1 == m_item[i])
		{
			break;
		}
	}
	// 使工具栏的屏幕监控按钮变亮且有效
	m_toolBar.GetToolBarCtrl().EnableButton(ID_SCREENMONITOR, true);
	return 0;
}


/******************************************************************
Function	: OnCreatOneStudentDlg
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:10
Parameter	: wparam--响应消息时传入的数值，这里没有使用
Parameter	: lparam--响应消息时传入的数值，这里是学生在一对多对话框中显示的序号
Return		: LRESULT
Desc		: 响应一对多屏幕监控双击选中学生机实现一对一屏幕监控
			  响应ID_ONSTUINSCREEN 消息
******************************************************************/
LRESULT CTeacherDlg::OnCreatOneStudentDlg(WPARAM wparam, LPARAM lparam)
{
	int* itemOrder = (int*)lparam;
	int item = m_item[*itemOrder];
	if (-1 != item)
	{
		CItemData* itemData = (CItemData*)m_list.GetItemData(item);
		itemData->EndScreenMonitor();
		if (m_oneStuScreenDlg != NULL)
		{
			delete m_oneStuScreenDlg;
			m_oneStuScreenDlg = NULL;
		}
		m_oneStuScreenDlg = new COneStuScreenDlg(this);
		m_oneStuScreenDlg->Create(IDD_SCREEN_DIALOG1);
		m_oneStuScreenDlg->ShowWindow(SW_NORMAL);
		itemData->BeginMonitor(true, *itemOrder);
	}

	return 0;
}


/******************************************************************
Function	: OnSetOneStuBmpCompressData
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:11
Parameter	: wparam--响应消息时传入的数值，这里是BMP结构体对象
Parameter	: lparam--响应消息时传入的数值，这里是学生在一对多对话框中显示的序号
Return		: LRESULT
Desc		: 接收来自学生机的屏幕图像数据响应消息ID_STEONSTUSCREENDATA
******************************************************************/
LRESULT CTeacherDlg::OnSetOneStuBmpCompressData(WPARAM wparam, LPARAM lparam)
{
	BMP* bmp = (BMP*)wparam;
	int* itemOrder = (int*)lparam;
	m_oneStuScreenDlg->ShowBmp(bmp, *itemOrder);
	return 0;
}


/******************************************************************
Function	: OnEndOneStudentMonitor
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:11
Parameter	: wparam--响应消息时传入的数值，这里没有使用
Parameter	: lparam--响应消息时传入的数值，这里是学生在一对多对话框中显示的序号
Return		: LRESULT
Desc		: 结束一对一屏幕监控
******************************************************************/
LRESULT CTeacherDlg::OnEndOneStudentMonitor(WPARAM wparam, LPARAM lparam)
{
	int* itemOrder = (int*)lparam;
	int item = m_item[*itemOrder];
	if (-1 != item)
	{
		CItemData* itemData = (CItemData*)m_list.GetItemData(item);
		itemData->EndScreenMonitor();
		itemData->BeginMonitor(*itemOrder);
	}


	return 0;
}


/******************************************************************
Function	: OnBeginScreenMonitor
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:11
Return		: void
Desc		: 响应消息ID_ONSCREENMONITOR并调用发送屏幕监控消息到学生机的方法
******************************************************************/
void CTeacherDlg::OnBeginScreenMonitor()
{
	int selCount = m_list.GetSelectedCount();
	if (selCount > 2)
	{
		MessageBox(_T("同时监控的学生机子数量不能超过2台"));
		return;
	}
	if (0 == selCount)
	{
		MessageBox(_T("请选择要监控的学生机"));
		return;
	}
	DeletepScreenDlg();
	BeginScreenMonitor(selCount);
}


/******************************************************************
Function	: BeginScreenMonitor
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:12
Parameter	: selCount--要监控的学生机的数量
Return		: void
Desc		: 发送开始屏幕监控消息到教师机
******************************************************************/
void CTeacherDlg::BeginScreenMonitor(int selCount)
{
	int widthCount = (int)ceil((sqrt(double(selCount))));
	m_pScreenDlg = new CScreenMonitorDlg(this, widthCount);
	m_pScreenDlg->Create(IDD_SCREEN_DIALOG);
	m_pScreenDlg->ShowWindow(SW_NORMAL);


 	POSITION pos_s;
	pos_s = m_list.GetFirstSelectedItemPosition();
	int itemOrder = 0;
	memset(m_item, -1, sizeof(int)*MAX_MONITOR);
	while (pos_s != NULL)
	{
		int item = m_list.GetNextSelectedItem(pos_s);
		m_item[itemOrder] = item;
		CItemData* itemData = (CItemData*)m_list.GetItemData(item);
		itemData->BeginMonitor(itemOrder);
		itemOrder = itemOrder + 1;
	}

// 	int itemOrder = 0;
// 	memset(m_item, -1, sizeof(int)*MAX_MONITOR);
// //	pos_s = m_list.GetFirstSelectedItemPosition();
// 
// 	for (int i = 0; i < m_list.GetItemCount(); i++)
// 	{
// 		if (m_list.GetCheck(i))
// 		{
// 			CString str;
// 			str.Format(_T("第%d行的checkbox为选中状态"), i);
// 			AfxMessageBox(str);
// //			int item = m_list.GetNextSelectedItem(pos_s);
// // 			m_item[itemOrder] = i;
// // 			CItemData* itemData = (CItemData*)m_list.GetItemData(i);
// // 			itemData->BeginMonitor(itemOrder);
// // 			itemOrder = itemOrder + 1;
// 
// 		}
// 	}
	// 使工具栏的屏幕监控按钮变灰且无效
	m_toolBar.GetToolBarCtrl().EnableButton(ID_SCREENMONITOR, false);
}


/******************************************************************
Function	: OnMulticast
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:12
Return		: void
Desc		: 开始广播响应消息ID_MULTICAST
******************************************************************/
void CTeacherDlg::OnMulticast()
{
	m_pMulticast = new CMulticast();
	m_pMulticast->SetIsMulticastEnd(false);
	for (int i = 0; i < m_list.GetItemCount(); i++)
	{
		CItemData* itemData = (CItemData*)m_list.GetItemData(i);
		itemData->BeginMulticast();
	}
	::CloseHandle(CreateThread(0, 0, OnSendScreenData, (LPVOID)this, 0, NULL));
	m_isStopMulticast = true;

	::CloseHandle(CreateThread(0, 0, SwitchButton, (LPVOID)this, 0, NULL));
}


/******************************************************************
Function	: OnMulticastStop
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:12
Return		: void
Desc		: 停止关闭响应消息ID_MULTICAST_STOP
******************************************************************/
void CTeacherDlg::OnMulticastStop()
{
	
	for (int i = 0; i < m_list.GetItemCount(); i++)
	{
		CItemData* itemData = (CItemData*)m_list.GetItemData(i);
		itemData->EndMulticast();
	}
	m_pMulticast->SetIsMulticastEnd(true);
	m_isStopMulticast = false;
	//DeletepMulticast();

	::CloseHandle(CreateThread(0, 0, SwitchButton, (LPVOID)this, 0, NULL));
}


/******************************************************************
Function	: OnSendScreenData
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:13
Parameter	: self--CTeacherDlg类对象
Return		: DWORD WINAPI
Desc		: 发送教师机屏幕图像数据线程的调用方法
******************************************************************/
DWORD WINAPI CTeacherDlg::OnSendScreenData(LPVOID self)
{
	CTeacherDlg* t = (CTeacherDlg*)self;
	t->SendScreenData();
	return 0;
}

/******************************************************************
Function	: SwitchButton
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018/06/12
Parameter	: LPVOID self
Return		: DWORD WINAPI
Desc		: 
******************************************************************/
DWORD WINAPI CTeacherDlg::SwitchButton(LPVOID self)
{
	CTeacherDlg* t = (CTeacherDlg*)self;
	t->m_toolBar.GetToolBarCtrl().EnableButton(ID_MULTICAST, false);
	t->m_toolBar.GetToolBarCtrl().EnableButton(ID_MULTICAST_STOP, false);

	Sleep(1000);

	if (t->m_isStopMulticast) {
		t->m_toolBar.GetToolBarCtrl().EnableButton(ID_MULTICAST_STOP, true);
	} else {
		t->m_toolBar.GetToolBarCtrl().EnableButton(ID_MULTICAST, true);
	}
	return 0;
}


/******************************************************************
Function	: SendScreenData
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:17
Return		: void
Desc		: 发送教师机屏幕图像数据到学生机
******************************************************************/
void CTeacherDlg::SendScreenData()
{
	m_pMulticast->SendScreenData();
}


/******************************************************************
Function	: OnDestroy
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:17
Return		: void
Desc		: 窗口关闭销毁最终处理
******************************************************************/
void CTeacherDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	DeletepScreenDlg();
	DeletepOneStuScreenDlg();
	DeleteSocketMsg();
}


/******************************************************************
Function	: DeletepMulticast
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:16
Return		: void
Desc		: 删除广播对象释放内存
******************************************************************/
void CTeacherDlg::DeletepMulticast()
{
	if (m_pMulticast != NULL)
	{
		delete m_pMulticast;
		m_pMulticast = NULL;
	}
}


/******************************************************************
Function	: DeletepScreenDlg
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:16
Return		: void
Desc		: 释放new 的内存
******************************************************************/
void CTeacherDlg::DeletepScreenDlg()
{
	if (m_pScreenDlg != NULL)
	{
		delete m_pScreenDlg;
		m_pScreenDlg = NULL;
	}
}


/******************************************************************
Function	: DeletepOneStuScreenDlg
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:16
Return		: void
Desc		: 释放new 的内存
******************************************************************/
void CTeacherDlg::DeletepOneStuScreenDlg()
{
	if (m_oneStuScreenDlg != NULL)
	{
		delete m_oneStuScreenDlg;
		m_oneStuScreenDlg = NULL;
	}
}


/******************************************************************
Function	: DeleteSocketMsg
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:16
Return		: void
Desc		: 释放SOCKET 资源
******************************************************************/
void CTeacherDlg::DeleteSocketMsg()
{
	if (m_socketMsg != INVALID_SOCKET)
	{
		closesocket(m_socketMsg);
		m_socketMsg = NULL;
	}
}


/******************************************************************
Function	: OnNMClickList1
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:15
Parameter	: pNMHDR
Parameter	: pResult
Return		: void
Desc		: CListControl双击消息
******************************************************************/
void CTeacherDlg::OnNMClickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO:  在此添加控件通知处理程序代码
// 	static bool isClick = false;
// 	// 没有行被选中则iItm = -1
// 	if (pNMItemActivate->iItem != -1)
// 	{
// 		if (false == isClick)
// 		{
// 			m_list.SetCheck(pNMItemActivate->iItem, true);
// 			isClick = true;
// 		}
// 		else
// 		{
// 			m_list.SetCheck(pNMItemActivate->iItem, false);
// 			isClick = false;
// 		}
// 	}
	*pResult = 0;
}


/******************************************************************
Function	: OnNMDblclkList1
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:13
Parameter	: pNMHDR
Parameter	: pResult
Return		: void
Desc		: CListControl双击消息
******************************************************************/
void CTeacherDlg::OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	OnBeginScreenMonitor();
	*pResult = 0;
}
