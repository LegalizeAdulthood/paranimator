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

struct OutputConfig
{
    std::string directory;
    std::string par;
    std::string entry;
    std::string script;
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
    const OutputConfig &output() const
    {
        return m_output;
    }
    int parallel() const
    {
        return m_parallel;
    }
    const std::string &video() const
    {
        return m_video;
    }
    int num_frames() const
    {
        return m_num_frames;
    }

private:
    NamedFileParSet m_from;
    NamedFileParSet m_to;
    std::vector<std::string> m_interpolate;
    OutputConfig m_output;
    int m_parallel{1};
    std::string m_video;
    int m_num_frames;
};

} // namespace ParFile
