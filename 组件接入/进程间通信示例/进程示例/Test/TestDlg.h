
// TestDlg.h : header file
//

#pragma once

#define WM_USER_INIT              (WM_USER + 0X001)
#define WM_USER_SETHWND           (WM_USER + 0X002)
#define WM_USER_SETPOS            (WM_USER + 0X003)
#define WM_USER_QRY_ACCOUNTINFO   (WM_USER + 0X004)		// 通知来查询和订阅
#define WM_USER_PUSH_ACCOUNTINFO  (WM_USER + 0X005)		// 推送账户信息
#define WM_USER_RECV_SUB          (WM_USER + 0X006)	
#define WM_USER_EXITAPP           (WM_USER + 0X007)
#define WM_USER_TEST              (WM_USER + 0X008)
#define WM_USER_TEST_INVOKE       (WM_USER + 0X009)
#define WM_USER_SENDWIDGET_REQ    (WM_USER + 0X010)



// CTestDlg dialog
class CTestDlg : public CDialogEx
{
// Construction
public:
	CTestDlg(CWnd* pParent = nullptr);	// standard constructor
	void SetWindowSize(int width, int height)
	{
		// 设置新的大小
		int newWidth = 600; // 新宽度
		int newHeight = 400; // 新高度

		//SetWindowPos(NULL, 0, 0, width, height, 0);
		SetWindowPos(&CWnd::wndTopMost, 0, 0, width, height, SWP_NOZORDER);

		Invalidate();
		UpdateWindow();
	}

	void UpdateText(CString strInfo);
	void CloswWindow()
	{
		SendMessage(WM_CLOSE);
	}
// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TEST_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	CEdit m_editCtrl; // 声明编辑框控件

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnEnChangeEdit1();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButton5();
private:
	CComboBox m_pComboType;
public:
	CEdit m_pEditContent;
	CEdit m_pEditGroup;
	CButton m_pBtnSendWidget;
	afx_msg void OnBnClickedButtonSendwidget();
	afx_msg void OnBnClickedButtonSendwidgetGroup();
};
