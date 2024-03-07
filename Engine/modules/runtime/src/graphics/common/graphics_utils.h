#pragma once
#include "graphics/interface/graphics_types.h"
namespace Cyber
{
    #define cyber_round_up(value, multiple) (((value + multiple - 1) / (multiple)) * multiple)
    #define cyber_round_down(value, multiple) (value - value % multiple)

    void graphics_util_init_root_signature_tables(struct IRootSignature* rootSignature, const struct RootSignatureCreateDesc& desc);
    void graphics_util_free_root_signature_tables(struct IRootSignature* rootSignature);
    bool graphics_util_pool_free_signature(struct IRootSignaturePool* pool, struct IRootSignature* signature);

    IRootSignaturePool* graphics_util_create_root_signature_pool(const RootSignaturePoolCreateDesc& desc);
    IRootSignature* graphics_util_try_allocate_signature(IRootSignaturePool* pool, IRootSignature* RSTables, const struct RootSignatureCreateDesc& desc);
    bool graphics_util_add_signature(IRootSignaturePool* pool, IRootSignature* sig, const RootSignatureCreateDesc& desc);
    bool graphics_util_pool_free_signature(IRootSignaturePool* pool, IRootSignature* signature);
    void graphics_util_free_root_signature_pool(IRootSignaturePool* pool);

    eastl::string GetHLSLProfileString(SHADER_STAGE stage, ShaderVersion version);
}



