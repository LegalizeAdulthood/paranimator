// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/Config.h>

#include <nlohmann/json.hpp>

#include <cstddef>
#include <iomanip>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace ParFile
{

using Object = nlohmann::json;

static Object parse_json(std::string_view json_text)
{
    try
    {
        Object json = Object::parse(json_text.begin(), json_text.end());
        if (!json.is_object())
        {
            throw std::runtime_error("Invalid config, root is not an object");
        }
        return json;
    }
    catch (const nlohmann::json::exception &bang)
    {
        throw std::runtime_error("Invalid config JSON: " + std::string{bang.what()});
    }
}

static const Object &load_object(const Object &json, std::string_view name)
{
    const std::string key{name};
    if (!json.contains(key) || !json.at(key).is_object())
    {
        throw std::runtime_error("Invalid config, missing object '" + std::string{name} + "'");
    }
    return json.at(key);
}

static std::string load_string(const Object &json, std::string_view name, std::string_view field)
{
    const std::string key{field};
    if (!json.contains(key) || !json.at(key).is_string())
    {
        std::string msg{"Invalid config, missing string '" + std::string{field} + "'"};
        if (!name.empty())
        {
            msg += " in '" + std::string{name} + "'";
        }
        throw std::runtime_error(msg);
    }
    return json.at(key).get<std::string>();
}

static std::string load_string(const Object &json, std::string_view field)
{
    return load_string(json, {}, field);
}

static std::optional<std::string> load_optional_string(const Object &json, std::string_view field)
{
    const std::string key{field};
    if (!json.contains(key))
    {
        return std::nullopt;
    }
    if (!json.at(key).is_string())
    {
        throw std::runtime_error("Invalid config, field '" + std::string{field} + "' is not a string");
    }
    return json.at(key).get<std::string>();
}

static std::optional<bool> load_optional_bool(const Object &json, std::string_view field)
{
    const std::string key{field};
    if (!json.contains(key))
    {
        return std::nullopt;
    }
    if (!json.at(key).is_boolean())
    {
        throw std::runtime_error("Invalid config, field '" + std::string{field} + "' is not a boolean");
    }
    return json.at(key).get<bool>();
}

static std::vector<std::string> load_string_array(const Object &json, std::string_view name)
{
    const std::string key{name};
    if (!json.contains(key) || !json.at(key).is_array())
    {
        throw std::runtime_error("Invalid config, missing array '" + std::string{name} + "'");
    }
    std::vector<std::string> result;
    for (const Object &item : json.at(key))
    {
        if (!item.is_string())
        {
            throw std::runtime_error("Invalid config, array '" + std::string{name} + "' contains non-string value");
        }
        result.emplace_back(item.get<std::string>());
    }
    return result;
}

static NamedFileParSet load_named_file_par_set(const Object &json, std::string_view name)
{
    const Object &named_params{load_object(json, name)};

    NamedFileParSet result;
    result.file = load_string(named_params, name, "file");
    result.name = load_string(named_params, name, "name");
    return result;
}

static OutputConfig load_output_config(const Object &json)
{
    const Object &output{load_object(json, "output")};

    OutputConfig result;
    result.directory = load_string(output, "output", "directory");
    result.par = load_string(output, "output", "par");
    result.entry = load_string(output, "output", "entry");
    result.script = load_string(output, "output", "script");
    return result;
}

static int load_int(const Object &json, std::string_view name)
{
    const std::string key{name};
    if (!json.contains(key) || !json.at(key).is_number_integer())
    {
        throw std::runtime_error("Invalid config, missing integer '" + std::string{name} + "'");
    }
    return json.at(key).get<int>();
}

static int load_positive_int(const Object &json, std::string_view name)
{
    const int value{load_int(json, name)};
    if (value < 1)
    {
        throw std::runtime_error("Invalid config, integer '" + std::string{name} + "' must be positive");
    }
    return value;
}

static std::string format_config_double(double value)
{
    std::ostringstream out;
    out << std::setprecision(12) << value;
    return out.str();
}

static double load_double(const Object &json, std::string_view name)
{
    const std::string key{name};
    if (!json.contains(key) || !json.at(key).is_number())
    {
        throw std::runtime_error("Invalid config, missing number '" + std::string{name} + "'");
    }
    return json.at(key).get<double>();
}

static double load_optional_double(const Object &json, std::string_view name, double default_value)
{
    const std::string key{name};
    if (!json.contains(key))
    {
        return default_value;
    }
    if (!json.at(key).is_number())
    {
        throw std::runtime_error("Invalid config, field '" + std::string{name} + "' is not a number");
    }
    return json.at(key).get<double>();
}

static TrackMode load_track_mode(const Object &json)
{
    const std::optional<std::string> mode{load_optional_string(json, "mode")};
    if (!mode)
    {
        return TrackMode::KEYFRAMES;
    }
    return parse_track_mode(*mode);
}

static TrackKind load_track_kind(const Object &json)
{
    const std::optional<std::string> kind{load_optional_string(json, "type")};
    if (!kind)
    {
        return TrackKind::PARAMETER;
    }
    return parse_track_kind(*kind);
}

static KeyframeConfig load_keyframe_config(const Object &json, TrackMode mode)
{
    KeyframeConfig result;
    result.frame = load_int(json, "frame");
    if (mode == TrackMode::PWM)
    {
        result.mix = load_double(json, "mix");
    }
    else
    {
        result.value = load_string(json, "value");
        if (const std::optional<std::string> curve{load_optional_string(json, "curve")})
        {
            result.curve = parse_curve(*curve);
        }
    }
    return result;
}

static std::vector<KeyframeConfig> load_keyframes(const Object &json, TrackMode mode)
{
    const std::string key{"keys"};
    if (!json.contains(key) || !json.at(key).is_array())
    {
        throw std::runtime_error("Invalid config, missing array 'keys'");
    }
    std::vector<KeyframeConfig> result;
    for (const Object &item : json.at(key))
    {
        if (!item.is_object())
        {
            throw std::runtime_error("Invalid config, array 'keys' contains non-object value");
        }
        result.emplace_back(load_keyframe_config(item, mode));
    }
    return result;
}

static std::string load_camera_keyframe_value(const Object &json, bool number_value)
{
    const std::string key{"value"};
    if (number_value)
    {
        if (!json.contains(key) || !json.at(key).is_number())
        {
            throw std::runtime_error("Invalid config, missing number 'value'");
        }
        return format_config_double(json.at(key).get<double>());
    }
    return load_string(json, "value");
}

static KeyframeConfig load_camera_keyframe_config(const Object &json, bool number_value)
{
    KeyframeConfig result;
    result.frame = load_int(json, "frame");
    result.value = load_camera_keyframe_value(json, number_value);
    if (const std::optional<std::string> curve{load_optional_string(json, "curve")})
    {
        result.curve = parse_curve(*curve);
    }
    return result;
}

static std::vector<KeyframeConfig> load_camera_keyframes(const Object &json, std::string_view name, bool number_value)
{
    const std::string key{"keys"};
    if (!json.contains(key) || !json.at(key).is_array())
    {
        throw std::runtime_error("Invalid config, missing array '" + std::string{name} + ".keys'");
    }
    std::vector<KeyframeConfig> result;
    for (const Object &item : json.at(key))
    {
        if (!item.is_object())
        {
            throw std::runtime_error(
                "Invalid config, array '" + std::string{name} + ".keys' contains non-object value");
        }
        result.emplace_back(load_camera_keyframe_config(item, number_value));
    }
    return result;
}

static Camera2DValueTrackConfig load_camera2d_value_track_config(
    const Object &json, std::string_view name, ParameterType expected_type, bool number_value)
{
    const Object &track{load_object(json, name)};
    Camera2DValueTrackConfig result;
    result.type = parse_parameter_type(load_string(track, name, "type"));
    if (result.type != expected_type)
    {
        throw std::runtime_error("Invalid config, camera2d '" + std::string{name} + "' has the wrong type");
    }
    result.normalize = load_optional_bool(track, "normalize").value_or(false);
    result.keys = load_camera_keyframes(track, name, number_value);
    return result;
}

static Camera2DConfig load_camera2d_config(const Object &json)
{
    Camera2DConfig result;
    result.name = load_string(json, "name");
    result.output = load_string(json, "output");
    result.aspect = load_string(json, "aspect");
    if (result.aspect != "source")
    {
        throw std::runtime_error("Invalid config, camera2d aspect must be 'source'");
    }
    result.look_at = load_camera2d_value_track_config(json, "look-at", ParameterType::POINT2, false);
    result.view_up = load_camera2d_value_track_config(json, "view-up", ParameterType::VECTOR2, false);
    result.height = load_camera2d_value_track_config(json, "height", ParameterType::DOUBLE, true);
    return result;
}

static std::string load_id_3d_view_keyframe_value(const Object &json, ParameterType type)
{
    const std::string key{"value"};
    if (type == ParameterType::INTEGER)
    {
        if (!json.contains(key) || !json.at(key).is_number_integer())
        {
            throw std::runtime_error("Invalid config, missing integer 'value'");
        }
        return std::to_string(json.at(key).get<int>());
    }
    return load_string(json, "value");
}

static KeyframeConfig load_id_3d_view_keyframe_config(const Object &json, ParameterType type)
{
    KeyframeConfig result;
    result.frame = load_int(json, "frame");
    result.value = load_id_3d_view_keyframe_value(json, type);
    if (const std::optional<std::string> curve{load_optional_string(json, "curve")})
    {
        result.curve = parse_curve(*curve);
    }
    return result;
}

static std::vector<KeyframeConfig> load_id_3d_view_keyframes(
    const Object &json, std::string_view name, ParameterType type)
{
    const std::string key{"keys"};
    if (!json.contains(key) || !json.at(key).is_array())
    {
        throw std::runtime_error("Invalid config, missing array '" + std::string{name} + ".keys'");
    }

    std::vector<KeyframeConfig> result;
    for (const Object &item : json.at(key))
    {
        if (!item.is_object())
        {
            throw std::runtime_error(
                "Invalid config, array '" + std::string{name} + ".keys' contains non-object value");
        }
        result.emplace_back(load_id_3d_view_keyframe_config(item, type));
    }
    return result;
}

static std::optional<Id3DViewValueTrackConfig> load_optional_id_3d_view_value_track_config(
    const Object &json, std::string_view name, ParameterType expected_type, int expected_arity)
{
    const std::string key{name};
    if (!json.contains(key))
    {
        return {};
    }

    const Object &track{load_object(json, name)};
    Id3DViewValueTrackConfig result;
    result.type = parse_parameter_type(load_string(track, name, "type"));
    if (result.type != expected_type)
    {
        throw std::runtime_error("Invalid config, id-3d-view '" + std::string{name} + "' has the wrong type");
    }
    if (expected_arity > 0)
    {
        result.arity = load_positive_int(track, "arity");
        if (*result.arity != expected_arity)
        {
            throw std::runtime_error("Invalid config, id-3d-view '" + std::string{name} + "' has the wrong arity");
        }
    }
    result.keys = load_id_3d_view_keyframes(track, name, result.type);
    return result;
}

static Id3DViewOutputsConfig load_id_3d_view_outputs_config(const Object &json)
{
    const Object &outputs{load_object(json, "outputs")};
    Id3DViewOutputsConfig result;
    result.rotation = load_optional_string(outputs, "rotation");
    result.perspective = load_optional_string(outputs, "perspective");
    result.xyshift = load_optional_string(outputs, "xyshift");
    if (!result.rotation && !result.perspective && !result.xyshift)
    {
        throw std::runtime_error("Invalid config, id-3d-view outputs must name at least one output");
    }
    return result;
}

static void validate_id_3d_view_member(
    const char *name, const std::optional<std::string> &output, const std::optional<Id3DViewValueTrackConfig> &track)
{
    if (output && !track)
    {
        throw std::runtime_error("Invalid config, id-3d-view output '" + std::string{name} + "' has no track");
    }
    if (!output && track)
    {
        throw std::runtime_error("Invalid config, id-3d-view track '" + std::string{name} + "' has no output");
    }
}

static Id3DViewConfig load_id_3d_view_config(const Object &json)
{
    Id3DViewConfig result;
    result.name = load_string(json, "name");
    result.outputs = load_id_3d_view_outputs_config(json);
    result.rotation = load_optional_id_3d_view_value_track_config(json, "rotation", ParameterType::NUMERIC_TUPLE, 3);
    result.perspective = load_optional_id_3d_view_value_track_config(json, "perspective", ParameterType::INTEGER, 0);
    result.xyshift = load_optional_id_3d_view_value_track_config(json, "xyshift", ParameterType::NUMERIC_TUPLE, 2);
    validate_id_3d_view_member("rotation", result.outputs.rotation, result.rotation);
    validate_id_3d_view_member("perspective", result.outputs.perspective, result.perspective);
    validate_id_3d_view_member("xyshift", result.outputs.xyshift, result.xyshift);
    return result;
}

static PathKind load_path_kind(const Object &json)
{
    const std::string kind{load_string(json, "kind")};
    if (kind == "constant")
    {
        return PathKind::CONSTANT;
    }
    if (kind == "line")
    {
        return PathKind::LINE;
    }
    if (kind == "circle")
    {
        return PathKind::CIRCLE;
    }
    if (kind == "ellipse")
    {
        return PathKind::ELLIPSE;
    }
    if (kind == "lissajous")
    {
        return PathKind::LISSAJOUS;
    }
    if (kind == "spiral")
    {
        return PathKind::SPIRAL;
    }
    if (kind == "bezier")
    {
        return PathKind::BEZIER;
    }
    if (kind == "catmull-rom")
    {
        return PathKind::CATMULL_ROM;
    }
    throw std::runtime_error("Invalid config, unknown path kind '" + kind + "'");
}

static void validate_nonnegative(double value, std::string_view name)
{
    if (value < 0.0)
    {
        throw std::runtime_error("Invalid config, path field '" + std::string{name} + "' must be nonnegative");
    }
}

static void validate_positive(double value, std::string_view name)
{
    if (value <= 0.0)
    {
        throw std::runtime_error("Invalid config, path field '" + std::string{name} + "' must be positive");
    }
}

static PathConfig load_path_config(const Object &json)
{
    if (!json.is_object())
    {
        throw std::runtime_error("Invalid config, field 'path' is not an object");
    }

    PathConfig result;
    result.kind = load_path_kind(json);
    switch (result.kind)
    {
    case PathKind::CONSTANT:
        result.value = load_string(json, "value");
        break;
    case PathKind::LINE:
        result.from = load_string(json, "from");
        result.to = load_string(json, "to");
        break;
    case PathKind::CIRCLE:
        result.center = load_string(json, "center");
        result.radius = load_double(json, "radius");
        validate_nonnegative(result.radius, "radius");
        result.turns = load_optional_double(json, "turns", 1.0);
        result.phase = load_optional_double(json, "phase", 0.0);
        break;
    case PathKind::ELLIPSE:
        result.center = load_string(json, "center");
        result.x_radius = load_double(json, "x-radius");
        result.y_radius = load_double(json, "y-radius");
        validate_nonnegative(result.x_radius, "x-radius");
        validate_nonnegative(result.y_radius, "y-radius");
        result.turns = load_optional_double(json, "turns", 1.0);
        result.phase = load_optional_double(json, "phase", 0.0);
        break;
    case PathKind::LISSAJOUS:
        result.center = load_string(json, "center");
        result.x_radius = load_double(json, "x-radius");
        result.y_radius = load_double(json, "y-radius");
        result.x_frequency = load_double(json, "x-frequency");
        result.y_frequency = load_double(json, "y-frequency");
        validate_nonnegative(result.x_radius, "x-radius");
        validate_nonnegative(result.y_radius, "y-radius");
        validate_positive(result.x_frequency, "x-frequency");
        validate_positive(result.y_frequency, "y-frequency");
        result.phase = load_optional_double(json, "phase", 0.0);
        break;
    case PathKind::SPIRAL:
        result.center = load_string(json, "center");
        result.from_radius = load_double(json, "from-radius");
        result.to_radius = load_double(json, "to-radius");
        validate_nonnegative(result.from_radius, "from-radius");
        validate_nonnegative(result.to_radius, "to-radius");
        result.turns = load_optional_double(json, "turns", 1.0);
        result.phase = load_optional_double(json, "phase", 0.0);
        break;
    case PathKind::BEZIER:
        result.control_points = load_string_array(json, "control-points");
        if (result.control_points.size() < 2U)
        {
            throw std::runtime_error("Invalid config, bezier path requires at least two control points");
        }
        break;
    case PathKind::CATMULL_ROM:
        result.control_points = load_string_array(json, "control-points");
        if (result.control_points.size() < 4U)
        {
            throw std::runtime_error("Invalid config, catmull-rom path requires at least four control points");
        }
        break;
    }
    return result;
}

static std::vector<KeyframeConfig> load_path_keyframes(const PathConfig &path, int num_frames)
{
    if (num_frames < 2)
    {
        throw std::runtime_error("Invalid config, path tracks require at least two frames");
    }

    switch (path.kind)
    {
    case PathKind::CONSTANT:
        return {{0, path.value}, {num_frames - 1, path.value, Curve::HOLD}};
    case PathKind::LINE:
        return {{0, path.from}, {num_frames - 1, path.to, Curve::LINEAR}};
    case PathKind::CIRCLE:
    case PathKind::ELLIPSE:
    case PathKind::LISSAJOUS:
    case PathKind::SPIRAL:
    case PathKind::BEZIER:
    case PathKind::CATMULL_ROM:
        return {};
    }
    throw std::runtime_error("Invalid config, unknown path kind");
}

static ColorMapEffectKind load_color_map_effect_kind(const Object &json)
{
    const std::string kind{load_string(json, "kind")};
    if (kind == "brightness")
    {
        return ColorMapEffectKind::BRIGHTNESS;
    }
    if (kind == "contrast")
    {
        return ColorMapEffectKind::CONTRAST;
    }
    if (kind == "gamma")
    {
        return ColorMapEffectKind::GAMMA;
    }
    if (kind == "hue-shift")
    {
        return ColorMapEffectKind::HUE_SHIFT;
    }
    if (kind == "mask-blend")
    {
        return ColorMapEffectKind::MASK_BLEND;
    }
    if (kind == "pulse")
    {
        return ColorMapEffectKind::PULSE;
    }
    if (kind == "remap")
    {
        return ColorMapEffectKind::REMAP;
    }
    if (kind == "reverse")
    {
        return ColorMapEffectKind::REVERSE;
    }
    if (kind == "saturation")
    {
        return ColorMapEffectKind::SATURATION;
    }
    if (kind == "sparkle")
    {
        return ColorMapEffectKind::SPARKLE;
    }
    if (kind == "ping-pong")
    {
        return ColorMapEffectKind::PING_PONG;
    }
    throw std::runtime_error("Invalid config, unknown color map effect kind '" + kind + "'");
}

static ColorMapRangeConfig load_color_map_range_value(const Object &range)
{
    if (!range.is_array() || range.size() != 2U)
    {
        throw std::runtime_error("Invalid config, color map range must have two entries");
    }
    if (!range.at(0).is_number_integer() || !range.at(1).is_number_integer())
    {
        throw std::runtime_error("Invalid config, color map range entries must be integers");
    }
    return ColorMapRangeConfig{range.at(0).get<int>(), range.at(1).get<int>()};
}

static std::optional<ColorMapRangeConfig> load_color_map_range(const Object &json)
{
    const std::string key{"range"};
    if (!json.contains(key))
    {
        return std::nullopt;
    }
    return load_color_map_range_value(json.at(key));
}

static std::vector<ColorMapRangeConfig> load_color_map_ranges(const Object &json)
{
    const std::string key{"ranges"};
    if (!json.contains(key) || !json.at(key).is_array())
    {
        throw std::runtime_error("Invalid config, missing array 'ranges'");
    }
    std::vector<ColorMapRangeConfig> result;
    for (const Object &range : json.at(key))
    {
        result.emplace_back(load_color_map_range_value(range));
    }
    return result;
}

static std::vector<int> load_int_array(const Object &json, std::string_view name)
{
    const std::string key{name};
    if (!json.contains(key) || !json.at(key).is_array())
    {
        throw std::runtime_error("Invalid config, missing array '" + std::string{name} + "'");
    }
    std::vector<int> result;
    for (const Object &item : json.at(key))
    {
        if (!item.is_number_integer())
        {
            throw std::runtime_error("Invalid config, array '" + std::string{name} + "' contains non-integer value");
        }
        result.emplace_back(item.get<int>());
    }
    return result;
}

static NumberKeyframeConfig load_number_keyframe_config(const Object &json)
{
    NumberKeyframeConfig result;
    result.frame = load_int(json, "frame");
    result.value = load_double(json, "value");
    if (const std::optional<std::string> curve{load_optional_string(json, "curve")})
    {
        result.curve = parse_curve(*curve);
    }
    return result;
}

static NumberTrackConfig load_number_track_config(const Object &json, std::string_view name)
{
    const Object &track{load_object(json, name)};
    const std::string key{"keys"};
    if (!track.contains(key) || !track.at(key).is_array())
    {
        throw std::runtime_error("Invalid config, missing array '" + std::string{name} + ".keys'");
    }

    NumberTrackConfig result;
    for (const Object &item : track.at(key))
    {
        if (!item.is_object())
        {
            throw std::runtime_error(
                "Invalid config, array '" + std::string{name} + ".keys' contains non-object value");
        }
        result.keys.emplace_back(load_number_keyframe_config(item));
    }
    return result;
}

static ColorMapGradientStopConfig load_color_map_gradient_stop_config(const Object &json)
{
    ColorMapGradientStopConfig result;
    result.index = load_int(json, "index");
    result.color = load_string(json, "color");
    return result;
}

static ColorMapGradientConfig load_color_map_gradient_config(const Object &json)
{
    const std::string kind{load_string(json, "kind")};
    if (kind != "gradient")
    {
        throw std::runtime_error("Invalid config, unknown color map source kind '" + kind + "'");
    }

    const std::string key{"stops"};
    if (!json.contains(key) || !json.at(key).is_array())
    {
        throw std::runtime_error("Invalid config, missing array 'stops'");
    }

    ColorMapGradientConfig result;
    for (const Object &item : json.at(key))
    {
        if (!item.is_object())
        {
            throw std::runtime_error("Invalid config, array 'stops' contains non-object value");
        }
        result.stops.emplace_back(load_color_map_gradient_stop_config(item));
    }
    return result;
}

static void load_color_map_source_config(const Object &json, ColorMapConfig &result)
{
    const std::string key{"source"};
    if (!json.contains(key))
    {
        return;
    }

    const Object &source{json.at(key)};
    if (source.is_string())
    {
        result.source = source.get<std::string>();
        return;
    }
    if (source.is_object())
    {
        result.gradient = load_color_map_gradient_config(source);
        return;
    }
    throw std::runtime_error("Invalid config, color map source must be a string or object");
}

static ColorMapEffectConfig load_color_map_effect_config(const Object &json)
{
    ColorMapEffectConfig result;
    result.kind = load_color_map_effect_kind(json);
    result.range = load_color_map_range(json);
    switch (result.kind)
    {
    case ColorMapEffectKind::BRIGHTNESS:
    case ColorMapEffectKind::CONTRAST:
    case ColorMapEffectKind::GAMMA:
    case ColorMapEffectKind::HUE_SHIFT:
    case ColorMapEffectKind::SATURATION:
        result.amount = load_number_track_config(json, "amount");
        break;
    case ColorMapEffectKind::MASK_BLEND:
        result.ranges = load_color_map_ranges(json);
        result.source = load_string(json, "source");
        result.amount = load_number_track_config(json, "amount");
        break;
    case ColorMapEffectKind::PING_PONG:
        result.offset = load_number_track_config(json, "offset");
        break;
    case ColorMapEffectKind::PULSE:
        result.color = load_string(json, "color");
        result.amount = load_number_track_config(json, "amount");
        break;
    case ColorMapEffectKind::REMAP:
        result.indices = load_int_array(json, "indices");
        break;
    case ColorMapEffectKind::REVERSE:
        break;
    case ColorMapEffectKind::SPARKLE:
        result.seed = load_int(json, "seed");
        result.amount = load_number_track_config(json, "amount");
        break;
    }
    return result;
}

static std::vector<ColorMapEffectConfig> load_color_map_effects(const Object &json)
{
    const std::string key{"effects"};
    if (!json.contains(key))
    {
        return {};
    }
    if (!json.at(key).is_array())
    {
        throw std::runtime_error("Invalid config, missing array 'effects'");
    }

    std::vector<ColorMapEffectConfig> result;
    for (const Object &item : json.at(key))
    {
        if (!item.is_object())
        {
            throw std::runtime_error("Invalid config, array 'effects' contains non-object value");
        }
        result.emplace_back(load_color_map_effect_config(item));
    }
    return result;
}

static PwmConfig load_pwm_config(const Object &json)
{
    PwmConfig result;
    result.a = load_string(json, "a");
    result.b = load_string(json, "b");
    result.window = load_int(json, "window");
    if (result.window < 2)
    {
        throw std::runtime_error("Invalid config, pwm window must be at least 2");
    }
    return result;
}

static ColorMapConfig load_color_map_config(const Object &json)
{
    ColorMapConfig result;
    result.format = parse_track_format(load_string(json, "format"));
    result.output = load_string(json, "output");
    load_color_map_source_config(json, result);
    result.effects = load_color_map_effects(json);
    return result;
}

static TrackConfig load_track_config(const Object &json, int num_frames)
{
    TrackConfig result;
    result.kind = load_track_kind(json);
    result.mode = load_track_mode(json);
    if (result.kind == TrackKind::CAMERA2D)
    {
        result.camera2d = load_camera2d_config(json);
        result.parameter = result.camera2d->name;
        return result;
    }
    if (result.kind == TrackKind::ID_3D_VIEW)
    {
        result.id_3d_view = load_id_3d_view_config(json);
        result.parameter = result.id_3d_view->name;
        return result;
    }

    result.parameter = load_string(json, "parameter");
    if (result.kind == TrackKind::COLOR_MAP)
    {
        result.color_map = load_color_map_config(json);
    }
    if (result.mode == TrackMode::PWM)
    {
        result.pwm = load_pwm_config(json);
    }
    if (json.contains("keys") && json.contains("path"))
    {
        throw std::runtime_error("Invalid config, track cannot contain both 'keys' and 'path'");
    }
    if (json.contains("keys"))
    {
        result.keys = load_keyframes(json, result.mode);
    }
    else if (json.contains("path"))
    {
        if (result.kind != TrackKind::PARAMETER || result.mode != TrackMode::KEYFRAMES)
        {
            throw std::runtime_error("Invalid config, path is only valid for parameter tracks");
        }
        result.path = load_path_config(json.at("path"));
        result.keys = load_path_keyframes(*result.path, num_frames);
    }
    else if (result.kind != TrackKind::COLOR_MAP)
    {
        throw std::runtime_error("Invalid config, missing array 'keys'");
    }
    return result;
}

static std::vector<TrackConfig> load_tracks(const Object &json, std::string_view name, int num_frames)
{
    const std::string key{name};
    if (!json.contains(key) || !json.at(key).is_array())
    {
        throw std::runtime_error("Invalid config, missing array '" + std::string{name} + "'");
    }
    std::vector<TrackConfig> result;
    for (const Object &item : json.at(key))
    {
        if (!item.is_object())
        {
            throw std::runtime_error("Invalid config, array '" + std::string{name} + "' contains non-object value");
        }
        result.emplace_back(load_track_config(item, num_frames));
    }
    return result;
}

Config read_config(std::string_view json_text)
{
    const Object json = parse_json(json_text);
    Config result;
    result.parameter_catalogs = load_string_array(json, "parameter-catalogs");
    result.source = load_named_file_par_set(json, "source");
    result.output = load_output_config(json);
    result.video = load_string(json, "video");
    result.num_frames = load_int(json, "num-frames");
    result.tracks = load_tracks(json, "tracks", result.num_frames);

    if (json.contains("parallel"))
    {
        if (!json.at("parallel").is_number_integer())
        {
            throw std::runtime_error("Invalid config, 'parallel' is not a number");
        }
        result.parallel = json.at("parallel").get<int>();
    }
    return result;
}

} // namespace ParFile
