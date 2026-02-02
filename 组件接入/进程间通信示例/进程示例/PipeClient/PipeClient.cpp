#include "PipeClient.h"
#include <iostream>
#include "app.h"
#include <sstream>
#include <iomanip>
#include <iostream>
#include <list>

#define HEADER_SIZE 64
#define ACTION_SIZE 4
#define LENGTH_SIZE 8
#define PRE_SIZE    12
#define UUID_SIZE   36

#define ACTION_INVK "INVK"
#define ACTION_RESP "RESP"
#define ACTION_NOTF "NOTF"
#define ACTION_SUBS "SUBS"
#define ACTION_PUSH "PUSH"

static OnInvoke        s_oninvoke        = nullptr;
static OnNotify        s_onnotify        = nullptr;
static OnPush          s_onpush          = nullptr;
static OnSubscribe     s_onsubscribe     = nullptr;
static OnFreeVoidPtr   s_onFreeVoidPtr   = nullptr;

static std::mutex  s_pipeClient_mutex;
static PipeClient* s_pipeClient       = nullptr;
static int         s_protocol_level   = 1;
static bool        s_detaillog        = false;
static double      s_delete_log_cycle = 3.0f;
void TimerThread(const std::string& strPath, double delete_log_cycle) {
	// 定义定时器周期（25分钟）
	std::chrono::minutes interval(25);

	while (true) {
		// 创建并分离线程
		std::thread thread([](const std::string& strPath, double delete_log_cycle) {
			app::CleanLogFile(strPath, delete_log_cycle);
			}, strPath, delete_log_cycle);

		thread.detach();

		// 等待10分钟
		std::this_thread::sleep_for(interval);
	}
}

PipeClient::PipeClient(const std::string& pipeName, const std::string& logfile) : _bRelease(false)
{
#ifdef _WIN32
    _pipeName = R"(\\.\pipe\)" + pipeName;
#else
    // Unix Domain Socket path (compatible with C# NamedPipeServerStream)
    // C# NamedPipeServerStream on Linux uses Unix Domain Socket
    // If pipeName doesn't contain '/', it's just a name, we'll handle it in Connect()
    if (pipeName.find('/') == std::string::npos) {
        _pipeName = pipeName;  // Store as-is, Connect() will prepend /tmp/ if needed
    } else {
        _pipeName = pipeName;  // Full path provided
    }
#endif
    std::string log_file = app::addTimestampToFilename(logfile);
    app::g_logQueue.set_basefile(log_file);	

    // 启动定时器线程
    std::string parentPath = app::extractParentPath(logfile);
    std::thread timer_thread(TimerThread, parentPath, s_delete_log_cycle);
    timer_thread.detach();
}

PipeClient::~PipeClient()
{
	if (_bRelease)
	{
		ExitApp();
	}
}

void PipeClient::ExitApp()
{
	_bRelease = true;

	{
		std::lock_guard<std::mutex> lock(_recvTaskMutex);
		_recvTaskStop = true;
	}

	// 断开管道
	Disconnect();
}

bool PipeClient::Connect()
{
	std::lock_guard<std::mutex> lock(_pipeMutex);

	if (_pipeHandle != INVALID_PIPE_HANDLE)
	{
		Disconnect();
	}

#ifdef _WIN32
	_pipeHandle = CreateFileA(
		_pipeName.c_str(),
		GENERIC_READ | GENERIC_WRITE,
		0,
		nullptr,
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED,
		nullptr);

	if (_pipeHandle == INVALID_HANDLE_VALUE)
	{
		DWORD le = GetLastError();
		app::RecordInfo("Err-connect CreatePipe failed errcode: %d  pipename: %s", le, _pipeName.c_str());
		return false;
	}

	app::RecordInfo("Info-connect success pipename: %s", _pipeName.c_str());

	DWORD mode = PIPE_READMODE_BYTE/* | PIPE_WAIT*/;
	if (!SetNamedPipeHandleState(_pipeHandle, &mode, nullptr, nullptr))
	{
		CloseHandle(_pipeHandle);
		_pipeHandle = INVALID_HANDLE_VALUE;
		return false;
	}
#else
	// Linux Unix Domain Socket connection (compatible with C# NamedPipeServerStream)
	_pipeHandle = socket(AF_UNIX, SOCK_STREAM, 0);
	if (_pipeHandle == INVALID_PIPE_HANDLE)
	{
		int err = errno;
		app::RecordInfo("Err-connect socket failed errcode: %d  pipename: %s", err, _pipeName.c_str());
		return false;
	}

	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	
	// Handle pipe name format: C# (.NET 7) may use just the name or full path
	std::string socketPath = _pipeName;
	if (socketPath.find('/') == std::string::npos) {
		// If no path separator, assume it's just a name
		// .NET 7 NamedPipeServerStream on Linux creates socket files like:
		// /tmp/.NET-Core-Pipe-{name}-{pid}-{random} or /tmp/{name}
		// Try /tmp/{name} first (simpler format)
		socketPath = "/tmp/" + socketPath;
	}
	
	// .NET 7 NamedPipeServerStream on Linux creates socket files with specific naming
	// The actual socket file might be: /tmp/.NET-Core-Pipe-{name}-{pid}-{random}
	// But when connecting, we can use the simple name and let the system resolve it
	// If connection fails, the user should check the actual socket file path
	strncpy(addr.sun_path, socketPath.c_str(), sizeof(addr.sun_path) - 1);
	addr.sun_path[sizeof(addr.sun_path) - 1] = '\0';

	if (connect(_pipeHandle, (struct sockaddr*)&addr, sizeof(addr)) == -1)
	{
		int err = errno;
		app::RecordInfo("Err-connect failed errcode: %d  pipename: %s (tried: %s)", err, _pipeName.c_str(), socketPath.c_str());
		close(_pipeHandle);
		_pipeHandle = INVALID_PIPE_HANDLE;
		return false;
	}

	app::RecordInfo("Info-connect success pipename: %s (socket: %s)", _pipeName.c_str(), socketPath.c_str());
#endif

	_connected  = true;
	_stopRead   = false;
	_readThread = std::thread(&PipeClient::RecieveLoop, this);
	_readThread.detach();
	_recvTaskThread = std::thread(&PipeClient::HandleRecieve, this);
	_recvTaskThread.detach();
	return true;
}

