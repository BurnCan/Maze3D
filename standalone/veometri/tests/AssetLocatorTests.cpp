#include "veometri/platform/AssetLocator.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>

int main()
{
    const auto roots = veometri::platform::AssetLocator::candidateRoots("/tmp/prefix/bin/veometri");
    if (roots.size() != 3 || roots[0] != "/tmp/prefix/bin/assets" ||
        roots[1] != "/tmp/prefix/share/veometri")
    {
        std::cerr << "Asset candidate order or installed layout is incorrect\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
