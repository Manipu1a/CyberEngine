#include "editor/property_registry.h"
#include "editor/resource_type_registry.h"
#include "asset/mesh_importer.h"
#include "core/file_helper.hpp"
#include "log/Log.h"
#include "imgui/imgui.h"
#include "imgui/ImGuizmo.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <filesystem>
#include <string>

namespace Cyber
{
    namespace Editor
    {
        PropertyDrawContext::EulerCacheEntry*
        PropertyDrawContext::find_or_create_euler(void* inst, size_t off)
        {
            for (auto& e : euler_cache)
            {
                if (e.instance == inst && e.offset == off)
                    return &e;
            }
            EulerCacheEntry entry{};
            entry.instance = inst;
            entry.offset   = off;
            euler_cache.push_back(entry);
            return &euler_cache.back();
        }

        PropertyDrawContext::Float3LockEntry*
        PropertyDrawContext::find_or_create_float3_lock(void* inst, size_t off)
        {
            for (auto& e : float3_lock_cache)
            {
                if (e.instance == inst && e.offset == off)
                    return &e;
            }
            Float3LockEntry entry{};
            entry.instance = inst;
            entry.offset   = off;
            float3_lock_cache.push_back(entry);
            return &float3_lock_cache.back();
        }

        void PropertyDrawContext::clear()
        {
            euler_cache.clear();
            float3_lock_cache.clear();
        }

        PropertyRegistry& PropertyRegistry::get()
        {
            static PropertyRegistry instance;
            return instance;
        }

        PropertyRegistry::Builder PropertyRegistry::register_component(const char* type_name)
        {
            const eastl::string key(type_name);
            bool known = false;
            for (const auto& existing : m_component_order)
            {
                if (existing == key)
                {
                    known = true;
                    break;
                }
            }
            if (!known)
                m_component_order.push_back(key);

            ComponentProps& props = m_components[key];
            props = ComponentProps{};
            props.type_name = type_name;
            return Builder(&props);
        }

        const ComponentProps* PropertyRegistry::find(const char* type_name) const
        {
            auto it = m_components.find(eastl::string(type_name));
            if (it == m_components.end())
                return nullptr;
            return &it->second;
        }

        PropertyRegistry::Builder& PropertyRegistry::Builder::inherits(const char* parent_type_name)
        {
            if (m_props) m_props->parent_type_name = parent_type_name;
            return *this;
        }

        PropertyRegistry::Builder& PropertyRegistry::Builder::display(const char* display_name)
        {
            if (m_current)
                m_current->display_name = display_name;
            else if (m_current_function)
                m_current_function->display_name = display_name;
            else if (m_props)
                m_props->display_name = display_name;
            return *this;
        }

        PropertyRegistry::Builder& PropertyRegistry::Builder::abstract()
        {
            if (m_props) m_props->is_abstract = true;
            return *this;
        }

        PropertyRegistry::Builder& PropertyRegistry::Builder::add_field(
            const char* name, PropertyType type, std::size_t offset)
        {
            if (!m_props) return *this;
            Property p{};
            p.name   = name;
            p.type   = type;
            p.offset = offset;
            m_props->fields.push_back(p);
            m_current = &m_props->fields.back();
            m_current_function = nullptr;
            return *this;
        }

