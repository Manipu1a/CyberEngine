#pragma once
#include "common/cyber_graphics_config.h"
#include "graphics/interface/vertex_input.h"
#include "interface/graphics_types.h"
#include "eastl/array.h"
#include "device_object.h"


namespace Cyber
{
    namespace RenderObject
    {
        struct CYBER_GRAPHICS_API PipelineShaderCreateDesc
        {
            class IShaderLibrary* m_library;
            const char8_t* m_entry;
            SHADER_STAGE m_stage;
        };

        struct CYBER_GRAPHICS_API RenderPipelineCreateDesc
        {
            class IRootSignature* root_signature;
            PipelineShaderCreateDesc* vertex_shader;
            PipelineShaderCreateDesc* tesc_shader;
            PipelineShaderCreateDesc* tese_shader;
            PipelineShaderCreateDesc* geometry_shader;
            PipelineShaderCreateDesc* fragment_shader;
            VertexLayoutDesc* vertex_layout;
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

        struct CYBER_GRAPHICS_API IRenderPipeline : public IDeviceObject
        {

        };

        template<typename EngineImplTraits>
        class RenderPipelineBase : public DeviceObjectBase<typename EngineImplTraits::RenderPipelineInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using RenderPipelineInterface = typename EngineImplTraits::RenderPipelineInterface;
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;
            using TRenderPipelineBase = typename DeviceObjectBase<RenderPipelineInterface, RenderDeviceImplType>;

            RenderPipelineBase(RenderDeviceImplType* device, const RenderPipelineCreateDesc& desc) : TRenderPipelineBase(device) { 
                InitializePipelineDesc(desc);
             };
            virtual ~RenderPipelineBase() = default;

            const RenderPipelineCreateDesc& get_graphics_pipeline_desc() const { return graphics_pipeline_data.desc; }
            
        protected:

            void InitializePipelineDesc(const RenderPipelineCreateDesc& create_desc)
            {
                auto& graphics_pipeline = graphics_pipeline_data.desc;
                graphics_pipeline = create_desc;

                VertexLayoutDesc* input_layout = cyber_new<VertexLayoutDesc>();
                input_layout->attribute_count = graphics_pipeline.vertex_layout->attribute_count;
                VertexAttribute* attributes = cyber_new_n<VertexAttribute>(input_layout->attribute_count);
                for(uint32_t i = 0; i < input_layout->attribute_count; i++)
                {
                    attributes[i] = graphics_pipeline.vertex_layout->attributes[i];
                    if(attributes[i].value_type == VALUE_TYPE_FLOAT32 || attributes[i].value_type == VALUE_TYPE_FLOAT16)
                    {
                        attributes[i].is_normalized = false;
                    }
                }

                graphics_pipeline.vertex_layout->attributes = attributes;

                eastl::array<uint32_t, MAX_BUFFER_SLOTS> strides, tight_strides;

                
            }

        protected:

            struct GraphicsPipelineData
            {
                RenderPipelineCreateDesc desc;
                uint32_t* strides = nullptr;
                uint8_t buffer_slots_used = 0;
            };

            GraphicsPipelineData graphics_pipeline_data;
        };
    }

}