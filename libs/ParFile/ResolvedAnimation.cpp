// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/ResolvedAnimation.h>

#include <algorithm>
#include <stdexcept>

namespace ParFile
{

ResolvedAnimation resolve_animation(const Config &config, const ParameterCatalog &catalog, const ParSet &source)
{
    ResolvedAnimation result;
    result.frame_name = config.output.entry;
    result.video = config.video;
    result.num_frames = config.num_frames;
    result.source = source;

    for (const TrackConfig &track : config.tracks)
    {
        const ParameterMetadata &metadata{catalog.metadata(track.parameter)};
        const auto is_name{[&](const Parameter &param) { return param.name == track.parameter; }};
        const auto it{std::find_if(source.params.begin(), source.params.end(), is_name)};
        if (it == source.params.end())
        {
            throw std::runtime_error("Parameter set '" + source.name + "' has no parameter '" + track.parameter + "'");
        }
        result.tracks.push_back({track.parameter, metadata, it->value, track.keys});
    }

    return result;
}

} // namespace ParFile
