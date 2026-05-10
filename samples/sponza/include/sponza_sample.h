#pragma once
#include "gameruntime/sampleapp.h"
#include "gameruntime/cyber_game.config.h"
#include "common/smart_ptr.h"
#include "EASTL/vector.h"

namespace Cyber
{
    namespace RenderObject
    {
        class IRootSignature;
        class IDescriptorSet;
        class IRenderPipeline;
    }

    namespace ModelLoader
    {
        class Model;
    }

    namespace Samples
    {
        class CYBER_GAME_API SponzaApp : public SampleApp
        {
            struct SponzaVertex
            {
                float3 position;
                float3 normal;
                float2 uv;
            };

            struct SceneConstants
            {
                float4x4 view_proj_matrix;
                float4x4 model_matrix;
                float4 camera_pos;
                float4 light_direction;
                float4 light_color;
            };

            struct DrawPrimitive
            {
                uint32_t first_index;
                uint32_t index_count;
                Cyber::RefCntAutoPtr<RenderObject::ITexture_View> base_color_view;
            };

        public:
            SponzaApp();
            ~SponzaApp();

            virtual void update(float deltaTime) override;
            virtual void draw_ui(ImGuiContext* in_imgui_context) override;

            virtual void on_create_gfx_objects() override;
            virtual void on_create_pipelines() override;
            virtual void on_load_data() override;
            virtual void on_create_resources() override;

        protected:
            Cyber::RefCntAutoPtr<RenderObject::IRenderPass> render_pass;
            Cyber::RefCntAutoPtr<RenderObject::IRenderPipeline> pipeline;
            Cyber::RefCntAutoPtr<RenderObject::IRootSignature> root_signature;
            Cyber::RefCntAutoPtr<RenderObject::IBuffer> vertex_buffer;
            Cyber::RefCntAutoPtr<RenderObject::IBuffer> index_buffer;
            Cyber::RefCntAutoPtr<RenderObject::IBuffer> scene_cbuffer;

            eastl::vector<DrawPrimitive> draw_primitives;

            ModelLoader::Model* model = nullptr;

            // Camera
            float3 camera_pos = { 0.0f, 5.0f, 0.0f };
            float camera_yaw = 0.0f;
            float camera_pitch = 0.0f;
            float camera_speed = 10.0f;

            // Lighting
            float3 light_direction = { 0.577f, -0.577f, 0.577f };
            float3 light_color = { 1.0f, 1.0f, 1.0f };

            // Scene scale
            float scene_scale = 0.1f;
        };
    }
}
