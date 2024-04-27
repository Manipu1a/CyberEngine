#pragma once
#include "common/cyber_graphics_config.h"
#include "graphics_types.h"
#include "device_object.h"
#include "platform/memory.h"
#include <string>
namespace Cyber
{
    namespace RenderObject
    {
        struct CYBER_GRAPHICS_API IShaderResource : public IDeviceObject
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
        class ShaderResourceBase : public DeviceObjectBase<typename EngineImplTraits::ShaderResourceInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using ShaderResourceInterface = typename EngineImplTraits::ShaderResourceInterface;
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
            using TShaderResourceBase = typename DeviceObjectBase<ShaderResourceInterface, RenderDeviceImplType>;

            ShaderResourceBase(RenderDeviceImplType* device) : TShaderResourceBase(device) {  };
            virtual ~ShaderResourceBase() = default;

            CYBER_FORCE_INLINE void set_name(const char8_t* name)
            {
                this->m_name = name;
                this->m_nameHash = graphics_name_hash(name, std::char_traits<char8_t>::length(name)+1);
            }

            CYBER_FORCE_INLINE const char8_t* get_name() const
            {
                return this->m_name;
            }

            CYBER_FORCE_INLINE uint64_t get_name_hash() const
            {
                return this->m_nameHash;
            }

            CYBER_FORCE_INLINE void set_type(GRAPHICS_RESOURCE_TYPE type)
            {
                this->m_type = type;
            }

            CYBER_FORCE_INLINE GRAPHICS_RESOURCE_TYPE get_type() const
            {
                return this->m_type;
            }

            CYBER_FORCE_INLINE void set_dimension(TEXTURE_DIMENSION dimension)
            {
                this->m_dimension = dimension;
            }

            CYBER_FORCE_INLINE TEXTURE_DIMENSION get_dimension() const
            {
                return this->m_dimension;
            }

            CYBER_FORCE_INLINE void set_set(uint32_t set)
            {
                this->m_set = set;
            }

            CYBER_FORCE_INLINE uint32_t get_set() const
            {
                return this->m_set;
            }

            CYBER_FORCE_INLINE void set_binding(uint32_t binding)
            {
                this->m_binding = binding;
            }

            CYBER_FORCE_INLINE uint32_t get_binding() const
            {
                return this->m_binding;
            }

            CYBER_FORCE_INLINE void set_size(uint32_t size)
            {
                this->m_size = size;
            }

            CYBER_FORCE_INLINE uint32_t get_size() const
            {
                return this->m_size;
            }

            CYBER_FORCE_INLINE void set_offset(uint32_t offset)
            {
                this->m_offset = offset;
            }

            CYBER_FORCE_INLINE uint32_t get_offset() const
            {
                return this->m_offset;
            }

            CYBER_FORCE_INLINE void set_stages(SHADER_STAGE stages)
            {
                this->m_stages = stages;
            }

            CYBER_FORCE_INLINE SHADER_STAGE get_stages() const
            {
                return this->m_stages;
            }

            virtual void free() override
            {
                if(m_name != nullptr)
                    cyber_free((void*)m_name);
            }

        protected:
            const char8_t* m_name;
            uint64_t m_nameHash;
            GRAPHICS_RESOURCE_TYPE m_type;
            TEXTURE_DIMENSION m_dimension;
            uint32_t m_set;
            uint32_t m_binding;
            uint32_t m_size;
            uint32_t m_offset;
            SHADER_STAGE m_stages;
        };
    }

}
