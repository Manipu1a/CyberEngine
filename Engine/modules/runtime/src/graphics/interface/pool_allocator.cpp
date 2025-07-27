#include "interface/pool_allocator.h"
#include "platform/memory.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(RenderObject)

void PoolAllocationData::reset()
{
    size = 0;
    alignment = 0;
    set_type(AllocationType::Unknown);
    pool_index = 0;
    offset = 0;
    locked = false;

    previous_allocation = nullptr;
    next_allocation = nullptr;
    alias_allocation = nullptr;
}

void PoolAllocationData::init_as_head(int16_t in_pool_index)
{
    reset();

    set_type(AllocationType::Head);
    next_allocation = this;
    previous_allocation = this;
    pool_index = in_pool_index;
}

void PoolAllocationData::init_as_free(uint16_t in_pool_index, uint32_t in_size, uint32_t in_alignment, uint32_t in_offset)
{
    reset();

    size = in_size;
    alignment = in_alignment;
    set_type(AllocationType::Free);
    offset = in_offset;
    pool_index = in_pool_index;
}

void PoolAllocationData::init_as_allocated(uint32_t in_size, uint32_t in_pool_alignment, uint32_t in_allocation_alignment, PoolAllocationData* in_free)
{
    cyber_check(in_free != nullptr);

    reset();

    size = in_size;
    alignment = in_allocation_alignment;
    set_type(AllocationType::Allocated);
    offset = MemoryPool::get_aligned_offset(in_free->offset, in_pool_alignment, in_allocation_alignment);
    pool_index = in_free->pool_index;
    locked = true;

    uint32_t alignment_size = MemoryPool::get_alignment_size(in_size, in_pool_alignment, in_allocation_alignment);
    in_free->size -= alignment_size;
    in_free->offset += alignment_size;
    in_free->add_before(this);
}

void PoolAllocationData::move_from(PoolAllocationData& in_allocated, bool is_locked)
{
    reset();

    size = in_allocated.size;
    alignment = in_allocated.alignment;
    type = in_allocated.type;
    offset = in_allocated.offset;
    pool_index = in_allocated.pool_index;
    locked = is_locked;

    if(in_allocated.previous_allocation)
    {
        in_allocated.previous_allocation->next_allocation = this;
        in_allocated.next_allocation->previous_allocation = this;
        previous_allocation = in_allocated.previous_allocation;
        next_allocation = in_allocated.next_allocation;
    }

    in_allocated.reset();
}

void PoolAllocationData::mark_free(uint32_t in_pool_alignment, uint32_t in_allocation_alignment)
{
    set_type(AllocationType::Free);

    size = MemoryPool::get_alignment_size(size, in_pool_alignment, in_allocation_alignment);
    offset = align_down(offset, in_allocation_alignment);
    alignment = in_allocation_alignment;
}

void PoolAllocationData::merge(PoolAllocationData* in_other)
{
    cyber_check(is_free() && in_other->is_free());
    cyber_check((offset + size) == in_other->offset);
    cyber_check(pool_index == in_other->pool_index);

    size += in_other->size;

    in_other->remove_from_linked_list();
    in_other->reset();
}

void PoolAllocationData::remove_from_linked_list()
{
    previous_allocation->next_allocation = next_allocation;
    next_allocation->previous_allocation = previous_allocation;
}

void PoolAllocationData::add_before(PoolAllocationData* in_other)
{
    previous_allocation->next_allocation = in_other;
    in_other->previous_allocation = previous_allocation;

    previous_allocation = in_other;
    in_other->next_allocation = this;
}

void PoolAllocationData::add_after(PoolAllocationData* in_other)
{
    next_allocation->previous_allocation = in_other;
    in_other->next_allocation = next_allocation;

    next_allocation = in_other;
    in_other->previous_allocation = this;
}

static int32_t desired_allocation_pool_size = 32;

uint32_t MemoryPool::get_alignment_size(uint32_t in_size_bytes, uint32_t in_pool_alignment, uint32_t in_allocation_alignment)
{
    uint32_t alignment_size = in_allocation_alignment;
    if(in_pool_alignment > in_allocation_alignment)
    {
        alignment_size = in_pool_alignment;
    }

    return align_up(in_size_bytes, alignment_size);
}

uint32_t MemoryPool::get_aligned_offset(uint32_t in_offset, uint32_t in_pool_alignment, uint32_t in_allocation_alignment)
{
    uint32_t alignment_size = in_allocation_alignment;
    if(in_pool_alignment > in_allocation_alignment)
    {
        alignment_size = in_pool_alignment;
    }

    return align_up(in_offset, alignment_size);
}

MemoryPool::MemoryPool(int16_t in_pool_index, uint64_t in_pool_size, uint32_t in_pool_alignment, PoolResourceType in_supported_resource_type, FreeListOrder in_free_list_order)
    : pool_index(in_pool_index),
      pool_size(in_pool_size),
      pool_alignment(in_pool_alignment),
      supported_resource_type(in_supported_resource_type),
      free_list_order(in_free_list_order),
      free_size(0),
      alignment_waste(0),
      allocated_blocks(0)
{

}

MemoryPool::~MemoryPool()
{
    destroy();
}

void MemoryPool::init()
{
    free_size = pool_size;

    allocated_pools.reserve(desired_allocation_pool_size);
    for(int32_t index = 0; index < desired_allocation_pool_size; ++index)
    {
        PoolAllocationData* allocation_data = cyber_new<PoolAllocationData>();
        allocation_data->reset();
        allocated_pools.push_back(allocation_data);
    }

    // Initialize the head block
    head_block.init_as_head(pool_index);
    PoolAllocationData* free_block = get_new_allocation_data();
    free_block->init_as_free(pool_index, pool_size, pool_alignment, 0);
    head_block.add_after(free_block);
    free_blocks.push_back(free_block);

    validate();
}

void MemoryPool::destroy()
{

}

bool MemoryPool::try_allocate(uint32_t in_size_in_bytes, uint32_t in_allocation_alignment, PoolResourceType in_resource_type, PoolAllocationData& out_allocation)
{
    cyber_check(is_resource_type_supported(in_resource_type));

    int32_t free_block_index = find_free_block(in_size_in_bytes, in_allocation_alignment);
    if(free_block_index != -1)
    {
        uint32_t aligned_size = get_alignment_size(in_size_in_bytes, pool_alignment, in_allocation_alignment);

        PoolAllocationData* free_block = free_blocks[free_block_index];
        cyber_check(free_block->get_size() >= aligned_size);

        // remove from the free blocks, resort it
        free_blocks.erase(free_blocks.begin() + free_block_index);

        // update
        out_allocation.init_as_allocated(in_size_in_bytes, pool_alignment, in_allocation_alignment, free_block);
        cyber_check((out_allocation.get_offset() % in_allocation_alignment) == 0);

        // update working state
        cyber_check(free_size >= aligned_size);
        free_size -= aligned_size;
        alignment_waste += (aligned_size - in_size_in_bytes);
        allocated_blocks++;

        // free block is empty then release otherwise sorted reinsert
        if(free_block->get_size() == 0)
        {
            free_block->remove_from_linked_list();
            release_allocation_data(free_block);

            validate();
        }
        else 
        {
            add_to_free_blocks(free_block);
        }
    }
    else
    {
        return false;
    }
}

void MemoryPool::deallocate(PoolAllocationData& in_allocation)
{
    cyber_check(in_allocation.is_locked());
    cyber_check(pool_index == in_allocation.get_pool_index());

    uint32_t allocation_alignment = in_allocation.get_alignment();

    // free block should not be locked anymore - can be reused immediatly when we get to actual pool deallocate
    bool bLocked = false;
    uint64_t allocation_size = in_allocation.get_size();

    PoolAllocationData* free_block = get_new_allocation_data();
    free_block->move_from(in_allocation, bLocked);
    free_block->mark_free(pool_alignment, allocation_alignment);

    // update working state
    free_size += free_block->get_size();
    alignment_waste -= (free_block->get_size() - allocation_size);
    allocated_blocks--;

    add_to_free_blocks(free_block);
}

int32_t MemoryPool::find_free_block(uint32_t in_size_bytes, uint32_t in_allocation_alignment)
{
    uint32_t aligned_size = get_alignment_size(in_size_bytes, pool_alignment, in_allocation_alignment);

    if(free_size < aligned_size)
    {
        return -1;
    }

    int32_t find_index = -1;
    for(int32_t index = 0; index < free_blocks.size(); ++index)
    {
        PoolAllocationData* free_block = free_blocks[index];
        if(free_block->get_size() >= aligned_size)
        {
            find_index = index;
            break;
        }
    }

    return find_index;
}

PoolAllocationData* MemoryPool::add_to_free_blocks(PoolAllocationData* in_free_block)
{
    cyber_check(in_free_block->is_free());
    cyber_check(is_aligned(in_free_block->get_offset(), pool_alignment));
    cyber_check(is_aligned(in_free_block->get_size(), pool_alignment));

    PoolAllocationData* free_block = in_free_block;
    // 与前一个空闲块合并
    if(free_block->get_previous()->is_free() && !free_block->get_previous()->is_locked())
    {
        PoolAllocationData* previous_free_block = free_block->get_previous();
        previous_free_block->merge(free_block);
        remove_from_free_blocks(previous_free_block);
        release_allocation_data(free_block);

        free_block = previous_free_block;
    }

    // 与后一个空闲块合并
    if(free_block->get_next()->is_free() && !free_block->get_next()->is_locked())
    {
        PoolAllocationData* next_free_block = free_block->get_next();
        free_block->merge(next_free_block);
        remove_from_free_blocks(next_free_block);
        release_allocation_data(next_free_block);
    }

    // 插入到空闲块列表中
    int32_t insert_index = 0;
    if(free_list_order == FreeListOrder::SortBySize)
    {
        for(; insert_index < free_blocks.size(); ++insert_index)
        {
            if(free_blocks[insert_index]->get_size() > free_block->get_size())
            {
                break;
            }
        }
    }
    else if(free_list_order == FreeListOrder::SortByOffset)
    {
        for(; insert_index < free_blocks.size(); ++insert_index)
        {
            if(free_blocks[insert_index]->get_offset() > free_block->get_offset())
            {
                break;
            }
        }
    }

    free_blocks.insert(free_blocks.begin() + insert_index, free_block);

    validate();

    return free_block;
}

void MemoryPool::remove_from_free_blocks(PoolAllocationData* in_free_block)
{
    for(auto it = free_blocks.begin(); it != free_blocks.end(); ++it)
    {
        if(*it == in_free_block)
        {
            free_blocks.erase(it);
            return;
        }
    }
}

PoolAllocationData* MemoryPool::get_new_allocation_data()
{
    if(allocated_pools.size() > 0)
    {
        PoolAllocationData* data = allocated_pools.back();
        allocated_pools.pop_back();
        return data;
    }
    return cyber_new<PoolAllocationData>();
}

void MemoryPool::release_allocation_data(PoolAllocationData* in_allocation_data)
{
    in_allocation_data->reset();
    if(allocated_pools.size() >= desired_allocation_pool_size)
    {
        delete in_allocation_data;
    }
    else  
    {
        allocated_pools.push_back(in_allocation_data);
    }
}

void MemoryPool::validate()
{

}

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE