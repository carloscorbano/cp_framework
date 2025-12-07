#include <iostream>
#include <cp_framework/framework.hpp>

int main()
{
    try
    {
        cp::Framework framework;
        framework.Init();
        framework.Run();
    }
    catch (const std::exception &e)
    {
        std::cout << "Program terminated with error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}