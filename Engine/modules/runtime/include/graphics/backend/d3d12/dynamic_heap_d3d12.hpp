#pragma once
#include "d3d12.config.h"
#include "core/debug.h"
#include "eastl/map.h"
#include "render_device_d3d12.h"
#include "platform/configure.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(RenderObject)

struct Dynamic_Allocation_D3D12
{
    Dynamic_Allocation_D3D12() noexcept {}
    Dynamic_Allocation_D3D12(ID3D12Resource* buffer, uint64_t offset, uint64_t size, void* cpu_address, D3D12_GPU_VIRTUAL_ADDRESS gpu_address) noexcept
        : buffer(buffer), offset(offset), size(size), cpu_address(cpu_address), gpu_address(gpu_address) 
        {

        }
    
    ID3D12Resource* buffer;
    uint64_t offset;
    uint64_t size;
    void* cpu_address;
    D3D12_GPU_VIRTUAL_ADDRESS gpu_address;
};

class Dynamic_Page_D3D12
{
public:
    Dynamic_Page_D3D12(ID3D12Device* d3d12_device, uint64_t size);

    void* get_cpu_address(uint64_t offset)
    {
        cyber_check(d3d12_buffer);
        cyber_assert(offset < get_size(), "Offset ({0}) exceeds buffer size ({1})", offset, get_size());
        return reinterpret_cast<uint8_t*>(cpu_virtual_address) + offset;
    }

    D3D12_GPU_VIRTUAL_ADDRESS get_gpu_address(uint64_t offset)
    {
        cyber_check(d3d12_buffer);
        cyber_assert(offset < get_size(), "Offset ({0}) exceeds buffer size ({1})", offset, get_size());
        return gpu_virtual_address + offset;
    }

    ID3D12Resource* get_d3d12_buffer()
    {
        return d3d12_buffer;
    }
    uint64_t get_size() const
    {
        cyber_check(d3d12_buffer);
        return d3d12_buffer->GetDesc().Width;
    }

    bool is_valid() const { return d3d12_buffer != nullptr; }
private:
    ID3D12Resource* d3d12_buffer;
    void* cpu_virtual_address = nullptr; // The CPU-wri
    D3D12_GPU_VIRTUAL_ADDRESS gpu_virtual_address = 0;
};

class Dynamic_Memory_Manager_D3D12
{
public:
    Dynamic_Memory_Manager_D3D12(RenderDevice_D3D12_Impl& render_device_impl, uint32_t num_pages_to_reserve, uint64_t page_size);
    ~Dynamic_Memory_Manager_D3D12();

    void release_pages(eastl::vector<Dynamic_Page_D3D12>& pages, uint64_t queue_mask);
    void destroy();

    Dynamic_Page_D3D12 allocate_page(uint64_t size_in_bytes);
private:
    RenderDevice_D3D12_Impl& render_device;

    std::mutex available_pages_mutex;
    eastl::multimap<uint64_t, Dynamic_Page_D3D12, eastl::less<uint64_t>> available_pages;
};

class Dynamic_Heap_D3D12
{
public:
    Dynamic_Heap_D3D12(Dynamic_Memory_Manager_D3D12& dynamic_mem_mgr, eastl::string heap_name, uint64_t page_size) : 
        global_dynamic_mem_mgr(dynamic_mem_mgr),
        heap_name(eastl::move(heap_name)),
        page_size(page_size)
    {

    }
    
    ~Dynamic_Heap_D3D12();

    Dynamic_Allocation_D3D12 allocate(uint64_t size_in_bytes, uint64_t alignment );
    void release_allocated_pages(uint64_t queue_mask);

    static constexpr uint64_t invalid_offset = static_cast<uint64_t>(-1);
    
private:
    Dynamic_Memory_Manager_D3D12& global_dynamic_mem_mgr;
    const eastl::string heap_name;
    eastl::vector<Dynamic_Page_D3D12> allocated_pages;
    const uint64_t page_size;

    uint64_t current_offset = 0;
    uint64_t available_size = 0;

    uint64_t current_allocated_size = 0;
    uint64_t current_used_size = 0;
    uint64_t current_aligned_size = 0;
    uint64_t peak_allocated_size = 0;
    uint64_t peak_used_size = 0;
    uint64_t peak_aligned_size = 0;
};

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE
