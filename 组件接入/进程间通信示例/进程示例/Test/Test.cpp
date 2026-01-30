
// Test.cpp : Defines the class behaviors for the application.
//

#include "pch.h"
#include "framework.h"
#include "Test.h"
#include "TestDlg.h"
#include "PipeClient.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <string>
#include <fstream>
#include <sstream>
#include <set>
#include <process.h>
#include <Windows.h>
#include <mmsystem.h>
#include <atlconv.h> // 包含 ATL 转换宏

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include <app.h>


// CTestApp

BEGIN_MESSAGE_MAP(CTestApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CTestApp construction

struct AccountInfo
{
	std::string ID;
	int         Type   = 0;
	int         Status = 0;
};

std::map<std::string, AccountInfo> g_mapActInfo;
std::map<std::string, RpcRequest>  g_mapRequest;
std::set<std::string> g_setSub;

std::mutex g_mutex;
HWND g_dlgHwnd = NULL;
CTestApp::CTestApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance

	AccountInfo info;
	info.ID = "123456";
	g_mapActInfo["123456"] = info;
}


// The one and only CTestApp object

CTestApp theApp;
CTestDlg* pDlg = nullptr;

// CTestApp initialization


typedef bool     (*Bridge_InitClientFunc)(const char*, int, const char*, int, double, bool);
typedef RET_CALL (*Bridge_InvokeAsyncFunc)(void*, InvokeCallback, int);
typedef RET_CALL (*Bridge_InvokeWidgetAsyncFunc)(const char*, InvokeType, void*, bool,InvokeCallback, int);
typedef RET_CALL (*Bridge_InvokeFunc)(void*, void*, int*, int);
typedef RET_CALL (*Bridge_InvokeWidgetFunc)(const char*, InvokeType, void*, bool, void*, int*, int);
typedef RET_CALL (*Bridge_NotifyFunc)(void*);
typedef RET_CALL (*Bridge_NotifyWidgetFunc)(const char*, InvokeType, void*, bool);
typedef RET_CALL (*Bridge_SubscribeFunc)(void*);
typedef RET_CALL (*Bridge_PushFunc)(void*);
typedef void     (*Bridge_ExitFunc)();
typedef void     (*Bridge_RegisterFunc)(OnInvoke, OnNotify, OnSubscribe, OnPush, OnFreeVoidPtr);

void InvokeAsyncCallback(RET_CALL ret, void* _out, int _outsize)
{
	RpcResponse* resp = static_cast<RpcResponse*>(_out);
	if (!resp)
	{
		return;
	}

	if (ret == RET_CALL::Ok)
	{
		int code = resp->code;
		std::string id = resp->id;
		Json::FastWriter writer;
		std::string strresult = writer.write(resp->result);
		int i = -1;
		AfxMessageBox(_T("异步测试结果返回"), MB_OK);
	}
	else
	{
		// undo
	}
}

void* __cdecl HandleInvoke(void* _in, int size)
{
	if (!_in)
	{
		return nullptr;
	}

	RpcRequest* request = static_cast<RpcRequest*>(_in);
	RpcResponse* resp = new RpcResponse();

	resp->id = request->id;

	// 根据 method 判断具体业务
	if (request->method == "qry_account")
	{
		// 查询账户
		Json::Value& js = request->param;

		if (js.isMember("ID") && js["ID"].asString() == "ALL") {
			// 查询所有账户信息
			Json::Value js_array(Json::arrayValue);
			Json::Value js1;
			js1["ID"] = "123456"; // 资金账户id：123456
			js1["Type"] = 1;      // 账户类型：1-普通
			js1["Status"] = 1;    // 账户状态：1-在线

			Json::Value js2;
			js2["ID"] = "456789"; // 资金账户id：456789
			js2["Type"] = 2;      // 账户类型：2-信用
			js2["Status"] = 0;    // 账户状态：0-不在线

			js_array.append(js1);
			js_array.append(js2);

			resp->result = js_array;
			resp->code = 0; // 若想返回报错，则code设置为非0，然后给 resp->error 赋值即可
		}
		else {
			if (js.isMember("ID") && js["ID"].asString() == "123456") {
				// 查询指定账户ID的账户信息（假设 ID 为 "123456"，即返回账户123456的信息）
				Json::Value jsAccount;
				jsAccount["ID"] = "123456"; // 资金账户id：123456
				jsAccount["Type"] = 1;      // 账户类型：1-普通
				jsAccount["Status"] = 1;    // 账户状态：1-在线

				resp->result = jsAccount;
				resp->code = 0;
			}
		}

		std::lock_guard<std::mutex> lock(g_mutex);
		if (g_dlgHwnd && ::IsWindow(g_dlgHwnd))
		{
			::PostMessage(g_dlgHwnd, WM_USER_RECV_SUB, 0, 0);
		}
	}
	else
	{
		// 其他查询业务
	}

	return (void*)resp;
}

