// SPDX-License-Identifier: GPL-3.0-only
//
#pragma once

#include <array>
#include <cstddef>
#include <iosfwd>

namespace ParFile
{

constexpr std::size_t COLOR_MAP_SIZE{256};

struct RgbColor
{
    int red{};
    int green{};
    int blue{};
};

using ColorMap = std::array<RgbColor, COLOR_MAP_SIZE>;

ColorMap read_color_map(std::istream &contents);
void write_color_map(std::ostream &contents, const ColorMap &map);
ColorMap interpolate_color_map(const ColorMap &from, const ColorMap &to, double blend);
ColorMap rotate_color_map(const ColorMap &map, int offset);

} // namespace ParFile
