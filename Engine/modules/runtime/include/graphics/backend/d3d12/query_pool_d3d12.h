#pragma once

#include "graphics/interface/query_pool.h"
#include "engine_impl_traits_d3d12.hpp"

namespace Cyber
{
    namespace RenderObject
    {
        class RenderDevice_D3D12_Impl;
    }

    namespace RenderObject
    {
        // Buffer Interface
        struct CYBER_GRAPHICS_API IQueryPool_D3D12 : public IQueryPool
        {
            
        };

        class CYBER_GRAPHICS_API QueryPool_D3D12_Impl : public QueryPoolBase<EngineD3D12ImplTraits>
        {
        public:
            using TQueryPoolBase = QueryPoolBase<EngineD3D12ImplTraits>;
            using RenderDeviceImplType = EngineD3D12ImplTraits::RenderDeviceImplType;

            QueryPool_D3D12_Impl(class RenderDevice_D3D12_Impl* device) : TQueryPoolBase(device) {}
        protected:
            friend class RenderObject::RenderDevice_D3D12_Impl;
        };

    }
}
