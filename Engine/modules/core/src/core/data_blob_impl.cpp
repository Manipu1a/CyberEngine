#include "core/data_blob_impl.hpp"
#include "platform/memory.h"
CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(Core)

DataBlobImpl* DataBlobImpl::create(size_t size, const void* data)
{
    return cyber_new<DataBlobImpl>(size, data);
}

DataBlobImpl::DataBlobImpl(size_t size, const void* data)
    : m_Size(size),
        m_Data(size)
{
    if(data != nullptr)
        memcpy(m_Data.data(), data, size);
}

DataBlobImpl::~DataBlobImpl()
{
}

void DataBlobImpl::resize( size_t newSize )
{
    m_Size = newSize;
    m_Data.resize(newSize);
}

size_t DataBlobImpl::get_size() const
{
    return m_Size;
}

void* DataBlobImpl::get_data_ptr()
{
    return m_Data.data();
}

const void* DataBlobImpl::get_const_data_ptr() const
{
    return m_Data.data();
}

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE