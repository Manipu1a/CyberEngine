#include "gui/cyber_gui.h"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_win32.h"
#include "imgui/backends/imgui_impl_dx12.h"
#include "platform/memory.h"
#include "graphics/backend/d3d12/render_device_d3d12.h"

namespace Cyber
{
    namespace GUI
    {
        GUIApplication::GUIApplication()
        {

        }
        GUIApplication::~GUIApplication()
        {

        }

        void GUIApplication::initialize(RenderObject::CERenderDevice* device, HWND hwnd)
        {
            RenderObject::CERenderDevice_D3D12* device_d3d12 = static_cast<RenderObject::CERenderDevice_D3D12*>(device);
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            desc.NumDescriptors = 1;
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            desc.NodeMask = 0;
            g_imguiSrvDescHeap = cyber_new<RHIDescriptorHeap_D3D12>();
            CHECK_HRESULT(device_d3d12->GetD3D12Device()->CreateDescriptorHeap(&desc, IID_ARGS(&g_imguiSrvDescHeap->pCurrentHeap)));
            // Setup Dear ImGui context
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO(); (void)io;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

            // Setup Dear ImGui style
            ImGui::StyleColorsDark();
            ImGui::StyleColorsLight();
            
            // Setup Platform/Renderer backends
            ImGui_ImplWin32_Init(hwnd);
            
            ImGui_ImplDX12_Init(device_d3d12->GetD3D12Device(), 3,
                DXGI_FORMAT_R8G8B8A8_UNORM, g_imguiSrvDescHeap->pCurrentHeap,
                g_imguiSrvDescHeap->pCurrentHeap->GetCPUDescriptorHandleForHeapStart(),
                g_imguiSrvDescHeap->pCurrentHeap->GetGPUDescriptorHandleForHeapStart());
                

        }
        void GUIApplication::run()
        {

        }
        void GUIApplication::update(RHIRenderPassEncoder* encoder, float deltaTime)
        {
            // Start the Dear ImGui frame
            ImGui_ImplDX12_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();
            bool show_demo_window = true;
            ImGui::ShowDemoWindow(&show_demo_window);

            // Rendering
            ImGui::Render();
            RHICommandBuffer_D3D12* Cmd = static_cast<RHICommandBuffer_D3D12*>(encoder);
            ID3D12DescriptorHeap* GuidescriptorHeaps[] = { g_imguiSrvDescHeap->pCurrentHeap };
            Cmd->pDxCmdList->SetDescriptorHeaps(1, GuidescriptorHeaps);
            ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), Cmd->pDxCmdList);

            
        }

        void GUIApplication::finalize()
        {
            // Cleanup
            ImGui_ImplDX12_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
            
        }
    }
}