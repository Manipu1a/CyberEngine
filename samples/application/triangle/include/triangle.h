#pragma once
#include "rhi/rhi.h"
#include "rendergraph/render_graph.h"
#include "gameruntime/GameApplication.h"

namespace Cyber
{
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
            virtual void create_api_objects();
            virtual void create_render_pipeline();
            virtual void create_resource();
            virtual void finalize();

            RHIDevice* device;
        };
    }
}
