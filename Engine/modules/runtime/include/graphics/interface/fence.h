#pragma once
#include "common/cyber_graphics_config.h"
#include "render_object.h"
namespace Cyber
{
    namespace RenderObject
    {

        struct CYBER_GRAPHICS_API IFence
        {
            
        };

        template<typename EngineImplTraits>
        class FenceBase : public RenderObjectBase<typename EngineImplTraits::FenceInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
            using FenceInterface = typename EngineImplTraits::FenceInterface;
            using TFenceBase = typename FenceBase<FenceInterface, RenderDeviceImplType>;

            FenceBase(RenderDeviceImplType* device) : TFenceBase(device) {  };
            virtual ~FenceBase() = default;
        protected:
        
        };
    }

}