        PropertyRegistry::Builder& PropertyRegistry::Builder::range(float lo, float hi)
        {
            if (m_current) { m_current->min_val = lo; m_current->max_val = hi; }
            return *this;
        }
        PropertyRegistry::Builder& PropertyRegistry::Builder::min(float v)
        {
            if (m_current) m_current->min_val = v;
            return *this;
        }
        PropertyRegistry::Builder& PropertyRegistry::Builder::max(float v)
        {
            if (m_current) m_current->max_val = v;
            return *this;
        }
        PropertyRegistry::Builder& PropertyRegistry::Builder::speed(float s)
        {
            if (m_current) m_current->drag_speed = s;
            return *this;
        }
        PropertyRegistry::Builder& PropertyRegistry::Builder::readonly()
        {
            if (m_current)
            {
                m_current->readonly = true;
                m_current->flags |= PropertyFlag_ReadOnly;
            }
            return *this;
        }
        PropertyRegistry::Builder& PropertyRegistry::Builder::serializable()
        {
            if (m_current) m_current->flags |= PropertyFlag_Serializable;
            return *this;
        }
        PropertyRegistry::Builder& PropertyRegistry::Builder::asset(PropertyAssetKind kind)
        {
            if (m_current) m_current->asset_kind = kind;
            return *this;
        }
        PropertyRegistry::Builder& PropertyRegistry::Builder::uniform_scale()
        {
            if (m_current) m_current->uniform_scale = true;
            return *this;
        }
        PropertyRegistry::Builder& PropertyRegistry::Builder::as_color()
        {
            if (m_current) m_current->type = PropertyType::Color3;
            return *this;
        }
        PropertyRegistry::Builder& PropertyRegistry::Builder::as_euler()
        {
            if (m_current) m_current->type = PropertyType::QuaternionEuler;
            return *this;
        }
        PropertyRegistry::Builder& PropertyRegistry::Builder::on_changed(void (*cb)(void*))
        {
            if (m_current) m_current->on_changed = cb;
            return *this;
        }
        PropertyRegistry::Builder& PropertyRegistry::Builder::function(const char* name, const char* signature)
        {
            if (!m_props) return *this;
            FunctionDesc f{};
            f.name = name;
            f.signature = signature ? signature : "";
            m_props->functions.push_back(f);
            m_current = nullptr;
            m_current_function = &m_props->functions.back();
            return *this;
        }
        PropertyRegistry::Builder& PropertyRegistry::Builder::script_callable()
        {
            if (m_current_function) m_current_function->flags |= FunctionFlag_ScriptCallable;
            return *this;
        }
        PropertyRegistry::Builder& PropertyRegistry::Builder::const_function()
        {
            if (m_current_function) m_current_function->flags |= FunctionFlag_Const;
            return *this;
        }

        namespace
        {
            const char* property_label(const Property& p)
            {
                return p.display_name && p.display_name[0] ? p.display_name : p.name;
            }

            bool property_readonly(const Property& p)
            {
                return p.readonly || ((p.flags & PropertyFlag_ReadOnly) != 0);
            }

            ResourceCategory asset_category(PropertyAssetKind kind)
            {
                switch (kind)
                {
                    case PropertyAssetKind::Model:   return ResourceCategory::Model;
                    case PropertyAssetKind::Texture: return ResourceCategory::Texture;
                    case PropertyAssetKind::Scene:   return ResourceCategory::Scene;
                    case PropertyAssetKind::None:
                    default:                         return ResourceCategory::Unknown;
                }
            }

