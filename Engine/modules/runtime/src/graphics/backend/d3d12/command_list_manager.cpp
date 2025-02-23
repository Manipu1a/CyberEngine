#include "backend/d3d12/command_list_manager.h"
#include "core/debug.h"
#include "backend/d3d12/render_device_d3d12.h"
CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(RenderObject)

CommandListManager::CommandListManager(class RenderDevice_D3D12_Impl* device, D3D12_COMMAND_LIST_TYPE list_type)
    : device(device), list_type(list_type)
{

}

CommandListManager::~CommandListManager()
{

}

void CommandListManager::create_new_command_list(ID3D12GraphicsCommandList** command_list, ID3D12CommandAllocator** command_allocator, uint32_t& version)
{
    request_allocator(command_allocator);

    auto* d3d12_device = device->GetD3D12Device();

    const IID cmd_list_iids[] = {
#ifdef D3D12_H_HAS_MESH_SHADER
    __uuidof(ID3D12GraphicsCommandList6),
    __uuidof(ID3D12GraphicsCommandList5),
#endif
    __uuidof(ID3D12GraphicsCommandList4),
    __uuidof(ID3D12GraphicsCommandList3),
    __uuidof(ID3D12GraphicsCommandList2),
    __uuidof(ID3D12GraphicsCommandList1),
    __uuidof(ID3D12GraphicsCommandList),
    };

    HRESULT hr = E_FAIL;
    for(uint32_t i = 0; i < _countof(cmd_list_iids); ++i)
    {
        hr = d3d12_device->CreateCommandList(0, list_type, *command_allocator, nullptr, cmd_list_iids[i], command_list);
        if(SUCCEEDED(hr))
        {
            version = _countof(cmd_list_iids) - 1 - i;
            break;
        }
    }

    cyber_assert(SUCCEEDED(hr), "Failed to create command list");
    (*command_list)->SetName(L"Command list");
}

void CommandListManager::request_allocator(ID3D12CommandAllocator** command_allocator)
{
    std::lock_guard<std::mutex> lock(allocator_mutex);

    cyber_warn((*command_allocator) == nullptr, "Allocator is not nullptr");
    *command_allocator = nullptr;

    auto* d3d12_device = device->GetD3D12Device();
    auto hr = d3d12_device->CreateCommandAllocator(list_type, IID_PPV_ARGS(command_allocator));
    cyber_assert(hr == S_OK, "Failed to create command allocator");

    eastl::wstring allocator_name(eastl::wstring::CtorSprintf(), L"Cmd list allocator %d", num_allocators.fetch_add(1));
    (*command_allocator)->SetName(allocator_name.c_str());
}

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE

