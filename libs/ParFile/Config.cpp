// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/Config.h>

#include <ParFile/NumberTrack.h>

#include <nlohmann/json.hpp>

#include <algorithm>
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

static char ascii_lower(char value)
{
    if (value >= 'A' && value <= 'Z')
    {
        return static_cast<char>(value - 'A' + 'a');
    }
    return value;
}

static bool has_gif_extension(std::string_view text)
{
    const std::string_view extension{".gif"};
    if (text.size() < extension.size())
    {
        return false;
    }
    const std::string_view suffix{text.substr(text.size() - extension.size())};
    return std::equal(
        extension.begin(), extension.end(), suffix.begin(), [](char lhs, char rhs) { return lhs == ascii_lower(rhs); });
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
    result.frames = load_optional_string(output, "frames");
    result.layers = load_optional_string(output, "layers");
    result.compose_script = load_optional_string(output, "compose-script");
    result.background = load_optional_string(output, "background");
    if (result.layers && !has_gif_extension(*result.layers))
    {
        throw std::runtime_error("Invalid config, output layers must use .gif extension");
    }
    if (result.compose_script && (!result.frames || !result.layers))
    {
        throw std::runtime_error("Invalid config, compose-script requires output frames and layers");
    }
    return result;
}

static ComposeOperator load_compose_operator(const Object &json)
{
    const std::optional<std::string> compose{load_optional_string(json, "compose")};
    if (!compose)
    {
        return ComposeOperator::SOURCE_OVER;
    }
    if (*compose == "clear")
    {
        return ComposeOperator::CLEAR;
    }
    if (*compose == "copy")
    {
        return ComposeOperator::COPY;
    }
    if (*compose == "destination")
    {
        return ComposeOperator::DESTINATION;
    }
    if (*compose == "source-over")
    {
        return ComposeOperator::SOURCE_OVER;
    }
    if (*compose == "destination-over")
    {
        return ComposeOperator::DESTINATION_OVER;
    }
    if (*compose == "source-in")
    {
        return ComposeOperator::SOURCE_IN;
    }
    if (*compose == "destination-in")
    {
        return ComposeOperator::DESTINATION_IN;
    }
    if (*compose == "source-out")
    {
        return ComposeOperator::SOURCE_OUT;
    }
    if (*compose == "destination-out")
    {
        return ComposeOperator::DESTINATION_OUT;
    }
    if (*compose == "source-atop")
    {
        return ComposeOperator::SOURCE_ATOP;
    }
    if (*compose == "destination-atop")
    {
        return ComposeOperator::DESTINATION_ATOP;
    }
    if (*compose == "xor")
    {
        return ComposeOperator::XOR;
    }
    if (*compose == "add")
    {
        return ComposeOperator::ADD;
    }
    if (*compose == "subtract")
    {
        return ComposeOperator::SUBTRACT;
    }
    if (*compose == "multiply")
    {
        return ComposeOperator::MULTIPLY;
    }
    if (*compose == "divide")
    {
        return ComposeOperator::DIVIDE;
    }
    if (*compose == "min")
    {
        return ComposeOperator::MIN;
    }
    if (*compose == "max")
    {
        return ComposeOperator::MAX;
    }
    if (*compose == "difference")
    {
        return ComposeOperator::DIFFERENCE;
    }
    if (*compose == "average")
    {
        return ComposeOperator::AVERAGE;
    }
    if (*compose == "screen")
    {
        return ComposeOperator::SCREEN;
    }
    if (*compose == "overlay")
    {
        return ComposeOperator::OVERLAY;
    }
    throw std::runtime_error("Invalid config, unknown compose operator '" + *compose + "'");
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

static PathConfig load_path_config(const Object &json);
static std::vector<KeyframeConfig> load_path_keyframes(const PathConfig &path, int num_frames);

static Camera2DValueTrackConfig load_camera2d_value_track_config(
    const Object &json, std::string_view name, ParameterType expected_type, bool number_value, int num_frames)
{
    const Object &track{load_object(json, name)};
    Camera2DValueTrackConfig result;
    result.type = parse_parameter_type(load_string(track, name, "type"));
    if (result.type != expected_type)
    {
        throw std::runtime_error("Invalid config, camera2d '" + std::string{name} + "' has the wrong type");
    }
    result.normalize = load_optional_bool(track, "normalize").value_or(false);
    if (track.contains("keys") && track.contains("path"))
    {
        throw std::runtime_error(
            "Invalid config, camera2d '" + std::string{name} + "' cannot contain both keys and path");
    }
    if (track.contains("path"))
    {
        result.path = load_path_config(track.at("path"));
        result.keys = load_path_keyframes(*result.path, num_frames);
        if (!result.keys.empty())
        {
            result.path.reset();
        }
    }
    else
    {
        result.keys = load_camera_keyframes(track, name, number_value);
    }
    return result;
}

static std::optional<Camera2DValueTrackConfig> load_optional_camera2d_value_track_config(
    const Object &json, std::string_view name, ParameterType expected_type, bool number_value, int num_frames)
{
    if (!json.contains(std::string{name}))
    {
        return {};
    }
    return load_camera2d_value_track_config(json, name, expected_type, number_value, num_frames);
}

static Camera2DConfig load_camera2d_config(const Object &json, int num_frames)
{
    Camera2DConfig result;
    result.name = load_string(json, "name");
    result.output = load_string(json, "output");
    result.aspect = load_string(json, "aspect");
    if (result.aspect != "source")
    {
        throw std::runtime_error("Invalid config, camera2d aspect must be 'source'");
    }
    result.look_at = load_camera2d_value_track_config(json, "look-at", ParameterType::POINT2, false, num_frames);
    result.view_up =
        load_optional_camera2d_value_track_config(json, "view-up", ParameterType::VECTOR2, false, num_frames);
    result.eye = load_optional_camera2d_value_track_config(json, "eye", ParameterType::POINT2, false, num_frames);
    if (!result.view_up && !result.eye)
    {
        throw std::runtime_error("Invalid config, camera2d requires view-up or eye");
    }
    result.height = load_camera2d_value_track_config(json, "height", ParameterType::DOUBLE, true, num_frames);
    result.skew = load_optional_camera2d_value_track_config(json, "skew", ParameterType::DOUBLE, true, num_frames);
    return result;
}

static std::string load_view_keyframe_value(const Object &json, ParameterType type)
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
    if (type == ParameterType::DOUBLE)
    {
        if (!json.contains(key) || !json.at(key).is_number())
        {
            throw std::runtime_error("Invalid config, missing number 'value'");
        }
        return format_config_double(json.at(key).get<double>());
    }
    return load_string(json, "value");
}

