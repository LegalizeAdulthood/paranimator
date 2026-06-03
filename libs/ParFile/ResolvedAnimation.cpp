// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/ResolvedAnimation.h>

#include <algorithm>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

namespace ParFile
{

namespace
{

bool starts_with(std::string_view text, std::string_view prefix)
{
    return text.size() >= prefix.size() && text.substr(0, prefix.size()) == prefix;
}

bool ends_with(std::string_view text, std::string_view suffix)
{
    return text.size() >= suffix.size() && text.substr(text.size() - suffix.size()) == suffix;
}

const Parameter *find_source_parameter(const ParSet &source, std::string_view name)
{
    const std::string key{name};
    const auto is_name{[&](const Parameter &param) { return param.name == key; }};
    const auto it{std::find_if(source.params.begin(), source.params.end(), is_name)};
    if (it == source.params.end())
    {
        return nullptr;
    }
    return &*it;
}

const Parameter &source_parameter(const ParSet &source, std::string_view name)
{
    const Parameter *param{find_source_parameter(source, name)};
    if (param == nullptr)
    {
        throw std::runtime_error("Parameter set '" + source.name + "' has no parameter '" + std::string{name} + "'");
    }
    return *param;
}

std::string source_type(const ParSet &source)
{
    return source_parameter(source, "type").value;
}

bool source_is_formula(const ParSet &source)
{
    const Parameter *type{find_source_parameter(source, "type")};
    return type != nullptr && type->value == "formula";
}

std::string source_formula_name(const ParSet &source)
{
    return source_parameter(source, "formulaname").value;
}

int formula_function_slot(std::string_view name)
{
    if (name.size() == 3U && name[0] == 'f' && name[1] == 'n' && name[2] >= '1' && name[2] <= '4')
    {
        return name[2] - '1';
    }
    return -1;
}

std::string default_function_value(int slot)
{
    std::string result{"ident"};
    for (int i = 0; i < slot; ++i)
    {
        result += "/ident";
    }
    return result;
}

int parse_params_slot(std::string_view parameter)
{
    if (!starts_with(parameter, "params[") || parameter.back() != ']')
    {
        throw std::runtime_error("Invalid params slot track '" + std::string{parameter} + "'");
    }

    const std::string slot_text{parameter.substr(7, parameter.size() - 8)};
    try
    {
        std::size_t length{};
        const int slot{std::stoi(slot_text, &length)};
        if (length != slot_text.size())
        {
            throw std::runtime_error("Invalid params slot track '" + std::string{parameter} + "'");
        }
        return slot;
    }
    catch (const std::invalid_argument &)
    {
        throw std::runtime_error("Invalid params slot track '" + std::string{parameter} + "'");
    }
    catch (const std::out_of_range &)
    {
        throw std::runtime_error("Params slot out of range in track '" + std::string{parameter} + "'");
    }
}

std::optional<std::string> formula_member_name(std::string_view track, std::string_view formula_name)
{
    const std::string dotted_prefix{std::string{formula_name} + "."};
    if (starts_with(track, dotted_prefix))
    {
        std::string knob{track.substr(dotted_prefix.size())};
        if (!knob.empty())
        {
            return knob;
        }
    }

    const std::string bracket_prefix{std::string{formula_name} + "[\""};
    constexpr std::string_view BRACKET_SUFFIX{"\"]"};
    if (starts_with(track, bracket_prefix) && ends_with(track, BRACKET_SUFFIX))
    {
        const std::size_t start{bracket_prefix.size()};
        const std::size_t length{track.size() - start - BRACKET_SUFFIX.size()};
        if (length > 0U)
        {
            return std::string{track.substr(start, length)};
        }
    }

    return {};
}

ResolvedTrack resolve_params_slot(
    const TrackConfig &track, const ParameterCatalog &catalog, const ParSet &source, std::string_view fractal_type)
{
    const int slot{parse_params_slot(track.parameter)};
    const ParamsSlotMetadata &slot_metadata{catalog.params_slot(fractal_type, slot)};
    const Parameter &params{source_parameter(source, "params")};
    return {
        track.parameter, slot_metadata.metadata, params.value, track.keys, "params", {slot}, track.mode, track.pwm,
        track.path};
}

ResolvedTrack resolve_params_group(
    const TrackConfig &track, const ParameterCatalog &catalog, const ParSet &source, std::string_view fractal_type)
{
    const std::string_view group{std::string_view{track.parameter}.substr(7)};
    if (group.empty())
    {
        throw std::runtime_error("Invalid params group track '" + track.parameter + "'");
    }
    const ParamsGroupMetadata &group_metadata{catalog.params_group(fractal_type, group)};
    const Parameter &params{source_parameter(source, "params")};
    return {track.parameter, group_metadata.metadata, params.value, track.keys, "params", group_metadata.slots,
        track.mode, track.pwm, track.path};
}

ResolvedTrack resolve_formula_params_knob(
    const TrackConfig &track, const ParameterCatalog &catalog, const ParSet &source, std::string_view formula_name)
{
    const std::optional<std::string> knob{formula_member_name(track.parameter, formula_name)};
    if (!knob)
    {
        throw std::runtime_error("Invalid formula params track '" + track.parameter + "'");
    }
    const FormulaParamsKnobMetadata &knob_metadata{catalog.formula_params_knob(formula_name, *knob)};
    const Parameter &params{source_parameter(source, "params")};
    return {track.parameter, knob_metadata.metadata, params.value, track.keys, "params", knob_metadata.slots,
        track.mode, track.pwm, track.path};
}

ResolvedTrack resolve_formula_function(
    const TrackConfig &track, const ParameterCatalog &catalog, const ParSet &source, std::string_view formula_name)
{
    const std::optional<std::string> name{formula_member_name(track.parameter, formula_name)};
    if (!name)
    {
        throw std::runtime_error("Invalid formula function track '" + track.parameter + "'");
    }
    const FormulaFunctionMetadata &function_metadata{catalog.formula_function(formula_name, *name)};
    const Parameter *function{find_source_parameter(source, "function")};
    const std::string base_value{
        function == nullptr ? default_function_value(function_metadata.slot) : function->value};
    return {track.parameter, function_metadata.metadata, base_value, track.keys, "function", {function_metadata.slot},
        track.mode, track.pwm, track.path};
}

ResolvedTrack resolve_regular_track(const TrackConfig &track, const ParameterCatalog &catalog, const ParSet &source)
{
    const ParameterMetadata &metadata{catalog.metadata(track.parameter)};
    const Parameter &parameter{source_parameter(source, track.parameter)};
    return {track.parameter, metadata, parameter.value, track.keys, track.parameter, {}, track.mode, track.pwm,
        track.path};
}

ResolvedTrack resolve_track(const TrackConfig &track, const ParameterCatalog &catalog, const ParSet &source)
{
    if (source_is_formula(source))
    {
        const std::string formula_name{source_formula_name(source)};
        if (const std::optional<std::string> member_name{formula_member_name(track.parameter, formula_name)})
        {
            if (formula_function_slot(*member_name) >= 0)
            {
                return resolve_formula_function(track, catalog, source, formula_name);
            }
            return resolve_formula_params_knob(track, catalog, source, formula_name);
        }
    }
    if (starts_with(track.parameter, "params["))
    {
        return resolve_params_slot(track, catalog, source, source_type(source));
    }
    if (starts_with(track.parameter, "params."))
    {
        return resolve_params_group(track, catalog, source, source_type(source));
    }
    return resolve_regular_track(track, catalog, source);
}

void validate_slotted_track_overlaps(const std::vector<ResolvedTrack> &tracks)
{
    for (std::size_t i = 0; i < tracks.size(); ++i)
    {
        for (std::size_t j = i + 1; j < tracks.size(); ++j)
        {
            if (tracks[i].output_parameter != tracks[j].output_parameter || tracks[i].slots.empty() ||
                tracks[j].slots.empty())
            {
                continue;
            }
            for (const int left : tracks[i].slots)
            {
                if (std::find(tracks[j].slots.begin(), tracks[j].slots.end(), left) != tracks[j].slots.end())
                {
                    throw std::runtime_error("Tracks '" + tracks[i].parameter + "' and '" + tracks[j].parameter +
                        "' both write '" + tracks[i].output_parameter + "' slot " + std::to_string(left));
                }
            }
        }
    }
}

} // namespace

ResolvedAnimation resolve_animation(const Config &config, const ParameterCatalog &catalog, const ParSet &source)
{
    ResolvedAnimation result;
    result.frame_name = config.output.entry;
    result.video = config.video;
    result.num_frames = config.num_frames;
    result.source = source;

    for (const TrackConfig &track : config.tracks)
    {
        if (track.kind != TrackKind::COLOR_MAP)
        {
            result.tracks.push_back(resolve_track(track, catalog, source));
        }
    }
    validate_slotted_track_overlaps(result.tracks);

    return result;
}

} // namespace ParFile
