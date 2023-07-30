#include "triangle.h"
#include "platform/memory.h"
#include "renderer/renderer.h"
#include "rendergraph/render_graph_resource.h"

namespace Cyber
{
    namespace Samples
    {
        TrignaleApp::TrignaleApp()
        {

        }

        TrignaleApp::~TrignaleApp()
        {
            
        }

        void TrignaleApp::initialize(Cyber::WindowDesc& desc)
        {

        }

        void TrignaleApp::run()
        {

        }

        void TrignaleApp::create_api_objects()
        {

        }

        void TrignaleApp::create_resource()
        {

        }

        void TrignaleApp::create_render_pipeline()
        {
            //render_graph::RenderGraph* graph = cyber_new<render_graph::RenderGraph>();
            namespace render_graph = Cyber::render_graph;
            render_graph::RenderGraph* graph = render_graph::RenderGraph::create([=](render_graph::RenderGraphBuilder& builder)
            {
                builder.with_device(device)
                .backend_api(ERHIBackend::RHI_BACKEND_D3D12);
            });
            
            auto builder = graph->get_builder();
            auto tex = builder->create_texture(
                render_graph::RGTextureCreateDesc{ 
                .mWidth = 1920, .mHeight = 1080 , .mFormat = RHI_FORMAT_R8G8B8A8_SRGB }
                , u8"tex");

            auto backbuffer = builder->create_texture(
                render_graph::RGTextureCreateDesc{ 
                .mWidth = 1920, .mHeight = 1080 , .mFormat = RHI_FORMAT_R8G8B8A8_SRGB }
                , u8"color");

            builder->add_render_pass(
                u8"ColorPass",
                render_graph::RGRenderPassCreateDesc{
                .pipeline = nullptr,
                .render_targets = {
                    {0, backbuffer}
                }},
                 [=](render_graph::RGRenderPassCreateDesc& desc)
                {
                    desc.add_input(u8"Tex", tex)
                    .add_render_target(0, backbuffer);
                });


            graph->add_custom_phase<render_graph::RenderGraphPhase_Prepare>();
            graph->add_custom_phase<render_graph::RenderGraphPhase_Render>();
        }

        void TrignaleApp::finalize()
        {

        }
    }
}

        int WINAPI WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     PSTR    lpCmdLine,
                     int       nCmdShow)
        {
            AllocConsole();
            FILE* stream;
            freopen_s(&stream, "CON", "r", stdin);
            freopen_s(&stream, "CON", "w", stdout);
            SetConsoleTitle(L"Console");
            //std::cout << "test log" << std::endl;
            Cyber::Log::initLog();
            CB_CORE_INFO("initLog");

            Cyber::WindowDesc desc;
            desc.title = L"Cyber";
            desc.mWndW = 1280;
            desc.mWndH = 720;
            desc.hInstance = hInstance;
            desc.cmdShow = nCmdShow;
            Cyber::Samples::TrignaleApp app;
            app.initialize(desc);
            app.run();

            FreeConsole();
            return 1;
        }
