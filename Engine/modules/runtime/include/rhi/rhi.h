#pragma once
#include "cyber_rhi_config.h"
#include "flags.h"
#include <EASTL/vector.h>
#include <stdint.h>
#include "core/Core.h"
#include "core/Window.h"

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

    union ClearValue
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
    };
    
    class CYBER_RHI_API RHIAdapter
    {
    public:
        Cyber::Ref<RHIInstance> pInstance;
    };

    class CYBER_RHI_API RHIDevice
    {
    public:
        Cyber::Ref<RHIAdapter> pAdapter;
    };

    class CYBER_RHI_API RHIFence
    {
    public:
        Cyber::Ref<RHIDevice> pDevice;
    };

    class CYBER_RHI_API RHISemaphore
    {
    public:
        Cyber::Ref<RHIDevice> pDevice;
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
        Ref<RHIRootSignaturePool> pool;
        Ref<RHIRootSignature> pool_next;
    };
    
    struct CYBER_RHI_API RHIDescriptorSet
    {
        Ref<RHIRootSignature> root_signature;
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
        const char* name;
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
        const char* entry_name;
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

    /// Texture group
    struct CYBER_RHI_API TextureCreationDesc
    {
        /// Optimized clear value (recommended to use this same value when clearing the rendertarget)
        ClearValue mClearValue;
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
        const char* pName;
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
    };

    struct CYBER_RHI_API RHIInstanceCreateDesc
    {
        ERHIBackend mBackend;
        bool mEnableDebugLayer;
        bool mEnableGpuBasedValidation;
        bool mEnableSetName;
    };

    // Objects
    class CYBER_RHI_API RHIInstance
    {
        ERHIBackend mBackend;
        ERHINvAPI_Status mNvAPIStatus;
        ERHIAGSReturenCode mAgsStatus;
        bool mEnableSetName;
    };
    /// Device Group
    struct CYBER_RHI_API QueueGroupDesc
    {
        ERHIQueueType mQueueType;
        uint32_t mQueueCount;
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

    struct CYBER_RHI_API DeviceCreateDesc
    {
        bool bDisablePipelineCache;
        eastl::vector<QueueGroupDesc> mQueueGroupsDesc;
        uint32_t mQueueGroupCount;
    };

    class CYBER_RHI_API RHIQueue
    {
    public:
        Cyber::Ref<RHIDevice> pDevice;
        ERHIQueueType mType;
        RHIQueueIndex mIdex;
    };

    class CYBER_RHI_API RHICommandPool
    {
    public:
        Cyber::Ref<RHIQueue> pQueue;
    };

    class CYBER_RHI_API RHICommandBuffer
    {
    public:
        Cyber::Ref<RHIDevice> pDevice;
        Cyber::Ref<RHICommandPool> pPool;
        ERHIPipelineType mCurrentDispatch;
    };

    class CYBER_RHI_API RHIQueryPool
    {
    public:
        Cyber::Ref<RHIDevice> pDevice;
        uint32_t mCount;
    };

    class CYBER_RHI_API RHISwapChain
    {
    public:
        Ref<RHITexture*> mBackBuffers;
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
        Ref<RHICommandBuffer> pCmds;
        RHIFence mSignalFence;
        RHISemaphore* pWaitSemaphores;
        RHISemaphore* pSignalSemaphores;
        uint32_t mCmdsCount;
        uint32_t mWaitSemaphoreCount;
        uint32_t mSignalSemaphoreCount;
    };

    struct CYBER_RHI_API RHIQueueGroupDesc
    {
        ERHIQueueType mQueueType;
        uint32_t mQueueCount;
    };

    struct CYBER_RHI_API RHIQueuePresentDesc
    {
        
    };

    struct CYBER_RHI_API RHISwapChainCreateDesc
    {
        /// Window handle
        Cyber::WindowHandle mWindowHandle;
        /// Present Queues
        Ref<RHIQueue> mPresentQueue;
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
        /// Set whether swapchain will be presented using vsync
        bool mEnableVsync;
        /// We can toogle to using FLIP model if app desires
        bool mUseFlipSwapEffect;
    };

    struct CYBER_RHI_API RHIPipelineShaderCreateDesc
    {
        Ref<RHIShaderLibrary> library;
        const char8_t* entry;
        ERHIShaderStage stage;
    };

    struct CYBER_RHI_API RHIRootSignatureCreateDesc
    {
        struct RHIPipelineShaderCreateDesc* shaders;
        uint32_t shader_count;
        const RHISampler** static_samplers;
        const char8_t* const* static_sampler_names;
        uint32_t static_sampler_count;
        const char8_t* const* push_constant_names;
        uint32_t push_constant_count;
        Ref<RHIRootSignaturePool> pool;
    };
    
    struct CYBER_RHI_API RHIDescriptorSetCreateDesc
    {
        Ref<RHIRootSignature> root_signature;
        uint32_t set_index;
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
        Ref<RHIRootSignature> root_signature;
        Ref<RHIPipelineShaderCreateDesc> vertex_shader;
        Ref<RHIPipelineShaderCreateDesc> tesc_shader;
        Ref<RHIPipelineShaderCreateDesc> tese_shader;
        Ref<RHIPipelineShaderCreateDesc> geometry_shader;
        Ref<RHIPipelineShaderCreateDesc> fragment_shader;
        const RHIVertexLayout* vertex_layout;
        Ref<RHIBlendStateCreateDesc> blend_state;
        Ref<RHIDepthStateCreateDesc> depth_stencil_state;
        Ref<RHIRasterizerStateCreateDesc> rasterizer_state;

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
        inline static RHI& GetRHIContext()
        {
            cyber_assert(globalRHI, "RHI has not been initialized!");
            return *gloablRHI;
        }

    protected:
        static RHI* gloablRHI;

    public:
        // Instance APIs
        virtual InstanceRHIRef rhi_create_instance(Ref<RHIDevice> pDevice, const RHIInstanceCreateDesc& instanceDesc) 
        {
            cyber_core_assert(false, "Empty implement rhi_create_instance!");
            return CreateRef<RHIInstance>();
        }

        // Device APIS
        virtual void rhi_create_device(Ref<RHIDevice> pDevice, Ref<RHIAdapter> pAdapter, const DeviceCreateDesc& deviceDesc) {}

        // API Object APIs
        virtual FenceRHIRef rhi_create_fence(Ref<RHIDevice> pDevice) 
        {
            cyber_core_assert(false, "Empty implement rhi_create_fence!");
            return CreateRef<RHIFence>();
        };
        virtual void rhi_wait_fences(const RHIFence* pFences, uint32_t fenceCount) {}
        virtual void rhi_query_fence_status(Ref<RHIFence> pFence) {}
        virtual SwapChainRef rhi_create_swap_chain(Ref<RHIDevice> pDevice, const RHISwapChainCreateDesc& swapchainDesc)
        {
            cyber_core_assert(false, "Empty implement rhi_create_swap_chain!");
            return CreateRef<RHISwapChain>();
        }
        // Queue APIs
        virtual QueueRHIRef rhi_get_queue(Ref<RHIDevice> pDevice, ERHIQueueType type, uint32_t index) 
        { 
            cyber_core_assert(false, "Empty implement rhi_get_queue!");
            return CreateRef<RHIQueue>();
        }

        // Command APIs
        virtual CommandPoolRef rhi_create_command_pool(Ref<RHIQueue> pQueue, const CommandPoolCreateDesc& commandPoolDesc)
        {
            cyber_core_assert(false, "Empty implement rhi_create_command_pool!");
            return CreateRef<RHICommandPool>();
        }

        virtual CommandBufferRef rhi_create_command_buffer(Ref<RHICommandPool> pPool, const CommandBufferCreateDesc& commandBufferDesc)
        {
            cyber_core_assert(false, "Empty implement rhi_create_command_buffer!");
            return CreateRef<RHICommandBuffer>();
        }
        //virtual void rhi_submit_queue(Ref<RHIQueue> pQueue, const )

        /// RootSignature
        virtual RootSignatureRHIRef rhi_create_root_signature(Ref<RHIDevice> pDevice, const RHIRootSignatureCreateDesc& rootSigDesc)
        {
            cyber_core_assert(false, "Empty implement rhi_create_root_signature!");
            return CreateRef<RHIRootSignature>();
        }

        virtual DescriptorSetRHIRef rhi_create_descriptor_set(Ref<RHIDevice> pDevice, const RHIDescriptorSetCreateDesc& dSetDesc)
        {
            cyber_core_assert(false, "Empty implement rhi_create_descriptor_set!");
            return CreateRef<RHIDescriptorSet>();
        }

        virtual Ref<RHIRenderPipeline> rhi_create_render_pipeline(Ref<RHIDevice> pDevice, const RHIRenderPipelineCreateDesc& pipelineDesc)
        {
            cyber_core_assert(false, "Empty implement rhi_create_render_pipeline!");
            return CreateRef<RHIRenderPipeline>();
        }

        // Resource APIs
        virtual Texture2DRHIRef rhi_create_texture(Ref<RHIDevice> pDevice, const TextureCreationDesc& textureDesc) 
        {
            cyber_core_assert(false, "Empty implement rhi_create_texture!");
            return CreateRef<RHITexture2D>();
        }
        virtual void rhi_remove_texture() {}
        virtual BufferRHIRef rhi_create_buffer(Ref<RHIDevice> pDevice, const BufferCreateDesc& bufferDesc) 
        {
            cyber_core_assert(false, "Empty implement rhi_create_texture!");
            return CreateRef<RHIBuffer>();
        }

        virtual void rhi_remove_buffer() {}
        virtual void rhi_map_buffer() {}

        virtual void rhi_create_rendertarget() {}

        // Shader
        virtual ShaderLibraryRHIRef rhi_create_shader_library(Ref<RHIDevice> device, const struct RHIShaderLibraryCreateDesc& desc)
        {
            cyber_core_assert(false, "Empty implement rhi_create_shader_library!");
            return CreateRef<RHIShaderLibrary>();
        }
    };

    #define RHI_SINGLE_GPU_NODE_COUNT 1
    #define RHI_SINGLE_GPU_NODE_MASK 1
    #define RHI_SINGLE_GPU_NODE_INDEX 0

}
