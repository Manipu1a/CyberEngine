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

        class TrignaleApp : public SampleApp
        {
        public:
            TrignaleApp();
            ~TrignaleApp();

            virtual void initialize(Cyber::WindowDesc& desc) override;
            virtual void run() override;
            virtual void update(float deltaTime) override;

        public:
            void raster_draw();
            void create_gfx_objects();
            void create_render_pipeline();
            void create_resource();
            void create_ui();
            void draw_ui();
            void finalize();

        protected:
            RHIRootSignature* root_signature = nullptr;
            RHIDescriptorSet* descriptor_set = nullptr;
            RHIRenderPipeline* pipeline = nullptr;


            class GUI::GUIApplication* gui_app = nullptr;
        };
    }
}
