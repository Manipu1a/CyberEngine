#pragma once
#include "common/cyber_graphics_config.h"
#include "device_object.h"

namespace Cyber
{
    namespace RenderObject
    {
        
        class IRootSignature;

        struct CYBER_GRAPHICS_API DescriptorSetCreateDesc
        {
            RenderObject::IRootSignature* root_signature;
            uint32_t set_index;
        };

        struct CYBER_GRAPHICS_API IDescriptorSet : public IDeviceObject
        {
            virtual DescriptorSetCreateDesc get_create_desc() = 0; 
            virtual RenderObject::IRootSignature* get_root_signature() const = 0;
            virtual void set_root_signature(RenderObject::IRootSignature* pRootSignature) = 0;

            virtual uint32_t get_set_index() const = 0;
            virtual void set_set_index(uint32_t setIndex) = 0;
        };


        template<typename EngineImplTraits>
        class DescriptorSetBase : public DeviceObjectBase<typename EngineImplTraits::DescriptorSetInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using DescriptorSetInterface = typename EngineImplTraits::DescriptorSetInterface;
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
            using TDescriptorSetBase = typename DeviceObjectBase<DescriptorSetInterface, RenderDeviceImplType>;
            DescriptorSetBase(RenderDeviceImplType* device, DescriptorSetCreateDesc desc) : TDescriptorSetBase(device) { m_desc = desc; };

            virtual ~DescriptorSetBase() = default;

            virtual RenderObject::IRootSignature* get_root_signature() const
            {
                return m_pRootSignature;
            }

            virtual void set_root_signature(RenderObject::IRootSignature* pRootSignature)
            {
                m_pRootSignature = pRootSignature;
            }

            virtual uint32_t get_set_index() const
            {
                return m_setIndex;
            }

            virtual void set_set_index(uint32_t setIndex)
            {
                m_setIndex = setIndex;
            }

            virtual DescriptorSetCreateDesc get_create_desc() override
            {
                return m_desc;
            }
        protected:
            DescriptorSetCreateDesc m_desc;
            RenderObject::IRootSignature* m_pRootSignature;
            uint32_t m_setIndex;
        };
    }

}