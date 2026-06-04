// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/ResolvedAnimation.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

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

double parse_double(std::string_view text)
{
    try
    {
        std::size_t length{};
        const std::string value{text};
        const double result{std::stod(value, &length)};
        if (length != value.size() || !std::isfinite(result))
        {
            throw std::runtime_error("Invalid numeric value '" + value + "'");
        }
        return result;
    }
    catch (const std::invalid_argument &)
    {
        throw std::runtime_error("Invalid numeric value '" + std::string{text} + "'");
    }
    catch (const std::out_of_range &)
    {
        throw std::runtime_error("Numeric value out of range '" + std::string{text} + "'");
    }
}

std::vector<double> parse_slash_doubles(std::string_view text)
{
    std::vector<double> result;
    std::size_t start{};
    while (start <= text.size())
    {
        const std::size_t slash{text.find('/', start)};
        const std::size_t end{slash == std::string_view::npos ? text.size() : slash};
        result.push_back(parse_double(text.substr(start, end - start)));
        if (slash == std::string_view::npos)
        {
            break;
        }
        start = slash + 1U;
    }
    return result;
}

double point_distance(double x0, double y0, double x1, double y1)
{
    const double dx{x1 - x0};
    const double dy{y1 - y0};
    return std::sqrt(dx * dx + dy * dy);
}

double source_corners_aspect(std::string_view value)
{
    const std::vector<double> values{parse_slash_doubles(value)};
    double width{};
    double height{};
    if (values.size() == 4U)
    {
        width = std::abs(values[1] - values[0]);
        height = std::abs(values[3] - values[2]);
    }
    else if (values.size() == 6U)
    {
        width = point_distance(values[4], values[5], values[1], values[2]);
        height = point_distance(values[0], values[3], values[4], values[5]);
    }
    else
    {
        throw std::runtime_error("Source corners parameter must have 4 or 6 values");
    }
    if (width <= 0.0 || height <= 0.0)
    {
        throw std::runtime_error("Source corners parameter has zero width or height");
    }
    return width / height;
}

double source_video_aspect(std::string_view video)
{
    if (video == "F6")
    {
        return 4.0 / 3.0;
    }
    throw std::runtime_error(
        "Camera2D center-mag output requires a known source video shape; unsupported video mode '" +
        std::string{video} + "'");
}

double center_mag_x_mag_factor(std::string_view value)
{
    const std::vector<double> values{parse_slash_doubles(value)};
    if (values.size() < 3U || values.size() > 6U)
    {
        throw std::runtime_error("Source center-mag parameter must have 3 through 6 values");
    }
    if (values.size() < 4U || values[3] == 0.0)
    {
        return 1.0;
    }
    return values[3];
}

double source_center_mag_aspect(std::string_view value, std::string_view video)
{
    return source_video_aspect(video) / std::abs(center_mag_x_mag_factor(value));
}

