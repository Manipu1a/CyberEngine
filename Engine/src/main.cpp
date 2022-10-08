#include <iostream>

#include <EASTL/vector.h>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

int main(int argc, char** argv)
{
    eastl::vector<int, eastl::allocator> test{1,2,3};

    for(auto iter : test)
    {
        std::cout << iter << std::endl;
    }
    
    spdlog::info("Cyber Engine");
    
    return 0;
}
