#pragma once
#include "sampleapp.h"

namespace Cyber
{
    namespace GUI
    {
        class GUIApplication;
    }
    
    namespace Samples
    {
        class TrignaleApp : public Cyber::GameApplication
        {
        public:
            TrignaleApp();
            ~TrignaleApp();

            virtual void initialize(Cyber::WindowDesc& desc) override;
            virtual void run() override;
            virtual void update(float deltaTime) override;

        public:
            void raster_draw();
            virtual void create_gfx_objects() override;
            void create_render_pipeline();
            void create_resource();
            void create_ui();
            void draw_ui();
            void finalize();

        protected:
            RenderObject::IRootSignature* root_signature = nullptr;
            RenderObject::IDescriptorSet* descriptor_set = nullptr;
            RenderObject::IRenderPipeline* pipeline = nullptr;
            RenderObject::RenderSubpassDesc subpass_desc;
            RenderObject::RenderPassAttachmentDesc attachment_desc;
            RenderObject::AttachmentReference attachment_ref;
            class GUI::GUIApplication* gui_app = nullptr;
        };
    }
}
