#pragma once

namespace Cyber
{
    #define cyber_round_up(value, multiple) (((value + multiple - 1) / (multiple)) * multiple)
    #define cyber_round_down(value, multiple) (value - value % multiple)

    void rhi_util_init_root_signature_tables(struct RHIRootSignature* rootSignature, const struct RHIRootSignatureCreateDesc& desc);
    void rhi_util_free_root_signature_tables(struct RHIRootSignature* rootSignature);
}



