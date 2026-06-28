#pragma once
#include "gameruntime/sampleapp.h"
#include "gameruntime/cyber_game.config.h"
#include "common/smart_ptr.h"

namespace Cyber
{
    namespace GUI
    {
        class GUIApplication;
    }

    namespace RenderObject
    {
        struct IRootSignature;
        struct IDescriptorSet;
        struct IRenderPipeline;
    }

    namespace Samples
    {
        class CYBER_GAME_API ShadowApp : public SampleApp
        {
            static const uint32_t MAX_CASCADE_COUNT = 4;

            struct CubeVertex
            {
                float3 position;
                float3 normal;
                float2 uv;
            };

            struct ViewConstants
            {
                float4x4 model_matrix;
                float4x4 view_projection_matrix;
                float4x4 view_matrix;
            };

            struct LightConstants
            {
                float4 light_direction;
                float4 light_color;
            };

            struct CSMConstants
            {
                float4x4 cascade_view_proj_matrices[MAX_CASCADE_COUNT];
                float4 cascade_splits;   // view-space split distances
                float4 shadow_params;    // x=resolution, y=1/resolution, z=cascade_count, w=depth_bias
                float4 debug_params;     // x=show_cascade_debug
            };

            struct ShadowMVPConstants
            {
                float4x4 shadow_mvps[MAX_CASCADE_COUNT];
            };

            struct RenderItem
            {
                Cyber::RefCntAutoPtr<RenderObject::IBuffer> vertex_buffer;
                Cyber::RefCntAutoPtr<RenderObject::IBuffer> index_buffer = nullptr;
                Cyber::RefCntAutoPtr<RenderObject::IBuffer> constant_buffer = nullptr;
                Cyber::RefCntAutoPtr<RenderObject::IBuffer> shadow_constant_buffer = nullptr;
                uint32_t index_count = 0;
                uint32_t vertex_stride = sizeof(CubeVertex);
                float4x4 model_matrix = float4x4::Identity();
            };

        public:
            ShadowApp();
            ~ShadowApp();

            virtual void update(float deltaTime) override;
            virtual bool use_engine_forward_pipeline() const override { return false; }

            // Use initialization hooks
            virtual void on_create_gfx_objects() override;
            virtual void on_create_pipelines() override;
            virtual void on_create_resources() override;

        public:
            void raster_draw();
            void create_shadow_pipeline();
            virtual void draw_ui(ImGuiContext* in_imgui_context) override;

        private:
            void draw_render_item(RenderObject::IDeviceContext* device_context, const RenderItem& item, bool is_shadow_pass = false, uint32_t cascade_index = 0);
            void update_render_item_constants(const RenderItem& item, const float4x4& view_matrix, const float4x4& view_proj_matrix, const float4x4 cascade_shadow_vp[MAX_CASCADE_COUNT]);
            void calculate_cascade_splits(float near_plane, float far_plane, float splits[MAX_CASCADE_COUNT]);
            void calculate_cascade_matrix(const float4x4& camera_view, const float4x4& camera_proj,
                                          float near_split, float far_split,
                                          const float3& light_dir,
                                          float4x4& out_light_vp);
            void rebuild_shadow_resources();

        protected:
            Cyber::RefCntAutoPtr<RenderObject::IRootSignature> root_signature;
            Cyber::RefCntAutoPtr<RenderObject::IDescriptorSet> descriptor_set;
            Cyber::RefCntAutoPtr<RenderObject::IRenderPipeline> pipeline;
            Cyber::RefCntAutoPtr<RenderObject::IRenderPipeline> shadow_pipeline;
            Cyber::RefCntAutoPtr<RenderObject::IRenderPass> render_pass;
            Cyber::RefCntAutoPtr<RenderObject::ITexture> shadow_depth;
            Cyber::RefCntAutoPtr<RenderObject::ITexture_View> shadow_array_srv;
            Cyber::RefCntAutoPtr<RenderObject::ITexture_View> cascade_dsv_views[MAX_CASCADE_COUNT];
            Cyber::RefCntAutoPtr<RenderObject::ITexture_View> test_texture_view;
            Cyber::RefCntAutoPtr<RenderObject::IBuffer> light_constant_buffer;
            Cyber::RefCntAutoPtr<RenderObject::IBuffer> csm_constant_buffer;
            Cyber::RefCntAutoPtr<RenderObject::IBuffer> cascade_index_buffer;

            RenderItem cube_item;
            RenderItem plane_item;

            float3 light_direction = { 0.577f, -0.577f, 0.577f };
            float3 light_color = { 1.0f, 1.0f, 1.0f };

            // CSM parameters
            uint32_t cascade_count = 4;
            float split_lambda = 0.5f;
            uint32_t shadow_resolution = 1024;
            float depth_bias = 0.005f;
            bool show_cascade_debug = false;
            float camera_near = 0.1f;
            float camera_far = 100.0f;
            bool needs_rebuild = false;
        };
    }

}
