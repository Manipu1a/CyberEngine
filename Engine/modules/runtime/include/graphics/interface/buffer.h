#pragma once
#include "common/flags.h"
#include "common/cyber_graphics_config.h"
#include "graphics/interface/buffer_view.h"
#include "core/debug.h"
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

        /// Buffer类型
        CYBER_TYPED_ENUM(BUFFER_MODE, uint8_t)
        {
            BUFFER_MODE_UNDEFINED = 0,
            /// 格式化缓冲区，将缓冲区数据解释为具有特定DXGI_FORMAT的元素序列
            /// 在shader中通过Buffer<type>访问
            BUFFER_MODE_FORMATTED,
            /// 结构化缓冲区，将缓冲区数据解释为用户定义的结构体数组, Format必须是DXGI_FORMAT_UNKNOWN
            /// 在shader中通过StructuredBuffer<type>访问
            BUFFER_MODE_STRUCTURED,
            /// 原始缓冲区，缓冲区数据被视为原始字节序列，不强制任何特定的格式
            /// 在shader中通过ByteAddressBuffer\RWByteAddressBuffer访问
            BUFFER_MODE_RAW,
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
            /// Buffer mode (formatted, structured, raw)
            BUFFER_MODE mode;
            /// CPU access flags
            CPU_ACCESS_FLAGS cpu_access_flags;
            /// What type of queue the buffer is owned by
            COMMAND_QUEUE_TYPE queueType;
            /// What state will the buffer get created in
            GRAPHICS_RESOURCE_STATE startState;
            /// Image format
            TEXTURE_FORMAT format;
        };

        
        // Buffer Interface
        struct CYBER_GRAPHICS_API IBuffer : public IDeviceObject
        {
            virtual void* get_mapped_data() = 0;
            virtual uint64_t get_size() const = 0;
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
            using BufferViewImplType = typename EngineImplTraits::BufferViewImplType;
            using TDeviceObjectBase = DeviceObjectBase<BufferInterface, RenderDeviceImplType>;

            BufferBase(RenderDeviceImplType* device, BufferCreateDesc desc) : TDeviceObjectBase(device), default_uav_view(nullptr), default_srv_view(nullptr)
            {
                m_pCpuMappedAddress = nullptr;
                m_size = 0;
                m_nodeIndex = 0;
                m_desc = desc;
                
                create_default_views();
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

            virtual IBuffer_View* create_view_internal(const BufferViewCreateDesc& desc) const = 0;
        protected:

            void create_default_views()
            {
                auto CreateDefaultView = [&](BUFFER_VIEW_TYPE view_type)
                {
                    BufferViewCreateDesc view_desc = {};
                    view_desc.buffer = this;
                    view_desc.format = m_desc.format;
                    view_desc.view_type = view_type;

                    auto* view = create_view_internal(view_desc);
                    cyber_assert(view != nullptr, "Failed to create default buffer view");

                    BufferViewImplType* buffer_view_impl = static_cast<BufferViewImplType*>(view);

                    return buffer_view_impl;
                };

                if(m_desc.bind_flags & GRAPHICS_RESOURCE_BIND_SHADER_RESOURCE && (m_desc.mode == BUFFER_MODE_STRUCTURED || m_desc.mode == BUFFER_MODE_RAW))
                {
                    default_uav_view = (CreateDefaultView(BUFFER_VIEW_TYPE::BUFFER_VIEW_SHADER_RESOURCE));
                }

                if(m_desc.bind_flags & GRAPHICS_RESOURCE_BIND_UNORDERED_ACCESS && (m_desc.mode == BUFFER_MODE_STRUCTURED || m_desc.mode == BUFFER_MODE_RAW))
                {
                    default_srv_view = (CreateDefaultView(BUFFER_VIEW_TYPE::BUFFER_VIEW_UNORDERED_ACCESS));
                }
            }

            /// CPU address of the mapped buffer (applicable to buffers created in CPU accessible heaps (CPU, CPU_TO_GPU, GPU_TO_CPU)
            void* m_pCpuMappedAddress;
            uint64_t m_size : 32;
            uint64_t m_nodeIndex : 4;
            BufferCreateDesc m_desc;
            GRAPHICS_RESOURCE_STATE buffer_state;
            /// Default views for this buffer
            BufferViewImplType* default_uav_view;
            BufferViewImplType* default_srv_view;

            friend RenderDeviceImplType;
        };
    }
}