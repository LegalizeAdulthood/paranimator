// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/Interpolator.h>

#include <ParFile/Config.h>
#include <ParFile/Interpolant.h>
#include <ParFile/ParFile.h>
#include <ParFile/ParameterCatalog.h>
#include <ParFile/ResolvedAnimation.h>

#include <algorithm>
#include <boost/algorithm/string/split.hpp>
#include <boost/format.hpp>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>
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

static std::string read_text(const std::filesystem::path &path)
{
    std::ifstream in{path};
    if (!in)
    {
        throw std::runtime_error("Unable to read file '" + path.string() + "'");
    }
    return {std::istreambuf_iterator<char>{in}, std::istreambuf_iterator<char>{}};
}

static ParameterCatalog load_parameter_catalog(const Config &config)
{
    if (config.parameter_catalogs.size() != 1U)
    {
        throw std::runtime_error("Expected exactly one parameter catalog");
    }
    return read_parameter_catalog(read_text(config.parameter_catalogs[0]));
}

static ResolvedAnimation load_animation(const Config &config)
{
    const ParSet source{load_par_set(config.source)};
    const ParameterCatalog catalog{load_parameter_catalog(config)};
    return resolve_animation(config, catalog, source);
}

static std::vector<std::string> split_slash_values(std::string_view value)
{
    std::vector<std::string> result;
    boost::algorithm::split(result, value, [](char c) { return c == '/'; });
    return result;
}

static std::string join_slash_values(const std::vector<std::string> &values)
{
    std::string result;
    for (const std::string &value : values)
    {
        if (!result.empty())
        {
            result += '/';
        }
        result += value;
    }
    return result;
}

static void merge_slotted_value(Parameter &param, const std::string &value, const std::vector<int> &slots)
{
    std::vector<std::string> current{split_slash_values(param.value)};
    const std::vector<std::string> update{split_slash_values(value)};
    if (current.size() < update.size())
    {
        const std::size_t old_size{current.size()};
        current.resize(update.size());
        std::copy(std::next(update.begin(), static_cast<std::ptrdiff_t>(old_size)), update.end(),
            std::next(current.begin(), static_cast<std::ptrdiff_t>(old_size)));
    }
    for (const int slot : slots)
    {
        if (slot < 0 || static_cast<std::size_t>(slot) >= update.size())
        {
            throw std::runtime_error("Slotted value for '" + param.name + "' is missing slot " + std::to_string(slot));
        }
        current[static_cast<std::size_t>(slot)] = update[static_cast<std::size_t>(slot)];
    }
    param.value = join_slash_values(current);
}

std::vector<InterpolantPtr> Interpolator::load_interpolants(const ResolvedAnimation &animation)
{
    std::vector<InterpolantPtr> result;
    for (const ResolvedTrack &track : animation.tracks)
    {
        result.emplace_back(create_interpolant(track, animation.num_frames));
    }
    return result;
}

Interpolator::Interpolator(const Config &config) :
    Interpolator(load_animation(config))
{
}

Interpolator::Interpolator(const ResolvedAnimation &animation) :
    m_frame_name(animation.frame_name),
    m_video(animation.video),
    m_source(animation.source),
    m_interpolants(load_interpolants(animation))
{
}

ParSet Interpolator::operator()()
{
    ++m_frame;
    ParSet par_set{m_source};
    for (const InterpolantPtr &lerper : m_interpolants)
    {
        const auto it{std::find_if(par_set.params.begin(), par_set.params.end(),
            [&](const Parameter &param) { return param.name == lerper->name(); })};
        const std::string value{lerper->step()};
        if (lerper->has_value())
        {
            if (it == par_set.params.end())
            {
                par_set.params.push_back({lerper->name(), value});
            }
            else if (!lerper->output_slots().empty())
            {
                merge_slotted_value(*it, value, lerper->output_slots());
            }
            else
            {
                it->value = value;
            }
        }
        else if (it != par_set.params.end())
        {
            par_set.params.erase(it);
        }
    }
    par_set.name = (boost::format(m_frame_name) % m_frame).str();
    par_set.params.push_back({"batch", "yes"});
    par_set.params.push_back({"savename", par_set.name + ".gif"});
    par_set.params.push_back({"overwrite", "yes"});
    par_set.params.push_back({"video", m_video});
    return par_set;
}

} // namespace ParFile
