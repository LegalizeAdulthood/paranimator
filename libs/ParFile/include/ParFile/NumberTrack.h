// SPDX-License-Identifier: GPL-3.0-only
//
#pragma once

#include <ParFile/Config.h>

#include <string_view>

namespace ParFile
{

void validate_number_track_keyframes(
    std::string_view name, const std::vector<NumberKeyframeConfig> &keys, int num_frames);
double number_track_value_at_frame(const NumberTrackConfig &track, int frame);

} // namespace ParFile
