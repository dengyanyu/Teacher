#include "stdafx.h"
#include "Teacher.h"
#include "TeacherDlg.h"
#include "afxdialogex.h"
#include "ItemData.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTeacherDlg �Ի���



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


// CTeacherDlg ��Ϣ�������

BOOL CTeacherDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO:  �ڴ���Ӷ���ĳ�ʼ������
	SetMainDlgSytle();
	// ����һ���߳����ڼ������������SOCKET
	::CloseHandle(CreateThread(0, 0, OnIPListen, (LPVOID)this, 0, NULL));
	::CloseHandle(CreateThread(0, 0, OnMsgListen, (LPVOID)this, 0, NULL));

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}


void CTeacherDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	CDialogEx::OnSysCommand(nID, lParam);
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�
void CTeacherDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CTeacherDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


/******************************************************************
Function	: OnMsgListen
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:07
Parameter	: self--CTeacherDlg��
Return		: DWORD WINAPI
Desc		: ����ѧ������¼�̵߳����з���
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
Desc		: ����ѧ����¼������Ψһ��ID
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
Desc		: ����¼ѧ��������Ψһ��ID
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
	// ������Ϣ���߳�����
	itemData->Run();
}


/******************************************************************
Function	: SetMainDlgSytle
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:08
Return		: void
Desc		: ���ý�ʦ����������
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
Desc		: ΪListControl ������ͷ
******************************************************************/
void CTeacherDlg::InsertColumn()
{
	// ����List Control ���Ϊ����ѡ�У����񣬴��и�ѡ�򣨿ؼ���View����ֵҪ����ΪReport��
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	// ��List Control �����
	m_list.InsertColumn(0, _T("����"), LVCFMT_CENTER, 150);
	m_list.InsertColumn(1, _T("����"), LVCFMT_CENTER, 150);
	m_list.InsertColumn(2, _T("ѧ��"), LVCFMT_CENTER, 150);
	m_list.DeleteColumn(0);
}


/******************************************************************
Function	: CreateStatusBar
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:08
Return		: void
Desc		: ����״̬��
******************************************************************/
void CTeacherDlg::CreateStatusBar()
{
	// �ײ�״̬���Ĵ���
	BOOL hbar = m_statusBar.Create(this);
	// ״̬����ID
	CRect ct;
	GetClientRect(&ct);
	UINT b[2] = { 1009, 1010 };
	hbar = m_statusBar.SetIndicators(b, 2);
	m_statusBar.SetPaneInfo(0, b[0], SBPS_NORMAL, (int)(ct.Width() * 0.5));
	m_statusBar.SetPaneInfo(1, b[1], SBPS_NORMAL, (int)(ct.Width() * 0.5));
	m_statusBar.SetPaneText(0, _T("���ߣ�ʯ�̽�"));
	m_statusBar.SetPaneText(1, _T("����������0 ̨"));
	// ��ʾ��������״̬��
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
}


/******************************************************************
Function	: CreatToolBar
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:08
Return		: void
Desc		: ����������
******************************************************************/
void CTeacherDlg::CreatToolBar()
{
	// ����ÿһ����������ID��֮��Ҫ�����Ϣ��Ӧ
	const UINT t[3] = { 1001, 1003, 1002 };
	m_toolBar.CreateEx(this);
	m_toolBar.SetButtons(t, 3);
	m_toolBar.SetSizes(CSize(60, 56), CSize(24, 24));
	// �������������
	m_toolBar.SetButtonText(0, _T("��ʼͶӰ"));
	m_toolBar.SetButtonText(1, _T("ֹͣͶӰ"));
	m_toolBar.SetButtonText(2, _T("��Ļ���"));

	// ���������ͼƬ
	m_toolBar.GetToolBarCtrl().SetImageList(&m_imagelist);
	m_toolBar.GetToolBarCtrl().EnableButton(ID_MULTICAST_STOP, false);
}


/******************************************************************
Function	: LoadBitBmp
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:08
Return		: void
Desc		: ����λͼ��Դ
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
Parameter	: wparam--��Ӧ��Ϣʱ�������ֵ��������CItemData�����
Parameter	: lparam--��Ӧ��Ϣʱ�������ֵ������û��ʹ��
Return		: LRESULT
Desc		: ��ӦID_STUDENTLOGIN ��Ϣ
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
Desc		: ��ʾ��¼ѧ��������Ϣ
******************************************************************/
void  CTeacherDlg::ShowStudentInfo()
{
	int c = m_list.GetItemCount();
	CString online;
	TCHAR* szText;
	online.Format(_T("����������%d ̨"), c);
	szText = online.GetBuffer(online.GetLength());
	::SendMessageW(m_statusBar.m_hWnd, SB_SETTEXTW, (WPARAM)1, (LPARAM)szText);
	online.ReleaseBuffer();
}


