#pragma once
#include "platform/configure.h"
#include "core/interface/data_blob.h"
#include "eastl/vector.h"
#include "cyber_runtime.config.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(Core)

class CYBER_RUNTIME_API DataBlobImpl final : public IDataBlob
{
public:
    static DataBlobImpl* create(size_t size = 0, const void* data = nullptr);

    virtual ~DataBlobImpl();

    virtual void resize(size_t newSize) override;
    virtual size_t get_size() const override;
    virtual void* get_data_ptr() override;
    virtual const void* get_const_data_ptr() const override;

    template<typename T>
    T* get_data_ptr()
    {
        return reinterpret_cast<T*>(get_data_ptr());
    }

    explicit DataBlobImpl(size_t size, const void* data);

private:
    size_t m_Size;
    eastl::vector<uint8_t> m_Data;
};

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE