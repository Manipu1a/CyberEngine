#pragma once
#include "sampleapp.h"
#include "graphics/rhi/frame_buffer.h"

namespace Cyber
{
    namespace GUI
    {
        class GUIApplication;
    }

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
            RHITexture* base_color_texture = nullptr;
            Cyber::RenderObject::CEFrameBuffer* frame_buffer = nullptr;
            Cyber::RenderObject::CERenderPass* render_pass = nullptr;
        };
    }
}