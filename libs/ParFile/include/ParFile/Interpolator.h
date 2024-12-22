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

    int num_frames() const
    {
        return m_num_frames;
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
