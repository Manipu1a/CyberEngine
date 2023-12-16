#pragma once
#include "graphics/rhi/rhi.h"

namespace Cyber
{
    #define cyber_round_up(value, multiple) (((value + multiple - 1) / (multiple)) * multiple)
    #define cyber_round_down(value, multiple) (value - value % multiple)

    void rhi_util_init_root_signature_tables(struct RHIRootSignature* rootSignature, const struct RHIRootSignatureCreateDesc& desc);
    void rhi_util_free_root_signature_tables(struct RHIRootSignature* rootSignature);
    bool rhi_util_pool_free_signature(struct RHIRootSignaturePool* pool, struct RHIRootSignature* signature);

    RHIRootSignaturePool* rhi_util_create_root_signature_pool(const RHIRootSignaturePoolCreateDesc& desc);
    RHIRootSignature* rhi_util_try_allocate_signature(RHIRootSignaturePool* pool, RHIRootSignature* RSTables, const struct RHIRootSignatureCreateDesc& desc);
    bool rhi_util_add_signature(RHIRootSignaturePool* pool, RHIRootSignature* sig, const RHIRootSignatureCreateDesc& desc);
    bool rhi_util_pool_free_signature(RHIRootSignaturePool* pool, RHIRootSignature* signature);
    void rhi_util_free_root_signature_pool(RHIRootSignaturePool* pool);

    eastl::string GetHLSLProfileString(ERHIShaderStage stage, ShaderVersion version);
}



