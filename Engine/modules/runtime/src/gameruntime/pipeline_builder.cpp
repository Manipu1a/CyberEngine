#include "gameruntime/pipeline_builder.h"
#include "graphics/interface/render_device.hpp"
#include "graphics/interface/render_pipeline.h"
#include "graphics/interface/root_signature.hpp"
#include "graphics/interface/sampler.h"
#include "platform/memory.h"
#include "resource/resource_loader.h"

namespace Cyber
{
    PipelineBuilder::PipelineBuilder(RenderObject::IRenderDevice* device)
        : m_device(device)
    {
    }

    PipelineBuilder& PipelineBuilder::vertex_shader(const char8_t* path, const char8_t* entry)
    {
        ResourceLoader::ShaderLoadDesc load_desc = {};
        load_desc.target = SHADER_TARGET_6_0;
        load_desc.stage_load_desc = ResourceLoader::ShaderStageLoadDesc{
            .file_name = path,
            .stage = SHADER_STAGE_VERT,
            .entry_point_name = entry,
        };
        m_vs_shader = ResourceLoader::add_shader(m_device, load_desc);
        m_vs_desc.m_stage = SHADER_STAGE_VERT;
        m_vs_desc.m_library = m_vs_shader;
        m_vs_desc.m_entry = entry;
        return *this;
    }

    PipelineBuilder& PipelineBuilder::pixel_shader(const char8_t* path, const char8_t* entry)
    {
        ResourceLoader::ShaderLoadDesc load_desc = {};
        load_desc.target = SHADER_TARGET_6_0;
        load_desc.stage_load_desc = ResourceLoader::ShaderStageLoadDesc{
            .file_name = path,
            .stage = SHADER_STAGE_FRAG,
            .entry_point_name = entry,
        };
        m_ps_shader = ResourceLoader::add_shader(m_device, load_desc);
        m_ps_desc.m_stage = SHADER_STAGE_FRAG;
        m_ps_desc.m_library = m_ps_shader;
        m_ps_desc.m_entry = entry;
        return *this;
    }

    PipelineBuilder& PipelineBuilder::vertex_layout(const RenderObject::VertexAttribute* attribs, uint32_t count)
    {
        m_vertex_attribs.assign(attribs, attribs + count);
        m_vertex_layout.attribute_count = count;
        m_vertex_layout.attributes = m_vertex_attribs.data();
        return *this;
    }

    PipelineBuilder& PipelineBuilder::static_sampler(const char8_t* name, RenderObject::ISampler* sampler)
    {
        m_sampler_names.push_back(name);
        m_samplers.push_back(sampler);
        return *this;
    }

    PipelineBuilder& PipelineBuilder::blend_opaque()
    {
        m_has_blend = true;
        m_blend_state = {};
        m_blend_state.render_target_count = 1;
        m_blend_state.src_factors[0] = BLEND_CONSTANT_ONE;
        m_blend_state.dst_factors[0] = BLEND_CONSTANT_ZERO;
        m_blend_state.blend_modes[0] = BLEND_MODE_ADD;
        m_blend_state.src_alpha_factors[0] = BLEND_CONSTANT_ONE;
        m_blend_state.dst_alpha_factors[0] = BLEND_CONSTANT_ZERO;
        m_blend_state.blend_alpha_modes[0] = BLEND_MODE_ADD;
        m_blend_state.alpha_to_coverage = false;
        m_blend_state.masks[0] = COLOR_WRITE_MASK_ALL;
        return *this;
    }

    PipelineBuilder& PipelineBuilder::blend_alpha()
    {
        m_has_blend = true;
        m_blend_state = {};
        m_blend_state.render_target_count = 1;
        m_blend_state.src_factors[0] = BLEND_CONSTANT_SRC_ALPHA;
        m_blend_state.dst_factors[0] = BLEND_CONSTANT_ONE_MINUS_SRC_ALPHA;
        m_blend_state.blend_modes[0] = BLEND_MODE_ADD;
        m_blend_state.src_alpha_factors[0] = BLEND_CONSTANT_ONE;
        m_blend_state.dst_alpha_factors[0] = BLEND_CONSTANT_ZERO;
        m_blend_state.blend_alpha_modes[0] = BLEND_MODE_ADD;
        m_blend_state.alpha_to_coverage = false;
        m_blend_state.masks[0] = COLOR_WRITE_MASK_ALL;
        return *this;
    }

    PipelineBuilder& PipelineBuilder::depth_test(bool enable, bool write, COMPARE_MODE func)
    {
        m_has_depth = true;
        m_depth_state = {};
        m_depth_state.depth_test = enable;
        m_depth_state.depth_write = write;
        m_depth_state.depth_func = func;
        m_depth_state.stencil_test = false;
        return *this;
    }

    PipelineBuilder& PipelineBuilder::render_target_format(TEXTURE_FORMAT format)
    {
        m_color_format = format;
        return *this;
    }

