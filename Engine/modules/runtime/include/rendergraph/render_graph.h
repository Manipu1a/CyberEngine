#pragma once

#include "render_graph_flag.h"
#include "render_graph_builder.h"
#include "platform/configure.h"

namespace Cyber
{
    namespace render_graph
    {
        class CYBER_RUNTIME_API RenderGraph
        {
        public:
            RenderGraph() = default;
            ~RenderGraph() = default;

            using RenderGraphSetupFunction = eastl::function<void(RenderGraphBuilder&)>;
            static RenderGraph* create(const RenderGraphSetupFunction& setup) CYBER_NOEXCEPT;
            static void destroy(RenderGraph* graph) CYBER_NOEXCEPT;
        private:
            class RenderGraphBuilder* graphBuilder;
        };
    }
}
