// SPDX-License-Identifier: GPL-3.0-only
//
#pragma once

#include <ParFile/AnimationEnums.h>

#include <optional>
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
    std::optional<Curve> curve;
    std::optional<double> mix;
};

struct PwmConfig
{
    std::string a;
    std::string b;
    int window{};
};

enum class ColorMapEffectKind
{
    REVERSE,
    PING_PONG
};

struct ColorMapRangeConfig
{
    int first{};
    int last{};
};

struct ColorMapGradientStopConfig
{
    int index{};
    std::string color;
};

struct ColorMapGradientConfig
{
    std::vector<ColorMapGradientStopConfig> stops;
};

struct NumberKeyframeConfig
{
    int frame{};
    double value{};
    std::optional<Curve> curve;
};

struct NumberTrackConfig
{
    std::vector<NumberKeyframeConfig> keys;
};

struct ColorMapEffectConfig
{
    ColorMapEffectKind kind{ColorMapEffectKind::REVERSE};
    std::optional<ColorMapRangeConfig> range;
    std::optional<NumberTrackConfig> offset;
};

struct ColorMapConfig
{
    TrackFormat format{TrackFormat::AT_FILE};
    std::string output;
    std::optional<std::string> source;
    std::optional<ColorMapGradientConfig> gradient;
    std::vector<ColorMapEffectConfig> effects;
};

struct TrackConfig
{
    std::string parameter;
    std::vector<KeyframeConfig> keys;
    TrackMode mode{TrackMode::KEYFRAMES};
    std::optional<PwmConfig> pwm;
    TrackKind kind{TrackKind::PARAMETER};
    std::optional<ColorMapConfig> color_map;
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
