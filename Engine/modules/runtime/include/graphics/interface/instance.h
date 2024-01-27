#pragma once
#include "common/cyber_graphics_config.h"
#include "render_object.h"
#include "common/flags.h"
namespace Cyber
{
    namespace RenderObject
    {
        struct CYBER_GRAPHICS_API InstanceCreateDesc
        {
            bool enable_debug_layer;
            bool enable_gpu_based_validation;
            bool enable_set_name;
        };

        struct CYBER_GRAPHICS_API IInstance
        {
            
        };

        template<typename EngineImplTraits>
        class InstanceBase : public RenderObjectBase<typename EngineImplTraits::InstanceInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;

            InstanceBase(RenderDeviceImplType* device);
            virtual ~InstanceBase() = default;
        protected:
            ERHIBackend mBackend;
            ERHINvAPI_Status mNvAPIStatus;
            ERHIAGSReturenCode mAgsStatus;
            bool mEnableSetName;
        };
    }
}