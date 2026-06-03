// SPDX-License-Identifier: GPL-3.0-only
//
#pragma once

#include <ParFile/Config.h>
#include <ParFile/ParFile.h>
#include <ParFile/ParameterCatalog.h>

#include <string>
#include <vector>

namespace ParFile
{

struct ResolvedTrack
{
    std::string parameter;
    ParameterMetadata metadata;
    std::string base_value;
    std::vector<KeyframeConfig> keys;
};

struct ResolvedAnimation
{
    std::string frame_name;
    std::string video;
    int num_frames{};
    ParSet source;
    std::vector<ResolvedTrack> tracks;
};

ResolvedAnimation resolve_animation(const Config &config, const ParameterCatalog &catalog, const ParSet &source);

} // namespace ParFile
