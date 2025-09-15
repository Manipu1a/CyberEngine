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
        class IRootSignature;
        class IDescriptorSet;
        class IRenderPipeline;
    }

    namespace Samples
    {
        class CYBER_GAME_API ShadowApp : public SampleApp
        {
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
                float4x4 shadow_matrix;
            };

            struct LightConstants
            {
                float4 light_direction;
                float4 light_color;
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

            virtual void initialize() override;
            virtual void run() override;
            virtual void update(float deltaTime) override;
            virtual void present() override;

        public:
            void raster_draw();
            void create_gfx_objects();
            void create_render_pipeline();
            void create_shadow_pipeline();

            void create_resource();
            void create_ui();
            virtual void draw_ui(ImGuiContext* in_imgui_context) override;
            void finalize();

        private:
            void draw_render_item(RenderObject::IDeviceContext* device_context, const RenderItem& item, bool is_shadow_pass = false);
            void update_render_item_constants(const RenderItem& item, const float4x4& view_proj_matrix, const float4x4& shadow_view_proj_matrix);

        protected:
            Cyber::RefCntAutoPtr<RenderObject::IRootSignature> root_signature;
            Cyber::RefCntAutoPtr<RenderObject::IDescriptorSet> descriptor_set;
            Cyber::RefCntAutoPtr<RenderObject::IRenderPipeline> pipeline;
            Cyber::RefCntAutoPtr<RenderObject::IRenderPipeline> shadow_pipeline;
            RenderObject::RenderSubpassDesc subpass_desc[2];
            Cyber::RefCntAutoPtr<RenderObject::IRenderPass> render_pass;
            Cyber::RefCntAutoPtr<RenderObject::ITexture> shadow_depth;
            Cyber::RefCntAutoPtr<RenderObject::ITexture_View> test_texture_view;
            Cyber::RefCntAutoPtr<RenderObject::IBuffer> light_constant_buffer;

            RenderItem cube_item;
            RenderItem plane_item;
            
            float3 light_direction = { 0.577f, -0.577f, 0.577f };
            float3 light_color = { 1.0f, 1.0f, 1.0f };
        };
    }

}
