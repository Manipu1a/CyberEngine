#pragma once
#include "common/cyber_graphics_config.h"
#include "device_object.h"
#include "interface/graphics_types.h"
#include "tools/string_tool.hpp"
#include "platform/memory.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(RenderObject)

// The maximum number of layout elements.
#define CYBER_MAX_LAYOUT_ELEMENTS 16
// Compute layout element offset automatically
#define CYBER_LAYOUT_ELEMENT_AUTO_OFFSET 0xFFFFFFFFU
// Compute layout element stride automatically
#define CYBER_LAYOUT_ELEMENT_AUTO_STRIDE 0xFFFFFFFFU

static const uint32_t MAX_LAYOUT_ELEMENTS = CYBER_MAX_LAYOUT_ELEMENTS;
static const uint32_t LAYOUT_ELEMENT_AUTO_OFFSET = CYBER_LAYOUT_ELEMENT_AUTO_OFFSET;
static const uint32_t LAYOUT_ELEMENT_AUTO_STRIDE = CYBER_LAYOUT_ELEMENT_AUTO_STRIDE;

CYBER_TYPED_ENUM(VERTEX_INPUT_RATE, uint8_t)
{
    INPUT_RATE_VERTEX = 0,
    INPUT_RATE_INSTANCE = 1,
    INPUT_RATE_COUNT,
};

// Description of a single element of the input layout
struct CYBER_GRAPHICS_API VertexAttribute 
{
    /// HLSL semantic. Default value ("ATTRIB") allows HLSL shaders to be converted
    /// to GLSL and used in OpenGL backend as well as compiled to SPIRV and used
    /// in Vulkan backend.
    /// Any value other than default will only work in Direct3D11 and Direct3D12 backends.
    const char8_t* hlsl_semantic = CYBER_UTF8("ATTRIB");

    /// Input index of the element that is specified in the vertex shader.
    /// In Direct3D11 and Direct3D12 backends this is the semantic index.
    uint32_t input_index = 0;

    /// Buffer slot index that this element is read from.
    uint32_t buffer_slot = 0;

    /// Number of components in the element. Allowed values are 1, 2, 3, and 4.
    uint32_t num_components = 0;

    /// Type of the element components, see Diligent::VALUE_TYPE for details.
    VALUE_TYPE value_type = VALUE_TYPE_FLOAT32;
    
    /// For signed and unsigned integer value types
    /// (VT_INT8, VT_INT16, VT_INT32, VT_UINT8, VT_UINT16, VT_UINT32)
    /// indicates if the value should be normalized to [-1,+1] or
    /// [0, 1] range respectively. For floating point types
    /// (VT_FLOAT16 and VT_FLOAT32), this member is ignored.
    bool is_normalized = true;

    /// Relative offset, in bytes, to the element bits.
    /// If this value is set to LAYOUT_ELEMENT_AUTO_OFFSET (default value), the offset will
    /// be computed automatically by placing the element right after the previous one.
    uint32_t relative_offset = LAYOUT_ELEMENT_AUTO_OFFSET;

    /// Stride, in bytes, between two elements, for this buffer slot.
    /// If this value is set to LAYOUT_ELEMENT_AUTO_STRIDE, the stride will be
    /// computed automatically assuming that all elements in the same buffer slot are
    /// packed one after another. If the buffer slot contains multiple layout elements,
    /// they all must specify the same stride or use LAYOUT_ELEMENT_AUTO_STRIDE value.
    uint32_t stride = LAYOUT_ELEMENT_AUTO_STRIDE;

    VERTEX_INPUT_RATE input_rate = INPUT_RATE_VERTEX;

    /// The number of instances to draw using the same per-instance data before advancing
    /// in the buffer by one element.
    uint32_t instance_data_step_rate = 1;

    constexpr VertexAttribute() noexcept {}

