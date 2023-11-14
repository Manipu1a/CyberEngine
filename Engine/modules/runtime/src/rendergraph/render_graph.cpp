#include "rendergraph/render_graph.h"
#include "platform/memory.h"
#include "rendergraph/render_graph_builder.h"
#include "rendergraph/render_graph_resource.h"

namespace Cyber
{
    namespace render_graph
    {
        void RenderGraphFrameExecutor::initialize(RHIQueue* queue, RHIDevice* device)
        {
            CommandPoolCreateDesc cmd_pool_desc = {};
            //gfx_cmd_pool = rhi_create_command_pool(queue, cmd_pool_desc);
            CommandBufferCreateDesc cmd_buffer_desc = {};
            cmd_buffer_desc.is_secondary = false;
            //gfx_cmd_buffer = rhi_create_command_buffer(gfx_cmd_pool, cmd_buffer_desc);
        }

        void RenderGraphFrameExecutor::finalize()
        {

        }

        /////////////////////////////
        RenderGraph::RenderGraph()
        {

        }

        RenderGraph::~RenderGraph()
        {

        }

        RenderGraph* RenderGraph::create(const RenderGraphSetupFunction& setup) CYBER_NOEXCEPT
        {
            RenderGraphBuilder* graphBuilder = cyber_new<RenderGraphBuilder>();
            setup(*graphBuilder);

            RenderGraph* graph = cyber_new<RenderGraph>();
            graph->graphBuilder = graphBuilder;
            graph->gfx_queue = graphBuilder->gfx_queue;
            graph->initialize();
            return graph;
        }

        void RenderGraph::initialize()
        {
            for(uint32_t i = 0; i < RG_MAX_FRAME_IN_FLIGHT; ++i)
            {
                frame_executors[i].initialize(gfx_queue, graphBuilder->device);
            }

        }

        void RenderGraph::destroy(RenderGraph* graph) CYBER_NOEXCEPT
        {
            cyber_delete(graph->graphBuilder);
            cyber_delete(graph);
        }

        template<typename Phase>
        void RenderGraph::add_custom_phase()
        {
            auto phase = cyber_new<Phase>();
            phases.push_back(phase);
        }

        uint32_t recursive_pass(PassNode* pass_node, uint32_t order)
        {
            pass_node->order = order;
            auto res = order;
            for(const auto& read : pass_node->read_edges)
            {
                auto from_node = read->from();
                res = recursive_pass((PassNode*)from_node, ++res);
            }
            return res;
        }

        void RenderGraph::execute()
        {
            bool find_present_pass = false;

            // 对Pass进行排序，资源可以确定依赖的pass，从而方便创建和销毁
            PassNode* present_node;
            for(uint32_t i = passes.size()-1; i >= 0; --i)
            {
                auto& pass = passes[i];
                
                if(!find_present_pass)
                {
                    if(pass->pass_type == RG_PRESENT_PASS)
                    {
                        find_present_pass = true;
                        present_node = pass;
                    }
                }
                else
                {
                    
                }
                //execute_pass(passes[i]->pass_handle);
            }

            auto pass_count = recursive_pass(present_node, 0);
            for(auto& pass : passes)
            {
                if(pass->order != -1)
                {
                    pass->order = pass_count - pass->order;
                }
                else
                {
                    culled_passes.push_back(pass);
                }
            }
            
        }

        void RenderGraph::execute_pass(class RGRenderPass* pass)
        {
            RenderPassContext pass_context;

            RHIRenderPassDesc beginRenderPassDesc = {};
            //pass_context.encoder = rhi_cmd_begin_render_pass(frame_executors[0].gfx_cmd_buffer, beginRenderPassDesc);

            pass->pass_function(*this, pass_context);
        }
    }
}