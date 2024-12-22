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
        return m_from.par_set;
    }
    const ParSet &to() const
    {
        return m_to.par_set;
    }

    ParSet operator()();

private:
    struct CenterMag
    {
        std::complex<double> center;
        double mag;
    };
    struct Interpolant
    {
        ParSet par_set;
        CenterMag center_mag;
    };
    static Interpolant load_interpolant(const Config &config, const NamedFileParSet &par_entry);
    int m_num_frames;
    std::string m_frame_name;
    std::string m_output;
    std::string m_script;
    Interpolant m_from;
    Interpolant m_to;
    int m_frame{};
};

} // namespace ParFile
