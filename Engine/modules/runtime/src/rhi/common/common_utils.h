#pragma once

#include "rhi/rhi.h"

namespace Cyber
{
    #define cyber_round_up(value, multiple) (((value + multiple - 1) / (multiple)) * multiple)
    #define cyber_round_down(value, multiple) (value - value % multiple)

    void rhi_util_init_root_signature_tables(RHIRootSignature* rootSignature, const struct RHIRootSignatureCreateDesc& desc);


}



