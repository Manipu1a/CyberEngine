#include "backend/d3d12/shader_library_d3d12.h"
#include "backend/d3d12/shader_reflection_d3d12.h"
#include "backend/d3d12/shader_resource_d3d12.h"
#include "platform/memory.h"
#include "interface/graphics_types.h"
#include <d3dcompiler.h>
#include "backend/d3d12/d3d12_utils.h"
#include <EASTL/EAStdC/EASprintf.h>
#include "EASTL/algorithm.h"

namespace Cyber
{
    namespace RenderObject
    {
        // Shader Reflection
        const char8_t* D3DShaderEntryName = u8"D3D12";
        static GRAPHICS_RESOURCE_TYPE gD3D12_TO_DESCRIPTOR[] = {
            GRAPHICS_RESOURCE_TYPE_UNIFORM_BUFFER,   // D3D_SIT_CBUFFER
            GRAPHICS_RESOURCE_TYPE_BUFFER,           // D3D_SIT_TBUFFER
            GRAPHICS_RESOURCE_TYPE_TEXTURE,          // D3D_SIT_TEXTURE
            GRAPHICS_RESOURCE_TYPE_SAMPLER,          // D3D_SIT_SAMPLER
            GRAPHICS_RESOURCE_TYPE_RW_TEXTURE,       // D3D_SIT_UAV_RWTYPED
            GRAPHICS_RESOURCE_TYPE_BUFFER,           // D3D_SIT_STRUCTURED
            GRAPHICS_RESOURCE_TYPE_RW_BUFFER,        // D3D_SIT_RWSTRUCTURED
            GRAPHICS_RESOURCE_TYPE_BUFFER,           // D3D_SIT_BYTEADDRESS
            GRAPHICS_RESOURCE_TYPE_RW_BUFFER,        // D3D_SIT_UAV_RWBYTEADDRESS
            GRAPHICS_RESOURCE_TYPE_RW_BUFFER,        // D3D_SIT_UAV_APPEND_STRUCTURED
            GRAPHICS_RESOURCE_TYPE_RW_BUFFER,       // D3D_SIT_UAV_CONSUME_STRUCTURED
            GRAPHICS_RESOURCE_TYPE_RW_BUFFER,       // D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER
            GRAPHICS_RESOURCE_TYPE_RAY_TRACING,     // D3D_SIT_RTACCELERATIONSTRUCTURE
        };

        static TEXTURE_DIMENSION gD3D12_TO_TEXTURE_DIM[D3D_SRV_DIMENSION_BUFFEREX + 1] = {
            TEX_DIMENSION_UNDEFINED,        // D3D_SRV_DIMENSION_UNKNOWN
            TEX_DIMENSION_UNDEFINED,        // D3D_SRV_DIMENSION_BUFFER
            TEX_DIMENSION_1D,               // D3D_SRV_DIMENSION_TEXTURE1D
            TEX_DIMENSION_1D_ARRAY,         // D3D_SRV_DIMENSION_TEXTURE1DARRAY
            TEX_DIMENSION_2D,               // D3D_SRV_DIMENSION_TEXTURE2D
            TEX_DIMENSION_2D_ARRAY,         // D3D_SRV_DIMENSION_TEXTURE2DARRAY
            TEX_DIMENSION_2DMS,             // D3D_SRV_DIMENSION_TEXTURE2DMS
            TEX_DIMENSION_2DMS_ARRAY,       // D3D_SRV_DIMENSION_TEXTURE2DMSARRAY
            TEX_DIMENSION_3D,               // D3D_SRV_DIMENSION_TEXTURE3D
            TEX_DIMENSION_CUBE,             // D3D_SRV_DIMENSION_TEXTURECUBE
            TEX_DIMENSION_CUBE_ARRAY,      // D3D_SRV_DIMENSION_TEXTURECUBEARRAY
            TEX_DIMENSION_UNDEFINED,       // D3D_SRV_DIMENSION_BUFFEREX
        };

