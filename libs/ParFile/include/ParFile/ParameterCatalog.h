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

class ParameterCatalog
{
public:
    ParameterCatalog() = default;
    ParameterCatalog(const ParameterCatalog &rhs) = default;
    ParameterCatalog(ParameterCatalog &&rhs) = default;
    ParameterCatalog(std::string_view json_text);
    ParameterCatalog &operator=(const ParameterCatalog &rhs) = default;
    ParameterCatalog &operator=(ParameterCatalog &&rhs) = default;

    const ParameterMetadata &metadata(std::string_view name) const;

private:
    std::vector<ParameterMetadata> m_parameters;
};

} // namespace ParFile
