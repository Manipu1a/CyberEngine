#pragma once
#include "common/cyber_graphics_config.h"
#include "render_object.h"
#include "graphics_types.h"

namespace Cyber
{
    namespace RenderObject
    {
        struct CYBER_GRAPHICS_API IShaderResource
        {
            virtual void free() = 0;

            virtual void set_name(const char8_t* name) = 0;
            virtual const char8_t* get_name() const = 0;
            virtual uint64_t get_name_hash() const = 0;
            
            virtual void set_type(GRAPHICS_RESOURCE_TYPE type) = 0;
            virtual GRAPHICS_RESOURCE_TYPE get_type() const = 0;

            virtual void set_dimension(TEXTURE_DIMENSION dimension) = 0;
            virtual TEXTURE_DIMENSION get_dimension() const = 0;

            virtual void set_set(uint32_t set) = 0;
            virtual uint32_t get_set() const = 0;

            virtual void set_binding(uint32_t binding) = 0;
            virtual uint32_t get_binding() const = 0;

            virtual void set_size(uint32_t size) = 0;
            virtual uint32_t get_size() const = 0;

            virtual void set_offset(uint32_t offset) = 0;
            virtual uint32_t get_offset() const = 0;

            virtual void set_stages(SHADER_STAGE stages) = 0;
            virtual SHADER_STAGE get_stages() const = 0;
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
                this->m_pName = name;
                this->m_NameHash = graphics_name_hash(name, strlen(name + 1));
            }

            CYBER_FORCE_INLINE const char8_t* get_name() const
            {
                return this->m_pName;
            }

            CYBER_FORCE_INLINE uint64_t get_name_hash() const
            {
                return this->m_NameHash;
            }

            CYBER_FORCE_INLINE void set_type(GRAPHICS_RESOURCE_STATE type)
            {
                this->m_Type = type;
            }

            CYBER_FORCE_INLINE GRAPHICS_RESOURCE_STATE get_type() const
            {
                return this->m_Type;
            }

            CYBER_FORCE_INLINE void set_dimension(TEXTURE_DIMENSION dimension)
            {
                this->m_Dimension = dimension;
            }

            CYBER_FORCE_INLINE TEXTURE_DIMENSION get_dimension() const
            {
                return this->m_Dimension;
            }

            CYBER_FORCE_INLINE void set_set(uint32_t set)
            {
                this->m_Set = set;
            }

            CYBER_FORCE_INLINE uint32_t get_set() const
            {
                return this->m_Set;
            }

            CYBER_FORCE_INLINE void set_binding(uint32_t binding)
            {
                this->m_Binding = binding;
            }

            CYBER_FORCE_INLINE uint32_t get_binding() const
            {
                return this->m_Binding;
            }

            CYBER_FORCE_INLINE void set_size(uint32_t size)
            {
                this->m_Size = size;
            }

            CYBER_FORCE_INLINE uint32_t get_size() const
            {
                return this->m_Size;
            }

            CYBER_FORCE_INLINE void set_offset(uint32_t offset)
            {
                this->m_Offset = offset;
            }

            CYBER_FORCE_INLINE uint32_t get_offset() const
            {
                return this->m_Offset;
            }

            CYBER_FORCE_INLINE void set_stages(SHADER_STAGE stages)
            {
                this->m_Stages = stages;
            }

            CYBER_FORCE_INLINE SHADER_STAGE get_stages() const
            {
                return this->m_Stages;
            }

            virtual void free() override
            {
                cyber_free((void*)m_pName);
            }

        protected:
            const char8_t* m_pName;
            uint64_t m_NameHash;
            GRAPHICS_RESOURCE_TYPE m_Type;
            TEXTURE_DIMENSION m_Dimension;
            uint32_t m_Set;
            uint32_t m_Binding;
            uint32_t m_Size;
            uint32_t m_Offset;
            SHADER_STAGE m_Stages;
        };
    }

}
