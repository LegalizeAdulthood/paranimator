// SPDX-License-Identifier: GPL-3.0-only
//
#pragma once

#include <ParFile/ColorMap.h>

#include <string_view>

namespace ParFile
{

RgbColor parse_color_spec(std::string_view text);

} // namespace ParFile
