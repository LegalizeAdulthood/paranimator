// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/Interpolator.h>

#include <ParFile/Config.h>
#include <ParFile/Interpolant.h>
#include <ParFile/ParFile.h>

#include <boost/format.hpp>
#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace ParFile
{

static ParSet load_par_set(const NamedFileParSet &par_entry)
{
    std::string filename{par_entry.file};
    std::string name{par_entry.name};
    std::ifstream in{filename};
    ParFilePtr file{create_par_file(in)};
    const auto it{
        std::find_if(file->cbegin(), file->cend(), [&](const ParSet &params) { return params.name == name; })};
    if (it == file->cend())
    {
        throw std::runtime_error("Couldn't find parameter set '" + name + "' in file '" + filename + "'");
    }
    return *it;
}

std::vector<InterpolantPtr> Interpolator::load_interpolants(const Config &config, const ParSet &source)
{
    std::vector<InterpolantPtr> result;
    for (const TrackConfig &track : config.tracks())
    {
        if (track.keys.size() != 2U)
        {
            throw std::runtime_error("Track '" + track.parameter + "' requires exactly two keyframes");
        }
        if (track.keys[0].frame != 0 || track.keys[1].frame != config.num_frames() - 1)
        {
            throw std::runtime_error("Track '" + track.parameter + "' must span the full frame range");
        }
        const auto is_name{[&](const Parameter &param) { return param.name == track.parameter; }};
        if (std::find_if(source.params.begin(), source.params.end(), is_name) == source.params.end())
        {
            throw std::runtime_error(
                "Parameter set '" + source.name + "' has no parameter '" + track.parameter + "'");
        }
        result.emplace_back(
            create_interpolant(track.parameter, track.keys[0].value, track.keys[1].value, config.num_frames()));
    }
    return result;
}

Interpolator::Interpolator(const Config &config) :
    m_frame_name(config.output().entry),
    m_video(config.video()),
    m_source(load_par_set(config.source())),
    m_interpolants(load_interpolants(config, m_source))
{}

ParSet Interpolator::operator()()
{
    ++m_frame;
    ParSet par_set{m_source};
    for (const InterpolantPtr &lerper : m_interpolants)
    {
        const auto it{std::find_if(par_set.params.begin(), par_set.params.end(),
            [&](const Parameter &param) { return param.name == lerper->name(); })};
        it->value = lerper->step();
    }
    par_set.name = (boost::format(m_frame_name) % m_frame).str();
    par_set.params.push_back({"batch", "yes"});
    par_set.params.push_back({"savename", par_set.name + ".gif"});
    par_set.params.push_back({"overwrite", "yes"});
    par_set.params.push_back({"video", m_video});
    return par_set;
}

} // namespace ParFile
