#include "graphics_utils.h"
#include <EASTL/vector.h>
#include "common/flags.h"
#include "interface/graphics_types.h"
#include "interface/device_context.h"
#include "parallel_hashmap/phmap.h"
#include "platform/memory.h"
#include "interface/root_signature.hpp"
#include "interface/root_signature_pool.h"

namespace Cyber
{
    struct RSCharacteristic
    {
        // table count & hash
        uint32_t table_count;
        size_t table_hash;
        uint32_t push_constant_count;
        size_t push_constant_hash;
        // static samplers
        uint32_t static_sampler_count;
        size_t static_sampler_hash;

        PIPELINE_TYPE pipeline_type;
        operator size_t() const
        {
            return Cyber::cyber_hash(this, sizeof(RSCharacteristic), (size_t)pipeline_type);
        }
        struct hasher{ inline size_t operator()(const RSCharacteristic& val) const { return (size_t)val;}};
        struct RSTResource
        {
            GRAPHICS_RESOURCE_TYPE type;
            TEXTURE_DIMENSION dim;
            uint32_t set;
            uint32_t binding;
            uint32_t size;
            uint32_t offset;
            SHADER_STAGE stages;
        };

        struct StaticSampler
        {
            uint32_t set;
            uint32_t binding;
            class ISampler* id;
        };
        struct PushConstant
        {
            uint32_t set;
            uint32_t binding;
            uint32_t size;
            uint32_t offset;
            SHADER_STAGE stages;
        };
    };

    class RootSignaturePoolImpl : public RenderObject::IRootSignaturePool
    {
    public:
        RootSignaturePoolImpl(const char8_t* name)
            : name((const char*)name)
        {

        }

        CYBER_FORCE_INLINE RSCharacteristic calculateCharacteristic(RenderObject::IRootSignature* RSTables, const struct RenderObject::RootSignatureCreateDesc* desc)
        {
            // calculate characteristic
            RSCharacteristic newCharacteristic = {};
            newCharacteristic.table_count = RSTables->m_parameterTableCount;
            newCharacteristic.table_hash = (size_t)this;
            for(uint32_t i = 0; i < RSTables->m_parameterTableCount; ++i)
            {
                for(uint32_t j = 0; j < RSTables->m_parameterTables[j].resource_count; ++j)
                {
                    const auto& res = RSTables->m_parameterTables[i].resources[j];
                    RSCharacteristic::RSTResource r = {};
                    r.type = res.type;
                    r.dim = res.dimension;
                    r.set = res.set;
                    r.binding = res.binding;
                    r.size = res.size;
                    r.offset = res.offset;
                    r.stages = res.stages;
                    newCharacteristic.table_hash = Cyber::cyber_hash(&r, sizeof(r), newCharacteristic.table_hash);
                }
            }
            newCharacteristic.push_constant_count = RSTables->push_constant_count;
            newCharacteristic.push_constant_hash = (size_t)this;
            for(uint32_t i = 0; i < desc->push_constant_count; ++i)
            {
                const auto& pc = RSTables->push_constants[i];
                RSCharacteristic::PushConstant p = {};
                p.set = pc.set;
                p.binding = pc.binding;
                p.size = pc.size;
                p.offset = pc.offset;
                p.stages = pc.stages;
                newCharacteristic.push_constant_hash = Cyber::cyber_hash(&p, sizeof(p), newCharacteristic.push_constant_hash);
            }
            newCharacteristic.static_sampler_count = desc->static_sampler_count;
            newCharacteristic.static_sampler_hash = ~0;
            // static samplers are well stable-sorted during RSTable intialization
            for(uint32_t i = 0; i < desc->static_sampler_count; ++i)
            {
                for(uint32_t j = 0; j < desc->static_sampler_count; ++j)
                {
                    if(strcmp((char*)desc->static_sampler_names[j], (char*)RSTables->static_samplers[i].name) == 0)
                    {
                        RSCharacteristic::StaticSampler s = {};
                        s.set = RSTables->static_samplers[i].set;
                        s.binding = RSTables->static_samplers[i].binding;
                        s.id = desc->static_samplers[j];
                        newCharacteristic.static_sampler_hash = Cyber::cyber_hash(&s, sizeof(s), newCharacteristic.static_sampler_hash);
                    }
                }
            }
            newCharacteristic.pipeline_type = RSTables->pipeline_type;
            return newCharacteristic;
        }

