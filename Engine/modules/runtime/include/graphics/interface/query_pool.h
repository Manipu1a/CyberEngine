#pragma once
#include "common/cyber_graphics_config.h"
#include "device_object.h"

namespace Cyber
{
    namespace RenderObject
    {
        struct CYBER_GRAPHICS_API IQueryPool
        {
        };

        template<typename EngineImplTraits>
        class QueryPoolBase : public DeviceObjectBase<typename EngineImplTraits::QueryPoolInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using QueryPoolInterface = typename EngineImplTraits::QueryPoolInterface;
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
            using TQueryPoolBase = typename DeviceObjectBase<QueryPoolInterface, RenderDeviceImplType>;

            QueryPoolBase(RenderDeviceImplType* device) : TQueryPoolBase(device) {  };

            virtual ~QueryPoolBase() = default;
        protected:
            uint32_t m_count;
        };
    }

}