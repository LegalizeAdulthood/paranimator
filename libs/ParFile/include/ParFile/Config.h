// SPDX-License-Identifier: GPL-3.0-only
//
#pragma once

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
    std::string curve;
};

struct TrackConfig
{
    std::string parameter;
    std::vector<KeyframeConfig> keys;
};

struct Config
{
    std::vector<std::string> parameter_catalogs;
    NamedFileParSet source;
    OutputConfig output;
    int parallel{1};
    std::string video;
    int num_frames{};
    std::vector<TrackConfig> tracks;
};

Config read_config(std::string_view json_text);

} // namespace ParFile
