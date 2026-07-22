#include "renderer/forward_pipeline.h"

#include "graphics/features/pre_depth.h"
#include "graphics/features/scene_color.h"
#include "graphics/features/shadow.h"
#include "graphics/interface/buffer.h"
#include "graphics/interface/device_context.h"
#include "graphics/interface/render_device.hpp"
#include "graphics/interface/swap_chain.hpp"
#include "graphics/interface/texture.hpp"
#include "graphics/interface/texture_view.h"
#include "graphics/rendergraph/render_graph.h"
#include "graphics/rendergraph/render_graph_builder.h"
#include "renderer/renderer.h"

namespace Cyber::Renderer
{
    namespace
    {
        void create_constant_buffer(RenderObject::IRenderDevice* device, uint32_t size,
            RefCntAutoPtr<RenderObject::IBuffer>& out_buffer)
        {
            RenderObject::BufferCreateDesc desc = {};
            desc.bind_flags = GRAPHICS_RESOURCE_BIND_UNIFORM_BUFFER;
            desc.size = size;
            desc.usage = GRAPHICS_RESOURCE_USAGE_DYNAMIC;
            desc.cpu_access_flags = CPU_ACCESS_WRITE;
            device->create_buffer(desc, nullptr, &out_buffer);
        }
    }

    ForwardPipeline::ForwardPipeline(Renderer* renderer)
        : m_renderer(renderer)
    {
    }

    ForwardPipeline::~ForwardPipeline()
    {
        if (m_render_graph)
        {
            render_graph::RenderGraph::destroy(m_render_graph);
            m_render_graph = nullptr;
        }
    }

    void ForwardPipeline::initialize()
    {
        m_device = m_renderer->get_render_device();
        m_context = m_renderer->get_device_context();

        create_resources();
        m_pipeline_cache.initialize(m_device);
        update_pass_context({});
        create_render_graph();
    }

    void ForwardPipeline::resize(uint32_t width, uint32_t height)
    {
        (void)width;
        (void)height;
        create_render_graph();
    }

    void ForwardPipeline::render(World* world, float delta_time)
    {
        (void)delta_time;
        if (!m_device || !m_context || !m_render_graph)
            return;

        ForwardFrameContext frame_context = begin_frame();
        frame_context.world = world;
        update_pass_context(frame_context);
        update_render_graph_resources(frame_context);
        m_render_graph->execute();
    }

    void ForwardPipeline::create_resources()
    {
        create_constant_buffer(m_device, sizeof(ForwardSceneConstants), m_scene_constants);

        RenderObject::TextureCreateDesc shadow_desc = {};
        shadow_desc.m_name = u8"Forward_ShadowMap";
        shadow_desc.m_format = TEX_FORMAT_D32_FLOAT;
        shadow_desc.m_width = m_shadow_resolution;
        shadow_desc.m_height = m_shadow_resolution;
        shadow_desc.m_depth = 1;
        shadow_desc.m_arraySize = 1;
        shadow_desc.m_mipLevels = 1;
        shadow_desc.m_dimension = TEX_DIMENSION_2D;
        shadow_desc.m_usage = GRAPHICS_RESOURCE_USAGE_DEFAULT;
        shadow_desc.m_bindFlags = GRAPHICS_RESOURCE_BIND_DEPTH_STENCIL | GRAPHICS_RESOURCE_BIND_SHADER_RESOURCE;
        shadow_desc.m_clearValue = fastclear_0000;
        RenderObject::ITexture* raw_shadow = nullptr;
        m_device->create_texture(shadow_desc, nullptr, &raw_shadow);
        m_shadow_map.attach(raw_shadow);
    }

    void ForwardPipeline::create_render_graph()
    {
        if (!m_device || !m_context || !m_shadow_map)
            return;

        if (m_render_graph)
        {
            render_graph::RenderGraph::destroy(m_render_graph);
            m_render_graph = nullptr;
        }

        auto& scene_target = m_renderer->get_scene_target(0);
        m_render_graph = render_graph::RenderGraph::create(
            [&](render_graph::RenderGraphBuilder& builder)
            {
                builder.with_device(m_device);

                m_rg_scene_color = builder.import_texture(scene_target.color_buffer, u8"Forward.SceneColor");
                m_rg_scene_depth = builder.import_texture(scene_target.depth_buffer, u8"Forward.SceneDepth");
                m_rg_shadow_map = builder.import_texture(m_shadow_map, u8"Forward.ShadowMap");

                builder.add_pass<PreDepthPass>(u8"PreDepth",
                    PreDepthPass::Resources{ m_rg_scene_depth }, &m_pass_context);
                builder.add_pass<ShadowPass>(u8"Shadow",
                    ShadowPass::Resources{ m_rg_shadow_map }, &m_pass_context);
                builder.add_pass<SceneColorPass>(u8"SceneColor",
                    SceneColorPass::Resources{ m_rg_scene_color, m_rg_scene_depth, m_rg_shadow_map },
                    &m_pass_context);
            });
    }

    void ForwardPipeline::update_render_graph_resources(const ForwardFrameContext& frame_context)
    {
        if (!m_render_graph)
            return;

        auto* builder = m_render_graph->get_builder();
        builder->update_imported_texture(m_rg_scene_color, frame_context.color_buffer);
        builder->update_imported_texture(m_rg_scene_depth, frame_context.depth_buffer);
        builder->update_imported_texture(m_rg_shadow_map, m_shadow_map);
    }

    void ForwardPipeline::update_pass_context(const ForwardFrameContext& frame_context)
    {
        m_pass_context.renderer = m_renderer;
        m_pass_context.device = m_device;
        m_pass_context.command_context = m_context;
        m_pass_context.scene_constants = m_scene_constants;
        m_pass_context.pipeline_cache = &m_pipeline_cache;
        m_pass_context.shadow_resolution = m_shadow_resolution;
        m_pass_context.frame = frame_context;
    }

    ForwardFrameContext ForwardPipeline::begin_frame()
    {
        ForwardFrameContext frame_context = {};

        auto* swap_chain = m_renderer->get_swap_chain();
        AcquireNextDesc acquire_desc = {};
        frame_context.back_buffer_index = m_device->acquire_next_image(swap_chain, acquire_desc);
        m_renderer->set_back_buffer_index(frame_context.back_buffer_index);

        auto& scene_target = m_renderer->get_scene_target(frame_context.back_buffer_index);
        frame_context.frame_buffer = m_renderer->get_frame_buffer();
        frame_context.color_buffer = scene_target.color_buffer;
        frame_context.depth_buffer = scene_target.depth_buffer;

        m_context->cmd_begin();
        return frame_context;
    }
}
