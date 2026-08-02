#pragma once

#include <memory>

namespace mesh_sculpt::app
{

class SculptApplication
{
public:
    SculptApplication();
    ~SculptApplication();

    SculptApplication(const SculptApplication&) = delete;
    SculptApplication& operator=(const SculptApplication&) = delete;
    SculptApplication(SculptApplication&&) = delete;
    SculptApplication& operator=(SculptApplication&&) = delete;

    int run();

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace mesh_sculpt::app
