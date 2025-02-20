#pragma once

#include "eastl/string.h"


inline eastl::string to_string(const char* str)
{
    return eastl::string(str);
}


inline eastl::string string_to_lower(const eastl::string& str)
{
    eastl::string lowerStr = str;
    lowerStr.make_lower();
    return lowerStr;
}

inline eastl::string string_to_upper(const eastl::string& str)
{
    eastl::string upperStr = str;
    upperStr.make_upper();
    return upperStr;
}

inline eastl::wstring to_wstring(const char* str)
{
    return eastl::wstring(str);
}

inline eastl::wstring u8_to_wstring(const char8_t* str)
{
    return eastl::wstring((const char*)str);
}

inline bool safe_u8string_equal(const char8_t* str1, const char8_t* str2)
{
    if (str1 == nullptr || str2 == nullptr)
        return str1 == str2;
    
    return strcmp( (const char*)str1,  (const char*)str2) == 0;
}

inline bool safe_string_equal(const char* str1, const char* str2)
{
    if (str1 == nullptr || str2 == nullptr)
        return str1 == str2;
    
    return strcmp(str1, str2) == 0;
}