void PipeClient::Disconnect()
{
	std::lock_guard<std::mutex> lock(_pipeMutex);

	_stopRead  = true;
	_connected = false;

	if (_pipeHandle != INVALID_PIPE_HANDLE)
	{
#ifdef _WIN32
		// 取消所有未完成的异步 I/O 操作
		CancelIoEx(_pipeHandle, nullptr);
		CloseHandle(_pipeHandle);
#else
		// Close Linux Unix Domain Socket
		shutdown(_pipeHandle, SHUT_RDWR);
		close(_pipeHandle);
#endif
		_pipeHandle = INVALID_PIPE_HANDLE;
	}
}

bool PipeClient::Isconnected()
{
	std::lock_guard<std::mutex> lock(_pipeMutex);
#ifdef _WIN32
	return _connected;
#else
	return _connected && _pipeHandle != INVALID_PIPE_HANDLE;
#endif
}

void PipeClient::RecieveLoop()
{
	while (!_stopRead)
	{
		// 读取消息头（固定为64字节），读取消息体
		// 协议格式："方法（4位）|data长度（8位）|预留符（12位）|uuid（36位，包含-）|数据"
		// 示例："INVK|00000013|000000000000|d736df35-3d89-4cc8-8520-d6789cc49bd3|msgfromserver"

		try
		{
			std::string action, uuid, msg, header;
#ifdef _WIN32
			{
				OVERLAPPED over = { 0 };
				over.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

				DWORD bytesRead = 0;

				//app::RecordInfo("Info-recv start...");
				char* charData = nullptr;
				char buffer[HEADER_SIZE + 1];
				ReadFile(_pipeHandle, buffer, HEADER_SIZE, &bytesRead, &over);
				if (GetLastError() == ERROR_IO_PENDING)
				{
					if (!GetOverlappedResult(_pipeHandle, &over, &bytesRead, TRUE))
					{
						DWORD le = GetLastError();

						if (!_stopRead)
						{
							app::RecordInfo("Err-recv head failed errcode: %d", le);
						}

						if (le == 109 || le == 183)
						{
							// 管道破坏不再处理
							CloseHandle(over.hEvent);
							CloseHandle(_pipeHandle);
							_pipeHandle = INVALID_HANDLE_VALUE;
							return;
						}
						else
						{
							continue;
						}
					}
				}

				buffer[HEADER_SIZE] = '\0';
				header = std::string(buffer, HEADER_SIZE);

				if (header.size() == HEADER_SIZE)
				{
					// 解析消息头
					// 方法（4位）
					action = header.substr(0, ACTION_SIZE);

					// 数据长度（8位）
					std::string strdataLength = header.substr(ACTION_SIZE + 1, LENGTH_SIZE);
					int dataLength = std::stoi(strdataLength);

					// 预留符（12位）
					std::string pre = header.substr(ACTION_SIZE + LENGTH_SIZE + 2, PRE_SIZE);

					// UUID（36位）
					uuid = header.substr(ACTION_SIZE + LENGTH_SIZE + PRE_SIZE + 3, UUID_SIZE);
					

					OVERLAPPED overData = { 0 };
					overData.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

					charData = new char[dataLength + 1];
					ReadFile(_pipeHandle, charData, dataLength, &bytesRead, &overData);
					if (GetLastError() == ERROR_IO_PENDING)
					{
						if (!GetOverlappedResult(_pipeHandle, &overData, &bytesRead, TRUE))
						{
							DWORD le = GetLastError();
							if (!_stopRead)
							{
								app::RecordInfo("Err-recv msg failed errcode: %d", le);
							}

							if (le == 109 || le == 183)
							{
								// 管道破坏不再处理
								CloseHandle(over.hEvent);
								CloseHandle(overData.hEvent);
								CloseHandle(_pipeHandle);
								_pipeHandle = INVALID_HANDLE_VALUE;
								return;
							}
							else
							{
								continue;
							}
						}
					}

					charData[dataLength] = '\0';
					msg = charData;

					delete[] charData;
					charData = nullptr;

					if (msg.size() != dataLength)
					{
						app::RecordInfo("Err-recv msg size msg: %s", msg.c_str());

						CloseHandle(overData.hEvent);
						CloseHandle(over.hEvent);
						continue;
					}
				}

				CloseHandle(over.hEvent);
			}
#else
			{
				char buffer[HEADER_SIZE + 1];
				ssize_t bytesRead = 0;
				size_t totalRead = 0;

				// 读取消息头
				while (totalRead < HEADER_SIZE && !_stopRead)
				{
					bytesRead = read(_pipeHandle, buffer + totalRead, HEADER_SIZE - totalRead);
					if (bytesRead <= 0)
					{
						if (errno == EINTR) {
							continue;
						}
						if (bytesRead == 0)
						{
							// EOF - pipe closed
							if (!_stopRead)
							{
								app::RecordInfo("Err-recv head pipe closed (EOF)");
							}
							close(_pipeHandle);
							_pipeHandle = INVALID_PIPE_HANDLE;
							return;
						}
						if (!_stopRead)
						{
							app::RecordInfo("Err-recv head failed errcode: %d", errno);
						}
						if (errno == EPIPE || errno == ECONNRESET)
						{
							// Broken pipe or connection reset (Unix Domain Socket)
							close(_pipeHandle);
							_pipeHandle = INVALID_PIPE_HANDLE;
							return;
						}
						return;
					}
					totalRead += bytesRead;
				}

				if (_stopRead) {
					return;
				}

				buffer[HEADER_SIZE] = '\0';
				header = std::string(buffer, HEADER_SIZE);

				if (header.size() == HEADER_SIZE)
				{
					// 解析消息头
					action = header.substr(0, ACTION_SIZE);
					std::string strdataLength = header.substr(ACTION_SIZE + 1, LENGTH_SIZE);
					int dataLength = std::stoi(strdataLength);
					std::string pre = header.substr(ACTION_SIZE + LENGTH_SIZE + 2, PRE_SIZE);
					uuid = header.substr(ACTION_SIZE + LENGTH_SIZE + PRE_SIZE + 3, UUID_SIZE);

					// 读取消息体
					char* charData = new char[dataLength + 1];
					totalRead = 0;
					while (totalRead < dataLength && !_stopRead)
					{
						bytesRead = read(_pipeHandle, charData + totalRead, dataLength - totalRead);
						if (bytesRead <= 0)
						{
							if (errno == EINTR) {
								continue;
							}
							if (bytesRead == 0)
							{
								// EOF - pipe closed
								if (!_stopRead)
								{
									app::RecordInfo("Err-recv msg pipe closed (EOF)");
								}
								delete[] charData;
								close(_pipeHandle);
								_pipeHandle = INVALID_PIPE_HANDLE;
								return;
							}
							if (!_stopRead)
							{
								app::RecordInfo("Err-recv msg failed errcode: %d", errno);
							}
							if (errno == EPIPE || errno == ECONNRESET)
							{
								// Broken pipe or connection reset (Unix Domain Socket)
								delete[] charData;
								close(_pipeHandle);
								_pipeHandle = INVALID_PIPE_HANDLE;
								return;
							}
							delete[] charData;
							return;
						}
						totalRead += bytesRead;
					}

					if (_stopRead) {
						delete[] charData;
						return;
					}

					charData[dataLength] = '\0';
					msg = std::string(charData, dataLength);
					delete[] charData;

					if (msg.size() != dataLength)
					{
						app::RecordInfo("Err-recv msg size msg: %s", msg.c_str());
						continue;
					}
				}
			}
#endif

			if (!header.empty())
			{
				if (s_detaillog)
				{
					app::RecordInfo("Info-recv %s%s", header.c_str(), msg.c_str());
				}
				else
				{
					app::RecordInfo("Info-recv %s", header.c_str());
				}
			}

			// 处理消息
			AddTask(action.c_str(), uuid.c_str(), msg.c_str());
		}
		catch (const std::exception& e)
		{
			app::RecordInfo("Err-exception recv err: %s", std::string(e.what()).c_str());
		}
	}
}


