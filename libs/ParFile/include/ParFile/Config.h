// SPDX-License-Identifier: GPL-3.0-only
//
#pragma once

#include <boost/json.hpp>

#include <string>
#include <vector>

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
    Config(const Config &rhs) = default;
    Config(Config &&rhs) = default;
    Config(const boost::json::object &json);
    Config &operator=(const Config &rhs) = default;
    Config &operator=(Config &&rhs) = default;

    const NamedFileParSet &from() const
    {
        return m_from;
    }
    const NamedFileParSet &to() const
    {
        return m_to;
    }
    const std::vector<std::string> &interpolate() const
    {
        return m_interpolate;
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
    std::vector<std::string> m_interpolate;
    std::string m_output;
    std::string m_script;
    std::string m_frame;
    int m_num_frames;
};

} // namespace ParFile
