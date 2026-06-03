// SPDX-License-Identifier: GPL-3.0-only
//
#pragma once

#include <ParFile/AnimationEnums.h>

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace ParFile
{

struct ParameterMetadata
{
    std::string name;
    ParameterType type{};
    std::optional<ParameterFormat> format;
    std::optional<Curve> default_curve;
    std::optional<ExtrapolateMode> extrapolate;
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
