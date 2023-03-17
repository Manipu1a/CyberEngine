#include "renderer/renderer.h"
#include "DirectXColors.h"
#include "DirectXMath.h"
#include <D3Dcompiler.h>
#include <array>
#include <combaseapi.h>
#include <cstdint>
#include <d3d12.h>
#include <d3d12sdklayers.h>
#include <d3dcommon.h>
#include <debugapi.h>
#include <dxgi.h>
#include <dxgiformat.h>
#include <dxgitype.h>
#include <handleapi.h>
#include <intsafe.h>
#include <memory>
#include <minwinbase.h>
#include <minwindef.h>
#include <stdlib.h>
#include <string>
#include <synchapi.h>
#include <vector>
#include <winbase.h>
#include <winerror.h>
#include <wingdi.h>
#include <winnt.h>

namespace Cyber
{
    Renderer::Renderer(HINSTANCE hInstance)
        : mhAppInst(hInstance)
    {

    }

    Renderer::~Renderer()
    {

    }

    HINSTANCE Renderer::RendererInst() const
    {

    }

    HWND Renderer::MainWnd() const
    {
        
    }

    float Renderer::AspectRatio() const
    {
        return static_cast<float>(mClientWidth) / mClientHeight;
    }

    bool Renderer::Get4xMsaaState() const
    {
        return false;
    }
    void Renderer::Set4xMsaaState(bool value)
    {

    }

    int Renderer::Run()
    {
        Update(0.0F);
        Draw(0.0f);
    }

    bool Renderer::Initialize()
    {
        if(!InitMainWindow())
            return false;

        if(!InitDirect3D())
            return false;

        OnResize();

        // Reset the comman list to prep for initialization commands.
        mCommandList->Reset(mDirectCmdListAlloc, nullptr);

        return true;
    }

    LRESULT Renderer::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {

    }

