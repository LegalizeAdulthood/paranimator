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

Interpolator::Interpolant Interpolator::load_interpolant(const Config &config, const NamedFileParSet &par_entry)
{
    Interpolant result;
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
        result.par_set = *it;
    }
    {
        const auto it{std::find_if(result.par_set.params.begin(), result.par_set.params.end(),
            [](const Parameter &param) { return param.name == "center-mag"; })};
        if (it == result.par_set.params.end())
        {
            throw std::runtime_error(
                "Missing center-mag parameter in set '" + par_entry.name + "' in file '" + par_entry.file + "'");
        }
        std::vector<std::string> values;
        boost::algorithm::split(values, it->value, [](char c) { return c == '/'; });
        if (values.size() < 3U)
        {
            throw std::runtime_error("Insufficient values in center-mag parameter in set '" + par_entry.name +
                "' in file '" + par_entry.file + "'");
        }
        result.center_mag.center.real(std::stod(values[0]));
        result.center_mag.center.imag(std::stod(values[1]));
        result.center_mag.mag = std::stod(values[2]);
    }
    
    return result;
}

Interpolator::Interpolator(const Config &config) :
    m_num_frames(config.num_frames()),
    m_frame_name(config.frame()),
    m_output(config.output()),
    m_script(config.script()),
    m_from(load_interpolant(config, config.from())),
    m_to(load_interpolant(config, config.to()))
{
}

ParSet Interpolator::operator()()
{
    ++m_frame;
    ParSet par_set{m_frame == 1 ? m_from.par_set : m_to.par_set};
    par_set.name = (boost::format(m_frame_name) % m_frame).str();
    par_set.params.push_back({"batch", "yes"});
    par_set.params.push_back({"savename", par_set.name + ".gif"});
    return par_set;
}

} // namespace ParFile
