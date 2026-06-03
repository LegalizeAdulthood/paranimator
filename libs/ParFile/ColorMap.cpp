// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/ColorMap.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <istream>
#include <iterator>
#include <ostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace ParFile
{

namespace
{

struct HslColor
{
    double hue{};
    double saturation{};
    double lightness{};
};

bool is_blank(const std::string &line)
{
    return std::all_of(line.begin(), line.end(), [](unsigned char c) { return std::isspace(c) != 0; });
}

int parse_component(const std::string &text, int line_number)
{
    try
    {
        std::size_t length{};
        const int value{std::stoi(text, &length)};
        if (length != text.size() || value < 0 || value > 255)
        {
            throw std::runtime_error("Invalid RGB component on map line " + std::to_string(line_number));
        }
        return value;
    }
    catch (const std::invalid_argument &)
    {
        throw std::runtime_error("Invalid RGB component on map line " + std::to_string(line_number));
    }
    catch (const std::out_of_range &)
    {
        throw std::runtime_error("Invalid RGB component on map line " + std::to_string(line_number));
    }
}

RgbColor parse_entry(const std::string &line, int line_number)
{
    std::istringstream entry{line};
    std::string red;
    std::string green;
    std::string blue;
    if (!(entry >> red >> green >> blue))
    {
        throw std::runtime_error("Malformed map entry on line " + std::to_string(line_number));
    }
    return {parse_component(red, line_number), parse_component(green, line_number), parse_component(blue, line_number)};
}

void validate_component(int value)
{
    if (value < 0 || value > 255)
    {
        throw std::runtime_error("Color map component is outside the range 0 through 255");
    }
}

int interpolate_component(int from, int to, double blend)
{
    return static_cast<int>(std::lround(from + blend * (to - from)));
}

int scale_component(int value, double amount)
{
    return std::clamp(static_cast<int>(std::lround(value * amount)), 0, 255);
}

int component_from_unit(double value)
{
    return std::clamp(static_cast<int>(std::lround(value * 255.0)), 0, 255);
}

double component_to_unit(int value)
{
    return value / 255.0;
}

void require_finite_amount(const std::string &effect, double amount)
{
    if (!std::isfinite(amount))
    {
        throw std::runtime_error("Color map " + effect + " amount must be finite");
    }
}

int gamma_component(int value, double amount)
{
    return component_from_unit(std::pow(component_to_unit(value), amount));
}

int contrast_component(int value, double amount)
{
    return std::clamp(static_cast<int>(std::lround((value - 128.0) * amount + 128.0)), 0, 255);
}

double wrap_degrees(double value)
{
    double result{std::fmod(value, 360.0)};
    if (result < 0.0)
    {
        result += 360.0;
    }
    return result;
}

RgbColor rgb_from_hsl(double hue, double saturation, double lightness)
{
    const double bounded_saturation{std::clamp(saturation, 0.0, 1.0)};
    const double bounded_lightness{std::clamp(lightness, 0.0, 1.0)};
    const double chroma{(1.0 - std::fabs(2.0 * bounded_lightness - 1.0)) * bounded_saturation};
    const double hue_sector{wrap_degrees(hue) / 60.0};
    const double x{chroma * (1.0 - std::fabs(std::fmod(hue_sector, 2.0) - 1.0))};
    double red{};
    double green{};
    double blue{};
    if (hue_sector < 1.0)
    {
        red = chroma;
        green = x;
    }
    else if (hue_sector < 2.0)
    {
        red = x;
        green = chroma;
    }
    else if (hue_sector < 3.0)
    {
        green = chroma;
        blue = x;
    }
    else if (hue_sector < 4.0)
    {
        green = x;
        blue = chroma;
    }
    else if (hue_sector < 5.0)
    {
        red = x;
        blue = chroma;
    }
    else
    {
        red = chroma;
        blue = x;
    }
    const double match{bounded_lightness - chroma / 2.0};
    return {component_from_unit(red + match), component_from_unit(green + match),
        component_from_unit(blue + match)};
}

HslColor hsl_from_rgb(const RgbColor &color)
{
    const double red{component_to_unit(color.red)};
    const double green{component_to_unit(color.green)};
    const double blue{component_to_unit(color.blue)};
    const double maximum{std::max(red, std::max(green, blue))};
    const double minimum{std::min(red, std::min(green, blue))};
    const double delta{maximum - minimum};
    const double lightness{(maximum + minimum) / 2.0};
    if (delta == 0.0)
    {
        return {0.0, 0.0, lightness};
    }

    double hue{};
    if (maximum == red)
    {
        hue = 60.0 * std::fmod((green - blue) / delta, 6.0);
    }
    else if (maximum == green)
    {
        hue = 60.0 * ((blue - red) / delta + 2.0);
    }
    else
    {
        hue = 60.0 * ((red - green) / delta + 4.0);
    }
    const double saturation{delta / (1.0 - std::fabs(2.0 * lightness - 1.0))};
    return {wrap_degrees(hue), saturation, lightness};
}

std::size_t wrap_index(int index, int size)
{
    const int wrapped{index % size};
    return static_cast<std::size_t>(wrapped < 0 ? wrapped + size : wrapped);
}

std::size_t rotate_source_index(std::size_t destination, int offset, int size)
{
    return wrap_index(static_cast<int>(destination) - offset, size);
}

void validate_range(int first, int last)
{
    if (first < 0 || last < 0 || first > last || last >= static_cast<int>(COLOR_MAP_SIZE))
    {
        throw std::runtime_error("Color map range is invalid");
    }
}

void validate_range(ColorMapRange range)
{
    validate_range(range.first, range.last);
}

void validate_ranges(const std::vector<ColorMapRange> &ranges)
{
    if (ranges.empty())
    {
        throw std::runtime_error("Color map ranges must not be empty");
    }
    for (const ColorMapRange &range : ranges)
    {
        validate_range(range);
    }
}

void validate_blend_amount(const std::string &effect, double amount)
{
    if (!std::isfinite(amount) || amount < 0.0 || amount > 1.0)
    {
        throw std::runtime_error("Color map " + effect + " amount must be between 0 and 1");
    }
}

int blend_component(int from, int to, double amount)
{
    return std::clamp(interpolate_component(from, to, amount), 0, 255);
}

RgbColor blend_color(const RgbColor &from, const RgbColor &to, double amount)
{
    return {blend_component(from.red, to.red, amount), blend_component(from.green, to.green, amount),
        blend_component(from.blue, to.blue, amount)};
}

int sparkle_component(int value, int amount, std::mt19937 &engine)
{
    std::uniform_int_distribution<int> distribution{-amount, amount};
    return std::clamp(value + distribution(engine), 0, 255);
}

void validate_sequence(const std::vector<ColorMapSequenceEntry> &sequence, int crossfade)
{
    if (sequence.empty())
    {
        throw std::runtime_error("Color map sequence requires at least one map");
    }
    if (crossfade < 0)
    {
        throw std::runtime_error("Color map sequence crossfade must not be negative");
    }
    for (std::size_t i = 0; i < sequence.size(); ++i)
    {
        if (sequence[i].frame < 0)
        {
            throw std::runtime_error("Color map sequence frames must not be negative");
        }
        if (i != 0U && sequence[i - 1U].frame >= sequence[i].frame)
        {
            throw std::runtime_error("Color map sequence frames must be increasing");
        }
    }
}

void validate_gradient_stops(const std::vector<ColorMapGradientStop> &stops)
{
    if (stops.size() < 2U)
    {
        throw std::runtime_error("Gradient color map requires at least two stops");
    }
    for (std::size_t i = 0; i < stops.size(); ++i)
    {
        if (stops[i].index < 0 || stops[i].index >= static_cast<int>(COLOR_MAP_SIZE))
        {
            throw std::runtime_error("Gradient color map stop index is outside the range 0 through 255");
        }
        validate_component(stops[i].color.red);
        validate_component(stops[i].color.green);
        validate_component(stops[i].color.blue);
        if (i != 0U && stops[i - 1U].index >= stops[i].index)
        {
            throw std::runtime_error("Gradient color map stop indexes must be increasing");
        }
    }
}

void validate_remap_indices(const std::vector<int> &indices)
{
    if (indices.size() != COLOR_MAP_SIZE)
    {
        throw std::runtime_error("Color map remap requires 256 indices");
    }
    for (int index : indices)
    {
        if (index < 0 || index >= static_cast<int>(COLOR_MAP_SIZE))
        {
            throw std::runtime_error("Color map remap index is outside the range 0 through 255");
        }
    }
}

int ping_pong_offset(int offset, int length)
{
    if (length <= 1)
    {
        return 0;
    }

    const int span{length - 1};
    const int period{span * 2};
    int phase{static_cast<int>(wrap_index(offset, period))};
    if (phase > span)
    {
        phase = period - phase;
    }
    return phase;
}

} // namespace

ColorMap read_color_map(std::istream &contents)
{
    ColorMap result;
    std::string line;
    std::size_t index{};
    int line_number{};
    while (std::getline(contents, line))
    {
        ++line_number;
        if (index >= COLOR_MAP_SIZE)
        {
            if (!is_blank(line))
            {
                throw std::runtime_error("Color map has more than 256 entries");
            }
            continue;
        }
        result[index] = parse_entry(line, line_number);
        ++index;
    }
    if (index != COLOR_MAP_SIZE)
    {
        throw std::runtime_error("Color map must contain exactly 256 entries");
    }
    return result;
}

void write_color_map(std::ostream &contents, const ColorMap &map)
{
    for (const RgbColor &color : map)
    {
        validate_component(color.red);
        validate_component(color.green);
        validate_component(color.blue);
        contents << color.red << ' ' << color.green << ' ' << color.blue << '\n';
    }
}

ColorMap interpolate_color_map(const ColorMap &from, const ColorMap &to, double blend)
{
    if (!std::isfinite(blend) || blend < 0.0 || blend > 1.0)
    {
        throw std::runtime_error("Color map blend must be between 0 and 1");
    }
    ColorMap result;
    for (std::size_t i = 0; i < result.size(); ++i)
    {
        result[i] = {interpolate_component(from[i].red, to[i].red, blend),
            interpolate_component(from[i].green, to[i].green, blend),
            interpolate_component(from[i].blue, to[i].blue, blend)};
    }
    return result;
}

ColorMap brightness_color_map(const ColorMap &map, double amount)
{
    require_finite_amount("brightness", amount);

    ColorMap result;
    for (std::size_t i = 0; i < result.size(); ++i)
    {
        result[i] = {scale_component(map[i].red, amount), scale_component(map[i].green, amount),
            scale_component(map[i].blue, amount)};
    }
    return result;
}

ColorMap contrast_color_map(const ColorMap &map, double amount)
{
    require_finite_amount("contrast", amount);

    ColorMap result;
    for (std::size_t i = 0; i < result.size(); ++i)
    {
        result[i] = {contrast_component(map[i].red, amount), contrast_component(map[i].green, amount),
            contrast_component(map[i].blue, amount)};
    }
    return result;
}

ColorMap gamma_color_map(const ColorMap &map, double amount)
{
    require_finite_amount("gamma", amount);
    if (amount <= 0.0)
    {
        throw std::runtime_error("Color map gamma amount must be positive");
    }

    ColorMap result;
    for (std::size_t i = 0; i < result.size(); ++i)
    {
        result[i] = {gamma_component(map[i].red, amount), gamma_component(map[i].green, amount),
            gamma_component(map[i].blue, amount)};
    }
    return result;
}

ColorMap hue_shift_color_map(const ColorMap &map, double amount)
{
    require_finite_amount("hue-shift", amount);

    ColorMap result;
    for (std::size_t i = 0; i < result.size(); ++i)
    {
        const HslColor hsl{hsl_from_rgb(map[i])};
        result[i] = rgb_from_hsl(hsl.hue + amount, hsl.saturation, hsl.lightness);
    }
    return result;
}

ColorMap gradient_color_map(const std::vector<ColorMapGradientStop> &stops)
{
    validate_gradient_stops(stops);
    ColorMap result;

    std::size_t stop_index{};
    for (std::size_t i = 0; i < result.size(); ++i)
    {
        while (stop_index + 1U < stops.size() &&
            stops[stop_index + 1U].index < static_cast<int>(i))
        {
            ++stop_index;
        }

        const ColorMapGradientStop &from{stops[stop_index]};
        if (static_cast<int>(i) <= from.index || stop_index + 1U == stops.size())
        {
            result[i] = from.color;
            continue;
        }

        const ColorMapGradientStop &to{stops[stop_index + 1U]};
        const double blend{(static_cast<int>(i) - from.index) / static_cast<double>(to.index - from.index)};
        result[i] = {interpolate_component(from.color.red, to.color.red, blend),
            interpolate_component(from.color.green, to.color.green, blend),
            interpolate_component(from.color.blue, to.color.blue, blend)};
    }
    return result;
}

ColorMap mask_blend_color_map(
    const ColorMap &map, const ColorMap &mask, const std::vector<ColorMapRange> &ranges, double amount)
{
    validate_ranges(ranges);
    validate_blend_amount("mask-blend", amount);

    ColorMap result{map};
    for (const ColorMapRange &range : ranges)
    {
        for (int i = range.first; i <= range.last; ++i)
        {
            const std::size_t index{static_cast<std::size_t>(i)};
            result[index] = blend_color(map[index], mask[index], amount);
        }
    }
    return result;
}

ColorMap pulse_color_map(const ColorMap &map, ColorMapRange range, RgbColor color, double amount)
{
    validate_range(range);
    validate_component(color.red);
    validate_component(color.green);
    validate_component(color.blue);
    validate_blend_amount("pulse", amount);

    ColorMap result{map};
    for (int i = range.first; i <= range.last; ++i)
    {
        const std::size_t index{static_cast<std::size_t>(i)};
        result[index] = blend_color(map[index], color, amount);
    }
    return result;
}

ColorMap remap_color_map(const ColorMap &map, const std::vector<int> &indices)
{
    validate_remap_indices(indices);

    ColorMap result;
    for (std::size_t i = 0; i < result.size(); ++i)
    {
        result[i] = map[static_cast<std::size_t>(indices[i])];
    }
    return result;
}

ColorMap saturation_color_map(const ColorMap &map, double amount)
{
    require_finite_amount("saturation", amount);

    ColorMap result;
    for (std::size_t i = 0; i < result.size(); ++i)
    {
        const HslColor hsl{hsl_from_rgb(map[i])};
        result[i] = rgb_from_hsl(hsl.hue, hsl.saturation * amount, hsl.lightness);
    }
    return result;
}

ColorMap sparkle_color_map(const ColorMap &map, ColorMapRange range, int seed, double amount)
{
    validate_range(range);
    if (!std::isfinite(amount) || amount < 0.0 || amount > 255.0)
    {
        throw std::runtime_error("Color map sparkle amount must be between 0 and 255");
    }

    const int rounded_amount{static_cast<int>(std::lround(amount))};
    std::mt19937 engine{static_cast<std::mt19937::result_type>(seed)};
    ColorMap result{map};
    for (int i = range.first; i <= range.last; ++i)
    {
        const std::size_t index{static_cast<std::size_t>(i)};
        result[index] = {sparkle_component(map[index].red, rounded_amount, engine),
            sparkle_component(map[index].green, rounded_amount, engine),
            sparkle_component(map[index].blue, rounded_amount, engine)};
    }
    return result;
}

ColorMap rotate_color_map(const ColorMap &map, int offset)
{
    ColorMap result;
    for (std::size_t i = 0; i < result.size(); ++i)
    {
        result[i] = map[rotate_source_index(i, offset, static_cast<int>(COLOR_MAP_SIZE))];
    }
    return result;
}

ColorMap rotate_color_map_range(const ColorMap &map, int first, int last, int offset)
{
    validate_range(first, last);
    ColorMap result{map};
    const int length{last - first + 1};
    for (int i = first; i <= last; ++i)
    {
        const std::size_t source{static_cast<std::size_t>(first) + rotate_source_index(i - first, offset, length)};
        result[static_cast<std::size_t>(i)] = map[source];
    }
    return result;
}

ColorMap reverse_color_map(const ColorMap &map)
{
    ColorMap result;
    std::reverse_copy(map.begin(), map.end(), result.begin());
    return result;
}

ColorMap reverse_color_map_range(const ColorMap &map, int first, int last)
{
    validate_range(first, last);
    ColorMap result{map};
    std::reverse_copy(std::next(map.begin(), first), std::next(map.begin(), last + 1),
        std::next(result.begin(), first));
    return result;
}

ColorMap ping_pong_color_map(const ColorMap &map, int offset)
{
    return rotate_color_map(map, ping_pong_offset(offset, static_cast<int>(COLOR_MAP_SIZE)));
}

ColorMap ping_pong_color_map_range(const ColorMap &map, int first, int last, int offset)
{
    validate_range(first, last);
    return rotate_color_map_range(map, first, last, ping_pong_offset(offset, last - first + 1));
}

ColorMap sequence_color_map(const std::vector<ColorMapSequenceEntry> &sequence, int frame, int crossfade)
{
    validate_sequence(sequence, crossfade);
    const ColorMapSequenceEntry *current{&sequence.front()};
    for (std::size_t i = 1; i < sequence.size(); ++i)
    {
        const ColorMapSequenceEntry &next{sequence[i]};
        if (frame < next.frame)
        {
            const int blend_start{std::max(current->frame, next.frame - crossfade)};
            if (crossfade != 0 && frame > blend_start)
            {
                const double blend{(frame - blend_start) / static_cast<double>(next.frame - blend_start)};
                return interpolate_color_map(current->map, next.map, blend);
            }
            return current->map;
        }
        current = &next;
    }
    return current->map;
}

} // namespace ParFile
