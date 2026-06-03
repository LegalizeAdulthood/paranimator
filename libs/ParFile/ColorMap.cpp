// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/ColorMap.h>

#include <algorithm>
#include <cctype>
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

} // namespace ParFile
