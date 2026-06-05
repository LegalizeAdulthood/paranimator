// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/Interpolant.h>

#include <ParFile/Config.h>
#include <ParFile/ParameterCatalog.h>

#include <gtest/gtest.h>

#include <initializer_list>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace
{

ParFile::ParameterMetadata metadata(const std::string &name, ParFile::ParameterType type,
    std::optional<double> min = {}, std::optional<double> max = {},
    ParFile::ExtrapolateMode extrapolate = ParFile::ExtrapolateMode::CLAMP,
    ParFile::Curve default_curve = ParFile::Curve::LINEAR)
{
    return {name, type, ParFile::ParameterFormat::SLASH, default_curve, extrapolate, min, max};
}

std::vector<ParFile::KeyframeConfig> keyframes(const std::string &from, const std::string &to, int num_steps)
{
    return {{0, from}, {num_steps - 1, to}};
}

std::vector<ParFile::KeyframeConfig> keyframes(
    const std::string &from, const std::string &to, ParFile::Curve curve, int num_steps)
{
    return {{0, from}, {num_steps - 1, to, curve}};
}

std::vector<ParFile::KeyframeConfig> integer_keyframes(int from, int to, ParFile::Curve curve, int num_steps)
{
    return {{0, from}, {num_steps - 1, to, curve}};
}

std::vector<ParFile::KeyframeConfig> number_array_keyframes(
    std::initializer_list<double> from, std::initializer_list<double> to, ParFile::Curve curve, int num_steps)
{
    using NumberArray = ParFile::KeyframeConfig::Value::NumberArray;

    return {{0, NumberArray{from.begin(), from.end()}}, {num_steps - 1, NumberArray{to.begin(), to.end()}, curve}};
}

ParFile::KeyframeConfig::Value::Array slash_array(std::string_view text)
{
    ParFile::KeyframeConfig::Value::Array result;
    std::size_t first{};
    while (first <= text.size())
    {
        const std::size_t next{text.find('/', first)};
        const std::size_t last{next == std::string_view::npos ? text.size() : next};
        result.emplace_back(text.substr(first, last - first));
        if (next == std::string_view::npos)
        {
            break;
        }
        first = next + 1U;
    }
    return result;
}

ParFile::KeyframeConfig bool_keyframe(int frame, bool value)
{
    ParFile::KeyframeConfig result;
    result.frame = frame;
    result.value = value;
    return result;
}

std::vector<ParFile::KeyframeConfig> bool_keyframes(bool from, bool to, int num_steps)
{
    return {bool_keyframe(0, from), bool_keyframe(num_steps - 1, to)};
}

ParFile::KeyframeConfig pwm_keyframe(int frame, double mix)
{
    ParFile::KeyframeConfig result;
    result.frame = frame;
    result.mix = mix;
    return result;
}

std::vector<ParFile::KeyframeConfig> pwm_keyframes(double from, double to, int num_steps)
{
    return {pwm_keyframe(0, from), pwm_keyframe(num_steps - 1, to)};
}

ParFile::ResolvedTrack resolved_track(const std::string &name, ParFile::ParameterType type,
    const std::vector<ParFile::KeyframeConfig> &keys, const std::string &base_value)
{
    return {name, metadata(name, type), base_value, keys, name, {}};
}

ParFile::ResolvedTrack resolved_track(const ParFile::ParameterMetadata &parameter_metadata,
    const std::vector<ParFile::KeyframeConfig> &keys, const std::string &base_value)
{
    return {parameter_metadata.name, parameter_metadata, base_value, keys, parameter_metadata.name, {}};
}

ParFile::ResolvedTrack resolved_params_track(const std::string &name, ParFile::ParameterType type,
    const std::vector<ParFile::KeyframeConfig> &keys, const std::string &base_value, std::vector<int> slots)
{
    return {name, metadata(name, type), base_value, keys, "params", std::move(slots)};
}

ParFile::PathConfig circle_path(const std::string &center, double radius, double turns = 1.0, double phase = 0.0)
{
    ParFile::PathConfig result;
    result.kind = ParFile::PathKind::CIRCLE;
    result.center = center;
    result.radius = radius;
    result.turns = turns;
    result.phase = phase;
    return result;
}

ParFile::PathConfig ellipse_path(
    const std::string &center, double x_radius, double y_radius, double turns = 1.0, double phase = 0.0)
{
    ParFile::PathConfig result;
    result.kind = ParFile::PathKind::ELLIPSE;
    result.center = center;
    result.x_radius = x_radius;
    result.y_radius = y_radius;
    result.turns = turns;
    result.phase = phase;
    return result;
}

ParFile::PathConfig lissajous_path(
    const std::string &center, double x_frequency, double y_frequency, double phase = 0.0)
{
    ParFile::PathConfig result;
    result.kind = ParFile::PathKind::LISSAJOUS;
    result.center = center;
    result.x_radius = 2.0;
    result.y_radius = 1.0;
    result.x_frequency = x_frequency;
    result.y_frequency = y_frequency;
    result.phase = phase;
    return result;
}

ParFile::PathConfig spiral_path(
    const std::string &center, double from_radius, double to_radius, double turns = 1.0, double phase = 0.0)
{
    ParFile::PathConfig result;
    result.kind = ParFile::PathKind::SPIRAL;
    result.center = center;
    result.from_radius = from_radius;
    result.to_radius = to_radius;
    result.turns = turns;
    result.phase = phase;
    return result;
}

ParFile::PathConfig bezier_path(std::vector<std::string> control_points)
{
    ParFile::PathConfig result;
    result.kind = ParFile::PathKind::BEZIER;
    result.control_points = std::move(control_points);
    return result;
}

ParFile::PathConfig catmull_rom_path(std::vector<std::string> control_points)
{
    ParFile::PathConfig result;
    result.kind = ParFile::PathKind::CATMULL_ROM;
    result.control_points = std::move(control_points);
    return result;
}

ParFile::ResolvedTrack resolved_path_track(
    const ParFile::ParameterMetadata &parameter_metadata, const ParFile::PathConfig &path)
{
    return {parameter_metadata.name, parameter_metadata, {}, {}, parameter_metadata.name, {},
        ParFile::TrackMode::KEYFRAMES, {}, path};
}

ParFile::ResolvedTrack resolved_params_path_track(
    const std::string &name, const ParFile::PathConfig &path, const std::string &base_value, std::vector<int> slots)
{
    return {name, metadata(name, ParFile::ParameterType::COMPLEX), base_value, {}, "params", std::move(slots),
        ParFile::TrackMode::KEYFRAMES, {}, path};
}

ParFile::ResolvedTrack resolved_pwm_track(const ParFile::ParameterMetadata &parameter_metadata, const std::string &a,
    const std::string &b, int window, double from, double to, int num_steps)
{
    ParFile::ResolvedTrack result{resolved_track(parameter_metadata, pwm_keyframes(from, to, num_steps), a)};
    result.mode = ParFile::TrackMode::PWM;
    result.pwm = ParFile::PwmConfig{ParFile::PwmEndpointConfig{a}, ParFile::PwmEndpointConfig{b}, window};
    return result;
}

ParFile::ResolvedTrack resolved_bool_pwm_track(const ParFile::ParameterMetadata &parameter_metadata,
    std::optional<ParFile::PwmEndpointConfig> a, std::optional<ParFile::PwmEndpointConfig> b, int window, double from,
    double to, int num_steps)
{
    ParFile::ResolvedTrack result{resolved_track(parameter_metadata, pwm_keyframes(from, to, num_steps), "false")};
    result.mode = ParFile::TrackMode::PWM;
    result.pwm = ParFile::PwmConfig{std::move(a), std::move(b), window};
    return result;
}

ParFile::ResolvedCamera2DValueTrack resolved_camera2d_value_track(const std::string &name, ParFile::ParameterType type,
    const std::vector<ParFile::KeyframeConfig> &keys, bool normalize = false)
{
    ParFile::ParameterMetadata parameter_metadata{metadata(name, type)};
    parameter_metadata.normalize = normalize;
    return {parameter_metadata, keys};
}

ParFile::ResolvedCamera2DValueTrack resolved_camera2d_path_value_track(
    const std::string &name, ParFile::ParameterType type, const ParFile::PathConfig &path, bool normalize = false)
{
    ParFile::ParameterMetadata parameter_metadata{metadata(name, type)};
    parameter_metadata.normalize = normalize;
    return {parameter_metadata, {}, path};
}

ParFile::ResolvedTrack resolved_camera2d_track(double aspect, const std::vector<ParFile::KeyframeConfig> &look_at_keys,
    const std::vector<ParFile::KeyframeConfig> &view_up_keys, const std::vector<ParFile::KeyframeConfig> &height_keys,
    ParFile::ParameterType output_type = ParFile::ParameterType::CORNERS,
    const std::string &output_parameter = "corners", double center_mag_x_mag_factor = 1.0,
    std::optional<std::vector<ParFile::KeyframeConfig>> skew_keys = {})
{
    ParFile::ResolvedCamera2DConfig camera2d;
    camera2d.aspect = aspect;
    camera2d.center_mag_x_mag_factor = center_mag_x_mag_factor;
    camera2d.look_at = resolved_camera2d_value_track("camera.look-at", ParFile::ParameterType::POINT2, look_at_keys);
    camera2d.view_up =
        resolved_camera2d_value_track("camera.view-up", ParFile::ParameterType::VECTOR2, view_up_keys, true);
    camera2d.height = resolved_camera2d_value_track("camera.height", ParFile::ParameterType::DOUBLE, height_keys);
    if (skew_keys)
    {
        camera2d.skew = resolved_camera2d_value_track("camera.skew", ParFile::ParameterType::DOUBLE, *skew_keys);
    }

    ParFile::ResolvedTrack result;
    result.parameter = "camera";
    result.metadata = metadata(output_parameter, output_type);
    result.base_value = output_type == ParFile::ParameterType::CENTER_MAG ? "-0.5/0/1" : "-3/-1/-2/2";
    result.output_parameter = output_parameter;
    result.camera2d = camera2d;
    return result;
}

ParFile::ResolvedTrack resolved_camera2d_eye_track(double aspect,
    const std::vector<ParFile::KeyframeConfig> &look_at_keys, const std::vector<ParFile::KeyframeConfig> &eye_keys,
    const std::vector<ParFile::KeyframeConfig> &height_keys,
    ParFile::ParameterType output_type = ParFile::ParameterType::CORNERS,
    const std::string &output_parameter = "corners", double center_mag_x_mag_factor = 1.0)
{
    ParFile::ResolvedCamera2DConfig camera2d;
    camera2d.aspect = aspect;
    camera2d.center_mag_x_mag_factor = center_mag_x_mag_factor;
    camera2d.look_at = resolved_camera2d_value_track("camera.look-at", ParFile::ParameterType::POINT2, look_at_keys);
    camera2d.eye = resolved_camera2d_value_track("camera.eye", ParFile::ParameterType::POINT2, eye_keys);
    camera2d.height = resolved_camera2d_value_track("camera.height", ParFile::ParameterType::DOUBLE, height_keys);

    ParFile::ResolvedTrack result;
    result.parameter = "camera";
    result.metadata = metadata(output_parameter, output_type);
    result.base_value = output_type == ParFile::ParameterType::CENTER_MAG ? "-0.5/0/1" : "-3/-1/-2/2";
    result.output_parameter = output_parameter;
    result.camera2d = camera2d;
    return result;
}

ParFile::ResolvedTrack resolved_camera2d_eye_path_track(double aspect,
    const std::vector<ParFile::KeyframeConfig> &look_at_keys, const ParFile::PathConfig &eye_path,
    const std::vector<ParFile::KeyframeConfig> &height_keys,
    ParFile::ParameterType output_type = ParFile::ParameterType::CORNERS,
    const std::string &output_parameter = "corners", double center_mag_x_mag_factor = 1.0)
{
    ParFile::ResolvedCamera2DConfig camera2d;
    camera2d.aspect = aspect;
    camera2d.center_mag_x_mag_factor = center_mag_x_mag_factor;
    camera2d.look_at = resolved_camera2d_value_track("camera.look-at", ParFile::ParameterType::POINT2, look_at_keys);
    camera2d.eye = resolved_camera2d_path_value_track("camera.eye", ParFile::ParameterType::POINT2, eye_path);
    camera2d.height = resolved_camera2d_value_track("camera.height", ParFile::ParameterType::DOUBLE, height_keys);

    ParFile::ResolvedTrack result;
    result.parameter = "camera";
    result.metadata = metadata(output_parameter, output_type);
    result.base_value = output_type == ParFile::ParameterType::CENTER_MAG ? "-0.5/0/1" : "-3/-1/-2/2";
    result.output_parameter = output_parameter;
    result.camera2d = camera2d;
    return result;
}

ParFile::ResolvedCamera3DValueTrack resolved_camera3d_value_track(const std::string &name, ParFile::ParameterType type,
    const std::vector<ParFile::KeyframeConfig> &keys, bool normalize = false)
{
    ParFile::ParameterMetadata parameter_metadata{metadata(name, type)};
    parameter_metadata.normalize = normalize;
    return {parameter_metadata, keys};
}

ParFile::ResolvedTrack resolved_camera3d_track(ParFile::Camera3DOutputKind output_kind, ParFile::ParameterType type,
    const std::string &output_parameter, const std::vector<ParFile::KeyframeConfig> &eye_keys,
    const std::vector<ParFile::KeyframeConfig> &look_at_keys, const std::vector<ParFile::KeyframeConfig> &view_up_keys,
    const std::string &base_value = {})
{
    ParFile::ResolvedCamera3DConfig camera3d;
    camera3d.output_kind = output_kind;
    camera3d.eye = resolved_camera3d_value_track("view.camera3d.eye", ParFile::ParameterType::POINT3, eye_keys);
    camera3d.look_at =
        resolved_camera3d_value_track("view.camera3d.look-at", ParFile::ParameterType::POINT3, look_at_keys);
    camera3d.view_up =
        resolved_camera3d_value_track("view.camera3d.view-up", ParFile::ParameterType::VECTOR3, view_up_keys, true);

    ParFile::ResolvedTrack result;
    result.parameter = "view.camera3d";
    result.metadata = metadata(output_parameter, type);
    result.base_value = base_value;
    result.output_parameter = output_parameter;
    result.camera3d = camera3d;
    return result;
}

ParFile::ParameterMetadata tuple_metadata(const std::string &name, int arity)
{
    ParFile::ParameterMetadata result{metadata(name, ParFile::ParameterType::NUMERIC_TUPLE)};
    result.arity = arity;
    return result;
}

ParFile::ParameterMetadata integer_tuple_metadata(const std::string &name, int arity)
{
    ParFile::ParameterMetadata result{metadata(name, ParFile::ParameterType::INTEGER_TUPLE)};
    result.arity = arity;
    return result;
}

ParFile::ParameterMetadata tuple_or_enum_metadata(const std::string &name, int arity)
{
    ParFile::ParameterMetadata result{metadata(name, ParFile::ParameterType::NUMERIC_TUPLE_OR_ENUM, {}, {},
        ParFile::ExtrapolateMode::CLAMP, ParFile::Curve::HOLD)};
    result.format = ParFile::ParameterFormat::SLASH;
    result.arity = arity;
    result.values = {"pixel"};
    return result;
}

ParFile::ParameterMetadata enum_metadata(const std::string &name)
{
    ParFile::ParameterMetadata result{
        metadata(name, ParFile::ParameterType::ENUM, {}, {}, ParFile::ExtrapolateMode::CLAMP, ParFile::Curve::HOLD)};
    result.format = ParFile::ParameterFormat::RAW;
    result.values = {"bof60", "zmag", "epsiloncross", "startrail"};
    return result;
}

ParFile::ParameterMetadata inside_metadata(const std::string &name)
{
    ParFile::ParameterMetadata result{metadata(
        name, ParFile::ParameterType::INSIDE, 0.0, 255.0, ParFile::ExtrapolateMode::CLAMP, ParFile::Curve::HOLD)};
    result.format = ParFile::ParameterFormat::RAW;
    result.values = {"maxiter", "zmag", "bof60", "bof61", "epsiloncross", "startrail", "period", "atan", "fmod"};
    return result;
}

ParFile::ParameterMetadata outside_metadata(const std::string &name)
{
    ParFile::ParameterMetadata result{metadata(
        name, ParFile::ParameterType::OUTSIDE, 0.0, 255.0, ParFile::ExtrapolateMode::CLAMP, ParFile::Curve::HOLD)};
    result.format = ParFile::ParameterFormat::RAW;
    result.values = {"iter", "real", "imag", "mult", "summ", "atan", "fmod", "tdis"};
    return result;
}

ParFile::ParameterMetadata integer_or_enum_metadata(const std::string &name)
{
    ParFile::ParameterMetadata result{metadata(name, ParFile::ParameterType::INTEGER_OR_ENUM, 0.0, 255.0,
        ParFile::ExtrapolateMode::CLAMP, ParFile::Curve::HOLD)};
    result.format = ParFile::ParameterFormat::RAW;
    result.values = {"normal", "show", "yes", "no"};
    return result;
}

ParFile::ParameterMetadata yes_no_metadata(const std::string &name)
{
    ParFile::ParameterMetadata result{
        metadata(name, ParFile::ParameterType::YES_NO, {}, {}, ParFile::ExtrapolateMode::CLAMP, ParFile::Curve::HOLD)};
    result.format = ParFile::ParameterFormat::RAW;
    return result;
}

ParFile::ParameterMetadata string_metadata(const std::string &name)
{
    ParFile::ParameterMetadata result{
        metadata(name, ParFile::ParameterType::STRING, {}, {}, ParFile::ExtrapolateMode::CLAMP, ParFile::Curve::HOLD)};
    result.format = ParFile::ParameterFormat::RAW;
    return result;
}

ParFile::ParameterMetadata miim_metadata()
{
    ParFile::ParameterMetadata result{
        metadata("miim", ParFile::ParameterType::MIIM, {}, {}, ParFile::ExtrapolateMode::CLAMP, ParFile::Curve::HOLD)};
    result.format = ParFile::ParameterFormat::SLASH;
    return result;
}

ParFile::ParameterMetadata potential_metadata()
{
    ParFile::ParameterMetadata result{metadata("potential", ParFile::ParameterType::POTENTIAL, {}, {},
        ParFile::ExtrapolateMode::CLAMP, ParFile::Curve::LINEAR)};
    result.format = ParFile::ParameterFormat::SLASH;
    return result;
}

std::vector<std::string> id_functions();

ParFile::ParameterMetadata function_list_metadata(const std::string &name)
{
    ParFile::ParameterMetadata result{name, ParFile::ParameterType::FUNCTION_LIST, ParFile::ParameterFormat::SLASH_LIST,
        ParFile::Curve::HOLD, ParFile::ExtrapolateMode::CLAMP, {}, {}, id_functions()};
    return result;
}

std::vector<ParFile::KeyframeConfig> function_list_keyframes(
    const std::string &from, const std::string &to, int num_steps)
{
    std::vector<ParFile::KeyframeConfig> result{keyframes(from, to, num_steps)};
    result[0].value = slash_array(from);
    result[1].value = slash_array(to);
    return result;
}

std::vector<std::string> id_functions()
{
    return {"sin", "cos", "tan", "cotan", "sinh", "cosh", "tanh", "cotanh", "exp", "log", "sqr", "recip", "ident",
        "cosxx", "flip", "conj", "zero", "one", "asin", "asinh", "acos", "acosh", "atan", "atanh", "sqrt", "abs",
        "cabs", "floor", "ceil", "trunc", "round"};
}

ParFile::ResolvedTrack resolved_function_track(
    const std::string &name, const std::vector<ParFile::KeyframeConfig> &keys, const std::string &base_value, int slot)
{
    ParFile::ParameterMetadata function_metadata{name, ParFile::ParameterType::ENUM, ParFile::ParameterFormat::RAW,
        ParFile::Curve::HOLD, ParFile::ExtrapolateMode::CLAMP, {}, {}, id_functions()};
    return {name, function_metadata, base_value, keys, "function", {slot}};
}

ParFile::ResolvedTrack resolved_function_pwm_track(const std::string &name, const std::string &a, const std::string &b,
    int window, double from, double to, int num_steps, const std::string &base_value, int slot)
{
    ParFile::ResolvedTrack result{resolved_function_track(name, pwm_keyframes(from, to, num_steps), base_value, slot)};
    result.mode = ParFile::TrackMode::PWM;
    result.pwm = ParFile::PwmConfig{ParFile::PwmEndpointConfig{a}, ParFile::PwmEndpointConfig{b}, window};
    return result;
}

ParFile::InterpolantPtr create_interpolant(
    const std::string &name, ParFile::ParameterType type, const std::string &from, const std::string &to, int num_steps)
{
    return ParFile::create_interpolant(resolved_track(name, type, keyframes(from, to, num_steps), from), num_steps);
}

ParFile::InterpolantPtr create_interpolant(const std::string &name, ParFile::ParameterType type,
    const std::vector<ParFile::KeyframeConfig> &keys, int num_steps)
{
    return ParFile::create_interpolant(resolved_track(name, type, keys, keys[0].value), num_steps);
}

ParFile::InterpolantPtr create_interpolant(
    const ParFile::ParameterMetadata &parameter_metadata, const std::string &from, const std::string &to, int num_steps)
{
    const std::vector<ParFile::KeyframeConfig> keys{keyframes(from, to, num_steps)};
    return ParFile::create_interpolant(resolved_track(parameter_metadata, keys, from), num_steps);
}

ParFile::InterpolantPtr create_interpolant(const ParFile::ParameterMetadata &parameter_metadata,
    const std::string &from, const std::string &to, ParFile::Curve curve, int num_steps)
{
    const std::vector<ParFile::KeyframeConfig> keys{keyframes(from, to, curve, num_steps)};
    return ParFile::create_interpolant(resolved_track(parameter_metadata, keys, from), num_steps);
}

ParFile::InterpolantPtr create_tuple_interpolant(const ParFile::ParameterMetadata &parameter_metadata,
    std::initializer_list<double> from, std::initializer_list<double> to, ParFile::Curve curve, int num_steps)
{
    const std::vector<ParFile::KeyframeConfig> keys{number_array_keyframes(from, to, curve, num_steps)};
    return ParFile::create_interpolant(
        resolved_track(parameter_metadata, keys, ParFile::keyframe_value_text(keys[0].value)), num_steps);
}

} // namespace

