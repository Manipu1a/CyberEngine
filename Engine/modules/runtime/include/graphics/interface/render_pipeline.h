
    struct CYBER_GRAPHICS_API RHIRenderPipelineCreateDesc
    {
        RHIRootSignature* root_signature;
        RHIPipelineShaderCreateDesc* vertex_shader;
        RHIPipelineShaderCreateDesc* tesc_shader;
        RHIPipelineShaderCreateDesc* tese_shader;
        RHIPipelineShaderCreateDesc* geometry_shader;
        RHIPipelineShaderCreateDesc* fragment_shader;
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