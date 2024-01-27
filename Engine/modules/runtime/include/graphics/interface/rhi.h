#pragma once
#include "graphics/common/cyber_graphics_config.h"
#include "graphics/common/flags.h"
#include <EASTL/vector.h>
#include <stdint.h>
#include "core/Core.h"
#include "core/Window.h"
#include "CyberLog/Log.h"
//#include "texture.h"
//#include "texture_view.h"

namespace Cyber
{
    #define RHI_MAX_VERTEX_ATTRIBUTES 15
    #define RHI_MAX_VERTEX_BINDINGS 15
    #define RHI_MAX_MRT_COUNT 8u
    #define RHI_ARRAY_LEN(array) ((sizeof(array)/ sizeof(array[0])))
    
    namespace RenderObject
    {
        class IRenderDevice;
        class ITexture;
        class ITextureView;
        class IBuffer;
        class IFrameBuffer;
        class ISwapChain;
        class ISemaphore;
        class IFence;
        class ICommandPool;
        class ICommandBuffer;
        class IShaderResource;
        class IShaderLibrary;
        class IRootSignature;
        class IRenderPass;
        class IRootSignaturePool;
    }

    typedef uint32_t RHIQueueIndex;
    struct RHIDescriptorSet;
    typedef Ref<RHIDescriptorSet> DescriptorSetRHIRef;

    struct CYBER_GRAPHICS_API RHISurface
    {
        HWND handle;
    };

    /// Shaders
    struct CYBER_GRAPHICS_API RHIShaderVariable
    {
        // Variable name 
        const char* pNmae;
        // parents resource index
        uint32_t parent_index;
    };
    struct CYBER_GRAPHICS_API ShaderMacro
    {
        const char* definition;
        const char* value;
    };
    
    struct CYBER_GRAPHICS_API RHIDescriptorSet
    {

    };

    struct CYBER_GRAPHICS_API RHIRenderPipeline
    {
        RenderObject::IRenderDevice* device;
        RenderObject::IRootSignature* root_signature;
    };
    /// Shader Reflection
    struct CYBER_GRAPHICS_API ShaderConstant
    {
        const void* value;
        uint32_t index;
        uint32_t size;
    };

    struct CYBER_GRAPHICS_API ShaderByteCodeBuffer 
    {
        static CYBER_CONSTEXPR const uint32_t stack_size = 128u * 1024;
        // Stack memory, no need to deallocate it. Used first, if a shader if too big we allocate heap mempry
        void* stack_memory;
        uint32_t stack_used;
    };

    struct CYBER_GRAPHICS_API ShaderVersion
    {
        uint32_t major = 0;
        uint32_t minor = 0;

        CYBER_CONSTEXPR  ShaderVersion() CYBER_NOEXCEPT
        {

        }

        CYBER_CONSTEXPR ShaderVersion(uint32_t major, uint32_t minor) CYBER_NOEXCEPT
            : major(major)
            , minor(minor)
        {

        }

        CYBER_CONSTEXPR bool operator==(const ShaderVersion& other) const
        {
            return major == other.major && minor == other.minor;
        }

        CYBER_CONSTEXPR bool operator!=(const ShaderVersion& other) const
        {
            return !(*this == other);
        }

        CYBER_CONSTEXPR bool operator<(const ShaderVersion& other) const
        {
            return major < other.major || (major == other.major && minor < other.minor);
        }

        CYBER_CONSTEXPR bool operator>(const ShaderVersion& other) const
        {
            return major > other.major || (major == other.major && minor > other.minor);
        }

        CYBER_CONSTEXPR bool operator<=(const ShaderVersion& other) const
        {
            return !(*this > other);
        }

        CYBER_CONSTEXPR bool operator>=(const ShaderVersion& other) const
        {
            return !(*this < other);
        }
    };

    struct CYBER_GRAPHICS_API RHIDescriptorData
    {
        // Update Via shader reflection
        const char8_t* name;
        // Update Via binding slot
        uint32_t binding;
        ERHIResourceType binding_type;
        union
        {
            struct
            {
                /// Offset to bind the buffer descriptor
                const uint64_t* offsets;
                const uint64_t* sizes;
            } buffers_params;
            struct
            {
                uint32_t uav_mip_slice;
                bool blend_mip_chain;
            } uav_params;
            bool enable_stencil_resource;
        };
        union
        {
            const void** ptrs;
            /// Array of texture descriptors (srv and uav textures)
            RenderObject::ITextureView** textures;
            /// Array of sampler descriptors
            struct RHISampler** samplers;
            /// Array of buffer descriptors (srv uav and cbv buffers)
            RenderObject::IBuffer** buffers;
            /// Array of pipeline descriptors
            RHIRenderPipeline** render_pipelines;

            /// DescriptorSet buffer extraction
            RHIDescriptorSet** descriptor_sets;
        };
        uint32_t count;
    };
    
    struct CYBER_GRAPHICS_API RHIBufferRange
    {
        uint64_t offset;
        uint64_t size;
    };

    struct CYBER_GRAPHICS_API RHIBufferBarrier
    {
        RenderObject::IBuffer* buffer;
        ERHIResourceState src_state;
        ERHIResourceState dst_state;
        uint8_t queue_acquire : 1;
        uint8_t queue_release : 1;
        ERHIQueueType queue_type : 5;
        struct {
            uint8_t begin_only : 1;
            uint8_t end_only : 1;
        } d3d12;
    };

