// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/JsonSchema.h>

#include <nlohmann/json-schema.hpp>
#include <nlohmann/json.hpp>

#include <fstream>
#include <stdexcept>
#include <string>

namespace ParFile
{

namespace
{

nlohmann::json read_json(const std::filesystem::path &path)
{
    std::ifstream in{path};
    if (!in)
    {
        throw std::runtime_error("Unable to read JSON file: " + path.string());
    }
    nlohmann::json json;
    in >> json;
    return json;
}

} // namespace

bool validate_json_schema(
    const std::filesystem::path &schema_path,
    const std::filesystem::path &instance_path)
{
    try
    {
        nlohmann::json_schema::json_validator validator;
        validator.set_root_schema(read_json(schema_path));
        validator.validate(read_json(instance_path));
        return true;
    }
    catch (...)
    {
        return false;
    }
}

} // namespace ParFile
