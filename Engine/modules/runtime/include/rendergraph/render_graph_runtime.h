#pragma once

#include "render_graph_flag.h"
#include "rhi/rhi.h"

typedef uint32_t RenderGraphResourceId;

typedef enum ERenderGraphResourceFlag
{
    RG_RESOURCE_FLAG_NONE                             = 0,         ///< No special properties.
    RG_RESOURCE_FLAG_CUBEMAP_COMPATIBLE_BIT           = (1 << 1),  ///< Supports cubemap views.
    RG_RESOURCE_FLAG_ROWMAJOR_IMAGE_BIT               = (1 << 2),  ///< Uses rowmajor image layout.
    RG_RESOURCE_FLAG_PREFER_GPU_LOCAL_CPU_VISIBLE_BIT = (1 << 3),  ///< Preferred to be in GPU-local CPU-visible heap
                                                                    ///  if available.
    RG_RESOURCE_FLAG_PREFER_DEDICATED_ALLOCATION_BIT = (1 << 4),   ///< Preferred to be in dedicated allocation or as
                                                                    ///  committed resource.
    RG_RESOURCE_FLAG_PERSISTENT_BIT = (1 << 15),                   ///< Resource data is persistent from frame to
                                                                    ///  frame.
} ERenderGraphResourceFlag;
typedef uint32_t ERenderGraphResourceFlags;

typedef enum ERenderGraphScheduleFlags
{
    /// No schedule flag bits are specified. Default options are used. When used as
    /// RpsRenderGraphUpdateInfo::scheduleFlags, the RpsRenderGraphCreateInfo::scheduleInfo::scheduleFlags specified
    /// at render graph creation time are used instead.
    RG_SCHEDULE_UNSPECIFIED = 0,

    /// Command nodes are kept in the program order.
    RG_SCHEDULE_KEEP_PROGRAM_ORDER_BIT = (1 << 0),

    /// Schedules in favor of reducing total GPU memory usage. Possible strategies include minimizing transient resource
    /// lifetimes and aggressive aliasing. This may increase the number of barriers generated.
    RG_SCHEDULE_PREFER_MEMORY_SAVING_BIT = (1 << 1),

    /// Schedules commands randomly (without changing program logic). Mostly useful for testing purposes. Applications
    /// should normally avoid using this flag for end-user scenarios. If RPS_SCHEDULE_KEEP_PROGRAM_ORDER_BIT is set,
    /// this flag will have no effect.
    RG_SCHEDULE_RANDOM_ORDER_BIT = (1 << 2),

    /// Avoids alternating between graphics and compute work on the same queue. This can help for some architectures
    /// where switching between graphics and compute produces extra overhead.
    RG_SCHEDULE_MINIMIZE_COMPUTE_GFX_SWITCH_BIT = (1 << 3),

    /// Disables dead code elimination optimization. By default, RPS removes nodes that have no visible effect (Not
    /// contributing to modification of external, temporal, persistent or CPU resources). This flag disables this
    /// optimization.
    RG_SCHEDULE_DISABLE_DEAD_CODE_ELIMINATION_BIT = (1 << 4),

    /// Disables work pipelining based on the workload type.
    RG_SCHEDULE_WORKLOAD_TYPE_PIPELINING_DISABLE_BIT = (1 << 5),

    /// Performs aggressive work pipelining based on the workload type. If
    /// RPS_SCHEDULE_WORKLOAD_TYPE_PIPELINING_DISABLE_BIT is set, this flag will have not effect.
    RG_SCHEDULE_WORKLOAD_TYPE_PIPELINING_AGGRESSIVE_BIT = (1 << 6),

    // Reserved for future use:

    /// Reserved for future use. Includes split barriers where appropriate.
    RG_SCHEDULE_ALLOW_SPLIT_BARRIERS_BIT = (1 << 16),

    /// Reserved for future use. Avoids rescheduling if possible and uses the existing schedule instead.
    RG_SCHEDULE_AVOID_RESCHEDULE_BIT = (1 << 17),

    /// Reserved for future use. Allows work to overlap between multiple frames.
    RG_SCHEDULE_ALLOW_FRAME_OVERLAP_BIT = (1 << 21),

    /// Reserved for future use. Tries to use render pass transitions instead of standalone transition nodes when
    /// possible. If RPS_SCHEDULE_DISABLE_RENDERPASS_TRANSITIONS_BIT is set, this flag will have no effect.
    RG_SCHEDULE_PREFER_RENDERPASS_TRANSITIONS_BIT = (1 << 22),

    /// Reserved for future use. Uses standalone transition nodes instead of render pass transitions.
    RG_SCHEDULE_DISABLE_RENDERPASS_TRANSITIONS_BIT = (1 << 23),

    // End reserved for future use. 

    /// Uses default options. This is identical to RPS_SCHEDULE_UNSPECIFIED in most cases, except when used as
    /// RpsRenderGraphUpdateInfo::scheduleFlags, instead using the default options regardless of
    /// RpsRenderGraphCreateInfo::scheduleInfo::scheduleFlags. This default behavior is a baseline set of criteria used
    /// for scheduling to which these flags can add additional ones.
    RG_SCHEDULE_DEFAULT = (1 << 30),

    /// Prioritizes application performance over a lower memory footprint.
    RG_SCHEDULE_DEFAULT_PERFORMANCE = RG_SCHEDULE_DEFAULT,

    /// Prioritizes a lower memory footprint over performance.
    RG_SCHEDULE_DEFAULT_MEMORY = RG_SCHEDULE_PREFER_MEMORY_SAVING_BIT,

} ERenderGraphScheduleFlags;
typedef uint32_t RenderGraphScheduleFlags;

typedef enum ERenderGraphQueueFlag
{
    RG_QUEUE_FLAG_NONE = 0,             ///< No capabilities.
    RG_QUEUE_FLAG_GRAPHICS = (1 << 0),  ///< Graphics capabilities.
    RG_QUEUE_FLAG_COMPUTE = (1 << 1),   ///< Compute capabilities.
    RG_QUEUE_FLAG_COPY = (1 << 2),      ///< Copy capabilities.
} ERenderGraphQueueFlag;
typedef uint32_t ERenderGraphQueueFlags;

/// @brief Resource access attribute.
struct RenderGraphAccessAttr
{
    ERHIResourceStates accessFlags;    ///< Resource state.
    ERHIShaderStages accessStages;     ///< Shader stages that access the resource.
};
/// @brief Parameters for a resource description.
struct RenderGraphResourceDesc
{
    ERHIResourceType type;      ///< Resource type.
    uint32_t temporalLayers;    ///< Number of temporal layers the resource consists of.
    ERenderGraphResourceFlags flags;

    union
    {
        struct
        {
            uint32_t width;
            uint32_t height;
            union
            {
                uint32_t depth;         ///< Depth of a 3D image resource.
                uint32_t arrayLayers;   ///< Number of array layers for a non-3D image resource.
            };
            uint32_t mipLevels;         ///< Number of mipmap levels.
            ERHIFormat format;          ///< Platform independent format to be interpreted by the runtime.
            uint32_t sampleCount;       ///< Number of MSAA samples of an image.
        } image;
        struct
        {
            uint32_t sizeInBytesLo;     ///< Lower 32 bits of the size of a buffer resource in bytes.
            uint32_t sizeInByteHi;      ///< Higher 32 bits of the size of a buffer resource in bytes.
        } buffer;
    };
};

/// @brief Subsection of a resource from the graphics API perspective.
struct RenderGraphSubresourceRange
{
    uint16_t baseMipLevel;      ///< First mipmapping level accessible in the range.
    uint16_t mipLevels;         ///< Number of mipmap levels in the range.
    uint32_t baseArrayLayer;    ///< First layer accessible in the range.
    uint32_t arrayLayers;       ///< Number of array layers accessible in the range.
};

/// @brief Parameters of a command callback context.
struct RenderGraphCmdCallbackContext
{
    /// Handle to the command buffer for command recording.
    Cyber::RHICommandBuffer* pCommandBuffer;

    /// User context passed as RpsRenderGraphRecordCommandInfo::pUserContext. Can vary per rpsRenderGraphRecordCommands
    /// call and can e.g. be used as per-thread context if doing multi-threaded recording.
    void* pUserRecordContext;

    /// User context specified with the command node callback function, for example via a rpsProgramBindNode call. Can
    /// vary per callback.
    void* pCmdCallbackContext;

    /// Pointer to an array of <c><i>void* const</i></c> with numArgs pointers to arguments to use for the callback.
    /// Must not be NULL if numArgs != 0.
    void* const* ppArgs;

    /// Number of arguments defined for the callback.
    uint32_t numArgs;

    /// User defined tag for associations with a specific node. Can be set by passing a value to
    /// <c><i>rpsCmdCallNode</i></c>.
    uint32_t userTag;
};

/// @brief Parameters for accessing a resource.
struct RenderGraphResourceAccessInfo
{
    RenderGraphResourceId resourceId;  /// ID of the resource to access
    RenderGraphSubresourceRange range;

};

/// @brief Command callback with usage parameters.
struct RenderGraphCmdCallBack
{

};

/// @brief Parameters for describing a node call parameter.
struct RenderGraphParameterDesc
{

};

/// @brief Parameters for describing a render graph node.
struct RenderGraphNodeDesc
{

};

/// @brief Parameters for describing a render graph signature.
struct RenderGraphSignatureDesc
{
    uint32_t numParams;
    uint32_t numNodeDescs;
    uint32_t maxExternalResources;

    
};

/// Parameters for creating an RenderGraph program.
struct RenderGraphProgramCreateInfo
{

};

struct RenderGraphCreateInfo
{
    struct 
    {
        ERenderGraphScheduleFlags scheduleFlags;    ///< Flags for scheduling behavior.
        uint32_t numQueues;                         ///< Number of queues available to the render graph. If 0, RPS assumes there
                                                    ///  is 1 graphics queue.
        
        const ERenderGraphQueueFlags* pQueueInfos;  ///< Array of queue flags for each queue. Must not be NULL if numQueues > 0.
    } scheduleInfo;

    struct
    {
        uint32_t numHeaps;                  ///< Number of memory heaps available to the render graph.
        const uint32_t* heapBudgetMiBs;
    } memoryInfo;


};
