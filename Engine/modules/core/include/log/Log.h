#pragma once
#include "cyber_core.config.h"
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <EASTL/shared_ptr.h>
#include <EASTL/unique_ptr.h>
#include "core/debug.h"

namespace Cyber
{
    class CYBER_CORE_API Log
    {
    public:
        static void initLog();
        
        static eastl::shared_ptr<spdlog::logger>& getCoreLogger();
        static eastl::shared_ptr<spdlog::logger>& getClientLogger();
        
        // Add a custom sink to both loggers
        static void addSink(std::shared_ptr<spdlog::sinks::sink> sink);
        
    private:
        static eastl::shared_ptr<spdlog::logger> sCoreLogger;
        static eastl::shared_ptr<spdlog::logger> sClientLogger;
    };
}


#define CB_CORE_TRACE(...)      ::Cyber::Log::getCoreLogger()->trace(__VA_ARGS__)
#define CB_CORE_INFO(...)       ::Cyber::Log::getCoreLogger()->info(__VA_ARGS__)
#define CB_CORE_WARN(...)       ::Cyber::Log::getCoreLogger()->warn(__VA_ARGS__)
#define CB_CORE_ERROR(...)      ::Cyber::Log::getCoreLogger()->error(__VA_ARGS__)
#define CB_CORE_CRITICAL(...)   ::Cyber::Log::getCoreLogger()->critical(__VA_ARGS__)

#define CB_TRACE(...)           ::Cyber::Log::getClientLogger()->trace(__VA_ARGS__)
#define CB_INFO(...)            ::Cyber::Log::getClientLogger()->info(__VA_ARGS__)
#define CB_WARN(...)            ::Cyber::Log::getClientLogger()->warn(__VA_ARGS__)
#define CB_ERROR(...)           ::Cyber::Log::getClientLogger()->error(__VA_ARGS__)
#define CB_CRITICAL(...)        ::Cyber::Log::getClientLogger()->critical(__VA_ARGS__)

#define cyber_log(...) CB_TRACE(__VA_ARGS__)
#define cyber_info(...) CB_INFO(__VA_ARGS__)

#ifdef CB_ENABLE_ASSERTS
    #define cyber_check(x) if(!(x)) {__debugbreak();}
    #define cyber_check_msg(x, ...) if(!(x)) {CB_ERROR("Assertion Failed: {0}", __VA_ARGS__);  __debugbreak();}
    #define cyber_assert(x, ...) if(!(x)) {CB_ERROR("Assertion Failed: {0}", __VA_ARGS__);  __debugbreak();}
    #define cyber_warn(...) CB_WARN(__VA_ARGS__)
    #define cyber_core_assert(x, ...) { if(!(x)) {CB_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__);  __debugbreak();}}
    #define cyber_error(...) {CB_ERROR(__VA_ARGS__);  __debugbreak();}
#else
    #define cyber_check(x)
    #define cyber_check_msg(x, ...)
    #define cyber_assert(x, ...)
    #define cyber_warn(...)
    #define cyber_core_assert(x, ...)
    #define cyber_error(...)
#endif

#if defined(_WINDOWS)
#define CHECK_HRESULT(exp)                                                              \
    do                                                                                  \
    {                                                                                   \
        HRESULT hres = (exp);                                                           \
        if(!SUCCEEDED(hres))                                                            \
        {                                                                               \
            cyber_assert(false, "{0}: FAILED with HRESULT: {1}", 15, (uint32_t)hres); \
        }                                                                               \
    } while (0)
#endif