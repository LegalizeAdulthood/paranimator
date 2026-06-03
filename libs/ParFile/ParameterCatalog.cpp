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

void load_optional_metadata_fields(ParameterMetadata &metadata, const Object &json)
{
    if (const std::optional<std::string> format{load_optional_string(json, "format")})
    {
        metadata.format = parse_parameter_format(*format);
    }
    if (const std::optional<std::string> curve{load_optional_string(json, "default-curve")})
    {
        metadata.default_curve = parse_curve(*curve);
    }
    if (const std::optional<std::string> extrapolate{load_optional_string(json, "extrapolate")})
    {
        metadata.extrapolate = parse_extrapolate_mode(*extrapolate);
    }
    metadata.min = load_optional_number(json, "min");
    metadata.max = load_optional_number(json, "max");
}

ParameterType load_formula_knob_type(std::string_view name, const Object &json)
{
    const std::string type{load_required_string(json, name, "type")};
    if (type == "integer")
    {
        return ParameterType::INTEGER;
    }
    if (type == "real")
    {
        return ParameterType::DOUBLE;
    }
    if (type == "complex")
    {
        return ParameterType::COMPLEX;
    }
    throw std::runtime_error("Unknown formula params knob type '" + type + "' for '" + std::string{name} + "'");
}

ParameterFormat default_formula_knob_format(ParameterType type)
{
    if (type == ParameterType::COMPLEX)
    {
        return ParameterFormat::SLASH_PAIR;
    }
    return ParameterFormat::RAW;
}

int formula_variable_index(std::string_view variable)
{
    if (variable.size() < 2U || variable[0] != 'p' || variable[1] < '1' || variable[1] > '4')
    {
        return -1;
    }
    return variable[1] - '1';
}

std::vector<int> load_formula_variable_slots(std::string_view name, ParameterType type, const std::string &variable)
{
    const int variable_index{formula_variable_index(variable)};
    const int base_slot{variable_index * 2};
    if (type == ParameterType::COMPLEX)
    {
        if (variable_index < 0 || variable.size() != 2U)
        {
            throw std::runtime_error(
                "Invalid complex formula params variable '" + variable + "' for '" + std::string{name} + "'");
        }
        return {base_slot, base_slot + 1};
    }

    if (variable_index < 0 || variable.size() <= 3U || variable.substr(2, 1) != ".")
    {
        throw std::runtime_error(
            "Invalid scalar formula params variable '" + variable + "' for '" + std::string{name} + "'");
    }
    const std::string component{variable.substr(3)};
    if (component == "real")
    {
        return {base_slot};
    }
    if (component == "imag")
    {
        return {base_slot + 1};
    }
    throw std::runtime_error(
        "Invalid scalar formula params variable '" + variable + "' for '" + std::string{name} + "'");
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

FormulaParamsKnobMetadata load_formula_params_knob(
    std::string_view formula_name, std::string_view knob_name, const Object &json)
{
    if (!json.is_object())
    {
        throw std::runtime_error(
            "Invalid formula params knob metadata '" + std::string{knob_name} + "', value is not an object");
    }

    FormulaParamsKnobMetadata result;
    result.name = std::string{knob_name};
    result.metadata.name = std::string{formula_name} + "." + std::string{knob_name};
    result.metadata.type = load_formula_knob_type(knob_name, json);
    result.metadata.format = default_formula_knob_format(result.metadata.type);
    load_optional_metadata_fields(result.metadata, json);
    const std::string variable{load_required_string(json, knob_name, "variable")};
    result.slots = load_formula_variable_slots(knob_name, result.metadata.type, variable);
    return result;
}

FormulaParamsMetadata load_formula_params(std::string_view formula_name, const Object &json)
{
    if (!json.is_object())
    {
        throw std::runtime_error(
            "Invalid formula entry metadata '" + std::string{formula_name} + "', params is not an object");
    }

    FormulaParamsMetadata result;
    if (json.contains("knobs"))
    {
        if (!json.at("knobs").is_object())
        {
            throw std::runtime_error(
                "Invalid formula entry metadata '" + std::string{formula_name} + "', params.knobs is not an object");
        }
        for (const auto &[name, knob] : json.at("knobs").items())
        {
            result.knobs.emplace_back(load_formula_params_knob(formula_name, name, knob));
        }
    }
    return result;
}

FormulaEntryMetadata load_formula_entry(std::string_view name, const Object &json)
{
    if (!json.is_object())
    {
        throw std::runtime_error("Invalid formula entry metadata '" + std::string{name} + "', value is not an object");
    }

    FormulaEntryMetadata result;
    result.name = std::string{name};
    if (json.contains("params"))
    {
        result.params = load_formula_params(name, json.at("params"));
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

void load_formula_entries(const Object &json, ParameterCatalog &result)
{
    if (!json.contains("formula-entries"))
    {
        return;
    }
    if (!json.at("formula-entries").is_object())
    {
        throw std::runtime_error("Invalid parameter catalog, 'formula-entries' is not an object");
    }
    for (const auto &[name, formula_entry] : json.at("formula-entries").items())
    {
        result.formula_entries.emplace_back(load_formula_entry(name, formula_entry));
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
    load_formula_entries(json, result);
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

const FormulaParamsKnobMetadata &ParameterCatalog::formula_params_knob(
    std::string_view formula_name, std::string_view knob) const
{
    const std::string formula_key{formula_name};
    const auto is_formula{[&](const FormulaEntryMetadata &metadata) { return metadata.name == formula_key; }};
    const auto formula_it{std::find_if(formula_entries.begin(), formula_entries.end(), is_formula)};
    if (formula_it == formula_entries.end())
    {
        throw std::runtime_error("Unknown formula entry metadata '" + std::string{formula_name} + "'");
    }

    const std::string knob_key{knob};
    const auto is_knob{[&](const FormulaParamsKnobMetadata &metadata) { return metadata.name == knob_key; }};
    const auto knob_it{std::find_if(formula_it->params.knobs.begin(), formula_it->params.knobs.end(), is_knob)};
    if (knob_it == formula_it->params.knobs.end())
    {
        throw std::runtime_error(
            "Unknown formula params knob '" + std::string{knob} + "' for formula '" + std::string{formula_name} + "'");
    }
    return *knob_it;
}

} // namespace ParFile
