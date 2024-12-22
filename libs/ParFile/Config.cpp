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
        std::string msg{"Invalid config, missing integer '" + std::string{name} + "'"};
        throw std::runtime_error(msg);
    }
    return static_cast<int>(json.at(name).as_int64());
}

Config::Config(const boost::json::object &json) :
    m_from(load_named_file_par_set(json, "from")),
    m_to(load_named_file_par_set(json, "to")),
    m_output(load_string(json, "output")),
    m_script(load_string(json, "script")),
    m_frame(load_string(json, "frame")),
    m_num_frames(load_int(json, "num_frames"))
{
}

} // namespace ParFile
