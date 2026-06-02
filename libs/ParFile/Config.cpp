// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/Config.h>

#include <nlohmann/json.hpp>

#include <stdexcept>
#include <string>
#include <string_view>

namespace ParFile
{

using Object = nlohmann::json;

static Object parse_json(std::string_view json_text)
{
    try
    {
        Object json{Object::parse(json_text.begin(), json_text.end())};
        if (!json.is_object())
        {
            throw std::runtime_error("Invalid config, root is not an object");
        }
        return json;
    }
    catch (const nlohmann::json::exception &bang)
    {
        throw std::runtime_error("Invalid config JSON: " + std::string{bang.what()});
    }
}

static const Object &load_object(const Object &json, std::string_view name)
{
    const std::string key{name};
    if (!json.contains(key) || !json.at(key).is_object())
    {
        throw std::runtime_error("Invalid config, missing object '" + std::string{name} + "'");
    }
    return json.at(key);
}

static std::string load_string(const Object &json, std::string_view name, std::string_view field)
{
    const std::string key{field};
    if (!json.contains(key) || !json.at(key).is_string())
    {
        std::string msg{"Invalid config, missing string '" + std::string{field} + "'"};
        if (!name.empty())
        {
            msg += " in '" + std::string{name} + "'";
        }
        throw std::runtime_error(msg);
    }
    return json.at(key).get<std::string>();
}

static std::string load_string(const Object &json, std::string_view field)
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

static OutputConfig load_output_config(const Object &json)
{
    const Object &output{load_object(json, "output")};

    OutputConfig result;
    result.directory = load_string(output, "output", "directory");
    result.par = load_string(output, "output", "par");
    result.entry = load_string(output, "output", "entry");
    result.script = load_string(output, "output", "script");
    return result;
}

static int load_int(const Object &json, std::string_view name)
{
    const std::string key{name};
    if (!json.contains(key) || !json.at(key).is_number_integer())
    {
        throw std::runtime_error("Invalid config, missing integer '" + std::string{name} + "'");
    }
    return json.at(key).get<int>();
}

static std::vector<std::string> load_string_vector(const Object &json, std::string_view name)
{
    const std::string key{name};
    if (!json.contains(key) || !json.at(key).is_array())
    {
        throw std::runtime_error("Invalid config, missing string array '" + std::string{name} + "'");
    }
    std::vector<std::string> result;
    for (const Object &val : json.at(key))
    {
        if (!val.is_string())
        {
            throw std::runtime_error(
                "Invalid config, string array '" + std::string{name} + "' contains non-string value");
        }
        const std::string value{val.get<std::string>()};
        if (value.empty())
        {
            throw std::runtime_error(
                "Invalid config, string array '" + std::string{name} + "' contains empty string value");
        }
        result.emplace_back(value);
    }
    if (result.empty())
    {
        throw std::runtime_error("Invalid config, string array '" + std::string{name} + "' is empty");
    }
    return result;
}

Config::Config(std::string_view json_text)
{
    const Object json{parse_json(json_text)};
    m_from = load_named_file_par_set(json, "from");
    m_to = load_named_file_par_set(json, "to");
    m_interpolate = load_string_vector(json, "interpolate");
    m_output = load_output_config(json);
    m_video = load_string(json, "video");
    m_num_frames = load_int(json, "num_frames");

    if (json.contains("parallel"))
    {
        if (!json.at("parallel").is_number_integer())
        {
            throw std::runtime_error("Invalid config, 'parallel' is not a number");
        }
        m_parallel = json.at("parallel").get<int>();
    }
}

} // namespace ParFile
