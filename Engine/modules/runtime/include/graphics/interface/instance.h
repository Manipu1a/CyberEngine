#pragma once
#include "common/cyber_graphics_config.h"
#include "device_object.h"
#include "common/flags.h"
#include "interface/graphics_types.h"

namespace Cyber
{
    namespace RenderObject
    {
        struct CYBER_GRAPHICS_API InstanceCreateDesc
        {
            bool m_enableDebugLayer;
            bool m_enableGpuBasedValidation;
            bool m_enableSetName;
        };

        struct CYBER_GRAPHICS_API IInstance
        {
            virtual const InstanceCreateDesc get_create_desc() const = 0;
        };

        template<typename EngineImplTraits>
        class InstanceBase : public DeviceObjectBase<typename EngineImplTraits::InstanceInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using InstanceInterface = typename EngineImplTraits::InstanceInterface;
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
            using TInstancetBase = typename DeviceObjectBase<InstanceInterface, RenderDeviceImplType>;

            InstanceBase(RenderDeviceImplType* device, const InstanceCreateDesc& desc) : TInstancetBase(device), m_desc(desc)
            {
                m_backend = device->get_backend();
                m_nvAPIStatus = device->get_nvapi_status();
                m_agsStatus = device->get_ags_status();
                m_enableSetName = desc.m_enableSetName;
            }

            virtual ~InstanceBase() = default;

            virtual const InstanceCreateDesc get_create_desc() const
            {
                return m_desc;
            }
        protected:
            InstanceCreateDesc m_desc;
            GRAPHICS_BACKEND m_backend;
            NVAPI_STATUS m_nvAPIStatus;
            AGS_RETURN_CODE m_agsStatus;
            bool m_enableSetName;
        };
    }
}