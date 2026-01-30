#pragma once
#include <windows.h>
#include <string>
#include <map>
#include <mutex>
#include <condition_variable>
#include <future>
#include <queue>
#include <atomic>
#include "jsonCpp/json.h"

#ifdef _WIN32
#define API_EXPORT __declspec(dllexport)
#else
#define API_EXPORT __attribute__((visibility("default")))
#endif // _WIN32

/// <summary>
/// Rpc请求体
/// </summary>
struct RpcRequest
{
	std::string id;
	std::string method;
	Json::Value param;
};

/// <summary>
/// Rpc应答体
/// </summary>
struct RpcResponse
{
	std::string id;
	int         code = 0;    // code = 0 表示业务返回成功，code != 0 表示业务返回错误，业务按需定义错误编码
	Json::Value error;
	Json::Value result;
};

/// <summary>
/// Rpc推送体
/// </summary>
struct RpcPush
{
	std::string topic;
	Json::Value param;
};


extern "C" {

	enum class RET_CALL{
		Exception = -3,
		Sendfail  = -2,
		Timeout   = -1,
		Pipenull  = 0,
		Ok        = 1
	};

	enum class InvokeType {
		/// <summary>
		/// 给具体实例发送消息，直接针对某个特定对象
		/// 暂不支持
		/// </summary>
		Instance = 0,

		/// <summary>
		/// 给某类的所有组件实例发送消息，作用于该类型的全部实例。
		/// </summary>
		Group,

		/// <summary>
		/// 给全局范围发送消息，所有监听该消息的对象都会收到。
		/// </summary>
		Global
	};

	typedef void*(__cdecl*OnInvoke)(void* _in, int size);
	typedef void(__cdecl* OnNotify)(void* _in, int size);
	typedef void(__cdecl* OnSubscribe)(void* _in, int size);
	typedef void(__cdecl* OnPush)(void* _in, int size);
	typedef void(__cdecl* OnFreeVoidPtr)(void* _in);
	typedef void(__cdecl  InvokeCallback)(RET_CALL, void*, int);

	API_EXPORT bool      InitClient(const char* in_msg, int size, const char* log_path, int log_path_size, int protocol_level = 1, 
		                            double del_log_cycle = 3.0, bool bdetaillog = false);
	API_EXPORT bool      InitClientForC(const char* in_msg, int size, const char* log_path, int log_path_size, double del_log_cycle = 3.0, bool bdetaillog = false);
	API_EXPORT void      InvokeAsync(void* _in, InvokeCallback callback, int timeout = 30000);
	API_EXPORT void      InvokeWidgetAsync(const char *Instanceid, InvokeType type, void* _in, bool currentPage, InvokeCallback callback, int timeout = 30000);
	API_EXPORT RET_CALL  Invoke(void* _in, void** _out, int* _out_size, int timeout = 30000);
	API_EXPORT RET_CALL  InvokeWidget(const char* Instanceid, InvokeType type, void* _in, bool currentPage, void** _out, int* _out_size, int timeout = 30000);
	API_EXPORT RET_CALL  Notify(void* _in);
	API_EXPORT RET_CALL  NotifyWidget(const char* Instanceid, InvokeType type, void* _in, bool currentPage);
	API_EXPORT RET_CALL  Subscribe(void* _in);
	API_EXPORT RET_CALL  Push(void* _in);
	API_EXPORT void*     CreateRpcRequest(const char* id, const char* method, const char* param);
	API_EXPORT void*     CreateRpcPush(const char* topic, const char* param);
	API_EXPORT void      FreeRpcAllocMemory(void* _in);
	API_EXPORT void      ExitApp();
	API_EXPORT void      Register(OnInvoke invoke, OnNotify notify, OnSubscribe subscribe, OnPush push, OnFreeVoidPtr FreeVoidPtr);
}

// 接收数据task
struct Task {
	std::string action;
	std::string uuid;
	std::string msg;
};

class PipeClient
{
public:
	PipeClient(const std::string& pipename, const std::string& logfile);
	~PipeClient();

	bool Connect();
	void Disconnect();
	bool Isconnected();

	std::string Invoke(void* _in, int timeout /*= 30000*/);
	bool SendMsg(const std::string& action, const std::string& uuid, void* param);
	void DeseriesBusiparam(void* _pIn, const std::string& action, const std::string& msg);
	void ExitApp();

private:
	void RecieveLoop();
	void HandleRecieve();
	void HandleTask(const Task& task);
	void AddTask(const std::string& action, const std::string& uuid, const std::string& msg);
	bool WriteToPiPe(const std::string head, const std::string& uuid, const std::string& msg);
	int  SeriesSendData(std::string& resp, const std::string& action, void* param);

private:
	PipeClient(const PipeClient&)     = delete;
	void operator=(const PipeClient&) = delete;

private:
	HANDLE            _pipeHandle = INVALID_HANDLE_VALUE;
	std::string       _pipeName;
	std::mutex        _pipeMutex;
	std::atomic<bool> _connected{false};
	
	std::thread       _readThread;
	std::atomic<bool> _stopRead{false};

	std::thread             _recvTaskThread;
	std::queue<Task>        _recvTaskQueue;
	std::mutex              _recvTaskMutex;
	std::condition_variable _recvTaskCondition;
	std::atomic<bool>       _recvTaskStop{false};

	std::mutex                                       _requestMutex;
	std::map<std::string, std::promise<std::string>> _pendingRequest;

	bool              _bRelease;
};
