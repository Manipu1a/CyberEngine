#pragma once

#include "EASTL/map.h"
#include "common/smart_ptr.h"
#include "cyber_runtime.config.h"
#include "graphics/rendergraph/render_graph_resource.h"
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
        struct ISampler;
        struct ITexture;
    }

    namespace Renderer
    {
        class Renderer;

        class CYBER_RUNTIME_API ForwardPassPipelineCache
        {
        public:
            void initialize(RenderObject::IRenderDevice* device);
            RenderObject::IRenderPipeline* get_depth_only(TEXTURE_FORMAT depth_format);

        private:
            RenderObject::IRenderDevice* device = nullptr;
            eastl::map<TEXTURE_FORMAT, RefCntAutoPtr<RenderObject::IRenderPipeline>> depth_pipelines;
        };

        struct CYBER_RUNTIME_API ForwardFrameContext
        {
            World* world = nullptr;
            uint32_t back_buffer_index = 0;
            RenderObject::IFrameBuffer* frame_buffer = nullptr;
            RenderObject::ITexture* color_buffer = nullptr;
            RenderObject::ITexture* depth_buffer = nullptr;
        };

        struct CYBER_RUNTIME_API ForwardPassContext
        {
            Renderer* renderer = nullptr;
            RenderObject::IRenderDevice* device = nullptr;
            RenderObject::IDeviceContext* command_context = nullptr;
            RenderObject::IBuffer* scene_constants = nullptr;
            ForwardPassPipelineCache* pipeline_cache = nullptr;
            uint32_t shadow_resolution = 2048;
            ForwardFrameContext frame;
        };

        struct ForwardSceneConstants
        {
            float4x4 view_proj_matrix;
            float4x4 model_matrix;
            float4 camera_pos;
            float4 light_direction;
            float4 light_color;
        };

        class CYBER_RUNTIME_API ForwardRenderPass : public render_graph::RGRenderPass
        {
        public:
            explicit ForwardRenderPass(ForwardPassContext* context);

        protected:
            void set_default_viewport(uint32_t width, uint32_t height) const;
            void update_scene_constants(const ForwardSceneConstants& constants) const;
            void draw_depth_only(const float4x4& view_proj, RenderObject::IRenderPipeline* pipeline) const;
            void draw_color(const float4x4& view_proj, const float3& eye,
                const float3& light_dir, const float3& light_color, float light_intensity,
                RenderObject::IRenderPipeline* pipeline, RenderObject::ITexture* fallback_texture) const;

            bool find_scene_view(float4x4& view_proj, float3& eye) const;
            bool find_main_light(float3& light_dir, float3& light_color, float& intensity) const;
            float4x4 build_shadow_view_projection(const float3& light_dir) const;

            ForwardPassContext* pass_context = nullptr;
        };
    }
}
