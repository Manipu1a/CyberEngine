#pragma once
#include "cyber_runtime.config.h"
#include "imgui/imgui.h"

#include <string>
#include <string_view>
#include <vector>
#include <cstdint>

namespace Cyber
{
    namespace Editor
    {
        enum class ResourceCategory : uint8_t
        {
            Unknown = 0,
            Model,
            Texture,
            Shader,
        };

        struct ResourceTypeInfo
        {
            std::string      extension;    // lowercase, leading dot (e.g. ".gltf")
            std::string      display_name; // e.g. "glTF Model"
            ResourceCategory category = ResourceCategory::Unknown;
            ImU32            tint_color = IM_COL32(200, 200, 200, 255);
        };

        // Central registry of file-extension → resource-type mappings the
        // editor recognises. Loaders (or the editor itself, during startup)
        // register their supported extensions here so that UI surfaces like
        // the Content Browser can filter by "is this something the engine
        // knows how to load?".
        class CYBER_RUNTIME_API ResourceTypeRegistry
        {
        public:
            static ResourceTypeRegistry& get();

            // Registers (or overwrites) a resource type by extension.
            // Extension is canonicalised to lowercase with a leading '.'.
            void register_type(const ResourceTypeInfo& info);

            bool                    is_registered(std::string_view ext) const;
            const ResourceTypeInfo* find(std::string_view ext) const;
            const std::vector<ResourceTypeInfo>& all_types() const { return m_types; }

            // Seeds the built-in types that the engine currently knows how
            // to load (models, textures, shaders). Idempotent.
            static void seed_defaults();

            const char* category_label(ResourceCategory category) const;

        private:
            ResourceTypeRegistry() = default;

            std::vector<ResourceTypeInfo> m_types;
        };
    }
}
