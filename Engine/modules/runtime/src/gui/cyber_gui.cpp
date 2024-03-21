#include "gui/cyber_gui.h"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_win32.h"
#include "imgui/backends/imgui_impl_dx12.h"
#include "platform/memory.h"
#include "platform/configure.h"
#include "graphics/backend/d3d12/render_device_d3d12.h"
#include "graphics/backend/d3d12/command_buffer_d3d12.h"


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

        void GUIApplication::initialize(RenderObject::IRenderDevice* device, HWND hwnd)
        {
            RenderObject::RenderDevice_D3D12_Impl* device_d3d12 = static_cast<RenderObject::RenderDevice_D3D12_Impl*>(device);
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            desc.NumDescriptors = 1;
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            desc.NodeMask = 0;
            g_imguiSrvDescHeap = cyber_new<RenderObject::DescriptorHeap_D3D12>();
            auto native_heap = g_imguiSrvDescHeap->get_heap();
            CHECK_HRESULT(device_d3d12->GetD3D12Device()->CreateDescriptorHeap(&desc, IID_ARGS(&native_heap)));
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
                DXGI_FORMAT_R8G8B8A8_UNORM, native_heap,
                native_heap->GetCPUDescriptorHandleForHeapStart(),
                native_heap->GetGPUDescriptorHandleForHeapStart());
                

        }
        void GUIApplication::run()
        {

        }
        void GUIApplication::update(RenderObject::RenderPassEncoder* encoder, float deltaTime)
        {
            // Start the Dear ImGui frame
            ImGui_ImplDX12_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();
            bool show_demo_window = true;
            ImGui::ShowDemoWindow(&show_demo_window);

            // Rendering
            ImGui::Render();
            CommandBuffer_D3D12_Impl* Cmd = static_cast<CommandBuffer_D3D12_Impl*>(encoder);
            ID3D12DescriptorHeap* GuidescriptorHeaps[] = { g_imguiSrvDescHeap->get_heap() };
            Cmd->m_pDxCmdList->SetDescriptorHeaps(1, GuidescriptorHeaps);
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