        RenderObject::IRootSignature* try_allocate(RenderObject::IRootSignature* RSTables, const struct RenderObject::RootSignatureCreateDesc* desc)
        {
            const auto character = calculateCharacteristic(RSTables, desc);
            const auto iter = characterMap.find(character);
            if(iter != characterMap.end())
            {
                ++counterMap[iter->second];
                return iter->second;
            }
            return nullptr;
        }

        bool deallocate(RenderObject::IRootSignature* rootSig)
        {
            auto trueSig = rootSig;
            while(trueSig->pool_next && rootSig->pool)
            {
                trueSig = trueSig->pool_next;
            }
            auto&& iter = counterMap.find(trueSig);
            if(iter != counterMap.end())
            {
                const auto oldCounterVal = iter->second;
                if(oldCounterVal <= 1)
                {
                    counterMap.erase(trueSig);
                    const auto& character = biCharacterMap[trueSig];
                    characterMap.erase(character);
                    biCharacterMap.erase(trueSig);
                    trueSig->pool = nullptr;
                    trueSig->pool_next = nullptr;
                    render_device->free_root_signature(trueSig);
                    return true;
                }
                iter->second--;
                return true;
            }
            return false;
        }

        bool insert(RenderObject::IRootSignature* RSTables, const struct RenderObject::RootSignatureCreateDesc* desc)
        {
            const auto character = calculateCharacteristic(RSTables, desc);
            const auto iter = characterMap.find(character);
            if(iter != characterMap.end())
            {
                return false;
            }
            characterMap[character] = RSTables;
            biCharacterMap[RSTables] = character;
            counterMap[RSTables] = 1;
            RSTables->pool = nullptr;
            RSTables->pool_next = nullptr;
            return true;
        }
        ~RootSignaturePoolImpl()
        {
            for(auto& iter : counterMap)
            {
                auto enforceDestroy = iter.first;
                enforceDestroy->pool = nullptr;
                enforceDestroy->pool_next = nullptr;
                render_device->free_root_signature(enforceDestroy);
            }
        }
    private:
        const eastl::string name;
        phmap::flat_hash_map<RSCharacteristic, RenderObject::IRootSignature*, RSCharacteristic::hasher> characterMap;
        phmap::flat_hash_map<RenderObject::IRootSignature*, RSCharacteristic> biCharacterMap;
        phmap::flat_hash_map<RenderObject::IRootSignature*, uint32_t> counterMap;
    };

    RenderObject::IRootSignaturePool* graphics_util_create_root_signature_pool(const RenderObject::RootSignaturePoolCreateDesc& desc)
    {
        return cyber_new<RootSignaturePoolImpl>(desc.name);
    }

    RenderObject::IRootSignature* graphics_util_try_allocate_signature(RenderObject::IRootSignaturePool* pool, RenderObject::IRootSignature* RSTables, const struct RenderObject::RootSignatureCreateDesc& desc)
    {
        return static_cast<RootSignaturePoolImpl*>(pool)->try_allocate(RSTables, &desc);
    }
    bool graphics_util_add_signature(RenderObject::IRootSignaturePool* pool, RenderObject::IRootSignature* sig, const RenderObject::RootSignatureCreateDesc& desc)
    {
        return static_cast<RootSignaturePoolImpl*>(pool)->insert(sig, &desc);
    }
    bool graphics_util_pool_free_signature(RenderObject::IRootSignaturePool* pool, RenderObject::IRootSignature* signature)
    {
        return static_cast<RootSignaturePoolImpl*>(pool)->deallocate(signature);
    }
    void graphics_util_free_root_signature_pool(Ref<RenderObject::IRootSignaturePool> pool)
    {
        auto P = static_cast<RootSignaturePoolImpl*>(pool.get());
        cyber_delete(P);
        pool.reset();
    }
}
