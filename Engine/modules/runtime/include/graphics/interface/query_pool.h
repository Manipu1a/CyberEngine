#pragma once
#include "common/cyber_graphics_config.h"
#include "render_object.h"

namespace Cyber
{
    namespace RenderObject
    {
        struct CYBER_GRAPHICS_API IQueryPool
        {
        };

        template<typename EngineImplTraits>
        class QueryPoolBase : public RenderObjectBase<typename EngineImplTraits::QueryPoolInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;

            QueryPoolBase(RenderDeviceImplType* device);

            virtual ~QueryPoolBase() = default;
        protected:
            uint32_t mCount;
        };
    }

}