static KeyframeConfig load_view_keyframe_config(const Object &json, ParameterType type)
{
    KeyframeConfig result;
    result.frame = load_int(json, "frame");
    result.value = load_view_keyframe_value(json, type);
    if (const std::optional<std::string> curve{load_optional_string(json, "curve")})
    {
        result.curve = parse_curve(*curve);
    }
    return result;
}

static std::vector<KeyframeConfig> load_view_keyframes(const Object &json, std::string_view name, ParameterType type)
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
        result.emplace_back(load_view_keyframe_config(item, type));
    }
    return result;
}

static Camera3DValueTrackConfig load_camera3d_value_track_config(
    const Object &json, std::string_view name, ParameterType expected_type)
{
    const Object &track{load_object(json, name)};
    Camera3DValueTrackConfig result;
    result.type = parse_parameter_type(load_string(track, name, "type"));
    if (result.type != expected_type)
    {
        throw std::runtime_error("Invalid config, camera3d '" + std::string{name} + "' has the wrong type");
    }
    result.normalize = load_optional_bool(track, "normalize").value_or(false);
    result.keys = load_view_keyframes(track, name, result.type);
    return result;
}

static std::optional<Camera3DConfig> load_optional_camera3d_config(const Object &json)
{
    if (!json.contains("camera3d"))
    {
        return {};
    }

    const Object &camera{load_object(json, "camera3d")};
    Camera3DConfig result;
    result.eye = load_camera3d_value_track_config(camera, "eye", ParameterType::POINT3);
    result.look_at = load_camera3d_value_track_config(camera, "look-at", ParameterType::POINT3);
    result.view_up = load_camera3d_value_track_config(camera, "view-up", ParameterType::VECTOR3);
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
    result.keys = load_view_keyframes(track, name, result.type);
    return result;
}

