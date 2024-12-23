// SPDX-License-Identifier: GPL-3.0-only
//
#pragma once

#include <ParFile/ParFile.h>

#include <boost/json/object.hpp>

#include <complex>
#include <string>

namespace ParFile
{

struct NamedFileParSet;
class Config;

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
    struct Interpolant
    {
        std::string name;
        int index;
        double from;
        double to;
    };
    static std::vector<Interpolant> load_interpolants(const Config &config);
    int m_num_frames;
    std::string m_frame_name;
    std::string m_output;
    std::string m_script;
    ParSet m_from;
    ParSet m_to;
    std::vector<Interpolant> m_interpolants;
    int m_frame{};
};

} // namespace ParFile
