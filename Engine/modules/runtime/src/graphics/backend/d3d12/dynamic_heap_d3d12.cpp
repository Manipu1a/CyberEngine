#include "backend/d3d12/dynamic_heap_d3d12.hpp"
#include "core/common.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(RenderObject)

Dynamic_Page_D3D12::Dynamic_Page_D3D12(ID3D12Device* d3d12_device, uint64_t size)
{
    D3D12_HEAP_PROPERTIES heap_properties = {};
    heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;
    heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heap_properties.CreationNodeMask = 1;
    heap_properties.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC resource_desc = {};
    resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resource_desc.Alignment = 0;
    resource_desc.Width = size;
    resource_desc.Height = 1;
    resource_desc.DepthOrArraySize = 1;
    resource_desc.MipLevels = 1;
    resource_desc.Format = DXGI_FORMAT_UNKNOWN;
    resource_desc.SampleDesc.Count = 1;
    resource_desc.SampleDesc.Quality = 0;
    resource_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resource_desc.Flags = D3D12_RESOURCE_FLAG_NONE;
    D3D12_RESOURCE_STATES default_usage = D3D12_RESOURCE_STATE_GENERIC_READ;

    auto hr = d3d12_device->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc, 
                                default_usage, nullptr, IID_PPV_ARGS(&d3d12_buffer));

    if (FAILED(hr))
    {
        cyber_error("Failed to create dynamic page");
        return;
    }

    d3d12_buffer->SetName(L"Dynamic Page");

    gpu_virtual_address = d3d12_buffer->GetGPUVirtualAddress();

    d3d12_buffer->Map(0, nullptr, &cpu_virtual_address);

    cyber_log("Created dynamic memory page. Size: {0} bytes, GPU Address: {1}, CPU Address: {2}", size, gpu_virtual_address, cpu_virtual_address);
}

Dynamic_Memory_Manager_D3D12::Dynamic_Memory_Manager_D3D12(RenderDevice_D3D12_Impl& render_device_impl, uint32_t num_pages_to_reserve, uint64_t page_size)
    : render_device(render_device_impl)
{
    for(uint32_t i = 0; i < num_pages_to_reserve; i++)
    {
        Dynamic_Page_D3D12 page(render_device.GetD3D12Device(), page_size);
        auto size = page.get_size();
        available_pages.emplace(size, eastl::move(page));
    }
}

Dynamic_Page_D3D12 Dynamic_Memory_Manager_D3D12::allocate_page(uint64_t size_in_bytes)
{
    std::lock_guard<std::mutex> available_pages_lock(available_pages_mutex);

    auto page_iter = available_pages.lower_bound(size_in_bytes);
    if(page_iter != available_pages.end())
    {
        cyber_check(page_iter->first >= size_in_bytes);
        Dynamic_Page_D3D12 page = eastl::move(page_iter->second);
        available_pages.erase(page_iter);
        return page;
    }
    else {
        return Dynamic_Page_D3D12(render_device.GetD3D12Device(), size_in_bytes);
    }
}

void Dynamic_Memory_Manager_D3D12::release_pages(eastl::vector<Dynamic_Page_D3D12>& pages, uint64_t queue_mask)
{

    struct Stable_page
    {
        Dynamic_Page_D3D12 page;
        Dynamic_Memory_Manager_D3D12* manager;

        Stable_page(Dynamic_Page_D3D12&& page, Dynamic_Memory_Manager_D3D12* manager)
            : page(eastl::move(page)), manager(manager)
        {

        }

        Stable_page(const Stable_page&) = delete;
        Stable_page& operator=(const Stable_page&) = delete;
        Stable_page& operator=(Stable_page&& other) = delete;

        Stable_page(Stable_page&& other) noexcept
            : page(eastl::move(other.page)), manager(other.manager)
        {
            other.manager = nullptr;
        }

        ~Stable_page()
        {
            if(manager)
            {
                std::lock_guard<std::mutex> available_pages_lock(manager->available_pages_mutex);
                auto page_size = page.get_size();
                manager->available_pages.emplace(page_size, eastl::move(page));
            }
        }
    };

    for(auto& page : pages)
    {
        auto page_size = page.get_size();
        available_pages.emplace(page_size, eastl::move(page));
    }
}

void Dynamic_Memory_Manager_D3D12::destroy()
{
    uint64_t total_allocated_size = 0;
    for(const auto& page : available_pages)
    {
        total_allocated_size += page.second.get_size();
    }

    cyber_log("Total allocated size: {0} bytes", total_allocated_size);

    available_pages.clear();
}

Dynamic_Memory_Manager_D3D12::~Dynamic_Memory_Manager_D3D12()
{
    cyber_assert(available_pages.empty(), "Dynamic memory manager destroyed with allocated pages remaining");
}

Dynamic_Heap_D3D12::~Dynamic_Heap_D3D12()
{
    cyber_assert(allocated_pages.empty(), "Dynamic heap destroyed with allocated pages remaining");

    auto peak_allocated_pages = peak_allocated_size / page_size;
}

Dynamic_Allocation_D3D12 Dynamic_Heap_D3D12::allocate(uint64_t size_in_bytes, uint64_t alignment)
{
    cyber_check(alignment > 0);
    cyber_assert(is_power_of_two(alignment), "Alignment must be a power of two");
    
    if(current_offset == invalid_offset || size_in_bytes + (align_up(current_offset, alignment) - current_offset) > available_size)
    {
        auto new_page_size = page_size;
        while(new_page_size < size_in_bytes)
            new_page_size *= 2;
        
        auto new_page = global_dynamic_mem_mgr.allocate_page(new_page_size);
        if(new_page.is_valid())
        {
            current_offset = 0;
            available_size = new_page.get_size();

            current_allocated_size += available_size;
            peak_allocated_size = eastl::max(peak_allocated_size, current_allocated_size);

            allocated_pages.emplace_back(eastl::move(new_page));
        }
    }

    if(current_offset != invalid_offset && size_in_bytes +(align_up(current_offset, alignment) - current_offset) <= available_size)
    {
        auto aligned_offset = align_up(current_offset, alignment);
        auto adjustd_size = size_in_bytes + (aligned_offset - current_offset);
        cyber_check(adjustd_size <= available_size);

        available_size -= adjustd_size;
        current_offset += adjustd_size;

        current_used_size += size_in_bytes;
        peak_used_size = eastl::max(peak_used_size, current_used_size);
        
        current_aligned_size += adjustd_size;
        peak_aligned_size = eastl::max(peak_aligned_size, current_aligned_size);

        auto& curr_page = allocated_pages.back();
        return Dynamic_Allocation_D3D12(curr_page.get_d3d12_buffer(), 
        aligned_offset, 
        size_in_bytes, 
        curr_page.get_cpu_address(aligned_offset),
        curr_page.get_gpu_address(aligned_offset));
    }
    else {
        return Dynamic_Allocation_D3D12();
    }
}

void Dynamic_Heap_D3D12::release_allocated_pages(uint64_t queue_mask)
{
    global_dynamic_mem_mgr.release_pages(allocated_pages, queue_mask);
    allocated_pages.clear();

    current_offset = invalid_offset;
    available_size = 0;
    current_allocated_size = 0;
    current_used_size = 0;
    current_aligned_size = 0;
}

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE
