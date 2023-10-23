#pragma once
#include "rhi/rhi.h"
#include "rendergraph/render_graph.h"
#include "gameruntime/GameApplication.h"

namespace Cyber
{
    namespace GUI
    {
        class GUIApplication;
    }
    
    namespace Samples
    {
        class TestApp
        {
        
        };
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
            void create_gfx_objects();
            void create_render_pipeline();
            void create_resource();
            void create_ui();
            void draw_ui();
            void finalize();

        public:
            ///-------------------------------------
            static const uint32_t MAX_FRAMES_IN_FLIGHT = 3;
            static const uint32_t BACK_BUFFER_COUNT = 3;
            RHIDevice* device = nullptr;
            RHIInstance* instance = nullptr;
            RHIAdapter* adapter = nullptr;
            RHIFence* present_fence = nullptr;
            RHIQueue* queue = nullptr;
            RHICommandPool* pool = nullptr;
            RHICommandBuffer* cmd = nullptr;
            RHISwapChain* swap_chain = nullptr;
            RHISurface* surface = nullptr;
            RHIFence* present_swmaphore = nullptr; 
            //RHITextureView* views[BACK_BUFFER_COUNT];
            RHIRenderPipeline* pipeline = nullptr;
            RHIRootSignature* root_signature = nullptr;
            RHIDescriptorSet* descriptor_set = nullptr;
            uint32_t backbuffer_index = 0;

            class GUI::GUIApplication* gui_app = nullptr;
        };
    }
}
