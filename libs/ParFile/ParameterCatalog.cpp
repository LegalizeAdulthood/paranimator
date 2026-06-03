// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/ParameterCatalog.h>

#include <nlohmann/json.hpp>

#include <algorithm>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

namespace ParFile
{

namespace
{

using Object = nlohmann::json;

Object parse_json(std::string_view json_text)
{
    try
    {
        Object json{Object::parse(json_text.begin(), json_text.end())};
        if (!json.is_object())
        {
            throw std::runtime_error("Invalid parameter catalog, root is not an object");
        }
        return json;
    }
    catch (const nlohmann::json::exception &bang)
    {
        throw std::runtime_error("Invalid parameter catalog JSON: " + std::string{bang.what()});
    }
}

const Object &load_parameters(const Object &json)
{
    const std::string key{"parameters"};
    if (!json.contains(key) || !json.at(key).is_object())
    {
        throw std::runtime_error("Invalid parameter catalog, missing object 'parameters'");
    }
    return json.at(key);
}

std::string load_required_string(const Object &json, std::string_view parameter, std::string_view field)
{
    const std::string key{field};
    if (!json.contains(key) || !json.at(key).is_string())
    {
        throw std::runtime_error(
            "Invalid parameter metadata '" + std::string{parameter} + "', missing string '" + std::string{field} + "'");
    }
    return json.at(key).get<std::string>();
}

std::string load_optional_string(const Object &json, std::string_view field)
{
    const std::string key{field};
    if (!json.contains(key))
    {
        return {};
    }
    if (!json.at(key).is_string())
    {
        throw std::runtime_error("Invalid parameter metadata, field '" + std::string{field} + "' is not a string");
    }
    return json.at(key).get<std::string>();
}

std::optional<double> load_optional_number(const Object &json, std::string_view field)
{
    const std::string key{field};
    if (!json.contains(key))
    {
        return {};
    }
    if (!json.at(key).is_number())
    {
        throw std::runtime_error("Invalid parameter metadata, field '" + std::string{field} + "' is not a number");
    }
    return json.at(key).get<double>();
}

ParameterMetadata load_metadata(std::string_view name, const Object &json)
{
    if (!json.is_object())
    {
        throw std::runtime_error("Invalid parameter metadata '" + std::string{name} + "', value is not an object");
    }

    ParameterMetadata result;
    result.name = std::string{name};
    result.type = load_required_string(json, name, "type");
    result.format = load_optional_string(json, "format");
    result.default_curve = load_optional_string(json, "default_curve");
    result.extrapolate = load_optional_string(json, "extrapolate");
    result.min = load_optional_number(json, "min");
    result.max = load_optional_number(json, "max");
    return result;
}

} // namespace

ParameterCatalog::ParameterCatalog(std::string_view json_text)
{
    const Object parameters{load_parameters(parse_json(json_text))};
    for (const auto &[name, metadata] : parameters.items())
    {
        m_parameters.emplace_back(load_metadata(name, metadata));
    }
}

const ParameterMetadata &ParameterCatalog::metadata(std::string_view name) const
{
    const std::string key{name};
    const auto matches{[&](const ParameterMetadata &metadata) { return metadata.name == key; }};
    const auto it{std::find_if(m_parameters.begin(), m_parameters.end(), matches)};
    if (it == m_parameters.end())
    {
        throw std::runtime_error("Unknown animated parameter '" + std::string{name} + "'");
    }
    return *it;
}

} // namespace ParFile
