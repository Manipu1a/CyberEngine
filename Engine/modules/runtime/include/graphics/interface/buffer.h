#pragma once
#include "common/flags.h"
#include "common/cyber_graphics_config.h"

namespace Cyber
{
    namespace RenderObject
    {
        class CERenderDevice;

        /// Buffer Group
        struct CYBER_GRAPHICS_API BufferCreateDesc
        {
            /// Size of the buffer (in bytes)
            uint64_t mSize;
            /// Set this to specify a counter buffer for this buffer (applicable to BUFFER_USAGE_STORAGE_SRV, BUFFER_USAGE_STORAGE_UAV)
            class Buffer* pCounterBuffer;
            /// Index of the first element accessible by the SRV/UAV (applicable to BUFFER_USAGE_STORAGE_SRV, BUFFER_USAGE_STORAGE_UAV)
            uint64_t mFirstElement;
            /// Number of elements in the buffer (applicable to BUFFER_USAGE_STORAGE_SRV, BUFFER_USAGE_STORAGE_UAV)
            uint64_t mElementCount;
            /// Size of each element (in bytes) in the buffer (applicable to BUFFER_USAGE_STORAGE_SRV, BUFFER_USAGE_STORAGE_UAV)
            uint64_t mStructStride;
            /// Debug name used in gpu profile
            const char8_t* pName;
            uint32_t* pSharedNodeIndices;
            /// Alignment
            uint32_t mAlignment;
            /// Decides which memory heap buffer will use (default, upload, readback)
            ERHIResourceMemoryUsage mMemoryUsage;
            /// Creation flags of the buffer
            ERHIBufferCreationFlags mFlags;
            /// What type of queue the buffer is owned by
            ERHIQueueType mQueueType;
            /// What state will the buffer get created in
            ERHIResourceState mStartState;
            /// ICB draw type
            ERHIIndirectArgumentType mICBDrawType;
            /// ICB max vertex buffers slots count
            uint32_t mICBMaxCommandCount;
            /// Image format
            ERHIFormat mFormat;
            /// Descriptor creation
            ERHIDescriptorType mDescriptors;
        };

        class CYBER_GRAPHICS_API Buffer
        {
        public:
            /// CPU address of the mapped buffer (applicable to buffers created in CPU accessible heaps (CPU, CPU_TO_GPU, GPU_TO_CPU)
            void* pCpuMappedAddress;
            uint64_t mSize : 32;
            uint64_t mDescriptors : 20;
            uint64_t mMemoryUsage : 3;
            uint64_t mNodeIndex : 4;
            RenderObject::CERenderDevice* device;

        protected:
            friend class RenderObject::CERenderDevice;
        };
    }
}