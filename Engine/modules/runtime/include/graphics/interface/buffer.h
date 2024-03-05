#pragma once
#include "common/flags.h"
#include "common/cyber_graphics_config.h"
#include "render_object.h"
#include "interface/graphics_types.h"

namespace Cyber
{
    namespace RenderObject
    {

        // Buffer Interface
        struct CYBER_GRAPHICS_API IBuffer
        {
            
        };

        /// Buffer Group
        struct CYBER_GRAPHICS_API BufferCreateDesc
        {
            /// Size of the buffer (in bytes)
            uint64_t m_Size;
            /// Set this to specify a counter buffer for this buffer (applicable to BUFFER_USAGE_STORAGE_SRV, BUFFER_USAGE_STORAGE_UAV)
            class IBuffer* m_pCounterBuffer;
            /// Index of the first element accessible by the SRV/UAV (applicable to BUFFER_USAGE_STORAGE_SRV, BUFFER_USAGE_STORAGE_UAV)
            uint64_t m_FirstElement;
            /// Number of elements in the buffer (applicable to BUFFER_USAGE_STORAGE_SRV, BUFFER_USAGE_STORAGE_UAV)
            uint64_t m_ElementCount;
            /// Size of each element (in bytes) in the buffer (applicable to BUFFER_USAGE_STORAGE_SRV, BUFFER_USAGE_STORAGE_UAV)
            uint64_t m_StructStride;
            /// Debug name used in gpu profile
            const char8_t* m_pName;
            uint32_t* m_pSharedNodeIndices;
            /// Alignment
            uint32_t m_Alignment;
            /// Decides which memory heap buffer will use (default, upload, readback)
            GRAPHICS_RESOURCE_MEMORY_USAGE m_MemoryUsage;
            /// Creation flags of the buffer
            BUFFER_CREATION_FLAG m_Flags;
            /// What type of queue the buffer is owned by
            QUEUE_TYPE m_QueueType;
            /// What state will the buffer get created in
            GRAPHICS_RESOURCE_STATE m_StartState;
            /// ICB draw type
            INDIRECT_ARGUMENT_TYPE m_ICBDrawType;
            /// ICB max vertex buffers slots count
            uint32_t m_ICBMaxCommandCount;
            /// Image format
            TEXTURE_FORMAT m_Format;
            /// Descriptor creation
            DESCRIPTOR_TYPE m_Descriptors;
        };

        template<typename EngineImplTraits>
        class CYBER_GRAPHICS_API Buffer : public RenderObjectBase<typename EngineImplTraits::BufferInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using BufferInterface = typename EngineImplTraits::BufferInterface;
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
            using TRenderObjectBase = RenderObjectBase<BufferInterface, RenderDeviceImplType>;

            Buffer(RenderDeviceImplType* device) : TRenderObjectBase(device) {}

            /// CPU address of the mapped buffer (applicable to buffers created in CPU accessible heaps (CPU, CPU_TO_GPU, GPU_TO_CPU)
            void* m_pCpuMappedAddress;
            uint64_t m_Size : 32;
            uint64_t m_Descriptors : 20;
            uint64_t m_MemoryUsage : 3;
            uint64_t m_NodeIndex : 4;
        protected:
            friend RenderDeviceImplType;
        };
    }
}