void PipeClient::AddTask(const std::string& action, const std::string& uuid, const std::string& msg) 
{
	{
		std::lock_guard<std::mutex> lock(_recvTaskMutex);
		_recvTaskQueue.push({ action, uuid, msg });
	}
	_recvTaskCondition.notify_one();
}

void PipeClient::HandleRecieve()
{
	while (!_recvTaskStop) {
		std::queue<Task> doTask;
		{
			std::unique_lock<std::mutex> lock(_recvTaskMutex);
			_recvTaskCondition.wait(lock, [this] { return !_recvTaskQueue.empty() || _recvTaskStop; });
			if (_recvTaskStop) {
				break;
			}

			if (_recvTaskQueue.empty())
			{
				continue;
			}
			
			while (!_recvTaskQueue.empty())
			{
				doTask.push(_recvTaskQueue.front());
				_recvTaskQueue.pop();
			}
		}

		while (!doTask.empty())
		{
			HandleTask(doTask.front());
			doTask.pop();
		}
	}
}

void PipeClient::HandleTask(const Task& task)
{
	try
	{
		// 处理任务
		if (task.action == ACTION_INVK)
		{
			if (s_oninvoke)
			{
				if (s_protocol_level == 2)
				{
					RpcResponse  _out;
					char* presp = (char*)s_oninvoke((void*)task.msg.c_str(), task.msg.size());
					if (presp)
					{
						DeseriesBusiparam(&_out, ACTION_RESP, presp);
						s_onFreeVoidPtr(presp);

						SendMsg(ACTION_RESP, task.uuid, &_out);
					}
				}
				else
				{
					RpcRequest  _in;
					DeseriesBusiparam(&_in, task.action, task.msg);
					RpcResponse* _out = static_cast<RpcResponse*>(s_oninvoke(&_in, 0));
					if (_out)
					{
						SendMsg(ACTION_RESP, task.uuid, _out);
						s_onFreeVoidPtr(_out);
					}
				}
			}
		}
		else if (task.action == ACTION_RESP)
		{
			std::lock_guard<std::mutex> lock(_requestMutex);
			auto it = _pendingRequest.find(task.uuid);
			if (it != _pendingRequest.end())
			{
				it->second.set_value(task.msg);
				_pendingRequest.erase(it);
			}
		}
		else if (task.action == ACTION_NOTF)
		{
			if (s_onnotify)
			{
				if (s_protocol_level == 2)
				{
					s_onnotify((void*)task.msg.c_str(), task.msg.size());
				}
				else
				{
					RpcRequest _in;
					DeseriesBusiparam(&_in, task.action, task.msg);
					s_onnotify(&_in, 0);
				}
			}
		}
		else if (task.action == ACTION_SUBS)
		{
			if (s_onsubscribe)
			{
				if (s_protocol_level == 2)
				{
					s_onsubscribe((void*)task.msg.c_str(), task.msg.size());
				}
				else
				{
					RpcRequest  _in;
					DeseriesBusiparam(&_in, task.action, task.msg);
					s_onsubscribe(&_in, 0);
				}
			}
		}
		else if (task.action == ACTION_PUSH)
		{
			if (s_onpush)
			{
				if (s_protocol_level == 2)
				{
					s_onpush((void*)task.msg.c_str(), task.msg.size());
				}
				else
				{
					RpcPush  _in;
					DeseriesBusiparam(&_in, task.action, task.msg);
					s_onpush(&_in, 0);
				}
			}
		}
		else
		{
			app::RecordInfo("Err-recv action valiad action: %s", task.action.c_str());
		}
	}
	catch (const std::exception& e)
	{
		app::RecordInfo("Err-exception handle task err: %s", std::string(e.what()).c_str());
	}	
}

