#pragma once
#include "common/flags.h"
#include "common/cyber_graphics_config.h"
#include "device_object.h"
#include "interface/graphics_types.h"

namespace Cyber
{
    namespace RenderObject
    {

        /// Buffer Group

        struct CYBER_GRAPHICS_API BufferData
        {
            const void* data;

            uint64_t data_size;

            class IRenderDevice* pDevice;

            class ICommandBuffer* pCommandBuffer;

            constexpr BufferData() noexcept {}

            constexpr BufferData(const void* _data, uint64_t _data_size, IRenderDevice* _pDevice, ICommandBuffer* _pCommandBuffer) noexcept
                : data(_data), data_size(_data_size), pDevice(_pDevice), pCommandBuffer(_pCommandBuffer) {}
        };


        struct CYBER_GRAPHICS_API BufferCreateDesc
        {
            /// Size of the buffer (in bytes)
            uint64_t size;
            /// Set this to specify a counter buffer for this buffer (applicable to BUFFER_USAGE_STORAGE_SRV, BUFFER_USAGE_STORAGE_UAV)
            class IBuffer* pCounterBuffer;
            /// Index of the first element accessible by the SRV/UAV (applicable to BUFFER_USAGE_STORAGE_SRV, BUFFER_USAGE_STORAGE_UAV)
            uint64_t firstElement;
            /// Number of elements in the buffer (applicable to BUFFER_USAGE_STORAGE_SRV, BUFFER_USAGE_STORAGE_UAV)
            uint64_t elementCount;
            /// Size of each element (in bytes) in the buffer (applicable to BUFFER_USAGE_STORAGE_SRV, BUFFER_USAGE_STORAGE_UAV)
            uint64_t structStride;
            /// Debug name used in gpu profile
            const char8_t* Name;
            uint32_t* pSharedNodeIndices;
            /// Alignment
            uint32_t alignment;
            /// Decides which memory heap buffer will use (default, upload, readback)
            GRAPHICS_RESOURCE_USAGE usage;
            /// Creation flags of the buffer
            GRAPHICS_RESOURCE_BIND_FLAGS bind_flags;
            /// CPU access flags
            CPU_ACCESS_FLAGS cpu_access_flags;
            /// What type of queue the buffer is owned by
            COMMAND_QUEUE_TYPE queueType;
            /// What state will the buffer get created in
            GRAPHICS_RESOURCE_STATE startState;
            /// ICB draw type
            INDIRECT_ARGUMENT_TYPE icbDrawType;
            /// ICB max vertex buffers slots count
            uint32_t icbMaxCommandCount;
            /// Image format
            TEXTURE_FORMAT format;
            /// Descriptor creation
            DESCRIPTOR_TYPE descriptors;
        };

        
        // Buffer Interface
        struct CYBER_GRAPHICS_API IBuffer : public IDeviceObject
        {
            virtual void* get_mapped_data() = 0;
            virtual uint64_t get_size() const = 0;
            virtual uint64_t get_descriptors() const = 0;
            virtual uint64_t get_memory_usage() const = 0;
            virtual uint64_t get_node_index() const = 0;
            virtual BufferCreateDesc get_create_desc() = 0;
            virtual void set_buffer_state(GRAPHICS_RESOURCE_STATE state) = 0;
            virtual GRAPHICS_RESOURCE_STATE get_buffer_state() const = 0;
        };

        template<typename EngineImplTraits>
        class CYBER_GRAPHICS_API BufferBase : public DeviceObjectBase<typename EngineImplTraits::BufferInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using BufferInterface = typename EngineImplTraits::BufferInterface;
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
            using TDeviceObjectBase = DeviceObjectBase<BufferInterface, RenderDeviceImplType>;

            BufferBase(RenderDeviceImplType* device, BufferCreateDesc desc) : TDeviceObjectBase(device) 
            {
                m_pCpuMappedAddress = nullptr;
                m_size = 0;
                m_descriptors = 0;
                m_memoryUsage = 0;
                m_nodeIndex = 0;
                m_desc = desc;
            }

            virtual ~BufferBase() = default;

            virtual void* get_mapped_data() override final
            {
                return m_pCpuMappedAddress;
            }

            virtual uint64_t get_size() const override final
            {
                return m_size;
            }

            virtual uint64_t get_descriptors() const override final
            {
                return m_descriptors;
            }

            virtual uint64_t get_memory_usage() const override final
            {
                return m_memoryUsage;
            }

            virtual uint64_t get_node_index() const override final
            {
                return m_nodeIndex;
            }

            virtual BufferCreateDesc get_create_desc() override final
            {
                return m_desc;
            }

            virtual void set_buffer_state(GRAPHICS_RESOURCE_STATE state) override final
            {
                buffer_state = state;
            }

            virtual GRAPHICS_RESOURCE_STATE get_buffer_state() const override final
            {
                return buffer_state;
            }

            bool is_known_state() const
            {
                return buffer_state != GRAPHICS_RESOURCE_STATE_UNKNOWN;
            }
        protected:
            /// CPU address of the mapped buffer (applicable to buffers created in CPU accessible heaps (CPU, CPU_TO_GPU, GPU_TO_CPU)
            void* m_pCpuMappedAddress;
            uint64_t m_size : 32;
            uint64_t m_descriptors : 20;
            uint64_t m_memoryUsage : 3;
            uint64_t m_nodeIndex : 4;
            BufferCreateDesc m_desc;
            GRAPHICS_RESOURCE_STATE buffer_state;

            friend RenderDeviceImplType;
        };
    }
}