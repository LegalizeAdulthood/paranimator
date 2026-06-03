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

struct ParamsSlotMetadata
{
    int index{};
    std::string name;
    ParameterMetadata metadata;
};

struct ParamsGroupMetadata
{
    std::string name;
    ParameterMetadata metadata;
    std::vector<int> slots;
};

struct FractalParamsMetadata
{
    std::vector<ParamsSlotMetadata> slots;
    std::vector<ParamsGroupMetadata> groups;
};

struct FractalTypeMetadata
{
    std::string name;
    FractalParamsMetadata params;
};

struct FormulaParamsKnobMetadata
{
    std::string name;
    ParameterMetadata metadata;
    std::vector<int> slots;
};

struct FormulaParamsMetadata
{
    std::vector<FormulaParamsKnobMetadata> knobs;
};

struct FormulaEntryMetadata
{
    std::string name;
    FormulaParamsMetadata params;
};

struct ParameterCatalog
{
    const ParameterMetadata &metadata(std::string_view name) const;
    const ParamsSlotMetadata &params_slot(std::string_view fractal_type, int slot) const;
    const ParamsGroupMetadata &params_group(std::string_view fractal_type, std::string_view group) const;
    const FormulaParamsKnobMetadata &formula_params_knob(std::string_view formula_name, std::string_view knob) const;

    std::vector<ParameterMetadata> parameters;
    std::vector<FractalTypeMetadata> fractal_types;
    std::vector<FormulaEntryMetadata> formula_entries;
};

ParameterCatalog read_parameter_catalog(std::string_view json_text);

} // namespace ParFile
