// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/Interpolator.h>

#include <ParFile/Config.h>
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
    ParFilePtr file{createParFile(in)};
    const auto it{
        std::find_if(file->cbegin(), file->cend(), [&](const ParSet &params) { return params.name == name; })};
    if (it == file->cend())
    {
        throw std::runtime_error("Couldn't find parameter set '" + name + "' in file '" + filename + "'");
    }
    return *it;
}

std::vector<Interpolator::Interpolant> Interpolator::load_interpolants(const Config &config)
{
    std::vector<Interpolant> result;
    std::vector<std::string> values;
    for (const std::string &interpolant : config.interpolate())
    {
        boost::algorithm::split(values, interpolant, [](char c) { return c == '/'; });
        if (values.size() == 1)
        {
            result.push_back({interpolant, 0});
        }
        else
        {
            for (int i = 0, num = std::stoi(values[1]); i < num; ++i)
            {
                result.push_back({interpolant, i});
            }
        }
    }

    return result;
}

Interpolator::Interpolator(const Config &config) :
    m_num_frames(config.num_frames()),
    m_frame_name(config.frame()),
    m_output(config.output()),
    m_script(config.script()),
    m_from(load_par_set(config.from())),
    m_to(load_par_set(config.to())),
    m_interpolants(load_interpolants(config))
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
    ParSet par_set{m_frame == m_num_frames ? m_to : m_from};
    if (m_frame != 1 && m_frame != m_num_frames)
    {
        const auto is_slash{[](char c) { return c == '/'; }};
        const auto is_center_mag{[](const Parameter &param) { return param.name == "center-mag"; }};
        auto from_it{std::find_if(m_from.params.begin(), m_from.params.end(), is_center_mag)};
        std::vector<std::string> from_value_text;
        boost::algorithm::split(from_value_text, from_it->value, is_slash);
        std::vector<double> from_values;
        from_values.resize(from_value_text.size());
        const auto to_double{[](const std::string &text) { return std::stod(text); }};
        std::transform(from_value_text.begin(), from_value_text.end(), from_values.begin(), to_double);
        std::vector<std::string> to_value_text;
        auto to_it{std::find_if(m_to.params.begin(), m_to.params.end(), is_center_mag)};
        boost::algorithm::split(to_value_text, to_it->value, is_slash);
        std::vector<double> to_values;
        to_values.resize(to_value_text.size());
        std::transform(to_value_text.begin(), to_value_text.end(), to_values.begin(), to_double);
        std::vector<double> lerped;
        lerped.reserve(from_values.size());
        const double fraction{(m_frame - 1) / static_cast<double>(m_num_frames - 1)};
        std::string result;
        for (size_t i = 0; i < from_values.size(); ++i)
        {
            const double value{from_values[i] + fraction * (to_values[i] - from_values[i])};
            if (i > 0)
            {
                result += '/';
            }
            result += (boost::format("%g") % value).str();
        }
        const auto it{std::find_if(par_set.params.begin(), par_set.params.end(), is_center_mag)};
        it->value = result;
    }
    par_set.name = (boost::format(m_frame_name) % m_frame).str();
    par_set.params.push_back({"batch", "yes"});
    par_set.params.push_back({"savename", par_set.name + ".gif"});
    return par_set;
}

} // namespace ParFile
