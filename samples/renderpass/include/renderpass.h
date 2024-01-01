#pragma once
#include "sampleapp.h"
#include "graphics/interface/frame_buffer.h"

namespace Cyber
{
    namespace GUI
    {
        class GUIApplication;
    }

    namespace RenderObject
    {
        class ITexture;
        class CERenderPass;
        class CEFrameBuffer;
    };

    namespace Samples
    {


        class RenderPassApp : public SampleApp
        {
        public:
            RenderPassApp();
            ~RenderPassApp();

            virtual void initialize(Cyber::WindowDesc& desc) override;
            virtual void run() override;
            virtual void update(float deltaTime) override;
        
        public:
            void raster_draw();
            void create_gfx_objects();
            void create_render_pipeline();
            void create_render_pass();
            void create_resource();
            void create_ui();
            void draw_ui();
            void finalize();
        protected:
            RHIRootSignature* root_signature = nullptr;
            RHIDescriptorSet* descriptor_set = nullptr;
            RHIRenderPipeline* pipeline = nullptr;

            RenderObject::IRenderPass* render_pass = nullptr;
            RenderObject::IFrameBuffer* frame_buffer = nullptr;

            struct GBuffer
            {
                RenderObject::ITexture* base_color_texture = nullptr;
                RenderObject::ITexture* depth_texture = nullptr;
                RenderObject::ITexture* final_color_texture = nullptr;
            } gbuffer;
        };
    }
}