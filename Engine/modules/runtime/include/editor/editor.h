#pragma once
#include "graphics/backend/d3d12/graphics_types_d3d12.h"
#include "graphics/interface/swap_chain.hpp"
#include "graphics/backend/d3d12/descriptor_heap_d3d12.h"
#include "application/application.h"
#include "imgui/imgui.h"
#include "cyber_runtime.config.h"

namespace Cyber
{
    namespace RenderObject
    {
        class IRenderDevice;
        class IDeviceContext;
    }

    namespace Editor
    {
        // Usage:
        //  static ExampleAppLog my_log;
        //  my_log.AddLog("Hello %d world\n", 123);
        //  my_log.Draw("title");
        struct CYBER_RUNTIME_API ExampleAppLog
        {
            ImGuiTextBuffer     Buf;
            ImGuiTextFilter     Filter;
            ImVector<int>       LineOffsets; // Index to lines offset. We maintain this with AddLog() calls.
            bool                AutoScroll;  // Keep scrolling if already at the bottom.

            ExampleAppLog()
            {
                AutoScroll = true;
                Clear();
            }

            void    Clear()
            {
                Buf.clear();
                LineOffsets.clear();
                LineOffsets.push_back(0);
            }

            void    AddLog(const char* fmt, ...) IM_FMTARGS(2)
            {
                int old_size = Buf.size();
                va_list args;
                va_start(args, fmt);
                Buf.appendfv(fmt, args);
                va_end(args);
                for (int new_size = Buf.size(); old_size < new_size; old_size++)
                    if (Buf[old_size] == '\n')
                        LineOffsets.push_back(old_size + 1);
            }

            void    Draw(const char* title, bool* p_open = NULL)
            {
                if (!ImGui::Begin(title, p_open))
                {
                    ImGui::End();
                    return;
                }

                // Options menu
                if (ImGui::BeginPopup("Options"))
                {
                    ImGui::Checkbox("Auto-scroll", &AutoScroll);
                    ImGui::EndPopup();
                }

                // Main window
                if (ImGui::Button("Options"))
                    ImGui::OpenPopup("Options");
                ImGui::SameLine();
                bool clear = ImGui::Button("Clear");
                ImGui::SameLine();
                bool copy = ImGui::Button("Copy");
                ImGui::SameLine();
                Filter.Draw("Filter", -100.0f);

                ImGui::Separator();

                if (ImGui::BeginChild("scrolling", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar))
                {
                    if (clear)
                        Clear();
                    if (copy)
                        ImGui::LogToClipboard();

                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
                    const char* buf = Buf.begin();
                    const char* buf_end = Buf.end();
                    if (Filter.IsActive())
                    {
                        // In this example we don't use the clipper when Filter is enabled.
                        // This is because we don't have random access to the result of our filter.
                        // A real application processing logs with ten of thousands of entries may want to store the result of
                        // search/filter.. especially if the filtering function is not trivial (e.g. reg-exp).
                        for (int line_no = 0; line_no < LineOffsets.Size; line_no++)
                        {
                            const char* line_start = buf + LineOffsets[line_no];
                            const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                            if (Filter.PassFilter(line_start, line_end))
                                ImGui::TextUnformatted(line_start, line_end);
                        }
                    }
                    else
                    {
                        // The simplest and easy way to display the entire buffer:
                        //   ImGui::TextUnformatted(buf_begin, buf_end);
                        // And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward
                        // to skip non-visible lines. Here we instead demonstrate using the clipper to only process lines that are
                        // within the visible area.
                        // If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them
                        // on your side is recommended. Using ImGuiListClipper requires
                        // - A) random access into your data
                        // - B) items all being the  same height,
                        // both of which we can handle since we have an array pointing to the beginning of each line of text.
                        // When using the filter (in the block of code above) we don't have random access into the data to display
                        // anymore, which is why we don't use the clipper. Storing or skimming through the search result would make
                        // it possible (and would be recommended if you want to search through tens of thousands of entries).
                        ImGuiListClipper clipper;
                        clipper.Begin(LineOffsets.Size);
                        while (clipper.Step())
                        {
                            for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
                            {
                                const char* line_start = buf + LineOffsets[line_no];
                                const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                                ImGui::TextUnformatted(line_start, line_end);
                            }
                        }
                        clipper.End();
                    }
                    ImGui::PopStyleVar();

                    // Keep up at the bottom of the scroll region if we were already at the bottom at the beginning of the frame.
                    // Using a scrollbar or mouse-wheel will take away from the bottom edge.
                    if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                        ImGui::SetScrollHereY(1.0f);
                }
                ImGui::EndChild();
                ImGui::End();
            }
        };

