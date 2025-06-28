#pragma once
#include "platform/configure.h"
#include "EASTL/string.h"
#include "core/data_blob_impl.hpp"
#include "cyber_core.config.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(Core)

enum FILE_ACCESS_MODE
{
    FILE_ACCESS_READ,
    FILE_ACCESS_WRITE,
    FILE_ACCESS_READ_WRITE,
};

struct CYBER_CORE_API FileOpenAttributes
{
    const char* file_path;
    FILE_ACCESS_MODE access_mode;
    FileOpenAttributes(const char* path, FILE_ACCESS_MODE mode)
        : file_path(path), access_mode(mode) {}
};

class CYBER_CORE_API BasicFile
{
public:
    BasicFile(const FileOpenAttributes& attributes);
    virtual ~BasicFile() = default;

    const eastl::string& get_path() const { return path; }
    void read(IDataBlob** data_blob);
protected:
    const eastl::string path;
    const FileOpenAttributes open_attributes;
    FILE* file_handle = nullptr;
};

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE