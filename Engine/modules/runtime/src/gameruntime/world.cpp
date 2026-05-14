#include "gameruntime/world.h"

CYBER_BEGIN_NAMESPACE(Cyber)

World::World() = default;

uint32_t World::add_node(SceneNode node)
{
    if (node.id == 0)
        node.id = m_next_id++;
    else
        m_next_id = (node.id >= m_next_id) ? node.id + 1 : m_next_id;

    const uint32_t id = node.id;
    m_nodes.push_back(std::move(node));
    m_dirty = true;
    return id;
}

bool World::remove_node(uint32_t id)
{
    for (auto it = m_nodes.begin(); it != m_nodes.end(); ++it)
    {
        if (it->id == id)
        {
            m_nodes.erase(it);
            m_dirty = true;
            return true;
        }
    }
    return false;
}

SceneNode* World::find_node(uint32_t id)
{
    for (auto& n : m_nodes)
        if (n.id == id)
            return &n;
    return nullptr;
}

const SceneNode* World::find_node(uint32_t id) const
{
    for (const auto& n : m_nodes)
        if (n.id == id)
            return &n;
    return nullptr;
}

SceneNode* World::add_empty_node(const eastl::string& name)
{
    SceneNode node;
    node.name = name;
    uint32_t id = add_node(std::move(node));
    return find_node(id);
}

void World::clear()
{
    m_nodes.clear();
    m_pending_loads.clear();
    m_source_path.clear();
    m_dirty = false;
    m_next_id = 1;
}

CYBER_END_NAMESPACE
