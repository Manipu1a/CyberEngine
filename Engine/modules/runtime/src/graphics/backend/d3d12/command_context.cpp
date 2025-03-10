#include "backend/d3d12/command_context.h"
#include "core/debug.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(RenderObject)

CommandContext::CommandContext(CommandListManager& command_list_manager)
{
    command_list_manager.create_new_command_list(&command_list, &command_allocator, max_interface_version);
}

CommandContext::~CommandContext()
{
    
}

ID3D12GraphicsCommandList* CommandContext::close(ID3D12CommandAllocator* out_command_allocator)
{

    cyber_assert(command_allocator != nullptr, "Command allocator is not initialized");
    auto hr = command_list->Close();
    cyber_assert(SUCCEEDED(hr), "Failed to close command list");

    out_command_allocator = std::move(command_allocator);
    return command_list;
}

void CommandContext::reset(CommandListManager& command_list_manager)
{
    cyber_check(command_list != nullptr);
    cyber_check(command_list->GetType() == command_list_manager.get_command_list_type());

    if(command_allocator != nullptr)
    {
        command_list_manager.request_allocator(&command_allocator);
        command_list->Reset(command_allocator, nullptr);
    }
}


CYBER_END_NAMESPACE
CYBER_END_NAMESPACE













