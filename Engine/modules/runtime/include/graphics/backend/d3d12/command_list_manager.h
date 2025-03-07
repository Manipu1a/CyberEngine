#pragma once
#include "platform/configure.h"
#include <mutex>
#include <d3d12.h>
CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(RenderObject)

class CommandListManager
{
public:
    CommandListManager(class RenderDevice_D3D12_Impl* device, D3D12_COMMAND_LIST_TYPE list_type);
    ~CommandListManager();

    CommandListManager(const CommandListManager&) = delete;
    CommandListManager(CommandListManager&&) = delete;
    CommandListManager& operator=(const CommandListManager&) = delete;
    CommandListManager& operator=(CommandListManager&&) = delete;

    void create_new_command_list(ID3D12GraphicsCommandList** command_list, ID3D12CommandAllocator** command_allocator, uint32_t& version);
    void request_allocator(ID3D12CommandAllocator** command_allocator);


    D3D12_COMMAND_LIST_TYPE get_command_list_type() const { return command_list_type; }

private:
    std::mutex allocator_mutex;
    std::atomic<uint32_t> num_allocators;
    
    const D3D12_COMMAND_LIST_TYPE command_list_type;
    class RenderDevice_D3D12_Impl* device;
};

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE