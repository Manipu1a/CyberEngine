#include "log/Log.h"
#include <vector>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/callback_sink.h>

namespace Cyber
{
    eastl::shared_ptr<spdlog::logger> Log::sCoreLogger = nullptr;
    eastl::shared_ptr<spdlog::logger> Log::sClientLogger = nullptr;

    eastl::shared_ptr<spdlog::logger>& Log::getCoreLogger()
	{
		return sCoreLogger;
	}

    eastl::shared_ptr<spdlog::logger>& Log::getClientLogger()
	{
		return sClientLogger;
	}
	
    void Log::initLog()
    {
        std::vector<spdlog::sink_ptr> logSinks;
		logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
		logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("Cyber.log", true)); // log file

		logSinks[0]->set_pattern("%^[%T] %n: %v%$");
		logSinks[1]->set_pattern("[%T] [%l] %n: %v");

		auto callback_sink = std::make_shared<spdlog::sinks::callback_sink_mt>([]( const spdlog::details::log_msg& msg) {

		});
		callback_sink->set_level(spdlog::level::trace);
		logSinks.emplace_back(callback_sink);

		sCoreLogger = eastl::make_shared<spdlog::logger>("CYBER", begin(logSinks), end(logSinks));
		spdlog::register_logger(std::make_shared<spdlog::logger>(*sCoreLogger.get()));
		sCoreLogger->set_level(spdlog::level::trace);
		sCoreLogger->flush_on(spdlog::level::trace);

		sClientLogger = eastl::make_shared<spdlog::logger>("APP", begin(logSinks), end(logSinks));
		spdlog::register_logger(std::make_shared<spdlog::logger>(*sClientLogger.get()));
		sClientLogger->set_level(spdlog::level::trace);
		sClientLogger->flush_on(spdlog::level::trace);
    }
}