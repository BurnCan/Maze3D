#include "mesh_sculpt/app/SculptApplication.h"

#include <cstdlib>
#include <exception>
#include <iostream>

int main()
{
    try
    {
        mesh_sculpt::app::SculptApplication application;
        return application.run();
    }
    catch (const std::exception& error)
    {
        std::cerr << "Mesh Sculpt failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "Mesh Sculpt failed with an unknown error.\n";
        return EXIT_FAILURE;
    }
}
