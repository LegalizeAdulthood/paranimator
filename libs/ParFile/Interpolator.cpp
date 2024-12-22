#include <ParFile/ParFile.h>

#include <boost/format.hpp>
#include <boost/json.hpp>
#include <tweeny/tweeny.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace ParFile
{

using Object = boost::json::object;
using Config = boost::json::object;

static ParSet load_named_par_set(const Object &par_entry)
{
    std::string filename{par_entry.at("file").as_string()};
    std::string name{par_entry.at("name").as_string()};
    std::ifstream in{filename};
    ParFilePtr file{createParFile(in)};
    auto it{std::find_if(file->cbegin(), file->cend(), [&](const ParSet &params) { return params.name == name; })};
    if (it == file->cend())
    {
        throw std::runtime_error("Couldn't find parameter set '" + name + "' in file '" + filename + "'");
    }
    return *it;
}

static ParSet load_config_params(const Object &config, std::string_view name)
{
    return load_named_par_set(config.at(name).as_object());
}

class Interpolator
{
public:
    Interpolator(const Config &config);

    int num_frames() const
    {
        return m_num_frames;
    }

    ParSet operator()();

private:
    int m_num_frames;
    std::string m_frame_name;
    std::string m_output;
    std::string m_script;
    ParSet m_begin;
    ParSet m_end;
    int m_frame{};
};

Interpolator::Interpolator(const Config &config) :
    m_num_frames(static_cast<int>(config.at("num_frames").as_int64())),
    m_frame_name(config.at("frame").as_string()),
    m_output(config.at("output").as_string()),
    m_script(config.at("script").as_string()),
    m_begin(load_config_params(config, "begin")),
    m_end(load_config_params(config, "end"))
{
}

ParSet Interpolator::operator()()
{
    ParSet par_set{m_begin};
    ++m_frame;
    par_set.name = boost::basic_format<char>{boost::format(m_frame_name) % m_frame}.str();
    par_set.params.push_back({"batch", "yes"});
    par_set.params.push_back({"savename", par_set.name + ".gif"});
    return par_set;
}

} // namespace ParFile
