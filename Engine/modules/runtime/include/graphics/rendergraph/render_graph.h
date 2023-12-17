#pragma once

#include "render_graph_flag.h"
//#include "render_graph_builder.h"
#include "render_graph_phase.h"
#include "platform/configure.h"
#include "cyber_runtime.config.h"
#include "eastl/functional.h"
#include "eastl/map.h"
#include "eastl/vector.h"
#include "graphics/interface/rhi.h"

#ifndef RG_MAX_FRAME_IN_FLIGHT
#define RG_MAX_FRAME_IN_FLIGHT 3
#endif

namespace Cyber
{
    namespace render_graph
    {
        class CYBER_RUNTIME_API RenderGraphFrameExecutor
        {
        public:
            RenderGraphFrameExecutor() = default;
            void initialize(class RHIQueue* queue, class RHIDevice* device);
            void finalize();

            class RHICommandPool* gfx_cmd_pool;
            class RHICommandBuffer* gfx_cmd_buffer;
        };

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
            void initialize();
            void execute();
            void execute_pass(class RGRenderPass* pass);
            template<typename Phase>
            void add_custom_phase();
        private:
            eastl::vector<class RenderGraphPhase*> phases;
            class RenderGraphBuilder* graphBuilder;
        public:
            eastl::map<const char8_t*, class RGRenderResource*> resource_map;
            eastl::vector<class ResourceNode*> resources;
            eastl::vector<class PassNode*> passes;
            eastl::vector<class ResourceNode*> culled_resources;
            eastl::vector<class PassNode*> culled_passes;
            RHIQueue* gfx_queue;
            RenderGraphFrameExecutor frame_executors[RG_MAX_FRAME_IN_FLIGHT];
        };
    }
}
