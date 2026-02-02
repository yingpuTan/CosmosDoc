#ifndef _GEN_UUID_H_
#define _GEN_UUID_H_

#pragma once
#ifdef _WIN32
#include <Windows.h>
#include <objbase.h>
#else
// Check if USE_UUID_LIB is defined by CMake, otherwise try to detect
#ifndef USE_UUID_LIB
    #ifdef __has_include
        #if __has_include(<uuid/uuid.h>)
            #include <uuid/uuid.h>
            #define USE_UUID_LIB 1
        #else
            #define USE_UUID_LIB 0
        #endif
    #else
        // Try to include, will fail if not available (install: sudo apt-get install uuid-dev)
        #include <uuid/uuid.h>
        #define USE_UUID_LIB 1
    #endif
#elif USE_UUID_LIB == 1
    #include <uuid/uuid.h>
#else
    // USE_UUID_LIB == 0, use fallback
#endif
// Include headers needed for fallback implementation
#if USE_UUID_LIB == 0
    #include <random>
    #include <sstream>
#endif
#endif

#include <string>
#include <fstream>
#include <stdarg.h>
#include <ctime>
#include <iomanip>
#include <chrono>
#include <regex>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <map>
#include <sstream>
#include <cstdlib>
#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <cstring>
#endif

namespace app
{
//
// GetUuid函数的跨平台实现
//
#ifdef _WIN32
    inline int Uuid2String(const GUID& uuid, std::string& strRet)
    {
        char   buf[64] = {0};
        int nLen = _snprintf_s(buf, sizeof(buf), sizeof(buf), "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
            uuid.Data1,
            uuid.Data2,
            uuid.Data3,
            uuid.Data4[0],uuid.Data4[1],
            uuid.Data4[2],uuid.Data4[3],
            uuid.Data4[4],uuid.Data4[5],
            uuid.Data4[6],uuid.Data4[7]);
        strRet = std::string(buf, nLen);
        return nLen;
    }

    inline int GetUuid(GUID& uuid, std::string* pRet)
    {
        //GUID& guid = uuid;
        //::CoInitialize(NULL);
        if (S_OK == ::CoCreateGuid(&uuid))
        {
            if(pRet != NULL)
            {
                Uuid2String(uuid, *pRet);
            }
        }
        //::CoUninitialize();
        return 0;
    }

    inline std::string GetUuid()
    {
        std::string strRet;
        GUID guid;
        GetUuid(guid, &strRet);
        return strRet;
    }
#else
    inline std::string GetUuid()
    {
#if defined(USE_UUID_LIB) && USE_UUID_LIB == 1
        // Linux返回标准UUID (使用uuid库)
        uuid_t uu;
        uuid_generate(uu);
        char uuid_str[37];
        uuid_unparse_lower(uu,uuid_str);
        return uuid_str;
#else
        // 回退实现：使用随机数生成UUID v4格式
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(0, 15);
        static std::uniform_int_distribution<> dis2(8, 11);
        
        std::stringstream ss;
        ss << std::hex;
        for (int i = 0; i < 8; i++) {
            ss << dis(gen);
        }
        ss << "-";
        for (int i = 0; i < 4; i++) {
            ss << dis(gen);
        }
        ss << "-4";
        for (int i = 0; i < 3; i++) {
            ss << dis(gen);
        }
        ss << "-";
        ss << dis2(gen);
        for (int i = 0; i < 3; i++) {
            ss << dis(gen);
        }
        ss << "-";
        for (int i = 0; i < 12; i++) {
            ss << dis(gen);
        }
        return ss.str();
#endif
    }
#endif

    std::string Format(const char* szFormat, ...)
    {
        std::string strResult;
        va_list ap;
        va_start(ap, szFormat);
        int buffer_size = vsnprintf(NULL, 0, szFormat, ap);
        va_end(ap);
        if (buffer_size > 0)
        {
            strResult.resize(buffer_size);
            va_start(ap, szFormat);
            vsnprintf((char*)strResult.data(), buffer_size + 1, szFormat, ap);
            va_end(ap);
        }
        else
        {
            // 错误处理
            // printf("%s\n", strerror(errno));
        }

        return strResult;
    }

    std::string GetSysWorkDir()
    {
#ifdef _WIN32
        static char g_szWorkDir[MAX_PATH] = { 0 };
#else
        static char g_szWorkDir[PATH_MAX] = { 0 };
#endif
        if (g_szWorkDir[0] == '\0')
        {
#ifdef _WIN32
            DWORD dwSize = GetModuleFileNameA(NULL, g_szWorkDir, MAX_PATH);
            for (int i = dwSize - 1; i >= 0; --i)
            {
                if (g_szWorkDir[i] == '\\')
                {
                    g_szWorkDir[i + 1] = '\0';
                    break;
                }
            }
#else
            ssize_t len = readlink("/proc/self/exe", g_szWorkDir, PATH_MAX - 1);
            if (len != -1)
            {
                g_szWorkDir[len] = '\0';
                for (int i = len - 1; i >= 0; --i)
                {
                    if (g_szWorkDir[i] == '/')
                    {
                        g_szWorkDir[i + 1] = '\0';
                        break;
                    }
                }
            }
            else
            {
                // Fallback to current directory
                if (getcwd(g_szWorkDir, PATH_MAX) == nullptr)
                {
                    g_szWorkDir[0] = '.';
                    g_szWorkDir[1] = '/';
                    g_szWorkDir[2] = '\0';
                }
            }
#endif
        }
        return g_szWorkDir;
    }

    // 解析路径，提取文件名和目录
    std::string extractParentPath(const std::string& path) {
        
        std::string parentPath = "";
        for (int i = path.length() - 1; i >= 0; --i) {
            if (path[i] == '/' || path[i] == '\\') {
                parentPath = path.substr(0, i);
                break;
            }
        }

        // 新的路径前缀
        std::string newPrefix = ".rpclogs/";

        // 重新组合文件路径
        if (!parentPath.empty()) {
            return parentPath + "/" + newPrefix;
        }
        else {
            return GetSysWorkDir() + newPrefix;
        }
    }

    std::string extractFilename(const std::string& path) {
        for (int i = path.length() - 1; i >= 0; --i) {
            if (path[i] == '/' || path[i] == '\\') {
                return path.substr(i + 1);
            }
        }
        return path; // 如果没有路径分隔符，返回整个字符串作为文件名
    }

