// SPDX-License-Identifier: GPL-3.0-only
//
#pragma once

#include <ParFile/ParFile.h>

#include <memory>
#include <string>
#include <vector>

namespace ParFile
{

struct Config;
class Interpolant;
struct NamedFileParSet;
using InterpolantPtr = std::shared_ptr<Interpolant>;

class Interpolator
{
public:
    Interpolator() = default;
    Interpolator(const Config &config);
    Interpolator(const Interpolator &rhs) = default;
    Interpolator(Interpolator &&rhs) = default;
    Interpolator &operator=(const Interpolator &rhs) = default;
    Interpolator &operator=(Interpolator &&rhs) = default;

    const ParSet &source() const
    {
        return m_source;
    }

    ParSet operator()();

private:
    static std::vector<InterpolantPtr> load_interpolants(const Config &config, const ParSet &source);
    std::string m_frame_name;
    std::string m_video;
    ParSet m_source;
    std::vector<InterpolantPtr> m_interpolants;
    int m_frame{};
};

} // namespace ParFile
