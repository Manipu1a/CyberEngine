#pragma once

#include "d3d12util.h"
#include "UploadBuffer.h"
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

namespace Cyber
{
    struct Vertex
    {
        DirectX::XMFLOAT3 Pos;
        DirectX::XMFLOAT4 Color;
    };

    static DirectX::XMFLOAT4X4 Identity4x4(
       1.0f, 0.0f, 0.0f, 0.0f,
       0.0f, 1.0f, 0.0f, 0.0f,
       0.0f, 0.0f, 1.0f, 0.0f,
       0.0f, 0.0f, 0.0f, 1.0f);

    struct ObjectConstants
    {
        DirectX::XMFLOAT4X4 WorldViewProj = Identity4x4;
    };

    class Renderer
    {
    protected:
        Renderer(HINSTANCE hInstance);
        virtual ~Renderer();
    public:
        HINSTANCE RendererInst() const;
        HWND MainWnd() const;
        float AspectRatio() const;

        bool Get4xMsaaState() const;
        void Set4xMsaaState(bool value);

        int Run();

        virtual bool Initialize();
        virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    protected:
        virtual void CreateRtvAndDsvDescriptorHeaps();
        virtual void OnResize();
        virtual void Update(float DeltaTime);
        virtual void Draw(float DeltaTime);

        bool InitMainWindow();
        bool InitDirect3D();
        void CreateCommandObjects();
        void CreateSwapChain();

        void FlushCommandQueue();

        ID3D12Resource* CurrentBavkBuffer() const;
        D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
        D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;

        void CalculateFrameStats();

        void LogAdapters();
        void LogAdapterOutputs(IDXGIAdapter* adapter);
        void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);
        
        void BuildDescriptorHeaps();
        void BuildConstantBuffers();
        void BuildRootSignature();
        void BuildShadersAndInputLayout();
        void BuildBoxGeometry();
        void BuildPSO();

        void Draw();
    protected:
        HINSTANCE mhAppInst = nullptr;
        IDXGIFactory4* mdxgiFactory;
        IDXGISwapChain* mSwapChain;
        ID3D12Device* md3dDevice;
        ID3D12Fence* mFence;
        UINT64 mCurrentFence = 0;

        ///-------------------------------------
        Ref<RHIDevice> pRHIDevice = nullptr;
        Ref<RHIAdapter> pRHIAdapter = nullptr;
        Ref<RHIFence> pRHIFence = nullptr;
        Ref<RHIQueue> pQueue = nullptr;
        Ref<RHICommandPool> pCmdPool = nullptr;
        Ref<RHICommandBuffer> pCmdBuffer = nullptr;

        ID3D12CommandQueue* mCommandQueue;
        ID3D12CommandAllocator* mDirectCmdListAlloc;
        ID3D12GraphicsCommandList* mCommandList;

        static const int SwapChainBufferCount = 2;
        int mCurrBackBuffer = 0;
        ID3D12Resource* mSwapChainBuffer[SwapChainBufferCount];
        ID3D12Resource* mDepthStencilBuffer;
        ID3D12DescriptorHeap* mRtvHeap;
        ID3D12DescriptorHeap* mDsvHeap;

        ID3D12RootSignature* mRootSignature = nullptr;
        ID3D12DescriptorHeap* mCbvHeap = nullptr;
        std::unique_ptr<UploadBuffer<ObjectConstants>> mObjectCB = nullptr;
        std::unique_ptr<MeshGeometry> mBoxGeo = nullptr;

        ID3DBlob* mvsByteCode = nullptr;
        ID3DBlob* mpsByteCode = nullptr;

        std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

        ID3D12PipelineState* mPSO = nullptr;

        DirectX::XMFLOAT4X4 mWorld = Identity4x4;
        DirectX::XMFLOAT4X4 mView = Identity4x4;
        DirectX::XMFLOAT4X4 mProj = Identity4x4;
        
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
