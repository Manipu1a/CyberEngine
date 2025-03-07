#pragma once
#include "platform/configure.h"
#include "command_list_manager.h"
CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(RenderObject)

class CommandContext
{
public:
    explicit CommandContext(CommandListManager& command_list_manager);

    CommandContext(const CommandContext&) = delete;
    CommandContext& operator=(const CommandContext&) = delete;
    CommandContext(CommandContext&&) = delete;
    CommandContext& operator=(CommandContext&&) = delete;

    ~CommandContext();
    
    ID3D12GraphicsCommandList* close(ID3D12CommandAllocator* out_command_allocator);
    void reset(CommandListManager& command_list_manager);
    
    
private:
    ID3D12GraphicsCommandList* command_list;
    ID3D12CommandAllocator* command_allocator;

    uint32_t max_interface_version;
};

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE
