#pragma once
#include "EASTL/vector.h"
#include "graphics/interface/graphics_types.h"

namespace Cyber
{
    namespace render_graph
    {
        class RenderGraph;
        class RGPass;
        class RGRenderResource;

        typedef enum ERGPassType
        {
            RG_RENDER_PASS,
            RG_COMPUTE_PASS,
            RG_PRESENT_PASS
        } ERGPassType;


        typedef enum ERGObjectType
        {
            RG_RENDER_PASS_NODE,
            RG_COMPUTE_PASS_NODE,
            RG_PRESENT_PASS_NODE,
            RG_BUFFER_NODE,
            RG_TEXTURE_NODE,

        } ERGObjectType;

        enum class ERGResourceAccess : uint8_t
        {
            Read = 1,
            Write = 2,
            ReadWrite = 3
        };

        struct PassResourceAccess
        {
            RGRenderResource* resource = nullptr;
            ERGResourceAccess access = ERGResourceAccess::Read;
        };

        struct Utf8StringLess
        {
            bool operator()(const char8_t* lhs, const char8_t* rhs) const CYBER_NOEXCEPT
            {
                if (lhs == rhs)
                    return false;
                if (!lhs)
                    return rhs != nullptr;
                if (!rhs)
                    return false;

                while (*lhs != 0 && *rhs != 0 && *lhs == *rhs)
                {
                    ++lhs;
                    ++rhs;
                }
                return *lhs < *rhs;
            }
        };

        struct RenderGraphNode
        {
            explicit RenderGraphNode(ERGObjectType type)
                : object_type(type)
                , order(UINT32_MAX)
            {
            }

            ERGObjectType object_type;
            uint32_t order;
        };

        struct PassNode : public RenderGraphNode
        {
            using PassDestroyFunction = void(*)(RGPass*);

            PassNode(ERGObjectType type, ERGPassType type_of_pass)
                : RenderGraphNode(type)
                , pass_type(type_of_pass)
                , pass_handle(nullptr)
                , destroy_pass(nullptr)
            {
            }

            ERGPassType pass_type;
            RGPass* pass_handle;
            PassDestroyFunction destroy_pass;
            eastl::vector<PassResourceAccess> resource_accesses;
            eastl::vector<PassNode*> dependencies;
        };

        struct RenderPassContext
        {
            RenderGraph* graph = nullptr;
            RGPass* pass = nullptr;
            RenderPassEncoder* encoder = nullptr;
            uint32_t frame_index = 0;
        };
    }
}
