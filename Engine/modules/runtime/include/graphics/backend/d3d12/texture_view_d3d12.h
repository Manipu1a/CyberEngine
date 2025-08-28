#pragma once

#include "graphics/interface/texture_view.h"
#include "engine_impl_traits_d3d12.hpp"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(RenderObject)

class RenderDevice_D3D12_Impl;
struct ITexture_View_D3D12 : public ITexture_View
{
};

class Texture_View_D3D12_Impl : public Texture_View<EngineD3D12ImplTraits>
{
public:
    using RenderDeviceImplType = EngineD3D12ImplTraits::RenderDeviceImplType;
    using TTextureViewBase = Texture_View<EngineD3D12ImplTraits>;
    Texture_View_D3D12_Impl(class RenderDevice_D3D12_Impl* device, const TextureViewCreateDesc& desc) : TTextureViewBase(device, desc)
    {
        m_dxDescriptorHandles.ptr = 0;
        m_srvDescriptorOffset = 0;
        m_uavDescriptorOffset = 0;
        m_rtvDsvDescriptorHandle.ptr = 0;
        gpu_descriptor_handle.ptr = 0;
    }
    virtual ~Texture_View_D3D12_Impl();

    
    /// used for somecase where need gpu resource handle, like imgui image...
    ///  OpenGL:       ImTextureID = GLuint                      (see ImGui_ImplOpenGL3_RenderDrawData()      in imgui_impl_opengl3.cpp)
    ///  DirectX9:     ImTextureID = LPDIRECT3DTEXTURE9          (see ImGui_ImplDX9_RenderDrawData()          in imgui_impl_dx9.cpp)
    ///  DirectX11:    ImTextureID = ID3D11ShaderResourceView*   (see ImGui_ImplDX11_RenderDrawData()         in imgui_impl_dx11.cpp)
    ///  DirectX12:    ImTextureID = D3D12_GPU_DESCRIPTOR_HANDLE (see ImGui_ImplDX12_RenderDrawData()         in imgui_impl_dx12.cpp)
    ///  SDL_Renderer: ImTextureID = SDL_Texture*                (see ImGui_ImplSDLRenderer2_RenderDrawData() in imgui_impl_sdlrenderer2.cpp)
    ///  Vulkan:       ImTextureID = VkDescriptorSet             (see ImGui_ImplVulkan_RenderDrawData()       in imgui_impl_vulkan.cpp)
    ///  WebGPU:       ImTextureID = WGPUTextureView             (see ImGui_ImplWGPU_RenderDrawData()         in imgui_impl_wgpu.cpp)
    virtual void* get_gpu_native_resource() override
    {
        return (void*)gpu_descriptor_handle.ptr;
    }

protected:
    /// handle store order: srv, uav
    /// Descriptor handle of the SRV/UAV in a CPU visible descriptor heap
    D3D12_CPU_DESCRIPTOR_HANDLE m_dxDescriptorHandles;
    /// Offset from mDxDescriptors for srv descriptor handle
    uint8_t m_srvDescriptorOffset;
    /// Offset from mDxDescriptors for uav descriptor handle
    uint8_t m_uavDescriptorOffset;
    /// Descriptor handle for RTV/DSV in a CPU visible descriptor heap
    D3D12_CPU_DESCRIPTOR_HANDLE m_rtvDsvDescriptorHandle;

    D3D12_GPU_DESCRIPTOR_HANDLE gpu_descriptor_handle;
    
    friend class RenderObject::RenderDevice_D3D12_Impl;
    friend class RenderObject::DeviceContext_D3D12_Impl;
};

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE