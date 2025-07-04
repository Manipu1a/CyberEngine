#pragma once

#include "common/cyber_graphics_config.h"
#include "interface/graphics_types.h"
#include "backend/d3d12/texture_d3d12.h"
#include "graphics_types_d3d12.h"
#include "interface/shader_reflection.hpp"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(RenderObject)


struct ShaderResourceViewCache
{
    void clear()
    {
        memset(bind_slot_mask, 0, sizeof(bind_slot_mask));

        for (int i = 0; i < SHADER_STAGE_COUNT; ++i)
        {
            for (int j = 0; j < MAX_SRVS; ++j)
            {
                resources[i][j] = nullptr;
                views[i][j] = nullptr;
            }
        }
    }
    Texture_D3D12_Impl* resources[SHADER_STAGE_COUNT][MAX_SRVS];
    Texture_View_D3D12_Impl* views[SHADER_STAGE_COUNT][MAX_SRVS];

    uint64_t bind_slot_mask[SHADER_STAGE_COUNT];
};

struct ConstantBufferViewCache
{
    void clear()
    {
        memset(bind_slot_mask, 0, sizeof(bind_slot_mask));

        for (int i = 0; i < SHADER_STAGE_COUNT; ++i)
        {
            for (int j = 0; j < MAX_CBVS; ++j)
            {
                resources[i][j] = nullptr;
                views[i][j] = nullptr;
            }
        }
    }

    Buffer_D3D12_Impl* resources[SHADER_STAGE_COUNT][MAX_CBVS];
    Buffer_View_D3D12_Impl* views[SHADER_STAGE_COUNT][MAX_CBVS];

    uint64_t bind_slot_mask[SHADER_STAGE_COUNT];
};

struct UnorderedAccessViewCache
{
    void clear()
    {
        memset(bind_slot_mask, 0, sizeof(bind_slot_mask));

        for (int i = 0; i < SHADER_STAGE_COUNT; ++i)
        {
            for (int j = 0; j < MAX_UAVS; ++j)
            {
                resources[i][j] = nullptr;
                views[i][j] = nullptr;
            }
        }
    }

    Buffer_D3D12_Impl* resources[SHADER_STAGE_COUNT][MAX_UAVS];
    Buffer_View_D3D12_Impl* views[SHADER_STAGE_COUNT][MAX_UAVS];
    uint64_t bind_slot_mask[SHADER_STAGE_COUNT];
};

struct ConstantBufferCache
{
    void clear()
    {
        memset(bind_slot_mask, 0, sizeof(bind_slot_mask));

        for (int i = 0; i < SHADER_STAGE_COUNT; ++i)
        {
            for (int j = 0; j < MAX_CBS; ++j)
            {
                resources[i][j] = nullptr;
            }
        }
    }
    Buffer_D3D12_Impl* resources[SHADER_STAGE_COUNT][MAX_CBS];
    D3D12_GPU_VIRTUAL_ADDRESS current_gpu_virtual_address[SHADER_STAGE_COUNT][MAX_CBS];

    uint16_t bind_slot_mask[SHADER_STAGE_COUNT];
};

class StateCache_D3D12
{
public:
    StateCache_D3D12()
    {
        bound_heaps[0] = nullptr;
        bound_heaps[1] = nullptr;

        bound_heap_start_handles[0].ptr = 0;
        bound_heap_start_handles[1].ptr = 0;

        bound_root_signature = nullptr;

        shader_resource_view_cache.clear();
        constant_buffer_cache.clear();
        unordered_access_view_cache.clear();
    }

    void set_new_shader_data(SHADER_STAGE stage, const ShaderRegisterCount& register_counts)
    {
        current_shader_srv_count[stage] = register_counts.shader_resource_count;
        current_shader_cbv_count[stage] = register_counts.constant_buffer_count;
        current_shader_uav_count[stage] = register_counts.unordered_access_count;
    }

    void clear_all_cache();

    // Cached in beginCmd to avoid fetching them during rendering
    struct DescriptorHeap_D3D12* bound_heaps[2];
    D3D12_GPU_DESCRIPTOR_HANDLE bound_heap_start_handles[2];
    // Command buffer state
    const ID3D12RootSignature* bound_root_signature;
    // shader parameters
    ShaderResourceViewCache shader_resource_view_cache;
    ConstantBufferViewCache constant_buffer_view_cache;
    UnorderedAccessViewCache unordered_access_view_cache;

    ConstantBufferCache constant_buffer_cache;

    uint32_t current_shader_srv_count[SHADER_STAGE_COUNT] = {0};
    uint32_t current_shader_cbv_count[SHADER_STAGE_COUNT] = {0};
    uint32_t current_shader_uav_count[SHADER_STAGE_COUNT] = {0};
    uint32_t current_shader_sampler_count[SHADER_STAGE_COUNT] = {0};
};

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE