#pragma once
#include "graphics/interface/graphics_types.h"

namespace Cyber
{
    namespace render_graph
    {
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

        struct DependencyGraph
        {
            
        };

        class RenderGraphEdge;

        struct RenderGraphNode
        {
            RenderGraphNode(ERGObjectType type) : object_type(type) {}

            eastl::vector<RenderGraphEdge*> read_edges;
            eastl::vector<RenderGraphEdge*> write_edges;
            ERGObjectType object_type;
            DependencyGraph* graph;
            uint32_t order;
        };

        struct PassNode : public RenderGraphNode
        {
            PassNode(ERGObjectType type) : RenderGraphNode(type) {}

            ERGPassType pass_type;
            class RGPass* pass_handle;
        };

        struct RenderGraphEdge
        {
            RenderGraphNode* to() const { return to_node; }
            RenderGraphNode* from() const { return from_node; }

            RenderGraphNode* from_node;
            RenderGraphNode* to_node;

            DependencyGraph* graph;
        };

        struct RenderPassContext
        {
        public:
            RenderPassEncoder* encoder;
        };
    }
}
