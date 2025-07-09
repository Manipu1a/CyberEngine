#pragma once
#include "gameruntime/sampleapp.h"
#include "gameruntime/cyber_game.config.h"

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
        class CYBER_GAME_API PBRApp : public SampleApp
        {
            struct CubeVertex
            {
                float3 position;
                float3 normal;
                float3 tangent;
                float2 uv;
            };

            struct ConstantMatrix
            {
                float4x4 ModelMatrix;
                float4x4 ViewMatrix;
                float4x4 ProjectionMatrix;
            };

        public:
            PBRApp();
            ~PBRApp();

            virtual void initialize() override;
            virtual void run() override;
            virtual void update(float deltaTime) override;
            virtual void present() override;

        public:
            void raster_draw();
            void create_gfx_objects();
            void create_render_pipeline();
            void create_resource();
            void create_ui();
            void draw_ui();
            void finalize();

        protected:
            RenderObject::IDescriptorSet* descriptor_set = nullptr;
            RenderObject::IRenderPipeline* pipeline = nullptr;
            RenderObject::IRenderPipeline* environment_pipeline = nullptr;

            RenderObject::RenderSubpassDesc subpass_desc[2];
            RenderObject::RenderPassAttachmentDesc attachment_desc;

            RenderObject::IRenderPass* render_pass = nullptr;
            
            RenderObject::ITexture_View* normal_texture_view = nullptr;
            RenderObject::ITexture_View* base_color_texture_view = nullptr;
            RenderObject::ITexture_View* environment_texture_view = nullptr;

            RenderObject::IBuffer* vertex_buffer = nullptr;
            RenderObject::IBuffer* index_buffer = nullptr;
            RenderObject::IBuffer* vertex_constant_buffer = nullptr;
            RenderObject::IBuffer* light_constant_buffer = nullptr;

            float3 LightDirection = { 0.0f, 0.0f, -10.0f};
            float3 LightColor = { 1.0f, 1.0f, 1.0f };
        };
    }

}
