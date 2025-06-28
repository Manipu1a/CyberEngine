#pragma once
#include "basic_file.h"
#include "platform/memory.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(Core)

class CYBER_CORE_API FileHelper
{
public:
    FileHelper(): file(nullptr) {}

    explicit FileHelper(const char* filePath, FILE_ACCESS_MODE accessMode = FILE_ACCESS_READ)
    {
        const char8_t* project_path = u8"../../../../";
        eastl::string full_path(eastl::string::CtorSprintf(), "%s%s", project_path, filePath);

        FileOpenAttributes attributes(full_path.c_str(), accessMode);
        file = cyber_new<BasicFile>(attributes);
    }

    operator BasicFile*() {return file;}
    BasicFile* operator->() {return file;}

private:
    BasicFile* file;
};

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE