// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/ColorMap.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <istream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace ParFile
{

namespace
{

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

std::size_t rotate_source_index(std::size_t destination, int offset)
{
    const int size{static_cast<int>(COLOR_MAP_SIZE)};
    const int source{(static_cast<int>(destination) - offset) % size};
    return static_cast<std::size_t>(source < 0 ? source + size : source);
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

ColorMap rotate_color_map(const ColorMap &map, int offset)
{
    ColorMap result;
    for (std::size_t i = 0; i < result.size(); ++i)
    {
        result[i] = map[rotate_source_index(i, offset)];
    }
    return result;
}

} // namespace ParFile