static Id3DViewOutputsConfig load_id_3d_view_outputs_config(const Object &json)
{
    const Object &outputs{load_object(json, "outputs")};
    Id3DViewOutputsConfig result;
    result.rotation = load_optional_string(outputs, "rotation");
    result.perspective = load_optional_string(outputs, "perspective");
    result.xyshift = load_optional_string(outputs, "xyshift");
    result.scalexyz = load_optional_string(outputs, "scalexyz");
    result.roughness = load_optional_string(outputs, "roughness");
    result.sphere = load_optional_string(outputs, "sphere");
    result.longitude = load_optional_string(outputs, "longitude");
    result.latitude = load_optional_string(outputs, "latitude");
    result.radius = load_optional_string(outputs, "radius");
    result.stereo = load_optional_string(outputs, "stereo");
    result.interocular = load_optional_string(outputs, "interocular");
    result.converge = load_optional_string(outputs, "converge");
    if (!result.rotation && !result.perspective && !result.xyshift && !result.scalexyz && !result.roughness &&
        !result.sphere && !result.longitude && !result.latitude && !result.radius && !result.stereo &&
        !result.interocular && !result.converge)
    {
        throw std::runtime_error("Invalid config, id-3d-view outputs must name at least one output");
    }
    return result;
}

static void validate_id_3d_view_member(const char *name, const std::optional<std::string> &output,
    const std::optional<Id3DViewValueTrackConfig> &track, const std::optional<Camera3DConfig> &camera3d)
{
    if (output && !track)
    {
        const std::string member{name};
        if (!camera3d || (member != "rotation" && member != "perspective" && member != "xyshift"))
        {
            throw std::runtime_error("Invalid config, id-3d-view output '" + member + "' has no track");
        }
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
    result.camera3d = load_optional_camera3d_config(json);
    result.rotation = load_optional_id_3d_view_value_track_config(json, "rotation", ParameterType::NUMERIC_TUPLE, 3);
    result.perspective = load_optional_id_3d_view_value_track_config(json, "perspective", ParameterType::INTEGER, 0);
    result.xyshift = load_optional_id_3d_view_value_track_config(json, "xyshift", ParameterType::NUMERIC_TUPLE, 2);
    result.scalexyz = load_optional_id_3d_view_value_track_config(json, "scalexyz", ParameterType::NUMERIC_TUPLE, 3);
    result.roughness = load_optional_id_3d_view_value_track_config(json, "roughness", ParameterType::INTEGER, 0);
    result.sphere = load_optional_id_3d_view_value_track_config(json, "sphere", ParameterType::ENUM, 0);
    result.longitude = load_optional_id_3d_view_value_track_config(json, "longitude", ParameterType::NUMERIC_TUPLE, 2);
    result.latitude = load_optional_id_3d_view_value_track_config(json, "latitude", ParameterType::NUMERIC_TUPLE, 2);
    result.radius = load_optional_id_3d_view_value_track_config(json, "radius", ParameterType::INTEGER, 0);
    result.stereo = load_optional_id_3d_view_value_track_config(json, "stereo", ParameterType::INTEGER, 0);
    result.interocular = load_optional_id_3d_view_value_track_config(json, "interocular", ParameterType::INTEGER, 0);
    result.converge = load_optional_id_3d_view_value_track_config(json, "converge", ParameterType::INTEGER, 0);
    validate_id_3d_view_member("rotation", result.outputs.rotation, result.rotation, result.camera3d);
    validate_id_3d_view_member("perspective", result.outputs.perspective, result.perspective, result.camera3d);
    validate_id_3d_view_member("xyshift", result.outputs.xyshift, result.xyshift, result.camera3d);
    validate_id_3d_view_member("scalexyz", result.outputs.scalexyz, result.scalexyz, result.camera3d);
    validate_id_3d_view_member("roughness", result.outputs.roughness, result.roughness, result.camera3d);
    validate_id_3d_view_member("sphere", result.outputs.sphere, result.sphere, result.camera3d);
    validate_id_3d_view_member("longitude", result.outputs.longitude, result.longitude, result.camera3d);
    validate_id_3d_view_member("latitude", result.outputs.latitude, result.latitude, result.camera3d);
    validate_id_3d_view_member("radius", result.outputs.radius, result.radius, result.camera3d);
    validate_id_3d_view_member("stereo", result.outputs.stereo, result.stereo, result.camera3d);
    validate_id_3d_view_member("interocular", result.outputs.interocular, result.interocular, result.camera3d);
    validate_id_3d_view_member("converge", result.outputs.converge, result.converge, result.camera3d);
    return result;
}

static std::optional<JulibrotViewValueTrackConfig> load_optional_julibrot_view_value_track_config(
    const Object &json, std::string_view name, ParameterType expected_type, int expected_arity)
{
    const std::string key{name};
    if (!json.contains(key))
    {
        return {};
    }

    const Object &track{load_object(json, name)};
    JulibrotViewValueTrackConfig result;
    result.type = parse_parameter_type(load_string(track, name, "type"));
    if (result.type != expected_type)
    {
        throw std::runtime_error("Invalid config, julibrot-view '" + std::string{name} + "' has the wrong type");
    }
    if (expected_arity > 0)
    {
        result.arity = load_positive_int(track, "arity");
        if (*result.arity != expected_arity)
        {
            throw std::runtime_error("Invalid config, julibrot-view '" + std::string{name} + "' has the wrong arity");
        }
    }
    result.keys = load_view_keyframes(track, name, result.type);
    return result;
}

static JulibrotViewOutputsConfig load_julibrot_view_outputs_config(const Object &json)
{
    const Object &outputs{load_object(json, "outputs")};
    JulibrotViewOutputsConfig result;
    result.mode = load_optional_string(outputs, "mode");
    result.geometry = load_optional_string(outputs, "geometry");
    result.eyes = load_optional_string(outputs, "eyes");
    result.from_to = load_optional_string(outputs, "from-to");
    if (!result.mode && !result.geometry && !result.eyes && !result.from_to)
    {
        throw std::runtime_error("Invalid config, julibrot-view outputs must name at least one output");
    }
    return result;
}

static void validate_julibrot_view_member(const char *name, const std::optional<std::string> &output,
    const std::optional<JulibrotViewValueTrackConfig> &track, const std::optional<Camera3DConfig> &camera3d)
{
    if (output && !track)
    {
        const std::string member{name};
        if (!camera3d || member != "geometry")
        {
            throw std::runtime_error("Invalid config, julibrot-view output '" + member + "' has no track");
        }
    }
    if (!output && track)
    {
        throw std::runtime_error("Invalid config, julibrot-view track '" + std::string{name} + "' has no output");
    }
}

static void reject_julibrot_camera_field(const Object &json, std::string_view name)
{
    if (json.contains(std::string{name}))
    {
        throw std::runtime_error("Invalid config, julibrot-view does not support '" + std::string{name} + "'");
    }
}

static JulibrotViewConfig load_julibrot_view_config(const Object &json)
{
    reject_julibrot_camera_field(json, "look-at");
    reject_julibrot_camera_field(json, "view-up");

    JulibrotViewConfig result;
    result.name = load_string(json, "name");
    result.outputs = load_julibrot_view_outputs_config(json);
    result.camera3d = load_optional_camera3d_config(json);
    result.mode = load_optional_julibrot_view_value_track_config(json, "mode", ParameterType::ENUM, 0);
    result.geometry = load_optional_julibrot_view_value_track_config(json, "geometry", ParameterType::NUMERIC_TUPLE, 6);
    result.eyes = load_optional_julibrot_view_value_track_config(json, "eyes", ParameterType::DOUBLE, 0);
    result.from_to = load_optional_julibrot_view_value_track_config(json, "from-to", ParameterType::NUMERIC_TUPLE, 4);
    validate_julibrot_view_member("mode", result.outputs.mode, result.mode, result.camera3d);
    validate_julibrot_view_member("geometry", result.outputs.geometry, result.geometry, result.camera3d);
    validate_julibrot_view_member("eyes", result.outputs.eyes, result.eyes, result.camera3d);
    validate_julibrot_view_member("from-to", result.outputs.from_to, result.from_to, result.camera3d);
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

static KeyframeConfig path_keyframe(int frame, const std::string &value)
{
    KeyframeConfig result;
    result.frame = frame;
    result.value = value;
    return result;
}

static KeyframeConfig path_keyframe(int frame, const std::string &value, Curve curve)
{
    KeyframeConfig result{path_keyframe(frame, value)};
    result.curve = curve;
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
        return {path_keyframe(0, path.value), path_keyframe(num_frames - 1, path.value, Curve::HOLD)};
    case PathKind::LINE:
        return {path_keyframe(0, path.from), path_keyframe(num_frames - 1, path.to, Curve::LINEAR)};
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

static void validate_percent_track(std::string_view name, const NumberTrackConfig &track, int num_frames)
{
    validate_number_track_keyframes(name, track.keys, num_frames);
    for (const NumberKeyframeConfig &key : track.keys)
    {
        if (key.value < 0.0 || key.value > 100.0)
        {
            throw std::runtime_error("Invalid config, '" + std::string{name} + "' value must be from 0 through 100");
        }
    }
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
    if (result.kind == TrackKind::CAMERA2D)
    {
        result.camera2d = load_camera2d_config(json, num_frames);
        result.parameter = result.camera2d->name;
        return result;
    }
    if (result.kind == TrackKind::ID_3D_VIEW)
    {
        result.id_3d_view = load_id_3d_view_config(json);
        result.parameter = result.id_3d_view->name;
        return result;
    }
    if (result.kind == TrackKind::JULIBROT_VIEW)
    {
        result.julibrot_view = load_julibrot_view_config(json);
        result.parameter = result.julibrot_view->name;
        return result;
    }

    result.mode = load_track_mode(json);
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

static LayerConfig load_layer_config(const Object &json, int num_frames)
{
    LayerConfig result;
    result.id = load_string(json, "id");
    result.source = load_named_file_par_set(json, "source");
    if (json.contains("opacity"))
    {
        result.opacity = load_number_track_config(json, "opacity");
        validate_percent_track("opacity", *result.opacity, num_frames);
    }
    result.write_when_hidden = load_optional_bool(json, "write-when-hidden").value_or(false);
    result.compose = load_compose_operator(json);
    result.tracks = load_tracks(json, "tracks", num_frames);
    return result;
}

static std::vector<LayerConfig> load_layers(const Object &json, int num_frames)
{
    const std::string key{"layers"};
    if (!json.contains(key) || !json.at(key).is_array())
    {
        throw std::runtime_error("Invalid config, missing array 'layers'");
    }

    std::vector<LayerConfig> result;
    for (const Object &item : json.at(key))
    {
        if (!item.is_object())
        {
            throw std::runtime_error("Invalid config, array 'layers' contains non-object value");
        }
        LayerConfig layer{load_layer_config(item, num_frames)};
        const auto is_id{[&](const LayerConfig &other) { return other.id == layer.id; }};
        if (std::find_if(result.begin(), result.end(), is_id) != result.end())
        {
            throw std::runtime_error("Invalid config, duplicate layer id '" + layer.id + "'");
        }
        result.emplace_back(layer);
    }
    if (result.empty())
    {
        throw std::runtime_error("Invalid config, layers must contain at least one layer");
    }
    return result;
}

Config read_config(std::string_view json_text)
{
    const Object json = parse_json(json_text);
    Config result;
    result.parameter_catalogs = load_string_array(json, "parameter-catalogs");
    result.output = load_output_config(json);
    result.video = load_string(json, "video");
    result.num_frames = load_int(json, "num-frames");

    if (json.contains("parallel"))
    {
        if (!json.at("parallel").is_number_integer())
        {
            throw std::runtime_error("Invalid config, 'parallel' is not a number");
        }
        result.parallel = json.at("parallel").get<int>();
    }
    if (json.contains("layers"))
    {
        if (json.contains("source") || json.contains("tracks"))
        {
            throw std::runtime_error("Invalid config, layers cannot be combined with top-level source or tracks");
        }
        result.layers = load_layers(json, result.num_frames);
        result.source = result.layers[0].source;
        result.tracks = result.layers[0].tracks;
    }
    else
    {
        result.source = load_named_file_par_set(json, "source");
        result.tracks = load_tracks(json, "tracks", result.num_frames);
    }
    return result;
}

} // namespace ParFile
