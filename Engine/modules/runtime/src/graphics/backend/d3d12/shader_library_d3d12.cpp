#include "backend/d3d12/shader_library_d3d12.h"
#include "backend/d3d12/shader_reflection_d3d12.h"
#include "backend/d3d12/vertex_input_d3d12.h"
#include "backend/d3d12/shader_resource_d3d12.h"
#include "platform/memory.h"

namespace Cyber
{
    namespace RenderObject
    {
        // Shader Reflection
        const char8_t* D3DShaderEntryName = u8"D3D12";
        static ERHIResourceType gD3D12_TO_DESCRIPTOR[] = {
            RHI_RESOURCE_TYPE_UNIFORM_BUFFER,   // D3D_SIT_CBUFFER
            RHI_RESOURCE_TYPE_BUFFER,           // D3D_SIT_TBUFFER
            RHI_RESOURCE_TYPE_TEXTURE,          // D3D_SIT_TEXTURE
            RHI_RESOURCE_TYPE_SAMPLER,          // D3D_SIT_SAMPLER
            RHI_RESOURCE_TYPE_RW_TEXTURE,       // D3D_SIT_UAV_RWTYPED
            RHI_RESOURCE_TYPE_BUFFER,           // D3D_SIT_STRUCTURED
            RHI_RESOURCE_TYPE_RW_BUFFER,        // D3D_SIT_RWSTRUCTURED
            RHI_RESOURCE_TYPE_BUFFER,           // D3D_SIT_BYTEADDRESS
            RHI_RESOURCE_TYPE_RW_BUFFER,        // D3D_SIT_UAV_RWBYTEADDRESS
            RHI_RESOURCE_TYPE_RW_BUFFER,        // D3D_SIT_UAV_APPEND_STRUCTURED
            RHI_RESOURCE_TYPE_RW_BUFFER,       // D3D_SIT_UAV_CONSUME_STRUCTURED
            RHI_RESOURCE_TYPE_RW_BUFFER,       // D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER
            RHI_RESOURCE_TYPE_RAY_TRACING,     // D3D_SIT_RTACCELERATIONSTRUCTURE
        };

        static ERHITextureDimension gD3D12_TO_TEXTURE_DIM[D3D_SRV_DIMENSION_BUFFEREX + 1] = {
            RHI_TEX_DIMENSION_UNDEFINED,        // D3D_SRV_DIMENSION_UNKNOWN
            RHI_TEX_DIMENSION_UNDEFINED,        // D3D_SRV_DIMENSION_BUFFER
            RHI_TEX_DIMENSION_1D,               // D3D_SRV_DIMENSION_TEXTURE1D
            RHI_TEX_DIMENSION_1D_ARRAY,         // D3D_SRV_DIMENSION_TEXTURE1DARRAY
            RHI_TEX_DIMENSION_2D,               // D3D_SRV_DIMENSION_TEXTURE2D
            RHI_TEX_DIMENSION_2D_ARRAY,         // D3D_SRV_DIMENSION_TEXTURE2DARRAY
            RHI_TEX_DIMENSION_2DMS,             // D3D_SRV_DIMENSION_TEXTURE2DMS
            RHI_TEX_DIMENSION_2DMS_ARRAY,       // D3D_SRV_DIMENSION_TEXTURE2DMSARRAY
            RHI_TEX_DIMENSION_3D,               // D3D_SRV_DIMENSION_TEXTURE3D
            RHI_TEX_DIMENSION_CUBE,             // D3D_SRV_DIMENSION_TEXTURECUBE
            RHI_TEX_DIMENSION_CUBE_ARRAY,      // D3D_SRV_DIMENSION_TEXTURECUBEARRAY
            RHI_TEX_DIMENSION_UNDEFINED,       // D3D_SRV_DIMENSION_BUFFEREX
        };