TEST(TestInterpolant, centerMag)
{
    const std::string from{"-0.5/0.0/1.0"};
    const std::string to{"-0.5/0.0/10.0"};
    const int num_steps{3};

    ParFile::InterpolantPtr interpolant{
        create_interpolant("center-mag", ParFile::ParameterType::CENTER_MAG, from, to, num_steps)};

    ASSERT_TRUE(interpolant);
    ASSERT_EQ("center-mag", interpolant->name());
}

TEST(TestInterpolant, centerMagFrom)
{
    const std::string from{"-0.5/0/1"};
    const std::string to{"-0.5/0/10"};
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{
        create_interpolant("center-mag", ParFile::ParameterType::CENTER_MAG, from, to, num_steps)};

    const std::string value{interpolant->step()};

    EXPECT_EQ(from, value);
}

TEST(TestInterpolant, centerMagMagnificationIsGeometric)
{
    const std::string from{"-0.5/0.0/1.0"};
    const std::string to{"-0.5/0.0/10.0"};
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{
        create_interpolant("center-mag", ParFile::ParameterType::CENTER_MAG, from, to, num_steps)};
    static_cast<void>(interpolant->step());

    const std::string value{interpolant->step()};

    EXPECT_EQ("-0.5/0/3.16228", value);
}

