// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/Interpolator.h>

#include <ParFile/ColorMap.h>
#include <ParFile/ColorSpec.h>
#include <ParFile/Config.h>
#include <ParFile/Interpolant.h>
#include <ParFile/NumberTrack.h>
#include <ParFile/OutputLayout.h>
#include <ParFile/ParFile.h>
#include <ParFile/ParameterCatalog.h>
#include <ParFile/ResolvedAnimation.h>

#include <algorithm>
#include <boost/algorithm/string/split.hpp>
#include <boost/format.hpp>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace ParFile
{

namespace
{

void validate_track_keyframes(const std::string &name, const std::vector<KeyframeConfig> &keys, int num_frames)
{
    if (keys.size() != 2U)
    {
        throw std::runtime_error("Track '" + name + "' requires exactly two keyframes");
    }
    if (keys[0].frame < 0 || keys[1].frame < 0 || keys[0].frame >= num_frames || keys[1].frame >= num_frames)
    {
        throw std::runtime_error("Track '" + name + "' has keyframes outside the frame range");
    }
    if (keys[0].frame >= keys[1].frame)
    {
        throw std::runtime_error("Track '" + name + "' keyframes must be in increasing order");
    }
}

bool color_map_effect_uses_amount(ColorMapEffectKind kind)
{
    return kind == ColorMapEffectKind::BRIGHTNESS || kind == ColorMapEffectKind::CONTRAST ||
        kind == ColorMapEffectKind::GAMMA || kind == ColorMapEffectKind::HUE_SHIFT ||
        kind == ColorMapEffectKind::MASK_BLEND || kind == ColorMapEffectKind::PULSE ||
        kind == ColorMapEffectKind::SATURATION || kind == ColorMapEffectKind::SPARKLE;
}

std::string color_map_effect_name(ColorMapEffectKind kind)
{
    switch (kind)
    {
    case ColorMapEffectKind::BRIGHTNESS:
        return "brightness";
    case ColorMapEffectKind::CONTRAST:
        return "contrast";
    case ColorMapEffectKind::GAMMA:
        return "gamma";
    case ColorMapEffectKind::HUE_SHIFT:
        return "hue-shift";
    case ColorMapEffectKind::MASK_BLEND:
        return "mask-blend";
    case ColorMapEffectKind::PULSE:
        return "pulse";
    case ColorMapEffectKind::REMAP:
        return "remap";
    case ColorMapEffectKind::SATURATION:
        return "saturation";
    case ColorMapEffectKind::SPARKLE:
        return "sparkle";
    case ColorMapEffectKind::REVERSE:
        return "reverse";
    case ColorMapEffectKind::PING_PONG:
        return "ping-pong";
    }
    return "unknown";
}

void validate_gamma_amounts(const NumberTrackConfig &track)
{
    for (const NumberKeyframeConfig &key : track.keys)
    {
        if (key.value <= 0.0)
        {
            throw std::runtime_error("Color map gamma amount must be positive");
        }
    }
}

void validate_amount_range(const std::string &name, const NumberTrackConfig &track, double min, double max)
{
    for (const NumberKeyframeConfig &key : track.keys)
    {
        if (key.value < min || key.value > max)
        {
            throw std::runtime_error("Color map " + name + " amount is outside the valid range");
        }
    }
}

double color_map_effect_amount_at_frame(const ColorMapEffectConfig &effect, int frame)
{
    if (!effect.amount)
    {
        throw std::runtime_error("Color map " + color_map_effect_name(effect.kind) + " effect is missing amount");
    }
    return number_track_value_at_frame(*effect.amount, frame);
}

void require_range(const ColorMapEffectConfig &effect)
{
    if (!effect.range)
    {
        throw std::runtime_error("Color map " + color_map_effect_name(effect.kind) + " effect is missing range");
    }
}

void validate_remap_indices(const std::vector<int> &indices)
{
    if (indices.size() != COLOR_MAP_SIZE)
    {
        throw std::runtime_error("Color map remap effect requires 256 indices");
    }
    for (int index : indices)
    {
        if (index < 0 || index >= static_cast<int>(COLOR_MAP_SIZE))
        {
            throw std::runtime_error("Color map remap index is outside the range 0 through 255");
        }
    }
}

ColorMapRange to_color_map_range(const ColorMapRangeConfig &range)
{
    return {range.first, range.last};
}

std::vector<ColorMapRange> to_color_map_ranges(const std::vector<ColorMapRangeConfig> &ranges)
{
    std::vector<ColorMapRange> result;
    result.reserve(ranges.size());
    for (const ColorMapRangeConfig &range : ranges)
    {
        result.push_back(to_color_map_range(range));
    }
    return result;
}

class ColorMapInterpolant : public Interpolant
{
public:
    ColorMapInterpolant(const TrackConfig &track, const std::filesystem::path &map_directory, int num_frames);
    ~ColorMapInterpolant() override = default;

    const std::string &name() const override
    {
        return m_parameter;
    }
    const std::vector<int> &output_slots() const override
    {
        return m_slots;
    }
    bool has_value() const override
    {
        return true;
    }
    std::string step() override;

private:
    double blend_at_frame(int frame) const;
    ColorMap map_at_frame(int frame) const;
    ColorMap apply_effect(const ColorMap &map, const ColorMapEffectConfig &effect, int frame) const;
    static ColorMap load_gradient_map(const ColorMapGradientConfig &gradient);
    std::string output_filename(int frame) const;
    ColorMap read_source_map(const std::string &filename) const;
    void write_generated_map(const std::filesystem::path &filename, const ColorMap &map) const;

    std::string m_parameter;
    std::vector<KeyframeConfig> m_keys;
    std::optional<std::string> m_source;
    std::optional<ColorMap> m_gradient;
    std::vector<ColorMapEffectConfig> m_effects;
    std::filesystem::path m_map_directory;
    std::string m_output;
    std::vector<int> m_slots;
    int m_frame{};
};

ColorMapInterpolant::ColorMapInterpolant(
    const TrackConfig &track, const std::filesystem::path &map_directory, int num_frames) :
    m_parameter(track.parameter),
    m_keys(track.keys),
    m_map_directory(map_directory)
{
    if (!track.color_map)
    {
        throw std::runtime_error("Color map track '" + track.parameter + "' is missing color map settings");
    }
    if (track.color_map->format != TrackFormat::AT_FILE)
    {
        throw std::runtime_error("Color map track '" + track.parameter + "' requires at-file format");
    }
    m_output = track.color_map->output;
    m_source = track.color_map->source;
    if (track.color_map->gradient)
    {
        m_gradient = load_gradient_map(*track.color_map->gradient);
    }
    if (m_source && m_gradient)
    {
        throw std::runtime_error("Color map track '" + track.parameter + "' has multiple sources");
    }
    m_effects = track.color_map->effects;
    if (!m_source && !m_gradient)
    {
        validate_track_keyframes(track.parameter, track.keys, num_frames);
        if (track.keys[1].curve == Curve::GEOMETRIC)
        {
            throw std::runtime_error("Color map track '" + track.parameter + "' does not support geometric curves");
        }
    }
    for (const ColorMapEffectConfig &effect : m_effects)
    {
        if (color_map_effect_uses_amount(effect.kind))
        {
            if (!effect.amount)
            {
                throw std::runtime_error(
                    "Color map " + color_map_effect_name(effect.kind) + " effect is missing amount");
            }
            validate_number_track_keyframes(
                "color map " + color_map_effect_name(effect.kind) + " amount", effect.amount->keys, num_frames);
            if (effect.kind == ColorMapEffectKind::GAMMA)
            {
                validate_gamma_amounts(*effect.amount);
            }
            if (effect.kind == ColorMapEffectKind::MASK_BLEND || effect.kind == ColorMapEffectKind::PULSE)
            {
                validate_amount_range(color_map_effect_name(effect.kind), *effect.amount, 0.0, 1.0);
            }
            if (effect.kind == ColorMapEffectKind::SPARKLE)
            {
                validate_amount_range(color_map_effect_name(effect.kind), *effect.amount, 0.0, 255.0);
            }
        }
        else if (effect.kind == ColorMapEffectKind::PING_PONG)
        {
            if (!effect.offset)
            {
                throw std::runtime_error("Color map ping-pong effect is missing offset");
            }
            validate_number_track_keyframes("color map ping-pong offset", effect.offset->keys, num_frames);
        }
        switch (effect.kind)
        {
        case ColorMapEffectKind::MASK_BLEND:
            if (effect.ranges.empty())
            {
                throw std::runtime_error("Color map mask-blend effect is missing ranges");
            }
            if (!effect.source)
            {
                throw std::runtime_error("Color map mask-blend effect is missing source");
            }
            break;
        case ColorMapEffectKind::PULSE:
            require_range(effect);
            if (!effect.color)
            {
                throw std::runtime_error("Color map pulse effect is missing color");
            }
            static_cast<void>(parse_color_spec(*effect.color));
            break;
        case ColorMapEffectKind::REMAP:
            validate_remap_indices(effect.indices);
            break;
        case ColorMapEffectKind::SPARKLE:
            require_range(effect);
            if (!effect.seed)
            {
                throw std::runtime_error("Color map sparkle effect is missing seed");
            }
            break;
        case ColorMapEffectKind::BRIGHTNESS:
        case ColorMapEffectKind::CONTRAST:
        case ColorMapEffectKind::GAMMA:
        case ColorMapEffectKind::HUE_SHIFT:
        case ColorMapEffectKind::SATURATION:
        case ColorMapEffectKind::REVERSE:
        case ColorMapEffectKind::PING_PONG:
            break;
        }
    }
}

double ColorMapInterpolant::blend_at_frame(int frame) const
{
    if (frame <= m_keys[0].frame)
    {
        return 0.0;
    }
    if (frame >= m_keys[1].frame)
    {
        return 1.0;
    }
    if (m_keys[1].curve == Curve::HOLD || m_keys[1].curve == Curve::STEP)
    {
        return 0.0;
    }
    return (frame - m_keys[0].frame) / static_cast<double>(m_keys[1].frame - m_keys[0].frame);
}

ColorMap ColorMapInterpolant::map_at_frame(int frame) const
{
    if (m_gradient)
    {
        ColorMap result{*m_gradient};
        for (const ColorMapEffectConfig &effect : m_effects)
        {
            result = apply_effect(result, effect, frame);
        }
        return result;
    }
    if (m_source)
    {
        ColorMap result{read_source_map(*m_source)};
        for (const ColorMapEffectConfig &effect : m_effects)
        {
            result = apply_effect(result, effect, frame);
        }
        return result;
    }
    const ColorMap from{read_source_map(m_keys[0].value)};
    const ColorMap to{read_source_map(m_keys[1].value)};
    return interpolate_color_map(from, to, blend_at_frame(frame));
}

ColorMap ColorMapInterpolant::apply_effect(const ColorMap &map, const ColorMapEffectConfig &effect, int frame) const
{
    switch (effect.kind)
    {
    case ColorMapEffectKind::BRIGHTNESS:
        return brightness_color_map(map, color_map_effect_amount_at_frame(effect, frame));
    case ColorMapEffectKind::CONTRAST:
        return contrast_color_map(map, color_map_effect_amount_at_frame(effect, frame));
    case ColorMapEffectKind::GAMMA:
        return gamma_color_map(map, color_map_effect_amount_at_frame(effect, frame));
    case ColorMapEffectKind::HUE_SHIFT:
        return hue_shift_color_map(map, color_map_effect_amount_at_frame(effect, frame));
    case ColorMapEffectKind::MASK_BLEND:
        if (!effect.source)
        {
            throw std::runtime_error("Color map mask-blend effect is missing source");
        }
        return mask_blend_color_map(map, read_source_map(*effect.source), to_color_map_ranges(effect.ranges),
            color_map_effect_amount_at_frame(effect, frame));
    case ColorMapEffectKind::PULSE:
        if (!effect.range || !effect.color)
        {
            throw std::runtime_error("Color map pulse effect is missing range or color");
        }
        return pulse_color_map(map, to_color_map_range(*effect.range), parse_color_spec(*effect.color),
            color_map_effect_amount_at_frame(effect, frame));
    case ColorMapEffectKind::REMAP:
        return remap_color_map(map, effect.indices);
    case ColorMapEffectKind::SATURATION:
        return saturation_color_map(map, color_map_effect_amount_at_frame(effect, frame));
    case ColorMapEffectKind::SPARKLE:
        if (!effect.range || !effect.seed)
        {
            throw std::runtime_error("Color map sparkle effect is missing range or seed");
        }
        return sparkle_color_map(
            map, to_color_map_range(*effect.range), *effect.seed, color_map_effect_amount_at_frame(effect, frame));
    case ColorMapEffectKind::REVERSE:
        if (effect.range)
        {
            return reverse_color_map_range(map, effect.range->first, effect.range->last);
        }
        return reverse_color_map(map);
    case ColorMapEffectKind::PING_PONG:
    {
        if (!effect.offset)
        {
            throw std::runtime_error("Color map ping-pong effect is missing offset");
        }
        const int offset{static_cast<int>(std::lround(number_track_value_at_frame(*effect.offset, frame)))};
        if (effect.range)
        {
            return ping_pong_color_map_range(map, effect.range->first, effect.range->last, offset);
        }
        return ping_pong_color_map(map, offset);
    }
    }
    return map;
}

ColorMap ColorMapInterpolant::load_gradient_map(const ColorMapGradientConfig &gradient)
{
    std::vector<ColorMapGradientStop> stops;
    stops.reserve(gradient.stops.size());
    for (const ColorMapGradientStopConfig &stop : gradient.stops)
    {
        stops.push_back({stop.index, parse_color_spec(stop.color)});
    }
    return gradient_color_map(stops);
}

std::string ColorMapInterpolant::output_filename(int frame) const
{
    const std::string formatted{(boost::format(m_output) % (frame + 1)).str()};
    const std::filesystem::path filename{formatted};
    if (filename.has_parent_path())
    {
        throw std::runtime_error("Color map output must be a filename, not a path");
    }
    return filename.string();
}

ColorMap ColorMapInterpolant::read_source_map(const std::string &filename) const
{
    std::ifstream in{filename};
    if (!in)
    {
        throw std::runtime_error("Unable to read color map '" + filename + "'");
    }
    return read_color_map(in);
}

void ColorMapInterpolant::write_generated_map(const std::filesystem::path &filename, const ColorMap &map) const
{
    std::filesystem::create_directories(filename.parent_path());
    std::ofstream out{filename.string().c_str()};
    if (!out)
    {
        throw std::runtime_error("Unable to write color map '" + filename.string() + "'");
    }
    write_color_map(out, map);
}

std::string ColorMapInterpolant::step()
{
    const int frame{m_frame};
    ++m_frame;
    const std::string filename{output_filename(frame)};
    write_generated_map(m_map_directory / filename, map_at_frame(frame));
    return '@' + filename;
}

} // namespace

static ParSet load_par_set(const NamedFileParSet &par_entry)
{
    std::string filename{par_entry.file};
    std::string name{par_entry.name};
    std::ifstream in{filename};
    ParFilePtr file{create_par_file(in)};
    const auto it{
        std::find_if(file->cbegin(), file->cend(), [&](const ParSet &params) { return params.name == name; })};
    if (it == file->cend())
    {
        throw std::runtime_error("Couldn't find parameter set '" + name + "' in file '" + filename + "'");
    }
    return *it;
}

static std::string read_text(const std::filesystem::path &path)
{
    std::ifstream in{path};
    if (!in)
    {
        throw std::runtime_error("Unable to read file '" + path.string() + "'");
    }
    return {std::istreambuf_iterator<char>{in}, std::istreambuf_iterator<char>{}};
}

static ParameterCatalog load_parameter_catalog(const Config &config)
{
    if (config.parameter_catalogs.size() != 1U)
    {
        throw std::runtime_error("Expected exactly one parameter catalog");
    }
    return read_parameter_catalog(read_text(config.parameter_catalogs[0]));
}

static ResolvedAnimation load_animation(const Config &config)
{
    const ParSet source{load_par_set(config.source)};
    const ParameterCatalog catalog{load_parameter_catalog(config)};
    return resolve_animation(config, catalog, source);
}

static std::vector<std::string> split_slash_values(std::string_view value)
{
    std::vector<std::string> result;
    boost::algorithm::split(result, value, [](char c) { return c == '/'; });
    return result;
}

static std::string join_slash_values(const std::vector<std::string> &values)
{
    std::string result;
    for (const std::string &value : values)
    {
        if (!result.empty())
        {
            result += '/';
        }
        result += value;
    }
    return result;
}

static void merge_slotted_value(Parameter &param, const std::string &value, const std::vector<int> &slots)
{
    std::vector<std::string> current{split_slash_values(param.value)};
    const std::vector<std::string> update{split_slash_values(value)};
    if (current.size() < update.size())
    {
        const std::size_t old_size{current.size()};
        current.resize(update.size());
        std::copy(std::next(update.begin(), static_cast<std::ptrdiff_t>(old_size)), update.end(),
            std::next(current.begin(), static_cast<std::ptrdiff_t>(old_size)));
    }
    for (const int slot : slots)
    {
        if (slot < 0 || static_cast<std::size_t>(slot) >= update.size())
        {
            throw std::runtime_error("Slotted value for '" + param.name + "' is missing slot " + std::to_string(slot));
        }
        current[static_cast<std::size_t>(slot)] = update[static_cast<std::size_t>(slot)];
    }
    param.value = join_slash_values(current);
}

std::vector<InterpolantPtr> Interpolator::load_interpolants(const ResolvedAnimation &animation)
{
    std::vector<InterpolantPtr> result;
    for (const ResolvedTrack &track : animation.tracks)
    {
        result.emplace_back(create_interpolant(track, animation.num_frames));
    }
    return result;
}

void Interpolator::load_color_map_interpolants(const Config &config)
{
    const OutputLayout output{config};
    for (const TrackConfig &track : config.tracks)
    {
        if (track.kind == TrackKind::COLOR_MAP)
        {
            m_interpolants.emplace_back(
                std::make_shared<ColorMapInterpolant>(track, output.map_directory(), config.num_frames));
        }
    }
}

Interpolator::Interpolator(const Config &config) :
    Interpolator(load_animation(config))
{
    load_color_map_interpolants(config);
}

Interpolator::Interpolator(const Config &config, std::string_view layer_id) :
    Interpolator(load_animation(config), layer_id)
{
    load_color_map_interpolants(config);
}

Interpolator::Interpolator(const ResolvedAnimation &animation) :
    Interpolator(animation, {})
{
}

Interpolator::Interpolator(const ResolvedAnimation &animation, std::string_view layer_id) :
    m_frame_name(animation.frame_name),
    m_layer_id(layer_id),
    m_video(animation.video),
    m_source(animation.source),
    m_interpolants(load_interpolants(animation))
{
}

ParSet Interpolator::operator()()
{
    ++m_frame;
    ParSet par_set{m_source};
    for (const InterpolantPtr &lerper : m_interpolants)
    {
        const auto it{std::find_if(par_set.params.begin(), par_set.params.end(),
            [&](const Parameter &param) { return param.name == lerper->name(); })};
        const std::string value{lerper->step()};
        if (lerper->has_value())
        {
            if (it == par_set.params.end())
            {
                par_set.params.push_back({lerper->name(), value});
            }
            else if (!lerper->output_slots().empty())
            {
                merge_slotted_value(*it, value, lerper->output_slots());
            }
            else
            {
                it->value = value;
            }
        }
        else if (it != par_set.params.end())
        {
            par_set.params.erase(it);
        }
    }
    if (m_layer_id.empty())
    {
        par_set.name = (boost::format(m_frame_name) % m_frame).str();
    }
    else
    {
        par_set.name = (boost::format(m_frame_name) % m_layer_id % m_frame).str();
    }
    par_set.params.push_back({"batch", "yes"});
    par_set.params.push_back({"savename", par_set.name + ".gif"});
    par_set.params.push_back({"overwrite", "yes"});
    par_set.params.push_back({"video", m_video});
    return par_set;
}

} // namespace ParFile
