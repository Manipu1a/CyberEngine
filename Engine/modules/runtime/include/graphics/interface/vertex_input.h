#pragma once
#include "common/cyber_graphics_config.h"
#include "render_object.h"
#include "common/flags.h"

namespace Cyber
{
    namespace RenderObject
    {

        struct CYBER_GRAPHICS_API IVertexInput
        {
            virtual void free() = 0;
        };

        template<typename EngineImplTraits>
        class VertexInputBase : public RenderObjectBase<typename EngineImplTraits::VertexInputInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;

            VertexInputBase(RenderDeviceImplType* device);
            
            virtual ~VertexInputBase() = default;

            CYBER_FORCE_INLINE void set_name(const char8_t* name)
            {
                this->name = name;
            }

            CYBER_FORCE_INLINE const char8_t* get_name() const
            {
                return this->name;
            }

            CYBER_FORCE_INLINE void set_semantics_name(const char8_t* semantics_name)
            {
                this->semantics_name = semantics_name;
            }

            CYBER_FORCE_INLINE const char8_t* get_semantics_name() const
            {
                return this->semantics_name;
            }

            CYBER_FORCE_INLINE void set_semantics_index(uint32_t semantics_index)
            {
                this->semantics_index = semantics_index;
            }

            CYBER_FORCE_INLINE uint32_t get_semantics_index() const
            {
                return this->semantics_index;
            }

            CYBER_FORCE_INLINE void set_binding(uint32_t binding)
            {
                this->binding = binding;
            }

            CYBER_FORCE_INLINE uint32_t get_binding() const
            {
                return this->binding;
            }

            CYBER_FORCE_INLINE void set_format(ERHIFormat format)
            {
                this->format = format;
            }

            CYBER_FORCE_INLINE ERHIFormat get_format() const
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
            ERHIFormat format;
        };
    }

}