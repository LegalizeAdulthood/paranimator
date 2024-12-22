// SPDX-License-Identifier: GPL-3.0-only
//
#pragma once

#include <boost/json.hpp>

#include <string>

namespace ParFile
{

struct NamedFileParSet
{
    std::string file;
    std::string name;
};

class Config
{
public:
    Config() = default;
    Config(const boost::json::object &json);

    const NamedFileParSet &from() const
    {
        return m_from;
    }
    const NamedFileParSet &to() const
    {
        return m_to;
    }
    const std::string &output() const
    {
        return m_output;
    }
    const std::string &script() const
    {
        return m_script;
    }
    const std::string &frame() const
    {
        return m_frame;
    }
    int num_frames() const
    {
        return m_num_frames;
    }

private:
    NamedFileParSet m_from;
    NamedFileParSet m_to;
    std::string m_output;
    std::string m_script;
    std::string m_frame;
    int m_num_frames;
};

} // namespace ParFile