/******************************************************************
Function	: OnLoginOutStudent
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:09
Parameter	: wparam--��Ӧ��Ϣʱ�������ֵ�������ǵ�¼ѧ����ID ֵ
Parameter	: lparam--��Ӧ��Ϣʱ�������ֵ������û��ʹ��
Return		: LRESULT
Desc		: ��¼ѧ�����ߣ���ӦID_STUDENTLOGINOUT ��Ϣ
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
Parameter	: wparam--��Ӧ��Ϣʱ�������ֵ��������BMP�ṹ�����
Parameter	: lparam--��Ӧ��Ϣʱ�������ֵ������û��ʹ��
Return		: LRESULT
Desc		: ��������ѧ��������Ļͼ������ID_SETBMPDATA ��Ϣ
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
Parameter	: wparam--��Ӧ��Ϣʱ�������ֵ������û��ʹ��
Parameter	: lparam--��Ӧ��Ϣʱ�������ֵ������û��ʹ��
Return		: LRESULT
Desc		: ������ѧ�����ļ����ӦID_MONITOREND
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
	// ʹ����������Ļ��ذ�ť��������Ч
	m_toolBar.GetToolBarCtrl().EnableButton(ID_SCREENMONITOR, true);
	return 0;
}


/******************************************************************
Function	: OnCreatOneStudentDlg
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:10
Parameter	: wparam--��Ӧ��Ϣʱ�������ֵ������û��ʹ��
Parameter	: lparam--��Ӧ��Ϣʱ�������ֵ��������ѧ����һ�Զ�Ի�������ʾ�����
Return		: LRESULT
Desc		: ��Ӧһ�Զ���Ļ���˫��ѡ��ѧ����ʵ��һ��һ��Ļ���
			  ��ӦID_ONSTUINSCREEN ��Ϣ
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
Parameter	: wparam--��Ӧ��Ϣʱ�������ֵ��������BMP�ṹ�����
Parameter	: lparam--��Ӧ��Ϣʱ�������ֵ��������ѧ����һ�Զ�Ի�������ʾ�����
Return		: LRESULT
Desc		: ��������ѧ��������Ļͼ��������Ӧ��ϢID_STEONSTUSCREENDATA
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
Parameter	: wparam--��Ӧ��Ϣʱ�������ֵ������û��ʹ��
Parameter	: lparam--��Ӧ��Ϣʱ�������ֵ��������ѧ����һ�Զ�Ի�������ʾ�����
Return		: LRESULT
Desc		: ����һ��һ��Ļ���
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
Desc		: ��Ӧ��ϢID_ONSCREENMONITOR�����÷�����Ļ�����Ϣ��ѧ�����ķ���
******************************************************************/
void CTeacherDlg::OnBeginScreenMonitor()
{
	int selCount = m_list.GetSelectedCount();
	if (selCount > 2)
	{
		MessageBox(_T("ͬʱ��ص�ѧ�������������ܳ���2̨"));
		return;
	}
	if (0 == selCount)
	{
		MessageBox(_T("��ѡ��Ҫ��ص�ѧ����"));
		return;
	}
	DeletepScreenDlg();
	BeginScreenMonitor(selCount);
}


/******************************************************************
Function	: BeginScreenMonitor
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:12
Parameter	: selCount--Ҫ��ص�ѧ����������
Return		: void
Desc		: ���Ϳ�ʼ��Ļ�����Ϣ����ʦ��
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
// 			str.Format(_T("��%d�е�checkboxΪѡ��״̬"), i);
// 			AfxMessageBox(str);
// //			int item = m_list.GetNextSelectedItem(pos_s);
// // 			m_item[itemOrder] = i;
// // 			CItemData* itemData = (CItemData*)m_list.GetItemData(i);
// // 			itemData->BeginMonitor(itemOrder);
// // 			itemOrder = itemOrder + 1;
// 
// 		}
// 	}
	// ʹ����������Ļ��ذ�ť�������Ч
	m_toolBar.GetToolBarCtrl().EnableButton(ID_SCREENMONITOR, false);
}


/******************************************************************
Function	: OnMulticast
Author		: shiyunjin(luoyibin_001@163.com)
Date		: 2018-6-13 11:12
Return		: void
Desc		: ��ʼ�㲥��Ӧ��ϢID_MULTICAST
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
Desc		: ֹͣ�ر���Ӧ��ϢID_MULTICAST_STOP
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
Parameter	: self--CTeacherDlg�����
Return		: DWORD WINAPI
Desc		: ���ͽ�ʦ����Ļͼ�������̵߳ĵ��÷���
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
Desc		: ���ͽ�ʦ����Ļͼ�����ݵ�ѧ����
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
Desc		: ���ڹر��������մ���
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
Desc		: ɾ���㲥�����ͷ��ڴ�
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
Desc		: �ͷ�new ���ڴ�
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
Desc		: �ͷ�new ���ڴ�
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
Desc		: �ͷ�SOCKET ��Դ
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
Desc		: CListControl˫����Ϣ
******************************************************************/
void CTeacherDlg::OnNMClickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
// 	static bool isClick = false;
// 	// û���б�ѡ����iItm = -1
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
Desc		: CListControl˫����Ϣ
******************************************************************/
void CTeacherDlg::OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	OnBeginScreenMonitor();
	*pResult = 0;
}