    // 在文件名后添加时间戳
    std::string addTimestampToFilename(const std::string& fullPath) {
        std::string filename = extractFilename(fullPath);
        size_t dotPos = filename.find_last_of('.');
        std::string stem = filename.substr(0, dotPos);
        std::string extension = filename.substr(dotPos);

#ifdef _WIN32
        SYSTEMTIME tm = {};
        GetLocalTime(&tm);
        std::string strDate = Format("_%04d%02d%02d", tm.wYear, tm.wMonth, tm.wDay);
#else
        time_t now = time(nullptr);
        struct tm* tm_info = localtime(&now);
        std::string strDate = Format("_%04d%02d%02d", 
            tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday);
#endif
        std::string newFilename = stem + strDate + extension;

        std::string parentPath = extractParentPath(fullPath);
#ifdef _WIN32
        CreateDirectoryA(parentPath.c_str(), nullptr);
#else
        // Create directory recursively on Linux
        std::string cmd = "mkdir -p " + parentPath;
        system(cmd.c_str());
#endif
        return parentPath + newFilename;
    }
    uint64_t GetTime() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
    }

    int getLastFileCounter(const std::string& strFolder)
    {
#ifdef _WIN32
        SYSTEMTIME tm = {};
        GetLocalTime(&tm);
        std::string strDate = app::Format("%4d%02d%02d", tm.wYear, tm.wMonth, tm.wDay);

        WIN32_FIND_DATAA findData = {};
        std::string strCurrentPath = strFolder;

        // 如果找到了反斜杠，则返回上一级路径
        size_t lastSlashPos = strFolder.find_last_of("\\/");
        if (lastSlashPos != std::string::npos) {
            strCurrentPath = strFolder.substr(0, lastSlashPos + 1);
        }

        HANDLE hFindFile = FindFirstFileA((strCurrentPath + "*").c_str(), &findData);

        int iDexMax = 0;
        while (FindNextFileA(hFindFile, &findData))
        {
            std::string strFileNames = findData.cFileName;
            size_t pos = strFileNames.find(strDate);
            if (pos != std::string::npos)
            {
                bool bIsDirs = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
                if (bIsDirs)
                {
                    //文件是目录
                    continue;
                }

                // 提取序号部分
                std::regex pattern(R"(.*\d{8}_(\d{3})\.log)");
                std::smatch matches;

                if (std::regex_match(strFileNames, matches, pattern)) {
                    std::string index = matches[1].str();
                    if (iDexMax < atoi(index.c_str())) {
                        iDexMax = atoi(index.c_str());
                    }
                }
                else {
                    continue;
                }
            }
        }

        FindClose(hFindFile);

        return iDexMax;
#else
        time_t now = time(nullptr);
        struct tm* tm_info = localtime(&now);
        std::string strDate = app::Format("%4d%02d%02d", 
            tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday);

        std::string strCurrentPath = strFolder;
        size_t lastSlashPos = strFolder.find_last_of("\\/");
        if (lastSlashPos != std::string::npos) {
            strCurrentPath = strFolder.substr(0, lastSlashPos + 1);
        }

        int iDexMax = 0;
        DIR* dir = opendir(strCurrentPath.c_str());
        if (dir != nullptr) {
            struct dirent* entry;
            while ((entry = readdir(dir)) != nullptr) {
                std::string strFileNames = entry->d_name;
                size_t pos = strFileNames.find(strDate);
                if (pos != std::string::npos) {
                    struct stat fileStat;
                    std::string fullPath = strCurrentPath + strFileNames;
                    if (stat(fullPath.c_str(), &fileStat) == 0) {
                        if (S_ISDIR(fileStat.st_mode)) {
                            continue;
                        }
                    }

                    // 提取序号部分
                    std::regex pattern(R"(.*\d{8}_(\d{3})\.log)");
                    std::smatch matches;

                    if (std::regex_match(strFileNames, matches, pattern)) {
                        std::string index = matches[1].str();
                        if (iDexMax < atoi(index.c_str())) {
                            iDexMax = atoi(index.c_str());
                        }
                    }
                }
            }
            closedir(dir);
        }

        return iDexMax;
#endif
    }

    // 线程安全的日志队列
    class LogQueue {
    public:
        void set_basefile(const std::string& filename)
        {
            size_t pos = filename.find_last_of('.');
            if (pos == std::string::npos)
            {
                return;
            }

            _strBasename = filename.substr(0, pos);

            // 初始化日志写入线程
            _dotaskThread = std::thread(&LogQueue::write_logs, this);
            _pushThread = std::thread(&LogQueue::push_taskthread, this);
            //_logThread.detach();
        }

        void push(const std::string& logEntry) {            
            {
                std::lock_guard<std::mutex> lock(_pushmutex);
                _login++;
                _pushqueue.push(logEntry);
            }

            _pushCondition.notify_one();
        }

        bool push_to_dotaskqueue()
        {
            if (_pushqueue.empty())
            {
                return true;
            }

            int curpop = 0;
            while (!_pushqueue.empty())
            {
                {
                    std::unique_lock<std::mutex> lock(_dotaskmutex);
                    _logout++;
                    _dotaskqueue.push(_pushqueue.front());
                }

                _pushqueue.pop();
                curpop++;

                // 单次最多推入500条数据
                if (curpop == 500) {
                    break;
                }
            }

            if (curpop > 0)
            {
                _dotaskCondition.notify_one();
            }

            return false;
        }

        // 将管道日志数据推入队列（避免堵塞管道线程）
        void push_taskthread() {
            while (true) {
                if (_pushstop)
                {
                    std::unique_lock<std::mutex> lock(_pushmutex);
                    if (push_to_dotaskqueue())
                    {
                        return;
                    }
                }
                else
                {
                    std::unique_lock<std::mutex> lock(_pushmutex);
                    // 使用 wait_for 实现 1 秒超时
                    if (_pushCondition.wait_for(lock, std::chrono::seconds(1)) == std::cv_status::timeout)
                    {
                        // 1 秒超时，1秒检查一次是否有日志数据
                    }

                    push_to_dotaskqueue();
                }
            }
        }

        void get_logmsg(std::string& logEntry)
        {
            int curpop = 0;
            while (!_dotaskqueue.empty())
            {
                logEntry += logEntry.empty() ? _dotaskqueue.front() : ("\n" + _dotaskqueue.front());
                _dotaskqueue.pop();
                curpop++;

                // 单次最多记录100条数据
                if (curpop == 100) {
                    break;
                }
            }
        }

        // 日志写入线程
        void write_logs() {
            if (_strBasename.empty())
            {
                return;
            }

            int fileCounter = 0;
            {
                std::lock_guard<std::mutex> lock(g_fileCounterMutex);
                if (g_fileCounterMap.size() == 0)
                {
                    // 首次启动，先获取路径下的文件，找到当前日期最后一个编号的文件
                    fileCounter = getLastFileCounter(_strBasename);
                    g_fileCounterMap[_strBasename] = fileCounter;
                }
                else
                {
                    auto it = g_fileCounterMap.find(_strBasename);
                    if (it != g_fileCounterMap.end())
                    {
                        fileCounter = it->second;
                    }
                }
            }

            std::string currentFilename = generateNewFilename(_strBasename, fileCounter);

            std::ofstream ofs;
            while (true) {
                std::string logEntry;
                if (_dotaskstop)
                {
                    std::unique_lock<std::mutex> lock(_dotaskmutex);
                    if (_dotaskqueue.empty())
                    {
                        if (ofs.is_open())
                        {
                            // 记录 管道推入日志和写入日志条数
                            //ofs << Format("进程退出，管道日志: %I64d 条，记录日志: %I64d 条", _login, _logout) << std::endl;
                            //ofs.flush();
                            ofs.close();
                        }

                        return;
                    }

                    get_logmsg(logEntry);
                }
                else
                {
                    std::unique_lock<std::mutex> lock(_dotaskmutex);
                    // 使用 wait_for 实现 2 秒超时
                    if (_dotaskCondition.wait_for(lock, std::chrono::seconds(2)) == std::cv_status::timeout)
                    {
                        // 2 秒超时，即使没有触发，也执行一次处理
                    }

                    if (_dotaskqueue.empty())
                    {
                        continue;
                    }

                    get_logmsg(logEntry);
                }

                if (logEntry.empty())
                {
                    continue;
                }

                // 检查当前文件大小
                std::ifstream::pos_type fileSize = getFileSize(currentFilename);
                if (fileSize >= maxFileSize) {

                    if (ofs.is_open())
                        ofs.close();

                    fileCounter++;
                    {
                        std::lock_guard<std::mutex> lock(g_fileCounterMutex);
                        g_fileCounterMap[_strBasename] = fileCounter;
                    }
                    currentFilename = generateNewFilename(_strBasename, fileCounter);
                }

                if (!ofs.is_open()) {
                    ofs.open(currentFilename.c_str(), std::ios::app);
                    if (!ofs.is_open())
                        continue;
                }
               
                ofs << logEntry << std::endl;
                ofs.flush();
            }

            if (ofs.is_open())
                ofs.close();
        }

        void stop() {
            {
                std::lock_guard<std::mutex> lock(_pushmutex);
                _pushstop = true;
            }
            _dotaskCondition.notify_all();

            if (_pushThread.joinable())
            {
                _pushThread.join();
            }

            {
                std::lock_guard<std::mutex> lock(_dotaskmutex);
                _dotaskstop = true;
            }
            _pushCondition.notify_all();

            if (_dotaskThread.joinable()) {
                _dotaskThread.join();
            }
        }

        // 获取当前文件大小（以字节为单位）
        std::ifstream::pos_type getFileSize(const std::string& filename) {
            std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
            if (!in.is_open()) {
                return 0;
            }
            return in.tellg();
        }

        // 生成带编号的新文件名
        std::string generateNewFilename(const std::string& baseFilename, int counter) {
            std::ostringstream oss;
            oss << baseFilename << "_" << std::setfill('0') << std::setw(3) << counter << ".log";
            return oss.str();
        }

    private:
        std::thread _pushThread;
        std::mutex _pushmutex;
        std::queue<std::string> _pushqueue;
        std::condition_variable _pushCondition;
        std::atomic<bool> _pushstop{ false };

        std::thread _dotaskThread;
        std::mutex _dotaskmutex;
        std::queue<std::string> _dotaskqueue;
        std::condition_variable _dotaskCondition;
        std::atomic<bool> _dotaskstop{ false };

        // 用于同步文件名计数器的互斥锁
        std::mutex g_fileCounterMutex;
        std::map<std::string, int> g_fileCounterMap;

#ifdef _WIN32
        __int64 _login = 0;
        __int64 _logout = 0;
#else
        int64_t _login = 0;
        int64_t _logout = 0;
#endif
        std::string _strBasename;   // 基础日志文件名
        const size_t maxFileSize = 50 * 1024 * 1024; // 50MB
    };  

    // 全局日志队列实例
    static LogQueue g_logQueue;

    void RecordInfo(const char* szFormat, ...) {
        // 构建日志消息
        va_list ap;
        va_start(ap, szFormat);
        int buffer_size = vsnprintf(nullptr, 0, szFormat, ap);
        va_end(ap);

        if (buffer_size <= 0) {
            return;
        }

        std::string strResult;
        strResult.resize(buffer_size);
        va_start(ap, szFormat);
        vsnprintf((char*)strResult.data(), buffer_size + 1, szFormat, ap);
        va_end(ap);

#ifdef _WIN32
        SYSTEMTIME tm = {};
        GetLocalTime(&tm);
        std::string log = app::Format("%4d%02d%02d %02d:%02d:%02d:%03d %s",
            tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond, tm.wMilliseconds, strResult.c_str());
#else
        time_t now = time(nullptr);
        struct tm* tm_info = localtime(&now);
        auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count() % 1000;
        std::string log = app::Format("%4d%02d%02d %02d:%02d:%02d:%03ld %s",
            tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday,
            tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec, now_ms, strResult.c_str());
#endif

        // 将日志消息推入队列
        g_logQueue.push(std::move(log));
    }

