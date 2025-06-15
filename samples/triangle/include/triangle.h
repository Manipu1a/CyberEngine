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
        class CYBER_GAME_API TrignaleApp : public SampleApp
        {
        public:
            TrignaleApp();
            ~TrignaleApp();

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
            RenderObject::IRootSignature* root_signature = nullptr;
            RenderObject::IDescriptorSet* descriptor_set = nullptr;
            RenderObject::IRenderPipeline* pipeline = nullptr;
            RenderObject::RenderSubpassDesc subpass_desc[2];
            RenderObject::RenderPassAttachmentDesc attachment_desc;
            RenderObject::AttachmentReference attachment_ref[2];
        };
    }

    static Cyber::Samples::SampleApp* create_sample_app()
    {
        return Cyber::cyber_new<Cyber::Samples::TrignaleApp>();
    }
}
