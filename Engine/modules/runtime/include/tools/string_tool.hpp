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