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
    }

    typedef uint32_t RHIQueueIndex;
    struct RHIFence;
    typedef Ref<RHIFence> FenceRHIRef;
    struct RHIInstance;
    typedef Ref<RHIInstance> InstanceRHIRef;
    struct RHICommandPool;
    struct RHIQueue;
    typedef Ref<RHIQueue> QueueRHIRef;
    typedef Ref<RHICommandPool> CommandPoolRef;
    struct RHICommandBuffer;
    typedef Ref<RHICommandBuffer> CommandBufferRef;
    struct RHISwapChain;
    typedef Ref<RHISwapChain> SwapChainRef;
    struct RHIRootSignature;
    typedef Ref<RHIRootSignature> RootSignatureRHIRef;
    struct RHIShaderLibrary;
    typedef Ref<RHIShaderLibrary> ShaderLibraryRHIRef;
    struct RHIDescriptorSet;
    typedef Ref<RHIDescriptorSet> DescriptorSetRHIRef;

    typedef enum ERHIBackend
    {
        RHI_BACKEND_D3D12,
        RHI_BACKEND_VULKAN,
        RHI_BACKEND_METAL
    } ERHIBackend;


    struct CYBER_GRAPHICS_API RHISurface
    {
        HWND handle;
    };

    struct CYBER_GRAPHICS_API RHIAdapter
    {
        RHIInstance* pInstance = nullptr;
    };

    struct CYBER_GRAPHICS_API RHIFence
    {
        RenderObject::IRenderDevice* pDevice;
    };

    struct CYBER_GRAPHICS_API RHISemaphore
    {
        RenderObject::IRenderDevice* pDevice;
    };

    /// Shaders
    struct CYBER_GRAPHICS_API RHIShaderResource
    {
        const char8_t* name;
        uint64_t name_hash;
        ERHIResourceType type;
        ERHITextureDimension dimension;
        uint32_t set;
        uint32_t binding;
        uint32_t size;
        uint32_t offset;
        ERHIShaderStages stages;
    };

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

    struct CYBER_GRAPHICS_API RHIParameterTable
    {
        // This should be stored here because shader could be destroyed after RS creation
        RHIShaderResource* resources = nullptr;
        uint32_t resource_count = 0;
        uint32_t set_index = 0;
    };

    struct RHIRootSignaturePool
    {
        RenderObject::IRenderDevice* render_device;
        ERHIPipelineType mPipelineType;
    };

    struct CYBER_GRAPHICS_API RHIRootSignature
    {
        RenderObject::IRenderDevice* render_device;
        RHIParameterTable* parameter_tables;
        uint32_t parameter_table_count;
        RHIShaderResource* push_constants;
        uint32_t push_constant_count;
        RHIShaderResource* static_samplers;
        uint32_t static_sampler_count;
        ERHIPipelineType pipeline_type;
        RHIRootSignaturePool* pool;
        RHIRootSignature* pool_next;
    };
    
    struct CYBER_GRAPHICS_API RHIDescriptorSet
    {
        RHIRootSignature* root_signature;
        uint32_t set_index;
    };

    struct CYBER_GRAPHICS_API RHIRenderPipeline
    {
        RenderObject::IRenderDevice* device;
        RHIRootSignature* root_signature;
    };
    /// Shader Reflection
    struct CYBER_GRAPHICS_API ShaderConstant
    {
        const void* value;
        uint32_t index;
        uint32_t size;
    };

    struct CYBER_GRAPHICS_API RHIVertexInput
    {
        // resource name
        const char8_t* name;
        const char8_t* semantics_name;
        uint32_t semantics_index;
        uint32_t binding;
        ERHIFormat format;
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

    struct CYBER_GRAPHICS_API RHIShaderReflection
    {
        const char8_t* entry_name;
        char* name_pool;
        RHIVertexInput* vertex_inputs;
        RHIShaderResource* shader_resources;
        ERHIShaderStage shader_stage;
        uint32_t name_pool_size;
        uint32_t vertex_input_count;
        uint32_t shader_resource_count;
        uint32_t variable_count;

        // Thread group size for compute shader
        uint32_t num_threads_per_group;
        // number of tessellation control point
        uint32_t num_control_point;
        uint32_t thread_group_sizes[3];
    };

    struct CYBER_GRAPHICS_API RHIShaderLibrary
    {
        RenderObject::IRenderDevice* pDevice;
        char8_t* pName;
        RHIShaderReflection* entry_reflections;
        uint32_t entry_count;
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

    struct CYBER_GRAPHICS_API RHISampler 
    {

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

    struct CYBER_GRAPHICS_API RHIInstanceCreateDesc
    {
        bool enable_debug_layer;
        bool enable_gpu_based_validation;
        bool enable_set_name;
    };

    // Objects
    struct CYBER_GRAPHICS_API RHIInstance
    {
        ERHIBackend mBackend;
        ERHINvAPI_Status mNvAPIStatus;
        ERHIAGSReturenCode mAgsStatus;
        bool mEnableSetName;
    };
    /// Device Group
    struct CYBER_GRAPHICS_API RHIQueueGroupDesc
    {
        ERHIQueueType queue_type;
        uint32_t queue_count;
    };

    struct CYBER_GRAPHICS_API RHIAdapterDetail
    {
        uint32_t mUniformBufferAlignment;
        uint32_t mUploadBufferTextureAlignment;
        uint32_t mUploadBufferTextureRowAlignment;
        uint32_t mMaxVertexInputBindings;
        uint32_t mWaveLaneCount;
        uint32_t mHostVisibleVRamBudget;
        bool mSupportHostVisibleVRam : 1;
        bool mMultiDrawIndirect : 1;
        bool mSupportGeomShader : 1;
        bool mSupportTessellation : 1;
        bool mIsUma : 1;
        bool mIsVirtual : 1;
        bool mIsCpu : 1;
    };

    struct CYBER_GRAPHICS_API RHIQueue
    {
        RenderObject::IRenderDevice* pDevice;
        ERHIQueueType mType;
        RHIQueueIndex mIdex;
    };

    struct CYBER_GRAPHICS_API RHICommandPool
    {
        RHIQueue* pQueue;
    };

    struct CYBER_GRAPHICS_API RHICommandBuffer
    {
        RenderObject::IRenderDevice* pDevice;
        Cyber::Ref<RHICommandPool> pPool;
        ERHIPipelineType mCurrentDispatch;
    };
    using RHIRenderPassEncoder = RHICommandBuffer;
    using RHIComputePassEncoder = RHICommandBuffer;

    struct CYBER_GRAPHICS_API RHIQueryPool
    {
        RenderObject::IRenderDevice* pDevice;
        uint32_t mCount;
    };

    struct CYBER_GRAPHICS_API RHISwapChain
    {
        RenderObject::ITexture** mBackBufferSRVs;
        RenderObject::ITextureView** mBackBufferSRVViews;
        uint32_t mBufferSRVCount;
        RenderObject::ITexture* mBackBufferDSV;
        RenderObject::ITextureView* mBackBufferDSVView;
    };

    struct CYBER_GRAPHICS_API CommandPoolCreateDesc
    {
        uint32_t ___nothing_and_useless__;
    };

    struct CYBER_GRAPHICS_API CommandBufferCreateDesc
    {
        bool is_secondary : 1;
    };

    struct CYBER_GRAPHICS_API RHIQueueSubmitDesc
    {
        RHICommandBuffer** pCmds;
        RHIFence* mSignalFence;
        RHISemaphore** pWaitSemaphores;
        RHISemaphore** pSignalSemaphores;
        uint32_t mCmdsCount;
        uint32_t mWaitSemaphoreCount;
        uint32_t mSignalSemaphoreCount;
    };

    struct CYBER_GRAPHICS_API RHIQueuePresentDesc
    {
        RHISwapChain* swap_chain;
        const RHISemaphore** wait_semaphores;
        uint32_t wait_semaphore_count;
        uint32_t index;
    };

    struct CYBER_GRAPHICS_API RHISwapChainCreateDesc
    {
        /// Present Queues
        RHIQueue* mPresentQueue;
        /// Present Queues Count
        uint32_t mPresentQueueCount;
        /// Number of backbuffers in the swapchain
        uint32_t mImageCount;
        /// Width of the swapchain
        uint32_t mWidth;
        /// Height of the swapchain 
        uint32_t mHeight;
        /// Format of the swapchain
        ERHIFormat mFormat;
        /// Surface
        RHISurface* surface;
        /// Set whether swapchain will be presented using vsync
        bool mEnableVsync;
        /// We can toogle to using FLIP model if app desires
        bool mUseFlipSwapEffect;
    };

    struct CYBER_GRAPHICS_API RHIAcquireNextDesc
    {
        RHISemaphore* signal_semaphore;
        RHIFence* fence;
    };

    struct CYBER_GRAPHICS_API RHIPipelineShaderCreateDesc
    {
        RHIShaderLibrary* library;
        const char8_t* entry;
        ERHIShaderStage stage;
    };

    struct CYBER_GRAPHICS_API RHIRootSignaturePoolCreateDesc
    {
        const char8_t* name;
    };

    struct CYBER_GRAPHICS_API RHIRootSignatureCreateDesc
    {
        RHIPipelineShaderCreateDesc** shaders;
        uint32_t shader_count;
        RHISampler** static_samplers;
        const char8_t* const* static_sampler_names;
        uint32_t static_sampler_count;
        const char8_t* const* push_constant_names;
        uint32_t push_constant_count;
        RHIRootSignaturePool* pool;
    };
    
    struct CYBER_GRAPHICS_API RHIDescriptorSetCreateDesc
    {
        RHIRootSignature* root_signature;
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

    struct CYBER_GRAPHICS_API RHIShaderLibraryCreateDesc
    {
        const char8_t* name;
        const char8_t* entry_point;
        const void* code;
        uint32_t code_size;

        ERHIShaderStage stage;
        EShaderCompiler shader_compiler;
        ERHIShaderTarget shader_target;
        uint32_t shader_macro_count;
        ShaderMacro* shader_macros;
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

    struct CYBER_GRAPHICS_API RHIRenderPipelineCreateDesc
    {
        RHIRootSignature* root_signature;
        RHIPipelineShaderCreateDesc* vertex_shader;
        RHIPipelineShaderCreateDesc* tesc_shader;
        RHIPipelineShaderCreateDesc* tese_shader;
        RHIPipelineShaderCreateDesc* geometry_shader;
        RHIPipelineShaderCreateDesc* fragment_shader;
        const RHIVertexLayout* vertex_layout;
        RHIBlendStateCreateDesc* blend_state;
        RHIDepthStateCreateDesc* depth_stencil_state;
        RHIRasterizerStateCreateDesc* rasterizer_state;

        const ERHIFormat* color_formats;
        uint32_t render_target_count;
        ERHITextureSampleCount sample_count;
        uint32_t sample_quality;
        ERHISlotMaskBit color_resolve_disable_mask;
        ERHIFormat depth_stencil_format;
        ERHIPrimitiveTopology prim_topology;
        bool enable_indirect_command;
    };

    #define RHI_SINGLE_GPU_NODE_COUNT 1
    #define RHI_SINGLE_GPU_NODE_MASK 1
    #define RHI_SINGLE_GPU_NODE_INDEX 0
}