        static ERHIFormat gD3D12_TO_VERTEX_FORMAT[] = {
            RHI_FORMAT_UNDEFINED,
            RHI_FORMAT_R32_UINT,
            RHI_FORMAT_R32_SINT,
            RHI_FORMAT_R32_SFLOAT,

            RHI_FORMAT_R32G32_UINT,
            RHI_FORMAT_R32G32_SINT,
            RHI_FORMAT_R32G32_SFLOAT,

            RHI_FORMAT_R32G32B32_UINT,
            RHI_FORMAT_R32G32B32_SINT,
            RHI_FORMAT_R32G32B32_SFLOAT,

            RHI_FORMAT_R32G32B32A32_UINT,
            RHI_FORMAT_R32G32B32A32_SINT,
            RHI_FORMAT_R32G32B32A32_SFLOAT,
        };

        #if !defined (XBOX) && defined (_WIN32)
        struct RHIUtil_DXCLoader
        {
            void Load()
            {
                dxcLibrary = LoadLibrary(L"dxcompiler.dll");
                pDxcCreateInstance = (void*)::GetProcAddress((HMODULE)dxcLibrary, "DxcCreateInstance");

            }
            void Unload()
            {
                pDxcCreateInstance = nullptr;
                ::FreeLibrary(dxcLibrary);
            }

            DxcCreateInstanceProc Get()
            {
                return (DxcCreateInstanceProc)pDxcCreateInstance;
            }
            HMODULE dxcLibrary;
            void* pDxcCreateInstance;
            uint32_t mMajorVersion;
            uint32_t mMinorVersion;
            uint32_t shader_model_major;
            uint32_t shader_model_minor;
        };

        //void* RHIUtil_DXCLoader::pDxcCreateInstance = nullptr;
        //HMODULE RHIUtil_DXCLoader::dxcLibrary = nullptr;
        static RHIUtil_DXCLoader DxcLoader;
        
        void TestModel()
        {
            auto procDxcCreateInstance = DxcLoader.Get();
            DxcLoader.shader_model_major = 6;
            constexpr char TestShader[] = R"(
            float4 main() : SV_Target0
            {
                return float4(0.0, 0.0, 0.0, 0.0);
            }
            )";

