#pragma once

namespace Cyber
{
    namespace RenderObject
    {
        class ITexture_D3D12;
        class ITextureView_D3D12;
        class IBuffer_D3D12;
        class IRenderDevice_D3D12;
        class ISwapChain_D3D12;
        class IFrameBuffer_D3D12;


        class Texture_D3D12_Impl;
        class TextureView_D3D12_Impl;
        class Buffer_D3D12_Impl;
        class RenderDevice_D3D12_Impl;
        class SawpChain_D3D12_Impl;
        class FrameBuffer_D3D12_Impl;

        struct EngineD3D12ImplTraits
        {
            using TextureInterface = ITexture_D3D12;
            using TextureViewInterface = ITextureView_D3D12;
            using BufferInterface = IBuffer_D3D12;
            using RenderDeviceInterface = IRenderDevice_D3D12;
            using SwapChainInterface = ISwapChain_D3D12;
            using FrameBufferInterface = IFrameBuffer_D3D12;

            using TextureImplType = Texture_D3D12_Impl;
            using TextureViewImplType = TextureView_D3D12_Impl;
            using BufferImplType = Buffer_D3D12_Impl;
            using RenderDeviceImplType = RenderDevice_D3D12_Impl;
            using SwapChainImplType = SawpChain_D3D12_Impl;
            using FrameBufferImplType = FrameBuffer_D3D12_Impl;
        };
    }
}