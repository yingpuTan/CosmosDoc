#pragma once

#include <chrono>
#include <functional>
#include <memory>
#include <string>

namespace platform {

// 返回当前进程可执行文件所在目录（不包含文件名）
std::string executable_dir();

// 路径拼接（用 / 作为分隔符，Windows 也兼容）
std::string path_join(const std::string& a, const std::string& b);

// 动态库加载
class DynamicLibrary {
public:
    DynamicLibrary() = default;
    ~DynamicLibrary();

    DynamicLibrary(const DynamicLibrary&) = delete;
    DynamicLibrary& operator=(const DynamicLibrary&) = delete;
    DynamicLibrary(DynamicLibrary&&) noexcept;
    DynamicLibrary& operator=(DynamicLibrary&&) noexcept;

    bool load(const std::string& path);
    void unload();
    void* symbol(const char* name) const;
    std::string last_error() const;

private:
    void* handle_ = nullptr;
    mutable std::string last_error_;
};

// 周期定时器（后台线程），析构/stop 时结束
class PeriodicTimer {
public:
    using Callback = std::function<void()>;
    PeriodicTimer(std::chrono::milliseconds interval, Callback cb);
    ~PeriodicTimer();

    PeriodicTimer(const PeriodicTimer&) = delete;
    PeriodicTimer& operator=(const PeriodicTimer&) = delete;

    void stop();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

// Windows 下做 GBK/UTF8 转换；非 Windows 默认认为输入已是 UTF-8，直接返回
std::string gbk_to_utf8(const std::string& s);
std::string utf8_to_gbk(const std::string& s);

// 生成 UUID 字符串（格式：xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx）
std::string uuid_v4();

}  // namespace platform


