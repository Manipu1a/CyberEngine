#include "editor/imgui_renderer.h"

namespace Cyber
{
    namespace Editor
    {
        ImGuiRenderer::ImGuiRenderer(const EditorCreateInfo& createInfo)
            : m_pDevice(createInfo.pDevice)
            , m_backBufferFmt(createInfo.BackBufferFmt)
            , m_depthBufferFmt(createInfo.DepthBufferFmt)
            , m_colorConversionMode(createInfo.ColorConversionMode)
        {

        }

        ImGuiRenderer::~ImGuiRenderer()
        {

        }

        void new_frame(uint32_t renderSurfaceWidth, uint32_t renderSurfaceHeight)
        {

        }
        void end_frame()
        {

        }
        void render_draw_data()
        {

        }
        void invalidate_device_objects()
        {

        }
        void create_device_objects()
        {

        }
        void create_fonts_texture()
        {
            
        }
    }
}