TEST(TestInterpolant, centerMagCenterFraction)
{
    const std::string from{"-1/-2/1"};
    const std::string to{"1/2/1"};
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{
        create_interpolant("center-mag", ParFile::ParameterType::CENTER_MAG, from, to, num_steps)};
    static_cast<void>(interpolant->step());

    const std::string value{interpolant->step()};

    EXPECT_EQ("0/0/1", value);
}

TEST(TestInterpolant, centerMagExtendedValuesInterpolate)
{
    const std::string from{"0/0/1/1/0/0"};
    const std::string to{"0/0/1/4/90/10"};
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{
        create_interpolant("center-mag", ParFile::ParameterType::CENTER_MAG, from, to, num_steps)};
    static_cast<void>(interpolant->step());

    const std::string value{interpolant->step()};

    EXPECT_EQ("0/0/1/2/45/5", value);
}

TEST(TestInterpolant, centerMagXMagFactorWritesFourValues)
{
    const std::string from{"0/0/1/2"};
    const std::string to{"0/0/1/8"};
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{
        create_interpolant("center-mag", ParFile::ParameterType::CENTER_MAG, from, to, num_steps)};
    static_cast<void>(interpolant->step());

    const std::string value{interpolant->step()};

    EXPECT_EQ("0/0/1/4", value);
}

TEST(TestInterpolant, centerMagInvalidArityRejected)
{
    const int num_steps{3};

    EXPECT_THROW(create_interpolant("center-mag", ParFile::ParameterType::CENTER_MAG, "0/0", "0/0", num_steps),
        std::runtime_error);
    EXPECT_THROW(create_interpolant(
                     "center-mag", ParFile::ParameterType::CENTER_MAG, "0/0/1/1/0/0/0", "0/0/1/1/0/0/0", num_steps),
        std::runtime_error);
}

TEST(TestInterpolant, centerMagTo)
{
    const std::string from{"-0.5/0/1"};
    const std::string to{"-0.5/0/10"};
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{
        create_interpolant("center-mag", ParFile::ParameterType::CENTER_MAG, from, to, num_steps)};
    static_cast<void>(interpolant->step());
    static_cast<void>(interpolant->step());

    const std::string value{interpolant->step()};

    EXPECT_EQ(to, value);
}

TEST(TestInterpolant, corners)
{
    const std::string from{"-3.570101/-0.0499383/-1.320061/1.320061"};
    const std::string to{"-1.8101050271/-1.8099342992/-6.37250230799e-05/6.4320896203e-05"};
    const int num_steps{3};

    ParFile::InterpolantPtr interpolant{
        create_interpolant("corners", ParFile::ParameterType::CORNERS, from, to, num_steps)};

    ASSERT_TRUE(interpolant);
    ASSERT_EQ("corners", interpolant->name());
}

TEST(TestInterpolant, cornersFrom)
{
    const std::string from{"-3.570101/-0.0499383/-1.320061/1.320061"};
    const std::string to{"-1.8101050271/-1.8099342992/-6.37250230799e-05/6.4320896203e-05"};
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{
        create_interpolant("corners", ParFile::ParameterType::CORNERS, from, to, num_steps)};

    const std::string value{interpolant->step()};

    EXPECT_EQ(from, value);
}

TEST(TestInterpolant, cornersFraction)
{
    const std::string from{"-3.570101/-0.0499383/-1.320061/1.320061"};
    const std::string to{"-1.8101050271/-1.8099342992/-6.37250230799e-05/6.4320896203e-05"};
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{
        create_interpolant("corners", ParFile::ParameterType::CORNERS, from, to, num_steps)};
    static_cast<void>(interpolant->step());

    const std::string value{interpolant->step()};

    EXPECT_EQ("-2.69010301355/-0.9299362996/-0.660062362512/0.660062660448", value);
}

TEST(TestInterpolant, cornersTo)
{
    const std::string from{"-3.570101/-0.0499383/-1.320061/1.320061"};
    const std::string to{"-1.8101050271/-1.8099342992/-6.37250230799e-05/6.4320896203e-05"};
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{
        create_interpolant("corners", ParFile::ParameterType::CORNERS, from, to, num_steps)};
    static_cast<void>(interpolant->step());
    static_cast<void>(interpolant->step());

    const std::string value{interpolant->step()};

    EXPECT_EQ(to, value);
}

TEST(TestInterpolant, cornersSixValueFraction)
{
    const std::string from{"0/10/0/10/1/2"};
    const std::string to{"10/20/10/20/3/4"};
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{
        create_interpolant("corners", ParFile::ParameterType::CORNERS, from, to, num_steps)};
    static_cast<void>(interpolant->step());

    const std::string value{interpolant->step()};

    EXPECT_EQ("5/15/5/15/2/3", value);
}

TEST(TestInterpolant, cornersMismatchedArityRejected)
{
    const std::string from{"0/10/0/10"};
    const std::string to{"10/20/10/20/3/4"};
    const int num_steps{3};

    EXPECT_THROW(
        create_interpolant("corners", ParFile::ParameterType::CORNERS, from, to, num_steps), std::runtime_error);
}

TEST(TestInterpolant, cornersInvalidArityRejected)
{
    const std::string from{"0/10/0/10/1"};
    const std::string to{"10/20/10/20/3"};
    const int num_steps{3};

    EXPECT_THROW(
        create_interpolant("corners", ParFile::ParameterType::CORNERS, from, to, num_steps), std::runtime_error);
}

TEST(TestInterpolant, camera2dAxisAlignedWritesFourValueCorners)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{
        ParFile::create_interpolant(resolved_camera2d_track(0.5, keyframes("0/0", "0/0", num_steps),
                                        keyframes("0/1", "0/1", num_steps), keyframes("4", "4", num_steps)),
            num_steps)};

    ASSERT_TRUE(interpolant);
    EXPECT_EQ("corners", interpolant->name());
    EXPECT_EQ("-1/1/-2/2", interpolant->step());
}

TEST(TestInterpolant, camera2dRotatedWritesThirdCorner)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{
        ParFile::create_interpolant(resolved_camera2d_track(0.5, keyframes("0/0", "0/0", num_steps),
                                        keyframes("1/1", "1/1", num_steps), keyframes("4", "4", num_steps)),
            num_steps)};

    ASSERT_TRUE(interpolant);
    EXPECT_EQ("0.707106781187/-0.707106781187/-2.12132034356/2.12132034356/-2.12132034356/"
              "-0.707106781187",
        interpolant->step());
}

TEST(TestInterpolant, camera2dEyeDerivesViewUp)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{
        ParFile::create_interpolant(resolved_camera2d_eye_track(0.5, keyframes("0/0", "0/0", num_steps),
                                        keyframes("1/0", "1/0", num_steps), keyframes("4", "4", num_steps)),
            num_steps)};

    ASSERT_TRUE(interpolant);
    EXPECT_EQ("2/-2/-1/1/-2/1", interpolant->step());
}

TEST(TestInterpolant, camera2dEyeCircleRotatesCenterMag)
{
    const int num_steps{5};
    ParFile::InterpolantPtr interpolant{
        ParFile::create_interpolant(resolved_camera2d_eye_path_track(4.0 / 3.0, keyframes("0/0", "0/0", num_steps),
                                        circle_path("0/0", 1.0, 1.0, 90.0), keyframes("3", "3", num_steps),
                                        ParFile::ParameterType::CENTER_MAG, "center-mag"),
            num_steps)};

    ASSERT_TRUE(interpolant);
    EXPECT_EQ("0/0/1", interpolant->step());
    EXPECT_EQ("0/0/1/1/-90", interpolant->step());
    EXPECT_EQ("0/0/1/1/180", interpolant->step());
    EXPECT_EQ("0/0/1/1/90", interpolant->step());
    EXPECT_EQ("0/0/1", interpolant->step());
}

