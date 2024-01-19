#pragma once

#include "graphics/interface/frame_buffer.h"
#include "engine_impl_traits_d3d12.hpp"

namespace Cyber
{
    namespace RenderObject
    {
        class RenderDevice_D3D12_Impl;

        struct IFrameBuffer_D3D12 : public IFrameBuffer 
        {
            
        };

        class CYBER_GRAPHICS_API FrameBuffer_D3D12_Impl final : public FrameBufferBase<EngineD3D12ImplTraits>
        {
        public:
            using RenderDeviceImplType = EngineD3D12ImplTraits::RenderDeviceImplType;
            
            FrameBuffer_D3D12_Impl(class RenderDevice_D3D12_Impl* device);

            virtual ~FrameBuffer_D3D12_Impl()
            {

            }

            friend class RenderObject::RenderDevice_D3D12_Impl;
        };
    }
}
