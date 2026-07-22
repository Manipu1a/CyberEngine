#pragma once

#include "render_graph_flag.h"
#include "base_type.h"
#include "render_graph_phase.h"
#include "platform/configure.h"
#include "cyber_runtime.config.h"
#include "eastl/functional.h"
#include "eastl/map.h"
#include "eastl/vector.h"
#include "graphics/interface/graphics_types.h"

#ifndef RG_MAX_FRAME_IN_FLIGHT
#define RG_MAX_FRAME_IN_FLIGHT 3
#endif

namespace Cyber
{  
    namespace RenderObject
    {
        struct ICommandBuffer;
        struct ICommandPool;
        class IQueue;
        struct IRenderDevice;
    }
    namespace render_graph
    {
        class CYBER_RUNTIME_API RenderGraphFrameExecutor
        {
        public:
            RenderGraphFrameExecutor() = default;
            void initialize(class RenderObject::IQueue* queue, class RenderObject::IRenderDevice* device);
            void finalize();

            RenderObject::ICommandPool* gfx_cmd_pool = nullptr;
            RenderObject::ICommandBuffer* gfx_cmd_buffer = nullptr;
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
            void reset_passes();
            void compile();
            void execute();
            void execute_pass(class RGPass* pass, uint32_t frame_index = 0);
            template<typename Phase>
            void add_custom_phase();
            void invalidate() CYBER_NOEXCEPT { compiled = false; }
        private:
            eastl::vector<class RenderGraphPhase*> phases;
            eastl::vector<class PassNode*> execution_order;
            class RenderGraphBuilder* graphBuilder = nullptr;
            bool compiled = false;
        public:
            eastl::map<const char8_t*, class RGRenderResource*, Utf8StringLess> resource_map;
            eastl::vector<class ResourceNode*> resources;
            eastl::vector<class PassNode*> passes;
            eastl::vector<class ResourceNode*> culled_resources;
            eastl::vector<class PassNode*> culled_passes;
            RenderObject::IQueue* gfx_queue = nullptr;
            RenderGraphFrameExecutor frame_executors[RG_MAX_FRAME_IN_FLIGHT];
        };
    }
}