void __cdecl HandleNotify(void* _in, int size)
{
	if (!_in)
	{
		return;
	}

	RpcRequest* request = static_cast<RpcRequest*>(_in);
	if (request->method == "setsize")
	{
		// 订阅账户
		Json::Value& js = request->param;

		double scaling = js["curscaling"].asDouble();
		int width = static_cast<int>(js["width"].asDouble() * scaling);
		int height = static_cast<int>(js["height"].asDouble() * scaling);
		{
			std::lock_guard<std::mutex> lock(g_mutex);
			if (g_dlgHwnd && ::IsWindow(g_dlgHwnd))
			{
				::PostMessage(g_dlgHwnd, WM_USER_SETPOS, (WPARAM)width, (LPARAM)height);
			}
		}
	}
	else if (request->method == "close")
	{
		// 退出
		::PostMessage(g_dlgHwnd, WM_USER_EXITAPP, NULL, NULL);
		//exit(0);
	}
	else
	{
		// 其他通知业务
	}
}

void __cdecl HandleSubscribe(void* _in, int size)
{
	if (!_in)
	{
		return;
	}

	RpcRequest* request = static_cast<RpcRequest*>(_in);

	if (request->method == "sub_account")
	{
		// 订阅账户
		Json::Value& js = request->param;

		if (js.isMember("ID") && !js["ID"].isNull() && !js["ID"].asString().empty()) {
			// 缓存订阅的key
			std::string strSubkey = request->method + "_" + js["ID"].asString();
			g_setSub.insert(strSubkey);
		}
	}
	else
	{
		// 其他订阅业务
	}
}

void __cdecl HandlePush(void* _in, int size)
{
	if (!_in)
	{
		return;
	}

	RpcPush* push = static_cast<RpcPush*>(_in);

	// 根据 topic 判断具体业务
	if (push->topic == "push_account")
	{
		// 推送账户信息
		Json::Value& js = push->param;

		std::string strID = js["ID"].asString();
		auto it = g_mapActInfo.find(strID);
		if (it != g_mapActInfo.end())
		{
			it->second.Type = js["Type"].asInt();
			it->second.Status = js["Status"].asInt();
		}
	}
	else
	{
		// 其他推送业务
	}
}

void __cdecl HandleFreevoid(void* param)
{
	RpcResponse *pResp = (RpcResponse*)param;
	if (pResp)
	{
		delete pResp;
		pResp = nullptr;
	}
}

void SplitString(const std::string& input, char delimiter, std::vector<std::string>& output)
{
	std::istringstream stream(input);
	std::string token;

	while (std::getline(stream, token, delimiter)) {
		output.push_back(token);
	}
}

