#pragma once

#include "engine_impl_traits_d3d12.hpp"
#include "interface/root_signature.hpp"
#include "interface/shader_reflection.hpp"
#include "eastl/map.h"
#include "eastl/array.h"
#include "d3dx12_root_signature.h"
#include "graphics_types_d3d12.h"
#include "CyberLog/Log.h"


namespace Cyber
{
    namespace RenderObject
    {
        class RenderDevice_D3D12_Impl;
    }

    namespace RenderObject
    {
        struct BindShaderResource
        {
            uint8_t max_cbv_count = 0;
            uint8_t max_srv_count = 0;
            uint8_t max_uav_count = 0;
            uint8_t max_sampler_count = 0;
        };

        enum RootParameterKeys
        {
            PS_SRVs,
            PS_CBVs,
            PS_RootCBVs,
            PS_Samplers,
            VS_SRVs,
            VS_CBVs,
            VS_RootCBVs,
            VS_Samplers,
            VS_UAVs,
            GS_SRVs,
            GS_CBVs,
            GS_RootCBVs,
            GS_Samplers,
            MS_SRVs,
            MS_CBVs,
            MS_RootCBVs,
            MS_Samplers,
            AS_SRVs,
            AS_CBVs,
            AS_RootCBVs,
            AS_Samplers,
            ALL_SRVs,
            ALL_CBVs,
            ALL_RootCBVs,
            ALL_Samplers,
            ALL_UAVs, // non-VS stages (PS, CS, etc.)
            RPK_RootParameterKeyCount,
        };

        // Buffer Interface
        struct CYBER_GRAPHICS_API IRootSignature_D3D12 : public IRootSignature
        {
            
        };

        class CYBER_GRAPHICS_API RootSignature_D3D12_Impl : public RootSignatureBase<EngineD3D12ImplTraits>
        {
        public:
            using TRootSignatureBase = RootSignatureBase<EngineD3D12ImplTraits>;
            using RenderDeviceImplType = EngineD3D12ImplTraits::RenderDeviceImplType;

            RootSignature_D3D12_Impl(class RenderDevice_D3D12_Impl* device, const RootSignatureCreateDesc& desc) : TRootSignatureBase(device, desc) {}

            virtual ~RootSignature_D3D12_Impl();

            const RenderObject::ShaderRegisterCount& get_register_counts(ShaderVisibility visibility) const
            {
                return register_counts_array[visibility];
            }

            void set_register_counts(ShaderVisibility visibility, const RenderObject::ShaderRegisterCount& counts)
            {
                register_counts_array[visibility] = counts;
            }

            void set_root_parameter(CD3DX12_ROOT_PARAMETER1* root_parameters, uint32_t count)
            {
                for (uint32_t i = 0; i < count && i < MAX_ROOT_PARAMETERS; ++i)
                {
                    root_parameters[i] = root_parameters[i];
                }
            }

            void set_root_descriptor_range(CD3DX12_DESCRIPTOR_RANGE1* root_descriptor_ranges, uint32_t count)
            {
                for (uint32_t i = 0; i < count && i < MAX_ROOT_PARAMETERS; ++i)
                {
                    this->root_descriptor_ranges[i] = root_descriptor_ranges[i];
                }
            }

            void set_versioned_root_signature_desc(const CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC& desc)
            {
                root_signature_desc = desc;
            }

            // 统计根签名的参数
            void analyze_signature();

            void set_srv_root_descriptor_table_bind_slot(SHADER_STAGE shader_stage, uint8_t root_parameter_index)
            {
                uint8_t* bind_slot = nullptr;
                switch (shader_stage)
                {
                    case SHADER_STAGE::SHADER_STAGE_VERT:
                        bind_slot = &bind_slots[VS_SRVs];
                        break;
                    case SHADER_STAGE::SHADER_STAGE_FRAG:
                        bind_slot = &bind_slots[PS_SRVs];
                        break;
                    case SHADER_STAGE::SHADER_STAGE_GEOM:
                        bind_slot = &bind_slots[GS_SRVs];
                        break;
                    case SHADER_STAGE::SHADER_STAGE_MESH:
                        bind_slot = &bind_slots[MS_SRVs];
                        break;
                    case SHADER_STAGE::SHADER_STAGE_AMPLIFICATION:
                        bind_slot = &bind_slots[AS_SRVs];
                        break;
                    case SHADER_STAGE::SHADER_STAGE_COMPUTE:
                    case SHADER_STAGE::SHADER_STAGE_COUNT:
                        bind_slot = &bind_slots[ALL_SRVs];
                        break;
                    default:
                        cyber_check(false);
                        return;
                }

                *bind_slot = root_parameter_index;

                has_srvs = true;
            }

