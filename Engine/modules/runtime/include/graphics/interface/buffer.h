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
        struct CYBER_GRAPHICS_API BufferCreateDesc
        {
            /// Size of the buffer (in bytes)
            uint64_t m_size;
            /// Set this to specify a counter buffer for this buffer (applicable to BUFFER_USAGE_STORAGE_SRV, BUFFER_USAGE_STORAGE_UAV)
            class IBuffer* m_pCounterBuffer;
            /// Index of the first element accessible by the SRV/UAV (applicable to BUFFER_USAGE_STORAGE_SRV, BUFFER_USAGE_STORAGE_UAV)
            uint64_t m_firstElement;
            /// Number of elements in the buffer (applicable to BUFFER_USAGE_STORAGE_SRV, BUFFER_USAGE_STORAGE_UAV)
            uint64_t m_elementCount;
            /// Size of each element (in bytes) in the buffer (applicable to BUFFER_USAGE_STORAGE_SRV, BUFFER_USAGE_STORAGE_UAV)
            uint64_t m_structStride;
            /// Debug name used in gpu profile
            const char8_t* m_pName;
            uint32_t* m_pSharedNodeIndices;
            /// Alignment
            uint32_t m_alignment;
            /// Decides which memory heap buffer will use (default, upload, readback)
            GRAPHICS_RESOURCE_MEMORY_USAGE m_memoryUsage;
            /// Creation flags of the buffer
            BUFFER_CREATION_FLAG m_flags;
            /// What type of queue the buffer is owned by
            QUEUE_TYPE m_queueType;
            /// What state will the buffer get created in
            GRAPHICS_RESOURCE_STATE m_startState;
            /// ICB draw type
            INDIRECT_ARGUMENT_TYPE m_icbDrawType;
            /// ICB max vertex buffers slots count
            uint32_t m_icbMaxCommandCount;
            /// Image format
            TEXTURE_FORMAT m_format;
            /// Descriptor creation
            DESCRIPTOR_TYPE m_descriptors;
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
        };

        template<typename EngineImplTraits>
        class CYBER_GRAPHICS_API Buffer : public DeviceObjectBase<typename EngineImplTraits::BufferInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using BufferInterface = typename EngineImplTraits::BufferInterface;
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
            using TBufferBase = typename DeviceObjectBase<BufferInterface, RenderDeviceImplType>;

            Buffer(RenderDeviceImplType* device, BufferCreateDesc desc) : TBufferBase(device) 
            {
                m_pCpuMappedAddress = nullptr;
                m_size = 0;
                m_descriptors = 0;
                m_memoryUsage = 0;
                m_nodeIndex = 0;
                m_desc = desc;
            }

            virtual ~Buffer() = default;

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
        protected:
            /// CPU address of the mapped buffer (applicable to buffers created in CPU accessible heaps (CPU, CPU_TO_GPU, GPU_TO_CPU)
            void* m_pCpuMappedAddress;
            uint64_t m_size : 32;
            uint64_t m_descriptors : 20;
            uint64_t m_memoryUsage : 3;
            uint64_t m_nodeIndex : 4;
            BufferCreateDesc m_desc;

            friend RenderDeviceImplType;
        };
    }
}