// SPDX-License-Identifier: GPL-3.0-only
//
#pragma once

#include <ParFile/ParFile.h>

#include <boost/json/object.hpp>

#include <string>

namespace ParFile
{

class Config;

class Interpolator
{
public:
    Interpolator(const Config &config);

    const ParSet &from() const
    {
        return m_from;
    }

    ParSet operator()();

private:
    int m_num_frames;
    std::string m_frame_name;
    std::string m_output;
    std::string m_script;
    ParSet m_from;
    ParSet m_to;
    int m_frame{};
};

} // namespace ParFile