BOOL CTestApp::InitInstance()
{
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);
	CWinApp::InitInstance();
	AfxEnableControlContainer();
	CShellManager *pShellManager = new CShellManager;
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	HMODULE hDll = LoadLibraryA("PipeClient.dll");
	if (!hDll)
	{
		std::cout << "LoadLibrary failed!";
		return -1;
	}

	// 函数指针
	Bridge_InitClientFunc			initClientFunc = (Bridge_InitClientFunc)GetProcAddress(hDll, "InitClientForC");
	Bridge_InvokeFunc				invokeFunc = (Bridge_InvokeFunc)GetProcAddress(hDll, "Invoke");
	Bridge_InvokeWidgetFunc			invokeWidgetFunc = (Bridge_InvokeWidgetFunc)GetProcAddress(hDll, "InvokeWidget");
	Bridge_InvokeAsyncFunc			invokeAsyncFunc = (Bridge_InvokeAsyncFunc)GetProcAddress(hDll, "InvokeAsync");
	Bridge_InvokeWidgetAsyncFunc	invokeWidgetAsyncFunc = (Bridge_InvokeWidgetAsyncFunc)GetProcAddress(hDll, "InvokeWidgetAsync");
	Bridge_NotifyFunc				notifyFunc = (Bridge_NotifyFunc)GetProcAddress(hDll, "Notify");
	Bridge_NotifyWidgetFunc			notifyWidgetFunc = (Bridge_NotifyWidgetFunc)GetProcAddress(hDll, "NotifyWidget");
	Bridge_SubscribeFunc			subscribeFunc = (Bridge_SubscribeFunc)GetProcAddress(hDll, "Subscribe");
	Bridge_PushFunc					pushFunc = (Bridge_PushFunc)GetProcAddress(hDll, "Push");
	Bridge_ExitFunc					exitFunc = (Bridge_ExitFunc)GetProcAddress(hDll, "ExitApp");
	Bridge_RegisterFunc				registerCallbackFunc = (Bridge_RegisterFunc)GetProcAddress(hDll, "Register");

	// 1、注册回调
	registerCallbackFunc(HandleInvoke, HandleNotify, HandleSubscribe, HandlePush, HandleFreevoid);

	// 2.解析命令行参数、获取父窗口句柄和初始大小
	std::string cmdLine = GetCommandLineA();
	std::vector<std::string> vecArg;
	SplitString(cmdLine,' ', vecArg);

	std::string strPipeName;
	HWND hParent = 0;
	RECT rect = { 0 };
	if (vecArg.size() > 2)
	{
		//解析第一个命令行参数，管道名
		strPipeName = vecArg[1];

		// 解析第二个命令行参数，句柄+初始化大小
		std::string InitArg = vecArg[2];
		std::vector<std::string> vecInit;
		SplitString(InitArg, '|', vecInit);
		if (vecArg.size() > 2)
		{
			hParent = (HWND)std::stol(vecInit[0].c_str());
			rect.right = std::stol(vecInit[1].c_str());
			rect.bottom = std::stol(vecInit[2].c_str());
		}
	}
	
	pDlg = new CTestDlg();
	pDlg->Create(IDD_TEST_DIALOG, hParent ? CWnd::FromHandle(hParent):(CWnd*)this);

	//调整窗口属性
	if (hParent != 0)
	{
		HWND hWnd = pDlg->GetSafeHwnd();
		LONG lPreStyle = GetWindowLong(hWnd, GWL_STYLE);
		LONG lPreExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
		lPreStyle &= ~WS_POPUP;
		lPreStyle |= WS_CHILD;

		lPreStyle &= ~(WS_BORDER | WS_THICKFRAME | WS_DLGFRAME);

		lPreExStyle &= ~(WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE);

		SetWindowLong(hWnd, GWL_STYLE, lPreStyle);
		SetWindowLong(hWnd, GWL_EXSTYLE, lPreExStyle);
		pDlg->SetParent(CWnd::FromHandle(hParent));
	}
	
	//移动窗口位置
	if (rect.right > 0 || rect.bottom > 0)
	{
		pDlg->MoveWindow(&rect);
	}

	MSG msg = { 0 };
	while (::GetMessage(&msg, NULL, 0, 0))
	{
		if (msg.message == WM_QUIT)
		{
			break;
		}
		else if (msg.message == WM_USER_INIT)
		{
			//system("pause");
			{
				std::lock_guard<std::mutex> lock(g_mutex);
				g_dlgHwnd = pDlg->GetSafeHwnd();
			}
			std::string log_path = "test_rpc_cc.log";
			if (!strPipeName.empty())
			{
				// 1、启动管道
				if (!initClientFunc(strPipeName.c_str(), strPipeName.length(), log_path.c_str(), log_path.size() + 1, 2, false))
				{
					std::cout << "InitClient failed!";
					return -1;
				}
				else
				{
					//启动成功，通知对方初始化完成
					RpcRequest param;
					param.id = app::GetUuid();
					param.method = "init_succ";
					notifyFunc(&param);
				}
			}
		}
		else if (msg.message == WM_USER_SETPOS)
		{
			int len = (int)msg.wParam;     
            int height = (int)msg.lParam;    
            pDlg->SetWindowSize(len, height);
			pDlg->ShowWindow(SW_SHOW);
		}
		else if (msg.message == WM_USER_EXITAPP)
		{
			exitFunc();
			return 0;
		}
		else if (msg.message == WM_USER_TEST_INVOKE)
		{
			int type = (int)msg.wParam;
			int interval = 30000;

			if (type == 1)
			{
				// 同步测试
				{
					RpcRequest param;
					param.id = app::GetUuid();
					param.method = "test_invoke";
					int outsize = 0;
					RpcResponse* out = new RpcResponse();
					RET_CALL ret = invokeFunc(&param, (void*)&out, &outsize, interval);
					if (ret == RET_CALL::Ok)
					{
						int code = out->code;
						std::string id = out->id;
						Json::FastWriter writer;
						std::string strresult = writer.write(out->result);
						AfxMessageBox(_T("同步测试结果正常返回"), MB_OK);
					}
					else
					{
						// undo
					}
				}
				AfxMessageBox(_T("同步测试后续流程 - 被阻塞"), MB_OK);
			}
			else if (type == 2)
			{
				// 异步测试
				{
					RpcRequest* param = new RpcRequest();
					param->id = app::GetUuid();
					param->method = "test_invoke_async";
					int outsize = 0;					
					invokeAsyncFunc(param, InvokeAsyncCallback, interval);
				}
				AfxMessageBox(_T("异步测试后续流程 - 不阻塞"), MB_OK);
			}
		}
		else if (msg.message == WM_USER_QRY_ACCOUNTINFO)
		{
			// 通知对方来查询和订阅
			RpcRequest param;
			param.id = app::GetUuid();
			param.method = "notf_sub";
			notifyFunc(&param);
		}
		else if (msg.message == WM_USER_TEST)
		{
			RpcRequest param;
			param.id = app::GetUuid();
			param.method = "notf_test";

			Json::Value jsParam;
			jsParam["text1"] = "你好呀1111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test\
1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test\
1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test\
1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test\
1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test\
1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test\
1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test\
1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test\
1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test\
1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test\
1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test\
1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test\
1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test\
1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test\
1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test\
1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test\
1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test\
1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test\
1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test\
1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test\
1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test\
1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test\
1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test\
1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test\
1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test\
1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test\
1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test\
1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test\
1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test\
1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test\
1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test\
1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test";
			param.param = jsParam;

			int count = 10000;
			while (count)
			{
				count--;				

				notifyFunc(&param);
			}	
		}
		else if (msg.message == WM_USER_PUSH_ACCOUNTINFO)
		{
			// 要推送的账户数据 123456，假设组成key为 
			std::string strSubkey = "sub_account_123456";
			auto it = g_setSub.find(strSubkey);
			if (it != g_setSub.end())
			{
				// 说明订阅过该key，则可以推送数据
				RpcPush param;
				param.topic = "push_account";

				Json::Value jsParam;
				jsParam["ID"] = "123456"; // 资金账户id：123456
				jsParam["Type"] = 1;      // 账户类型：1-普通
				jsParam["Status"] = 1;    // 账户状态：1-在线
				param.param = jsParam;

				pushFunc(&param);	
			}			
		}
		else if (msg.message == WM_USER_RECV_SUB)
		{
			CString strCstr = L"收到对方查询和订阅请求，并成功处理";
			pDlg->UpdateText(strCstr);
		}
		else if (msg.message == WM_USER_SENDWIDGET_REQ)
		{
			int interval = 30000;
			CString* pStrFormate = (CString*)msg.wParam;
			if (pStrFormate)
			{
				CString str = *pStrFormate;
				std::vector<CString> vecString;
				int pos = 0;
				int len = str.GetLength();
				
				while (pos < len) {
					int nextPos = str.Find(_T('|'), pos);  // 查找分隔符位置
					if (nextPos == -1) {
						nextPos = len;  // 未找到分隔符时，取剩余字符串
					}
					CString token = str.Mid(pos, nextPos - pos);  // 提取子字符串
					vecString.push_back(token);
					pos = nextPos + 1;  // 移动到分隔符后的位置
				}

				std::string strContent = std::string(CT2A(vecString[0]));
				std::string strGroup = std::string(CT2A(vecString[1]));
				std::string strType = std::string(CT2A(vecString[2]));
				int nInvokeType = std::string(CT2A(vecString[3])) == "1" ? 1 : 2;
				RpcRequest request;
				
				request.method = "textchanged";
				request.param["text"] = Json::Value(strContent);

				//请求
				if (strType == "0")
				{
					int outsize = 0;
					RpcResponse* out = new RpcResponse();
					RET_CALL ret = invokeWidgetFunc(strGroup.c_str(), (InvokeType)nInvokeType, &request, false, &out, &outsize, interval);
					if (ret == RET_CALL::Ok)
					{
						int code = out->code;
						std::string id = out->id;
						Json::FastWriter writer;
						std::string strresult = writer.write(out->result);
						CString strResult;
						strResult.Format(L"返回结果 code:%d, id:%s, result:%s", code, CString(id.c_str()), CString(strresult.c_str()));
					}
					else
					{
						// undo
					}
				}
				//通知
				else
				{
					notifyWidgetFunc(strGroup.c_str(), (InvokeType)nInvokeType, &request, false);
				}
			}
			
			delete pStrFormate;
			pStrFormate = nullptr;
		}

		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}

	// Delete the shell manager created above.
	if (pShellManager != nullptr)
	{
		delete pShellManager;
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
