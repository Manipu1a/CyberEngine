#include "graphics/backend/d3d12/root_signature_d3d12.h"
CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(RenderObject)

RootSignature_D3D12_Impl::~RootSignature_D3D12_Impl()
{
    if (dxRootSignature)
    {
        dxRootSignature->Release();
        dxRootSignature = nullptr;
    }
}

void RootSignature_D3D12_Impl::analyze_signature()
{
    memset(bind_slots, 0, sizeof(bind_slots));
    bhas_uavs = false;
    bhas_srvs = false;
    bhas_cbvs = false;
    bhas_samplers = false;
    bhas_root_cbs = false;

    for(uint32_t i = 0; i < root_signature_desc.Desc_1_1.NumParameters; ++i)
    {
        auto& curr_parameter = root_signature_desc.Desc_1_1.pParameters[i];
        // get binding space for the parameter
        uint32_t parameter_binding_space = ~0u;
        switch(curr_parameter.ParameterType)
        {
            case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
            {
                cyber_check(curr_parameter.DescriptorTable.NumDescriptorRanges == 1);
                parameter_binding_space = curr_parameter.DescriptorTable.pDescriptorRanges[0].RegisterSpace;

            }
            break;
            case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
            {
                parameter_binding_space = curr_parameter.Constants.RegisterSpace;
            }
            break;
            case D3D12_ROOT_PARAMETER_TYPE_CBV:
            case D3D12_ROOT_PARAMETER_TYPE_SRV:
            case D3D12_ROOT_PARAMETER_TYPE_UAV:
            {
                parameter_binding_space = curr_parameter.Descriptor.RegisterSpace;
            }
            break;
            default:
                // Unsupported parameter type
                break;
        }

        SHADER_STAGE shader_stage = SHADER_STAGE::SHADER_STAGE_COUNT;
        switch(curr_parameter.ShaderVisibility)
        {
            case D3D12_SHADER_VISIBILITY_ALL:
            {

            }
            break;
            case D3D12_SHADER_VISIBILITY_VERTEX:
            {
                shader_stage = SHADER_STAGE::SHADER_STAGE_VERT;
                break;
            }
            case D3D12_SHADER_VISIBILITY_GEOMETRY:
            {
                shader_stage = SHADER_STAGE::SHADER_STAGE_GEOM;
                break;
            }
            case D3D12_SHADER_VISIBILITY_PIXEL:
            {
                shader_stage = SHADER_STAGE::SHADER_STAGE_FRAG;
                break;
            }
            case D3D12_SHADER_VISIBILITY_AMPLIFICATION:
            {
                shader_stage = SHADER_STAGE::SHADER_STAGE_AMPLIFICATION;
                break;
            }
            case D3D12_SHADER_VISIBILITY_MESH:
            {
                shader_stage = SHADER_STAGE::SHADER_STAGE_MESH;
                break;
            }
            default:
                break;
        }

        switch(curr_parameter.ParameterType)
        {
            case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
            {
                const auto& current_range = curr_parameter.DescriptorTable.pDescriptorRanges[0];
                cyber_check(current_range.BaseShaderRegister == 0);
                cyber_check(current_range.RegisterSpace == parameter_binding_space);

                switch(current_range.RangeType)
                {
                case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
                {
                    set_max_src_count(shader_stage, current_range.NumDescriptors);
                    set_srv_root_descriptor_table_bind_slot(shader_stage, i);
                }
                break;
                case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
                {
                    set_max_uav_count(shader_stage, current_range.NumDescriptors);
                    set_uav_root_descriptor_table_bind_slot(shader_stage, i);
                }
                break;
                case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
                {
                    set_max_cbv_count(shader_stage, current_range.NumDescriptors);
                    set_cbv_root_descriptor_table_bind_slot(shader_stage, i);
                }
                break;
                default:
                    cyber_check(false);
                    break;
                }
            }
            break;
            case D3D12_ROOT_PARAMETER_TYPE_CBV:
            {
                increment_max_src_count(shader_stage, 1);
                if(curr_parameter.Descriptor.ShaderRegister == 0)
                {
                    set_cbv_root_descriptor_bind_slot(shader_stage, i);
                }

                update_cbv_register_mask_with_descriptor(shader_stage, curr_parameter.Descriptor);
            }
            break;
            default:
                cyber_check(false);
                break;
        }

    }
}

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE