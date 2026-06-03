// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/Config.h>

#include <nlohmann/json.hpp>

#include <cstddef>
#include <optional>
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

static double load_double(const Object &json, std::string_view name)
{
    const std::string key{name};
    if (!json.contains(key) || !json.at(key).is_number())
    {
        throw std::runtime_error("Invalid config, missing number '" + std::string{name} + "'");
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
    return result;
}

static TrackConfig load_track_config(const Object &json)
{
    TrackConfig result;
    result.parameter = load_string(json, "parameter");
    result.kind = load_track_kind(json);
    result.mode = load_track_mode(json);
    if (result.kind == TrackKind::COLOR_MAP)
    {
        result.color_map = load_color_map_config(json);
    }
    if (result.mode == TrackMode::PWM)
    {
        result.pwm = load_pwm_config(json);
    }
    result.keys = load_keyframes(json, result.mode);
    return result;
}

static std::vector<TrackConfig> load_tracks(const Object &json, std::string_view name)
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
        result.emplace_back(load_track_config(item));
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
    result.tracks = load_tracks(json, "tracks");

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