        static TEXTURE_FORMAT gD3D12_TO_VERTEX_FORMAT[] = {
            TEX_FORMAT_UNKNOWN,
            TEX_FORMAT_R32_UINT,
            TEX_FORMAT_R32_SINT,
            TEX_FORMAT_R32_FLOAT,

            TEX_FORMAT_RG32_UINT,
            TEX_FORMAT_RG32_SINT,
            TEX_FORMAT_RG32_FLOAT,

            TEX_FORMAT_RGB32_UINT,
            TEX_FORMAT_RGB32_SINT,
            TEX_FORMAT_RGB32_FLOAT,

            TEX_FORMAT_RGBA32_UINT,
            TEX_FORMAT_RGBA32_SINT,
            TEX_FORMAT_RGBA32_FLOAT,
        };

        void ShaderLibrary_D3D12_Impl::free_reflection()
        {
            if(m_pEntryReflections)
            {
                for(uint32_t i = 0; i < m_entryCount; ++i)
                {
                    RenderObject::ShaderReflection_D3D12_Impl* reflection = static_cast<RenderObject::ShaderReflection_D3D12_Impl*>(m_pEntryReflections[i]);
                    //reflection->free_vertex_inputs();
                    reflection->free_shader_resources();
                    cyber_free(reflection);
                }
            }
        }

        void ShaderLibrary_D3D12_Impl::Initialize_shader_reflection(const RenderObject::ShaderLibraryCreateDesc& desc)
        {
            ID3D12ShaderReflection* d3d12Reflection = nullptr;
            auto procDxcCreateInstance = d3d12_util_get_dxc_create_instance_proc();

            bool bUseDXC = false;
            switch(desc.shader_compiler)
            {
                case SHADER_COMPILER_DEFAULT:
                    bUseDXC = false;
                    break;
                case SHADER_COMPILER_GLSLANG:
                    bUseDXC = false;
                    break;
                case SHADER_COMPILER_DXC:
                    bUseDXC = true;
                    break;
                case SHADER_COMPILER_FXC:
                    bUseDXC = false;
                    break;
                default:
                    CB_CORE_ERROR("Invalid shader compiler");
                    return;
            }

            if(bUseDXC)
            {
                IDxcUtils* pDxcUtils;
                procDxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&pDxcUtils));

