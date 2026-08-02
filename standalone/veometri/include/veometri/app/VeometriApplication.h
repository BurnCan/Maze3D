#pragma once

#include <memory>

namespace veometri::app
{

class VeometriApplication
{
public:
    VeometriApplication();
    ~VeometriApplication();

    VeometriApplication(const VeometriApplication&) = delete;
    VeometriApplication& operator=(const VeometriApplication&) = delete;
    VeometriApplication(VeometriApplication&&) = delete;
    VeometriApplication& operator=(VeometriApplication&&) = delete;

    int run();

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace veometri::app
