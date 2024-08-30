#pragma once
#include "graphics/interface/graphics_types.h"
#include "interface/root_signature.hpp"
#include "interface/root_signature_pool.h"
#include "EASTL/string.h"
#include "core/debug.h"
namespace Cyber
{
    #define cyber_round_up(value, multiple) (((value + multiple - 1) / (multiple)) * multiple)
    #define cyber_round_down(value, multiple) (value - value % multiple)

    void graphics_util_init_root_signature_tables(struct RenderObject::IRootSignature* rootSignature, const struct RenderObject::RootSignatureCreateDesc& desc);
    void graphics_util_free_root_signature_tables(struct RenderObject::IRootSignature* rootSignature);
    bool graphics_util_pool_free_signature(struct RenderObject::RootSignaturePoolBase* pool, struct RenderObject::IRootSignature* signature);

    RenderObject::RootSignaturePoolBase* graphics_util_create_root_signature_pool(const RenderObject::RootSignaturePoolCreateDesc& desc);
    RenderObject::IRootSignature* graphics_util_try_allocate_signature(RenderObject::RootSignaturePoolBase* pool, RenderObject::IRootSignature* RSTables, const struct RenderObject::RootSignatureCreateDesc& desc);
    bool graphics_util_add_signature(RenderObject::RootSignaturePoolBase* pool, RenderObject::IRootSignature* sig, const RenderObject::RootSignatureCreateDesc& desc);
    bool graphics_util_pool_free_signature(RenderObject::RootSignaturePoolBase* pool, RenderObject::IRootSignature* signature);
    void graphics_util_free_root_signature_pool(RenderObject::RootSignaturePoolBase* pool);

    eastl::string GetHLSLProfileString(SHADER_STAGE stage, ShaderVersion version);

    template < VALUE_TYPE >
    struct VALUE_TYPE2CType
    {};

    template <> struct VALUE_TYPE2CType<VALUE_TYPE_INT8>
    {
        typedef int8_t cType;
    };

    template <> struct VALUE_TYPE2CType<VALUE_TYPE_INT16>
    {
        typedef uint16_t cType;
    };

    template <> struct VALUE_TYPE2CType<VALUE_TYPE_INT32>
    {
        typedef int32_t cType;
    };

    template <> struct VALUE_TYPE2CType<VALUE_TYPE_UINT8>
    {
        typedef uint8_t cType;
    };

    template <> struct VALUE_TYPE2CType<VALUE_TYPE_UINT16>
    {
        typedef uint16_t cType;
    };

    template <> struct VALUE_TYPE2CType<VALUE_TYPE_UINT32>
    {
        typedef uint32_t cType;
    };

    template <> struct VALUE_TYPE2CType<VALUE_TYPE_FLOAT16>
    {
        typedef uint16_t cType;
    };

    template <> struct VALUE_TYPE2CType<VALUE_TYPE_FLOAT32>
    {
        typedef float cType;
    };

    template <> struct VALUE_TYPE2CType<VALUE_TYPE_FLOAT64>
    {
        typedef double cType;
    };

    static constexpr uint32_t ValueTypeToSizeMap[]
    {
        0,
        sizeof(VALUE_TYPE2CType<VALUE_TYPE_INT8>::cType),
        sizeof(VALUE_TYPE2CType<VALUE_TYPE_INT16>::cType),
        sizeof(VALUE_TYPE2CType<VALUE_TYPE_INT32>::cType),
        sizeof(VALUE_TYPE2CType<VALUE_TYPE_UINT8>::cType),
        sizeof(VALUE_TYPE2CType<VALUE_TYPE_UINT16>::cType),
        sizeof(VALUE_TYPE2CType<VALUE_TYPE_UINT32>::cType),
        sizeof(VALUE_TYPE2CType<VALUE_TYPE_FLOAT16>::cType),
        sizeof(VALUE_TYPE2CType<VALUE_TYPE_FLOAT32>::cType),
        sizeof(VALUE_TYPE2CType<VALUE_TYPE_FLOAT64>::cType),
    };

    // Returns the size of the value type in bytes
    inline uint32_t GetValueSize(VALUE_TYPE Val)
    {
        cyber_assert(Val < VALUE_TYPE_COUNT, "Invalid value type");
        return ValueTypeToSizeMap[Val];
    }
}



