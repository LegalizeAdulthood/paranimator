// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/Interpolator.h>

#include <ParFile/Config.h>
#include <ParFile/Interpolant.h>
#include <ParFile/ParFile.h>

#include <boost/format.hpp>
#include <boost/algorithm/string/split.hpp>

#include <algorithm>
#include <filesystem>
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

std::vector<InterpolantPtr> Interpolator::load_interpolants(
    const Config &config, const ParSet &from, const ParSet &to, int num_steps)
{
    std::vector<InterpolantPtr> result;
    std::vector<std::string> values;
    for (const std::string &name : config.interpolate())
    {
        const auto is_name{[&](const Parameter &param) { return param.name == name; }};
        const auto from_param{std::find_if(from.params.begin(), from.params.end(), is_name)};
        if (from_param == from.params.end())
        {
            throw std::runtime_error("Parameter set '" + from.name + "' has no parameter '" + name + "'");
        }
        const auto to_param{std::find_if(to.params.begin(), to.params.end(), is_name)};
        if (to_param == to.params.end())
        {
            throw std::runtime_error("Parameter set '" + to.name + "' has no parameter '" + name + "'");
        }
        
        result.emplace_back(create_interpolant(name, from_param->value, to_param->value, num_steps));
    }

    return result;
}

Interpolator::Interpolator(const Config &config) :
    m_num_frames(config.num_frames()),
    m_frame_name(config.frame()),
    m_output(config.output()),
    m_script(config.script()),
    m_video(config.video()),
    m_from(load_par_set(config.from())),
    m_to(load_par_set(config.to())),
    m_interpolants(load_interpolants(config, m_from, m_to, m_num_frames))
{
    const auto it{std::find_if(
        m_from.params.begin(), m_from.params.end(), [](const Parameter &param) { return param.name == "center-mag"; })};
    if (it == m_from.params.end())
    {
        throw std::runtime_error("Missing center-mag parameter in set '" + m_from.name + "'");
    }
}

ParSet Interpolator::operator()()
{
    ++m_frame;
    ParSet par_set{m_from};
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
