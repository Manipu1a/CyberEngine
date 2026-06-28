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
        struct IRootSignature;
        struct IDescriptorSet;
        struct IRenderPipeline;
    }

    namespace Samples
    {
        class CYBER_GAME_API TrignaleApp : public SampleApp
        {
        public:
            TrignaleApp();
            ~TrignaleApp();

            virtual void initialize() override;
            virtual void run() override;
            virtual void update(float deltaTime) override;
            virtual void present() override;
            virtual bool use_engine_forward_pipeline() const override { return false; }

        public:
            void raster_draw();
            void create_gfx_objects();
            void create_render_pipeline();
            void create_resource();
            void create_ui();
            void draw_ui();
            void finalize();

        protected:
            RenderObject::IRootSignature* root_signature = nullptr;
            RenderObject::IDescriptorSet* descriptor_set = nullptr;
            RenderObject::IRenderPipeline* pipeline = nullptr;
            RenderObject::RenderSubpassDesc subpass_desc[2];
            RenderObject::RenderPassAttachmentDesc attachment_desc[2] = {};
            RenderObject::AttachmentReference attachment_ref[2];
            RenderObject::IRenderPass* render_pass = nullptr;
            
            RenderObject::ITexture_View* test_texture_view = nullptr;
            RenderObject::IBuffer* vertex_buffer = nullptr;
            RenderObject::IBuffer* index_buffer = nullptr;
        };
    }
}
