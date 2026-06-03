// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/ParameterCatalog.h>

#include <nlohmann/json.hpp>

#include <algorithm>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

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

std::optional<std::string> load_optional_string(const Object &json, std::string_view field)
{
    const std::string key{field};
    if (!json.contains(key))
    {
        return std::nullopt;
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

int load_required_int(const Object &json, std::string_view parameter, std::string_view field)
{
    const std::string key{field};
    if (!json.contains(key) || !json.at(key).is_number_integer())
    {
        throw std::runtime_error("Invalid parameter metadata '" + std::string{parameter} + "', missing integer '" +
            std::string{field} + "'");
    }
    return json.at(key).get<int>();
}

ParameterMetadata load_metadata(std::string_view name, const Object &json)
{
    if (!json.is_object())
    {
        throw std::runtime_error("Invalid parameter metadata '" + std::string{name} + "', value is not an object");
    }

    ParameterMetadata result;
    result.name = std::string{name};
    result.type = parse_parameter_type(load_required_string(json, name, "type"));
    if (const std::optional<std::string> format{load_optional_string(json, "format")})
    {
        result.format = parse_parameter_format(*format);
    }
    if (const std::optional<std::string> curve{load_optional_string(json, "default-curve")})
    {
        result.default_curve = parse_curve(*curve);
    }
    if (const std::optional<std::string> extrapolate{load_optional_string(json, "extrapolate")})
    {
        result.extrapolate = parse_extrapolate_mode(*extrapolate);
    }
    result.min = load_optional_number(json, "min");
    result.max = load_optional_number(json, "max");
    return result;
}

std::vector<int> load_slots(std::string_view name, const Object &json)
{
    const std::string key{"slots"};
    if (!json.contains(key) || !json.at(key).is_array())
    {
        throw std::runtime_error("Invalid params group metadata '" + std::string{name} + "', missing array 'slots'");
    }

    std::vector<int> result;
    for (const Object &slot : json.at(key))
    {
        if (!slot.is_number_integer())
        {
            throw std::runtime_error(
                "Invalid params group metadata '" + std::string{name} + "', slot value is not an integer");
        }
        result.emplace_back(slot.get<int>());
    }
    return result;
}

ParamsSlotMetadata load_params_slot(const Object &json)
{
    if (!json.is_object())
    {
        throw std::runtime_error("Invalid params slot metadata, value is not an object");
    }

    ParamsSlotMetadata result;
    result.index = load_required_int(json, "params slot", "index");
    result.name = load_required_string(json, "params slot", "name");
    const std::string metadata_name{std::string{"params["} + std::to_string(result.index) + "]"};
    result.metadata = load_metadata(metadata_name, json);
    return result;
}

ParamsGroupMetadata load_params_group(std::string_view name, const Object &json)
{
    if (!json.is_object())
    {
        throw std::runtime_error("Invalid params group metadata '" + std::string{name} + "', value is not an object");
    }

    ParamsGroupMetadata result;
    result.name = std::string{name};
    const std::string metadata_name{std::string{"params."} + std::string{name}};
    result.metadata = load_metadata(metadata_name, json);
    result.slots = load_slots(name, json);
    return result;
}

FractalParamsMetadata load_fractal_params(const Object &json, std::string_view fractal_type)
{
    if (!json.is_object())
    {
        throw std::runtime_error(
            "Invalid fractal type metadata '" + std::string{fractal_type} + "', params is not an object");
    }

    FractalParamsMetadata result;
    if (json.contains("slots"))
    {
        if (!json.at("slots").is_array())
        {
            throw std::runtime_error(
                "Invalid fractal type metadata '" + std::string{fractal_type} + "', params.slots is not an array");
        }
        for (const Object &slot : json.at("slots"))
        {
            result.slots.emplace_back(load_params_slot(slot));
        }
    }
    if (json.contains("groups"))
    {
        if (!json.at("groups").is_object())
        {
            throw std::runtime_error(
                "Invalid fractal type metadata '" + std::string{fractal_type} + "', params.groups is not an object");
        }
        for (const auto &[name, group] : json.at("groups").items())
        {
            result.groups.emplace_back(load_params_group(name, group));
        }
    }
    return result;
}

FractalTypeMetadata load_fractal_type(std::string_view name, const Object &json)
{
    if (!json.is_object())
    {
        throw std::runtime_error("Invalid fractal type metadata '" + std::string{name} + "', value is not an object");
    }

    FractalTypeMetadata result;
    result.name = std::string{name};
    if (json.contains("params"))
    {
        result.params = load_fractal_params(json.at("params"), name);
    }
    return result;
}

void load_fractal_types(const Object &json, ParameterCatalog &result)
{
    if (!json.contains("fractal-types"))
    {
        return;
    }
    if (!json.at("fractal-types").is_object())
    {
        throw std::runtime_error("Invalid parameter catalog, 'fractal-types' is not an object");
    }
    for (const auto &[name, fractal_type] : json.at("fractal-types").items())
    {
        result.fractal_types.emplace_back(load_fractal_type(name, fractal_type));
    }
}

} // namespace

ParameterCatalog read_parameter_catalog(std::string_view json_text)
{
    const Object json{parse_json(json_text)};
    const Object &parameters{load_parameters(json)};
    ParameterCatalog result;
    for (const auto &[name, metadata] : parameters.items())
    {
        result.parameters.emplace_back(load_metadata(name, metadata));
    }
    load_fractal_types(json, result);
    return result;
}

const ParameterMetadata &ParameterCatalog::metadata(std::string_view name) const
{
    const std::string key{name};
    const auto matches{[&](const ParameterMetadata &metadata) { return metadata.name == key; }};
    const auto it{std::find_if(parameters.begin(), parameters.end(), matches)};
    if (it == parameters.end())
    {
        throw std::runtime_error("Unknown animated parameter '" + std::string{name} + "'");
    }
    return *it;
}

const ParamsSlotMetadata &ParameterCatalog::params_slot(std::string_view fractal_type, int slot) const
{
    const std::string fractal_key{fractal_type};
    const auto is_fractal_type{[&](const FractalTypeMetadata &metadata) { return metadata.name == fractal_key; }};
    const auto fractal_it{std::find_if(fractal_types.begin(), fractal_types.end(), is_fractal_type)};
    if (fractal_it == fractal_types.end())
    {
        throw std::runtime_error("Unknown fractal type metadata '" + std::string{fractal_type} + "'");
    }

    const auto is_slot{[&](const ParamsSlotMetadata &metadata) { return metadata.index == slot; }};
    const auto slot_it{std::find_if(fractal_it->params.slots.begin(), fractal_it->params.slots.end(), is_slot)};
    if (slot_it == fractal_it->params.slots.end())
    {
        throw std::runtime_error(
            "Unknown params slot '" + std::to_string(slot) + "' for fractal type '" + std::string{fractal_type} + "'");
    }
    return *slot_it;
}

const ParamsGroupMetadata &ParameterCatalog::params_group(std::string_view fractal_type, std::string_view group) const
{
    const std::string fractal_key{fractal_type};
    const auto is_fractal_type{[&](const FractalTypeMetadata &metadata) { return metadata.name == fractal_key; }};
    const auto fractal_it{std::find_if(fractal_types.begin(), fractal_types.end(), is_fractal_type)};
    if (fractal_it == fractal_types.end())
    {
        throw std::runtime_error("Unknown fractal type metadata '" + std::string{fractal_type} + "'");
    }

    const std::string group_key{group};
    const auto is_group{[&](const ParamsGroupMetadata &metadata) { return metadata.name == group_key; }};
    const auto group_it{std::find_if(fractal_it->params.groups.begin(), fractal_it->params.groups.end(), is_group)};
    if (group_it == fractal_it->params.groups.end())
    {
        throw std::runtime_error(
            "Unknown params group '" + std::string{group} + "' for fractal type '" + std::string{fractal_type} + "'");
    }
    return *group_it;
}

} // namespace ParFile
