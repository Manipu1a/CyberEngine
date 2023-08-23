#pragma once
#include "cyber_rhi_config.h"
#include "flags.h"
#include <EASTL/vector.h>
#include <stdint.h>
#include "core/Core.h"
#include "core/Window.h"
#include "CyberLog/Log.h"

namespace Cyber
{
    #define RHI_MAX_VERTEX_ATTRIBUTES 15
    #define RHI_MAX_VERTEX_BINDINGS 15
    #define RHI_MAX_MRT_COUNT 8u
    #define RHI_ARRAY_LEN(array) ((sizeof(array)/ sizeof(array[0])))
    
    typedef uint32_t RHIQueueIndex;
    struct RHITexture;
    typedef Ref<RHITexture> TextureRHIRef;
    struct RHITexture2D;
    typedef Ref<RHITexture2D> Texture2DRHIRef;
    struct RHIBuffer;
    typedef Ref<RHIBuffer> BufferRHIRef;
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

    typedef union ERHIClearValue
    {
        struct
        {
            float r;
            float g;
            float b;
            float a;
        };
        struct
        {
            float depth;
            uint32_t stencil;
        };
    } ERHIClearValue;
    
    static const ERHIClearValue fastclear_0000 = {
        {0.f, 0.f, 0.f, 0.f}
    };
    static const ERHIClearValue fastclear_0001 = {
        {0.f, 0.f, 0.f, 1.f}
    };
    static const ERHIClearValue fastclear_1110 = {
        {1.f, 1.f, 1.f, 0.f}
    };
    static const ERHIClearValue fastclear_1111 = {
        {1.f, 1.f, 1.f, 1.f}
    };
    
    struct CYBER_RHI_API RHISurface
    {

    };

    struct CYBER_RHI_API RHIAdapter
    {
        RHIInstance* pInstance = nullptr;
    };

    struct CYBER_RHI_API RHIDevice
    {
        RHIAdapter* pAdapter;
    };

    struct CYBER_RHI_API RHIFence
    {
        RHIDevice* pDevice;
    };

    struct CYBER_RHI_API RHISemaphore
    {
        RHIDevice* pDevice;
    };

    /// Shaders
    struct CYBER_RHI_API RHIShaderResource
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

    struct CYBER_RHI_API RHIShaderVariable
    {
        // Variable name 
        const char* pNmae;
        // parents resource index
        uint32_t parent_index;
    };
    struct CYBER_RHI_API ShaderMacro
    {
        const char* definition;
        const char* value;
    };

    struct CYBER_RHI_API RHIParameterTable
    {
        // This should be stored here because shader could be destroyed after RS creation
        RHIShaderResource* resources;
        uint32_t resource_count;
        uint32_t set_index;
    };

    struct RHIRootSignaturePool
    {
        Ref<RHIDevice> pDevice;
        ERHIPipelineType mPipelineType;
    };

    struct CYBER_RHI_API RHIRootSignature
    {
        Ref<RHIDevice> device;
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
    
    struct CYBER_RHI_API RHIDescriptorSet
    {
        RHIRootSignature* root_signature;
        uint32_t set_index;
    };

    struct CYBER_RHI_API RHIRenderPipeline
    {
        Ref<RHIDevice> device;
        RHIRootSignature* root_signature;
    };
    /// Shader Reflection
    struct CYBER_RHI_API ShaderConstant
    {
        const void* value;
        uint32_t index;
        uint32_t size;
    };

    struct CYBER_RHI_API RHIVertexInput
    {
        // resource name
        const char8_t* name;
        const char8_t* semantics;
        ERHIFormat format;
    };
    
    struct CYBER_RHI_API ShaderByteCodeBuffer 
    {
        static CYBER_CONSTEXPR const uint32_t stack_size = 128u * 1024;
        // Stack memory, no need to deallocate it. Used first, if a shader if too big we allocate heap mempry
        void* stack_memory;
        uint32_t stack_used;
    };

    struct CYBER_RHI_API RHIShaderReflection
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

    struct CYBER_RHI_API RHIShaderLibrary
    {
        Ref<RHIDevice> pDevice;
        char8_t* pName;
        RHIShaderReflection* entry_reflections;
        uint32_t entry_count;
    };

    struct CYBER_RHI_API RHIDescriptorData
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
            struct RHITextureView** textures;
            /// Array of sampler descriptors
            struct RHISampler** samplers;
            /// Array of buffer descriptors (srv uav and cbv buffers)
            struct RHIBuffer** buffers;
            /// Array of pipeline descriptors
            RHIRenderPipeline** render_pipelines;

            /// DescriptorSet buffer extraction
            RHIDescriptorSet** descriptor_sets;
        };
        uint32_t count;
    };

    /// Texture group
    struct CYBER_RHI_API TextureCreateDesc
    {
        /// Optimized clear value (recommended to use this same value when clearing the rendertarget)
        ERHIClearValue mClearValue;
        /// Pointer to native texture handle if the texture does not own underlying resource
        void* mNativeHandle;
        /// Debug name used in gpu profile
        const char* pName;

        uint32_t mWidth;
        uint32_t mHeight;
        uint32_t mDepth;
        uint32_t mArraySize;
        uint32_t mMipLevels;
        ERHIDescriptorType mDescriptors;
        RHITextureCreationFlag mFlags;
        /// Number of multisamples per pixel (currently Textures created with mUsage TEXTURE_USAGE_SAMPLED_IMAGE only support SAMPLE_COUNT_1)
        ERHITextureSampleCount mSampleCount;
        /// The image quality level. The higher the quality, the lower the performance. The valid range is between zero and the value appropriate for mSampleCount
        uint32_t mSampleQuality;
        /// Image format
        ERHIFormat mFormat;
        /// What state will the texture get created in
        ERHIResourceState mStartState;
    };

    struct CYBER_RHI_API TextureViewCreateDesc
    {
        const char8_t* name;
        RHITexture* texture;
        ERHIFormat format;
        ERHITextureViewUsages usages;
        ERHITextureViewAspect aspects;
        ERHITextureDimension dimension;
        uint32_t base_array_layer;
        uint32_t array_layer_count;
        uint32_t base_mip_level;
        uint32_t mip_level_count;
    };

    struct CYBER_RHI_API RHITexture
    {
        uint32_t mWidth;
        uint32_t mHeight;
        uint32_t mDepth;
        uint32_t mArraySize;
        uint32_t mMipLevels;
        uint32_t mFormat;

        /// Flags specifying which aspects (COLOR,DEPTH,STENCIL) are included in the pVkImageView
        uint32_t mAspectMask;
        uint32_t mNodeIndex;
        uint32_t mIsCube;
        uint32_t mIsDedicated;

        uint32_t mOwnsImage;
        
        void* mNativeHandle;
    };

    struct RHITexture2D : public RHITexture
    {

    };

    struct RHITexture3D : public RHITexture
    {

    };
    
    struct RHITextureView
    {
        TextureViewCreateDesc create_info;
        Ref<RHIDevice> device;
    };

    struct CYBER_RHI_API RHISampler 
    {

    };

    /// Buffer Group
    struct CYBER_RHI_API BufferCreateDesc
    {
        /// Size of the buffer (in bytes)
        uint64_t mSize;
        /// Set this to specify a counter buffer for this buffer (applicable to BUFFER_USAGE_STORAGE_SRV, BUFFER_USAGE_STORAGE_UAV)
        struct RHIBuffer* pCounterBuffer;
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

    struct CYBER_RHI_API RHIBuffer
    {
        /// CPU address of the mapped buffer (applicable to buffers created in CPU accessible heaps (CPU, CPU_TO_GPU, GPU_TO_CPU)
        void* pCpuMappedAddress;
        uint64_t mSize : 32;
        uint64_t mDescriptors : 20;
        uint64_t mMemoryUsage : 3;
        uint64_t mNodeIndex : 4;
        Ref<RHIDevice> device;
    };
    
    struct CYBER_RHI_API RHIBufferRange
    {
        uint64_t offset;
        uint64_t size;
    };

    struct CYBER_RHI_API RHIBufferBarrier
    {
        RHIBuffer* buffer;
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

    struct CYBER_RHI_API RHITextureBarrier
    {
        RHITexture* texture;
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

    struct CYBER_RHI_API RHIResourceBarrierDesc
    {
        const RHIBufferBarrier* buffer_barriers;
        uint32_t buffer_barrier_count;
        const RHITextureBarrier* texture_barriers;
        uint32_t texture_barrier_count;
    };

    struct CYBER_RHI_API RHIInstanceCreateDesc
    {
        bool enable_debug_layer;
        bool enable_gpu_based_validation;
        bool enable_set_name;
    };

    // Objects
    struct CYBER_RHI_API RHIInstance
    {
        ERHIBackend mBackend;
        ERHINvAPI_Status mNvAPIStatus;
        ERHIAGSReturenCode mAgsStatus;
        bool mEnableSetName;
    };
    /// Device Group
    struct CYBER_RHI_API RHIQueueGroupDesc
    {
        ERHIQueueType queue_type;
        uint32_t queue_count;
    };

    struct CYBER_RHI_API RHIAdapterDetail
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

    struct CYBER_RHI_API RHIDeviceCreateDesc
    {
        bool bDisablePipelineCache;
        eastl::vector<RHIQueueGroupDesc> queue_groups;
        uint32_t queue_group_count;
    };

    struct CYBER_RHI_API RHIQueue
    {
        Cyber::Ref<RHIDevice> pDevice;
        ERHIQueueType mType;
        RHIQueueIndex mIdex;
    };

    struct CYBER_RHI_API RHICommandPool
    {
        RHIQueue* pQueue;
    };

    struct CYBER_RHI_API RHICommandBuffer
    {
        Cyber::Ref<RHIDevice> pDevice;
        Cyber::Ref<RHICommandPool> pPool;
        ERHIPipelineType mCurrentDispatch;
    };
    using RHIRenderPassEncoder = RHICommandBuffer;
    using RHIComputePassEncoder = RHICommandBuffer;

    struct CYBER_RHI_API RHIQueryPool
    {
        Cyber::Ref<RHIDevice> pDevice;
        uint32_t mCount;
    };

    struct CYBER_RHI_API RHISwapChain
    {
        RHITexture* mBackBuffers;
        uint32_t mBufferCount;
    };

    struct CYBER_RHI_API CommandPoolCreateDesc
    {
        uint32_t ___nothing_and_useless__;
    };

    struct CYBER_RHI_API CommandBufferCreateDesc
    {
        bool is_secondary : 1;
    };

    struct CYBER_RHI_API RHIQueueSubmitDesc
    {
        RHICommandBuffer** pCmds;
        RHIFence* mSignalFence;
        RHISemaphore** pWaitSemaphores;
        RHISemaphore** pSignalSemaphores;
        uint32_t mCmdsCount;
        uint32_t mWaitSemaphoreCount;
        uint32_t mSignalSemaphoreCount;
    };

    struct CYBER_RHI_API RHIQueuePresentDesc
    {
        RHISwapChain* swap_chain;
        const RHISemaphore** wait_semaphores;
        uint32_t wait_semaphore_count;
        uint32_t index;
    };

    struct CYBER_RHI_API RHISwapChainCreateDesc
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

    struct CYBER_RHI_API RHIAcquireNextDesc
    {
        RHISemaphore* signal_semaphore;
        RHIFence* fence;
    };

    struct CYBER_RHI_API RHIPipelineShaderCreateDesc
    {
        RHIShaderLibrary* library;
        const char8_t* entry;
        ERHIShaderStage stage;
    };

    struct CYBER_RHI_API RHIRootSignaturePoolCreateDesc
    {
        const char8_t* name;
    };

    struct CYBER_RHI_API RHIRootSignatureCreateDesc
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
    
    struct CYBER_RHI_API RHIDescriptorSetCreateDesc
    {
        RHIRootSignature* root_signature;
        uint32_t set_index;
    };

    struct CYBER_RHI_API RHIColorAttachment
    {
        RHITextureView* view;
        RHITextureView* resolve_view;
        ERHILoadAction load_action;
        ERHIStoreAction store_action;
        ERHIClearValue clear_value;
    };

    struct CYBER_RHI_API RHIDepthStencilAttachment
    {
        RHITextureView* view;
        ERHILoadAction depth_load_action;
        ERHIStoreAction depth_store_action;
        float clear_depth;
        uint8_t write_depth;
        ERHILoadAction stencil_load_action;
        ERHIStoreAction stencil_store_action;
        uint32_t clear_stencil;
        uint8_t write_stencil;
    };

    struct CYBER_RHI_API RHIRenderPassDesc
    {
        const char8_t* name;
        ERHITextureSampleCount sample_count;
        const RHIColorAttachment* color_attachments;
        const RHIDepthStencilAttachment* depth_stencil_attachment;
        uint32_t render_target_count;
    };

    struct CYBER_RHI_API RHIBlendStateCreateDesc
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

    struct CYBER_RHI_API RHIDepthStateCreateDesc
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

    struct CYBER_RHI_API RHIRasterizerStateCreateDesc
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

    struct CYBER_RHI_API RHIShaderLibraryCreateDesc
    {
        const char8_t* name;
        const void* code;
        uint32_t code_size;
        ERHIShaderStage stage;
    };

    struct CYBER_RHI_API RHIVertexAttribute 
    {
        char8_t semantic_name[64];
        uint32_t array_size;
        ERHIFormat format;
        uint32_t binding;
        uint32_t offset;
        uint32_t elem_stride;
        ERHIVertexInputRate input_rate;
    };

    struct CYBER_RHI_API RHIVertexLayout
    {
        uint32_t attribute_count;
        RHIVertexAttribute attributes[RHI_MAX_VERTEX_ATTRIBUTES];
    };

    struct CYBER_RHI_API RHIRenderPipelineCreateDesc
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

    class CYBER_RHI_API RHI
    {
    public:
        static void createRHI(ERHIBackend backend);
        static RHI* gloablRHI;
    public:
        // Instance APIs
        virtual RHIInstance* rhi_create_instance(const RHIInstanceCreateDesc& instanceDesc) 
        {
            cyber_core_assert(false, "Empty implement rhi_create_instance!");
            return nullptr;
        }
        virtual void rhi_free_instance(RHIInstance* instance)
        {
            cyber_core_assert(false, "Empty implement rhi_free_instance!");
        }
        // Device APIS
        virtual RHIDevice* rhi_create_device(RHIAdapter* pAdapter, const RHIDeviceCreateDesc& deviceDesc) 
        {
            cyber_core_assert(false, "Empty implement rhi_create_device!");
            return nullptr;
        }
        virtual void rhi_free_device(RHIDevice* pDevice) 
        {
            cyber_core_assert(false, "Empty implement rhi_free_device!");
        }
        // API Object APIs
        virtual RHISurface* rhi_surface_from_hwnd(RHIDevice* pDevice, HWND hwnd)
        {
            cyber_core_assert(false, "Empty implement rhi_surface_from_hwnd!");
            return nullptr;
        }
        virtual void rhi_free_surface(RHISurface* pSurface)
        {
            cyber_core_assert(false, "Empty implement rhi_free_surface!");
        }
        virtual RHIFence* rhi_create_fence(RHIDevice* pDevice) 
        {
            cyber_core_assert(false, "Empty implement rhi_create_fence!");
            return nullptr;
        };
        virtual void rhi_wait_fences(RHIFence** fences, uint32_t fenceCount)
        {
            cyber_core_assert(false, "Empty implement rhi_wait_fences!");
        }
        virtual void rhi_free_fence(RHIFence* fence)
        {
            cyber_core_assert(false, "Empty implement rhi_free_fence!");
        }
        virtual ERHIFenceStatus rhi_query_fence_status(RHIFence* pFence)
        {
            cyber_core_assert(false, "Empty implement rhi_query_fence_status!");
            return RHI_FENCE_STATUS_NOTSUBMITTED;
        }
        virtual RHISwapChain* rhi_create_swap_chain(RHIDevice* pDevice, const RHISwapChainCreateDesc& swapchainDesc)
        {
            cyber_core_assert(false, "Empty implement rhi_create_swap_chain!");
            return nullptr;
        }
        virtual void rhi_free_swap_chain(RHISwapChain* pSwapChain)
        {
            cyber_core_assert(false, "Empty implement rhi_free_swap_chain!");
        }
        virtual void rhi_enum_adapters(RHIInstance* instance, RHIAdapter** adapters, uint32_t* adapterCount)
        {
            cyber_core_assert(false, "Empty implement rhi_enum_adapters!");
        }
        virtual uint32_t rhi_acquire_next_image(RHISwapChain* pSwapChain, const RHIAcquireNextDesc& acquireDesc)
        {
            cyber_core_assert(false, "Empty implement rhi_acquire_next_image!");
            return 0;
        }
        // Queue APIs
        virtual RHIQueue* rhi_get_queue(RHIDevice* pDevice, ERHIQueueType type, uint32_t index) 
        { 
            cyber_core_assert(false, "Empty implement rhi_get_queue!");
            return nullptr;
        }
        virtual void rhi_submit_queue(RHIQueue* queue, const RHIQueueSubmitDesc& submitDesc)
        {
            cyber_core_assert(false, "Empty implement rhi_submit_queue!");
        }
        virtual void rhi_present_queue(RHIQueue* queue, const RHIQueuePresentDesc& presentDesc)
        {
            cyber_core_assert(false, "Empty implement rhi_present_queue!");
        }
        virtual void rhi_wait_queue_idle(RHIQueue* queue)
        {
            cyber_core_assert(false, "Empty implement rhi_wait_queue_idle!");
        }
        virtual void rhi_free_queue(RHIQueue* queue)
        {
            cyber_core_assert(false, "Empty implement rhi_free_queue!");
        }
        // Command APIs
        virtual RHICommandPool* rhi_create_command_pool(RHIQueue* pQueue, const CommandPoolCreateDesc& commandPoolDesc)
        {
            cyber_core_assert(false, "Empty implement rhi_create_command_pool!");
            return nullptr;
        }
        virtual void rhi_reset_command_pool(RHICommandPool* pPool)
        {
            cyber_core_assert(false, "Empty implement rhi_reset_command_pool!");
        }
        virtual void rhi_free_command_pool(RHICommandPool* pPool)
        {
            cyber_core_assert(false, "Empty implement rhi_free_command_pool!");
        }
        virtual RHICommandBuffer* rhi_create_command_buffer(RHICommandPool* pPool, const CommandBufferCreateDesc& commandBufferDesc)
        {
            cyber_core_assert(false, "Empty implement rhi_create_command_buffer!");
            return nullptr;
        }
        virtual void rhi_free_command_buffer(RHICommandBuffer* pCommandBuffer)
        {
            cyber_core_assert(false, "Empty implement rhi_free_command_buffer!");
        }
        /// RootSignature
        virtual RHIRootSignature* rhi_create_root_signature(RHIDevice* pDevice, const RHIRootSignatureCreateDesc& rootSigDesc)
        {
            cyber_core_assert(false, "Empty implement rhi_create_root_signature!");
            return nullptr;
        }
        virtual void rhi_free_root_signature(RHIRootSignature* pRootSignature)
        {
            cyber_core_assert(false, "Empty implement rhi_free_root_signature!");
        }
        virtual RHIDescriptorSet* rhi_create_descriptor_set(RHIDevice* pDevice, const RHIDescriptorSetCreateDesc& dSetDesc)
        {
            cyber_core_assert(false, "Empty implement rhi_create_descriptor_set!");
            return nullptr;
        }
        virtual void rhi_update_descriptor_set(RHIDescriptorSet* set, const RHIDescriptorData* updateDesc, uint32_t count)
        {
            cyber_core_assert(false, "Empty implement rhi_update_descriptor_set!");
        }
        virtual RHIRenderPipeline* rhi_create_render_pipeline(RHIDevice* pDevice, const RHIRenderPipelineCreateDesc& pipelineDesc)
        {
            cyber_core_assert(false, "Empty implement rhi_create_render_pipeline!");
            return nullptr;
        }
        virtual void rhi_free_render_pipeline(RHIRenderPipeline* pipeline)
        {
            cyber_core_assert(false, "Empty implement rhi_free_render_pipeline!");
        }

        // Resource APIs
        virtual RHITextureView* rhi_create_texture_view(RHIDevice* pDevice, const TextureViewCreateDesc& viewDesc)
        {
            cyber_core_assert(false, "Empty implement rhi_create_texture_view!");
            return nullptr;
        }
        virtual void rhi_free_texture_view(RHITextureView* view)
        {
            cyber_core_assert(false, "Empty implement rhi_free_texture_view!");
        }
        virtual RHITexture2D* rhi_create_texture(RHIDevice* pDevice, const TextureCreateDesc& textureDesc) 
        {
            cyber_core_assert(false, "Empty implement rhi_create_texture!");
            return nullptr;
        }
        virtual void rhi_free_texture(RHITexture* texture)
        {
            cyber_core_assert(false, "Empty implement rhi_free_texture!");
        }
        virtual RHIBuffer* rhi_create_buffer(RHIDevice* pDevice, const BufferCreateDesc& bufferDesc) 
        {
            cyber_core_assert(false, "Empty implement rhi_create_texture!");
            return nullptr;
        }
        virtual void rhi_free_buffer(RHIBuffer* buffer)
        {
            cyber_core_assert(false, "Empty implement rhi_free_buffer!");
        }
        virtual void rhi_map_buffer(RHIBuffer* buffer, const RHIBufferRange* range)
        {
            cyber_core_assert(false, "Empty implement rhi_map_buffer!");
        }
        virtual void rhi_unmap_buffer(RHIBuffer* buffer)
        {
            cyber_core_assert(false, "Empty implement rhi_unmap_buffer!");
        }

        // Shader
        virtual RHIShaderLibrary* rhi_create_shader_library(RHIDevice* device, const struct RHIShaderLibraryCreateDesc& desc)
        {
            cyber_core_assert(false, "Empty implement rhi_create_shader_library!");
            return nullptr;
        }
        virtual void rhi_free_shader_library(RHIShaderLibrary* shaderLibrary)
        {
            cyber_core_assert(false, "Empty implement rhi_free_shader_library!");
        }

        /// CMDS
        virtual void rhi_cmd_begin(RHICommandBuffer* cmd)
        {
            cyber_core_assert(false, "Empty implement rhi_cmd_begin!");
        }
        virtual void rhi_cmd_end(RHICommandBuffer* cmd)
        {
            cyber_core_assert(false, "Empty implement rhi_cmd_end!");
        }
        virtual void rhi_cmd_resource_barrier(RHICommandBuffer* cmd, const RHIResourceBarrierDesc& barrierDesc)
        {
            cyber_core_assert(false, "Empty implement rhi_cmd_resource_barrier!");
        }
        // Render Pass
        virtual RHIRenderPassEncoder* rhi_cmd_begin_render_pass(RHICommandBuffer* cmd, const RHIRenderPassDesc& beginRenderPassDesc)
        {
            cyber_core_assert(false, "Empty implement rhi_cmd_begin_renderpass!");
            return nullptr;
        }
        virtual void rhi_cmd_end_render_pass(RHICommandBuffer* cmd)
        {
            cyber_core_assert(false, "Empty implement rhi_cmd_end_renderpass!");
        }
        virtual void rhi_render_encoder_bind_descriptor_set(RHIRenderPassEncoder* encoder, RHIDescriptorSet* descriptorSet)
        {
            cyber_core_assert(false, "Empty implement rhi_cmd_encoder_bind_descriptor_set!");
        }
        virtual void rhi_render_encoder_set_viewport(RHIRenderPassEncoder* encoder, float x, float y, float width, float height, float min_depth, float max_depth)
        {
            cyber_core_assert(false, "Empty implement rhi_cmd_encoder_set_viewport!");
        }
        virtual void rhi_render_encoder_set_scissor(RHIRenderPassEncoder* encoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
        {
            cyber_core_assert(false, "Empty implement rhi_cmd_encoder_set_scissor!");
        }
        virtual void rhi_render_encoder_bind_pipeline(RHIRenderPassEncoder* encoder, RHIRenderPipeline* pipeline)
        {
            cyber_core_assert(false, "Empty implement rhi_cmd_encoder_bind_pipeline!");
        }
        virtual void rhi_render_encoder_bind_vertex_buffer(RHIRenderPassEncoder* encoder, uint32_t buffer_count, RHIBuffer** buffers,const uint32_t* strides, const uint32_t* offsets)
        {
            cyber_core_assert(false, "Empty implement rhi_cmd_encoder_bind_vertex_buffer!");
        }
        virtual void rhi_render_encoder_bind_index_buffer(RHIRenderPassEncoder* encoder, RHIBuffer* buffer, uint32_t index_stride, uint64_t offset)
        {
            cyber_core_assert(false, "Empty implement rhi_cmd_encoder_bind_index_buffer!");
        }
        virtual void rhi_render_encoder_push_constants(RHIRenderPassEncoder* encoder, RHIRootSignature* rs, const char8_t* name, const void* data)
        {
            cyber_core_assert(false, "Empty implement rhi_cmd_encoder_push_constants!");
        }
        virtual void rhi_render_encoder_draw(RHIRenderPassEncoder* encoder, uint32_t vertex_count, uint32_t first_vertex)
        {
            cyber_core_assert(false, "Empty implement rhi_cmd_encoder_draw!");
        }
        virtual void rhi_render_encoder_draw_instanced(RHIRenderPassEncoder* encoder, uint32_t vertex_count, uint32_t first_vertex, uint32_t instance_count, uint32_t first_instance)
        {
            cyber_core_assert(false, "Empty implement rhi_cmd_encoder_draw_instanced!");
        }
        virtual void rhi_render_encoder_draw_indexed(RHIRenderPassEncoder* encoder, uint32_t index_count, uint32_t first_index, uint32_t first_vertex)
        {
            cyber_core_assert(false, "Empty implement rhi_cmd_encoder_draw_indexed!");
        }
        virtual void rhi_render_encoder_draw_indexed_instanced(RHIRenderPassEncoder* encoder, uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_instance, uint32_t first_vertex)
        {
            cyber_core_assert(false, "Empty implement rhi_cmd_encoder_draw_indexed_instanced!");
        }
    };
    #define RHI_SINGLE_GPU_NODE_COUNT 1
    #define RHI_SINGLE_GPU_NODE_MASK 1
    #define RHI_SINGLE_GPU_NODE_INDEX 0

    CYBER_FORCE_INLINE RHIInstance* rhi_create_instance(const RHIInstanceCreateDesc& instanceDesc) 
    {
        return RHI::gloablRHI->rhi_create_instance(instanceDesc);
    }
    CYBER_FORCE_INLINE void rhi_free_instance(RHIInstance* instance)
    {
        RHI::gloablRHI->rhi_free_instance(instance);
    }
    // Device APIS
    CYBER_FORCE_INLINE RHIDevice* rhi_create_device(RHIAdapter* adapter, const RHIDeviceCreateDesc& deviceDesc) 
    {
        return RHI::gloablRHI->rhi_create_device(adapter, deviceDesc);
    }
    CYBER_FORCE_INLINE void rhi_free_device(RHIDevice* device) 
    {
        RHI::gloablRHI->rhi_free_device(device);
    }
    // API Object APIs
    CYBER_FORCE_INLINE RHISurface* rhi_surface_from_hwnd(RHIDevice* device, HWND hwnd)
    {
        return RHI::gloablRHI->rhi_surface_from_hwnd(device, hwnd);
    }
    CYBER_FORCE_INLINE void rhi_free_surface(RHISurface* surface)
    {
        RHI::gloablRHI->rhi_free_surface(surface);
    }
    CYBER_FORCE_INLINE RHIFence* rhi_create_fence(RHIDevice* device) 
    {
        return RHI::gloablRHI->rhi_create_fence(device);
    };
    CYBER_FORCE_INLINE void rhi_wait_fences(RHIFence** fences, uint32_t fenceCount)
    {
        RHI::gloablRHI->rhi_wait_fences(fences, fenceCount);
    }
    CYBER_FORCE_INLINE void rhi_free_fence(RHIFence* fence)
    {
        RHI::gloablRHI->rhi_free_fence(fence);
    }
    CYBER_FORCE_INLINE ERHIFenceStatus rhi_query_fence_status(RHIFence* fence)
    {
        return RHI::gloablRHI->rhi_query_fence_status(fence);
    }
    CYBER_FORCE_INLINE RHISwapChain* rhi_create_swap_chain(RHIDevice* device, const RHISwapChainCreateDesc& swapchainDesc)
    {
        return RHI::gloablRHI->rhi_create_swap_chain(device, swapchainDesc);
    }
    CYBER_FORCE_INLINE void rhi_free_swap_chain(RHISwapChain* swapchain)
    {
        RHI::gloablRHI->rhi_free_swap_chain(swapchain);
    }
    CYBER_FORCE_INLINE void rhi_enum_adapters(RHIInstance* instance, RHIAdapter** adapters, uint32_t* adapterCount)
    {
        RHI::gloablRHI->rhi_enum_adapters(instance, adapters, adapterCount);
    }
    CYBER_FORCE_INLINE uint32_t rhi_acquire_next_image(RHISwapChain* swapchain, const RHIAcquireNextDesc& acquireDesc)
    {
        return RHI::gloablRHI->rhi_acquire_next_image(swapchain, acquireDesc);
    }
    // Queue APIs
    CYBER_FORCE_INLINE RHIQueue* rhi_get_queue(RHIDevice* device, ERHIQueueType type, uint32_t index) 
    { 
        return RHI::gloablRHI->rhi_get_queue(device, type, index); 
    }
    CYBER_FORCE_INLINE void rhi_submit_queue(RHIQueue* queue, const RHIQueueSubmitDesc& submitDesc)
    {
        RHI::gloablRHI->rhi_submit_queue(queue, submitDesc);
    }
    CYBER_FORCE_INLINE void rhi_present_queue(RHIQueue* queue, const RHIQueuePresentDesc& presentDesc)
    {
        RHI::gloablRHI->rhi_present_queue(queue, presentDesc);
    }
    CYBER_FORCE_INLINE void rhi_wait_queue_idle(RHIQueue* queue)
    {
        RHI::gloablRHI->rhi_wait_queue_idle(queue);
    }
    CYBER_FORCE_INLINE void rhi_free_queue(RHIQueue* queue)
    {
        RHI::gloablRHI->rhi_free_queue(queue);
    }
    // Command APIs
    CYBER_FORCE_INLINE RHICommandPool* rhi_create_command_pool(RHIQueue* queue, const CommandPoolCreateDesc& commandPoolDesc)
    {
        return RHI::gloablRHI->rhi_create_command_pool(queue, commandPoolDesc);
    }
    CYBER_FORCE_INLINE void rhi_reset_command_pool(RHICommandPool* pool)
    {
        RHI::gloablRHI->rhi_reset_command_pool(pool);
    }
    CYBER_FORCE_INLINE void rhi_free_command_pool(RHICommandPool* pool)
    {
        RHI::gloablRHI->rhi_free_command_pool(pool);
    }
    CYBER_FORCE_INLINE RHICommandBuffer* rhi_create_command_buffer(RHICommandPool* pool, const CommandBufferCreateDesc& commandBufferDesc)
    {
        return RHI::gloablRHI->rhi_create_command_buffer(pool, commandBufferDesc);
    }
    CYBER_FORCE_INLINE void rhi_free_command_buffer(RHICommandBuffer* commandBuffer)
    {
        RHI::gloablRHI->rhi_free_command_buffer(commandBuffer);
    }
    /// RootSignature
    CYBER_FORCE_INLINE RHIRootSignature* rhi_create_root_signature(RHIDevice* device, const RHIRootSignatureCreateDesc& rootSigDesc)
    {
        return RHI::gloablRHI->rhi_create_root_signature(device, rootSigDesc);
    }
    CYBER_FORCE_INLINE void rhi_free_root_signature(RHIRootSignature* rootSignature)
    {
        RHI::gloablRHI->rhi_free_root_signature(rootSignature);
    }
    CYBER_FORCE_INLINE RHIDescriptorSet* rhi_create_descriptor_set(RHIDevice* device, const RHIDescriptorSetCreateDesc& descriptorSetDesc)
    {
        return RHI::gloablRHI->rhi_create_descriptor_set(device, descriptorSetDesc);
    }
    CYBER_FORCE_INLINE void rhi_update_descriptor_set(RHIDescriptorSet* set, const RHIDescriptorData* updateDesc, uint32_t count)
    {
        RHI::gloablRHI->rhi_update_descriptor_set(set, updateDesc, count);
    }
    CYBER_FORCE_INLINE RHIRenderPipeline* rhi_create_render_pipeline(RHIDevice* device, const RHIRenderPipelineCreateDesc& pipelineDesc)
    {
        return RHI::gloablRHI->rhi_create_render_pipeline(device, pipelineDesc);
    }
    CYBER_FORCE_INLINE void rhi_free_render_pipeline(RHIRenderPipeline* pipeline)
    {
        RHI::gloablRHI->rhi_free_render_pipeline(pipeline);
    }
    // Resource APIs
    CYBER_FORCE_INLINE RHITextureView* rhi_create_texture_view(RHIDevice* device, const TextureViewCreateDesc& viewDesc)
    {
        return RHI::gloablRHI->rhi_create_texture_view(device, viewDesc);
    }
    CYBER_FORCE_INLINE void rhi_free_texture_view(RHITextureView* view)
    {
        RHI::gloablRHI->rhi_free_texture_view(view);
    }
    CYBER_FORCE_INLINE RHITexture2D* rhi_create_texture(RHIDevice* device, const TextureCreateDesc& textureDesc) 
    {
        return RHI::gloablRHI->rhi_create_texture(device, textureDesc);
    }
    CYBER_FORCE_INLINE RHIBuffer* rhi_create_buffer(RHIDevice* device, const BufferCreateDesc& bufferDesc) 
    {
        return RHI::gloablRHI->rhi_create_buffer(device, bufferDesc);
    }
    CYBER_FORCE_INLINE void rhi_free_buffer(RHIBuffer* buffer)
    {
        RHI::gloablRHI->rhi_free_buffer(buffer);
    }
    CYBER_FORCE_INLINE void rhi_map_buffer(RHIBuffer* buffer, const RHIBufferRange* range)
    {
        RHI::gloablRHI->rhi_map_buffer(buffer, range);
    }
    CYBER_FORCE_INLINE void rhi_unmap_buffer(RHIBuffer* buffer)
    {
        RHI::gloablRHI->rhi_unmap_buffer(buffer);
    }
    // Shader
    CYBER_FORCE_INLINE RHIShaderLibrary* rhi_create_shader_library(RHIDevice* device, const struct RHIShaderLibraryCreateDesc& desc)
    {
        return RHI::gloablRHI->rhi_create_shader_library(device, desc);
    }
    CYBER_FORCE_INLINE void rhi_free_shader_library(RHIShaderLibrary* shaderLibrary)
    {
        RHI::gloablRHI->rhi_free_shader_library(shaderLibrary);
    }
    /// CMDS
    CYBER_FORCE_INLINE void rhi_cmd_begin(RHICommandBuffer* cmd)
    {
        RHI::gloablRHI->rhi_cmd_begin(cmd);
    }
    CYBER_FORCE_INLINE void rhi_cmd_end(RHICommandBuffer* cmd)
    {
        RHI::gloablRHI->rhi_cmd_end(cmd);
    }
    CYBER_FORCE_INLINE void rhi_cmd_resource_barrier(RHICommandBuffer* cmd, const RHIResourceBarrierDesc& barrierDesc)
    {
        RHI::gloablRHI->rhi_cmd_resource_barrier(cmd, barrierDesc);
    }
    // Render Pass
    CYBER_FORCE_INLINE RHIRenderPassEncoder* rhi_cmd_begin_render_pass(RHICommandBuffer* cmd, const RHIRenderPassDesc& beginRenderPassDesc)
    {
        return RHI::gloablRHI->rhi_cmd_begin_render_pass(cmd, beginRenderPassDesc);
    }
    CYBER_FORCE_INLINE void rhi_cmd_end_render_pass(RHICommandBuffer* cmd)
    {
        RHI::gloablRHI->rhi_cmd_end_render_pass(cmd);
    }
    CYBER_FORCE_INLINE void rhi_render_encoder_bind_descriptor_set(RHIRenderPassEncoder* encoder, RHIDescriptorSet* descriptorSet)
    {
        RHI::gloablRHI->rhi_render_encoder_bind_descriptor_set(encoder, descriptorSet);
    }
    CYBER_FORCE_INLINE void rhi_render_encoder_set_viewport(RHIRenderPassEncoder* encoder, float x, float y, float width, float height, float min_depth, float max_depth)
    {
        RHI::gloablRHI->rhi_render_encoder_set_viewport(encoder, x, y, width, height, min_depth, max_depth);
    }
    CYBER_FORCE_INLINE void rhi_render_encoder_set_scissor(RHIRenderPassEncoder* encoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        RHI::gloablRHI->rhi_render_encoder_set_scissor(encoder, x, y, width, height);
    }
    CYBER_FORCE_INLINE void rhi_render_encoder_bind_pipeline(RHIRenderPassEncoder* encoder, RHIRenderPipeline* pipeline)
    {
        RHI::gloablRHI->rhi_render_encoder_bind_pipeline(encoder, pipeline);
    }
    CYBER_FORCE_INLINE void rhi_render_encoder_bind_vertex_buffer(RHIRenderPassEncoder* encoder, uint32_t buffer_count, RHIBuffer** buffers,const uint32_t* strides, const uint32_t* offsets)
    {
        RHI::gloablRHI->rhi_render_encoder_bind_vertex_buffer(encoder, buffer_count, buffers, strides, offsets);
    }
    CYBER_FORCE_INLINE void rhi_render_encoder_bind_index_buffer(RHIRenderPassEncoder* encoder, RHIBuffer* buffer, uint32_t index_stride, uint64_t offset)
    {
        RHI::gloablRHI->rhi_render_encoder_bind_index_buffer(encoder, buffer, index_stride, offset);
    }
    CYBER_FORCE_INLINE void rhi_render_encoder_push_constants(RHIRenderPassEncoder* encoder, RHIRootSignature* rs, const char8_t* name, const void* data)
    {
        RHI::gloablRHI->rhi_render_encoder_push_constants(encoder, rs, name, data);
    }
    CYBER_FORCE_INLINE void rhi_render_encoder_draw(RHIRenderPassEncoder* encoder, uint32_t vertex_count, uint32_t first_vertex)
    {
        RHI::gloablRHI->rhi_render_encoder_draw(encoder, vertex_count, first_vertex);
    }
    CYBER_FORCE_INLINE void rhi_render_encoder_draw_instanced(RHIRenderPassEncoder* encoder, uint32_t vertex_count, uint32_t first_vertex, uint32_t instance_count, uint32_t first_instance)
    {
        RHI::gloablRHI->rhi_render_encoder_draw_instanced(encoder, vertex_count, first_vertex, instance_count, first_instance);
    }
    CYBER_FORCE_INLINE void rhi_render_encoder_draw_indexed(RHIRenderPassEncoder* encoder, uint32_t index_count, uint32_t first_index, uint32_t first_vertex)
    {
        RHI::gloablRHI->rhi_render_encoder_draw_indexed(encoder, index_count, first_index, first_vertex);
    }
    CYBER_FORCE_INLINE void rhi_render_encoder_draw_indexed_instanced(RHIRenderPassEncoder* encoder, uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_instance, uint32_t first_vertex)
    {
        RHI::gloablRHI->rhi_render_encoder_draw_indexed_instanced(encoder, index_count, first_index, instance_count, first_instance, first_vertex);
    } 
}
