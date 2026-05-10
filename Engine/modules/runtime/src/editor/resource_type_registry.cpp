#include "editor/resource_type_registry.h"

#include <algorithm>
#include <cctype>

namespace Cyber
{
    namespace Editor
    {
        namespace
        {
            std::string normalize_extension(std::string_view ext)
            {
                std::string out;
                out.reserve(ext.size() + 1);
                if (ext.empty() || ext.front() != '.')
                    out.push_back('.');
                for (char c : ext)
                    out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
                return out;
            }
        }

        ResourceTypeRegistry& ResourceTypeRegistry::get()
        {
            static ResourceTypeRegistry instance;
            return instance;
        }

        void ResourceTypeRegistry::register_type(const ResourceTypeInfo& info)
        {
            ResourceTypeInfo normalized = info;
            normalized.extension = normalize_extension(info.extension);

            auto it = std::find_if(m_types.begin(), m_types.end(),
                [&](const ResourceTypeInfo& existing) { return existing.extension == normalized.extension; });
            if (it != m_types.end())
                *it = normalized;
            else
                m_types.push_back(std::move(normalized));
        }

        bool ResourceTypeRegistry::is_registered(std::string_view ext) const
        {
            return find(ext) != nullptr;
        }

        const ResourceTypeInfo* ResourceTypeRegistry::find(std::string_view ext) const
        {
            if (ext.empty())
                return nullptr;
            const std::string needle = normalize_extension(ext);
            for (const auto& type : m_types)
            {
                if (type.extension == needle)
                    return &type;
            }
            return nullptr;
        }

        const char* ResourceTypeRegistry::category_label(ResourceCategory category) const
        {
            switch (category)
            {
                case ResourceCategory::Model:   return "Model";
                case ResourceCategory::Texture: return "Texture";
                case ResourceCategory::Shader:  return "Shader";
                case ResourceCategory::Unknown:
                default:                        return "Unknown";
            }
        }

        void ResourceTypeRegistry::seed_defaults()
        {
            auto& registry = ResourceTypeRegistry::get();

            // Avoid re-seeding if we've already populated.
            if (!registry.all_types().empty())
                return;

            const ImU32 model_tint   = IM_COL32(255, 180,  80, 255);
            const ImU32 texture_tint = IM_COL32( 80, 180, 255, 255);
            const ImU32 shader_tint  = IM_COL32(180, 255, 120, 255);

            // Models — see tools/ModelLoader (tinygltf)
            registry.register_type({ ".gltf", "glTF Model", ResourceCategory::Model, model_tint });

            // Textures — see tools/TextureLoader IMAGE_FILE_FORMAT + HDR fallback
            registry.register_type({ ".png",  "PNG Texture",  ResourceCategory::Texture, texture_tint });
            registry.register_type({ ".jpg",  "JPEG Texture", ResourceCategory::Texture, texture_tint });
            registry.register_type({ ".jpeg", "JPEG Texture", ResourceCategory::Texture, texture_tint });
            registry.register_type({ ".tiff", "TIFF Texture", ResourceCategory::Texture, texture_tint });
            registry.register_type({ ".tif",  "TIFF Texture", ResourceCategory::Texture, texture_tint });
            registry.register_type({ ".sgi",  "SGI Texture",  ResourceCategory::Texture, texture_tint });
            registry.register_type({ ".dds",  "DDS Texture",  ResourceCategory::Texture, texture_tint });
            registry.register_type({ ".ktx",  "KTX Texture",  ResourceCategory::Texture, texture_tint });
            registry.register_type({ ".hdr",  "HDR Texture",  ResourceCategory::Texture, texture_tint });

            // Shaders — HLSL sources compiled with DXC
            registry.register_type({ ".hlsl", "HLSL Shader",  ResourceCategory::Shader,  shader_tint });
        }
    }
}
