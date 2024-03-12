#pragma once
#include "common/cyber_graphics_config.h"
#include "interface/graphics_types.h"
#include "render_object.h"


namespace Cyber
{
    namespace RenderObject
    {
        struct CYBER_GRAPHICS_API PipelineShaderCreateDesc
        {
            class IShaderLibrary* library;
            const char8_t* entry;
            SHADER_STAGE stage;
        };

        struct CYBER_GRAPHICS_API RenderPipelineCreateDesc
        {
            class IRootSignature* root_signature;
            PipelineShaderCreateDesc* vertex_shader;
            PipelineShaderCreateDesc* tesc_shader;
            PipelineShaderCreateDesc* tese_shader;
            PipelineShaderCreateDesc* geometry_shader;
            PipelineShaderCreateDesc* fragment_shader;
            const VertexLayout* vertex_layout;
            BlendStateCreateDesc* blend_state;
            DepthStateCreateDesc* depth_stencil_state;
            RasterizerStateCreateDesc* rasterizer_state;

            const TEXTURE_FORMAT* color_formats;
            uint32_t render_target_count;
            TEXTURE_SAMPLE_COUNT sample_count;
            uint32_t sample_quality;
            SLOT_MASK_BIT color_resolve_disable_mask;
            TEXTURE_FORMAT depth_stencil_format;
            PRIMITIVE_TOPOLOGY prim_topology;
            bool enable_indirect_command;
        };

        struct CYBER_GRAPHICS_API IRenderPipeline
        {

        };

        template<typename EngineImplTraits>
        class RenderPipelineBase : public RenderObjectBase<typename EngineImplTraits::RenderPipelineInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using RenderPipelineInterface = typename EngineImplTraits::RenderPipelineInterface;
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
            using TRenderPipelineBase = typename RenderPipelineBase<RenderPipelineInterface, RenderDeviceImplType>;

            RenderPipelineBase(RenderDeviceImplType* device, const RenderPipelineCreateDesc& desc) : TRenderPipelineBase(device), m_desc(desc) {  };
            virtual ~RenderPipelineBase() = default;

        protected:
            RenderPipelineCreateDesc m_desc;
        };
    }

}