TEST(TestInterpolant, camera2dEyeCircleWritesRotatedCorners)
{
    const int num_steps{5};
    ParFile::InterpolantPtr interpolant{
        ParFile::create_interpolant(resolved_camera2d_eye_path_track(1.0, keyframes("0/0", "0/0", num_steps),
                                        circle_path("0/0", 1.0, 1.0, 90.0), keyframes("2", "2", num_steps)),
            num_steps)};

    ASSERT_TRUE(interpolant);
    EXPECT_EQ("-1/1/-1/1", interpolant->step());
    EXPECT_EQ("-1/1/1/-1/1/-1", interpolant->step());
    EXPECT_EQ("1/-1/1/-1/1/1", interpolant->step());
    EXPECT_EQ("1/-1/-1/1/-1/1", interpolant->step());
    EXPECT_EQ("-1/1/-1/1", interpolant->step());
}

TEST(TestInterpolant, camera2dEyeEqualLookAtRejected)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{
        ParFile::create_interpolant(resolved_camera2d_eye_track(0.5, keyframes("0/0", "0/0", num_steps),
                                        keyframes("0/0", "1/0", num_steps), keyframes("4", "4", num_steps)),
            num_steps)};

    ASSERT_TRUE(interpolant);
    EXPECT_THROW(static_cast<void>(interpolant->step()), std::runtime_error);
}

TEST(TestInterpolant, camera2dNormalizesViewUp)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{
        ParFile::create_interpolant(resolved_camera2d_track(0.5, keyframes("0/0", "0/0", num_steps),
                                        keyframes("0/2", "0/2", num_steps), keyframes("4", "4", num_steps)),
            num_steps)};

    ASSERT_TRUE(interpolant);
    EXPECT_EQ("-1/1/-2/2", interpolant->step());
}

TEST(TestInterpolant, camera2dHeightSupportsGeometricCurve)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant(
        resolved_camera2d_track(0.5, keyframes("0/0", "0/0", num_steps), keyframes("0/1", "0/1", num_steps),
            keyframes("4", "2", ParFile::Curve::GEOMETRIC, num_steps)),
        num_steps)};

    ASSERT_TRUE(interpolant);
    static_cast<void>(interpolant->step());
    EXPECT_EQ("-0.707106781187/0.707106781187/-1.41421356237/1.41421356237", interpolant->step());
}

TEST(TestInterpolant, camera2dAxisAlignedWritesCenterMag)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant(
        resolved_camera2d_track(4.0 / 3.0, keyframes("-0.5/0", "-0.25/0.5", num_steps),
            keyframes("0/1", "0/1", num_steps), keyframes("3", "1.5", ParFile::Curve::GEOMETRIC, num_steps),
            ParFile::ParameterType::CENTER_MAG, "center-mag"),
        num_steps)};

    ASSERT_TRUE(interpolant);
    EXPECT_EQ("center-mag", interpolant->name());
    EXPECT_EQ("-0.5/0/1", interpolant->step());
    EXPECT_EQ("-0.375/0.25/1.41421356237", interpolant->step());
    EXPECT_EQ("-0.25/0.5/2", interpolant->step());
}

TEST(TestInterpolant, camera2dCenterMagMagnificationUsesAspect)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant(
        resolved_camera2d_track(2.0, keyframes("0/0", "0/0", num_steps), keyframes("0/1", "0/1", num_steps),
            keyframes("4", "4", num_steps), ParFile::ParameterType::CENTER_MAG, "center-mag"),
        num_steps)};

    ASSERT_TRUE(interpolant);
    EXPECT_EQ("0/0/0.5", interpolant->step());
}

TEST(TestInterpolant, camera2dRotatedWritesCenterMagRotation)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant(
        resolved_camera2d_track(4.0 / 3.0, keyframes("0/0", "0/0", num_steps), keyframes("1/0", "1/0", num_steps),
            keyframes("3", "3", num_steps), ParFile::ParameterType::CENTER_MAG, "center-mag"),
        num_steps)};

    ASSERT_TRUE(interpolant);
    EXPECT_EQ("0/0/1/1/90", interpolant->step());
}

TEST(TestInterpolant, camera2dCenterMagWritesXMagFactor)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant(
        resolved_camera2d_track(2.0 / 3.0, keyframes("0/0", "0/0", num_steps), keyframes("0/1", "0/1", num_steps),
            keyframes("3", "3", num_steps), ParFile::ParameterType::CENTER_MAG, "center-mag", 2.0),
        num_steps)};

    ASSERT_TRUE(interpolant);
    EXPECT_EQ("0/0/2/2", interpolant->step());
}

TEST(TestInterpolant, camera2dSkewWritesCenterMag)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant(
        resolved_camera2d_track(4.0 / 3.0, keyframes("0/0", "0/0", num_steps), keyframes("0/1", "0/1", num_steps),
            keyframes("3", "3", num_steps), ParFile::ParameterType::CENTER_MAG, "center-mag", 1.0,
            std::optional<std::vector<ParFile::KeyframeConfig>>{keyframes("0", "10", num_steps)}),
        num_steps)};

    ASSERT_TRUE(interpolant);
    EXPECT_EQ("0/0/1", interpolant->step());
    EXPECT_EQ("0/0/1/1/0/5", interpolant->step());
    EXPECT_EQ("0/0/1/1/0/10", interpolant->step());
}

TEST(TestInterpolant, camera2dSkewWritesSixValueCorners)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant(
        resolved_camera2d_track(0.5, keyframes("0/0", "0/0", num_steps), keyframes("0/1", "0/1", num_steps),
            keyframes("4", "4", num_steps), ParFile::ParameterType::CORNERS, "corners", 1.0,
            std::optional<std::vector<ParFile::KeyframeConfig>>{keyframes("0", "10", num_steps)}),
        num_steps)};

    ASSERT_TRUE(interpolant);
    EXPECT_EQ("-1/1/-2/2", interpolant->step());
    EXPECT_EQ("-0.825022672948/0.825022672948/-2/2/-1.17497732705/-2", interpolant->step());
    EXPECT_EQ("-0.647346038583/0.647346038583/-2/2/-1.35265396142/-2", interpolant->step());
}

TEST(TestInterpolant, camera2dInvalidSkewRejected)
{
    const int num_steps{3};
    EXPECT_THROW(
        ParFile::create_interpolant(
            resolved_camera2d_track(0.5, keyframes("0/0", "0/0", num_steps), keyframes("0/1", "0/1", num_steps),
                keyframes("4", "4", num_steps), ParFile::ParameterType::CORNERS, "corners", 1.0,
                std::optional<std::vector<ParFile::KeyframeConfig>>{keyframes("nan", "nan", num_steps)}),
            num_steps),
        std::runtime_error);
}

TEST(TestInterpolant, camera3dRotationMapsCenteredEyeOrbit)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant(
        resolved_camera3d_track(ParFile::Camera3DOutputKind::ID_ROTATION, ParFile::ParameterType::NUMERIC_TUPLE,
            "rotation", keyframes("0/0/10", "10/0/0", num_steps), keyframes("0/0/0", "0/0/0", num_steps),
            keyframes("0/2/0", "0/2/0", num_steps)),
        num_steps)};

    ASSERT_TRUE(interpolant);
    EXPECT_EQ("0/0/0", interpolant->step());
    EXPECT_EQ("0/-45/0", interpolant->step());
    EXPECT_EQ("0/-90/0", interpolant->step());
}

TEST(TestInterpolant, camera3dPerspectiveUsesEyeDistance)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant(
        resolved_camera3d_track(ParFile::Camera3DOutputKind::ID_PERSPECTIVE, ParFile::ParameterType::INTEGER,
            "perspective", keyframes("0/0/24", "0/0/12", num_steps), keyframes("0/0/0", "0/0/0", num_steps),
            keyframes("0/1/0", "0/1/0", num_steps)),
        num_steps)};

    ASSERT_TRUE(interpolant);
    EXPECT_EQ("24", interpolant->step());
    EXPECT_EQ("18", interpolant->step());
    EXPECT_EQ("12", interpolant->step());
}

TEST(TestInterpolant, camera3dJulibrotGeometryUsesEyeDistance)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant(
        resolved_camera3d_track(ParFile::Camera3DOutputKind::JULIBROT_GEOMETRY, ParFile::ParameterType::NUMERIC_TUPLE,
            "julibrot3d", keyframes("0/0/24", "0/0/12", num_steps), keyframes("0/0/0", "0/0/0", num_steps),
            keyframes("0/1/0", "0/1/0", num_steps), "128/8/8/7/10/24"),
        num_steps)};

    ASSERT_TRUE(interpolant);
    EXPECT_EQ("128/8/8/7/10/24", interpolant->step());
    EXPECT_EQ("128/8/8/7/10/18", interpolant->step());
    EXPECT_EQ("128/8/8/7/10/12", interpolant->step());
}

TEST(TestInterpolant, camera3dEyeEqualLookAtRejected)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant(
        resolved_camera3d_track(ParFile::Camera3DOutputKind::ID_ROTATION, ParFile::ParameterType::NUMERIC_TUPLE,
            "rotation", keyframes("0/0/0", "0/0/10", num_steps), keyframes("0/0/0", "0/0/0", num_steps),
            keyframes("0/1/0", "0/1/0", num_steps)),
        num_steps)};

    ASSERT_TRUE(interpolant);
    EXPECT_THROW(static_cast<void>(interpolant->step()), std::runtime_error);
}

TEST(TestInterpolant, camera3dParallelViewUpRejected)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant(
        resolved_camera3d_track(ParFile::Camera3DOutputKind::ID_ROTATION, ParFile::ParameterType::NUMERIC_TUPLE,
            "rotation", keyframes("0/0/10", "0/0/10", num_steps), keyframes("0/0/0", "0/0/0", num_steps),
            keyframes("0/0/-1", "0/0/-1", num_steps)),
        num_steps)};

    ASSERT_TRUE(interpolant);
    EXPECT_THROW(static_cast<void>(interpolant->step()), std::runtime_error);
}

TEST(TestInterpolant, camera3dRollRejected)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant(
        resolved_camera3d_track(ParFile::Camera3DOutputKind::ID_ROTATION, ParFile::ParameterType::NUMERIC_TUPLE,
            "rotation", keyframes("0/0/10", "0/0/10", num_steps), keyframes("0/0/0", "0/0/0", num_steps),
            keyframes("1/0/0", "1/0/0", num_steps)),
        num_steps)};

    ASSERT_TRUE(interpolant);
    EXPECT_THROW(static_cast<void>(interpolant->step()), std::runtime_error);
}

TEST(TestInterpolant, integerFrom)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{
        create_interpolant("maxiter", ParFile::ParameterType::INTEGER, "100", "200", num_steps)};

    const std::string value{interpolant->step()};

    EXPECT_EQ("100", value);
}

TEST(TestInterpolant, integerTo)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{
        create_interpolant("maxiter", ParFile::ParameterType::INTEGER, "100", "200", num_steps)};
    static_cast<void>(interpolant->step());
    static_cast<void>(interpolant->step());

    const std::string value{interpolant->step()};

    EXPECT_EQ("200", value);
}

TEST(TestInterpolant, integerFraction)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{
        create_interpolant("maxiter", ParFile::ParameterType::INTEGER, "100", "200", num_steps)};
    static_cast<void>(interpolant->step());

    const std::string value{interpolant->step()};

    EXPECT_EQ("150", value);
}

TEST(TestInterpolant, integerRoundingIsStable)
{
    const int num_steps{4};
    ParFile::InterpolantPtr interpolant{
        create_interpolant("maxiter", ParFile::ParameterType::INTEGER, "0", "2", num_steps)};
    static_cast<void>(interpolant->step());

    const std::string value{interpolant->step()};

    EXPECT_EQ("1", value);
}

TEST(TestInterpolant, integerClampExtrapolation)
{
    const int num_steps{4};
    const std::vector<ParFile::KeyframeConfig> keys{{1, "10"}, {2, "20"}};
    ParFile::InterpolantPtr interpolant{
        create_interpolant("maxiter", ParFile::ParameterType::INTEGER, keys, num_steps)};

    EXPECT_EQ("10", interpolant->step());
    EXPECT_EQ("10", interpolant->step());
    EXPECT_EQ("20", interpolant->step());
    EXPECT_EQ("20", interpolant->step());
}

TEST(TestInterpolant, integerBaseExtrapolation)
{
    const int num_steps{4};
    const std::vector<ParFile::KeyframeConfig> keys{{1, "100"}, {2, "200"}};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant(
        resolved_track(
            metadata("maxiter", ParFile::ParameterType::INTEGER, {}, {}, ParFile::ExtrapolateMode::BASE), keys, "678"),
        num_steps)};

    EXPECT_EQ("678", interpolant->step());
    EXPECT_TRUE(interpolant->has_value());
    EXPECT_EQ("100", interpolant->step());
    EXPECT_TRUE(interpolant->has_value());
    EXPECT_EQ("200", interpolant->step());
    EXPECT_TRUE(interpolant->has_value());
    EXPECT_EQ("678", interpolant->step());
    EXPECT_TRUE(interpolant->has_value());
}

TEST(TestInterpolant, integerCycleExtrapolation)
{
    const int num_steps{5};
    const std::vector<ParFile::KeyframeConfig> keys{{1, "100"}, {3, "300"}};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant(
        resolved_track(
            metadata("maxiter", ParFile::ParameterType::INTEGER, {}, {}, ParFile::ExtrapolateMode::CYCLE), keys, "678"),
        num_steps)};

    EXPECT_EQ("300", interpolant->step());
    EXPECT_EQ("100", interpolant->step());
    EXPECT_EQ("200", interpolant->step());
    EXPECT_EQ("300", interpolant->step());
    EXPECT_EQ("100", interpolant->step());
}

TEST(TestInterpolant, integerHoldCurve)
{
    const int num_steps{4};
    ParFile::InterpolantPtr interpolant{create_interpolant("maxiter", ParFile::ParameterType::INTEGER,
        keyframes("100", "200", ParFile::Curve::HOLD, num_steps), num_steps)};

    EXPECT_EQ("100", interpolant->step());
    EXPECT_EQ("100", interpolant->step());
    EXPECT_EQ("100", interpolant->step());
    EXPECT_EQ("200", interpolant->step());
}

TEST(TestInterpolant, integerStepCurve)
{
    const int num_steps{4};
    ParFile::InterpolantPtr interpolant{create_interpolant("maxiter", ParFile::ParameterType::INTEGER,
        keyframes("100", "200", ParFile::Curve::STEP, num_steps), num_steps)};

    EXPECT_EQ("100", interpolant->step());
    EXPECT_EQ("100", interpolant->step());
    EXPECT_EQ("100", interpolant->step());
    EXPECT_EQ("200", interpolant->step());
}

TEST(TestInterpolant, integerGeometricCurveRejected)
{
    const int num_steps{4};

    EXPECT_THROW(create_interpolant("maxiter", ParFile::ParameterType::INTEGER,
                     keyframes("100", "200", ParFile::Curve::GEOMETRIC, num_steps), num_steps),
        std::runtime_error);
}

TEST(TestInterpolant, doubleFraction)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{
        create_interpolant("bailout", ParFile::ParameterType::DOUBLE, "1.25", "2.5", num_steps)};
    static_cast<void>(interpolant->step());

    const std::string value{interpolant->step()};

    EXPECT_EQ("1.875", value);
}

TEST(TestInterpolant, doublePreservesFractionalEndpoints)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{
        create_interpolant("bailout", ParFile::ParameterType::DOUBLE, "1.25", "2.5", num_steps)};

    EXPECT_EQ("1.25", interpolant->step());
    static_cast<void>(interpolant->step());
    EXPECT_EQ("2.5", interpolant->step());
}

TEST(TestInterpolant, doubleMinimumRejected)
{
    const int num_steps{3};

    EXPECT_THROW(create_interpolant(metadata("bailout", ParFile::ParameterType::DOUBLE, 1.0), "0.5", "2.5", num_steps),
        std::runtime_error);
}

TEST(TestInterpolant, doubleMaximumRejected)
{
    const int num_steps{3};

    EXPECT_THROW(
        create_interpolant(metadata("bailout", ParFile::ParameterType::DOUBLE, {}, 2.0), "1.25", "2.5", num_steps),
        std::runtime_error);
}

TEST(TestInterpolant, doubleOmitExtrapolation)
{
    const int num_steps{4};
    const std::vector<ParFile::KeyframeConfig> keys{{1, "1.5"}, {2, "2.5"}};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant(
        resolved_track(
            metadata("bailout", ParFile::ParameterType::DOUBLE, {}, {}, ParFile::ExtrapolateMode::OMIT), keys, "1.25"),
        num_steps)};

    static_cast<void>(interpolant->step());
    EXPECT_FALSE(interpolant->has_value());
    EXPECT_EQ("1.5", interpolant->step());
    EXPECT_TRUE(interpolant->has_value());
    EXPECT_EQ("2.5", interpolant->step());
    EXPECT_TRUE(interpolant->has_value());
    static_cast<void>(interpolant->step());
    EXPECT_FALSE(interpolant->has_value());
}

TEST(TestInterpolant, doublePingPongExtrapolation)
{
    const int num_steps{5};
    const std::vector<ParFile::KeyframeConfig> keys{{1, "1"}, {3, "3"}};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant(
        resolved_track(metadata("bailout", ParFile::ParameterType::DOUBLE, {}, {}, ParFile::ExtrapolateMode::PING_PONG),
            keys, "1.25"),
        num_steps)};

    EXPECT_EQ("2", interpolant->step());
    EXPECT_EQ("1", interpolant->step());
    EXPECT_EQ("2", interpolant->step());
    EXPECT_EQ("3", interpolant->step());
    EXPECT_EQ("2", interpolant->step());
}

TEST(TestInterpolant, numericTupleTwoValueFraction)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{create_tuple_interpolant(
        tuple_metadata("xyshift", 2), {0.0, 1.0}, {10.0, 11.0}, ParFile::Curve::LINEAR, num_steps)};

    EXPECT_EQ("0/1", interpolant->step());
    EXPECT_EQ("5/6", interpolant->step());
    EXPECT_EQ("10/11", interpolant->step());
}

TEST(TestInterpolant, numericTupleThreeValueFraction)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{create_tuple_interpolant(
        tuple_metadata("lightsource", 3), {0.0, 10.0, 20.0}, {10.0, 20.0, 30.0}, ParFile::Curve::LINEAR, num_steps)};

    EXPECT_EQ("0/10/20", interpolant->step());
    EXPECT_EQ("5/15/25", interpolant->step());
    EXPECT_EQ("10/20/30", interpolant->step());
}

TEST(TestInterpolant, numericTupleWrongArityRejected)
{
    const int num_steps{3};

    EXPECT_THROW(create_tuple_interpolant(
                     tuple_metadata("xyshift", 3), {0.0, 1.0}, {10.0, 11.0}, ParFile::Curve::LINEAR, num_steps),
        std::runtime_error);
}

TEST(TestInterpolant, numericTupleMissingArityRejected)
{
    const int num_steps{3};
    const ParFile::ParameterMetadata parameter_metadata{metadata("xyshift", ParFile::ParameterType::NUMERIC_TUPLE)};

    EXPECT_THROW(
        create_tuple_interpolant(parameter_metadata, {0.0, 1.0}, {10.0, 11.0}, ParFile::Curve::LINEAR, num_steps),
        std::runtime_error);
}

TEST(TestInterpolant, numericTupleStringKeyframesRejected)
{
    const int num_steps{3};

    EXPECT_THROW(create_interpolant(tuple_metadata("xyshift", 2), "0/1", "10/11", num_steps), std::runtime_error);
}

TEST(TestInterpolant, integerTupleRoundsComponents)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{create_tuple_interpolant(
        integer_tuple_metadata("distest", 2), {0.0, 10.0}, {1.0, 12.0}, ParFile::Curve::LINEAR, num_steps)};

    EXPECT_EQ("0/10", interpolant->step());
    EXPECT_EQ("1/11", interpolant->step());
    EXPECT_EQ("1/12", interpolant->step());
}

TEST(TestInterpolant, integerTupleRejectsFractionalEndpoint)
{
    const int num_steps{3};

    EXPECT_THROW(create_tuple_interpolant(
                     integer_tuple_metadata("distest", 2), {0.5, 10.0}, {1.0, 12.0}, ParFile::Curve::LINEAR, num_steps),
        std::runtime_error);
}

TEST(TestInterpolant, numericTupleOrEnumInterpolatesTupleArms)
{
    const int num_steps{3};
    ParFile::ParameterMetadata parameter_metadata{tuple_or_enum_metadata("initorbit", 2)};
    parameter_metadata.default_curve = ParFile::Curve::LINEAR;
    ParFile::InterpolantPtr interpolant{
        create_tuple_interpolant(parameter_metadata, {0.0, 0.0}, {2.0, 4.0}, ParFile::Curve::LINEAR, num_steps)};

    EXPECT_EQ("0/0", interpolant->step());
    EXPECT_EQ("1/2", interpolant->step());
    EXPECT_EQ("2/4", interpolant->step());
}

TEST(TestInterpolant, numericTupleOrEnumHoldsMixedArms)
{
    const int num_steps{3};
    using NumberArray = ParFile::KeyframeConfig::Value::NumberArray;
    std::vector<ParFile::KeyframeConfig> keys{{0, "pixel"}, {2, NumberArray{0.0, 0.0}}};
    ParFile::InterpolantPtr interpolant{
        ParFile::create_interpolant(resolved_track(tuple_or_enum_metadata("initorbit", 2), keys, "pixel"), num_steps)};

    EXPECT_EQ("pixel", interpolant->step());
    EXPECT_EQ("pixel", interpolant->step());
    EXPECT_EQ("0/0", interpolant->step());
}

TEST(TestInterpolant, numericTupleOrEnumRejectsStringNumericTuple)
{
    const int num_steps{3};

    EXPECT_THROW(
        create_interpolant(tuple_or_enum_metadata("initorbit", 2), "0/0", "pixel", num_steps), std::runtime_error);
}

TEST(TestInterpolant, potentialInterpolatesNumericFields)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{create_interpolant(
        potential_metadata(), "240/1000/500/16bit", "120/2000/1000/16bit", ParFile::Curve::LINEAR, num_steps)};

    EXPECT_EQ("240/1000/500/16bit", interpolant->step());
    EXPECT_EQ("180/1500/750/16bit", interpolant->step());
    EXPECT_EQ("120/2000/1000/16bit", interpolant->step());
}

TEST(TestInterpolant, potentialRejectsInvalidParserShapes)
{
    const int num_steps{3};
    const ParFile::ParameterMetadata parameter_metadata{potential_metadata()};

    EXPECT_THROW(
        create_interpolant(parameter_metadata, "240/1000", "120/2000/1000/16bit", num_steps), std::runtime_error);
    EXPECT_THROW(
        create_interpolant(parameter_metadata, "240/1000/16bit", "120/2000/16bit", num_steps), std::runtime_error);
    EXPECT_THROW(create_interpolant(parameter_metadata, "240/bad/500", "120/2000/1000", num_steps), std::runtime_error);
}

TEST(TestInterpolant, miimInterpolatesNumericFieldsAndHoldsMethods)
{
    const int num_steps{3};
    ParFile::ParameterMetadata parameter_metadata{miim_metadata()};
    parameter_metadata.default_curve = ParFile::Curve::LINEAR;
    ParFile::InterpolantPtr interpolant{create_interpolant(
        parameter_metadata, "breadth/left/0/10/20/30", "depth/right/10/20/30/40", ParFile::Curve::LINEAR, num_steps)};

    EXPECT_EQ("breadth/left/0/10/20/30", interpolant->step());
    EXPECT_EQ("breadth/left/5/15/25/35", interpolant->step());
    EXPECT_EQ("depth/right/10/20/30/40", interpolant->step());
}

TEST(TestInterpolant, miimCanonicalizesMethodAbbreviations)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{create_interpolant(miim_metadata(), "b/l", "w/r", num_steps)};

    EXPECT_EQ("breadth/left", interpolant->step());
    EXPECT_EQ("breadth/left", interpolant->step());
    EXPECT_EQ("walk/right", interpolant->step());
}

TEST(TestInterpolant, miimRejectsInvalidParserShapes)
{
    const int num_steps{3};
    const ParFile::ParameterMetadata parameter_metadata{miim_metadata()};

    EXPECT_THROW(create_interpolant(parameter_metadata, "breadth", "depth/right", num_steps), std::runtime_error);
    EXPECT_THROW(create_interpolant(parameter_metadata, "bad/left", "depth/right", num_steps), std::runtime_error);
    EXPECT_THROW(
        create_interpolant(parameter_metadata, "breadth/left/nope", "depth/right/1", num_steps), std::runtime_error);
    EXPECT_THROW(create_interpolant(parameter_metadata, "breadth/left/1/2/3/4/5", "depth/right/1/2/3/4/5", num_steps),
        std::runtime_error);
}

TEST(TestInterpolant, point3WritesThreeValueTuple)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{
        create_tuple_interpolant(metadata("lightsource", ParFile::ParameterType::POINT3), {0.0, 10.0, 20.0},
            {10.0, 20.0, 30.0}, ParFile::Curve::LINEAR, num_steps)};

    EXPECT_EQ("0/10/20", interpolant->step());
    EXPECT_EQ("5/15/25", interpolant->step());
    EXPECT_EQ("10/20/30", interpolant->step());
}

TEST(TestInterpolant, vector3NormalizesWhenRequested)
{
    const int num_steps{3};
    ParFile::ParameterMetadata parameter_metadata{metadata("view-up", ParFile::ParameterType::VECTOR3)};
    parameter_metadata.normalize = true;
    ParFile::InterpolantPtr interpolant{create_tuple_interpolant(
        parameter_metadata, {10.0, 0.0, 0.0}, {0.0, 10.0, 0.0}, ParFile::Curve::LINEAR, num_steps)};

    EXPECT_EQ("1/0/0", interpolant->step());
    EXPECT_EQ("0.707106781187/0.707106781187/0", interpolant->step());
    EXPECT_EQ("0/1/0", interpolant->step());
}

TEST(TestInterpolant, point3DoesNotNormalize)
{
    const int num_steps{3};
    ParFile::ParameterMetadata parameter_metadata{metadata("look-at", ParFile::ParameterType::POINT3)};
    parameter_metadata.normalize = true;
    ParFile::InterpolantPtr interpolant{create_tuple_interpolant(
        parameter_metadata, {10.0, 0.0, 0.0}, {0.0, 10.0, 0.0}, ParFile::Curve::LINEAR, num_steps)};

    EXPECT_EQ("10/0/0", interpolant->step());
    EXPECT_EQ("5/5/0", interpolant->step());
    EXPECT_EQ("0/10/0", interpolant->step());
}

TEST(TestInterpolant, enumHold)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{create_interpolant(enum_metadata("inside"), "bof60", "zmag", num_steps)};

    EXPECT_EQ("bof60", interpolant->step());
    EXPECT_EQ("bof60", interpolant->step());
    EXPECT_EQ("zmag", interpolant->step());
}

TEST(TestInterpolant, enumStep)
{
    const int num_steps{4};
    ParFile::InterpolantPtr interpolant{
        create_interpolant(enum_metadata("inside"), "bof60", "zmag", ParFile::Curve::STEP, num_steps)};

    EXPECT_EQ("bof60", interpolant->step());
    EXPECT_EQ("bof60", interpolant->step());
    EXPECT_EQ("bof60", interpolant->step());
    EXPECT_EQ("zmag", interpolant->step());
}

TEST(TestInterpolant, enumUnknownValueRejected)
{
    const int num_steps{3};

    EXPECT_THROW(create_interpolant(enum_metadata("inside"), "bof60", "unknown", num_steps), std::runtime_error);
}

TEST(TestInterpolant, enumLinearCurveRejected)
{
    const int num_steps{3};
    const ParFile::ParameterMetadata parameter_metadata{enum_metadata("inside")};

    EXPECT_THROW(
        ParFile::create_interpolant(
            resolved_track(parameter_metadata, keyframes("bof60", "zmag", ParFile::Curve::LINEAR, num_steps), "bof60"),
            num_steps),
        std::runtime_error);
}

TEST(TestInterpolant, passesDigitStringsAreEnumValues)
{
    const int num_steps{3};
    ParFile::ParameterMetadata parameter_metadata{enum_metadata("passes")};
    parameter_metadata.values = {"1", "2", "3", "g", "g6", "b", "d", "t", "s", "o", "p"};
    ParFile::InterpolantPtr interpolant{create_interpolant(parameter_metadata, "1", "g6", num_steps)};

    EXPECT_EQ("1", interpolant->step());
    EXPECT_EQ("1", interpolant->step());
    EXPECT_EQ("g6", interpolant->step());
}

TEST(TestInterpolant, passesIntegerKeyframesRejected)
{
    const int num_steps{3};
    ParFile::ParameterMetadata parameter_metadata{enum_metadata("passes")};
    parameter_metadata.values = {"1", "2", "3"};

    EXPECT_THROW(ParFile::create_interpolant(
                     resolved_track(parameter_metadata, integer_keyframes(1, 2, ParFile::Curve::HOLD, num_steps), "1"),
                     num_steps),
        std::runtime_error);
}

TEST(TestInterpolant, integerOrEnumInterpolatesIntegerArms)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{
        ParFile::create_interpolant(resolved_track(integer_or_enum_metadata("fillcolor"),
                                        integer_keyframes(0, 10, ParFile::Curve::LINEAR, num_steps), "normal"),
            num_steps)};

    EXPECT_EQ("0", interpolant->step());
    EXPECT_EQ("5", interpolant->step());
    EXPECT_EQ("10", interpolant->step());
}

TEST(TestInterpolant, integerOrEnumHoldsMixedArms)
{
    const int num_steps{3};
    std::vector<ParFile::KeyframeConfig> keys{{0, "normal"}, {2, 10}};
    ParFile::InterpolantPtr interpolant{
        ParFile::create_interpolant(resolved_track(integer_or_enum_metadata("fillcolor"), keys, "normal"), num_steps)};

    EXPECT_EQ("normal", interpolant->step());
    EXPECT_EQ("normal", interpolant->step());
    EXPECT_EQ("10", interpolant->step());
}

TEST(TestInterpolant, integerOrEnumRejectsStringNumericValue)
{
    const int num_steps{3};

    EXPECT_THROW(
        create_interpolant(integer_or_enum_metadata("fillcolor"), "1", "normal", num_steps), std::runtime_error);
}

TEST(TestInterpolant, stringHold)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{create_interpolant(string_metadata("formulaname"), "foo", "bar", num_steps)};

    EXPECT_EQ("foo", interpolant->step());
    EXPECT_EQ("foo", interpolant->step());
    EXPECT_EQ("bar", interpolant->step());
}

