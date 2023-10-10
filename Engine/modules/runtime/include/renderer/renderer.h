#pragma once

#include <d3d12.h>
#include <d3dcommon.h>
#include <dxgiformat.h>
#include <memory>
#include <vector>
#include <wrl.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include "rhi/rhi.h"
#include "platform/configure.h"

namespace Cyber
{
    struct CYBER_RUNTIME_API RendererDesc
    {
        ERHIBackend backend;
    };

    class CYBER_RUNTIME_API Renderer
    {
    public:
        Renderer()
        {

        }
        virtual ~Renderer()
        {

        }

    public:
        virtual void initialize(class Application* app, const RendererDesc& desc);
        virtual void finalize();
        void create_resource();
        void create_gfx_objects(class Application* app);
        void create_render_pipeline();
        
        virtual void update(float DeltaTime);
        virtual void raster_draw();
    public:
        RHIDevice* GetDevice() { return device; }
        
    protected:
        HINSTANCE mhAppInst = nullptr;
        IDXGIFactory4* mdxgiFactory;
        IDXGISwapChain* mSwapChain;
        ID3D12Device* md3dDevice;
        ID3D12Fence* mFence;
        UINT64 mCurrentFence = 0;

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

        ID3D12CommandQueue* mCommandQueue;
        ID3D12GraphicsCommandList* mCommandList;

        static const int SwapChainBufferCount = 2;
        int mCurrBackBuffer = 0;
        ID3D12Resource* mSwapChainBuffer[SwapChainBufferCount];
        ID3D12Resource* mDepthStencilBuffer;
        ID3D12DescriptorHeap* mRtvHeap;
        ID3D12DescriptorHeap* mDsvHeap;

        ID3D12RootSignature* mRootSignature = nullptr;
        ID3D12DescriptorHeap* mCbvHeap = nullptr;

        ID3DBlob* mvsByteCode = nullptr;
        ID3DBlob* mpsByteCode = nullptr;

        std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

        ID3D12PipelineState* mPSO = nullptr;
        
        D3D12_VIEWPORT mScreenViewport;
        D3D12_RECT mScissorRect;

        UINT mRtvDescriptorSize = 0;
        UINT mDsvDescriptorSize = 0;
        UINT mCbvSrvUavDescriptorSize = 0;

        D3D_DRIVER_TYPE md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
        DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
        DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
        int mClientWidth = 800;
        int mClientHeight = 600;
    };
}
