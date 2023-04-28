#include "common_utils.h"
#include <EASTL/vector.h>
#include "rhi/flags.h"
#include "rhi/rhi.h"
#include "parallel_hashmap/phmap.h"

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
        struct hasher{ inline size_t operator()(const RSCharacteristic& val) const { (size_t)val;}};
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
            struct Cyber::RHISampler* sampler;
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
            : name(name)
        {

        }
        FORCEINLINE RSCharacteristic calculateCharacteristic(RHIRootSignature* RSTables, const struct RHIRootSignatureCreateDesc* desc)
        {

        }

        RHIRootSignature* try_allocate(RHIRootSignature* RSTables, const struct RHIRootSignatureCreateDesc* desc)
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

        bool deallocate(RHIRootSignature* RSTables)
        {

        }
        bool insert(RHIRootSignature* RSTables, const struct RHIRootSignatureCreateDesc* desc)
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
            RSTables->pool = this;
            RSTables->pool_next = nullptr;
        }
        ~RHIRootSignaturePoolImpl()
        {

        }
    private:
        const eastl::string name;
        phmap::flat_hash_map<RSCharacteristic, RHIRootSignature*, RSCharacteristic::hasher> characterMap;
        phmap::flat_hash_map<RHIRootSignature*, RSCharacteristic> biCharacterMap;
        phmap::flat_hash_map<RHIRootSignature*, uint32_t> counterMap;
    };

}