        enum EDITOR_COLOR_CONVERSION_MODE : uint8_t
        {
            /// Automatically convert colors between sRGB and linear space, depending on the active shader stage.
            EDITOR_COLOR_CONVERSION_MODE_AUTO = 0,
            /// Always use sRGB conversion.
            EDITOR_COLOR_CONVERSION_SRGB_TO_LINEAR,
            /// Do not convert colors
            EDITOR_COLOR_CONVERSION_NONE,
        };

        struct EditorCreateInfo
        {
            static constexpr uint32_t DefaultInitialVBSize = 1024;
            static constexpr uint32_t DefaultInitialIBSize = 1024;

            class Core::Application* application = nullptr;
            class RenderObject::IRenderDevice* pDevice = nullptr;
            class RenderObject::ISwapChain* swap_chain = nullptr;

            HWND Hwnd = nullptr;

            TEXTURE_FORMAT BackBufferFmt = {};
            TEXTURE_FORMAT DepthBufferFmt = {};

            EDITOR_COLOR_CONVERSION_MODE ColorConversionMode = EDITOR_COLOR_CONVERSION_MODE_AUTO;

            uint32_t InitialVBSize = DefaultInitialVBSize;
            uint32_t InitialIBSize = DefaultInitialIBSize;

            EditorCreateInfo() noexcept {}
            EditorCreateInfo(class Core::Application* _app, RenderObject::IRenderDevice* pDevice, HWND hwnd, RenderObject::ISwapChain* _swap_chain, TEXTURE_FORMAT backBufferFmt, TEXTURE_FORMAT depthBufferFmt) noexcept
                : application(_app)
                , pDevice(pDevice)
                , swap_chain(_swap_chain)
                , Hwnd(hwnd)
                , BackBufferFmt(backBufferFmt)
                , DepthBufferFmt(depthBufferFmt)
            {
            }

            EditorCreateInfo(RenderObject::IRenderDevice* pDevice, HWND hwnd, RenderObject::SwapChainDesc swapChainDesc) noexcept
                : pDevice(pDevice)
                , Hwnd(hwnd)
                , BackBufferFmt(swapChainDesc.m_format)
                , DepthBufferFmt(swapChainDesc.m_depthStencilFormat)
            {
            }
        };

        class CYBER_RUNTIME_API Editor
        {
        public:
            Editor(const EditorCreateInfo& createInfo);
            ~Editor();
            
            // Non-copyable
            Editor(const Editor&) = delete;
            Editor(Editor&&) = delete;
            Editor& operator=(const Editor&) = delete;
            Editor& operator=(Editor&&) = delete;
            // Non-movable
            
            virtual void initialize(RenderObject::IRenderDevice* device, HWND hwnd);
            void finalize();
            /**/
            virtual void new_frame(uint32_t renderSurfaceWidth, uint32_t renderSurfaceHeight);
            void end_frame();
            void update(float deltaTime);
            void render(RenderObject::IDeviceContext* device_context, RenderObject::IRenderDevice* device);

            void invalidate_device_objects();
            void create_device_objects();
            void update_fonts_texture();

            ExampleAppLog log;

            static void register_log(uint32_t msg)
            {
                //log.AddLog("%s", msg);
            }
            
        protected:
            Core::Application* m_pApp = nullptr;
            class ImGuiRenderer* m_imguiRenderer;
        };
    }
}