    void Renderer::CreateRtvAndDsvDescriptorHeaps()
    {
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
        rtvHeapDesc.NumDescriptors = SwapChainBufferCount;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        rtvHeapDesc.NodeMask = 0;
        md3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&mRtvHeap));

        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
        dsvHeapDesc.NumDescriptors = 1;
        dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;;
        dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        dsvHeapDesc.NodeMask = 0;
        md3dDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&mDsvHeap));
    }

    void Renderer::OnResize()
    {
        FlushCommandQueue();

        mCommandList->Reset(mDirectCmdListAlloc, nullptr);

        for(int i = 0; i < SwapChainBufferCount; ++i)
        {
            mSwapChainBuffer[i]->Release();
            mSwapChainBuffer[i] = nullptr;
        }
        mDepthStencilBuffer->Release();
        mDepthStencilBuffer = nullptr;

        // Resize the swap chain
        mSwapChain->ResizeBuffers(SwapChainBufferCount, mClientWidth, mClientHeight, mBackBufferFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

        mCurrBackBuffer = 0;

        // Create the depth/stencil buffer and view
        D3D12_RESOURCE_DESC depthStencilDesc;
        depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        depthStencilDesc.Alignment = 0;
        depthStencilDesc.Width = mClientWidth;
        depthStencilDesc.Height = mClientHeight;
        depthStencilDesc.DepthOrArraySize = 1;
        depthStencilDesc.MipLevels = 1;
        depthStencilDesc.Format = mDepthStencilFormat;
        depthStencilDesc.SampleDesc.Count = 1;
        depthStencilDesc.SampleDesc.Quality = 0;
        depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        D3D12_CLEAR_VALUE optClear;
        optClear.Format = mDepthStencilFormat;
        optClear.DepthStencil.Depth = 1.0f;
        optClear.DepthStencil.Stencil = 0;

        D3D12_HEAP_PROPERTIES heapProperties;
        heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
        md3dDevice->CreateCommittedResource(
            &heapProperties, D3D12_HEAP_FLAG_NONE, &depthStencilDesc, D3D12_RESOURCE_STATE_COMMON, &optClear, IID_PPV_ARGS(&mDepthStencilBuffer));

        // Create descriptor to mip level 0 of entire resource using the format of the resource.
        md3dDevice->CreateDepthStencilView(mDepthStencilBuffer, nullptr, DepthStencilView());

        // Transition the resource from its initial state to be used as a depth buffer.
        D3D12_RESOURCE_BARRIER barrier;
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = mDepthStencilBuffer;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
        mCommandList->ResourceBarrier(1, &barrier);

        // Execute the resize commands.
        mCommandList->Close();
        ID3D12CommandList* cmdsLists[] = {mCommandList};
        mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

        // Wait until resize is complete.
        FlushCommandQueue();

        // Update the viewport transform to cover the client area.
        mScreenViewport.TopLeftX = 0;
        mScreenViewport.TopLeftY = 0;
        mScreenViewport.Width = static_cast<float>(mClientWidth);
        mScreenViewport.Height = static_cast<float>(mClientHeight);
        mScreenViewport.MinDepth = 0.0f;
        mScreenViewport.MaxDepth = 1.0f;

        mScissorRect = {0,0,mClientWidth, mClientHeight};
    }

    void Renderer::Update(float DeltaTime)
    {

    }
    void Renderer::Draw(float DeltaTime)
    {

    }

    bool Renderer::InitMainWindow()
    {
        return true;
    }

    bool Renderer::InitDirect3D()
    {
#if defined (DEBUG) || defined (_DEBUG)
        // Enable the D3D12 debug layer
        {
            ID3D12Debug* debugController;
            D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
            debugController->EnableDebugLayer();
            debugController->Release();
        }
#endif
        //RHI::GetRHIContext()->createRHI(RHI_BACKEND_D3D12);

        DeviceCreateDesc deviceDesc;
        deviceDesc.mQueueGroupCount = 0;
        deviceDesc.bDisablePipelineCache = false;
        RHI::GetRHIContext().rhi_create_device(pRHIDevice, pRHIAdapter, deviceDesc);

        pRHIFence = RHI::GetRHIContext().rhi_create_fence(pRHIDevice);

        mRtvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        mDsvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
        mCbvSrvUavDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        // Check 4X MSAA quality support for our back buffer format.
        // All Direct3D 11 capable devices support 4X MASS for all render
        // target formats, so we only need to check quality support.

        D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
        msQualityLevels.Format = mBackBufferFormat;
        msQualityLevels.SampleCount = 4;
        msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
        msQualityLevels.NumQualityLevels = 0;
        md3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msQualityLevels, sizeof(msQualityLevels));

#ifdef _DEBUG
        LogAdapters();
#endif

        CreateCommandObjects();
        CreateSwapChain();
        CreateRtvAndDsvDescriptorHeaps();

        return true;
    }

    void Renderer::CreateCommandObjects()
    {
        pQueue = RHI::GetRHIContext().rhi_get_queue(pRHIDevice, RHI_QUEUE_TYPE_GRAPHICS, 1);
        pCmdPool = RHI::GetRHIContext().rhi_create_command_pool(pQueue, CommandPoolCreateDesc());
        pCmdBuffer = RHI::GetRHIContext().rhi_create_command_buffer(pCmdPool, CommandBufferCreateDesc());
    }

    void Renderer::CreateSwapChain()
    {
        // Release the previous swapchain we will be recreating
        mSwapChain->Release();

        DXGI_SWAP_CHAIN_DESC sd;
        sd.BufferDesc.Width = mClientWidth;
        sd.BufferDesc.Height = mClientHeight;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferDesc.Format = mBackBufferFormat;
        sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.BufferCount = SwapChainBufferCount;
        sd.Windowed = true;
        sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        // Note: Swap chain used queue to perform flush.
        mdxgiFactory->CreateSwapChain(mCommandQueue, &sd, &mSwapChain);
    }

    void Renderer::FlushCommandQueue()
    {
        // Advance the fence value to mark commands up to this fence point
        mCurrentFence++;

        // Add an instruction to the command queue to set a new fence point. Because we
        // are on the GPU timeline, the new fence point won't be set until the GPU finished
        // processing all the commands prior to this Signal()
        mCommandQueue->Signal(mFence, mCurrentFence);

        if(mFence->GetCompletedValue() < mCurrentFence)
        {
            HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);

            // Fire event when GPU hits current fence.
            mFence->SetEventOnCompletion(mCurrentFence, eventHandle);

            // Wait until the GPU hits current fence event is fired.
            WaitForSingleObject(eventHandle, INFINITE);
            CloseHandle(eventHandle);
        }
    }

    ID3D12Resource* Renderer::CurrentBavkBuffer() const
    {
        return mSwapChainBuffer[mCurrBackBuffer];
    }

    D3D12_CPU_DESCRIPTOR_HANDLE Renderer::CurrentBackBufferView() const
    {
        D3D12_CPU_DESCRIPTOR_HANDLE handle;
        handle.ptr = mRtvHeap->GetCPUDescriptorHandleForHeapStart().ptr + (mCurrBackBuffer * mRtvDescriptorSize);

        return handle;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE Renderer::DepthStencilView() const
    {
        return mDsvHeap->GetCPUDescriptorHandleForHeapStart();
    }

    void Renderer::CalculateFrameStats()
    {
        // Code computes the average framed per second, and also the
        // average time it takes to render one frame. These stats
        // are appended to the window caption bar.

        static int frameCnt = 0;
        static float timeElapsed = 0.0f;

        frameCnt++;
    }

    void Renderer::LogAdapters()
    {
        UINT i = 0;
        IDXGIAdapter* adapter = nullptr;
        std::vector<IDXGIAdapter*> adapterList;
        while(mdxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
        {
            DXGI_ADAPTER_DESC desc;
            adapter->GetDesc(&desc);

            std::wstring text = L"***Adapter: ";
            text += desc.Description;
            text += L"\n";

            OutputDebugString(text.c_str());

            adapterList.push_back(adapter);

            ++i;
        }

        for(size_t i = 0; i < adapterList.size(); ++i)
        {
            LogAdapterOutputs(adapterList[i]);
            adapterList[i]->Release();
        }
    }

    void Renderer::LogAdapterOutputs(IDXGIAdapter* adapter)
    {
        UINT i = 0;
        IDXGIOutput* output = nullptr;
        while(adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
        {
            DXGI_OUTPUT_DESC desc;
            output->GetDesc(&desc);

            std::wstring text = L"***Output: ";
            text += desc.DeviceName;
            text += L"\n";
            OutputDebugString(text.c_str());

            LogOutputDisplayModes(output, mBackBufferFormat);

            output->Release();

            ++i;
        }
    }
    void Renderer::LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format)
    {
        UINT count = 0;
        UINT flags = 0;

        // Call with nullptr to get list count.
        output->GetDisplayModeList(format, flags, &count, nullptr);

        std::vector<DXGI_MODE_DESC> modeList(count);
        output->GetDisplayModeList(format, flags, &count, &modeList[0]);

        for(auto& x : modeList)
        {
            UINT n = x.RefreshRate.Numerator;
            UINT d = x.RefreshRate.Denominator;
            std::wstring text = 
                L"Width = " + std::to_wstring(x.Width) + L" " +
                L"Height = " + std::to_wstring(x.Height) + L" " +
                L"Refresh = " + std::to_wstring(n) + L"/" + std::to_wstring(d) +
                L"\n";
            
            ::OutputDebugString(text.c_str());
        }
    }

    void Renderer::BuildDescriptorHeaps()
    {
        D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
        cbvHeapDesc.NumDescriptors = 1;
        cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        cbvHeapDesc.NodeMask = 0;
        md3dDevice->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&mCbvHeap));
    }

    void Renderer::BuildConstantBuffers()
    {
        mObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(md3dDevice, 1, true);

        UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

        D3D12_GPU_VIRTUAL_ADDRESS cbAddress = mObjectCB->Resource()->GetGPUVirtualAddress();
        // Offset to the ith object constant buffer in the buffer.
        int boxCBIndex = 0;
        cbAddress += boxCBIndex*objCBByteSize;

        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
        cbvDesc.BufferLocation = cbAddress;
        cbvDesc.SizeInBytes = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

        md3dDevice->CreateConstantBufferView(&cbvDesc, mCbvHeap->GetCPUDescriptorHandleForHeapStart());
    }

    void Renderer::BuildRootSignature()
    {
        // Shader programs typically require resources as input (constant buffers,
	    // textures, samplers).  The root signature defines the resources the shader
	    // programs expect.  If we think of the shader programs as a function, and
	    // the input resources as function parameters, then the root signature can be
	    // thought of as defining the function signature.

        // Root parameter can be a table, root descriptor or root constants.
        CD3DX12_ROOT_PARAMETER slotRootParameter[1];

        // Create a single descriptor table of CBVs
        CD3DX12_DESCRIPTOR_RANGE cbvTable;
        cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
        slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

        // A root signature is an array of root parameters.
        CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, slotRootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        // create a root signature with a single slot which points to a dscriptor range consisting of a single constant bufer
        ID3DBlob* serializedRootSig = nullptr;
        ID3DBlob* errorBlob = nullptr;
        HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serializedRootSig, &errorBlob);

        if(errorBlob != nullptr)
        {

        }

        if(hr != S_OK)
        {
            return;
        }

        md3dDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(&mRootSignature));
    }

    void Renderer::BuildShadersAndInputLayout()
    {
        HRESULT hr = S_OK;

        mvsByteCode = d3dUtil::CompileShader(L"Shaders\\color.hlsl", nullptr, "VS", "vs_5_0");
        mpsByteCode = d3dUtil::CompileShader(L"Shaders\\color,hlsl", nullptr, "PS", "ps_5_0");

        mInputLayout = 
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
        };
    }

    void Renderer::BuildBoxGeometry()
    {
        std::array<Vertex, 8> vertices = 
        {
            Vertex({DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f), DirectX::XMFLOAT4(DirectX::Colors::White)}),
            Vertex({DirectX::XMFLOAT3(-1.0f, +1.0f, -1.0f), DirectX::XMFLOAT4(DirectX::Colors::Black)}),
            Vertex({DirectX::XMFLOAT3(+1.0f, +1.0f, -1.0f), DirectX::XMFLOAT4(DirectX::Colors::Red)}),
            Vertex({DirectX::XMFLOAT3(+1.0f, -1.0f, -1.0f), DirectX::XMFLOAT4(DirectX::Colors::Green)}),
            Vertex({DirectX::XMFLOAT3(-1.0f, -1.0f, +1.0f), DirectX::XMFLOAT4(DirectX::Colors::Blue)}),
            Vertex({DirectX::XMFLOAT3(-1.0f, +1.0f, +1.0f), DirectX::XMFLOAT4(DirectX::Colors::Yellow)}),
            Vertex({DirectX::XMFLOAT3(+1.0f, +1.0f, +1.0f), DirectX::XMFLOAT4(DirectX::Colors::Cyan)}),
            Vertex({DirectX::XMFLOAT3(+1.0f, -1.0f, +1.0f), DirectX::XMFLOAT4(DirectX::Colors::Magenta)}),
        };

        std::array<std::uint16_t, 36> indices = 
        {
            // front face
            0, 1, 2,
            0, 2, 3,
            // back face
	        4, 6, 5,
	        4, 7, 6,  
	        // left face
	        4, 5, 1,
	        4, 1, 0,  
	        // right face
	        3, 2, 6,
	        3, 6, 7,  
	        // top face
	        1, 5, 6,
	        1, 6, 2,  
	        // bottom face
	        4, 0, 3,
	        4, 3, 7
        };

        const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
        const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

        mBoxGeo = std::make_unique<MeshGeometry>();
        mBoxGeo->Name = "boxGeo";

        D3DCreateBlob(vbByteSize, &mBoxGeo->VertexBufferCPU);
        CopyMemory(mBoxGeo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

        D3DCreateBlob(ibByteSize, &mBoxGeo->IndexBufferCPU);
        CopyMemory(mBoxGeo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

        mBoxGeo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice, mCommandList,vertices.data(), vbByteSize, mBoxGeo->VertexBufferUploader);
        mBoxGeo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice, mCommandList, indices.data(), ibByteSize, mBoxGeo->IndexBufferUploader);

        mBoxGeo->VertexBytesStride = sizeof(Vertex);
        mBoxGeo->VertexBufferByteSize = vbByteSize;
        mBoxGeo->IndexFormat = DXGI_FORMAT_R16_UINT;
        mBoxGeo->IndexBufferByteSize = ibByteSize;

        SubmeshGeometry submesh;
        submesh.IndexCount = (UINT)indices.size();
        submesh.StartIndexLocation = 0;
        submesh.BaseVertexLocation = 0;

        mBoxGeo->DrawArgs["box"] = submesh;
    }

    void Renderer::BuildPSO()
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
        ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
        psoDesc.InputLayout = {mInputLayout.data(), (UINT)mInputLayout.size()};
        psoDesc.pRootSignature = mRootSignature;
        psoDesc.VS = 
        {
          reinterpret_cast<BYTE*>(mvsByteCode->GetBufferPointer()),
          mvsByteCode->GetBufferSize()  
        };
        psoDesc.PS = 
        {
            reinterpret_cast<BYTE*>(mpsByteCode->GetBufferPointer()),
            mpsByteCode->GetBufferSize()  
        };
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = mBackBufferFormat;
        psoDesc.SampleDesc.Count = 1;
        psoDesc.SampleDesc.Quality = 0;
        psoDesc.DSVFormat = mDepthStencilFormat;
        md3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPSO));
    }

    void Renderer::Draw()
    {
        // Reuse the memory associated with command recording.
        // We can onlu reset when the associated command lists have finished execution on the GPU.
        mDirectCmdListAlloc->Reset();

        // A command list can be reset after it has been added to the command queue via ExecuteCommandList.
        // Reusing the command list reuses memory.
        mCommandList->Reset(mDirectCmdListAlloc, mPSO);

        mCommandList->RSSetViewports(1, &mScreenViewport);
        mCommandList->RSSetScissorRects(1, &mScissorRect);

        // Indicate a state transition on the resource usage.
        mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBavkBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

        // Clear the back buffer and depth buffer.
        mCommandList->ClearRenderTargetView(CurrentBackBufferView(), DirectX::Colors::LightSteelBlue, 0, nullptr);
        mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

        // Specify the buffers we are going to render to.
        mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

        ID3D12DescriptorHeap* descriptorHeaps[] = { mCbvHeap };
        mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

        mCommandList->SetGraphicsRootSignature(mRootSignature);

        mCommandList->IASetVertexBuffers(0, 1, &mBoxGeo->VertexBufferView());
        mCommandList->IASetIndexBuffer(&mBoxGeo->IndexBufferView());
        mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        mCommandList->SetGraphicsRootDescriptorTable(0,mCbvHeap->GetGPUDescriptorHandleForHeapStart());

        mCommandList->DrawIndexedInstanced(mBoxGeo->DrawArgs["box"].IndexCount, 1, 0, 0, 0);

        // Indicate a state transition on the resource uasge.
        mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBavkBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

        // Done recording commands.
        mCommandList->Close();

        // Add the command list to the queue for execution.
        ID3D12CommandList* cmdLists[] = {mCommandList};
        mCommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

        // swap the back and front buffers
        mSwapChain->Present(0, 0);
        mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

        // Wait until frame commands are complete. This waiting is inefficient and is
        // done for simplicity. Later we will shadow how to organize our rendering code
        // so we do not have to wait per frame.
        FlushCommandQueue();
    }
}