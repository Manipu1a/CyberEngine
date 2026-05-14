#pragma once
#include "cyber_game.config.h"
#include "platform/configure.h"
#include "math/basic_math.hpp"
#include "common/smart_ptr.h"
#include "core/Core.h"
#include "component/primitive.h"
#include <EASTL/string.h>
#include <EASTL/vector.h>

namespace Cyber
{
    struct CYBER_GAME_API SceneNode
    {
        uint32_t id = 0;
        eastl::string name;
        uint32_t parent_id = 0;   // 0 == root

        // Components that belong to this node. Ownership is the node.
        // Transforms live on the components themselves (Primitive::position/
        // rotation/scale) rather than on the node — one node can hold a
        // mesh + camera + light side-by-side, each with its own transform.
        eastl::vector<Cyber::Scope<Component::Primitive>> components;

        SceneNode() = default;
        SceneNode(const SceneNode&) = delete;             // non-copyable (unique_ptr members)
        SceneNode& operator=(const SceneNode&) = delete;
        SceneNode(SceneNode&&) = default;
        SceneNode& operator=(SceneNode&&) = default;

        // Deep clone: duplicates each component via Primitive::clone(), so
        // id is left 0 (World assigns a fresh id on add) and runtime GPU
        // state on MeshComponents is reset so callers can re-enqueue loads.
        SceneNode deep_clone() const
        {
            SceneNode copy;
            copy.name      = name + "_copy";
            copy.parent_id = parent_id;
            copy.components.reserve(components.size());
            for (const auto& comp : components)
            {
                if (comp)
                    copy.components.push_back(comp->clone());
            }
            return copy;
        }

        // Typed lookup — returns the first component matching T's ComponentType,
        // or nullptr when the node has none of that kind. Requires T to derive
        // from Component::Primitive and expose ComponentType via type().
        template <typename T>
        T* find_component()
        {
            for (auto& comp : components)
            {
                if (auto* casted = dynamic_cast<T*>(comp.get()))
                    return casted;
            }
            return nullptr;
        }

        template <typename T>
        const T* find_component() const
        {
            for (const auto& comp : components)
            {
                if (auto* casted = dynamic_cast<const T*>(comp.get()))
                    return casted;
            }
            return nullptr;
        }
    };
}
