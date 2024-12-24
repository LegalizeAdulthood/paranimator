// SPDX-License-Identifier: GPL-3.0-only
//
#pragma once

#include <ParFile/ParFile.h>

#include <boost/json/object.hpp>

#include <memory>
#include <string>
#include <vector>

namespace ParFile
{

struct NamedFileParSet;
class Config;
class Interpolant;
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

    const ParSet &from() const
    {
        return m_from;
    }
    const ParSet &to() const
    {
        return m_to;
    }

    ParSet operator()();

private:
    static std::vector<InterpolantPtr> load_interpolants(
        const Config &config, const ParSet &from, const ParSet &to, int num_steps);
    int m_num_frames;
    std::string m_frame_name;
    std::string m_output;
    std::string m_script;
    std::string m_video;
    ParSet m_from;
    ParSet m_to;
    std::vector<InterpolantPtr> m_interpolants;
    int m_frame{};
};

} // namespace ParFile
