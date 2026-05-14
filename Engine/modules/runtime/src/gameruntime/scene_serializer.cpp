#include "gameruntime/scene_serializer.h"
#include "core/file_helper.hpp"
#include "log/Log.h"

#include "component/primitive.h"
#include "component/mesh_component.h"
#include "component/camera_component.h"
#include "component/directional_light_component.h"

#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <string>

namespace Cyber
{
    namespace SceneSerializer
    {
        namespace
        {
            using json = nlohmann::json;
            namespace fs = std::filesystem;
            using Component::Primitive;
            using Component::MeshComponent;
            using Component::CameraComponent;
            using Component::DirectionalLightComponent;
            using Component::ComponentType;

            fs::path resolve_for_read(const char* path)
            {
                fs::path p(path);
                if (p.is_absolute())
                    return p.lexically_normal();

                fs::path project_rooted = fs::path(Core::FileHelper::get_project_root()) / p;
                project_rooted = project_rooted.lexically_normal();
                if (fs::exists(project_rooted))
                    return project_rooted;

                fs::path cwd_rooted = (fs::current_path() / p).lexically_normal();
                if (fs::exists(cwd_rooted))
                    return cwd_rooted;

                return project_rooted;
            }

            fs::path resolve_for_write(const char* path)
            {
                fs::path p(path);
                if (p.is_absolute())
                    return p.lexically_normal();
                return (fs::path(Core::FileHelper::get_project_root()) / p).lexically_normal();
            }

            eastl::string to_project_relative(const eastl::string& in)
            {
                if (in.empty())
                    return in;
                fs::path p(in.c_str());
                if (!p.is_absolute())
                    return in;
                std::error_code ec;
                fs::path rel = fs::relative(p, fs::path(Core::FileHelper::get_project_root()), ec);
                if (ec || rel.empty() || rel.native().front() == '.')
                    return in;
                return eastl::string(rel.generic_string().c_str());
            }

            json vec3_to_json(const float3& v)   { return json::array({v.x, v.y, v.z}); }
            json vec4_to_json(const float4& v)   { return json::array({v.x, v.y, v.z, v.w}); }

            float3 vec3_from_json(const json& j, const float3& fallback)
            {
                if (!j.is_array() || j.size() < 3)
                    return fallback;
                return float3(j[0].get<float>(), j[1].get<float>(), j[2].get<float>());
            }

            quaternion_f quat_from_json(const json& j)
            {
                if (!j.is_array() || j.size() < 4)
                    return quaternion_f(0.0f, 0.0f, 0.0f, 1.0f);
                return quaternion_f(
                    j[0].get<float>(), j[1].get<float>(),
                    j[2].get<float>(), j[3].get<float>());
            }

            void write_transform(json& out, const Primitive& p)
            {
                out["position"] = vec3_to_json(p.position);
                out["rotation"] = vec4_to_json(p.rotation.q);
                out["scale"]    = vec3_to_json(p.scale);
            }

            void read_transform(const json& in, Primitive& p)
            {
                if (in.contains("position")) p.position = vec3_from_json(in["position"], p.position);
                if (in.contains("rotation")) p.rotation = quat_from_json(in["rotation"]);
                if (in.contains("scale"))    p.scale    = vec3_from_json(in["scale"],    p.scale);
            }

            // Writes one component as v2-format JSON. Returns false if the
            // component type is unsupported — caller skips it.
            bool write_component(json& out, const Primitive& comp)
            {
                json transform;
                write_transform(transform, comp);

                switch (comp.type())
                {
                    case ComponentType::Mesh:
                    {
                        const auto& mc = static_cast<const MeshComponent&>(comp);
                        out["type"]      = "MeshComponent";
                        out["transform"] = std::move(transform);
                        out["enabled"]   = mc.enabled;
                        if (!mc.name.empty())
                            out["name"] = std::string(mc.name.c_str());
                        out["model_resource"] = std::string(to_project_relative(mc.model_resource).c_str());
                        return true;
                    }
                    case ComponentType::Camera:
                    {
                        const auto& cc = static_cast<const CameraComponent&>(comp);
                        out["type"]      = "CameraComponent";
                        out["transform"] = std::move(transform);
                        out["enabled"]   = cc.enabled;
                        if (!cc.name.empty())
                            out["name"] = std::string(cc.name.c_str());
                        out["yaw"]   = cc.get_yaw();
                        out["pitch"] = cc.get_pitch();
                        out["fov"]   = cc.fov_deg;
                        out["near"]  = cc.near_z;
                        out["far"]   = cc.far_z;
                        return true;
                    }
                    case ComponentType::DirectionalLight:
                    {
                        const auto& dl = static_cast<const DirectionalLightComponent&>(comp);
                        out["type"]      = "DirectionalLightComponent";
                        out["transform"] = std::move(transform);
                        out["enabled"]   = dl.enabled;
                        if (!dl.name.empty())
                            out["name"] = std::string(dl.name.c_str());
                        out["color"]     = vec3_to_json(dl.color);
                        out["intensity"] = dl.intensity;
                        return true;
                    }
                    default:
                        return false;
                }
            }

