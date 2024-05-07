#pragma once
#include "editor.h"

namespace Cyber
{
    namespace RenderObject
    {
        class IRenderPipeline;
    }
    namespace Editor
    {
        class ImGuiRenderer
        {
        public:
            ImGuiRenderer(const EditorCreateInfo& createInfo);
            ~ImGuiRenderer();

            void new_frame(uint32_t renderSurfaceWidth, uint32_t renderSurfaceHeight);
            void end_frame();
            void render_draw_data();
            void invalidate_device_objects();
            void create_device_objects();
            void create_fonts_texture();

        protected:
            RenderObject::IRenderDevice* m_pDevice = nullptr;
            RenderObject::IBuffer* m_pVB = nullptr;
            RenderObject::IBuffer* m_pIB = nullptr;
            RenderObject::IBuffer* m_pVertexConstantBuffer = nullptr;
            RenderObject::IRenderPipeline* m_pPipeline = nullptr;
            RenderObject::ITextureView* m_pFontSRV = nullptr;
            RenderObject::IShaderResource* m_pSRB = nullptr;

            const TEXTURE_FORMAT m_backBufferFmt = {};
            const TEXTURE_FORMAT m_depthBufferFmt = {};
            uint32_t m_vertexBufferSize = 0;
            uint32_t m_indexBufferSize = 0;
            uint32_t m_renderSurfaceWidth = 0;
            uint32_t m_renderSurfaceHeight = 0;
            
            const EDITOR_COLOR_CONVERSION_MODE m_colorConversionMode;
            bool m_baseVertexSupported = false;
        };
    }
}
