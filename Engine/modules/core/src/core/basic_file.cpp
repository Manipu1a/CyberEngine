#include "core/basic_file.h"
#include "platform/memory.h"
CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(Core)

BasicFile::BasicFile(const FileOpenAttributes& attributes)
    : path(attributes.file_path), open_attributes(attributes)
{
    if (open_attributes.access_mode == FILE_ACCESS_READ)
    {
        file_handle = fopen(path.c_str(), "rb");
    }
    else if (open_attributes.access_mode == FILE_ACCESS_WRITE)
    {
        file_handle = fopen(path.c_str(), "wb");
    }
    else if (open_attributes.access_mode == FILE_ACCESS_READ_WRITE)
    {
        file_handle = fopen(path.c_str(), "r+b");
    }
}

void BasicFile::read(IDataBlob** data_blob)
{
    uint8_t* bytes = nullptr;
    if(file_handle)
    {
        fseek(file_handle, 0, SEEK_END);
        size_t file_size = ftell(file_handle);
        rewind(file_handle);

        bytes = (uint8_t*)cyber_malloc(file_size);
        fread(bytes, file_size, 1, file_handle);
        fclose(file_handle);
        *data_blob = Core::DataBlobImpl::create(file_size, bytes);
    }
}

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE