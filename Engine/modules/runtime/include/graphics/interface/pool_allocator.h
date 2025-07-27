#pragma once
#include "core/common.h"
#include "common/cyber_graphics_config.h"
#include "EASTL/vector.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(RenderObject)

enum class PoolResourceType : uint8_t
{
    Buffers = 0x1,
    RTDSTextures = 0x2,
    NonRTDSTextures = 0x4,
    Count = Buffers | RTDSTextures | NonRTDSTextures,
};

struct PoolAllocationData
{
public:
    void reset();

    void init_as_head(int16_t in_pool_index);
    void init_as_free(uint16_t in_pool_index, uint32_t in_size, uint32_t in_alignment, uint32_t in_offset);
    void init_as_allocated(uint32_t in_size, uint32_t in_pool_alignment, uint32_t in_allocation_alignment, PoolAllocationData* in_free);
    void move_from(PoolAllocationData& in_allocated, bool is_locked = false);

    void mark_free(uint32_t in_pool_alignment, uint32_t in_allocation_alignment);

    bool is_head() const { return get_type() == AllocationType::Head; }
    bool is_free() const { return get_type() == AllocationType::Free; }
    bool is_allocated() const { return get_type() == AllocationType::Allocated; }

    CYBER_FORCE_INLINE uint32_t get_size() const { return size; }
    CYBER_FORCE_INLINE uint32_t get_alignment() const { return alignment; }
    CYBER_FORCE_INLINE uint32_t get_offset() const { return offset; }
    CYBER_FORCE_INLINE uint32_t get_pool_index() const { return pool_index; }

    PoolAllocationData* get_next() const { return next_allocation; }
    PoolAllocationData* get_previous() const { return previous_allocation; }

    bool is_locked() const { return locked == 1; }
    void unlock() { locked = 0; }

protected:
    friend class MemoryPool;

    void merge(PoolAllocationData* in_other);

    void remove_from_linked_list();
    void add_before(PoolAllocationData* in_other);
    void add_after(PoolAllocationData* in_other);

    enum class AllocationType : uint8_t
    {
        Unknown = 0,
        Free = 1,
        Allocated = 2,
        Head = 3
    };

    CYBER_FORCE_INLINE AllocationType get_type() const { return (AllocationType) type; }
    CYBER_FORCE_INLINE void set_type(AllocationType in_type) { type = (uint32_t)in_type; }

    uint32_t size;
    uint32_t alignment;
    uint32_t offset;
    uint32_t pool_index : 16;
    uint32_t type;

    uint32_t locked : 1;
    uint32_t unused : 11;

    PoolAllocationData* previous_allocation;
    PoolAllocationData* next_allocation;
    PoolAllocationData* alias_allocation;
};

class MemoryPool
{
public:
    enum class FreeListOrder : uint8_t
    {
        SortBySize,
        SortByOffset
    };

    MemoryPool(int16_t in_pool_index, uint64_t in_pool_size, uint32_t in_pool_alignment, PoolResourceType in_supported_resource_type, FreeListOrder in_free_list_order);
    virtual ~MemoryPool();

    void init();
    void destroy();

    bool try_allocate(uint32_t in_size_in_bytes, uint32_t in_allocation_alignment, PoolResourceType in_resource_type, PoolAllocationData& out_allocation);
    void deallocate(PoolAllocationData& in_allocation);

    //void try_clear(IDeviceContext* device_context);

    int16_t get_pool_index() const { return pool_index; }
    uint64_t get_pool_size() const { return pool_size; }
    uint64_t get_free_size() const { return free_size; }
    uint64_t get_used_size() const { return pool_size - free_size; }
    uint32_t get_alignment_waste() const { return alignment_waste; }
    uint32_t get_allocated_blocks() const { return allocated_blocks; }
    PoolResourceType get_supported_resource_type() const { return supported_resource_type; }
    bool is_resource_type_supported(PoolResourceType in_resource_type) const
    {
        return any_flag_set(supported_resource_type, in_resource_type);
    }

    bool is_empty() const { return get_used_size() == 0; }
    bool is_full() const { return get_free_size() == 0; }

    static uint32_t get_alignment_size(uint32_t in_size_bytes, uint32_t in_pool_alignment, uint32_t in_allocation_alignment);
    static uint32_t get_aligned_offset(uint32_t in_offset, uint32_t in_pool_alignment, uint32_t in_allocation_alignment);
    
protected:
    int32_t find_free_block(uint32_t in_size_bytes, uint32_t in_allocation_alignment);
    PoolAllocationData* add_to_free_blocks(PoolAllocationData* in_free_block);
    void remove_from_free_blocks(PoolAllocationData* in_free_block);

    PoolAllocationData* get_new_allocation_data();
    void release_allocation_data(PoolAllocationData* in_allocation_data);

    void validate();

    int16_t pool_index;
    const uint64_t pool_size;
    const uint32_t pool_alignment;
    const PoolResourceType supported_resource_type;
    const FreeListOrder free_list_order;

    uint64_t free_size;
    uint64_t alignment_waste;
    uint32_t allocated_blocks;

    PoolAllocationData head_block;

    eastl::vector<PoolAllocationData*> free_blocks;
    eastl::vector<PoolAllocationData*> allocated_pools;
};

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE

