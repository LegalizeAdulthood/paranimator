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

enum class PathKind
{
    CONSTANT,
    LINE,
    CIRCLE,
    ELLIPSE,
    LISSAJOUS,
    SPIRAL,
    BEZIER,
    CATMULL_ROM
};

struct PathConfig
{
    PathKind kind{PathKind::CONSTANT};
    std::string value;
    std::string from;
    std::string to;
    std::string center;
    double radius{};
    double x_radius{};
    double y_radius{};
    double from_radius{};
    double to_radius{};
    double x_frequency{};
    double y_frequency{};
    double turns{1.0};
    double phase{};
    std::vector<std::string> control_points;
};

struct PwmConfig
{
    std::string a;
    std::string b;
    int window{};
};

enum class ColorMapEffectKind
{
    BRIGHTNESS,
    CONTRAST,
    GAMMA,
    HUE_SHIFT,
    MASK_BLEND,
    PULSE,
    REMAP,
    SATURATION,
    SPARKLE,
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
    std::vector<ColorMapRangeConfig> ranges;
    std::optional<NumberTrackConfig> amount;
    std::optional<NumberTrackConfig> offset;
    std::optional<std::string> color;
    std::optional<std::string> source;
    std::vector<int> indices;
    std::optional<int> seed;
};

struct ColorMapConfig
{
    TrackFormat format{TrackFormat::AT_FILE};
    std::string output;
    std::optional<std::string> source;
    std::optional<ColorMapGradientConfig> gradient;
    std::vector<ColorMapEffectConfig> effects;
};

struct Camera2DValueTrackConfig
{
    ParameterType type{};
    bool normalize{};
    std::vector<KeyframeConfig> keys;
};

struct Camera2DConfig
{
    std::string name;
    std::string output;
    std::string aspect;
    Camera2DValueTrackConfig look_at;
    Camera2DValueTrackConfig view_up;
    Camera2DValueTrackConfig height;
};

struct Id3DViewValueTrackConfig
{
    ParameterType type{};
    std::optional<int> arity;
    std::vector<KeyframeConfig> keys;
};

struct Id3DViewOutputsConfig
{
    std::optional<std::string> rotation;
    std::optional<std::string> perspective;
    std::optional<std::string> xyshift;
};

struct Id3DViewConfig
{
    std::string name;
    Id3DViewOutputsConfig outputs;
    std::optional<Id3DViewValueTrackConfig> rotation;
    std::optional<Id3DViewValueTrackConfig> perspective;
    std::optional<Id3DViewValueTrackConfig> xyshift;
};

struct TrackConfig
{
    std::string parameter;
    std::vector<KeyframeConfig> keys;
    TrackMode mode{TrackMode::KEYFRAMES};
    std::optional<PwmConfig> pwm;
    TrackKind kind{TrackKind::PARAMETER};
    std::optional<ColorMapConfig> color_map;
    std::optional<PathConfig> path;
    std::optional<Camera2DConfig> camera2d;
    std::optional<Id3DViewConfig> id_3d_view;
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
