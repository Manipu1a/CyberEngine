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
            virtual const InstanceCreateDesc get_create_desc() const = 0;
        };

        template<typename EngineImplTraits>
        class InstanceBase : public RenderObjectBase<typename EngineImplTraits::InstanceInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
            using TRenderObjectBase = RenderObjectBase<typename EngineImplTraits::InstanceInterface, typename EngineImplTraits::RenderDeviceImplType>;

            InstanceBase(RenderDeviceImplType* device, const InstanceCreateDesc& desc) : TRenderObjectBase(device), m_Desc(desc)
            {
                m_Backend = device->get_backend();
                m_NvAPIStatus = device->get_nvapi_status();
                m_AgsStatus = device->get_ags_status();
                m_EnableSetName = desc.enable_set_name;
            }

            virtual ~InstanceBase() = default;

            virtual const InstanceCreateDesc get_create_desc() const
            {
                return m_Desc;
            }
        protected:
            InstanceCreateDesc m_Desc;
            ERHIBackend m_Backend;
            ERHINvAPI_Status m_NvAPIStatus;
            ERHIAGSReturenCode m_AgsStatus;
            bool m_EnableSetName;
        };
    }
}