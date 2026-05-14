#pragma once
#include "cyber_runtime.config.h"
#include "math/basic_math.hpp"
#include <EASTL/string.h>
#include <EASTL/vector.h>
#include <EASTL/hash_map.h>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace Cyber
{
    namespace Editor
    {
        enum class PropertyType : uint8_t
        {
            Bool,
            Int,
            Float,
            Float3,
            Color3,           // stored as float3, drawn as ColorEdit3
            QuaternionEuler,  // stored as quaternion_f, drawn as euler DragFloat3
            String,           // eastl::string
        };

        struct Property
        {
            const char*  name        = "";
            PropertyType type        = PropertyType::Float;
            size_t       offset      = 0;
            bool         readonly    = false;
            float        min_val     = -3.4028235e+38f;
            float        max_val     =  3.4028235e+38f;
            float        drag_speed  = 0.1f;
            bool         uniform_scale = false;
            // Optional post-edit callback. Invoked after the field has been
            // mutated. Cast `instance` to the concrete component type inside.
            void       (*on_changed)(void* instance) = nullptr;
        };

        struct ComponentProps
        {
            eastl::string                 type_name;
            eastl::string                 parent_type_name;
            eastl::vector<Property>       fields;
        };

        // Per-frame state threaded through property drawing. Owns caches
        // that must survive between frames (e.g. the quaternion-to-euler
        // cache used by the rotation drawer to avoid single-axis drift).
        struct CYBER_RUNTIME_API PropertyDrawContext
        {
            struct EulerCacheEntry
            {
                void*                     instance  = nullptr;
                size_t                    offset    = 0;
                bool                      first_use = true;
                Math::Quaternion<float>   last_quat{0.0f, 0.0f, 0.0f, 1.0f};
                float                     euler[3]  = {0.0f, 0.0f, 0.0f};
            };
            struct Float3LockEntry
            {
                void*                     instance  = nullptr;
                size_t                    offset    = 0;
                bool                      locked    = false;
            };
            eastl::vector<EulerCacheEntry> euler_cache;
            eastl::vector<Float3LockEntry> float3_lock_cache;

            EulerCacheEntry* find_or_create_euler(void* inst, size_t off);
            Float3LockEntry* find_or_create_float3_lock(void* inst, size_t off);
            void             clear();
        };

        class CYBER_RUNTIME_API PropertyRegistry
        {
        public:
            static PropertyRegistry& get();

            // Fluent builder. Returned by register_component(). Methods mutate
            // the registry entry in place; copies share the pointer, so the
            // usual `static auto reg = registry.register_component(...).field(...)...`
            // pattern records every chained call even though the temporary
            // dies at the end of the full expression.
            class CYBER_RUNTIME_API Builder
            {
                ComponentProps* m_props   = nullptr;
                Property*       m_current = nullptr;
            public:
                Builder() = default;
                explicit Builder(ComponentProps* p) : m_props(p) {}

                Builder& inherits(const char* parent_type_name);

                template <typename Class, typename FieldT>
                Builder& field(const char* name, FieldT Class::*member)
                {
                    // Offset-from-member-pointer trick. 0x100 avoids the
                    // null-deref UB the nullptr variant implies. For our
                    // single-inheritance Primitive hierarchy the offset is
                    // stable across base/derived pointers.
                    constexpr std::size_t kDummy = 0x10000;
                    const std::size_t off = reinterpret_cast<std::size_t>(
                        &(reinterpret_cast<Class*>(kDummy)->*member)) - kDummy;
                    return add_field(name, deduce_type<FieldT>(), off);
                }

                Builder& range(float lo, float hi);
                Builder& min(float v);
                Builder& max(float v);
                Builder& speed(float s);
                Builder& readonly();
                Builder& uniform_scale();
                Builder& as_color();
                Builder& as_euler();
                Builder& on_changed(void (*cb)(void*));

            private:
                Builder& add_field(const char* name, PropertyType type, std::size_t offset);

                template <typename FieldT>
                static PropertyType deduce_type()
                {
                    using T = std::remove_cv_t<std::remove_reference_t<FieldT>>;
                    if constexpr (std::is_same_v<T, bool>) return PropertyType::Bool;
                    else if constexpr (std::is_integral_v<T>) return PropertyType::Int;
                    else if constexpr (std::is_same_v<T, float>) return PropertyType::Float;
                    else if constexpr (std::is_same_v<T, float3>) return PropertyType::Float3;
                    else if constexpr (std::is_same_v<T, Math::Quaternion<float>>) return PropertyType::QuaternionEuler;
                    else if constexpr (std::is_same_v<T, eastl::string>) return PropertyType::String;
                    else return PropertyType::Float;
                }
            };

            Builder register_component(const char* type_name);
            const ComponentProps* find(const char* type_name) const;

        private:
            eastl::hash_map<eastl::string, ComponentProps> m_components;
        };

        // Draw all registered fields of `type_name` (walking up the
        // inheritance chain, parent first). Returns true if anything was
        // edited this frame.
        CYBER_RUNTIME_API bool draw_component_properties(
            void* instance, const char* type_name, PropertyDrawContext& ctx);
    }
}

#define CYBER_CONCAT_IMPL_(a, b) a##b
#define CYBER_CONCAT_(a, b) CYBER_CONCAT_IMPL_(a, b)

// Declare a static initializer that registers the component with the
// property registry. Chain .inherits/.field/.range/... calls after it.
// Example:
//   CYBER_REGISTER_COMPONENT(CameraComponent, "CameraComponent")
//       .inherits("Primitive")
//       .field("fov_deg", &CameraComponent::fov_deg).range(1.0f, 179.0f);
#define CYBER_REGISTER_COMPONENT(Type, Name) \
    static auto CYBER_CONCAT_(_cyber_prop_reg_, __COUNTER__) = \
        ::Cyber::Editor::PropertyRegistry::get().register_component(Name)
