#include "platform.h"

#include <atomic>
#include <cstring>
#include <filesystem>
#include <random>
#include <thread>

#if defined(_WIN32)
#  define NOMINMAX
#  include <Windows.h>
#else
#  include <cerrno>
#  include <dlfcn.h>
#  include <unistd.h>
#endif

namespace platform {

std::string path_join(const std::string& a, const std::string& b) {
    if (a.empty()) return b;
    if (b.empty()) return a;
    if (a.back() == '/' || a.back() == '\\') return a + b;
    return a + "/" + b;
}

std::string executable_dir() {
#if defined(_WIN32)
    char buffer[MAX_PATH] = {0};
    DWORD len = GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    if (len == 0 || len >= MAX_PATH) return std::filesystem::current_path().string();
    std::filesystem::path p(buffer);
    return p.parent_path().string();
#else
    char buf[4096] = {0};
    ssize_t len = ::readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len <= 0) return std::filesystem::current_path().string();
    buf[len] = '\0';
    std::filesystem::path p(buf);
    return p.parent_path().string();
#endif
}

DynamicLibrary::~DynamicLibrary() { unload(); }

DynamicLibrary::DynamicLibrary(DynamicLibrary&& other) noexcept {
    handle_ = other.handle_;
    last_error_ = std::move(other.last_error_);
    other.handle_ = nullptr;
}

DynamicLibrary& DynamicLibrary::operator=(DynamicLibrary&& other) noexcept {
    if (this == &other) return *this;
    unload();
    handle_ = other.handle_;
    last_error_ = std::move(other.last_error_);
    other.handle_ = nullptr;
    return *this;
}

bool DynamicLibrary::load(const std::string& path) {
    unload();
    last_error_.clear();
#if defined(_WIN32)
    HMODULE h = ::LoadLibraryExA(path.c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
    if (!h) {
        DWORD err = ::GetLastError();
        last_error_ = "LoadLibraryExA failed, err=" + std::to_string(err);
        return false;
    }
    handle_ = reinterpret_cast<void*>(h);
    return true;
#else
    void* h = ::dlopen(path.c_str(), RTLD_NOW);
    if (!h) {
        const char* e = ::dlerror();
        last_error_ = e ? e : "dlopen failed";
        return false;
    }
    handle_ = h;
    return true;
#endif
}

void DynamicLibrary::unload() {
    if (!handle_) return;
#if defined(_WIN32)
    ::FreeLibrary(reinterpret_cast<HMODULE>(handle_));
#else
    ::dlclose(handle_);
#endif
    handle_ = nullptr;
}

void* DynamicLibrary::symbol(const char* name) const {
    if (!handle_) return nullptr;
#if defined(_WIN32)
    return reinterpret_cast<void*>(::GetProcAddress(reinterpret_cast<HMODULE>(handle_), name));
#else
    return ::dlsym(handle_, name);
#endif
}

std::string DynamicLibrary::last_error() const { return last_error_; }

struct PeriodicTimer::Impl {
    std::atomic<bool> stop{false};
    std::thread worker;
};

PeriodicTimer::PeriodicTimer(std::chrono::milliseconds interval, Callback cb)
    : impl_(std::make_unique<Impl>()) {
    impl_->worker = std::thread([interval, cb = std::move(cb), impl = impl_.get()]() mutable {
        while (!impl->stop.load(std::memory_order_relaxed)) {
            std::this_thread::sleep_for(interval);
            if (impl->stop.load(std::memory_order_relaxed)) break;
            try {
                cb();
            } catch (...) {
                // 避免异常炸掉后台线程
            }
        }
    });
}

PeriodicTimer::~PeriodicTimer() { stop(); }

void PeriodicTimer::stop() {
    if (!impl_) return;
    impl_->stop.store(true, std::memory_order_relaxed);
    if (impl_->worker.joinable()) impl_->worker.join();
    impl_.reset();
}

std::string gbk_to_utf8(const std::string& s) {
#if defined(_WIN32)
    int wide_len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), -1, nullptr, 0);
    if (wide_len == 0) return "";
    std::wstring wide(static_cast<size_t>(wide_len), L'\0');
    if (MultiByteToWideChar(CP_ACP, 0, s.c_str(), -1, wide.data(), wide_len) == 0) return "";
    int utf8_len = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (utf8_len == 0) return "";
    std::string out(static_cast<size_t>(utf8_len), '\0');
    if (WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, out.data(), utf8_len, nullptr, nullptr) == 0) return "";
    if (!out.empty() && out.back() == '\0') out.pop_back();
    return out;
#else
    return s;
#endif
}

std::string utf8_to_gbk(const std::string& s) {
#if defined(_WIN32)
    int wide_len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    if (wide_len == 0) return "";
    std::wstring wide(static_cast<size_t>(wide_len), L'\0');
    if (MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, wide.data(), wide_len) == 0) return "";
    int gbk_len = WideCharToMultiByte(936, 0, wide.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (gbk_len == 0) return "";
    std::string out(static_cast<size_t>(gbk_len), '\0');
    if (WideCharToMultiByte(936, 0, wide.c_str(), -1, out.data(), gbk_len, nullptr, nullptr) == 0) return "";
    if (!out.empty() && out.back() == '\0') out.pop_back();
    return out;
#else
    return s;
#endif
}

static uint32_t rand_u32(std::mt19937& rng) {
    std::uniform_int_distribution<uint32_t> dist(0, 0xFFFFFFFFu);
    return dist(rng);
}

std::string uuid_v4() {
    std::random_device rd;
    std::mt19937 rng(rd());
    uint8_t bytes[16];
    for (int i = 0; i < 16; i += 4) {
        uint32_t r = rand_u32(rng);
        bytes[i + 0] = static_cast<uint8_t>((r >> 24) & 0xFF);
        bytes[i + 1] = static_cast<uint8_t>((r >> 16) & 0xFF);
        bytes[i + 2] = static_cast<uint8_t>((r >> 8) & 0xFF);
        bytes[i + 3] = static_cast<uint8_t>(r & 0xFF);
    }
    // version 4
    bytes[6] = static_cast<uint8_t>((bytes[6] & 0x0F) | 0x40);
    // variant 10xx
    bytes[8] = static_cast<uint8_t>((bytes[8] & 0x3F) | 0x80);

    static const char* hex = "0123456789abcdef";
    std::string out;
    out.reserve(36);
    auto push2 = [&](uint8_t b) {
        out.push_back(hex[(b >> 4) & 0xF]);
        out.push_back(hex[b & 0xF]);
    };
    for (int i = 0; i < 16; ++i) {
        push2(bytes[i]);
        if (i == 3 || i == 5 || i == 7 || i == 9) out.push_back('-');
    }
    return out;
}

}  // namespace platform


