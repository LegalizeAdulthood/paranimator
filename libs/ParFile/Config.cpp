// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/Config.h>

#include <boost/json.hpp>

#include <string>
#include <string_view>

namespace ParFile
{

using Object = boost::json::object;

static Object load_object(const Object &json, std::string_view name)
{
    if (!json.try_at(name) || !json.at(name).is_object())
    {
        throw std::runtime_error("Invalid config, missing object '" + std::string{name} + "'");
    }
    return json.at(name).as_object();
}

static std::string_view load_string(const Object &json, std::string_view name, std::string_view field)
{
    if (!json.try_at(field) || !json.at(field).is_string())
    {
        std::string msg{"Invalid config, missing string '" + std::string{field} + "'"};
        if (!name.empty())
        {
            msg += " in '" + std::string{name} + "'";
        }
        throw std::runtime_error(msg);
    }
    return json.at(field).as_string();
}

static std::string_view load_string(const Object &json, std::string_view field)
{
    return load_string(json, {}, field);
}

static NamedFileParSet load_named_file_par_set(const Object &json, std::string_view name)
{
    const Object &named_params{load_object(json, name)};

    NamedFileParSet result;
    result.file = load_string(named_params, name, "file");
    result.name = load_string(named_params, name, "name");
    return result;
}

static int load_int(const Object &json, std::string_view name)
{
    if (!json.try_at(name) || !json.at(name).is_int64())
    {
        throw std::runtime_error("Invalid config, missing integer '" + std::string{name} + "'");
    }
    return static_cast<int>(json.at(name).as_int64());
}

static std::vector<std::string> load_string_vector(const Object &json, std::string_view name)
{
    if (!json.try_at(name) || !json.at(name).is_array())
    {
        throw std::runtime_error("Invalid config, missing string array '" + std::string{name} + "'");
    }
    std::vector<std::string> result;
    for (const boost::json::value &val : json.at(name).as_array())
    {
        if (!val.is_string())
        {
            throw std::runtime_error(
                "Invalid config, string array '" + std::string{name} + "' contains non-string value");
        }
        if (val.as_string().empty())
        {
            throw std::runtime_error(
                "Invalid config, string array '" + std::string{name} + "' contains empty string value");
        }
        result.emplace_back(val.as_string());
    }
    if (result.empty())
    {
        throw std::runtime_error("Invalid config, string array '" + std::string{name} + "' is empty");
    }
    return result;
}

Config::Config(const boost::json::object &json) :
    m_from(load_named_file_par_set(json, "from")),
    m_to(load_named_file_par_set(json, "to")),
    m_interpolate(load_string_vector(json, "interpolate")),
    m_output(load_string(json, "output")),
    m_script(load_string(json, "script")),
    m_frame(load_string(json, "frame")),
    m_video(load_string(json, "video")),
    m_num_frames(load_int(json, "num_frames"))
{
}

} // namespace ParFile
