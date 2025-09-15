#pragma once
#include "editor.h"

namespace Cyber
{
    namespace RenderObject
    {
        class IRenderPipeline;
        class IDescriptorSet;
        class IDeviceContext;

    }
    namespace Editor
    {
        class Editor;

        class ImGuiRenderer
        {
        public:
            ImGuiRenderer(const EditorCreateInfo& createInfo);
            ~ImGuiRenderer();

            void initialize(Editor* _editor = nullptr);
            void new_frame(uint32_t renderSurfaceWidth, uint32_t renderSurfaceHeight);
            void end_frame();
            void render_draw_data(RenderObject::IDeviceContext* device_context, ImDrawData* drawData);
            void invalidate_device_objects();
            void create_device_objects();
            void create_fonts_texture();

        protected:
            float4 transform_clip_rect(const ImVec2& display_size, const float4 rect) const;

        protected:
            RefCntAutoPtr<RenderObject::IRenderDevice> render_device = nullptr;
            RefCntAutoPtr<RenderObject::ISwapChain> swap_chain = nullptr;
            RefCntAutoPtr<RenderObject::IBuffer> vertex_buffer = nullptr;
            RefCntAutoPtr<RenderObject::IBuffer> index_buffer = nullptr;
            RefCntAutoPtr<RenderObject::IBuffer> vertex_constant_buffer = nullptr;
            RefCntAutoPtr<RenderObject::IDescriptorSet> descriptor_set = nullptr;
            RefCntAutoPtr<RenderObject::IRenderPipeline> render_pipeline = nullptr;
            RefCntAutoPtr<RenderObject::IRenderPass> render_pass = nullptr;
            RefCntAutoPtr<RenderObject::ITexture_View> font_srv = nullptr;
            RefCntAutoPtr<RenderObject::ITexture> font_texture = nullptr;
            RefCntAutoPtr<RenderObject::IShaderResource> m_pSRB = nullptr;
            RefCntAutoPtr<RenderObject::IShaderResource> texture_resource = nullptr;

            RenderObject::RenderSubpassDesc subpass_desc[1] = {};

            const TEXTURE_FORMAT m_backBufferFmt = {};
            const TEXTURE_FORMAT m_depthBufferFmt = {};
            uint32_t vertex_buffer_size = 0;
            uint32_t index_buffer_size = 0;
            uint32_t render_surface_width = 0;
            uint32_t render_surface_height = 0;
            
            const EDITOR_COLOR_CONVERSION_MODE m_colorConversionMode;
            bool m_baseVertexSupported = false;
            uint32_t m_backBufferIndex = 0;
        };
    }
}
