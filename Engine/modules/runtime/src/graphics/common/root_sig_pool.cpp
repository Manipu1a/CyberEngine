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
            class RenderObject::ISampler* id;
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

    class RootSignaturePoolImpl : public RenderObject::RootSignaturePoolBase
    {
    public:
        RootSignaturePoolImpl(const RenderObject::RootSignaturePoolCreateDesc& desc)
            : RootSignaturePoolBase(desc)
        {

        }

        CYBER_FORCE_INLINE RSCharacteristic calculateCharacteristic(RenderObject::IRootSignature* RSTables, const struct RenderObject::RootSignatureCreateDesc* desc)
        {
            // calculate characteristic
            RSCharacteristic newCharacteristic = {};
            newCharacteristic.table_count = RSTables->get_parameter_table_count();
            newCharacteristic.table_hash = (size_t)this;
            for(uint32_t i = 0; i < RSTables->get_parameter_table_count(); ++i)
            {
                for(uint32_t j = 0; j < RSTables->get_parameter_table(j)->m_resourceCount; ++j)
                {
                    const auto& res = RSTables->get_parameter_table(i)->m_ppResources[j];
                    RSCharacteristic::RSTResource r = {};
                    r.type = res->get_type();
                    r.dim = res->get_dimension();
                    r.set = res->get_set();
                    r.binding = res->get_binding();
                    r.size = res->get_size();
                    r.offset = res->get_offset();
                    r.stages = res->get_stages();
                    newCharacteristic.table_hash = Cyber::cyber_hash(&r, sizeof(r), newCharacteristic.table_hash);
                }
            }
            newCharacteristic.push_constant_count = RSTables->get_push_constant_count();
            newCharacteristic.push_constant_hash = (size_t)this;
            for(uint32_t i = 0; i < desc->m_pushConstantCount; ++i)
            {
                const auto& pc = RSTables->get_push_constant(i);
                RSCharacteristic::PushConstant p = {};
                p.set = pc->get_set();
                p.binding = pc->get_binding();
                p.size = pc->get_size();
                p.offset = pc->get_offset();
                p.stages = pc->get_stages();
                newCharacteristic.push_constant_hash = Cyber::cyber_hash(&p, sizeof(p), newCharacteristic.push_constant_hash);
            }
            newCharacteristic.static_sampler_count = desc->m_staticSamplerCount;
            newCharacteristic.static_sampler_hash = ~0;
            // static samplers are well stable-sorted during RSTable intialization
            for(uint32_t i = 0; i < desc->m_staticSamplerCount; ++i)
            {
                for(uint32_t j = 0; j < desc->m_staticSamplerCount; ++j)
                {
                    if(strcmp((char*)desc->m_staticSamplerNames[j], (char*)RSTables->get_static_sampler(i)->get_name()) == 0)
                    {
                        RSCharacteristic::StaticSampler s = {};
                        s.set = RSTables->get_static_sampler(i)->get_set();
                        s.binding = RSTables->get_static_sampler(i)->get_binding();
                        s.id = desc->m_staticSamplers[j];
                        newCharacteristic.static_sampler_hash = Cyber::cyber_hash(&s, sizeof(s), newCharacteristic.static_sampler_hash);
                    }
                }
            }
            newCharacteristic.pipeline_type = RSTables->get_pipeline_type();
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
            while(trueSig->get_pool_next() && rootSig->get_pool())
            {
                trueSig = trueSig->get_pool_next();
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
                    trueSig->set_pool(nullptr);
                    trueSig->set_pool_next(nullptr);
                    m_renderDevice->free_root_signature(trueSig);
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
            RSTables->set_pool(nullptr);
            RSTables->set_pool_next(nullptr);
            return true;
        }
        ~RootSignaturePoolImpl()
        {
            for(auto& iter : counterMap)
            {
                auto enforceDestroy = iter.first;
                enforceDestroy->set_pool(nullptr);
                enforceDestroy->set_pool_next(nullptr);
                m_renderDevice->free_root_signature(enforceDestroy);
            }
        }
    private:
        const eastl::string name;
        phmap::flat_hash_map<RSCharacteristic, RenderObject::IRootSignature*, RSCharacteristic::hasher> characterMap;
        phmap::flat_hash_map<RenderObject::IRootSignature*, RSCharacteristic> biCharacterMap;
        phmap::flat_hash_map<RenderObject::IRootSignature*, uint32_t> counterMap;
    };

    RenderObject::RootSignaturePoolBase* graphics_util_create_root_signature_pool(const RenderObject::RootSignaturePoolCreateDesc& desc)
    {
        return cyber_new<RootSignaturePoolImpl>(desc);
    }

    RenderObject::IRootSignature* graphics_util_try_allocate_signature(RenderObject::RootSignaturePoolBase* pool, RenderObject::IRootSignature* RSTables, const struct RenderObject::RootSignatureCreateDesc& desc)
    {
        return static_cast<RootSignaturePoolImpl*>(pool)->try_allocate(RSTables, &desc);
    }
    bool graphics_util_add_signature(RenderObject::RootSignaturePoolBase* pool, RenderObject::IRootSignature* sig, const RenderObject::RootSignatureCreateDesc& desc)
    {
        return static_cast<RootSignaturePoolImpl*>(pool)->insert(sig, &desc);
    }
    bool graphics_util_pool_free_signature(RenderObject::RootSignaturePoolBase* pool, RenderObject::IRootSignature* signature)
    {
        return static_cast<RootSignaturePoolImpl*>(pool)->deallocate(signature);
    }
    void graphics_util_free_root_signature_pool(Ref<RenderObject::RootSignaturePoolBase> pool)
    {
        auto P = static_cast<RootSignaturePoolImpl*>(pool.get());
        cyber_delete(P);
        pool.reset();
    }
}
