#pragma once

#include "common/smart_ptr.h"
#include "cyber_runtime.config.h"
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

        struct CYBER_RUNTIME_API ForwardFrameContext
        {
            uint32_t back_buffer_index = 0;
            RenderObject::IFrameBuffer* frame_buffer = nullptr;
            RenderObject::ITexture* color_buffer = nullptr;
            RenderObject::ITexture* depth_buffer = nullptr;
            RenderObject::ITexture_View* color_view = nullptr;
            RenderObject::ITexture_View* depth_view = nullptr;
        };

        class CYBER_RUNTIME_API ForwardPipeline
        {
        public:
            explicit ForwardPipeline(Renderer* renderer);
            ~ForwardPipeline();

            void initialize();
            void resize(uint32_t width, uint32_t height);
            void render(World* world, float delta_time);

        private:
            struct SceneConstants
            {
                float4x4 view_proj_matrix;
                float4x4 model_matrix;
                float4 camera_pos;
                float4 light_direction;
                float4 light_color;
            };

            void create_render_passes();
            void create_pipelines();
            void create_resources();

            ForwardFrameContext begin_frame();
            void execute_predepth_pass(World* world, const ForwardFrameContext& frame_context);
            void execute_shadow_pass(World* world);
            void execute_color_pass(World* world, const ForwardFrameContext& frame_context);

            void draw_depth_only(World* world, const float4x4& view_proj, RenderObject::IRenderPipeline* pipeline);
            void draw_color(World* world, const float4x4& view_proj, const float3& eye, const float3& light_dir, const float3& light_color, float light_intensity);

            void set_default_viewport(uint32_t width, uint32_t height);
            void update_scene_constants(const SceneConstants& constants);
            bool find_scene_view(World* world, float4x4& view_proj, float3& eye) const;
            bool find_main_light(World* world, float3& light_dir, float3& light_color, float& intensity) const;
            float4x4 build_shadow_view_projection(const float3& light_dir) const;

            Renderer* m_renderer = nullptr;
            RenderObject::IRenderDevice* m_device = nullptr;
            RenderObject::IDeviceContext* m_context = nullptr;

            RefCntAutoPtr<RenderObject::IRenderPass> m_predepth_pass;
            RefCntAutoPtr<RenderObject::IRenderPass> m_shadow_pass;
            RefCntAutoPtr<RenderObject::IRenderPass> m_color_pass;

            RefCntAutoPtr<RenderObject::IRenderPipeline> m_depth_pipeline;
            RefCntAutoPtr<RenderObject::IRenderPipeline> m_color_pipeline;
            RefCntAutoPtr<RenderObject::IRootSignature> m_depth_root_signature;
            RefCntAutoPtr<RenderObject::IRootSignature> m_color_root_signature;
            RefCntAutoPtr<RenderObject::ISampler> m_sampler;

            RefCntAutoPtr<RenderObject::IBuffer> m_scene_constants;
            RefCntAutoPtr<RenderObject::ITexture> m_white_texture;
            RefCntAutoPtr<RenderObject::ITexture> m_shadow_map;
            RefCntAutoPtr<RenderObject::IFrameBuffer> m_shadow_frame_buffer;

            uint32_t m_shadow_resolution = 2048;
        };
    }
}
