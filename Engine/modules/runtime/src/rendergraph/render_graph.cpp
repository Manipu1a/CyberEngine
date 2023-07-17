#include "rendergraph/render_graph.h"
#include "platform/memory.h"

namespace Cyber
{
    namespace render_graph
    {
        RenderGraph* RenderGraph::create(const RenderGraphSetupFunction& setup) CYBER_NOEXCEPT
        {
            RenderGraphBuilder* graphBuilder = cyber_new<RenderGraphBuilder>();
            setup(*graphBuilder);

            RenderGraph* graph = cyber_new<RenderGraph>();
            graph->graphBuilder = graphBuilder;
            return graph;
        }

        void RenderGraph::destroy(RenderGraph* graph) CYBER_NOEXCEPT
        {
            cyber_delete(graph->graphBuilder);
            cyber_delete(graph);
        }
    }
}