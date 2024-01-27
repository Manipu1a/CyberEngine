#pragma once
#include "common/cyber_graphics_config.h"
#include "interface/rhi.h"
#include "render_object.h"


namespace Cyber
{
    namespace RenderObject
    {
        struct CYBER_GRAPHICS_API PipelineShaderCreateDesc
        {
            class IShaderLibrary* library;
            const char8_t* entry;
            ERHIShaderStage stage;
        };

        struct CYBER_GRAPHICS_API RenderPipelineCreateDesc
        {
            class IRootSignature* root_signature;
            PipelineShaderCreateDesc* vertex_shader;
            PipelineShaderCreateDesc* tesc_shader;
            PipelineShaderCreateDesc* tese_shader;
            PipelineShaderCreateDesc* geometry_shader;
            PipelineShaderCreateDesc* fragment_shader;
            const RHIVertexLayout* vertex_layout;
            RHIBlendStateCreateDesc* blend_state;
            RHIDepthStateCreateDesc* depth_stencil_state;
            RHIRasterizerStateCreateDesc* rasterizer_state;

            const ERHIFormat* color_formats;
            uint32_t render_target_count;
            ERHITextureSampleCount sample_count;
            uint32_t sample_quality;
            ERHISlotMaskBit color_resolve_disable_mask;
            ERHIFormat depth_stencil_format;
            ERHIPrimitiveTopology prim_topology;
            bool enable_indirect_command;
        };

        struct CYBER_GRAPHICS_API IRenderPipeline
        {

        };

        template<typename EngineImplTraits>
        class RenderPipelineBase : public RenderObjectBase<typename EngineImplTraits::RenderPipelineInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;

            RenderPipelineBase(RenderDeviceImplType* device);
            virtual ~RenderPipelineBase() = default;
        protected:

        };
    }

}