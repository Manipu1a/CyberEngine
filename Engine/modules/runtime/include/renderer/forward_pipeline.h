#pragma once

#include "common/smart_ptr.h"
#include "cyber_runtime.config.h"
#include "graphics/features/forward_pass.h"
#include "graphics/interface/graphics_types.h"
#include "math/basic_math.hpp"

namespace Cyber
{
    class World;

    namespace RenderObject
    {
        struct IBuffer;
        struct IDeviceContext;
        struct IFrameBuffer;
        struct IRenderDevice;
        struct IRenderPass;
        struct IRenderPipeline;
        struct IRootSignature;
        struct ISampler;
        struct ITexture;
        struct ITexture_View;
    }

    namespace Renderer
    {
        class Renderer;

        class CYBER_RUNTIME_API ForwardPipeline
        {
        public:
            explicit ForwardPipeline(Renderer* renderer);
            ~ForwardPipeline();

            void initialize();
            void resize(uint32_t width, uint32_t height);
            void render(World* world, float delta_time);

        private:
            void create_pipelines();
            void create_resources();
            void create_render_graph();
            void update_render_graph_resources(const ForwardFrameContext& frame_context);
            void update_pass_context(const ForwardFrameContext& frame_context);

            ForwardFrameContext begin_frame();

            Renderer* m_renderer = nullptr;
            RenderObject::IRenderDevice* m_device = nullptr;
            RenderObject::IDeviceContext* m_context = nullptr;

            RefCntAutoPtr<RenderObject::IRenderPipeline> m_depth_pipeline;
            RefCntAutoPtr<RenderObject::IRenderPipeline> m_color_pipeline;
            RefCntAutoPtr<RenderObject::ISampler> m_sampler;

            RefCntAutoPtr<RenderObject::IBuffer> m_scene_constants;
            RefCntAutoPtr<RenderObject::ITexture> m_white_texture;
            RefCntAutoPtr<RenderObject::ITexture> m_shadow_map;

            render_graph::RenderGraph* m_render_graph = nullptr;
            render_graph::RGTextureRef m_rg_scene_color = nullptr;
            render_graph::RGTextureRef m_rg_scene_depth = nullptr;
            render_graph::RGTextureRef m_rg_shadow_map = nullptr;
            ForwardPassContext m_pass_context;

            uint32_t m_shadow_resolution = 2048;
        };
    }
}
