#include "editor/editor.h"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_win32.h"
#include "imgui/backends/imgui_impl_dx12.h"
#include "platform/memory.h"
#include "platform/configure.h"
#include "graphics/backend/d3d12/render_device_d3d12.h"
#include "graphics/backend/d3d12/command_buffer_d3d12.h"
#include "core/debug.h"
#include "editor/imgui_renderer.h"

namespace Cyber
{
    namespace Editor
    {
        Editor::Editor(const EditorCreateInfo& createInfo)
        {
            m_imguiRenderer = cyber_new<ImGuiRenderer>(createInfo);
        }

        Editor::~Editor()
        {

        }

        void Editor::initialize(RenderObject::IRenderDevice* device, HWND hwnd)
        {
            RenderObject::RenderDevice_D3D12_Impl* device_d3d12 = static_cast<RenderObject::RenderDevice_D3D12_Impl*>(device);
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            desc.NumDescriptors = 1;
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            desc.NodeMask = 0;
            g_imguiSrvDescHeap = cyber_new<RenderObject::DescriptorHeap_D3D12>();
            ID3D12DescriptorHeap* native_heap = nullptr;
            CHECK_HRESULT(device_d3d12->GetD3D12Device()->CreateDescriptorHeap(&desc, IID_ARGS(&native_heap)));
            g_imguiSrvDescHeap->set_heap(native_heap);
            // Setup Dear ImGui context
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO(); (void)io;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
           // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

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

        void Editor::run()
        {

        }

        void Editor::update(RenderObject::ICommandBuffer* encoder, float deltaTime)
        {
            // Start the Dear ImGui frame
            ImGui_ImplDX12_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();
            bool show_demo_window = true;
            ImGui::ShowDemoWindow(&show_demo_window);

            // Rendering
            ImGui::Render();
            RenderObject::CommandBuffer_D3D12_Impl* Cmd = static_cast<RenderObject::CommandBuffer_D3D12_Impl*>(encoder);
            ID3D12DescriptorHeap* GuidescriptorHeaps[] = { g_imguiSrvDescHeap->get_heap() };
            Cmd->get_dx_cmd_list()->SetDescriptorHeaps(1, GuidescriptorHeaps);
            ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), Cmd->get_dx_cmd_list());

            // Update and Render additional Platform Windows
            ImGuiIO& io = ImGui::GetIO();
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault(nullptr, Cmd->get_dx_cmd_list());
            }
        }

        void Editor::finalize()
        {
            // Cleanup
            ImGui_ImplDX12_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
        }

        void Editor::new_frame(uint32_t renderSurfaceWidth, uint32_t renderSurfaceHeight)
        {
            m_imguiRenderer->new_frame(renderSurfaceWidth, renderSurfaceHeight);
        }

        void Editor::end_frame()
        {
            m_imguiRenderer->end_frame();
        }

        void Editor::render()
        {
            m_imguiRenderer->render_draw_data();
        }

        void Editor::invalidate_device_objects()
        {
            m_imguiRenderer->invalidate_device_objects();
        }

        void Editor::create_device_objects()
        {
            m_imguiRenderer->create_device_objects();
        }

        void Editor::create_fonts_texture()
        {

        }
    }
}