TEST(TestInterpolant, stringStep)
{
    const int num_steps{4};
    ParFile::InterpolantPtr interpolant{
        create_interpolant(string_metadata("passes"), "g1", "g6", ParFile::Curve::STEP, num_steps)};

    EXPECT_EQ("g1", interpolant->step());
    EXPECT_EQ("g1", interpolant->step());
    EXPECT_EQ("g1", interpolant->step());
    EXPECT_EQ("g6", interpolant->step());
}

TEST(TestInterpolant, stringLinearCurveRejected)
{
    const int num_steps{3};
    const ParFile::ParameterMetadata parameter_metadata{string_metadata("formulaname")};

    EXPECT_THROW(ParFile::create_interpolant(resolved_track(parameter_metadata,
                                                 keyframes("foo", "bar", ParFile::Curve::LINEAR, num_steps), "foo"),
                     num_steps),
        std::runtime_error);
}

TEST(TestInterpolant, stringBooleanKeyframesRejected)
{
    const int num_steps{3};
    const ParFile::ParameterMetadata parameter_metadata{string_metadata("formulaname")};

    EXPECT_THROW(ParFile::create_interpolant(
                     resolved_track(parameter_metadata, bool_keyframes(false, true, num_steps), "false"), num_steps),
        std::runtime_error);
}

TEST(TestInterpolant, yesNoFormatsBooleanKeyframesAsIdValues)
{
    const int num_steps{3};
    const ParFile::ParameterMetadata parameter_metadata{yes_no_metadata("showorbit")};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant(
        resolved_track(parameter_metadata, bool_keyframes(false, true, num_steps), "false"), num_steps)};

    EXPECT_EQ("no", interpolant->step());
    EXPECT_EQ("no", interpolant->step());
    EXPECT_EQ("yes", interpolant->step());
}

TEST(TestInterpolant, yesNoStringKeyframesRejected)
{
    const int num_steps{3};
    const ParFile::ParameterMetadata parameter_metadata{yes_no_metadata("showorbit")};

    EXPECT_THROW(ParFile::create_interpolant(
                     resolved_track(parameter_metadata, keyframes("no", "yes", num_steps), "false"), num_steps),
        std::runtime_error);
}

TEST(TestInterpolant, enumPwmMixZeroEmitsA)
{
    const int num_steps{4};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant(
        resolved_pwm_track(enum_metadata("inside"), "bof60", "zmag", 2, 0.0, 0.0, num_steps), num_steps)};

    EXPECT_EQ("bof60", interpolant->step());
    EXPECT_EQ("bof60", interpolant->step());
    EXPECT_EQ("bof60", interpolant->step());
    EXPECT_EQ("bof60", interpolant->step());
}

TEST(TestInterpolant, insideMethodHold)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{create_interpolant(inside_metadata("inside"), "bof60", "zmag", num_steps)};

    EXPECT_EQ("bof60", interpolant->step());
    EXPECT_EQ("bof60", interpolant->step());
    EXPECT_EQ("zmag", interpolant->step());
}

TEST(TestInterpolant, insideMethodStep)
{
    const int num_steps{4};
    ParFile::InterpolantPtr interpolant{
        create_interpolant(inside_metadata("inside"), "bof60", "zmag", ParFile::Curve::STEP, num_steps)};

    EXPECT_EQ("bof60", interpolant->step());
    EXPECT_EQ("bof60", interpolant->step());
    EXPECT_EQ("bof60", interpolant->step());
    EXPECT_EQ("zmag", interpolant->step());
}

TEST(TestInterpolant, outsideMethodHold)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{create_interpolant(outside_metadata("outside"), "real", "tdis", num_steps)};

    EXPECT_EQ("real", interpolant->step());
    EXPECT_EQ("real", interpolant->step());
    EXPECT_EQ("tdis", interpolant->step());
}

TEST(TestInterpolant, outsideMethodStep)
{
    const int num_steps{4};
    ParFile::InterpolantPtr interpolant{
        create_interpolant(outside_metadata("outside"), "real", "tdis", ParFile::Curve::STEP, num_steps)};

    EXPECT_EQ("real", interpolant->step());
    EXPECT_EQ("real", interpolant->step());
    EXPECT_EQ("real", interpolant->step());
    EXPECT_EQ("tdis", interpolant->step());
}

TEST(TestInterpolant, insidePwmMixOneEmitsB)
{
    const int num_steps{4};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant(
        resolved_pwm_track(inside_metadata("inside"), "bof60", "zmag", 2, 1.0, 1.0, num_steps), num_steps)};

    EXPECT_EQ("zmag", interpolant->step());
    EXPECT_EQ("zmag", interpolant->step());
    EXPECT_EQ("zmag", interpolant->step());
    EXPECT_EQ("zmag", interpolant->step());
}

TEST(TestInterpolant, yesNoPwmDefaultsToFalseTrue)
{
    const int num_steps{4};
    ParFile::InterpolantPtr off_interpolant{ParFile::create_interpolant(
        resolved_bool_pwm_track(yes_no_metadata("showorbit"), {}, {}, 2, 0.0, 0.0, num_steps), num_steps)};
    ParFile::InterpolantPtr on_interpolant{ParFile::create_interpolant(
        resolved_bool_pwm_track(yes_no_metadata("showorbit"), {}, {}, 2, 1.0, 1.0, num_steps), num_steps)};

    EXPECT_EQ("no", off_interpolant->step());
    EXPECT_EQ("no", off_interpolant->step());
    EXPECT_EQ("yes", on_interpolant->step());
    EXPECT_EQ("yes", on_interpolant->step());
}

TEST(TestInterpolant, yesNoPwmStringEndpointsRejected)
{
    const int num_steps{4};
    const ParFile::PwmEndpointConfig off{"no"};
    const ParFile::PwmEndpointConfig on{"yes"};

    EXPECT_THROW(ParFile::create_interpolant(
                     resolved_bool_pwm_track(yes_no_metadata("showorbit"), off, on, 2, 0.0, 1.0, num_steps), num_steps),
        std::runtime_error);
}

TEST(TestInterpolant, outsidePwmWindowBelowTwoRejected)
{
    const int num_steps{4};

    EXPECT_THROW(
        ParFile::create_interpolant(
            resolved_pwm_track(outside_metadata("outside"), "real", "tdis", 1, 0.0, 1.0, num_steps), num_steps),
        std::runtime_error);
}

TEST(TestInterpolant, insideColorIndexHold)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{create_interpolant(inside_metadata("inside"), "0", "255", num_steps)};

    EXPECT_EQ("0", interpolant->step());
    EXPECT_EQ("0", interpolant->step());
    EXPECT_EQ("255", interpolant->step());
}

TEST(TestInterpolant, insideUnknownStringRejected)
{
    const int num_steps{3};

    EXPECT_THROW(create_interpolant(inside_metadata("inside"), "bof60", "unknown", num_steps), std::runtime_error);
}

TEST(TestInterpolant, insideColorIndexOutOfRangeRejected)
{
    const int num_steps{3};

    EXPECT_THROW(create_interpolant(inside_metadata("inside"), "0", "256", num_steps), std::runtime_error);
}

TEST(TestInterpolant, insideLinearCurveRejected)
{
    const int num_steps{3};
    const ParFile::ParameterMetadata parameter_metadata{inside_metadata("inside")};

    EXPECT_THROW(
        ParFile::create_interpolant(
            resolved_track(parameter_metadata, keyframes("bof60", "zmag", ParFile::Curve::LINEAR, num_steps), "bof60"),
            num_steps),
        std::runtime_error);
}

TEST(TestInterpolant, paramsComplexInterpolatesSlashPair)
{
    const int num_steps{3};
    const std::vector<ParFile::KeyframeConfig> keys{{0, "0/1"}, {2, "2/3"}};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant(
        resolved_params_track("params.c", ParFile::ParameterType::COMPLEX, keys, "0/1/52", {0, 1}), num_steps)};

    ASSERT_TRUE(interpolant);
    EXPECT_EQ("params", interpolant->name());
    ASSERT_EQ(2U, interpolant->output_slots().size());
    EXPECT_EQ(0, interpolant->output_slots()[0]);
    EXPECT_EQ(1, interpolant->output_slots()[1]);
    EXPECT_EQ("0/1/52", interpolant->step());
    EXPECT_EQ("1/2/52", interpolant->step());
    EXPECT_EQ("2/3/52", interpolant->step());
}

TEST(TestInterpolant, complexCirclePathReturnsToStartAfterOneTurn)
{
    const int num_steps{5};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant(
        resolved_params_path_track("params.c", circle_path("0/0", 1.0), "0/0/52", {0, 1}), num_steps)};

    ASSERT_TRUE(interpolant);
    const std::string first{interpolant->step()};
    static_cast<void>(interpolant->step());
    static_cast<void>(interpolant->step());
    static_cast<void>(interpolant->step());
    const std::string last{interpolant->step()};

    EXPECT_EQ("1/0/52", first);
    EXPECT_EQ(first, last);
}

TEST(TestInterpolant, pointEllipsePathUsesIndependentRadii)
{
    const int num_steps{5};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant(
        resolved_path_track(metadata("look-at", ParFile::ParameterType::POINT2), ellipse_path("0/0", 2.0, 1.0)),
        num_steps)};

    ASSERT_TRUE(interpolant);
    EXPECT_EQ("2/0", interpolant->step());
    EXPECT_EQ("0/1", interpolant->step());
    EXPECT_EQ("-2/0", interpolant->step());
    EXPECT_EQ("0/-1", interpolant->step());
    EXPECT_EQ("2/0", interpolant->step());
}

TEST(TestInterpolant, circlePathPhaseChangesStartingPoint)
{
    const int num_steps{5};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant(
        resolved_path_track(metadata("look-at", ParFile::ParameterType::POINT2), circle_path("0/0", 1.0, 1.0, 90.0)),
        num_steps)};

    ASSERT_TRUE(interpolant);
    EXPECT_EQ("0/1", interpolant->step());
}

TEST(TestInterpolant, lissajousPathIsDeterministicForSamePhaseAndFrequency)
{
    const int num_steps{6};
    ParFile::InterpolantPtr left{ParFile::create_interpolant(
        resolved_path_track(metadata("look-at", ParFile::ParameterType::POINT2), lissajous_path("0/0", 3.0, 2.0, 45.0)),
        num_steps)};
    ParFile::InterpolantPtr right{ParFile::create_interpolant(
        resolved_path_track(metadata("look-at", ParFile::ParameterType::POINT2), lissajous_path("0/0", 3.0, 2.0, 45.0)),
        num_steps)};

    ASSERT_TRUE(left);
    ASSERT_TRUE(right);
    for (int i = 0; i < num_steps; ++i)
    {
        EXPECT_EQ(left->step(), right->step());
    }
}

