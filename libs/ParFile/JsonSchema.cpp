// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/JsonSchema.h>

#include <nlohmann/json-schema.hpp>
#include <nlohmann/json.hpp>

#include <string_view>

namespace ParFile
{

namespace
{

nlohmann::json parse_json(std::string_view json_text)
{
    return nlohmann::json::parse(json_text.begin(), json_text.end());
}

} // namespace

bool validate_json_schema(
    std::string_view schema_json,
    std::string_view instance_json)
{
    try
    {
        nlohmann::json_schema::json_validator validator;
        validator.set_root_schema(parse_json(schema_json));
        validator.validate(parse_json(instance_json));
        return true;
    }
    catch (...)
    {
        return false;
    }
}

} // namespace ParFile
