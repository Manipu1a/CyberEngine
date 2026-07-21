#include "rendergraph/render_graph.h"
#include "platform/memory.h"
#include "rendergraph/render_graph_builder.h"
#include "rendergraph/render_graph_resource.h"

namespace Cyber
{
    namespace render_graph
    {
        void RenderGraphFrameExecutor::initialize(RenderObject::IQueue*, RenderObject::IRenderDevice*)
        {
            // Command allocator and command buffer creation will be connected to the RHI here.
        }

        void RenderGraphFrameExecutor::finalize()
        {
            gfx_cmd_buffer = nullptr;
            gfx_cmd_pool = nullptr;
        }

        RenderGraph::RenderGraph() = default;

        RenderGraph::~RenderGraph()
        {
            for (uint32_t i = 0; i < RG_MAX_FRAME_IN_FLIGHT; ++i)
                frame_executors[i].finalize();

            for (auto* phase : phases)
                cyber_delete(phase);
            phases.clear();

            for (auto* pass_node : passes)
            {
                if (pass_node->pass_handle && pass_node->destroy_pass)
                    pass_node->destroy_pass(pass_node->pass_handle);
                cyber_delete(pass_node);
            }
            passes.clear();
            execution_order.clear();
            culled_passes.clear();

            for (auto* resource_node : resources)
            {
                if (resource_node->object_type == RG_TEXTURE_NODE)
                    cyber_delete(static_cast<TextureNode*>(resource_node));
                else if (resource_node->object_type == RG_BUFFER_NODE)
                    cyber_delete(static_cast<BufferNode*>(resource_node));
                else
                    cyber_delete(resource_node);
            }
            resources.clear();
            culled_resources.clear();

            for (auto& entry : resource_map)
            {
                RGRenderResource* resource = entry.second;
                if (!resource)
                    continue;

                switch (resource->resource_type)
                {
                case ERGResourceType::Texture:
                    cyber_delete(static_cast<RGTexture*>(resource));
                    break;
                case ERGResourceType::Buffer:
                    cyber_delete(static_cast<RGBuffer*>(resource));
                    break;
                case ERGResourceType::DepthStencil:
                    cyber_delete(static_cast<RGDepthStencil*>(resource));
                    break;
                default:
                    cyber_delete(resource);
                    break;
                }
            }
            resource_map.clear();

            cyber_delete(graphBuilder);
            graphBuilder = nullptr;
        }

        RenderGraph* RenderGraph::create(const RenderGraphSetupFunction& setup) CYBER_NOEXCEPT
        {
            RenderGraph* graph = cyber_new<RenderGraph>();
            graph->graphBuilder = cyber_new<RenderGraphBuilder>(graph);

            if (setup)
                setup(*graph->graphBuilder);

            graph->gfx_queue = graph->graphBuilder->gfx_queue;
            graph->initialize();
            graph->compile();
            return graph;
        }

        void RenderGraph::initialize()
        {
            for (uint32_t i = 0; i < RG_MAX_FRAME_IN_FLIGHT; ++i)
                frame_executors[i].initialize(gfx_queue, graphBuilder->device);
        }

        void RenderGraph::destroy(RenderGraph* graph) CYBER_NOEXCEPT
        {
            cyber_delete(graph);
        }

        template<typename Phase>
        void RenderGraph::add_custom_phase()
        {
            auto phase = cyber_new<Phase>();
            phases.push_back(phase);
        }

        static bool writes_resource(ERGResourceAccess access)
        {
            return access == ERGResourceAccess::Write || access == ERGResourceAccess::ReadWrite;
        }

        static bool has_resource_hazard(const PassNode* previous, const PassNode* current)
        {
            for (const auto& previous_access : previous->resource_accesses)
            {
                for (const auto& current_access : current->resource_accesses)
                {
                    if (previous_access.resource == current_access.resource &&
                        (writes_resource(previous_access.access) || writes_resource(current_access.access)))
                        return true;
                }
            }
            return false;
        }

        void RenderGraph::compile()
        {
            execution_order.clear();
            culled_passes.clear();

            for (auto* pass : passes)
            {
                pass->dependencies.clear();
                pass->order = UINT32_MAX;
            }

            // Registration order resolves WAW and WAR ambiguity. Resource hazards become
            // explicit pass dependencies; read/read access remains freely reorderable.
            for (size_t current_index = 0; current_index < passes.size(); ++current_index)
            {
                PassNode* current = passes[current_index];
                for (size_t previous_index = 0; previous_index < current_index; ++previous_index)
                {
                    PassNode* previous = passes[previous_index];
                    if (has_resource_hazard(previous, current))
                        current->dependencies.push_back(previous);
                }
            }

            eastl::vector<uint8_t> scheduled;
            scheduled.resize(passes.size(), 0);

            while (execution_order.size() < passes.size())
            {
                bool made_progress = false;
                for (size_t pass_index = 0; pass_index < passes.size(); ++pass_index)
                {
                    if (scheduled[pass_index])
                        continue;

                    PassNode* candidate = passes[pass_index];
                    bool ready = true;
                    for (auto* dependency : candidate->dependencies)
                    {
                        if (dependency->order == UINT32_MAX)
                        {
                            ready = false;
                            break;
                        }
                    }

                    if (!ready)
                        continue;

                    candidate->order = static_cast<uint32_t>(execution_order.size());
                    execution_order.push_back(candidate);
                    scheduled[pass_index] = 1;
                    made_progress = true;
                }

                if (!made_progress)
                {
                    cyber_assert(false, "RenderGraph contains a cyclic pass dependency");
                    break;
                }
            }

            compiled = execution_order.size() == passes.size();
        }

        void RenderGraph::execute()
        {
            if (!compiled)
                compile();

            if (!compiled)
                return;

            for (auto* pass_node : execution_order)
                execute_pass(pass_node->pass_handle, 0);
        }

        void RenderGraph::execute_pass(RGPass* pass, uint32_t frame_index)
        {
            if (!pass || frame_index >= RG_MAX_FRAME_IN_FLIGHT)
                return;

            RenderPassContext pass_context;
            pass_context.graph = this;
            pass_context.pass = pass;
            pass_context.frame_index = frame_index;
            pass_context.encoder = frame_executors[frame_index].gfx_cmd_buffer;

            pass->execute(*this, pass_context);
        }
    }
}