std::string PipeClient::Invoke(void* _in, int timeout /*= 30000*/)
{
	if (!_in)
	{
		app::RecordInfo("Info-send fail param is null");
		return "ERR:INVK:SENDFALSE";
	}

	std::string uuid = app::GetUuid();
	std::promise<std::string> promise;
	std::future<std::string> future = promise.get_future();

	{
		std::lock_guard<std::mutex> lock(_requestMutex);
		_pendingRequest[uuid] = std::move(promise);
	}

	if (SendMsg(ACTION_INVK, uuid, _in))
	{
		if (future.wait_for(std::chrono::milliseconds(timeout)) == std::future_status::timeout)
		{
			{
				std::lock_guard<std::mutex> lock(_requestMutex);
				_pendingRequest.erase(uuid);
			}

			app::RecordInfo("Err-invk timeout");
			return "ERR:INVK:TIMEOUT";
		}

		return std::move(future.get());
	}
	else
	{
		std::lock_guard<std::mutex> lock(_requestMutex);
		_pendingRequest.erase(uuid);
		return "ERR:INVK:SENDFALSE";
	}
}

bool PipeClient::SendMsg(const std::string& action, const std::string& uuid, void* param)
{
	if (!param)
	{
		app::RecordInfo("Info-send fail param is null");
		return false;
	}

	std::string resp;
	int size = SeriesSendData(resp, action, param);

	std::ostringstream oss;
	oss << action << "|"
		<< std::setw(8) << std::setfill('0') << size << "|"
		<< std::string(PRE_SIZE, '0') << "|"
		<< uuid << "|";

	std::string head = oss.str();

	return WriteToPiPe(head, uuid, std::move(head + resp));
}