    constexpr VertexAttribute(uint32_t _input_index,
                              uint32_t _buffer_slot,
                              uint32_t _num_components,
                              VALUE_TYPE _value_type,
                              bool _is_normalized = VertexAttribute{}.is_normalized,
                              uint32_t _relative_offset = VertexAttribute{}.relative_offset,
                              uint32_t _stride = VertexAttribute{}.stride,
                              VERTEX_INPUT_RATE _input_rate = VertexAttribute{}.input_rate,
                              uint32_t _instance_data_step_rate = VertexAttribute{}.instance_data_step_rate) noexcept :
        input_index(_input_index),
        buffer_slot(_buffer_slot),
        num_components(_num_components),
        value_type(_value_type),
        is_normalized(_is_normalized),
        relative_offset(_relative_offset),
        stride(_stride),
        input_rate(_input_rate),
        instance_data_step_rate(_instance_data_step_rate)
    {}

    constexpr VertexAttribute(const char8_t* _hlsl_semantic,
                              uint32_t _input_index,
                              uint32_t _buffer_slot,
                              uint32_t _num_components,
                              VALUE_TYPE _value_type,
                              bool _is_normalized = VertexAttribute{}.is_normalized,
                              uint32_t _relative_offset = VertexAttribute{}.relative_offset,
                              uint32_t _stride = VertexAttribute{}.stride,
                              VERTEX_INPUT_RATE _input_rate = VertexAttribute{}.input_rate,
                              uint32_t _instance_data_step_rate = VertexAttribute().instance_data_step_rate ) noexcept :
        hlsl_semantic(_hlsl_semantic),
        input_index(_input_index),
        buffer_slot(_buffer_slot),
        num_components(_num_components),
        value_type(_value_type),
        is_normalized(_is_normalized),
        relative_offset(_relative_offset),
        stride(_stride),
        input_rate(_input_rate),
        instance_data_step_rate(_instance_data_step_rate)
    {}

    constexpr VertexAttribute(uint32_t _input_index,
                              uint32_t _buffer_slot,
                              uint32_t _num_components,
                              VALUE_TYPE _value_type,
                              bool _is_normalized,
                              VERTEX_INPUT_RATE _input_rate,
                              uint32_t _instance_data_step_rate = VertexAttribute{}.instance_data_step_rate) noexcept :
        input_index(_input_index),
        buffer_slot(_buffer_slot),
        num_components(_num_components),
        value_type(_value_type),
        is_normalized(_is_normalized),
        relative_offset(VertexAttribute{}.relative_offset),
        stride(VertexAttribute{}.stride),
        input_rate(_input_rate),
        instance_data_step_rate(_instance_data_step_rate)
    {}
                              
    bool operator == (const VertexAttribute& rhs) const
    {   
        return safe_u8string_equal( rhs.hlsl_semantic, hlsl_semantic) &&
               input_index == rhs.input_index &&
               buffer_slot == rhs.buffer_slot &&
               num_components == rhs.num_components &&
               value_type == rhs.value_type &&
               is_normalized == rhs.is_normalized &&
               relative_offset == rhs.relative_offset &&
               stride == rhs.stride &&
               input_rate == rhs.input_rate &&
               instance_data_step_rate == rhs.instance_data_step_rate;
    }

    bool operator != (const VertexAttribute& rhs) const
    {
        return !(*this == rhs);
    }
};

typedef struct VertexAttribute VertexAttribute;

struct CYBER_GRAPHICS_API VertexLayoutDesc
{
    uint32_t attribute_count = 0;
    const VertexAttribute* attributes = nullptr;

    constexpr VertexLayoutDesc() noexcept {}

    constexpr VertexLayoutDesc(uint32_t _attribute_count, const VertexAttribute* _attributes) noexcept :
        attribute_count(_attribute_count),
        attributes(_attributes)
    {}

    bool operator == (const VertexLayoutDesc& rhs) const
    {
        if(attribute_count != rhs.attribute_count)
            return false;

        for(uint32_t i = 0; i < attribute_count; ++i)
        {
            if(attributes[i] != rhs.attributes[i])
                return false;
        }
        return true;
    }

    bool operator != (const VertexLayoutDesc& rhs) const
    {
        return !(*this == rhs);
    }
};

typedef struct VertexLayoutDesc VertexLayoutDesc;

/*
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
*/ 
CYBER_END_NAMESPACE
CYBER_END_NAMESPACE