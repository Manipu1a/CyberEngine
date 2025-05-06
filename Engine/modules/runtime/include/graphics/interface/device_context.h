#pragma once
#include "common/cyber_graphics_config.h"
#include "graphics_types.h"
#include "frame_buffer.h"
#include "render_device.hpp"

namespace Cyber
{
    namespace RenderObject
    {
        struct DeviceContextDesc
        {
            const char8_t* name;
            
            COMMAND_QUEUE_TYPE queue_type;

            bool is_deferrd_context = false;

            uint8_t context_id = 0;
            
            uint8_t queue_id = 0xFF;
        };

        struct CYBER_GRAPHICS_API IDeviceContext
        {

        };

        template<typename EngineImplTraits>
        class CYBER_GRAPHICS_API DeviceContextBase : public ObjectBase<EngineImplTraits::DeviceContextInterface>
        {
        public:
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;

            DeviceContextBase(RenderDeviceImplType* device, const DeviceContextDesc& desc) : render_device(device), desc(desc) {}
            
            bool is_deferred_context() const { return desc.is_deferrd_context; }
            
            DeviceContextIndex get_context_id() const { return desc.context_id; }

            DeviceContextIndex get_execution_context_id() const
            {
                return is_deferred_context() ? immediate_context_id : get_context_id();
            }
            
            SoftwareQueueIndex get_command_queue_id() const
            {
                return (SoftwareQueueIndex)get_execution_context_id();
            }
            
        protected:
            RenderDeviceImplType* render_device = nullptr;

            DeviceContextDesc desc;

            DeviceContextIndex immediate_context_id = 0xFF;
        };
    }
}