bool PipeClient::WriteToPiPe(const std::string head, const std::string& uuid, const std::string& msg)
{
	app::RecordInfo("Info-send begin %s", s_detaillog ? msg.c_str() : head.c_str());

	bool bRet = true;
#ifdef _WIN32
	{
		OVERLAPPED over = { 0 };
		over.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

		DWORD bytesWritten;
		bool result = WriteFile(_pipeHandle, msg.c_str(), (DWORD)msg.size(), &bytesWritten, &over);
		if (GetLastError() == ERROR_IO_PENDING)
		{
			if (!GetOverlappedResult(_pipeHandle, &over, &bytesWritten, TRUE))
			{
				DWORD le = GetLastError();
				app::RecordInfo("Err-send write pipe errcode: %d", le);

				if (le == 109 || le == 183)
				{
					// 管道破坏不再处理
					CloseHandle(over.hEvent);
					CloseHandle(_pipeHandle);
					_pipeHandle = INVALID_HANDLE_VALUE;
					return false;
				}
			}
		}

		CloseHandle(over.hEvent);

		if (!bRet)
		{
			return bRet;
		}
	}
#else
	{
		ssize_t bytesWritten = 0;
		size_t totalWritten = 0;
		const char* data = msg.c_str();
		size_t dataSize = msg.size();

		while (totalWritten < dataSize)
		{
			bytesWritten = write(_pipeHandle, data + totalWritten, dataSize - totalWritten);
			if (bytesWritten < 0)
			{
				if (errno == EINTR) {
					continue;
				}
				app::RecordInfo("Err-send write pipe errcode: %d", errno);
				if (errno == EPIPE || errno == ECONNRESET)
				{
					// Broken pipe or connection reset (Unix Domain Socket)
					close(_pipeHandle);
					_pipeHandle = INVALID_PIPE_HANDLE;
					return false;
				}
				return false;
			}
			totalWritten += bytesWritten;
		}
	}
#endif

	app::RecordInfo("Info-send succ uuid: %s", uuid.c_str());

	return bRet;
}

int PipeClient::SeriesSendData(std::string& resp, const std::string& action, void* param)
{
	if (!param)
	{
		app::RecordInfo("series param is null");
		return 0;
	}	

	Json::Value js;

	if (action == ACTION_INVK || action == ACTION_NOTF || action == ACTION_SUBS) {
		RpcRequest* request = static_cast<RpcRequest*>(param);
		js["id"] = request->id;
		js["method"] = request->method;
		if (!request->param.isNull()) {
			js["param"] = request->param;
		}
	}
	else if (action == ACTION_RESP) {
		RpcResponse* resp = static_cast<RpcResponse*>(param);
		js["id"] = resp->id;
		js["code"] = resp->code;
		if (!resp->error.isNull()) {
			js["error"] = resp->error;
		}
		if (!resp->result.isNull()) {
			js["result"] = resp->result;
		}
	}
	else if (action == ACTION_PUSH) {
		RpcPush* push = static_cast<RpcPush*>(param);
		js["topic"] = push->topic;
		if (!push->param.isNull()) {
			js["param"] = push->param;
		}
	}

	Json::FastWriter writer;
	resp = writer.write(js);
	if (!resp.empty() && resp.back() == '\n') {
		resp.pop_back();  // 去掉最后一个字符
	}

	//resp = std::move(js.toStyledString());
	return (int)resp.size();
}

