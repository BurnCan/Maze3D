#include "veometri/app/VeometriApplication.h"
#include "veometri/platform/AssetLocator.h"

#include <cstdlib>
#include <exception>
#include <iostream>

int main(int argc, char** argv)
{
    try
    {
        if (argc > 0) veometri::platform::AssetLocator::setExecutablePath(argv[0]);
        veometri::app::VeometriApplication application;
        return application.run();
    }
    catch (const std::exception& error)
    {
        std::cerr << "Veometri failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "Veometri failed with an unknown error.\n";
        return EXIT_FAILURE;
    }
}
