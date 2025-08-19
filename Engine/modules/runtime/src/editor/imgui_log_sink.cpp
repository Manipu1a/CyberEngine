#include "editor/imgui_log_sink.h"
#include "editor/editor.h"
#include <spdlog/details/log_msg.h>
#include <spdlog/details/fmt_helper.h>

namespace Cyber
{
    namespace Editor
    {
        void ImGuiLogSink::sink_it_(const spdlog::details::log_msg& msg)
        {
            if (!m_imgui_log && !m_callback)
                return;
                
            spdlog::memory_buf_t formatted;
            
            std::string level_str;
            switch (msg.level)
            {
                case spdlog::level::trace:
                    level_str = "[TRACE]";
                    break;
                case spdlog::level::debug:
                    level_str = "[DEBUG]";
                    break;
                case spdlog::level::info:
                    level_str = "[INFO]";
                    break;
                case spdlog::level::warn:
                    level_str = "[WARN]";
                    break;
                case spdlog::level::err:
                    level_str = "[ERROR]";
                    break;
                case spdlog::level::critical:
                    level_str = "[CRITICAL]";
                    break;
                default:
                    level_str = "[UNKNOWN]";
                    break;
            }
            
            spdlog::details::fmt_helper::append_string_view(msg.payload, formatted);
            
            std::string log_line = fmt::format("[{:%H:%M:%S}] {} {}: {}\n", 
                msg.time, 
                level_str,
                msg.logger_name,
                std::string(formatted.data(), formatted.size()));
            
            if (m_imgui_log)
            {
                m_imgui_log->AddLog("%s", log_line.c_str());
            }
            
            if (m_callback)
            {
                m_callback(log_line);
            }
        }
    }
}