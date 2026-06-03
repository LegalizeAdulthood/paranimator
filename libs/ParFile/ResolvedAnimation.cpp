// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/ResolvedAnimation.h>

#include <algorithm>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

namespace ParFile
{

namespace
{

bool starts_with(std::string_view text, std::string_view prefix)
{
    return text.size() >= prefix.size() && text.substr(0, prefix.size()) == prefix;
}

const Parameter &source_parameter(const ParSet &source, std::string_view name)
{
    const std::string key{name};
    const auto is_name{[&](const Parameter &param) { return param.name == key; }};
    const auto it{std::find_if(source.params.begin(), source.params.end(), is_name)};
    if (it == source.params.end())
    {
        throw std::runtime_error("Parameter set '" + source.name + "' has no parameter '" + std::string{name} + "'");
    }
    return *it;
}

std::string source_type(const ParSet &source)
{
    return source_parameter(source, "type").value;
}

int parse_params_slot(std::string_view parameter)
{
    if (!starts_with(parameter, "params[") || parameter.back() != ']')
    {
        throw std::runtime_error("Invalid params slot track '" + std::string{parameter} + "'");
    }

    const std::string slot_text{parameter.substr(7, parameter.size() - 8)};
    try
    {
        std::size_t length{};
        const int slot{std::stoi(slot_text, &length)};
        if (length != slot_text.size())
        {
            throw std::runtime_error("Invalid params slot track '" + std::string{parameter} + "'");
        }
        return slot;
    }
    catch (const std::invalid_argument &)
    {
        throw std::runtime_error("Invalid params slot track '" + std::string{parameter} + "'");
    }
    catch (const std::out_of_range &)
    {
        throw std::runtime_error("Params slot out of range in track '" + std::string{parameter} + "'");
    }
}

ResolvedTrack resolve_params_slot(
    const TrackConfig &track, const ParameterCatalog &catalog, const ParSet &source, std::string_view fractal_type)
{
    const int slot{parse_params_slot(track.parameter)};
    const ParamsSlotMetadata &slot_metadata{catalog.params_slot(fractal_type, slot)};
    const Parameter &params{source_parameter(source, "params")};
    return {track.parameter, slot_metadata.metadata, params.value, track.keys, "params", {slot}};
}

ResolvedTrack resolve_params_group(
    const TrackConfig &track, const ParameterCatalog &catalog, const ParSet &source, std::string_view fractal_type)
{
    const std::string_view group{std::string_view{track.parameter}.substr(7)};
    if (group.empty())
    {
        throw std::runtime_error("Invalid params group track '" + track.parameter + "'");
    }
    const ParamsGroupMetadata &group_metadata{catalog.params_group(fractal_type, group)};
    const Parameter &params{source_parameter(source, "params")};
    return {track.parameter, group_metadata.metadata, params.value, track.keys, "params", group_metadata.slots};
}

ResolvedTrack resolve_regular_track(const TrackConfig &track, const ParameterCatalog &catalog, const ParSet &source)
{
    const ParameterMetadata &metadata{catalog.metadata(track.parameter)};
    const Parameter &parameter{source_parameter(source, track.parameter)};
    return {track.parameter, metadata, parameter.value, track.keys, track.parameter, {}};
}

ResolvedTrack resolve_track(const TrackConfig &track, const ParameterCatalog &catalog, const ParSet &source)
{
    if (starts_with(track.parameter, "params["))
    {
        return resolve_params_slot(track, catalog, source, source_type(source));
    }
    if (starts_with(track.parameter, "params."))
    {
        return resolve_params_group(track, catalog, source, source_type(source));
    }
    return resolve_regular_track(track, catalog, source);
}

} // namespace

ResolvedAnimation resolve_animation(const Config &config, const ParameterCatalog &catalog, const ParSet &source)
{
    ResolvedAnimation result;
    result.frame_name = config.output.entry;
    result.video = config.video;
    result.num_frames = config.num_frames;
    result.source = source;

    for (const TrackConfig &track : config.tracks)
    {
        result.tracks.push_back(resolve_track(track, catalog, source));
    }

    return result;
}

} // namespace ParFile