                IDxcBlob *pReflectionData;
                m_pShaderResult->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(&pReflectionData), nullptr);
                if(pReflectionData != nullptr)
                {
                    DxcBuffer ReflectionData;
                    ReflectionData.Encoding = DXC_CP_ACP;
                    ReflectionData.Ptr = pReflectionData->GetBufferPointer();
                    ReflectionData.Size = pReflectionData->GetBufferSize();

                    if(SUCCEEDED(pDxcUtils->CreateReflection(&ReflectionData, IID_PPV_ARGS(&d3d12Reflection))))
                    {
                        D3D12_SHADER_DESC* shader_desc;
                        d3d12Reflection->GetDesc(shader_desc);
                    }
                }
            }
            else 
            {
                D3DReflect(m_pShaderBlob->GetBufferPointer(), m_pShaderBlob->GetBufferSize(), __uuidof(d3d12Reflection), reinterpret_cast<void**>(&d3d12Reflection));
            }

            if(d3d12Reflection)
            {
                collect_shader_reflection_data(d3d12Reflection, desc.stage);
            }

            d3d12Reflection->Release();
        }

        void ShaderLibrary_D3D12_Impl::collect_shader_reflection_data(ID3D12ShaderReflection* d3d12Reflection, SHADER_STAGE stage)
        {
            D3D12_SHADER_DESC shaderDesc;
            d3d12Reflection->GetDesc(&shaderDesc);
            reflection_record_shader_resource(d3d12Reflection, stage, shaderDesc);

            RenderObject::ShaderReflection_D3D12_Impl* reflection = static_cast<RenderObject::ShaderReflection_D3D12_Impl*>(m_pEntryReflections[0]);
            reflection->set_shader_stage(stage);
            // Collect vertex inputs
            if(stage == SHADER_STAGE_VERT)
            {
                reflection->set_vertex_input_count(shaderDesc.InputParameters);
                auto vertex_inputs = (RenderObject::IVertexInput**)cyber_calloc(shaderDesc.InputParameters, sizeof(RenderObject::IVertexInput*));
                // Count the string sizes of the vertex inputs for the name pool
                /*for(UINT i = 0; i < shaderDesc.InputParameters; ++i)
                {
                    D3D12_SIGNATURE_PARAMETER_DESC paramDesc;
                    d3d12Reflection->GetInputParameterDesc(i, &paramDesc);
                    // Get the length of the semantic name
                    bool hasParamIndex = paramDesc.SemanticIndex > 0 || !strcmp(paramDesc.SemanticName, "TEXCOORD");
                    uint32_t source_len = (uint32_t)strlen(paramDesc.SemanticName) + (hasParamIndex ? 1 : 0);

                    vertex_inputs[i] = cyber_new<RenderObject::VertexInput_D3D12_Impl>(get_device());
                    vertex_inputs[i]->set_name((char8_t*)cyber_malloc(sizeof(char8_t) * (source_len+1)));
                    vertex_inputs[i]->set_semantics_name((char8_t*)cyber_malloc(sizeof(char8_t) * (source_len+1)));
                    if(hasParamIndex)
                        EA::StdC::Sprintf((char*)vertex_inputs[i]->get_name(), "%s%u", paramDesc.SemanticName, paramDesc.SemanticIndex);
                    else
                        EA::StdC::Sprintf((char*)vertex_inputs[i]->get_name(), "%s", paramDesc.SemanticName);

                    EA::StdC::Sprintf((char*)vertex_inputs[i]->get_semantics_name(), "%s", paramDesc.SemanticName);
                    vertex_inputs[i]->set_semantics_index(paramDesc.SemanticIndex);
                    vertex_inputs[i]->set_binding(paramDesc.Register);
                    const uint32_t comps = (uint32_t)log2(paramDesc.Mask);
                    vertex_inputs[i]->set_format(gD3D12_TO_VERTEX_FORMAT[(paramDesc.ComponentType + 3 * comps)]);
                }
                reflection->set_vertex_inputs(vertex_inputs);*/
            }
            else if(stage == SHADER_STAGE_COMPUTE)
            {
                uint32_t thread_group_sizes[3];
                d3d12Reflection->GetThreadGroupSize(
                    &thread_group_sizes[0], 
                    &thread_group_sizes[1], 
                    &thread_group_sizes[2]);
                
                reflection->set_thread_group_sizes(thread_group_sizes[0], thread_group_sizes[1], thread_group_sizes[2]);
            }
        }

        void ShaderLibrary_D3D12_Impl::reflection_record_shader_resource( ID3D12ShaderReflection* d3d12Reflection, SHADER_STAGE stage, const D3D12_SHADER_DESC& shaderDesc)
        {
            // Get the number of bound resources
            m_entryCount = 1;
            m_pEntryReflections = (RenderObject::IShaderReflection**)cyber_calloc(m_entryCount, sizeof(RenderObject::IShaderReflection*));
            RenderObject::ShaderReflection_D3D12_Impl* reflection = cyber_new<RenderObject::ShaderReflection_D3D12_Impl>(get_device());
            reflection->set_entry_name(m_name);
            reflection->set_shader_resource_count(shaderDesc.BoundResources);
            auto shader_resources = (RenderObject::IShaderResource**)cyber_calloc(shaderDesc.BoundResources, sizeof(RenderObject::IShaderResource*));
            RenderObject::ShaderRegisterCount register_count;

            // Count string sizes of the bound resources for the name pool
            for(UINT i = 0;i < shaderDesc.BoundResources; ++i)
            {
                D3D12_SHADER_INPUT_BIND_DESC bindDesc;
                d3d12Reflection->GetResourceBindingDesc(i, &bindDesc);
                const size_t source_len = strlen(bindDesc.Name);
                shader_resources[i] = cyber_new<RenderObject::ShaderResource_D3D12_Impl>(get_device());
                RenderObject::ShaderResource_D3D12_Impl* resource = static_cast<RenderObject::ShaderResource_D3D12_Impl*>(shader_resources[i]);

                resource->set_name((char8_t*)cyber_malloc(sizeof(char8_t) * (source_len + 1)));
                
                // We are very sure it's windows platform
                strcpy_s((char*)resource->get_name(), source_len + 1, bindDesc.Name);
                resource->set_type(gD3D12_TO_DESCRIPTOR[bindDesc.Type]);
                resource->set_set(bindDesc.Space);
                resource->set_binding(bindDesc.BindPoint);
                resource->set_size(bindDesc.BindCount);
                resource->set_stages(stage);
                resource->set_dimension(DXGIUtil_TranslateSRVDimension(bindDesc.Dimension));

                if(bindDesc.Type == D3D_SIT_CBUFFER || bindDesc.Type == D3D_SIT_TBUFFER)
                {
                    register_count.constant_buffer_count = eastl::max(register_count.constant_buffer_count, bindDesc.BindPoint + bindDesc.BindCount);
                }
                else if(bindDesc.Type == D3D_SIT_SAMPLER)
                {
                    register_count.sampler_count = eastl::max(register_count.sampler_count, bindDesc.BindPoint + bindDesc.BindCount);
                }
                else if(bindDesc.Type == D3D_SIT_TEXTURE)
                {
                    register_count.shader_resource_count = eastl::max(register_count.shader_resource_count, bindDesc.BindPoint + bindDesc.BindCount);
                }
                else if(bindDesc.Type == D3D_SIT_UAV_RWTYPED || bindDesc.Type == D3D_SIT_UAV_RWSTRUCTURED || 
                    bindDesc.Type == D3D_SIT_UAV_RWBYTEADDRESS || bindDesc.Type == D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER ||
                    bindDesc.Type == D3D_SIT_UAV_APPEND_STRUCTURED)
                {
                    register_count.unordered_access_count = eastl::max(register_count.unordered_access_count, bindDesc.BindPoint + bindDesc.BindCount);
                }

                if(shaderDesc.ConstantBuffers && bindDesc.Type == D3D_SIT_CBUFFER)
                {
                    ID3D12ShaderReflectionConstantBuffer* buffer = d3d12Reflection->GetConstantBufferByName(bindDesc.Name);
                    cyber_assert(buffer, "D3D12 reflection failed : CBV not found!");
                    D3D12_SHADER_BUFFER_DESC bufferDesc;
                    buffer->GetDesc(&bufferDesc);
                    resource->set_size(bufferDesc.Size);
                }
                // RWTyped is considered as DESCRIPTOR_TYPE_TEXTURE by default so we handle the case for RWBuffer here
                if(bindDesc.Type == D3D_SHADER_INPUT_TYPE::D3D_SIT_UAV_RWTYPED && bindDesc.Dimension == D3D_SRV_DIMENSION_BUFFER)
                {
                    resource->set_type(GRAPHICS_RESOURCE_TYPE_RW_BUFFER);
                }
                // Buffer<> is considered as DESCRIPTOR_TYPE_TEXTURE by default so we handle the case for Buffer<> here
                if(bindDesc.Type == D3D_SHADER_INPUT_TYPE::D3D_SIT_TEXTURE && bindDesc.Dimension == D3D_SRV_DIMENSION_BUFFER)
                {
                    resource->set_type(GRAPHICS_RESOURCE_TYPE_BUFFER);
                }
            }
            reflection->set_shader_register_count(register_count);
            reflection->set_shader_resources( shader_resources );
            m_pEntryReflections[0] = reflection;
        }
    }
}