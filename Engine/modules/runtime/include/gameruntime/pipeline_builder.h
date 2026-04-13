#pragma once
#include "cyber_runtime.config.h"
#include "common/smart_ptr.h"
#include "graphics/interface/graphics_types.h"
#include "graphics/interface/render_pipeline.h"
#include "graphics/interface/root_signature.hpp"
#include "graphics/interface/sampler.h"
#include "resource/resource_loader.h"
#include "EASTL/vector.h"

namespace Cyber
{
    namespace RenderObject
    {
        class IRenderDevice;
        class IRenderPipeline;
        class IRootSignature;
        class ISampler;
        class IShaderLibrary;
    }

    namespace Renderer
    {
        class Renderer;
    }

    class CYBER_RUNTIME_API PipelineBuilder
    {
    public:
        PipelineBuilder(RenderObject::IRenderDevice* device);

        // Shader setup
        PipelineBuilder& vertex_shader(const char8_t* path, const char8_t* entry = CYBER_UTF8("VSMain"));
        PipelineBuilder& pixel_shader(const char8_t* path, const char8_t* entry = CYBER_UTF8("PSMain"));

        // Vertex layout
        PipelineBuilder& vertex_layout(const RenderObject::VertexAttribute* attribs, uint32_t count);

        // Static sampler
        PipelineBuilder& static_sampler(const char8_t* name, RenderObject::ISampler* sampler);

        // Blend state presets
        PipelineBuilder& blend_opaque();
        PipelineBuilder& blend_alpha();

        // Depth/stencil
        PipelineBuilder& depth_test(bool enable = true, bool write = true, COMPARE_MODE func = CMP_LESS_EQUAL);

        // Render target info
        PipelineBuilder& render_target_format(TEXTURE_FORMAT format);
        PipelineBuilder& depth_format(TEXTURE_FORMAT format);
        PipelineBuilder& topology(PRIMITIVE_TOPOLOGY topo);

        // Use existing root signature instead of creating one
        PipelineBuilder& root_signature(RenderObject::IRootSignature* sig);

        // Push constant names
        PipelineBuilder& push_constant(const char8_t* name);

        // Root descriptor names
        PipelineBuilder& root_descriptor(const char8_t* name);

        // Build pipeline; optionally outputs root signature for descriptor set creation
        RefCntAutoPtr<RenderObject::IRenderPipeline> build(
            RefCntAutoPtr<RenderObject::IRootSignature>* out_root_sig = nullptr);

    private:
        RenderObject::IRenderDevice* m_device = nullptr;

        RefCntAutoPtr<RenderObject::IShaderLibrary> m_vs_shader;
        RefCntAutoPtr<RenderObject::IShaderLibrary> m_ps_shader;
        RenderObject::PipelineShaderCreateDesc m_vs_desc = {};
        RenderObject::PipelineShaderCreateDesc m_ps_desc = {};

        RenderObject::VertexLayoutDesc m_vertex_layout = {};
        eastl::vector<RenderObject::VertexAttribute> m_vertex_attribs;

        eastl::vector<RenderObject::ISampler*> m_samplers;
        eastl::vector<const char8_t*> m_sampler_names;

        BlendStateCreateDesc m_blend_state = {};
        DepthStateCreateDesc m_depth_state = {};
        bool m_has_blend = false;
        bool m_has_depth = false;

        TEXTURE_FORMAT m_color_format = TEX_FORMAT_RGBA8_UNORM;
        TEXTURE_FORMAT m_depth_stencil_format = TEX_FORMAT_D32_FLOAT;
        uint32_t m_render_target_count = 1;
        PRIMITIVE_TOPOLOGY m_topology = PRIM_TOPO_TRIANGLE_LIST;

        RenderObject::IRootSignature* m_external_root_sig = nullptr;

        eastl::vector<const char8_t*> m_push_constant_names;
        eastl::vector<const char8_t*> m_root_descriptor_names;
    };
}
