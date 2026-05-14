#pragma once
#include "cyber_game.config.h"
#include "platform/configure.h"
#include <EASTL/vector.h>
#include <EASTL/string.h>
#include "scene_node.h"

CYBER_BEGIN_NAMESPACE(Cyber)

class CYBER_GAME_API World
{
public:
    World();

    // Non-copyable (holds a vector of non-copyable SceneNodes).
    World(const World&) = delete;
    World& operator=(const World&) = delete;
    World(World&&) = default;
    World& operator=(World&&) = default;

    // --- Node API ---
    uint32_t add_node(SceneNode node);         // returns node id
    bool     remove_node(uint32_t id);         // returns true if removed
    SceneNode* find_node(uint32_t id);
    const SceneNode* find_node(uint32_t id) const;

    // Convenience: create an empty named node and return a pointer to it.
    SceneNode* add_empty_node(const eastl::string& name);

    const eastl::vector<SceneNode>& get_nodes() const { return m_nodes; }
    eastl::vector<SceneNode>&       get_nodes_mutable() { return m_nodes; }

    const eastl::string& source_path() const { return m_source_path; }
    void set_source_path(eastl::string path) { m_source_path = std::move(path); }

    bool is_dirty() const { return m_dirty; }
    void set_dirty(bool v) { m_dirty = v; }

    void clear();

    // --- Pending async load queue (drained by the active sample) ---
    // Records a target (node_id, component_index) + the resource path so the
    // sample can load it on the main thread later.
    struct PendingLoad
    {
        uint32_t      node_id = 0;
        uint32_t      component_index = 0;
        eastl::string model_resource;
    };
    void enqueue_pending_load(uint32_t node_id, uint32_t component_index, eastl::string path)
    {
        PendingLoad p;
        p.node_id = node_id;
        p.component_index = component_index;
        p.model_resource = std::move(path);
        m_pending_loads.push_back(std::move(p));
    }
    eastl::vector<PendingLoad>& pending_loads() { return m_pending_loads; }

    // --- Component visitors ---
    // fn signature: void(SceneNode&, Component::Primitive&, uint32_t index)
    template <typename F>
    void for_each_component(F&& fn)
    {
        for (auto& node : m_nodes)
        {
            for (uint32_t i = 0; i < node.components.size(); ++i)
            {
                if (node.components[i])
                    fn(node, *node.components[i], i);
            }
        }
    }

    // fn signature: void(SceneNode&, T&, uint32_t index) — only invoked for
    // components where a dynamic_cast to T succeeds.
    template <typename T, typename F>
    void for_each_component_of(F&& fn)
    {
        for (auto& node : m_nodes)
        {
            for (uint32_t i = 0; i < node.components.size(); ++i)
            {
                if (T* typed = dynamic_cast<T*>(node.components[i].get()))
                    fn(node, *typed, i);
            }
        }
    }

    template <typename T, typename F>
    void for_each_component_of(F&& fn) const
    {
        for (const auto& node : m_nodes)
        {
            for (uint32_t i = 0; i < node.components.size(); ++i)
            {
                if (const T* typed = dynamic_cast<const T*>(node.components[i].get()))
                    fn(node, *typed, i);
            }
        }
    }

protected:
    eastl::vector<SceneNode>    m_nodes;
    eastl::string               m_source_path;
    bool                        m_dirty = false;
    uint32_t                    m_next_id = 1;
    eastl::vector<PendingLoad>  m_pending_loads;
};

CYBER_END_NAMESPACE
