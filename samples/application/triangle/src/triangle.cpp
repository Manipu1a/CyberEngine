#include "triangle.h"
#include "platform/memory.h"
#include "renderer/renderer.h"
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
                builder.with_device(device);
                builder.backend_api(ERHIBackend::RHI_BACKEND_D3D12);
            });
            
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
