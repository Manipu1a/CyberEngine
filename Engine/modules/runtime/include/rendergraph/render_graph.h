#pragma once

#include "render_graph_flag.h"
#include "render_graph_builder.h"
#include "render_graph_phase.h"
#include "platform/configure.h"

namespace Cyber
{
    namespace render_graph
    {
        class CYBER_RUNTIME_API RenderGraph
        {
        public:
            RenderGraph();
            ~RenderGraph();

            using RenderGraphSetupFunction = eastl::function<void(RenderGraphBuilder&)>;
            static RenderGraph* create(const RenderGraphSetupFunction& setup) CYBER_NOEXCEPT;
            static void destroy(RenderGraph* graph) CYBER_NOEXCEPT;
        public:
            CYBER_FORCE_INLINE RenderGraphBuilder* get_builder() const CYBER_NOEXCEPT { return graphBuilder; }
            void execute();

            template<typename Phase>
            void add_custom_phase();
        private:
            eastl::vector<class RenderGraphPhase*> phases;
            class RenderGraphBuilder* graphBuilder;
        };
    }
}
