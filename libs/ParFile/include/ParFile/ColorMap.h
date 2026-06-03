// SPDX-License-Identifier: GPL-3.0-only
//
#pragma once

#include <array>
#include <cstddef>
#include <iosfwd>
#include <vector>

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

struct ColorMapSequenceEntry
{
    int frame{};
    ColorMap map;
};

ColorMap read_color_map(std::istream &contents);
void write_color_map(std::ostream &contents, const ColorMap &map);
ColorMap interpolate_color_map(const ColorMap &from, const ColorMap &to, double blend);
ColorMap rotate_color_map(const ColorMap &map, int offset);
ColorMap rotate_color_map_range(const ColorMap &map, int first, int last, int offset);
ColorMap sequence_color_map(const std::vector<ColorMapSequenceEntry> &sequence, int frame, int crossfade);

} // namespace ParFile