    PipelineBuilder& PipelineBuilder::render_target_count(uint32_t count)
    {
        m_render_target_count = count;
        return *this;
    }

    PipelineBuilder& PipelineBuilder::depth_format(TEXTURE_FORMAT format)
    {
        m_depth_stencil_format = format;
        return *this;
    }

    PipelineBuilder& PipelineBuilder::topology(PRIMITIVE_TOPOLOGY topo)
    {
        m_topology = topo;
        return *this;
    }

    PipelineBuilder& PipelineBuilder::root_signature(RenderObject::IRootSignature* sig)
    {
        m_external_root_sig = sig;
        return *this;
    }

    PipelineBuilder& PipelineBuilder::push_constant(const char8_t* name)
    {
        m_push_constant_names.push_back(name);
        return *this;
    }

    PipelineBuilder& PipelineBuilder::root_descriptor(const char8_t* name)
    {
        m_root_descriptor_names.push_back(name);
        return *this;
    }

    RefCntAutoPtr<RenderObject::IRenderPipeline> PipelineBuilder::build()
    {
        // Create root signature if not provided externally
        RenderObject::IRootSignature* root_sig = m_external_root_sig;
        if (!root_sig)
        {
            RenderObject::RootSignatureCreateDesc rs_desc = {};
            rs_desc.vertex_shader = m_vs_shader ? &m_vs_desc : nullptr;
            rs_desc.pixel_shader = m_ps_shader ? &m_ps_desc : nullptr;
            rs_desc.m_staticSamplers = m_samplers.empty() ? nullptr : m_samplers.data();
            rs_desc.m_staticSamplerNames = m_sampler_names.empty() ? nullptr : m_sampler_names.data();
            rs_desc.m_staticSamplerCount = (uint32_t)m_samplers.size();
            rs_desc.m_pushConstantNames = m_push_constant_names.empty() ? nullptr : m_push_constant_names.data();
            rs_desc.m_pushConstantCount = (uint32_t)m_push_constant_names.size();
            rs_desc.root_descriptor_names = m_root_descriptor_names.empty() ? nullptr : m_root_descriptor_names.data();
            rs_desc.root_descriptor_count = (uint32_t)m_root_descriptor_names.size();
            rs_desc.m_pPool = nullptr;

            root_sig = m_device->create_root_signature(rs_desc);
        }

        // Default blend if not set
        if (!m_has_blend)
        {
            blend_opaque();
        }

        // Default depth if not set
        if (!m_has_depth)
        {
            depth_test(true, true, CMP_LESS_EQUAL);
        }
        m_blend_state.render_target_count = m_render_target_count;

        RenderObject::RenderPipelineCreateDesc rp_desc = {};
        rp_desc.vertex_shader = m_vs_shader ? &m_vs_desc : nullptr;
        rp_desc.mesh_shader = nullptr;
        rp_desc.amplification_shader = nullptr;
        rp_desc.geometry_shader = nullptr;
        rp_desc.pixel_shader = m_ps_shader ? &m_ps_desc : nullptr;
        rp_desc.compute_shader = nullptr;
        rp_desc.vertex_layout = m_vertex_attribs.empty() ? nullptr : &m_vertex_layout;
        rp_desc.blend_state = &m_blend_state;
        rp_desc.depth_stencil_state = &m_depth_state;
        rp_desc.rasterizer_state = nullptr;
        // Note: RenderPipelineCreateDesc::m_staticSamplers resolves ISampler to Cyber::ISampler
        // (forward-declared in graphics_types.h), which differs from Cyber::RenderObject::ISampler.
        // Use reinterpret_cast to bridge this namespace mismatch.
        rp_desc.m_staticSamplers = m_samplers.empty() ? nullptr : reinterpret_cast<ISampler**>(m_samplers.data());
        rp_desc.m_staticSamplerNames = m_sampler_names.empty() ? nullptr : m_sampler_names.data();
        rp_desc.m_staticSamplerCount = (uint32_t)m_samplers.size();
        rp_desc.m_pushConstantNames = m_push_constant_names.empty() ? nullptr : m_push_constant_names.data();
        rp_desc.m_pushConstantCount = (uint32_t)m_push_constant_names.size();
        rp_desc.root_descriptor_names = m_root_descriptor_names.empty() ? nullptr : m_root_descriptor_names.data();
        rp_desc.root_descriptor_count = (uint32_t)m_root_descriptor_names.size();
        rp_desc.color_formats = &m_color_format;
        rp_desc.render_target_count = m_render_target_count;
        rp_desc.sample_count = SAMPLE_COUNT_1;
        rp_desc.sample_quality = 0;
        rp_desc.depth_stencil_format = m_depth_stencil_format;
        rp_desc.prim_topology = m_topology;
        rp_desc.enable_indirect_command = false;

        RefCntAutoPtr<RenderObject::IRenderPipeline> pipeline;
        m_device->create_render_pipeline(rp_desc, &pipeline);
        return pipeline;
    }
}
