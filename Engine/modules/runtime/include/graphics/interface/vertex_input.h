#pragma once
#include "common/cyber_graphics_config.h"
#include "device_object.h"
#include "interface/graphics_types.h"
#include "platform/memory.h"

namespace Cyber
{
    namespace RenderObject
    {

        struct CYBER_GRAPHICS_API IVertexInput : public IDeviceObject
        {
            virtual void free() = 0;

            virtual void set_name(const char8_t* name) = 0;
            virtual const char8_t* get_name() const = 0;

            virtual void set_semantics_name(const char8_t* semantics_name) = 0;
            virtual const char8_t* get_semantics_name() const = 0;

            virtual void set_semantics_index(uint32_t semantics_index) = 0;
            virtual uint32_t get_semantics_index() const = 0;

            virtual void set_binding(uint32_t binding) = 0;
            virtual uint32_t get_binding() const = 0;

            virtual void set_format(TEXTURE_FORMAT format) = 0;
            virtual TEXTURE_FORMAT get_format() const = 0;

        };

        template<typename EngineImplTraits>
        class VertexInputBase : public DeviceObjectBase<typename EngineImplTraits::VertexInputInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using VertexInputInterface = typename EngineImplTraits::VertexInputInterface;
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
            using TVertexInputBase = typename DeviceObjectBase<VertexInputInterface, RenderDeviceImplType>;

            VertexInputBase(RenderDeviceImplType* device) : TVertexInputBase(device) {  };
            
            virtual ~VertexInputBase() = default;

            CYBER_FORCE_INLINE virtual void set_name(const char8_t* name) override
            {
                this->name = name;
            }

            CYBER_FORCE_INLINE virtual const char8_t* get_name() const override
            {
                return this->name;
            }

            CYBER_FORCE_INLINE virtual void set_semantics_name(const char8_t* semantics_name) override
            {
                this->semantics_name = semantics_name;
            }

            CYBER_FORCE_INLINE virtual const char8_t* get_semantics_name() const override
            {
                return this->semantics_name;
            }

            CYBER_FORCE_INLINE virtual void set_semantics_index(uint32_t semantics_index) override
            {
                this->semantics_index = semantics_index;
            }

            CYBER_FORCE_INLINE virtual uint32_t get_semantics_index() const override
            {
                return this->semantics_index;
            }

            CYBER_FORCE_INLINE virtual void set_binding(uint32_t binding) override
            {
                this->binding = binding;
            }

            CYBER_FORCE_INLINE virtual uint32_t get_binding() const override
            {
                return this->binding;
            }

            CYBER_FORCE_INLINE virtual void set_format(TEXTURE_FORMAT format) override
            {
                this->format = format;
            }

            CYBER_FORCE_INLINE virtual TEXTURE_FORMAT get_format() const override
            {
                return this->format;
            }

            virtual void free() override
            {
                cyber_free((void*)name);
            }
        protected:
            // resource name
            const char8_t* name;
            const char8_t* semantics_name;
            uint32_t semantics_index;
            uint32_t binding;
            TEXTURE_FORMAT format;
        };
    }

}