            // Constructs a component from its v2 JSON form. Returns nullptr on
            // unknown type strings.
            Scope<Primitive> read_component(const json& jc)
            {
                if (!jc.contains("type") || !jc["type"].is_string())
                    return {};

                std::string type = jc["type"].get<std::string>();
                Scope<Primitive> out;

                if (type == "MeshComponent")
                {
                    auto mc = Scope<MeshComponent>(new MeshComponent());
                    if (jc.contains("transform")) read_transform(jc["transform"], *mc);
                    if (jc.contains("enabled"))   mc->enabled = jc["enabled"].get<bool>();
                    if (jc.contains("name"))      mc->name    = jc["name"].get<std::string>().c_str();
                    if (jc.contains("model_resource"))
                        mc->model_resource = jc["model_resource"].get<std::string>().c_str();
                    out = Scope<Primitive>(mc.release());
                }
                else if (type == "CameraComponent")
                {
                    auto cc = Scope<CameraComponent>(new CameraComponent());
                    if (jc.contains("transform")) read_transform(jc["transform"], *cc);
                    if (jc.contains("enabled"))   cc->enabled = jc["enabled"].get<bool>();
                    if (jc.contains("name"))      cc->name    = jc["name"].get<std::string>().c_str();
                    if (jc.contains("yaw"))       cc->set_yaw(jc["yaw"].get<float>());
                    if (jc.contains("pitch"))     cc->set_pitch(jc["pitch"].get<float>());
                    if (jc.contains("fov"))       cc->fov_deg = jc["fov"].get<float>();
                    if (jc.contains("near"))      cc->near_z  = jc["near"].get<float>();
                    if (jc.contains("far"))       cc->far_z   = jc["far"].get<float>();
                    out = Scope<Primitive>(cc.release());
                }
                else if (type == "DirectionalLightComponent")
                {
                    auto dl = Scope<DirectionalLightComponent>(new DirectionalLightComponent());
                    if (jc.contains("transform")) read_transform(jc["transform"], *dl);
                    if (jc.contains("enabled"))   dl->enabled = jc["enabled"].get<bool>();
                    if (jc.contains("name"))      dl->name    = jc["name"].get<std::string>().c_str();
                    if (jc.contains("color"))     dl->color   = vec3_from_json(jc["color"], dl->color);
                    if (jc.contains("intensity")) dl->intensity = jc["intensity"].get<float>();
                    out = Scope<Primitive>(dl.release());
                }
                else
                {
                    CB_WARN("SceneSerializer: unknown component type '%s', skipping", type.c_str());
                }

                return out;
            }

            // v1 migration: old format stored transform + mesh.source on the
            // node itself, plus camera/sun blocks at the root. Build equivalent
            // v2 components so older scene files keep loading.
            void migrate_v1_node(World& world, const json& jn, uint32_t& highest_id)
            {
                SceneNode node;
                if (jn.contains("id"))   node.id = jn["id"].get<uint32_t>();
                if (jn.contains("name")) node.name = jn["name"].get<std::string>().c_str();
                if (node.id > highest_id) highest_id = node.id;

                auto mesh = Scope<MeshComponent>(new MeshComponent());
                if (jn.contains("transform")) read_transform(jn["transform"], *mesh);
                if (jn.contains("mesh") && jn["mesh"].is_object() && jn["mesh"].contains("source"))
                    mesh->model_resource = jn["mesh"]["source"].get<std::string>().c_str();
                node.components.push_back(Scope<Primitive>(mesh.release()));

                world.add_node(std::move(node));
            }

            void migrate_v1_camera(World& world, const json& jc)
            {
                SceneNode cam_node;
                cam_node.name = "MainCamera";

                auto cc = Scope<CameraComponent>(new CameraComponent());
                if (jc.contains("position")) cc->position = vec3_from_json(jc["position"], cc->position);
                if (jc.contains("yaw"))      cc->set_yaw(jc["yaw"].get<float>());
                if (jc.contains("pitch"))    cc->set_pitch(jc["pitch"].get<float>());
                if (jc.contains("fov"))      cc->fov_deg = jc["fov"].get<float>();
                if (jc.contains("near"))     cc->near_z  = jc["near"].get<float>();
                if (jc.contains("far"))      cc->far_z   = jc["far"].get<float>();
                cam_node.components.push_back(Scope<Primitive>(cc.release()));

                world.add_node(std::move(cam_node));
            }

            void migrate_v1_sun(World& world, const json& js)
            {
                SceneNode sun_node;
                sun_node.name = "Sun";

                auto dl = Scope<DirectionalLightComponent>(new DirectionalLightComponent());
                if (js.contains("direction"))
                {
                    float3 dir = vec3_from_json(js["direction"], float3(0.0f, -1.0f, 0.0f));
                    dl->set_direction(dir);
                }
                if (js.contains("color"))     dl->color     = vec3_from_json(js["color"], dl->color);
                if (js.contains("intensity")) dl->intensity = js["intensity"].get<float>();
                sun_node.components.push_back(Scope<Primitive>(dl.release()));

                world.add_node(std::move(sun_node));
            }
        }

        bool save(World& world, const char* path)
        {
            if (!path || !*path)
            {
                CB_ERROR("SceneSerializer::save called with empty path");
                return false;
            }

            json root;
            root["version"] = 2;
            root["name"] = path;

            json nodes = json::array();
            for (const auto& node : world.get_nodes())
            {
                json n;
                n["id"] = node.id;
                n["name"] = std::string(node.name.c_str());
                n["parent_id"] = node.parent_id;

                json comps = json::array();
                for (const auto& comp : node.components)
                {
                    if (!comp)
                        continue;
                    json jc;
                    if (write_component(jc, *comp))
                        comps.push_back(std::move(jc));
                }
                n["components"] = std::move(comps);
                nodes.push_back(std::move(n));
            }
            root["nodes"] = std::move(nodes);

            fs::path out_path = resolve_for_write(path);
            std::error_code ec;
            if (out_path.has_parent_path())
                fs::create_directories(out_path.parent_path(), ec);

            std::ofstream ofs(out_path);
            if (!ofs.is_open())
            {
                CB_ERROR("SceneSerializer::save failed to open %s", out_path.string().c_str());
                return false;
            }
            ofs << root.dump(2);
            ofs.close();

            world.set_source_path(eastl::string(path));
            world.set_dirty(false);
            CB_INFO("Saved scene to %s", out_path.string().c_str());
            return true;
        }

        bool load(World& world, const char* path)
        {
            if (!path || !*path)
            {
                CB_ERROR("SceneSerializer::load called with empty path");
                return false;
            }

            world.clear();

            fs::path in_path = resolve_for_read(path);
            if (!fs::exists(in_path))
            {
                CB_ERROR("SceneSerializer::load: file not found: %s", in_path.string().c_str());
                return false;
            }

            std::ifstream ifs(in_path);
            if (!ifs.is_open())
            {
                CB_ERROR("SceneSerializer::load: cannot open %s", in_path.string().c_str());
                return false;
            }

            json root;
            try
            {
                ifs >> root;
            }
            catch (const std::exception& e)
            {
                CB_ERROR("SceneSerializer::load: json parse error (%s)", e.what());
                return false;
            }

            if (!root.is_object())
            {
                CB_ERROR("SceneSerializer::load: root is not a json object");
                return false;
            }

            int version = 1;
            if (root.contains("version") && root["version"].is_number_integer())
                version = root["version"].get<int>();

            uint32_t highest_id = 0;

            if (version >= 2)
            {
                // v2+: nodes carry a components array.
                if (root.contains("nodes") && root["nodes"].is_array())
                {
                    for (const auto& jn : root["nodes"])
                    {
                        SceneNode node;
                        if (jn.contains("id"))        node.id        = jn["id"].get<uint32_t>();
                        if (jn.contains("name"))      node.name      = jn["name"].get<std::string>().c_str();
                        if (jn.contains("parent_id")) node.parent_id = jn["parent_id"].get<uint32_t>();
                        if (node.id > highest_id) highest_id = node.id;

                        if (jn.contains("components") && jn["components"].is_array())
                        {
                            for (const auto& jc : jn["components"])
                            {
                                Scope<Primitive> comp = read_component(jc);
                                if (comp)
                                    node.components.push_back(std::move(comp));
                            }
                        }
                        world.add_node(std::move(node));
                    }
                }
            }
            else
            {
                // v1 migration path.
                CB_INFO("SceneSerializer: migrating v1 scene %s to v2 in-memory", in_path.string().c_str());

                if (root.contains("nodes") && root["nodes"].is_array())
                {
                    for (const auto& jn : root["nodes"])
                        migrate_v1_node(world, jn, highest_id);
                }
                if (root.contains("camera") && root["camera"].is_object())
                    migrate_v1_camera(world, root["camera"]);
                if (root.contains("sun") && root["sun"].is_object())
                    migrate_v1_sun(world, root["sun"]);
            }

            world.set_source_path(eastl::string(path));
            world.set_dirty(false);
            (void)highest_id;

            CB_INFO("Loaded scene from %s (%zu node(s))", in_path.string().c_str(), (size_t)world.get_nodes().size());
            return true;
        }
    }
}