            void set_sampler_root_descriptor_table_bind_slot(SHADER_STAGE shader_stage, uint8_t root_parameter_index)
            {
                uint8_t* bind_slot = nullptr;
                switch (shader_stage)
                {
                    case SHADER_STAGE::SHADER_STAGE_VERT:
                        bind_slot = &bind_slots[VS_Samplers];
                        break;
                    case SHADER_STAGE::SHADER_STAGE_FRAG:
                        bind_slot = &bind_slots[PS_Samplers];
                        break;
                    case SHADER_STAGE::SHADER_STAGE_GEOM:
                        bind_slot = &bind_slots[GS_Samplers];
                        break;
                    case SHADER_STAGE::SHADER_STAGE_MESH:
                        bind_slot = &bind_slots[MS_Samplers];
                        break;
                    case SHADER_STAGE::SHADER_STAGE_AMPLIFICATION:
                        bind_slot = &bind_slots[AS_Samplers];
                        break;
                    case SHADER_STAGE::SHADER_STAGE_COMPUTE:
                    case SHADER_STAGE::SHADER_STAGE_COUNT:
                        bind_slot = &bind_slots[ALL_Samplers];
                        break;
                    default:
                        cyber_check(false);
                        return;
                }

                *bind_slot = root_parameter_index;

                has_samplers = true;
            }

            void set_cbv_root_descriptor_table_bind_slot(SHADER_STAGE shader_stage, uint8_t root_parameter_index)
            {
                uint8_t* bind_slot = nullptr;
                switch (shader_stage)
                {
                    case SHADER_STAGE::SHADER_STAGE_VERT:
                        bind_slot = &bind_slots[VS_CBVs];
                        break;
                    case SHADER_STAGE::SHADER_STAGE_FRAG:
                        bind_slot = &bind_slots[PS_CBVs];
                        break;
                    case SHADER_STAGE::SHADER_STAGE_GEOM:
                        bind_slot = &bind_slots[GS_CBVs];
                        break;
                    case SHADER_STAGE::SHADER_STAGE_MESH:
                        bind_slot = &bind_slots[MS_CBVs];
                        break;
                    case SHADER_STAGE::SHADER_STAGE_AMPLIFICATION:
                        bind_slot = &bind_slots[AS_CBVs];
                        break;
                    case SHADER_STAGE::SHADER_STAGE_COMPUTE:
                    case SHADER_STAGE::SHADER_STAGE_COUNT:
                        bind_slot = &bind_slots[ALL_CBVs];
                        break;
                    default:
                        cyber_check(false);
                        return;
                }

                *bind_slot = root_parameter_index;

                has_cbvs = true;
            }

            void set_uav_root_descriptor_table_bind_slot(SHADER_STAGE shader_stage, uint8_t root_parameter_index)
            {
                const uint32_t map_slot_index  = shader_stage == SHADER_STAGE_VERT ? VS_UAVs : ALL_UAVs;
                uint8_t* bind_slot = &bind_slots[map_slot_index];

                *bind_slot = root_parameter_index;

                has_uavs = true;
            }
            
        protected:
            static constexpr uint32_t MAX_ROOT_PARAMETERS = 32;

            eastl::array<RenderObject::ShaderRegisterCount, ShaderVisibility::SV_SHADERVISIBILITY_COUNT> register_counts_array;

            ID3D12RootSignature* dxRootSignature;

            CD3DX12_ROOT_PARAMETER1 root_parameters[MAX_ROOT_PARAMETERS];
            CD3DX12_DESCRIPTOR_RANGE1 root_descriptor_ranges[MAX_ROOT_PARAMETERS];
            CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC root_signature_desc;

            D3D12_ROOT_PARAMETER1 root_constant_parameter;
            uint32_t root_parameter_index;
            uint8_t bind_slots[RPK_RootParameterKeyCount];

            uint8_t has_uavs : 1;
            uint8_t has_srvs : 1;
            uint8_t has_cbvs: 1;
            uint8_t has_samplers : 1;
            uint8_t has_root_cbs : 1;

            friend class DeviceContext_D3D12_Impl;
            friend class RenderDevice_D3D12_Impl;
        };

    }
}