            std::string lowercase(std::string s)
            {
                std::transform(s.begin(), s.end(), s.begin(),
                    [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                return s;
            }

            bool asset_path_matches_property(const Property& p, const char* utf8_path)
            {
                if (p.asset_kind == PropertyAssetKind::None)
                    return true;
                if (!utf8_path || !*utf8_path)
                    return false;

                const std::string ext = lowercase(std::filesystem::path(utf8_path).extension().string());
                const ResourceTypeInfo* info = ResourceTypeRegistry::get().find(ext);
                if (!info || info->category != asset_category(p.asset_kind))
                    return false;

                if (p.asset_kind == PropertyAssetKind::Model)
                {
                    if (ext != ".meshasset")
                    {
                        CB_WARN("Rejected model asset {}: MeshComponent model_resource only accepts .meshasset files",
                                utf8_path);
                        return false;
                    }

                    MeshEditorAssetInfo mesh_info;
                    if (!MeshImporter::ReadInfo(utf8_path, mesh_info))
                    {
                        CB_WARN("Rejected invalid mesh asset: {}", utf8_path);
                        return false;
                    }
                    if (!mesh_info.isCooked)
                    {
                        CB_WARN("Rejected legacy mesh asset {}: reimport is required", utf8_path);
                        return false;
                    }

                    return true;
                }

                return true;
            }

            eastl::string to_project_relative_asset_path(const char* utf8_path)
            {
                if (!utf8_path || !*utf8_path)
                    return {};

                namespace fs = std::filesystem;
                fs::path p(utf8_path);
                std::error_code ec;
                fs::path rel = fs::relative(p, fs::path(Core::FileHelper::get_project_root()), ec);
                if (!ec && !rel.empty() && rel.native().front() != '.')
                    return eastl::string(rel.generic_string().c_str());
                return eastl::string(utf8_path);
            }

            float clamp_property_value(float value, const Property& p)
            {
                if (value < p.min_val) return p.min_val;
                if (value > p.max_val) return p.max_val;
                return value;
            }

            void clamp_property_float3(float values[3], const Property& p)
            {
                values[0] = clamp_property_value(values[0], p);
                values[1] = clamp_property_value(values[1], p);
                values[2] = clamp_property_value(values[2], p);
            }

            int first_changed_axis(const float before[3], const float after[3])
            {
                for (int i = 0; i < 3; ++i)
                {
                    if (before[i] != after[i])
                        return i;
                }
                return -1;
            }

            void apply_uniform_scale_edit(float values[3], const float before[3], const Property& p)
            {
                const int axis = first_changed_axis(before, values);
                if (axis < 0)
                    return;

                const float old_axis_value = before[axis];
                const float new_axis_value = clamp_property_value(values[axis], p);
                if (old_axis_value > -0.000001f && old_axis_value < 0.000001f)
                {
                    values[0] = new_axis_value;
                    values[1] = new_axis_value;
                    values[2] = new_axis_value;
                    return;
                }

                const float ratio = new_axis_value / old_axis_value;
                values[0] = clamp_property_value(before[0] * ratio, p);
                values[1] = clamp_property_value(before[1] * ratio, p);
                values[2] = clamp_property_value(before[2] * ratio, p);
                values[axis] = new_axis_value;
            }

            bool draw_float3_drag(void* instance, const Property& p, PropertyDrawContext& ctx, float values[3])
            {
                const char* label = property_label(p);
                if (!p.uniform_scale)
                {
                    const bool changed = ImGui::DragFloat3(label, values, p.drag_speed,
                                                           p.min_val, p.max_val, "%.3f");
                    if (changed)
                        clamp_property_float3(values, p);
                    return changed;
                }

                auto* lock_state = ctx.find_or_create_float3_lock(instance, p.offset);
                const float before[3] = { values[0], values[1], values[2] };

                const bool was_locked = lock_state->locked;
                if (was_locked)
                    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
                if (ImGui::SmallButton("1:1"))
                    lock_state->locked = !lock_state->locked;
                if (was_locked)
                    ImGui::PopStyleColor();
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("%s", lock_state->locked ? "Uniform scale locked" : "Uniform scale unlocked");
                ImGui::SameLine();

                const bool changed = ImGui::DragFloat3(label, values, p.drag_speed,
                                                       p.min_val, p.max_val, "%.3f");
                if (changed)
                {
                    if (lock_state->locked)
                        apply_uniform_scale_edit(values, before, p);
                    else
                        clamp_property_float3(values, p);
                }
                return changed;
            }

            bool draw_property(void* instance, const Property& p, PropertyDrawContext& ctx)
            {
                void* field_ptr = static_cast<char*>(instance) + p.offset;
                ImGui::PushID(p.name);
                const char* label = property_label(p);
                const bool readonly = property_readonly(p);
                bool changed = false;

                switch (p.type)
                {
                    case PropertyType::Bool:
                    {
                        bool* v = static_cast<bool*>(field_ptr);
                        if (readonly) ImGui::LabelText(label, "%s", *v ? "true" : "false");
                        else          changed = ImGui::Checkbox(label, v);
                        break;
                    }
                    case PropertyType::Int:
                    {
                        int* v = static_cast<int*>(field_ptr);
                        if (readonly) ImGui::LabelText(label, "%d", *v);
                        else          changed = ImGui::DragInt(label, v, p.drag_speed,
                                                                 (int)p.min_val, (int)p.max_val);
                        break;
                    }
                    case PropertyType::Float:
                    {
                        float* v = static_cast<float*>(field_ptr);
                        if (readonly) ImGui::LabelText(label, "%.3f", *v);
                        else          changed = ImGui::DragFloat(label, v, p.drag_speed,
                                                                   p.min_val, p.max_val);
                        break;
                    }
                    case PropertyType::Float3:
                    {
                        float3* v = static_cast<float3*>(field_ptr);
                        if (readonly)
                            ImGui::LabelText(label, "%.3f, %.3f, %.3f", v->x, v->y, v->z);
                        else
                            changed = draw_float3_drag(instance, p, ctx, &v->x);
                        break;
                    }
                    case PropertyType::Color3:
                    {
                        float3* v = static_cast<float3*>(field_ptr);
                        if (readonly)
                            ImGui::LabelText(label, "%.3f, %.3f, %.3f", v->x, v->y, v->z);
                        else
                            changed = ImGui::ColorEdit3(label, &v->x);
                        break;
                    }
                    case PropertyType::QuaternionEuler:
                    {
                        auto* quat  = static_cast<quaternion_f*>(field_ptr);
                        auto* cache = ctx.find_or_create_euler(instance, p.offset);
                        const bool external =
                            cache->first_use ||
                            quat->q.x != cache->last_quat.q.x ||
                            quat->q.y != cache->last_quat.q.y ||
                            quat->q.z != cache->last_quat.q.z ||
                            quat->q.w != cache->last_quat.q.w;
                        if (external)
                        {
                            float4x4 mat = quat->to_matrix();
                            float tr[3], eu[3], sc[3];
                            ImGuizmo::DecomposeMatrixToComponents(mat.Data(), tr, eu, sc);
                            cache->euler[0] = eu[0];
                            cache->euler[1] = eu[1];
                            cache->euler[2] = eu[2];
                            cache->last_quat = *quat;
                            cache->first_use = false;
                        }
                        if (readonly)
                        {
                            ImGui::LabelText(label, "%.3f, %.3f, %.3f",
                                             cache->euler[0], cache->euler[1], cache->euler[2]);
                        }
                        else if (ImGui::DragFloat3(label, cache->euler, p.drag_speed,
                                                   p.min_val, p.max_val, "%.3f"))
                        {
                            const float deg2rad = 3.14159265358979323846f / 180.0f;
                            auto qx = quaternion_f::rotation_from_axis_angle(float3(1,0,0), cache->euler[0] * deg2rad);
                            auto qy = quaternion_f::rotation_from_axis_angle(float3(0,1,0), cache->euler[1] * deg2rad);
                            auto qz = quaternion_f::rotation_from_axis_angle(float3(0,0,1), cache->euler[2] * deg2rad);
                            *quat = quaternion_f::mul(quaternion_f::mul(qz, qy), qx);
                            cache->last_quat = *quat;
                            changed = true;
                        }
                        break;
                    }
                    case PropertyType::String:
                    {
                        auto* v = static_cast<eastl::string*>(field_ptr);
                        if (readonly)
                        {
                            ImGui::LabelText(label, "%s", v->empty() ? "(empty)" : v->c_str());
                        }
                        else
                        {
                            const bool is_asset_reference = p.asset_kind != PropertyAssetKind::None;
                            char buf[1024];
                            std::snprintf(buf, sizeof(buf), "%s", v->c_str());
                            const ImGuiInputTextFlags input_flags =
                                is_asset_reference ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_None;
                            if (ImGui::InputText(label, buf, sizeof(buf), input_flags) && !is_asset_reference)
                            {
                                *v = buf;
                                changed = true;
                            }
                            if (is_asset_reference && ImGui::BeginDragDropTarget())
                            {
                                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CYBER_ASSET"))
                                {
                                    const char* path = static_cast<const char*>(payload->Data);
                                    if (asset_path_matches_property(p, path))
                                    {
                                        *v = to_project_relative_asset_path(path);
                                        changed = true;
                                    }
                                }
                                ImGui::EndDragDropTarget();
                            }
                            if (is_asset_reference && ImGui::IsItemHovered())
                                ImGui::SetTooltip("Drop a matching Content Browser asset here");
                            if (is_asset_reference && !v->empty())
                            {
                                ImGui::SameLine();
                                if (ImGui::SmallButton("Clear"))
                                {
                                    v->clear();
                                    changed = true;
                                }
                            }
                        }
                        break;
                    }
                }

                ImGui::PopID();

                if (changed && p.on_changed)
                    p.on_changed(instance);
                return changed;
            }

            bool draw_chain(void* instance, const char* type_name, PropertyDrawContext& ctx)
            {
                const auto* props = PropertyRegistry::get().find(type_name);
                if (!props) return false;
                bool changed = false;
                if (!props->parent_type_name.empty())
                {
                    if (draw_chain(instance, props->parent_type_name.c_str(), ctx))
                        changed = true;
                }
                for (const auto& p : props->fields)
                {
                    if (draw_property(instance, p, ctx))
                        changed = true;
                }
                return changed;
            }
        }

        bool draw_component_properties(void* instance, const char* type_name, PropertyDrawContext& ctx)
        {
            return draw_chain(instance, type_name, ctx);
        }
    }
}
