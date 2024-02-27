#pragma once
#include "common/cyber_graphics_config.h"
#include "render_object.h"
#include "rhi.h"

namespace Cyber
{
    namespace RenderObject
    {

        struct CYBER_GRAPHICS_API IShaderResource
        {
            virtual void free() = 0;
        };

        template<typename EngineImplTraits>
        class ShaderResourceBase : public RenderObjectBase<typename EngineImplTraits::ShaderResourceInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;

            ShaderResourceBase(RenderDeviceImplType* device);
            virtual ~ShaderResourceBase() = default;

            CYBER_FORCE_INLINE void set_name(const char8_t* name)
            {
                this->name = name;
                this->name_hash = graphics_name_hash(name, strlen(name + 1));
            }

            CYBER_FORCE_INLINE const char8_t* get_name() const
            {
                return this->name;
            }

            CYBER_FORCE_INLINE uint64_t get_name_hash() const
            {
                return this->name_hash;
            }

            CYBER_FORCE_INLINE void set_type(ERHIResourceType type)
            {
                this->type = type;
            }

            CYBER_FORCE_INLINE ERHIResourceType get_type() const
            {
                return this->type;
            }

            CYBER_FORCE_INLINE void set_dimension(ERHITextureDimension dimension)
            {
                this->dimension = dimension;
            }

            CYBER_FORCE_INLINE ERHITextureDimension get_dimension() const
            {
                return this->dimension;
            }

            CYBER_FORCE_INLINE void set_set(uint32_t set)
            {
                this->set = set;
            }

            CYBER_FORCE_INLINE uint32_t get_set() const
            {
                return this->set;
            }

            CYBER_FORCE_INLINE void set_binding(uint32_t binding)
            {
                this->binding = binding;
            }

            CYBER_FORCE_INLINE uint32_t get_binding() const
            {
                return this->binding;
            }

            CYBER_FORCE_INLINE void set_size(uint32_t size)
            {
                this->size = size;
            }

            CYBER_FORCE_INLINE uint32_t get_size() const
            {
                return this->size;
            }

            CYBER_FORCE_INLINE void set_offset(uint32_t offset)
            {
                this->offset = offset;
            }

            CYBER_FORCE_INLINE uint32_t get_offset() const
            {
                return this->offset;
            }

            CYBER_FORCE_INLINE void set_stages(ERHIShaderStages stages)
            {
                this->stages = stages;
            }

            CYBER_FORCE_INLINE ERHIShaderStages get_stages() const
            {
                return this->stages;
            }

            virtual void free() override
            {
                cyber_free((void*)name);
            }
        protected:
            const char8_t* name;
            uint64_t name_hash;
            ERHIResourceType type;
            ERHITextureDimension dimension;
            uint32_t set;
            uint32_t binding;
            uint32_t size;
            uint32_t offset;
            ERHIShaderStages stages;
        };
    }

}
