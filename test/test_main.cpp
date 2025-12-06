#include <iostream>
#include <cp_framework/framework.hpp>
#include <cp_framework/systems/eventSystem.hpp>
#include <cp_framework/core/debug.hpp>

struct onEvent : cp::Event
{
};

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