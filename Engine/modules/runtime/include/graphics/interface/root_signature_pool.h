#pragma once
#include "common/cyber_graphics_config.h"
#include "render_pass.h"
#include "device_object.h"
#include "interface/graphics_types.h"

namespace Cyber
{
    namespace RenderObject
    {
        
        struct CYBER_GRAPHICS_API RootSignaturePoolCreateDesc
        {
            class IRenderDevice* m_renderDevice;
            const char8_t* m_name;
        };

        class RootSignaturePoolBase
        {
        public:
            RootSignaturePoolBase( const RootSignaturePoolCreateDesc& desc) {  };
            
            virtual ~RootSignaturePoolBase() = default;

            virtual RootSignaturePoolCreateDesc get_create_desc() const
            {
                return m_desc;
            }
        protected:
            PIPELINE_TYPE m_pipelineType;
            IRenderDevice* m_renderDevice;
            RootSignaturePoolCreateDesc m_desc;
        };
    }

}