TEST(TestInterpolant, spiralPathRadiusChangesOverTime)
{
    const int num_steps{5};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant(
        resolved_path_track(metadata("look-at", ParFile::ParameterType::POINT2), spiral_path("0/0", 1.0, 3.0)),
        num_steps)};

    ASSERT_TRUE(interpolant);
    EXPECT_EQ("1/0", interpolant->step());
    EXPECT_EQ("0/1.5", interpolant->step());
    EXPECT_EQ("-2/0", interpolant->step());
    EXPECT_EQ("0/-2.5", interpolant->step());
    EXPECT_EQ("3/0", interpolant->step());
}

TEST(TestInterpolant, invalidPathFrequencyOrRadiusRejected)
{
    const int num_steps{5};

    EXPECT_THROW(ParFile::create_interpolant(resolved_path_track(metadata("look-at", ParFile::ParameterType::POINT2),
                                                 lissajous_path("0/0", 0.0, 1.0)),
                     num_steps),
        std::runtime_error);
    EXPECT_THROW(ParFile::create_interpolant(resolved_path_track(metadata("look-at", ParFile::ParameterType::POINT2),
                                                 spiral_path("0/0", 1.0, -1.0)),
                     num_steps),
        std::runtime_error);
}

TEST(TestInterpolant, bezierPathHitsFirstAndLastControlPoints)
{
    const int num_steps{5};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant(
        resolved_path_track(metadata("look-at", ParFile::ParameterType::POINT2), bezier_path({"0/0", "2/4", "4/0"})),
        num_steps)};

    ASSERT_TRUE(interpolant);
    EXPECT_EQ("0/0", interpolant->step());
    EXPECT_EQ("1/1.5", interpolant->step());
    EXPECT_EQ("2/2", interpolant->step());
    EXPECT_EQ("3/1.5", interpolant->step());
    EXPECT_EQ("4/0", interpolant->step());
}

TEST(TestInterpolant, bezierParamsPathPreservesOtherSlots)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant(
        resolved_params_path_track("params.c", bezier_path({"0/1", "2/3", "4/5"}), "0/1/52", {0, 1}), num_steps)};

    ASSERT_TRUE(interpolant);
    EXPECT_EQ("params", interpolant->name());
    EXPECT_EQ("0/1/52", interpolant->step());
    EXPECT_EQ("2/3/52", interpolant->step());
    EXPECT_EQ("4/5/52", interpolant->step());
}

TEST(TestInterpolant, bezierTuplePathPreservesArity)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant(
        resolved_path_track(tuple_metadata("position", 3), bezier_path({"0/1/2", "2/3/4", "4/5/6"})), num_steps)};

    ASSERT_TRUE(interpolant);
    EXPECT_EQ("0/1/2", interpolant->step());
    EXPECT_EQ("2/3/4", interpolant->step());
    EXPECT_EQ("4/5/6", interpolant->step());
}

TEST(TestInterpolant, invalidBezierPathRejected)
{
    const int num_steps{3};

    EXPECT_THROW(
        ParFile::create_interpolant(
            resolved_path_track(metadata("look-at", ParFile::ParameterType::POINT2), bezier_path({"0/0"})), num_steps),
        std::runtime_error);
    EXPECT_THROW(ParFile::create_interpolant(resolved_path_track(metadata("look-at", ParFile::ParameterType::POINT2),
                                                 bezier_path({"0/0", "1/1/1"})),
                     num_steps),
        std::runtime_error);
}

TEST(TestInterpolant, catmullRomPathPassesThroughControlPoints)
{
    const int num_steps{7};
    ParFile::InterpolantPtr interpolant{
        ParFile::create_interpolant(resolved_path_track(metadata("look-at", ParFile::ParameterType::POINT2),
                                        catmull_rom_path({"0/0", "1/2", "3/2", "4/0"})),
            num_steps)};

    ASSERT_TRUE(interpolant);
    EXPECT_EQ("0/0", interpolant->step());
    EXPECT_EQ("0.4375/1.125", interpolant->step());
    EXPECT_EQ("1/2", interpolant->step());
    EXPECT_EQ("2/2.25", interpolant->step());
    EXPECT_EQ("3/2", interpolant->step());
    EXPECT_EQ("3.5625/1.125", interpolant->step());
    EXPECT_EQ("4/0", interpolant->step());
}

TEST(TestInterpolant, catmullRomTuplePathPreservesArity)
{
    const int num_steps{4};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant(
        resolved_path_track(tuple_metadata("position", 3), catmull_rom_path({"0/1/2", "2/3/4", "4/5/6", "6/7/8"})),
        num_steps)};

    ASSERT_TRUE(interpolant);
    EXPECT_EQ("0/1/2", interpolant->step());
    EXPECT_EQ("2/3/4", interpolant->step());
    EXPECT_EQ("4/5/6", interpolant->step());
    EXPECT_EQ("6/7/8", interpolant->step());
}

TEST(TestInterpolant, invalidCatmullRomPathRejected)
{
    const int num_steps{7};

    EXPECT_THROW(ParFile::create_interpolant(resolved_path_track(metadata("look-at", ParFile::ParameterType::POINT2),
                                                 catmull_rom_path({"0/0", "1/1", "2/2"})),
                     num_steps),
        std::runtime_error);
    EXPECT_THROW(ParFile::create_interpolant(resolved_path_track(metadata("look-at", ParFile::ParameterType::POINT2),
                                                 catmull_rom_path({"0/0", "1/1", "2/2", "3/3/3"})),
                     num_steps),
        std::runtime_error);
}

TEST(TestInterpolant, paramsSlotPreservesOtherSlots)
{
    const int num_steps{3};
    const std::vector<ParFile::KeyframeConfig> keys{{0, "2"}, {2, "4"}};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant(
        resolved_params_track("params[0]", ParFile::ParameterType::DOUBLE, keys, "0/1", {0}), num_steps)};

    ASSERT_TRUE(interpolant);
    EXPECT_EQ("params", interpolant->name());
    ASSERT_EQ(1U, interpolant->output_slots().size());
    EXPECT_EQ(0, interpolant->output_slots()[0]);
    EXPECT_EQ("2/1", interpolant->step());
    EXPECT_EQ("3/1", interpolant->step());
    EXPECT_EQ("4/1", interpolant->step());
}

TEST(TestInterpolant, paramsIntegerInterpolatesAndRoundsSlot)
{
    const int num_steps{4};
    const std::vector<ParFile::KeyframeConfig> keys{{0, "0"}, {3, "2"}};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant(
        resolved_params_track("MandelbrotMix4.iterations", ParFile::ParameterType::INTEGER, keys, "0/1/0", {2}),
        num_steps)};

    ASSERT_TRUE(interpolant);
    EXPECT_EQ("params", interpolant->name());
    EXPECT_EQ("0/1/0", interpolant->step());
    EXPECT_EQ("0/1/1", interpolant->step());
    EXPECT_EQ("0/1/1", interpolant->step());
    EXPECT_EQ("0/1/2", interpolant->step());
}

TEST(TestInterpolant, formulaFunctionAcceptsLegalIdFunctions)
{
    const int num_steps{3};
    const std::vector<ParFile::KeyframeConfig> keys{{0, "tan"}, {2, "log"}};
    ParFile::InterpolantPtr interpolant{
        ParFile::create_interpolant(resolved_function_track("MandelbrotMix4.fn2", keys, "sin/cos", 1), num_steps)};

    ASSERT_TRUE(interpolant);
    EXPECT_EQ("function", interpolant->name());
    ASSERT_EQ(1U, interpolant->output_slots().size());
    EXPECT_EQ(1, interpolant->output_slots()[0]);
    EXPECT_EQ("sin/tan", interpolant->step());
    EXPECT_EQ("sin/tan", interpolant->step());
    EXPECT_EQ("sin/log", interpolant->step());
}

TEST(TestInterpolant, functionListAcceptsLegalIdFunctions)
{
    const int num_steps{3};
    const ParFile::ParameterMetadata parameter_metadata{function_list_metadata("function")};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant(
        resolved_track(parameter_metadata, function_list_keyframes("sin/cos", "tan/log", num_steps), "sin/cos"),
        num_steps)};

    ASSERT_TRUE(interpolant);
    EXPECT_EQ("sin/cos", interpolant->step());
    EXPECT_EQ("sin/cos", interpolant->step());
    EXPECT_EQ("tan/log", interpolant->step());
}

TEST(TestInterpolant, functionListRejectsStringKeyframes)
{
    const int num_steps{3};
    const ParFile::ParameterMetadata parameter_metadata{function_list_metadata("function")};

    EXPECT_THROW(
        ParFile::create_interpolant(
            resolved_track(parameter_metadata, keyframes("sin/cos", "tan/log", num_steps), "sin/cos"), num_steps),
        std::runtime_error);
}

TEST(TestInterpolant, functionListRejectsUnknownIdFunction)
{
    const int num_steps{3};
    const ParFile::ParameterMetadata parameter_metadata{function_list_metadata("function")};

    EXPECT_THROW(
        ParFile::create_interpolant(
            resolved_track(parameter_metadata, function_list_keyframes("sin/cos", "tan/unknown", num_steps), "sin/cos"),
            num_steps),
        std::runtime_error);
}

TEST(TestInterpolant, functionSlotPwmWritesSelectedFunction)
{
    const int num_steps{4};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant(
        resolved_function_pwm_track("function[1]", "tan", "log", 2, 0.0, 1.0, num_steps, "sin/cos", 1), num_steps)};

    ASSERT_TRUE(interpolant);
    EXPECT_EQ("function", interpolant->name());
    ASSERT_EQ(1U, interpolant->output_slots().size());
    EXPECT_EQ(1, interpolant->output_slots()[0]);
    EXPECT_EQ("sin/tan", interpolant->step());
    EXPECT_EQ("sin/tan", interpolant->step());
    EXPECT_EQ("sin/log", interpolant->step());
    EXPECT_EQ("sin/log", interpolant->step());
}

TEST(TestInterpolant, formulaFunctionRejectsUnknownIdFunction)
{
    const int num_steps{3};
    const std::vector<ParFile::KeyframeConfig> keys{{0, "tan"}, {2, "unknown"}};

    EXPECT_THROW(
        ParFile::create_interpolant(resolved_function_track("MandelbrotMix4.fn2", keys, "sin/cos", 1), num_steps),
        std::runtime_error);
}
