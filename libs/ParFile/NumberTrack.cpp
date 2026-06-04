// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/NumberTrack.h>

#include <ParFile/AnimationEnums.h>

#include <stdexcept>
#include <string>

namespace ParFile
{

void validate_number_track_keyframes(
    std::string_view name, const std::vector<NumberKeyframeConfig> &keys, int num_frames)
{
    const std::string label{name};
    if (keys.size() != 2U)
    {
        throw std::runtime_error("Number track '" + label + "' requires exactly two keyframes");
    }
    if (keys[0].frame < 0 || keys[1].frame < 0 || keys[0].frame >= num_frames || keys[1].frame >= num_frames)
    {
        throw std::runtime_error("Number track '" + label + "' has keyframes outside the frame range");
    }
    if (keys[0].frame >= keys[1].frame)
    {
        throw std::runtime_error("Number track '" + label + "' keyframes must be in increasing order");
    }
    if (keys[1].curve == Curve::GEOMETRIC)
    {
        throw std::runtime_error("Number track '" + label + "' does not support geometric curves");
    }
}

double number_track_value_at_frame(const NumberTrackConfig &track, int frame)
{
    const NumberKeyframeConfig &from{track.keys[0]};
    const NumberKeyframeConfig &to{track.keys[1]};
    if (frame <= from.frame)
    {
        return from.value;
    }
    if (frame >= to.frame)
    {
        return to.value;
    }
    if (to.curve == Curve::HOLD || to.curve == Curve::STEP)
    {
        return from.value;
    }
    const double fraction{(frame - from.frame) / static_cast<double>(to.frame - from.frame)};
    return from.value + fraction * (to.value - from.value);
}

} // namespace ParFile