void PipeClient::DeseriesBusiparam(void* _pIn, const std::string& action, const std::string& msg)
{
	if (!_pIn)
	{
		app::RecordInfo("deseries _pIn is null");
		return;
	}

	Json::Value js;
	Json::Reader reader;

	bool parseResult = reader.parse(msg, js);
	if (!parseResult) {
		// 处理解析错误
		app::RecordInfo("deseries _pIn Failed to parse JSON");
		return; // 或者抛出异常
	}

	if (action == ACTION_INVK || action == ACTION_NOTF || action == ACTION_SUBS) {
		RpcRequest* request = static_cast<RpcRequest*>(_pIn);
		// 处理 id
		if (js.isMember("id") && !js["id"].isNull()) {
			request->id = js["id"].asString();
		}
		else {
			request->id = "";
		}

		// 处理 method
		if (js.isMember("method") && !js["method"].isNull()) {
			request->method = js["method"].asString();
		}
		else {
			request->method = "";
		}

		// 处理 param
		if (js.isMember("param") && !js["param"].isNull()) {
			request->param = js["param"];
		}
		else {
			request->param = Json::Value();
		}
	}
	else if (action == ACTION_RESP) {
		RpcResponse* resp = static_cast<RpcResponse*>(_pIn);
		// 处理 id
		if (js.isMember("id") && !js["id"].isNull()) {
			resp->id = js["id"].asString();
		}
		else {
			resp->id = "";
		}

		// 处理 code
		if (js.isMember("code") && !js["code"].isNull()) {
			resp->code = js["code"].asInt();
		}
		else {
			resp->code = -777;
		}

		// 处理 error
		if (js.isMember("error") && !js["error"].isNull()) {
			resp->error = js["error"];
		}
		else {
			resp->error = Json::Value();
		}

		// 处理 result
		if (js.isMember("result") && !js["result"].isNull()) {
			resp->result = js["result"];
		}
		else {
			resp->result = Json::Value();
		}
	}
	else if (action == ACTION_PUSH) {
		RpcPush* push = static_cast<RpcPush*>(_pIn);
		// 处理 topic
		if (js.isMember("topic") && !js["topic"].isNull()) {
			push->topic = js["topic"].asString();
		}
		else {
			push->topic = "";
		}

		// 处理 param
		if (js.isMember("param") && !js["param"].isNull()) {
			push->param = js["param"];
		}
		else {
			push->param = Json::Value();
		}
	}
}

API_EXPORT bool InitClient(const char* in_msg, int size, const char* log_path, int log_path_size, int protocol_level /*= 1*/, 
	double del_log_cycle /*= 3.0*/, bool bdetaillog /*= false*/)
{
	std::lock_guard<std::mutex> lock(s_pipeClient_mutex);

	if (!s_pipeClient)
	{
		s_protocol_level = protocol_level;
		s_delete_log_cycle = del_log_cycle;
		s_detaillog = bdetaillog;
		s_pipeClient = new PipeClient(std::string(in_msg, size), std::string(log_path, log_path_size));
		if (s_pipeClient->Connect())
		{
			return true;
		}
		else
		{
			app::RecordInfo("Err-connect failed errcode");

			delete s_pipeClient;
			s_pipeClient = nullptr;
			return false;
		}
	}

	return true;
}

API_EXPORT bool InitClientForC(const char* in_msg, int size, const char* log_path, int log_path_size, double del_log_cycle /*= 3.0*/, bool bdetaillog /*= false*/)
{
	return InitClient(in_msg, size, log_path, log_path_size, 1, del_log_cycle, bdetaillog);
}

API_EXPORT void Register(OnInvoke invoke, OnNotify notify, OnSubscribe subscribe, OnPush push, OnFreeVoidPtr FreeVoidPtr)
{
	s_oninvoke        = invoke;
	s_onnotify        = notify;
	s_onpush          = push;
	s_onsubscribe     = subscribe;
	s_onFreeVoidPtr   = FreeVoidPtr;
}

// 全局队列和线程控制
std::queue<std::function<void()>> g_taskQueueInvoke;
std::mutex g_mutexInvoke;
std::condition_variable g_conditionInvoke;
bool g_stopThreadInvoke = false;

// 程序退出时停止线程


// 固定线程函数
void AsyncInvokeWorker()
{
	while (true)
	{
		std::function<void()> task;
		{
			std::unique_lock<std::mutex> lock(g_mutexInvoke);
			g_conditionInvoke.wait(lock, [] { return !g_taskQueueInvoke.empty() || g_stopThreadInvoke; });

			if (g_stopThreadInvoke)
			{
				return;	// 进程退出则队列中的任务不再处理
			}

			task = std::move(g_taskQueueInvoke.front());
			g_taskQueueInvoke.pop();
		}

		if (task)
		{
			task();

			// 在任务执行完成后立即处理下一个任务
			while (true)
			{
				std::function<void()> nextTask;
				{
					std::unique_lock<std::mutex> lock(g_mutexInvoke);
					if (g_stopThreadInvoke)
					{
						return;	// 进程退出则队列中的任务不再处理
					}

					if (!g_taskQueueInvoke.empty())
					{
						nextTask = std::move(g_taskQueueInvoke.front());
						g_taskQueueInvoke.pop();
					}
					else
					{
						break; // 如果队列为空，退出循环
					}
				}

				if (nextTask)
				{
					nextTask(); // 执行下一个任务
				}
			}
		}
	}
}

