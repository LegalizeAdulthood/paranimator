// SPDX-License-Identifier: GPL-3.0-only
//
#pragma once

#include <cstddef>
#include <string>
#include <string_view>
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

struct KeyframeConfig
{
    int frame{};
    std::string value;
};

struct TrackConfig
{
    std::string parameter;
    std::vector<KeyframeConfig> keys;
};

class Config
{
public:
    Config() = default;
    Config(const Config &rhs) = default;
    Config(Config &&rhs) = default;
    Config(std::string_view json_text);
    Config &operator=(const Config &rhs) = default;
    Config &operator=(Config &&rhs) = default;

    const NamedFileParSet &source() const
    {
        return m_source;
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
    std::size_t num_tracks() const
    {
        return m_tracks.size();
    }
    const std::vector<TrackConfig> &tracks() const
    {
        return m_tracks;
    }

private:
    NamedFileParSet m_source;
    OutputConfig m_output;
    int m_parallel{1};
    std::string m_video;
    int m_num_frames{};
    std::vector<TrackConfig> m_tracks;
};

} // namespace ParFile
