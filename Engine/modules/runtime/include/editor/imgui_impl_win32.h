#pragma once
#include "editor.h"

namespace Cyber
{
    namespace Editor
    {
        class Editor_Impl_Win32 : public Editor
        {
        public:
            static Editor_Impl_Win32* create(const EditorCreateInfo& createInfo)
            {
                return new Editor_Impl_Win32(createInfo);
            }
            
            Editor_Impl_Win32(const EditorCreateInfo& createInfo);
            ~Editor_Impl_Win32();

            virtual void initialize(RenderObject::IRenderDevice* device, HWND hwnd) override;
            virtual void new_frame(uint32_t renderSurfaceWidth, uint32_t renderSurfaceHeight) override;
        };
    }
}