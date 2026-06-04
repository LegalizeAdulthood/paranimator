// SPDX-License-Identifier: GPL-3.0-only
//
#pragma once

#include <ParFile/Config.h>
#include <ParFile/ParFile.h>
#include <ParFile/ParameterCatalog.h>

#include <optional>
#include <string>
#include <vector>

namespace ParFile
{

struct ResolvedCamera2DValueTrack
{
    ParameterMetadata metadata;
    std::vector<KeyframeConfig> keys;
};

struct ResolvedCamera2DConfig
{
    double aspect{};
    double center_mag_x_mag_factor{1.0};
    ResolvedCamera2DValueTrack look_at;
    ResolvedCamera2DValueTrack view_up;
    ResolvedCamera2DValueTrack height;
};

struct ResolvedTrack
{
    std::string parameter;
    ParameterMetadata metadata;
    std::string base_value;
    std::vector<KeyframeConfig> keys;
    std::string output_parameter;
    std::vector<int> slots;
    TrackMode mode{TrackMode::KEYFRAMES};
    std::optional<PwmConfig> pwm;
    std::optional<PathConfig> path;
    std::optional<ResolvedCamera2DConfig> camera2d;
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
