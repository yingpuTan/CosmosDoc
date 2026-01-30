
// TestDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "Test.h"
#include "TestDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CTestDlg dialog



CTestDlg::CTestDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TEST_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTestDlg::UpdateText(CString strInfo)
{
	m_editCtrl.SetWindowText(strInfo);
}

void CTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_editCtrl); // 关联编辑框控件
	DDX_Control(pDX, IDC_COMBO_TYPE, m_pComboType);
	DDX_Control(pDX, IDC_EDIT_CONTENT, m_pEditContent);
	DDX_Control(pDX, IDC_EDIT_GROUP, m_pEditGroup);
	DDX_Control(pDX, IDC_BUTTON_SENDWIDGET, m_pBtnSendWidget);
}

BEGIN_MESSAGE_MAP(CTestDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_ERASEBKGND()
	ON_BN_CLICKED(IDC_BUTTON1, &CTestDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CTestDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CTestDlg::OnBnClickedButton3)
	ON_EN_CHANGE(IDC_EDIT1, &CTestDlg::OnEnChangeEdit1)
	ON_BN_CLICKED(IDC_BUTTON4, &CTestDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &CTestDlg::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON_SENDWIDGET, &CTestDlg::OnBnClickedButtonSendwidget)
	ON_BN_CLICKED(IDC_BUTTON_SENDWIDGET_Group, &CTestDlg::OnBnClickedButtonSendwidgetGroup)
END_MESSAGE_MAP()


// CTestDlg message handlers

BOOL CTestDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_pComboType.SetCurSel(0);

	// TODO: Add extra initialization here
	PostMessage(WM_USER_INIT, NULL, NULL);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTestDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

BOOL CTestDlg::OnEraseBkgnd(CDC* pDC)
{
	CRect rect;
	GetClientRect(&rect);

	CBrush brush(RGB(169, 204, 200));
	pDC->FillRect(&rect, &brush);

	return TRUE;
}

void CTestDlg::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here
	// 推送账户信息
	PostMessage(WM_USER_PUSH_ACCOUNTINFO);
}


void CTestDlg::OnBnClickedButton3()
{
	// TODO: Add your control notification handler code here
	// 通知查询订阅
	UpdateText(L"");
	PostMessage(WM_USER_QRY_ACCOUNTINFO);
}


void CTestDlg::OnEnChangeEdit1()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}


void CTestDlg::OnBnClickedButton4()
{
	// TODO: Add your control notification handler code here
	PostMessage(WM_USER_TEST);
}

void CTestDlg::OnBnClickedButton1()
{
	//PostMessage(WM_QUIT);
	// TODO: Add your control notification handler code here

	PostMessage(WM_USER_TEST_INVOKE, (WPARAM)1, NULL);
}

void CTestDlg::OnBnClickedButton5()
{
	// TODO: Add your control notification handler code here

	PostMessage(WM_USER_TEST_INVOKE, (WPARAM)2, NULL);
}


void CTestDlg::OnBnClickedButtonSendwidget()
{
	CString strContent;
	m_pEditContent.GetWindowText(strContent);

	CString strGroup;
	m_pEditGroup.GetWindowText(strGroup);

	int nType = m_pComboType.GetCurSel();

	CString *pStrFormat =  new CString();
	if (pStrFormat)
	{
		pStrFormat->Format(L"%s|%s|%d|2", strContent, strGroup, nType);
		PostMessage(WM_USER_SENDWIDGET_REQ, (WPARAM)pStrFormat, NULL);
	}
	
	// TODO: Add your control notification handler code here
}


void CTestDlg::OnBnClickedButtonSendwidgetGroup()
{
	CString strContent;
	m_pEditContent.GetWindowText(strContent);

	CString strGroup;
	m_pEditGroup.GetWindowText(strGroup);

	int nType = m_pComboType.GetCurSel();

	CString* pStrFormat = new CString();
	if (pStrFormat)
	{
		pStrFormat->Format(L"%s|%s|%d|1", strContent, strGroup, nType);
		PostMessage(WM_USER_SENDWIDGET_REQ, (WPARAM)pStrFormat, NULL);
	}
	// TODO: Add your control notification handler code here
}
