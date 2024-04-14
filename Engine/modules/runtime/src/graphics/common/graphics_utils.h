#pragma once
#include "graphics/interface/graphics_types.h"
#include "interface/root_signature.hpp"
#include "interface/root_signature_pool.h"

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
}



