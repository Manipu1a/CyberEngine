#pragma once
#include <spdlog/sinks/base_sink.h>
#include <mutex>
#include <functional>
#include <string>

namespace Cyber
{
    namespace Editor
    {
        struct ExampleAppLog;
        
        class ImGuiLogSink : public spdlog::sinks::base_sink<std::mutex>
        {
        public:
            using LogCallback = std::function<void(const std::string&)>;
            
            ImGuiLogSink(ExampleAppLog* log) : m_imgui_log(log) {}
            
            void set_log_callback(LogCallback callback)
            {
                m_callback = callback;
            }
            
            void set_imgui_log(ExampleAppLog* log)
            {
                m_imgui_log = log;
            }
            
        protected:
            void sink_it_(const spdlog::details::log_msg& msg) override;
            void flush_() override {}
            
        private:
            LogCallback m_callback;
            ExampleAppLog* m_imgui_log = nullptr;
        };
    }
}