#ifdef _WIN32
    using TraverseCallback = std::function<bool(const std::string&, const WIN32_FIND_DATAA&)>;
#else
    struct FileInfo {
        std::string fileName;
        bool isDirectory;
    };
    using TraverseCallback = std::function<bool(const std::string&, const FileInfo&)>;
#endif

    void Traverse(const std::string& strBasePath, bool bRecursive, TraverseCallback fnCallback)
    {
#ifdef _WIN32
        std::queue<std::string> queueDir;
        queueDir.push(strBasePath);

        while (!queueDir.empty())
        {
            std::string strCurrentPath = queueDir.front() + "/";
            queueDir.pop();

            WIN32_FIND_DATAA findData = {};
            HANDLE hFindFile = FindFirstFileA((strCurrentPath + "*").c_str(), &findData);

            while (FindNextFileA(hFindFile, &findData))
            {
                std::string strFileNames = findData.cFileName;
                if (strFileNames == "." || strFileNames == "..")
                {
                    //如果文件是.或者.., 跳过
                    continue;
                }

                bool bIsDirs = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
                //如果文件是目录且要求递归，则继续遍历
                if (bIsDirs && bRecursive)
                {
                    queueDir.push(strCurrentPath + strFileNames);
                }

                if (bIsDirs)
                {
                    continue;
                }

                if (!fnCallback(strCurrentPath + strFileNames, findData))
                {
                    FindClose(hFindFile);
                    return;
                }
            }

            FindClose(hFindFile);
        }
#else
        std::queue<std::string> queueDir;
        queueDir.push(strBasePath);

        while (!queueDir.empty())
        {
            std::string strCurrentPath = queueDir.front();
            if (strCurrentPath.back() != '/') {
                strCurrentPath += "/";
            }
            queueDir.pop();

            DIR* dir = opendir(strCurrentPath.c_str());
            if (dir == nullptr) {
                continue;
            }

            struct dirent* entry;
            while ((entry = readdir(dir)) != nullptr)
            {
                std::string strFileNames = entry->d_name;
                if (strFileNames == "." || strFileNames == "..")
                {
                    continue;
                }

                std::string fullPath = strCurrentPath + strFileNames;
                struct stat fileStat;
                bool bIsDirs = false;
                if (stat(fullPath.c_str(), &fileStat) == 0) {
                    bIsDirs = S_ISDIR(fileStat.st_mode);
                }

                if (bIsDirs && bRecursive)
                {
                    queueDir.push(fullPath);
                }

                if (bIsDirs)
                {
                    continue;
                }

                FileInfo fileInfo;
                fileInfo.fileName = strFileNames;
                fileInfo.isDirectory = bIsDirs;
                if (!fnCallback(fullPath, fileInfo))
                {
                    closedir(dir);
                    return;
                }
            }

            closedir(dir);
        }
#endif
    }

    time_t GetFileCreateTime(const std::string& strFileName)
    {
#ifdef _WIN32
        FILETIME ft = {};
        HANDLE hFile = CreateFileA(strFileName.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

        if (hFile != INVALID_HANDLE_VALUE)
        {
            GetFileTime(hFile, &ft, nullptr, nullptr);
            CloseHandle(hFile);
        }
        else
        {
            DWORD dwErrorCode = GetLastError();
            return 0;
        }
        ULARGE_INTEGER uli;
        uli.LowPart = ft.dwLowDateTime;
        uli.HighPart = ft.dwHighDateTime;

        time_t t = (LONGLONG)(uli.QuadPart - 116444736000000000ull) / 10000000ull;
        return t;
#else
        struct stat fileStat;
        if (stat(strFileName.c_str(), &fileStat) == 0) {
            return fileStat.st_ctime;
        }
        return 0;
#endif
    }

    bool Delete(const std::string& strFileName)
    {
#ifdef _WIN32
        return DeleteFileA(strFileName.c_str());
#else
        return unlink(strFileName.c_str()) == 0;
#endif
    }

    int CleanLogFile(const std::string& strFolder, const double& DayCycle)
    {
#ifdef _WIN32
        Traverse(strFolder, true, [&](const std::string& strFullPath,
            const WIN32_FIND_DATAA& findData) -> bool {
                time_t creationTime = GetFileCreateTime(strFullPath);
                time_t now = time(NULL);
                time_t deadline = now - time_t(DayCycle * 24 * 60 * 60);

                if (creationTime < deadline)
                {
                    Delete(strFullPath);
                }
                return true;
            });
#else
        Traverse(strFolder, true, [&](const std::string& strFullPath,
            const FileInfo& findData) -> bool {
                time_t creationTime = GetFileCreateTime(strFullPath);
                time_t now = time(NULL);
                time_t deadline = now - time_t(DayCycle * 24 * 60 * 60);

                if (creationTime < deadline)
                {
                    Delete(strFullPath);
                }
                return true;
            });
#endif
        return 0;
    }    
}

#endif    //_GEN_UUID_H_