    struct CYBER_GRAPHICS_API RHITextureBarrier
    {
        RenderObject::ITexture* texture;
        ERHIResourceState src_state;
        ERHIResourceState dst_state;
        uint8_t queue_acquire : 1;
        uint8_t queue_release : 1;
        ERHIQueueType queue_type : 5;
        /// Specify whether following barrier targets particular subresource
        uint8_t subresource_barrier : 1;
        /// Following values are ignored if subresource_barrier is false
        uint8_t mip_level : 7;
        uint16_t array_layer;
        struct {
            uint8_t begin_only : 1;
            uint8_t end_only : 1;
        } d3d12;
    };

    struct CYBER_GRAPHICS_API RHIResourceBarrierDesc
    {
        const RHIBufferBarrier* buffer_barriers;
        uint32_t buffer_barrier_count;
        const RHITextureBarrier* texture_barriers;
        uint32_t texture_barrier_count;
    };

    /// Device Group
    struct CYBER_GRAPHICS_API RHIQueueGroupDesc
    {
        ERHIQueueType queue_type;
        uint32_t queue_count;
    };

    using RHIRenderPassEncoder = RenderObject::ICommandBuffer;
    using RHIComputePassEncoder = RenderObject::ICommandBuffer;

    struct CYBER_GRAPHICS_API RHIAcquireNextDesc
    {
        RenderObject::ISemaphore* signal_semaphore;
        RenderObject::IFence* fence;
    };

    struct CYBER_GRAPHICS_API RHIDescriptorSetCreateDesc
    {
        RenderObject::IRootSignature* root_signature;
        uint32_t set_index;
    };

    struct CYBER_GRAPHICS_API RHIColorAttachment
    {
        RenderObject::ITextureView* view;
        RenderObject::ITextureView* resolve_view;
        ERHILoadAction load_action;
        ERHIStoreAction store_action;
        ERHIClearValue clear_value;
    };

    struct CYBER_GRAPHICS_API RHIDepthStencilAttachment
    {
        RenderObject::ITextureView* view;
        ERHILoadAction depth_load_action;
        ERHIStoreAction depth_store_action;
        float clear_depth;
        uint8_t write_depth;
        ERHILoadAction stencil_load_action;
        ERHIStoreAction stencil_store_action;
        uint32_t clear_stencil;
        uint8_t write_stencil;
    };


    struct CYBER_GRAPHICS_API RHIBlendStateCreateDesc
    {
        /// Source blend factor per render target
        ERHIBlendConstant src_factors[RHI_MAX_MRT_COUNT];
        /// Destination blend factor per render target
        ERHIBlendConstant dst_factors[RHI_MAX_MRT_COUNT];
        /// Source blend alpha factor per render target
        ERHIBlendConstant src_alpha_factors[RHI_MAX_MRT_COUNT];
        /// Destination blend alpha factor per render target
        ERHIBlendConstant dst_alpha_factors[RHI_MAX_MRT_COUNT];
        /// Blend mode per render target
        ERHIBlendMode blend_modes[RHI_MAX_MRT_COUNT];
        /// Blend alpha mode per render target
        ERHIBlendMode blend_alpha_modes[RHI_MAX_MRT_COUNT];
        /// Color write mask per render target
        int32_t masks[RHI_MAX_MRT_COUNT];
        /// Enable alpha to coverage
        bool alpha_to_coverage : 1;
        /// Set whether each render target has an unique blend function. When false the blend function in slot 0 will be used for all render targets.
        bool independent_blend : 1;
    };

    struct CYBER_GRAPHICS_API RHIDepthStateCreateDesc
    {
        bool depth_test : 1;
        bool depth_write : 1;
        ERHICompareMode depth_func;
        bool stencil_test : 1;
        uint8_t stencil_read_mask;
        uint8_t stencil_write_mask;
        ERHICompareMode stencil_front_func;
        ERHIStencilOp stencil_front_fail_op;
        ERHIStencilOp stencil_front_depth_fail_op;
        ERHIStencilOp stencil_front_pass_op;
        ERHICompareMode stencil_back_func;
        ERHIStencilOp stencil_back_fail_op;
        ERHIStencilOp stencil_back_depth_fail_op;
        ERHIStencilOp stencil_back_pass_op;   
    };

    struct CYBER_GRAPHICS_API RHIRasterizerStateCreateDesc
    {  
        ERHICullMode cull_mode;
        int32_t depth_bias;
        float slope_scaled_depth_bias;
        ERHIFillMode fill_mode;
        ERHIFrontFace front_face;
        bool enable_multisample : 1;
        bool enable_scissor : 1;
        bool enable_depth_clip : 1;
    };

    struct CYBER_GRAPHICS_API RHIVertexAttribute 
    {
        char8_t semantic_name[64];
        uint32_t array_size;
        ERHIFormat format;
        uint32_t binding;
        uint32_t offset;
        uint32_t elem_stride;
        ERHIVertexInputRate input_rate;
    };

    struct CYBER_GRAPHICS_API RHIVertexLayout
    {
        uint32_t attribute_count;
        RHIVertexAttribute attributes[RHI_MAX_VERTEX_ATTRIBUTES];
    };


    #define RHI_SINGLE_GPU_NODE_COUNT 1
    #define RHI_SINGLE_GPU_NODE_MASK 1
    #define RHI_SINGLE_GPU_NODE_INDEX 0
}
