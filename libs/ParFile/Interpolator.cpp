// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/Interpolator.h>

#include <ParFile/Config.h>
#include <ParFile/ParFile.h>

#include <boost/format.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace ParFile
{

static ParSet load_config_params(const NamedFileParSet &par_entry)
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

Interpolator::Interpolator(const Config &config) :
    m_num_frames(config.num_frames()),
    m_frame_name(config.frame()),
    m_output(config.output()),
    m_script(config.script()),
    m_from(load_config_params(config.from())),
    m_to(load_config_params(config.to()))
{
}

ParSet Interpolator::operator()()
{
    ParSet par_set{m_from};
    ++m_frame;
    par_set.name = (boost::format(m_frame_name) % m_frame).str();
    par_set.params.push_back({"batch", "yes"});
    par_set.params.push_back({"savename", par_set.name + ".gif"});
    return par_set;
}

} // namespace ParFile
