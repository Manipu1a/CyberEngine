#pragma once
#include "common/cyber_graphics_config.h"
#include "device_object.h"
#include "interface/graphics_types.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(RenderObject)

CYBER_TYPED_ENUM(BUFFER_VIEW_TYPE, uint8_t)
{
    BUFFER_VIEW_TYPE_UNDEFINED = 0,
    BUFFER_VIEW_UNIFORM_BUFFER, // Uniform Buffer View
    BUFFER_VIEW_SHADER_RESOURCE, // Shader Resource View
    BUFFER_VIEW_UNORDERED_ACCESS, // Unordered Access View
};

struct CYBER_GRAPHICS_API BufferViewCreateDesc
{
    BUFFER_VIEW_TYPE view_type;
    TEXTURE_FORMAT format; // Format of the buffer view, if applicable
    uint32_t first_element; // Index of the first element in the buffer view
    uint32_t element_count; // Number of elements in the buffer view
    uint32_t struct_stride; // Size of each element in bytes

    IBuffer* buffer; // Pointer to the buffer this view is created from
};

struct CYBER_GRAPHICS_API IBuffer_View : public IDeviceObject
{
    virtual const BufferViewCreateDesc& get_create_desc() = 0;
};


template<typename EngineImplTraits>
class CYBER_GRAPHICS_API Buffer_View : public DeviceObjectBase<typename EngineImplTraits::BufferViewInterface, typename EngineImplTraits::RenderDeviceImplType>
{
public:
    using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
    using BufferViewInterface = typename EngineImplTraits::BufferViewInterface;
    using TBuffer_View = DeviceObjectBase<BufferViewInterface, RenderDeviceImplType>;

    Buffer_View(RenderDeviceImplType* device, const BufferViewCreateDesc& desc) : TBuffer_View(device), m_desc(desc)
    {
    }

    virtual ~Buffer_View() {}

    virtual const BufferViewCreateDesc& get_create_desc() override
    {
        return m_desc;
    }

private:
    BufferViewCreateDesc m_desc;
};
CYBER_END_NAMESPACE
CYBER_END_NAMESPACE