void PackWidgetRequset(const char* Instanceid, InvokeType type, void* _in, bool currentPage)
{
	RpcRequest* request = static_cast<RpcRequest*>(_in);
	if (request)
	{
		Json::Value widgetParam;
		widgetParam["method"] = request->method;
		widgetParam["type"] = (int)type;
		widgetParam["currentPage"] = currentPage;
		widgetParam["instanceid"] = Instanceid == nullptr ? "" : Instanceid;

		request->param["Cosmos:WidgetComunication:Invoke"] = widgetParam;
		request->method = "Cosmos:WidgetComunication:Invoke";
	}
}

// 单例线程
std::thread g_asyncThread(AsyncInvokeWorker);

void StopAsyncInvokeWorker()
{
	{
		std::lock_guard<std::mutex> lock(g_mutexInvoke);
		g_stopThreadInvoke = true;
	}

	// 进程退出则队列中的任务不再处理
	g_conditionInvoke.notify_all();
	if (g_asyncThread.joinable())
	{
		g_asyncThread.detach();
	}
}

// 修改后的 AsyncInvoke 函数
API_EXPORT void InvokeAsync(void* _in, InvokeCallback callback, int timeout /*= 30000*/)
{
	auto task = [_in, callback, timeout]() {
		RpcRequest* request = static_cast<RpcRequest*>(_in);
		if (request)
		{
			RpcRequest* param = new RpcRequest(*request);
			RpcResponse* resp = new RpcResponse();
			void* out = resp;
			int out_size = 0;
			RET_CALL result = Invoke(param, &out, &out_size, timeout);
			FreeRpcAllocMemory(param);

			// 调用回调函数，传递结果
			callback(result, out, out_size);
			FreeRpcAllocMemory(out);
		}
	};

	{
		std::lock_guard<std::mutex> lock(g_mutexInvoke);
		g_taskQueueInvoke.push(std::move(task));
	}

	g_conditionInvoke.notify_one();
}

API_EXPORT void  InvokeWidgetAsync(const char* Instanceid, InvokeType type, void* _in, bool currentPage, InvokeCallback callback, int timeout)
{
	PackWidgetRequset(Instanceid, type, _in, currentPage);
	InvokeAsync(_in, callback, timeout);
}

API_EXPORT RET_CALL Invoke(void* _in, void** _out, int* _out_size, int timeout)
{
	RET_CALL ret_invk = RET_CALL::Exception;

	try
	{
		if (s_pipeClient)
		{
			std::string strRet = std::move(s_pipeClient->Invoke(_in, timeout));
			if (strRet == "ERR:INVK:TIMEOUT")
			{
				RpcRequest* request = static_cast<RpcRequest*>(_in);
				Json::Value js;
				js["id"] = request->id;
				js["code"] = -888;
				js["error"]["msg"] = strRet;
				js["result"] = Json::Value();  // 空 JSON 对象
				//std::string strRet = js.toStyledString();
				Json::FastWriter writer;
				std::string strRet = writer.write(js);
				if (!strRet.empty() && strRet.back() == '\n') {
					strRet.pop_back();  // 去掉最后一个字符
				}
				ret_invk = RET_CALL::Timeout;
			}
			else if (strRet == "ERR:INVK:SENDFALSE")
			{
				RpcRequest* request = static_cast<RpcRequest*>(_in);
				Json::Value js;
				js["id"] = request->id;
				js["code"] = -999;
				js["error"]["msg"] = strRet;
				js["result"] = Json::Value();  // 空 JSON 对象
				Json::FastWriter writer;
				std::string strRet = writer.write(js);
				if (!strRet.empty() && strRet.back() == '\n') {
					strRet.pop_back();  // 去掉最后一个字符
				}
				ret_invk = RET_CALL::Sendfail;
			}
			else
			{
				ret_invk = RET_CALL::Ok;
			}

			if (s_protocol_level == 2)
			{
				int size = strRet.size() + 1;
				char* cstr = new char[size];
#ifdef _WIN32
				strcpy_s(cstr, size, strRet.c_str());
#else
				strncpy(cstr, strRet.c_str(), size - 1);
				cstr[size - 1] = '\0';
#endif
				*_out = cstr;
				*_out_size = size;
			}
			else
			{
				s_pipeClient->DeseriesBusiparam(*_out, ACTION_RESP, strRet);
			}
		}
		else
		{
			ret_invk = RET_CALL::Pipenull;
			app::RecordInfo("Err-send invk pipeclient is null");
		}
	}
	catch (const std::exception& e)
	{
		app::RecordInfo("Err-send exception invoke failed errinfo: %s", std::string(e.what()).c_str());
	}

	return ret_invk;
}

