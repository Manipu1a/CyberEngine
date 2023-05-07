#include "common_utils.h"
#include <EASTL/vector.h>
#include "rhi/flags.h"
#include "rhi/rhi.h"
#include "parallel_hashmap/phmap.h"
#include "platform/memory.h"

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

        ERHIPipelineType pipeline_type;
        operator size_t() const
        {
            return Cyber::cyber_hash(this, sizeof(RSCharacteristic), (size_t)pipeline_type);
        }
        struct hasher{ inline size_t operator()(const RSCharacteristic& val) const { return (size_t)val;}};
        struct RSTResource
        {
            ERHIResourceType type;
            ERHITextureDimension dim;
            uint32_t set;
            uint32_t binding;
            uint32_t size;
            uint32_t offset;
            ERHIShaderStages stages;
        };

        struct StaticSampler
        {
            uint32_t set;
            uint32_t binding;
            Cyber::RHISampler* id;
        };
        struct PushConstant
        {
            uint32_t set;
            uint32_t binding;
            uint32_t size;
            uint32_t offset;
            ERHIShaderStages stages;
        };
    };

    class RHIRootSignaturePoolImpl : public Cyber::RHIRootSignaturePool
    {
    public:
        RHIRootSignaturePoolImpl(const char8_t* name)
            : name((const char*)name)
        {

        }

        FORCEINLINE RSCharacteristic calculateCharacteristic(Ref<RHIRootSignature> RSTables, const struct RHIRootSignatureCreateDesc* desc)
        {
            // calculate characteristic
            RSCharacteristic newCharacteristic = {};
            newCharacteristic.table_count = RSTables->parameter_table_count;
            newCharacteristic.table_hash = (size_t)this;
            for(uint32_t i = 0; i < RSTables->parameter_table_count; ++i)
            {
                for(uint32_t j = 0; j < RSTables->parameter_tables[j].resource_count; ++j)
                {
                    const auto& res = RSTables->parameter_tables[i].resources[j];
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

        Ref<RHIRootSignature> try_allocate(Ref<RHIRootSignature> RSTables, const struct RHIRootSignatureCreateDesc* desc)
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

        bool deallocate(Ref<RHIRootSignature> rootSig)
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
                    rhi_free_root_signature(trueSig);
                    return true;
                }
                iter->second--;
                return true;
            }
            return false;
        }

        bool insert(Ref<RHIRootSignature> RSTables, const struct RHIRootSignatureCreateDesc* desc)
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
        ~RHIRootSignaturePoolImpl()
        {
            for(auto& iter : counterMap)
            {
                auto enforceDestroy = iter.first;
                enforceDestroy->pool = nullptr;
                enforceDestroy->pool_next = nullptr;
                rhi_free_root_signature(enforceDestroy);
            }
        }
    private:
        const eastl::string name;
        phmap::flat_hash_map<RSCharacteristic, Ref<RHIRootSignature>, RSCharacteristic::hasher> characterMap;
        phmap::flat_hash_map<Ref<RHIRootSignature>, RSCharacteristic> biCharacterMap;
        phmap::flat_hash_map<Ref<RHIRootSignature>, uint32_t> counterMap;
    };

    Ref<RHIRootSignaturePool> rhi_util_create_root_signature_pool(const RHIRootSignaturePoolCreateDesc& desc)
    {
        return Ref<RHIRootSignaturePool>(new RHIRootSignaturePoolImpl(desc.name));
    }

    Ref<RHIRootSignature> rhi_util_try_allocate_signature(Ref<RHIRootSignaturePool> pool, Ref<RHIRootSignature> RSTables, const struct RHIRootSignatureCreateDesc& desc)
    {
        return static_cast<RHIRootSignaturePoolImpl*>(pool.get())->try_allocate(RSTables, &desc);
    }
    bool rhi_util_add_signature(Ref<RHIRootSignaturePool> pool, Ref<RHIRootSignature> sig, const RHIRootSignatureCreateDesc& desc)
    {
        return static_cast<RHIRootSignaturePoolImpl*>(pool.get())->insert(sig, &desc);
    }
    bool rhi_util_pool_free_signature(Ref<RHIRootSignaturePool> pool, Ref<RHIRootSignature> signature)
    {
        return static_cast<RHIRootSignaturePoolImpl*>(pool.get())->deallocate(signature);
    }
    void rhi_util_free_root_signature_pool(Ref<RHIRootSignaturePool> pool)
    {
        auto P = static_cast<RHIRootSignaturePoolImpl*>(pool.get());
        cyber_delete(P);
        pool.reset();
    }
}
