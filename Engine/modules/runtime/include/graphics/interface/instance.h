#pragma once
#include "common/cyber_graphics_config.h"
#include "device_object.h"
#include "common/flags.h"
#include "interface/render_device.hpp"
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

        struct CYBER_GRAPHICS_API IInstance : public IDeviceObject
        {
            virtual const InstanceCreateDesc get_create_desc() const = 0;
            virtual void optional_enable_debug_layer() = 0;
            virtual void initialize_environment() = 0;
            virtual void de_initialize_environment() = 0;
            virtual void enum_adapters(IAdapter** adapters, uint32_t* adapterCount) = 0;
            virtual void query_all_adapters(uint32_t& count, bool& foundSoftwareAdapter) = 0;
            virtual IRenderDevice* create_render_device(IAdapter* adapter, const RenderDeviceCreateDesc& desc) = 0;

            virtual void free() = 0;
        };

        template<typename EngineImplTraits>
        class InstanceBase : public ObjectBase<typename EngineImplTraits::InstanceInterface>
        {
        public:
            using InstanceInterface = typename EngineImplTraits::InstanceInterface;
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
            using TInstancetBase = ObjectBase<InstanceInterface>;

            InstanceBase(const InstanceCreateDesc& desc) : TInstancetBase(), m_desc(desc)
            {
               // m_backend = device->get_backend();
               // m_nvAPIStatus = device->get_nvapi_status();
               // m_agsStatus = device->get_ags_status();
                m_enableSetName = desc.m_enableSetName;
            }

            virtual ~InstanceBase() = default;

            virtual const InstanceCreateDesc get_create_desc() const override
            {
                return m_desc;
            }

            RenderDeviceImplType* get_render_device() const
            {
                return m_renderDevice;
            }
            
            
            virtual void free() override
            {
                delete this;
            }
        protected:
            InstanceCreateDesc m_desc;
            GRAPHICS_BACKEND m_backend;
            NVAPI_STATUS m_nvAPIStatus;
            AGS_RETURN_CODE m_agsStatus;
            bool m_enableSetName;
            RenderDeviceImplType* m_renderDevice;
        };
    }
}