API_EXPORT RET_CALL InvokeWidget(const char* Instanceid, InvokeType type, void* _in, bool currentPage, void** _out, int* _out_size, int timeout)
{
	PackWidgetRequset(Instanceid, type, _in, currentPage);
	return Invoke(_in, _out, _out_size, timeout);
}
API_EXPORT RET_CALL Notify(void* _in)
{
	RET_CALL ret_notf = RET_CALL::Exception;

	try
	{
		if (s_pipeClient)
		{
			ret_notf = s_pipeClient->SendMsg(ACTION_NOTF, app::GetUuid(), _in) ? RET_CALL::Ok : RET_CALL::Sendfail;
		}
		else
		{
			ret_notf = RET_CALL::Pipenull;
			app::RecordInfo("Err-send notf pipeclient is null");
		}
	}
	catch (const std::exception& e)
	{
		app::RecordInfo("Err-send notify failed errinfo: %s", std::string(e.what()).c_str());
	}

	return ret_notf;
}

API_EXPORT RET_CALL NotifyWidget(const char* Instanceid, InvokeType type, void* _in, bool currentPage)
{
	PackWidgetRequset(Instanceid, type, _in, currentPage);
	return Notify(_in);
}

API_EXPORT RET_CALL Subscribe(void* _in)
{
	RET_CALL ret_subs = RET_CALL::Exception;

	try
	{
		if (s_pipeClient)
		{
			ret_subs = s_pipeClient->SendMsg(ACTION_SUBS, app::GetUuid(), _in) ? RET_CALL::Ok : RET_CALL::Sendfail;
		}
		else
		{
			ret_subs = RET_CALL::Pipenull;
			app::RecordInfo("Err-send subs pipeclient is null");
		}
	}
	catch (const std::exception& e)
	{
		app::RecordInfo("Err-send subscribe failed errinfo: %s", std::string(e.what()).c_str());
	}

	return ret_subs;
}

API_EXPORT RET_CALL Push(void* _in)
{
	RET_CALL ret_push = RET_CALL::Exception;

	try
	{
		if (s_pipeClient)
		{
			ret_push = s_pipeClient->SendMsg(ACTION_PUSH, app::GetUuid(), _in) ? RET_CALL::Ok : RET_CALL::Sendfail;
		}
		else
		{
			ret_push = RET_CALL::Pipenull;
			app::RecordInfo("Err-send push pipeclient is null");
		}
	}
	catch (const std::exception& e)
	{
		app::RecordInfo("Err-send push failed errinfo: %s", std::string(e.what()).c_str());
	}

	return ret_push;
}

API_EXPORT void* CreateRpcRequest(const char* id, const char* method, const char* param)
{
	RpcRequest* req = new RpcRequest();
	req->id = id ? std::string(id) : "";
	req->method = method ? std::string(method) : "";

	Json::Value reqParam;
	if (param == nullptr) {
		reqParam = Json::Value();
	}
	else {
		std::string paramStr(param);
		Json::Reader reader;
		if (reader.parse(paramStr, reqParam)) {
			// 解析成功，reqParam 已经是解析后的 JSON 对象
		}
		else {
			// 处理解析错误
			app::RecordInfo("createrequest param Failed to parse JSON");
			reqParam = Json::Value(); // 或者设置为其他默认值
		}
	}

	req->param = reqParam;

	return static_cast<void*>(req);
}

API_EXPORT void* CreateRpcPush(const char* topic, const char* param)
{
	RpcPush* push = new RpcPush();
	push->topic = topic ? std::string(topic) : "";

	Json::Value pushParam;
	if (param == nullptr) {
		pushParam = Json::Value();
	}
	else {
		std::string paramStr(param);
		Json::Reader reader;
		if (reader.parse(paramStr, pushParam)) {
			// 解析成功，pushParam 已经是解析后的 JSON 对象
		}
		else {
			// 处理解析错误
			app::RecordInfo("createrepush param Failed to parse JSON");
			pushParam = Json::Value(); // 或者设置为其他默认值
		}
	}

	push->param = pushParam;

	return static_cast<void*>(push);
}

API_EXPORT void FreeRpcAllocMemory(void* _in)
{
	if (_in)
	{
		delete _in;
		_in = nullptr;
	}
}

API_EXPORT void ExitApp()
{
	StopAsyncInvokeWorker();

	if (s_pipeClient)
	{
		s_pipeClient->ExitApp();
	}

	// 停止日志队列，记录剩余日志
	app::g_logQueue.stop();
}

#ifdef _BUILD_AS_EXE
int main()
{
	return 0;
}


#endif // _BUILD_AS_EXE
