// SPDX-License-Identifier: GPL-3.0-only
//
#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace ParFile
{

struct ParameterMetadata
{
    std::string name;
    std::string type;
    std::string format;
    std::string default_curve;
    std::string extrapolate;
    std::optional<double> min;
    std::optional<double> max;
};

struct ParameterCatalog
{
    const ParameterMetadata &metadata(std::string_view name) const;

    std::vector<ParameterMetadata> parameters;
};

ParameterCatalog read_parameter_catalog(std::string_view json_text);

} // namespace ParFile