            IDxcLibrary* pLibrary = nullptr;
            IDxcCompiler* pCompiler = nullptr;
            procDxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&pLibrary));
            procDxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler));

            IDxcBlobEncoding* pSource = nullptr;
            pLibrary->CreateBlobWithEncodingFromPinned(TestShader, sizeof(TestShader), 0, &pSource);

            eastl::vector<const wchar_t*> DxilArgs;

            for(uint32_t MinorVer = 1;;++MinorVer)
            {
                eastl::wstring Profile(eastl::wstring::CtorSprintf(), L"ps_6_%d" , MinorVer);
                IDxcOperationResult* pdxcResult = nullptr;
                auto hr = pCompiler->Compile(pSource, L"", L"main", Profile.c_str(), !DxilArgs.empty() ? DxilArgs.data() : nullptr, (uint32_t)DxilArgs.size(), nullptr, 0, nullptr, &pdxcResult);
                if(FAILED(hr))
                {
                    break;
                }

                HRESULT status = E_FAIL;
                if(FAILED(pdxcResult->GetStatus(&status)))
                {
                    break;
                }
                if(FAILED(status))
                {
                    break;
                }

                DxcLoader.shader_model_minor = MinorVer;
            }
        }

        void LoadDxcDLL()
        {
            DxcLoader.Load();

            auto procDxcCreateInstance = DxcLoader.Get();
            TestModel();

            if(procDxcCreateInstance)
            {
                IDxcValidator* pValidator = nullptr;
                if(SUCCEEDED(procDxcCreateInstance(CLSID_DxcValidator, IID_PPV_ARGS(&pValidator))))
                {
                    IDxcVersionInfo* pVersionInfo;
                    if(SUCCEEDED(pValidator->QueryInterface(IID_PPV_ARGS(&pVersionInfo))))
                    {
                        pVersionInfo->GetVersion(&DxcLoader.mMajorVersion, &DxcLoader.mMinorVersion);

                    }
                }
                CB_INFO("Loaded DX Shader Compiler {0}.{1}. Max supported shader model: {2}.{3}", DxcLoader.mMajorVersion, DxcLoader.mMinorVersion, DxcLoader.shader_model_major, DxcLoader.shader_model_minor);
                //LOG_INFO_MESSAGE("Loaded DX Shader Compiler ", m_MajorVer, ".", m_MinorVer, ". Max supported shader model: ", m_MaxShaderModel.Major, '.', m_MaxShaderModel.Minor);
            }
        }

        void D3D12Util_UnloadDxcDLL()
        {
            DxcLoader.Unload();
        }

        DxcCreateInstanceProc D3D12Util_GetDxcCreateInstanceProc()
        {
            if(DxcLoader.pDxcCreateInstance == nullptr)
            {
                D3D12Util_LoadDxcDLL();
            }
            return DxcLoader.Get();
        }
        #endif

        void ShaderLibrary_D3D12_Impl::free_reflection()
        {
            if(entry_reflections)
            {
                RenderObject::ShaderReflection_D3D12_Impl* reflection = static_cast<RenderObject::ShaderReflection_D3D12_Impl*>(entry_reflections);
                if(reflection->vertex_inputs)
                {
                    for(uint32_t i = 0; i < reflection->vertex_input_count; ++i)
                    {
                        reflection->vertex_inputs[i].free();
                    }
                    cyber_free(reflection->vertex_inputs);
                }
                if(reflection->shader_resources)
                {
                    for(uint32_t i = 0; i < reflection->shader_resource_count; ++i)
                    {
                        reflection->shader_resources[i].free();
                    }
                    cyber_free(reflection->shader_resources);
                }
                cyber_free(reflection);
            }
        }

        void ShaderLibrary_D3D12_Impl::Initialize_shader_reflection(const RenderObject::ShaderLibraryCreateDesc& desc)
        {
            ID3D12ShaderReflection* d3d12Reflection = nullptr;
            auto procDxcCreateInstance = D3D12Util_GetDxcCreateInstanceProc();

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

        void ShaderLibrary_D3D12_Impl::collect_shader_reflection_data(ID3D12ShaderReflection* d3d12Reflection, ERHIShaderStage stage)
        {
            D3D12_SHADER_DESC shaderDesc;
            d3d12Reflection->GetDesc(&shaderDesc);
            reflection_record_shader_resource(d3d12Reflection, stage, shaderDesc);
            RenderObject::ShaderReflection_D3D12_Impl* reflection = static_cast<RenderObject::ShaderReflection_D3D12_Impl*>(entry_reflections);
            reflection->shader_stage = stage;

            // Collect vertex inputs
            if(stage == RHI_SHADER_STAGE_VERT)
            {
                reflection->vertex_input_count = shaderDesc.InputParameters;
                auto vertex_inputs = (RenderObject::VertexInput_D3D12_Impl*)cyber_calloc(reflection->vertex_input_count, sizeof(RenderObject::VertexInput_D3D12_Impl));
                // Count the string sizes of the vertex inputs for the name pool
                for(UINT i = 0; i < shaderDesc.InputParameters; ++i)
                {
                    D3D12_SIGNATURE_PARAMETER_DESC paramDesc;
                    d3d12Reflection->GetInputParameterDesc(i, &paramDesc);
                    // Get the length of the semantic name
                    bool hasParamIndex = paramDesc.SemanticIndex > 0 || !strcmp(paramDesc.SemanticName, "TEXCOORD");
                    uint32_t source_len = (uint32_t)strlen(paramDesc.SemanticName) + (hasParamIndex ? 1 : 0);

                    vertex_inputs[i].set_name((char8_t*)cyber_malloc(sizeof(char8_t) * (source_len+1)));
                    vertex_inputs[i].set_semantics_name((char8_t*)cyber_malloc(sizeof(char8_t) * (source_len+1)));
                    if(hasParamIndex)
                        EA::StdC::Sprintf((char*)vertex_inputs[i].get_name(), "%s%u", paramDesc.SemanticName, paramDesc.SemanticIndex);
                    else
                        EA::StdC::Sprintf((char*)vertex_inputs[i].get_name(), "%s", paramDesc.SemanticName);

                    EA::StdC::Sprintf((char*)vertex_inputs[i].get_semantics_name(), "%s", paramDesc.SemanticName);
                    vertex_inputs[i].set_semantics_index(paramDesc.SemanticIndex);
                    vertex_inputs[i].set_binding(paramDesc.Register);
                    const uint32_t comps = (uint32_t)log2(paramDesc.Mask);
                    vertex_inputs[i].set_format(gD3D12_TO_VERTEX_FORMAT[(paramDesc.ComponentType + 3 * comps)]);
                }
                reflection->vertex_inputs = vertex_inputs;
            }
            else if(stage == RHI_SHADER_STAGE_COMPUTE)
            {
                d3d12Reflection->GetThreadGroupSize(
                    &reflection->thread_group_sizes[0], 
                    &reflection->thread_group_sizes[1], 
                    &reflection->thread_group_sizes[2]);
            }
        }

        void ShaderLibrary_D3D12_Impl::reflection_record_shader_resource( ID3D12ShaderReflection* d3d12Reflection, ERHIShaderStage stage, const D3D12_SHADER_DESC& shaderDesc)
        {
            // Get the number of bound resources
            entry_count = 1;
            entry_reflections = (RenderObject::ShaderReflection_D3D12_Impl*)cyber_calloc(entry_count, sizeof(RenderObject::ShaderReflection_D3D12_Impl));
            RenderObject::ShaderReflection_D3D12_Impl* reflection = static_cast<RenderObject::ShaderReflection_D3D12_Impl*>(entry_reflections);
            reflection->entry_name = D3DShaderEntryName;
            reflection->shader_resource_count = shaderDesc.BoundResources;
            reflection->shader_resources = (RenderObject::ShaderResource_D3D12_Impl*)cyber_calloc(shaderDesc.BoundResources, sizeof(RenderObject::ShaderResource_D3D12_Impl));

            // Count string sizes of the bound resources for the name pool
            for(UINT i = 0;i < shaderDesc.BoundResources; ++i)
            {
                D3D12_SHADER_INPUT_BIND_DESC bindDesc;
                d3d12Reflection->GetResourceBindingDesc(i, &bindDesc);
                const size_t source_len = strlen(bindDesc.Name);
                RenderObject::ShaderResource_D3D12_Impl* resource = static_cast<RenderObject::ShaderResource_D3D12_Impl*>(&reflection->shader_resources[i]);

                resource->set_name((char8_t*)cyber_malloc(sizeof(char8_t) * (source_len + 1)));
                
                // We are very sure it's windows platform
                strcpy_s((char*)resource->get_name(), source_len + 1, bindDesc.Name);
                resource->set_type(gD3D12_TO_DESCRIPTOR[bindDesc.Type]);
                resource->set_set(bindDesc.Space);
                resource->set_binding(bindDesc.BindPoint);
                resource->set_size(bindDesc.BindCount);
                resource->set_stages(stage);
                resource->set_dimension(RHI_TEX_DIMENSION_UNDEFINED);
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
                    resource->set_type(RHI_RESOURCE_TYPE_RW_BUFFER);
                }
                // Buffer<> is considered as DESCRIPTOR_TYPE_TEXTURE by default so we handle the case for Buffer<> here
                if(bindDesc.Type == D3D_SHADER_INPUT_TYPE::D3D_SIT_TEXTURE && bindDesc.Dimension == D3D_SRV_DIMENSION_BUFFER)
                {
                    resource->set_type(RHI_RESOURCE_TYPE_BUFFER);
                }
            }
        }
    }
}