double source_camera2d_aspect(
    const ParameterMetadata &output_metadata, std::string_view output_value, std::string_view video)
{
    if (output_metadata.type == ParameterType::CORNERS)
    {
        return source_corners_aspect(output_value);
    }
    if (output_metadata.type == ParameterType::CENTER_MAG)
    {
        return source_center_mag_aspect(output_value, video);
    }
    throw std::runtime_error(
        "Camera2D output parameter '" + output_metadata.name + "' must have type corners or center-mag");
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
    return {track.parameter, slot_metadata.metadata, params.value, track.keys, "params", {slot}, track.mode, track.pwm,
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
    return {
        track.parameter, metadata, parameter.value, track.keys, track.parameter, {}, track.mode, track.pwm, track.path};
}

ParameterMetadata camera2d_value_metadata(
    std::string_view camera_name, std::string_view member_name, ParameterType type, bool normalize)
{
    ParameterMetadata result;
    result.name = std::string{camera_name} + "." + std::string{member_name};
    result.type = type;
    result.format = type == ParameterType::DOUBLE ? ParameterFormat::RAW : ParameterFormat::SLASH;
    result.default_curve = Curve::LINEAR;
    result.extrapolate = ExtrapolateMode::CLAMP;
    result.normalize = normalize;
    return result;
}

ResolvedCamera2DValueTrack resolve_camera2d_value_track(
    const Camera2DValueTrackConfig &track, std::string_view camera_name, std::string_view member_name, bool normalize)
{
    return {camera2d_value_metadata(camera_name, member_name, track.type, normalize), track.keys, track.path};
}

std::optional<ResolvedCamera2DValueTrack> resolve_optional_camera2d_value_track(
    const std::optional<Camera2DValueTrackConfig> &track, std::string_view camera_name, std::string_view member_name,
    bool normalize)
{
    if (!track)
    {
        return {};
    }
    return resolve_camera2d_value_track(*track, camera_name, member_name, normalize);
}

ResolvedTrack resolve_camera2d_track(
    const TrackConfig &track, const ParameterCatalog &catalog, const ParSet &source, std::string_view video)
{
    if (!track.camera2d)
    {
        throw std::runtime_error("Camera2D track '" + track.parameter + "' is missing camera settings");
    }

    const Camera2DConfig &camera{*track.camera2d};
    const ParameterMetadata &output_metadata{catalog.metadata(camera.output)};
    const Parameter &output{source_parameter(source, camera.output)};
    ResolvedCamera2DConfig camera2d;
    camera2d.aspect = source_camera2d_aspect(output_metadata, output.value, video);
    if (output_metadata.type == ParameterType::CENTER_MAG)
    {
        camera2d.center_mag_x_mag_factor = center_mag_x_mag_factor(output.value);
    }
    camera2d.look_at = resolve_camera2d_value_track(camera.look_at, camera.name, "look-at", false);
    camera2d.view_up = resolve_optional_camera2d_value_track(camera.view_up, camera.name, "view-up", true);
    camera2d.eye = resolve_optional_camera2d_value_track(camera.eye, camera.name, "eye", false);
    camera2d.height = resolve_camera2d_value_track(camera.height, camera.name, "height", false);
    camera2d.skew = resolve_optional_camera2d_value_track(camera.skew, camera.name, "skew", false);

    ResolvedTrack result;
    result.parameter = camera.name;
    result.metadata = output_metadata;
    result.base_value = output.value;
    result.output_parameter = camera.output;
    result.camera2d = camera2d;
    return result;
}

std::string id_3d_view_base_value(
    const ParSet &source, const std::string &output_parameter, const std::vector<KeyframeConfig> &keys)
{
    const Parameter *parameter{find_source_parameter(source, output_parameter)};
    if (parameter != nullptr)
    {
        return parameter->value;
    }
    if (keys.empty())
    {
        throw std::runtime_error("Id 3D view output '" + output_parameter + "' has no keyframes");
    }
    return keys[0].value;
}

void validate_id_3d_view_output_metadata(
    std::string_view output, const ParameterMetadata &metadata, ParameterType expected_type, int expected_arity)
{
    if (metadata.type != expected_type)
    {
        throw std::runtime_error("Id 3D view output '" + std::string{output} + "' has the wrong type");
    }
    if (expected_arity > 0 && (!metadata.arity || *metadata.arity != expected_arity))
    {
        throw std::runtime_error("Id 3D view output '" + std::string{output} + "' has the wrong arity");
    }
}

ResolvedTrack resolve_id_3d_view_member(const Id3DViewConfig &view, const char *member_name, const std::string &output,
    const Id3DViewValueTrackConfig &member, const ParameterCatalog &catalog, const ParSet &source,
    ParameterType expected_type, int expected_arity)
{
    const ParameterMetadata &metadata{catalog.metadata(output)};
    validate_id_3d_view_output_metadata(output, metadata, expected_type, expected_arity);

    ResolvedTrack result;
    result.parameter = view.name + "." + member_name;
    result.metadata = metadata;
    result.base_value = id_3d_view_base_value(source, output, member.keys);
    result.keys = member.keys;
    result.output_parameter = output;
    return result;
}

void add_id_3d_view_member(std::vector<ResolvedTrack> &tracks, const Id3DViewConfig &view, const char *member_name,
    const std::optional<std::string> &output, const std::optional<Id3DViewValueTrackConfig> &member,
    const ParameterCatalog &catalog, const ParSet &source, ParameterType expected_type, int expected_arity)
{
    if (output && member)
    {
        tracks.emplace_back(resolve_id_3d_view_member(
            view, member_name, *output, *member, catalog, source, expected_type, expected_arity));
    }
}

std::vector<ResolvedTrack> resolve_id_3d_view_track(
    const TrackConfig &track, const ParameterCatalog &catalog, const ParSet &source)
{
    if (!track.id_3d_view)
    {
        throw std::runtime_error("Id 3D view track '" + track.parameter + "' is missing view settings");
    }

    const Id3DViewConfig &view{*track.id_3d_view};
    std::vector<ResolvedTrack> result;
    add_id_3d_view_member(result, view, "rotation", view.outputs.rotation, view.rotation, catalog, source,
        ParameterType::NUMERIC_TUPLE, 3);
    add_id_3d_view_member(result, view, "perspective", view.outputs.perspective, view.perspective, catalog, source,
        ParameterType::INTEGER, 0);
    add_id_3d_view_member(
        result, view, "xyshift", view.outputs.xyshift, view.xyshift, catalog, source, ParameterType::NUMERIC_TUPLE, 2);
    add_id_3d_view_member(result, view, "scalexyz", view.outputs.scalexyz, view.scalexyz, catalog, source,
        ParameterType::NUMERIC_TUPLE, 3);
    add_id_3d_view_member(
        result, view, "roughness", view.outputs.roughness, view.roughness, catalog, source, ParameterType::INTEGER, 0);
    add_id_3d_view_member(
        result, view, "sphere", view.outputs.sphere, view.sphere, catalog, source, ParameterType::ENUM, 0);
    add_id_3d_view_member(result, view, "longitude", view.outputs.longitude, view.longitude, catalog, source,
        ParameterType::NUMERIC_TUPLE, 2);
    add_id_3d_view_member(result, view, "latitude", view.outputs.latitude, view.latitude, catalog, source,
        ParameterType::NUMERIC_TUPLE, 2);
    add_id_3d_view_member(
        result, view, "radius", view.outputs.radius, view.radius, catalog, source, ParameterType::INTEGER, 0);
    add_id_3d_view_member(
        result, view, "stereo", view.outputs.stereo, view.stereo, catalog, source, ParameterType::INTEGER, 0);
    add_id_3d_view_member(result, view, "interocular", view.outputs.interocular, view.interocular, catalog, source,
        ParameterType::INTEGER, 0);
    add_id_3d_view_member(
        result, view, "converge", view.outputs.converge, view.converge, catalog, source, ParameterType::INTEGER, 0);
    return result;
}

std::string julibrot_view_base_value(
    const ParSet &source, const std::string &output_parameter, const std::vector<KeyframeConfig> &keys)
{
    const Parameter *parameter{find_source_parameter(source, output_parameter)};
    if (parameter != nullptr)
    {
        return parameter->value;
    }
    if (keys.empty())
    {
        throw std::runtime_error("Julibrot view output '" + output_parameter + "' has no keyframes");
    }
    return keys[0].value;
}

void validate_julibrot_view_output_metadata(
    std::string_view output, const ParameterMetadata &metadata, ParameterType expected_type, int expected_arity)
{
    if (metadata.type != expected_type)
    {
        throw std::runtime_error("Julibrot view output '" + std::string{output} + "' has the wrong type");
    }
    if (expected_arity > 0 && (!metadata.arity || *metadata.arity != expected_arity))
    {
        throw std::runtime_error("Julibrot view output '" + std::string{output} + "' has the wrong arity");
    }
}

ResolvedTrack resolve_julibrot_view_member(const JulibrotViewConfig &view, const char *member_name,
    const std::string &output, const JulibrotViewValueTrackConfig &member, const ParameterCatalog &catalog,
    const ParSet &source, ParameterType expected_type, int expected_arity)
{
    const ParameterMetadata &metadata{catalog.metadata(output)};
    validate_julibrot_view_output_metadata(output, metadata, expected_type, expected_arity);

    ResolvedTrack result;
    result.parameter = view.name + "." + member_name;
    result.metadata = metadata;
    result.base_value = julibrot_view_base_value(source, output, member.keys);
    result.keys = member.keys;
    result.output_parameter = output;
    return result;
}

void add_julibrot_view_member(std::vector<ResolvedTrack> &tracks, const JulibrotViewConfig &view,
    const char *member_name, const std::optional<std::string> &output,
    const std::optional<JulibrotViewValueTrackConfig> &member, const ParameterCatalog &catalog, const ParSet &source,
    ParameterType expected_type, int expected_arity)
{
    if (output && member)
    {
        tracks.emplace_back(resolve_julibrot_view_member(
            view, member_name, *output, *member, catalog, source, expected_type, expected_arity));
    }
}

std::vector<ResolvedTrack> resolve_julibrot_view_track(
    const TrackConfig &track, const ParameterCatalog &catalog, const ParSet &source)
{
    if (!track.julibrot_view)
    {
        throw std::runtime_error("Julibrot view track '" + track.parameter + "' is missing view settings");
    }

    const JulibrotViewConfig &view{*track.julibrot_view};
    std::vector<ResolvedTrack> result;
    add_julibrot_view_member(
        result, view, "mode", view.outputs.mode, view.mode, catalog, source, ParameterType::ENUM, 0);
    add_julibrot_view_member(result, view, "geometry", view.outputs.geometry, view.geometry, catalog, source,
        ParameterType::NUMERIC_TUPLE, 6);
    add_julibrot_view_member(
        result, view, "eyes", view.outputs.eyes, view.eyes, catalog, source, ParameterType::DOUBLE, 0);
    add_julibrot_view_member(
        result, view, "from-to", view.outputs.from_to, view.from_to, catalog, source, ParameterType::NUMERIC_TUPLE, 4);
    return result;
}

ResolvedTrack resolve_track(
    const TrackConfig &track, const ParameterCatalog &catalog, const ParSet &source, std::string_view video)
{
    if (track.kind == TrackKind::CAMERA2D)
    {
        return resolve_camera2d_track(track, catalog, source, video);
    }
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
        if (track.kind == TrackKind::COLOR_MAP)
        {
            continue;
        }
        if (track.kind == TrackKind::ID_3D_VIEW)
        {
            std::vector<ResolvedTrack> tracks{resolve_id_3d_view_track(track, catalog, source)};
            result.tracks.insert(result.tracks.end(), tracks.begin(), tracks.end());
        }
        else if (track.kind == TrackKind::JULIBROT_VIEW)
        {
            std::vector<ResolvedTrack> tracks{resolve_julibrot_view_track(track, catalog, source)};
            result.tracks.insert(result.tracks.end(), tracks.begin(), tracks.end());
        }
        else
        {
            result.tracks.push_back(resolve_track(track, catalog, source, config.video));
        }
    }
    validate_slotted_track_overlaps(result.tracks);

    return result;
}

} // namespace ParFile
