#pragma once
#include "cyber_log.config.h"
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <EASTL/shared_ptr.h>
#include <EASTL/unique_ptr.h>
namespace Cyber
{
    class CYBER_LOG_API Log
    {
    public:
        static void initLog();
        
        static eastl::shared_ptr<spdlog::logger>& getCoreLogger();
        static eastl::shared_ptr<spdlog::logger>& getClientLogger();
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