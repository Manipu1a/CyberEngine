#pragma once
#include "common/cyber_graphics_config.h"
#include "device_object.h"

namespace Cyber
{
    namespace RenderObject
    {
        class IInstance;
        
        struct CYBER_GRAPHICS_API AdapterDetail
        {
            uint32_t m_uniformBufferAlignment;
            uint32_t m_uniformBufferTextureAlignment;
            uint32_t m_uploadBufferTextureRowPitchAlignment;
            uint32_t m_maxVertexInputBindings;
            uint32_t m_waveLaneCount;
            uint32_t m_hostVisibleVRamBudget;
            bool m_supportHostVisibleVRam : 1;
            bool m_multiDrawIndirect : 1;
            bool m_supportGeomShader : 1;
            bool m_supportTessellation : 1;
            bool m_isUma : 1;
            bool m_isVirtual : 1;
            bool m_isCpu : 1;
        };

        struct CYBER_GRAPHICS_API IAdapter : public IDeviceObject
        {
            virtual IInstance* get_instance() const = 0;
            virtual void free() = 0;
        };

        template<typename EngineImplTraits>
        class AdapterBase : public DeviceObjectBase<typename EngineImplTraits::AdapterInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using AdapterInterface = typename EngineImplTraits::AdapterInterface;
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
            using TRenderObjectBase = typename DeviceObjectBase<AdapterInterface, RenderDeviceImplType>;

            AdapterBase(RenderDeviceImplType* device) : TRenderObjectBase(device) {}

            virtual ~AdapterBase() = default;

            virtual IInstance* get_instance() const override final
            {
                return m_pInstance;
            }
            
            virtual void free() override final
            {
                delete this;
            }
        protected:
            class IInstance* m_pInstance = nullptr;
        };
    }

}