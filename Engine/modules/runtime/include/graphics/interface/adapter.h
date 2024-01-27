#pragma once
#include "common/cyber_graphics_config.h"
#include "render_object.h"

namespace Cyber
{
    namespace RenderObject
    {
        struct CYBER_GRAPHICS_API AdapterDetail
        {
            uint32_t mUniformBufferAlignment;
            uint32_t mUploadBufferTextureAlignment;
            uint32_t mUploadBufferTextureRowAlignment;
            uint32_t mMaxVertexInputBindings;
            uint32_t mWaveLaneCount;
            uint32_t mHostVisibleVRamBudget;
            bool mSupportHostVisibleVRam : 1;
            bool mMultiDrawIndirect : 1;
            bool mSupportGeomShader : 1;
            bool mSupportTessellation : 1;
            bool mIsUma : 1;
            bool mIsVirtual : 1;
            bool mIsCpu : 1;
        };

        struct CYBER_GRAPHICS_API IAdapter
        {
            
        };

        template<typename EngineImplTraits>
        class AdapterBase : public RenderObjectBase<typename EngineImplTraits::AdapterInterface, typename EngineImplTraits::RenderDeviceImplType>
        {
        public:
            using RenderDeviceImplType = typename EngineImplTraits::RenderDeviceImplType;

            AdapterBase(RenderDeviceImplType* device);

            virtual ~AdapterBase() = default;
        protected:
            class IInstance* pInstance = nullptr;
        };
    }

}