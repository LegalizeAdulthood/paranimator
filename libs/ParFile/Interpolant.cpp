// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/Interpolant.h>

#include <ParFile/Config.h>
#include <ParFile/ParFile.h>
#include <ParFile/ParameterCatalog.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/format.hpp>

#include <algorithm>
#include <cmath>
#include <complex>
#include <cstddef>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace ParFile
{

namespace
{

class SegmentEvaluator
{
public:
    SegmentEvaluator() = default;
    explicit SegmentEvaluator(int num_steps) :
        m_num_steps(num_steps)
    {
    }

    double fraction(int step) const
    {
        return (step - 1) / static_cast<double>(m_num_steps - 1);
    }

    double linear_value_at(int step, double from, double to) const
    {
        const double local_fraction{fraction(step)};
        return from + local_fraction * (to - from);
    }

    double geometric_value_at(int step, double from, double to) const
    {
        return from * std::pow(to / from, fraction(step));
    }

    std::complex<double> linear_value_at(
        int step, const std::complex<double> &from, const std::complex<double> &to) const
    {
        const double local_fraction{fraction(step)};
        return from + local_fraction * (to - from);
    }

private:
    int m_num_steps{};
};

class Base : public Interpolant
{
public:
    Base() = default;
    Base(std::string_view name, int num_steps) :
        m_name(name),
        m_segment(num_steps)
    {
    }
    Base(std::string_view name, int num_steps, const std::vector<int> &output_slots) :
        m_name(name),
        m_output_slots(output_slots),
        m_segment(num_steps)
    {
    }
    Base(const Base &rhs) = delete;
    Base &operator=(const Base &rhs) = delete;
    Base &operator=(Base &&rhs) = delete;
    ~Base() override = default;

    const std::string &name() const override
    {
        return m_name;
    }
    const std::vector<int> &output_slots() const override
    {
        return m_output_slots;
    }
    bool has_value() const override
    {
        return m_has_value;
    }

protected:
    std::string m_name;
    std::vector<int> m_output_slots;
    int m_step{};
    bool m_has_value{true};
    SegmentEvaluator m_segment;
};

void validate_keyframes(const std::string &name, const std::vector<KeyframeConfig> &keys, int num_steps)
{
    if (keys.size() != 2U)
    {
        throw std::runtime_error("Track '" + name + "' requires exactly two keyframes");
    }
    if (keys[0].frame < 0 || keys[1].frame < 0 || keys[0].frame >= num_steps || keys[1].frame >= num_steps)
    {
        throw std::runtime_error("Track '" + name + "' has keyframes outside the frame range");
    }
    if (keys[0].frame >= keys[1].frame)
    {
        throw std::runtime_error("Track '" + name + "' keyframes must be in increasing order");
    }
}

void validate_full_range(const std::string &name, const std::vector<KeyframeConfig> &keys, int num_steps)
{
    if (keys[0].frame != 0 || keys[1].frame != num_steps - 1)
    {
        throw std::runtime_error("Track '" + name + "' must span the full frame range");
    }
}

int parse_integer(const std::string &text)
{
    try
    {
        std::size_t length{};
        const int value{std::stoi(text, &length)};
        if (length != text.size())
        {
            throw std::runtime_error("Invalid integer value '" + text + "'");
        }
        return value;
    }
    catch (const std::invalid_argument &)
    {
        throw std::runtime_error("Invalid integer value '" + text + "'");
    }
    catch (const std::out_of_range &)
    {
        throw std::runtime_error("Integer value out of range '" + text + "'");
    }
}

double parse_double(const std::string &text)
{
    try
    {
        std::size_t length{};
        const double value{std::stod(text, &length)};
        if (length != text.size() || !std::isfinite(value))
        {
            throw std::runtime_error("Invalid double value '" + text + "'");
        }
        return value;
    }
    catch (const std::invalid_argument &)
    {
        throw std::runtime_error("Invalid double value '" + text + "'");
    }
    catch (const std::out_of_range &)
    {
        throw std::runtime_error("Double value out of range '" + text + "'");
    }
}

std::string format_double(double value)
{
    return (boost::format("%.12g") % value).str();
}

std::vector<double> parse_slash_doubles(const std::string &value)
{
    std::vector<std::string> value_text;
    boost::algorithm::split(value_text, value, [](char c) { return c == '/'; });

    std::vector<double> result;
    result.reserve(value_text.size());
    std::transform(value_text.begin(), value_text.end(), std::back_inserter(result), parse_double);
    return result;
}

std::string format_slash_doubles(const std::vector<double> &values)
{
    std::string result;
    for (double value : values)
    {
        if (!result.empty())
        {
            result += '/';
        }
        result += format_double(value);
    }
    return result;
}

std::vector<std::string> split_slash_strings(const std::string &value)
{
    std::vector<std::string> result;
    boost::algorithm::split(result, value, [](char c) { return c == '/'; });
    return result;
}

std::string format_slash_strings(const std::vector<std::string> &values)
{
    std::string result;
    for (const std::string &value : values)
    {
        if (!result.empty())
        {
            result += '/';
        }
        result += value;
    }
    return result;
}

std::complex<double> parse_slash_pair(const std::string &value)
{
    const std::vector<double> values{parse_slash_doubles(value)};
    if (values.size() != 2U)
    {
        throw std::runtime_error("Complex value must have 2 slash-delimited values; have " +
            std::to_string(values.size()) + " in '" + value + "'");
    }
    return {values[0], values[1]};
}

void validate_enum_value(const ParameterMetadata &metadata, const std::string &value)
{
    const auto it{std::find(metadata.values.begin(), metadata.values.end(), value)};
    if (it == metadata.values.end())
    {
        throw std::runtime_error("Invalid enum value '" + value + "' for parameter '" + metadata.name + "'");
    }
}

void validate_bounds(const ParameterMetadata &metadata, double value)
{
    if (metadata.min && value < *metadata.min)
    {
        throw std::runtime_error("Value for parameter '" + metadata.name + "' is below minimum");
    }
    if (metadata.max && value > *metadata.max)
    {
        throw std::runtime_error("Value for parameter '" + metadata.name + "' is above maximum");
    }
}

bool is_coloring_type(ParameterType type)
{
    return type == ParameterType::INSIDE || type == ParameterType::OUTSIDE;
}

void validate_coloring_value(const ParameterMetadata &metadata, const std::string &value)
{
    const auto enum_it{std::find(metadata.values.begin(), metadata.values.end(), value)};
    if (enum_it != metadata.values.end())
    {
        return;
    }

    try
    {
        validate_bounds(metadata, parse_integer(value));
    }
    catch (const std::runtime_error &)
    {
        throw std::runtime_error("Invalid " + std::string{to_string(metadata.type)} + " value '" + value +
            "' for parameter '" + metadata.name + "'");
    }
}

void validate_discrete_value(const ParameterMetadata &metadata, const std::string &value)
{
    if (is_coloring_type(metadata.type))
    {
        validate_coloring_value(metadata, value);
        return;
    }
    validate_enum_value(metadata, value);
}

bool is_pwm_type(ParameterType type)
{
    return type == ParameterType::ENUM || type == ParameterType::INSIDE || type == ParameterType::OUTSIDE;
}

bool has_validated_discrete_values(ParameterType type)
{
    return type == ParameterType::ENUM || type == ParameterType::INSIDE || type == ParameterType::OUTSIDE;
}

double validate_mix(const std::string &name, const KeyframeConfig &key)
{
    if (!key.mix)
    {
        throw std::runtime_error("PWM track '" + name + "' keyframe is missing mix");
    }
    if (*key.mix < 0.0 || *key.mix > 1.0)
    {
        throw std::runtime_error("PWM track '" + name + "' mix must be in the range 0 through 1");
    }
    return *key.mix;
}

void validate_scalar_curve(std::string_view type, Curve curve)
{
    if (curve != Curve::LINEAR && curve != Curve::HOLD && curve != Curve::STEP)
    {
        throw std::runtime_error("Unsupported " + std::string{type} + " curve '" + std::string{to_string(curve)} + "'");
    }
}

void validate_discrete_curve(std::string_view type, Curve curve)
{
    if (curve != Curve::HOLD && curve != Curve::STEP)
    {
        throw std::runtime_error("Unsupported " + std::string{type} + " curve '" + std::string{to_string(curve)} + "'");
    }
}

int positive_mod(int value, int modulus)
{
    const int result{value % modulus};
    if (result < 0)
    {
        return result + modulus;
    }
    return result;
}

int extrapolated_frame(int frame, int from_frame, int to_frame, ExtrapolateMode extrapolate)
{
    if (extrapolate == ExtrapolateMode::CYCLE)
    {
        const int range_length{to_frame - from_frame + 1};
        return from_frame + positive_mod(frame - from_frame, range_length);
    }
    if (extrapolate == ExtrapolateMode::PING_PONG)
    {
        const int span{to_frame - from_frame};
        if (span <= 0)
        {
            return from_frame;
        }
        const int period{span * 2};
        int offset{positive_mod(frame - from_frame, period)};
        if (offset > span)
        {
            offset = period - offset;
        }
        return from_frame + offset;
    }
    return frame;
}

ExtrapolateMode extrapolate_mode(const ParameterMetadata &metadata)
{
    return metadata.extrapolate.value_or(ExtrapolateMode::CLAMP);
}

Curve default_curve(const ParameterMetadata &metadata)
{
    if (!metadata.default_curve)
    {
        throw std::runtime_error("Missing default curve for parameter '" + metadata.name + "'");
    }
    return *metadata.default_curve;
}

void validate_params_slot(std::string_view name, const std::vector<double> &values, int slot)
{
    if (slot < 0 || static_cast<std::size_t>(slot) >= values.size())
    {
        throw std::runtime_error("Track '" + std::string{name} + "' references missing params slot " +
            std::to_string(slot) + " in source value");
    }
}

int tuple_alias_arity(ParameterType type)
{
    switch (type)
    {
    case ParameterType::POINT2:
    case ParameterType::VECTOR2:
        return 2;
    case ParameterType::POINT3:
    case ParameterType::VECTOR3:
        return 3;
    default:
        return 0;
    }
}

int tuple_arity(const ParameterMetadata &metadata)
{
    const int alias_arity{tuple_alias_arity(metadata.type)};
    if (alias_arity != 0)
    {
        if (metadata.arity && *metadata.arity != alias_arity)
        {
            throw std::runtime_error("Numeric tuple parameter '" + metadata.name + "' arity does not match type");
        }
        return alias_arity;
    }
    if (!metadata.arity)
    {
        throw std::runtime_error("Numeric tuple parameter '" + metadata.name + "' is missing arity");
    }
    return *metadata.arity;
}

bool is_vector_alias(ParameterType type)
{
    return type == ParameterType::VECTOR2 || type == ParameterType::VECTOR3;
}

void normalize_vector(const ParameterMetadata &metadata, std::vector<double> &values)
{
    if (!metadata.normalize || !is_vector_alias(metadata.type))
    {
        return;
    }

    double length_squared{};
    for (double value : values)
    {
        length_squared += value * value;
    }
    if (length_squared == 0.0)
    {
        throw std::runtime_error("Cannot normalize zero vector parameter '" + metadata.name + "'");
    }
    const double length{std::sqrt(length_squared)};
    for (double &value : values)
    {
        value /= length;
    }
}

bool is_planar_path(PathKind kind)
{
    return kind == PathKind::CIRCLE || kind == PathKind::ELLIPSE || kind == PathKind::LISSAJOUS ||
        kind == PathKind::SPIRAL;
}

bool is_bezier_path(PathKind kind)
{
    return kind == PathKind::BEZIER;
}

bool is_control_point_path(PathKind kind)
{
    return kind == PathKind::BEZIER || kind == PathKind::CATMULL_ROM;
}

bool is_explicit_path(PathKind kind)
{
    return is_planar_path(kind) || is_control_point_path(kind);
}

double clean_path_component(double value)
{
    return std::abs(value) < 1.0e-12 ? 0.0 : value;
}

std::vector<double> clean_path_components(std::vector<double> values)
{
    for (double &value : values)
    {
        value = clean_path_component(value);
    }
    return values;
}

std::string format_slash_pair(const std::complex<double> &value)
{
    return format_slash_doubles({clean_path_component(value.real()), clean_path_component(value.imag())});
}

double degrees_to_radians(double degrees)
{
    constexpr double PI{3.141592653589793238462643383279502884};
    return degrees * PI / 180.0;
}

void validate_path_radius(double value, std::string_view name)
{
    if (value < 0.0)
    {
        throw std::runtime_error("Path field '" + std::string{name} + "' must be nonnegative");
    }
}

void validate_path_frequency(double value, std::string_view name)
{
    if (value <= 0.0)
    {
        throw std::runtime_error("Path field '" + std::string{name} + "' must be positive");
    }
}

class PlanarPathEvaluator
{
public:
    PlanarPathEvaluator(const PathConfig &path, int num_steps);

    std::complex<double> value_at(int frame) const;

private:
    double fraction_at(int frame) const;
    std::complex<double> ellipse_value_at(double fraction) const;
    std::complex<double> lissajous_value_at(double fraction) const;
    std::complex<double> spiral_value_at(double fraction) const;

    PathKind m_kind{PathKind::CIRCLE};
    std::complex<double> m_center;
    double m_x_radius{};
    double m_y_radius{};
    double m_from_radius{};
    double m_to_radius{};
    double m_x_frequency{};
    double m_y_frequency{};
    double m_turns{};
    double m_phase{};
    int m_num_steps{};
};

PlanarPathEvaluator::PlanarPathEvaluator(const PathConfig &path, int num_steps) :
    m_kind(path.kind),
    m_center(parse_slash_pair(path.center)),
    m_turns(path.turns),
    m_phase(path.phase),
    m_num_steps(num_steps)
{
    if (num_steps < 2)
    {
        throw std::runtime_error("Planar path requires at least two frames");
    }
    if (path.kind == PathKind::CIRCLE)
    {
        m_x_radius = path.radius;
        m_y_radius = path.radius;
        validate_path_radius(m_x_radius, "radius");
    }
    else if (path.kind == PathKind::ELLIPSE)
    {
        m_x_radius = path.x_radius;
        m_y_radius = path.y_radius;
        validate_path_radius(m_x_radius, "x-radius");
        validate_path_radius(m_y_radius, "y-radius");
    }
    else if (path.kind == PathKind::LISSAJOUS)
    {
        m_x_radius = path.x_radius;
        m_y_radius = path.y_radius;
        m_x_frequency = path.x_frequency;
        m_y_frequency = path.y_frequency;
        validate_path_radius(m_x_radius, "x-radius");
        validate_path_radius(m_y_radius, "y-radius");
        validate_path_frequency(m_x_frequency, "x-frequency");
        validate_path_frequency(m_y_frequency, "y-frequency");
    }
    else if (path.kind == PathKind::SPIRAL)
    {
        m_from_radius = path.from_radius;
        m_to_radius = path.to_radius;
        validate_path_radius(m_from_radius, "from-radius");
        validate_path_radius(m_to_radius, "to-radius");
    }
    else
    {
        throw std::runtime_error("Planar path requires a circle, ellipse, lissajous, or spiral path");
    }
}

double PlanarPathEvaluator::fraction_at(int frame) const
{
    return frame / static_cast<double>(m_num_steps - 1);
}

std::complex<double> PlanarPathEvaluator::ellipse_value_at(double fraction) const
{
    const double radians{degrees_to_radians(m_phase + 360.0 * m_turns * fraction)};
    return {m_center.real() + std::cos(radians) * m_x_radius, m_center.imag() + std::sin(radians) * m_y_radius};
}

std::complex<double> PlanarPathEvaluator::lissajous_value_at(double fraction) const
{
    const double x_radians{degrees_to_radians(m_phase + 360.0 * m_x_frequency * fraction)};
    const double y_radians{degrees_to_radians(360.0 * m_y_frequency * fraction)};
    return {m_center.real() + std::cos(x_radians) * m_x_radius, m_center.imag() + std::sin(y_radians) * m_y_radius};
}

std::complex<double> PlanarPathEvaluator::spiral_value_at(double fraction) const
{
    const double radians{degrees_to_radians(m_phase + 360.0 * m_turns * fraction)};
    const double radius{m_from_radius + fraction * (m_to_radius - m_from_radius)};
    return {m_center.real() + std::cos(radians) * radius, m_center.imag() + std::sin(radians) * radius};
}

std::complex<double> PlanarPathEvaluator::value_at(int frame) const
{
    const double fraction{fraction_at(frame)};
    if (m_kind == PathKind::LISSAJOUS)
    {
        return lissajous_value_at(fraction);
    }
    if (m_kind == PathKind::SPIRAL)
    {
        return spiral_value_at(fraction);
    }
    return ellipse_value_at(fraction);
}

int path_arity(const ParameterMetadata &metadata)
{
    if (metadata.type == ParameterType::COMPLEX)
    {
        return 2;
    }
    if (metadata.type == ParameterType::NUMERIC_TUPLE || tuple_alias_arity(metadata.type) != 0)
    {
        return tuple_arity(metadata);
    }
    throw std::runtime_error("Path track '" + metadata.name + "' requires a complex or tuple target");
}

class ControlPointPathEvaluator
{
public:
    ControlPointPathEvaluator(const PathConfig &path, int num_steps, int arity);

    std::vector<double> value_at(int frame) const;

private:
    double fraction_at(int frame) const;
    std::vector<double> bezier_value_at(double fraction) const;
    std::vector<double> catmull_rom_value_at(double fraction) const;

    PathKind m_kind{PathKind::BEZIER};
    std::vector<std::vector<double>> m_control_points;
    int m_num_steps{};
};

ControlPointPathEvaluator::ControlPointPathEvaluator(const PathConfig &path, int num_steps, int arity) :
    m_kind(path.kind),
    m_num_steps(num_steps)
{
    if (num_steps < 2)
    {
        throw std::runtime_error("Control point path requires at least two frames");
    }
    if (arity <= 0)
    {
        throw std::runtime_error("Control point path requires a positive arity");
    }

    const std::size_t min_control_points{m_kind == PathKind::CATMULL_ROM ? 4U : 2U};
    const std::string path_name{m_kind == PathKind::CATMULL_ROM ? "Catmull-Rom" : "Bezier"};
    if (path.control_points.size() < min_control_points)
    {
        throw std::runtime_error(
            path_name + " path requires at least " + std::to_string(min_control_points) + " control points");
    }

    m_control_points.reserve(path.control_points.size());
    for (const std::string &control_point : path.control_points)
    {
        std::vector<double> values{parse_slash_doubles(control_point)};
        if (values.size() != static_cast<std::size_t>(arity))
        {
            throw std::runtime_error(path_name + " control point '" + control_point + "' has arity " +
                std::to_string(values.size()) + ", expected " + std::to_string(arity));
        }
        m_control_points.emplace_back(std::move(values));
    }
}

double ControlPointPathEvaluator::fraction_at(int frame) const
{
    return frame / static_cast<double>(m_num_steps - 1);
}

std::vector<double> ControlPointPathEvaluator::bezier_value_at(double fraction) const
{
    std::vector<std::vector<double>> values{m_control_points};
    for (std::size_t order{values.size() - 1U}; order > 0U; --order)
    {
        for (std::size_t point{}; point < order; ++point)
        {
            for (std::size_t component{}; component < values[point].size(); ++component)
            {
                values[point][component] =
                    values[point][component] + fraction * (values[point + 1U][component] - values[point][component]);
            }
        }
    }
    return clean_path_components(values[0]);
}

std::vector<double> ControlPointPathEvaluator::catmull_rom_value_at(double fraction) const
{
    const double position{fraction * static_cast<double>(m_control_points.size() - 1U)};
    const std::size_t last_segment{m_control_points.size() - 2U};
    const std::size_t segment{std::min(static_cast<std::size_t>(std::floor(position)), last_segment)};
    const double local{position - static_cast<double>(segment)};
    const double local2{local * local};
    const double local3{local2 * local};
    const std::vector<double> &p1{m_control_points[segment]};
    const std::vector<double> &p2{m_control_points[segment + 1U]};

    std::vector<double> result(p1.size());
    for (std::size_t i{}; i < result.size(); ++i)
    {
        const double p0{segment == 0U ? 2.0 * p1[i] - p2[i] : m_control_points[segment - 1U][i]};
        const double p3{
            segment + 2U < m_control_points.size() ? m_control_points[segment + 2U][i] : 2.0 * p2[i] - p1[i]};
        result[i] = 0.5 *
            ((2.0 * p1[i]) + (-p0 + p2[i]) * local + (2.0 * p0 - 5.0 * p1[i] + 4.0 * p2[i] - p3) * local2 +
                (-p0 + 3.0 * p1[i] - 3.0 * p2[i] + p3) * local3);
    }
    return clean_path_components(result);
}

std::vector<double> ControlPointPathEvaluator::value_at(int frame) const
{
    const double fraction{fraction_at(frame)};
    if (m_kind == PathKind::CATMULL_ROM)
    {
        return catmull_rom_value_at(fraction);
    }
    return bezier_value_at(fraction);
}

class ComplexPathInterpolant : public Base
{
public:
    ComplexPathInterpolant(const ResolvedTrack &track, int num_steps);
    ~ComplexPathInterpolant() override = default;

    std::string step() override;

private:
    PlanarPathEvaluator m_path;
};

ComplexPathInterpolant::ComplexPathInterpolant(const ResolvedTrack &track, int num_steps) :
    Base(track.output_parameter, num_steps),
    m_path(*track.path, num_steps)
{
}

std::string ComplexPathInterpolant::step()
{
    const std::complex<double> value{m_path.value_at(m_step)};
    ++m_step;
    return format_slash_pair(value);
}

class ParamsComplexPathInterpolant : public Base
{
public:
    ParamsComplexPathInterpolant(const ResolvedTrack &track, int num_steps);
    ~ParamsComplexPathInterpolant() override = default;

    std::string step() override;

private:
    PlanarPathEvaluator m_path;
    std::vector<double> m_base_values;
};

ParamsComplexPathInterpolant::ParamsComplexPathInterpolant(const ResolvedTrack &track, int num_steps) :
    Base(track.output_parameter, num_steps, track.slots),
    m_path(*track.path, num_steps),
    m_base_values(parse_slash_doubles(track.base_value))
{
    if (track.slots.size() != 2U)
    {
        throw std::runtime_error("Track '" + track.parameter + "' requires exactly two params slots");
    }
    validate_params_slot(track.parameter, m_base_values, track.slots[0]);
    validate_params_slot(track.parameter, m_base_values, track.slots[1]);
}

std::string ParamsComplexPathInterpolant::step()
{
    const std::complex<double> value{m_path.value_at(m_step)};
    ++m_step;

    std::vector<double> values{m_base_values};
    values[static_cast<std::size_t>(m_output_slots[0])] = clean_path_component(value.real());
    values[static_cast<std::size_t>(m_output_slots[1])] = clean_path_component(value.imag());
    return format_slash_doubles(values);
}

class Point2PathInterpolant : public Base
{
public:
    Point2PathInterpolant(const ResolvedTrack &track, int num_steps);
    ~Point2PathInterpolant() override = default;

    std::string step() override;

private:
    PlanarPathEvaluator m_path;
};

Point2PathInterpolant::Point2PathInterpolant(const ResolvedTrack &track, int num_steps) :
    Base(track.output_parameter, num_steps),
    m_path(*track.path, num_steps)
{
}

std::string Point2PathInterpolant::step()
{
    const std::complex<double> value{m_path.value_at(m_step)};
    ++m_step;
    return format_slash_pair(value);
}

class TuplePathInterpolant : public Base
{
public:
    TuplePathInterpolant(const ResolvedTrack &track, int num_steps);
    ~TuplePathInterpolant() override = default;

    std::string step() override;

private:
    ParameterMetadata m_metadata;
    ControlPointPathEvaluator m_path;
};

TuplePathInterpolant::TuplePathInterpolant(const ResolvedTrack &track, int num_steps) :
    Base(track.output_parameter, num_steps),
    m_metadata(track.metadata),
    m_path(*track.path, num_steps, path_arity(track.metadata))
{
}

std::string TuplePathInterpolant::step()
{
    std::vector<double> values{m_path.value_at(m_step)};
    ++m_step;
    normalize_vector(m_metadata, values);
    return format_slash_doubles(clean_path_components(values));
}

class ParamsTuplePathInterpolant : public Base
{
public:
    ParamsTuplePathInterpolant(const ResolvedTrack &track, int num_steps);
    ~ParamsTuplePathInterpolant() override = default;

    std::string step() override;

private:
    ParameterMetadata m_metadata;
    ControlPointPathEvaluator m_path;
    std::vector<double> m_base_values;
};

ParamsTuplePathInterpolant::ParamsTuplePathInterpolant(const ResolvedTrack &track, int num_steps) :
    Base(track.output_parameter, num_steps, track.slots),
    m_metadata(track.metadata),
    m_path(*track.path, num_steps, path_arity(track.metadata)),
    m_base_values(parse_slash_doubles(track.base_value))
{
    const int arity{path_arity(track.metadata)};
    if (track.slots.size() != static_cast<std::size_t>(arity))
    {
        throw std::runtime_error("Track '" + track.parameter + "' requires " + std::to_string(arity) + " params slots");
    }
    for (const int slot : track.slots)
    {
        validate_params_slot(track.parameter, m_base_values, slot);
    }
}

std::string ParamsTuplePathInterpolant::step()
{
    std::vector<double> path_values{m_path.value_at(m_step)};
    ++m_step;
    normalize_vector(m_metadata, path_values);

    std::vector<double> values{m_base_values};
    for (std::size_t i{}; i < m_output_slots.size(); ++i)
    {
        values[static_cast<std::size_t>(m_output_slots[i])] = clean_path_component(path_values[i]);
    }
    return format_slash_doubles(values);
}

struct CenterMag
{
    CenterMag() = default;
    CenterMag(const std::string &value);
    CenterMag(const CenterMag &rhs) = default;
    CenterMag(CenterMag &&rhs) = default;
    CenterMag &operator=(const CenterMag &rhs) = default;
    CenterMag &operator=(CenterMag &&rhs) = default;

    std::complex<double> center;
    double mag;
    double x_mag_factor{1.0};
    double rotation{};
    double skew{};
};

CenterMag::CenterMag(const std::string &value)
{
    const std::vector<double> values{parse_slash_doubles(value)};
    if (values.size() < 3U || values.size() > 6U)
    {
        throw std::runtime_error("Center-mag parameter must have 3 through 6 values; have " +
            std::to_string(values.size()) + " in '" + value + "'");
    }
    center = std::complex<double>(values[0], values[1]);
    mag = values[2];
    if (values.size() > 3U && values[3] != 0.0)
    {
        x_mag_factor = values[3];
    }
    if (values.size() > 4U)
    {
        rotation = values[4];
    }
    if (values.size() > 5U)
    {
        skew = values[5];
    }
}

double geometric_or_linear_value_at(const SegmentEvaluator &segment, int step, double from, double to)
{
    if (from > 0.0 && to > 0.0)
    {
        return segment.geometric_value_at(step, from, to);
    }
    return segment.linear_value_at(step, from, to);
}

bool is_default_value(double value, double default_value)
{
    constexpr double TOLERANCE{1.0e-12};
    return std::abs(value - default_value) < TOLERANCE;
}

std::string format_center_mag_value(double value, bool precise)
{
    if (precise)
    {
        return format_double(value);
    }
    return (boost::format("%g") % value).str();
}

std::string format_center_mag(const CenterMag &value, bool precise)
{
    std::vector<double> values{
        value.center.real(), value.center.imag(), value.mag, value.x_mag_factor, value.rotation, value.skew};
    values = clean_path_components(values);
    std::size_t count{3};
    if (!is_default_value(values[5], 0.0))
    {
        count = 6;
    }
    else if (!is_default_value(values[4], 0.0))
    {
        count = 5;
    }
    else if (!is_default_value(values[3], 1.0))
    {
        count = 4;
    }
    values.resize(count);
    std::string result;
    for (double item : values)
    {
        if (!result.empty())
        {
            result += '/';
        }
        result += format_center_mag_value(item, precise);
    }
    return result;
}

class CenterMagInterpolant : public Base
{
public:
    CenterMagInterpolant(std::string_view name, const std::string &from, const std::string &to, int num_steps);
    CenterMagInterpolant(const CenterMagInterpolant &rhs) = delete;
    CenterMagInterpolant(CenterMagInterpolant &&rhs) = delete;
    CenterMagInterpolant &operator=(const CenterMagInterpolant &rhs) = delete;
    CenterMagInterpolant &operator=(CenterMagInterpolant &&rhs) = delete;
    ~CenterMagInterpolant() override = default;

    std::string step() override;

private:
    CenterMag m_from;
    CenterMag m_to;
};

CenterMagInterpolant::CenterMagInterpolant(
    std::string_view name, const std::string &from, const std::string &to, int num_steps) :
    Base(name, num_steps),
    m_from(from),
    m_to(to)
{
}

std::string CenterMagInterpolant::step()
{
    ++m_step;
    CenterMag result;
    result.center = m_segment.linear_value_at(m_step, m_from.center, m_to.center);
    result.mag = geometric_or_linear_value_at(m_segment, m_step, m_from.mag, m_to.mag);
    result.x_mag_factor = geometric_or_linear_value_at(m_segment, m_step, m_from.x_mag_factor, m_to.x_mag_factor);
    result.rotation = m_segment.linear_value_at(m_step, m_from.rotation, m_to.rotation);
    result.skew = m_segment.linear_value_at(m_step, m_from.skew, m_to.skew);
    return format_center_mag(result, false);
}

struct Corners
{
    Corners() = default;
    Corners(const std::string &value);
    Corners(const Corners &rhs) = default;
    Corners(Corners &&rhs) = default;
    Corners &operator=(const Corners &rhs) = default;
    Corners &operator=(Corners &&rhs) = default;

    std::vector<double> values;
};

class CornersInterpolant : public Base
{
public:
    CornersInterpolant(std::string_view name, const std::string &from, const std::string &to, int num_steps);
    ~CornersInterpolant() override = default;

    std::string step() override;

private:
    Corners m_from;
    Corners m_to;
};

Corners::Corners(const std::string &value)
{
    values = parse_slash_doubles(value);
    if (values.size() != 4U && values.size() != 6U)
    {
        throw std::runtime_error(
            "Corners parameter must have 4 or 6 values; have " + std::to_string(values.size()) + " in '" + value + "'");
    }
}

CornersInterpolant::CornersInterpolant(
    std::string_view name, const std::string &from, const std::string &to, int num_steps) :
    Base(name, num_steps),
    m_from(from),
    m_to(to)
{
    if (m_from.values.size() != m_to.values.size())
    {
        throw std::runtime_error("Corners keyframes must have matching arity");
    }
}

std::string CornersInterpolant::step()
{
    ++m_step;
    std::string result;
    for (std::size_t i = 0; i < m_from.values.size(); ++i)
    {
        if (!result.empty())
        {
            result += '/';
        }
        const double value{m_segment.linear_value_at(m_step, m_from.values[i], m_to.values[i])};
        result += format_double(value);
    }
    return result;
}

struct Point2D
{
    double x{};
    double y{};
};

Point2D operator+(const Point2D &lhs, const Point2D &rhs)
{
    return {lhs.x + rhs.x, lhs.y + rhs.y};
}

Point2D operator-(const Point2D &lhs, const Point2D &rhs)
{
    return {lhs.x - rhs.x, lhs.y - rhs.y};
}

Point2D operator*(const Point2D &lhs, double scale)
{
    return {lhs.x * scale, lhs.y * scale};
}

Point2D operator/(const Point2D &lhs, double scale)
{
    return {lhs.x / scale, lhs.y / scale};
}

double length(const Point2D &value)
{
    return std::hypot(value.x, value.y);
}

Point2D point2_from_values(const std::vector<double> &values, std::string_view name)
{
    if (values.size() != 2U)
    {
        throw std::runtime_error("Camera2D value '" + std::string{name} + "' requires two components");
    }
    return {values[0], values[1]};
}

Point2D derived_view_up(const Point2D &look, const Point2D &eye)
{
    const Point2D up{eye - look};
    const double magnitude{length(up)};
    if (magnitude == 0.0)
    {
        throw std::runtime_error("Camera2D eye must not equal look-at");
    }
    return up / magnitude;
}

class Camera2DValueEvaluator
{
public:
    Camera2DValueEvaluator(const ResolvedCamera2DValueTrack &track, int num_steps, bool positive);

    std::vector<double> value_at(int frame) const;

private:
    std::vector<double> parse_value(const std::string &value) const;
    void validate_curve() const;
    void validate_positive_values() const;
    std::vector<double> path_value_at(int frame) const;

    ParameterMetadata m_metadata;
    int m_from_frame{};
    int m_to_frame{};
    std::vector<double> m_from;
    std::vector<double> m_to;
    Curve m_curve{};
    bool m_positive{};
    std::optional<PlanarPathEvaluator> m_planar_path;
    std::optional<ControlPointPathEvaluator> m_control_path;
};

Camera2DValueEvaluator::Camera2DValueEvaluator(const ResolvedCamera2DValueTrack &track, int num_steps, bool positive) :
    m_metadata(track.metadata),
    m_positive(positive)
{
    if (track.path)
    {
        if (!is_explicit_path(track.path->kind))
        {
            throw std::runtime_error("Camera2D value '" + m_metadata.name + "' has no supported path generator");
        }
        if (is_control_point_path(track.path->kind))
        {
            m_control_path.emplace(*track.path, num_steps, path_arity(track.metadata));
        }
        else
        {
            m_planar_path.emplace(*track.path, num_steps);
            if (path_arity(track.metadata) != 2)
            {
                throw std::runtime_error("Camera2D planar path '" + m_metadata.name + "' requires two components");
            }
        }
        return;
    }

    validate_keyframes(track.metadata.name, track.keys, num_steps);
    validate_full_range(track.metadata.name, track.keys, num_steps);
    m_from_frame = track.keys[0].frame;
    m_to_frame = track.keys[1].frame;
    m_from = parse_value(track.keys[0].value);
    m_to = parse_value(track.keys[1].value);
    m_curve = default_curve(track.metadata);
    if (track.keys[1].curve)
    {
        m_curve = *track.keys[1].curve;
    }
    validate_curve();
    validate_positive_values();
}

std::vector<double> Camera2DValueEvaluator::parse_value(const std::string &value) const
{
    if (m_metadata.type == ParameterType::DOUBLE)
    {
        return {parse_double(value)};
    }
    const std::vector<double> values{parse_slash_doubles(value)};
    const std::size_t arity{static_cast<std::size_t>(tuple_arity(m_metadata))};
    if (values.size() != arity)
    {
        throw std::runtime_error(
            "Camera2D value '" + m_metadata.name + "' requires " + std::to_string(arity) + " components");
    }
    return values;
}

void Camera2DValueEvaluator::validate_curve() const
{
    if (m_curve == Curve::GEOMETRIC && m_metadata.type != ParameterType::DOUBLE)
    {
        throw std::runtime_error("Camera2D value '" + m_metadata.name + "' does not support geometric curves");
    }
    if (m_curve != Curve::GEOMETRIC)
    {
        validate_scalar_curve(to_string(m_metadata.type), m_curve);
    }
}

void Camera2DValueEvaluator::validate_positive_values() const
{
    if (m_positive && (m_from[0] <= 0.0 || m_to[0] <= 0.0))
    {
        throw std::runtime_error("Camera2D value '" + m_metadata.name + "' must be positive");
    }
}

std::vector<double> Camera2DValueEvaluator::path_value_at(int frame) const
{
    if (m_planar_path)
    {
        const std::complex<double> value{m_planar_path->value_at(frame)};
        return clean_path_components({value.real(), value.imag()});
    }
    if (m_control_path)
    {
        return m_control_path->value_at(frame);
    }
    throw std::runtime_error("Camera2D value '" + m_metadata.name + "' has no path generator");
}

std::vector<double> Camera2DValueEvaluator::value_at(int frame) const
{
    if (m_planar_path || m_control_path)
    {
        std::vector<double> values{path_value_at(frame)};
        normalize_vector(m_metadata, values);
        return clean_path_components(values);
    }

    std::vector<double> values{m_from};
    if (frame >= m_to_frame)
    {
        values = m_to;
    }
    else if (frame > m_from_frame && m_curve != Curve::HOLD && m_curve != Curve::STEP)
    {
        const double fraction{(frame - m_from_frame) / static_cast<double>(m_to_frame - m_from_frame)};
        for (std::size_t i{}; i < values.size(); ++i)
        {
            if (m_curve == Curve::GEOMETRIC)
            {
                values[i] = m_from[i] * std::pow(m_to[i] / m_from[i], fraction);
            }
            else
            {
                values[i] = m_from[i] + fraction * (m_to[i] - m_from[i]);
            }
        }
    }
    normalize_vector(m_metadata, values);
    return clean_path_components(values);
}

std::optional<Camera2DValueEvaluator> optional_camera2d_value_evaluator(
    const std::optional<ResolvedCamera2DValueTrack> &track, int num_steps, bool positive)
{
    if (!track)
    {
        return {};
    }
    return Camera2DValueEvaluator{*track, num_steps, positive};
}

const ResolvedCamera2DConfig &camera2d_config(const ResolvedTrack &track)
{
    if (!track.camera2d)
    {
        throw std::runtime_error("Camera2D track '" + track.parameter + "' is missing camera settings");
    }
    return *track.camera2d;
}

bool is_normal_axis_aligned(const Point2D &up)
{
    constexpr double TOLERANCE{1.0e-12};
    return std::abs(up.x) < TOLERANCE && std::abs(up.y - 1.0) < TOLERANCE;
}

constexpr double CAMERA2D_PI{3.141592653589793238462643383279502884};

double camera2d_rotation(const Point2D &up)
{
    return std::atan2(up.x, up.y) * 180.0 / CAMERA2D_PI;
}

Point2D rotate_camera2d_point(const Point2D &value, double radians)
{
    const double cosine{std::cos(radians)};
    const double sine{std::sin(radians)};
    return {value.x * cosine + value.y * sine, -value.x * sine + value.y * cosine};
}

bool finite_camera2d_point(const Point2D &value)
{
    return std::isfinite(value.x) && std::isfinite(value.y);
}

void validate_camera2d_point(const Point2D &value)
{
    if (!finite_camera2d_point(value))
    {
        throw std::runtime_error("Camera2D corners output contains a non-finite point");
    }
}

std::string format_camera2d_corners(double aspect, const Point2D &look, const Point2D &up, double height, double skew)
{
    if (!std::isfinite(skew))
    {
        throw std::runtime_error("Camera2D skew must be finite");
    }

    const double half_height{height / 2.0};
    const double half_width{height * aspect / 2.0};
    const double tan_skew{std::tan(degrees_to_radians(skew))};
    if (!std::isfinite(tan_skew))
    {
        throw std::runtime_error("Camera2D skew produces a degenerate affine grid");
    }

    const double skew_offset{half_height * tan_skew};
    Point2D top_left{-half_width + skew_offset, half_height};
    Point2D bottom_right{half_width - skew_offset, -half_height};
    Point2D bottom_left{-half_width - skew_offset, -half_height};

    const double rotation{camera2d_rotation(up)};
    const double rotation_radians{degrees_to_radians(rotation)};
    top_left = rotate_camera2d_point(top_left, rotation_radians) + look;
    bottom_right = rotate_camera2d_point(bottom_right, rotation_radians) + look;
    bottom_left = rotate_camera2d_point(bottom_left, rotation_radians) + look;
    validate_camera2d_point(top_left);
    validate_camera2d_point(bottom_right);
    validate_camera2d_point(bottom_left);

    if (is_normal_axis_aligned(up) && is_default_value(skew, 0.0))
    {
        return format_slash_doubles(clean_path_components({bottom_left.x, bottom_right.x, bottom_left.y, top_left.y}));
    }
    return format_slash_doubles(
        clean_path_components({top_left.x, bottom_right.x, bottom_right.y, top_left.y, bottom_left.x, bottom_left.y}));
}

std::string format_camera2d_center_mag(
    double aspect, double x_mag_factor, const Point2D &look, const Point2D &up, double height, double skew)
{
    const double magnification{4.0 / (aspect * height)};
    CenterMag value;
    value.center = {look.x, look.y};
    value.mag = magnification;
    value.x_mag_factor = x_mag_factor;
    value.rotation = camera2d_rotation(up);
    value.skew = skew;
    return format_center_mag(value, true);
}

class Camera2DInterpolant : public Base
{
public:
    Camera2DInterpolant(const ResolvedTrack &track, int num_steps);
    ~Camera2DInterpolant() override = default;

    std::string step() override;

private:
    ParameterType m_output_type{};
    double m_aspect{};
    double m_center_mag_x_mag_factor{1.0};
    Camera2DValueEvaluator m_look_at;
    std::optional<Camera2DValueEvaluator> m_view_up;
    std::optional<Camera2DValueEvaluator> m_eye;
    Camera2DValueEvaluator m_height;
    std::optional<Camera2DValueEvaluator> m_skew;
};

Camera2DInterpolant::Camera2DInterpolant(const ResolvedTrack &track, int num_steps) :
    Base(track.output_parameter, num_steps),
    m_output_type(track.metadata.type),
    m_aspect(camera2d_config(track).aspect),
    m_center_mag_x_mag_factor(camera2d_config(track).center_mag_x_mag_factor),
    m_look_at(camera2d_config(track).look_at, num_steps, false),
    m_view_up(optional_camera2d_value_evaluator(camera2d_config(track).view_up, num_steps, false)),
    m_eye(optional_camera2d_value_evaluator(camera2d_config(track).eye, num_steps, false)),
    m_height(camera2d_config(track).height, num_steps, true),
    m_skew(optional_camera2d_value_evaluator(camera2d_config(track).skew, num_steps, false))
{
    if (track.metadata.type != ParameterType::CORNERS && track.metadata.type != ParameterType::CENTER_MAG)
    {
        throw std::runtime_error("Camera2D track '" + track.parameter + "' requires a corners or center-mag output");
    }
    if (m_aspect <= 0.0)
    {
        throw std::runtime_error("Camera2D track '" + track.parameter + "' requires a positive aspect");
    }
    if (!m_view_up && !m_eye)
    {
        throw std::runtime_error("Camera2D track '" + track.parameter + "' requires view-up or eye");
    }
}

std::string Camera2DInterpolant::step()
{
    const int frame{m_step};
    ++m_step;

    const Point2D look{point2_from_values(m_look_at.value_at(frame), "look-at")};
    Point2D up;
    if (m_eye)
    {
        const Point2D eye{point2_from_values(m_eye->value_at(frame), "eye")};
        up = derived_view_up(look, eye);
    }
    else
    {
        up = point2_from_values(m_view_up->value_at(frame), "view-up");
    }
    const double height{m_height.value_at(frame)[0]};
    const double skew{m_skew ? m_skew->value_at(frame)[0] : 0.0};

    if (m_output_type == ParameterType::CENTER_MAG)
    {
        return format_camera2d_center_mag(m_aspect, m_center_mag_x_mag_factor, look, up, height, skew);
    }
    return format_camera2d_corners(m_aspect, look, up, height, skew);
}

struct Point3D
{
    double x{};
    double y{};
    double z{};
};

Point3D operator+(const Point3D &lhs, const Point3D &rhs)
{
    return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
}

Point3D operator-(const Point3D &lhs, const Point3D &rhs)
{
    return {lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
}

Point3D operator*(const Point3D &lhs, double scale)
{
    return {lhs.x * scale, lhs.y * scale, lhs.z * scale};
}

Point3D operator/(const Point3D &lhs, double scale)
{
    return {lhs.x / scale, lhs.y / scale, lhs.z / scale};
}

double dot(const Point3D &lhs, const Point3D &rhs)
{
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

Point3D cross(const Point3D &lhs, const Point3D &rhs)
{
    return {lhs.y * rhs.z - lhs.z * rhs.y, lhs.z * rhs.x - lhs.x * rhs.z, lhs.x * rhs.y - lhs.y * rhs.x};
}

double length(const Point3D &value)
{
    return std::sqrt(dot(value, value));
}

bool near_zero(double value)
{
    constexpr double TOLERANCE{1.0e-9};
    return std::abs(value) < TOLERANCE;
}

bool near(const Point3D &lhs, const Point3D &rhs)
{
    return near_zero(lhs.x - rhs.x) && near_zero(lhs.y - rhs.y) && near_zero(lhs.z - rhs.z);
}

Point3D point3_from_values(const std::vector<double> &values, std::string_view name)
{
    if (values.size() != 3U)
    {
        throw std::runtime_error("Camera3D value '" + std::string{name} + "' requires three components");
    }
    return {values[0], values[1], values[2]};
}

Point3D normalized_camera3d_vector(const Point3D &value, std::string_view name)
{
    const double magnitude{length(value)};
    if (magnitude == 0.0 || !std::isfinite(magnitude))
    {
        throw std::runtime_error("Camera3D value '" + std::string{name} + "' must be a nonzero vector");
    }
    return value / magnitude;
}

class Camera3DValueEvaluator
{
public:
    Camera3DValueEvaluator(const ResolvedCamera3DValueTrack &track, int num_steps);

    std::vector<double> value_at(int frame) const;

private:
    std::vector<double> parse_value(const std::string &value) const;

    ParameterMetadata m_metadata;
    int m_from_frame{};
    int m_to_frame{};
    std::vector<double> m_from;
    std::vector<double> m_to;
    Curve m_curve{};
};

Camera3DValueEvaluator::Camera3DValueEvaluator(const ResolvedCamera3DValueTrack &track, int num_steps) :
    m_metadata(track.metadata)
{
    validate_keyframes(track.metadata.name, track.keys, num_steps);
    validate_full_range(track.metadata.name, track.keys, num_steps);
    m_from_frame = track.keys[0].frame;
    m_to_frame = track.keys[1].frame;
    m_from = parse_value(track.keys[0].value);
    m_to = parse_value(track.keys[1].value);
    m_curve = default_curve(track.metadata);
    if (track.keys[1].curve)
    {
        m_curve = *track.keys[1].curve;
    }
    validate_scalar_curve(to_string(m_metadata.type), m_curve);
}

std::vector<double> Camera3DValueEvaluator::parse_value(const std::string &value) const
{
    const std::vector<double> values{parse_slash_doubles(value)};
    const std::size_t arity{static_cast<std::size_t>(tuple_arity(m_metadata))};
    if (values.size() != arity)
    {
        throw std::runtime_error(
            "Camera3D value '" + m_metadata.name + "' requires " + std::to_string(arity) + " components");
    }
    return values;
}

std::vector<double> Camera3DValueEvaluator::value_at(int frame) const
{
    std::vector<double> values{m_from};
    if (frame >= m_to_frame)
    {
        values = m_to;
    }
    else if (frame > m_from_frame && m_curve != Curve::HOLD && m_curve != Curve::STEP)
    {
        const double fraction{(frame - m_from_frame) / static_cast<double>(m_to_frame - m_from_frame)};
        for (std::size_t i{}; i < values.size(); ++i)
        {
            values[i] = m_from[i] + fraction * (m_to[i] - m_from[i]);
        }
    }
    normalize_vector(m_metadata, values);
    return clean_path_components(values);
}

struct Camera3DFrame
{
    Point3D eye;
    Point3D look_at;
    Point3D forward;
    Point3D right;
    Point3D up;
    double distance{};
};

Camera3DFrame camera3d_frame(const Point3D &eye, const Point3D &look_at, const Point3D &view_up, std::string_view name)
{
    Camera3DFrame result;
    result.eye = eye;
    result.look_at = look_at;
    const Point3D to_target{look_at - eye};
    result.distance = length(to_target);
    if (result.distance == 0.0 || !std::isfinite(result.distance))
    {
        throw std::runtime_error("Camera3D value '" + std::string{name} + "' has degenerate eye and look-at");
    }
    result.forward = to_target / result.distance;
    const Point3D up_hint{normalized_camera3d_vector(view_up, name)};
    result.right = cross(result.forward, up_hint);
    const double right_length{length(result.right)};
    if (right_length == 0.0 || !std::isfinite(right_length))
    {
        throw std::runtime_error("Camera3D value '" + std::string{name} + "' has parallel forward and view-up");
    }
    result.right = result.right / right_length;
    result.up = cross(result.right, result.forward);
    return result;
}

Point3D no_roll_camera3d_up(const Camera3DFrame &frame)
{
    const Point3D world_up{0.0, 1.0, 0.0};
    const Point3D right{cross(frame.forward, world_up)};
    const double right_length{length(right)};
    if (right_length == 0.0 || !std::isfinite(right_length))
    {
        throw std::runtime_error("Camera3D frame cannot represent a vertical view direction without roll");
    }
    return cross(right / right_length, frame.forward);
}

void require_centered_camera3d(const Camera3DFrame &frame, std::string_view output)
{
    if (!near(frame.look_at, {0.0, 0.0, 0.0}))
    {
        throw std::runtime_error("Camera3D output '" + std::string{output} + "' does not support center-of-interest");
    }
}

void require_no_roll_camera3d(const Camera3DFrame &frame, std::string_view output)
{
    if (!near(frame.up, no_roll_camera3d_up(frame)))
    {
        throw std::runtime_error("Camera3D output '" + std::string{output} + "' does not support roll");
    }
}

std::string id_camera3d_rotation(const Camera3DFrame &frame)
{
    require_centered_camera3d(frame, "rotation");
    require_no_roll_camera3d(frame, "rotation");
    const double horizontal{std::hypot(frame.forward.x, frame.forward.z)};
    const double x_rotation{-std::atan2(frame.forward.y, horizontal) * 180.0 / CAMERA2D_PI};
    const double y_rotation{std::atan2(frame.forward.x, -frame.forward.z) * 180.0 / CAMERA2D_PI};
    return format_slash_doubles(clean_path_components({x_rotation, y_rotation, 0.0}));
}

std::string id_camera3d_perspective(const Camera3DFrame &frame)
{
    require_centered_camera3d(frame, "perspective");
    require_no_roll_camera3d(frame, "perspective");
    return std::to_string(static_cast<int>(std::lround(frame.distance)));
}

std::string id_camera3d_xyshift(const Camera3DFrame &frame)
{
    require_centered_camera3d(frame, "xyshift");
    require_no_roll_camera3d(frame, "xyshift");
    return "0/0";
}

std::string julibrot_camera3d_geometry(const Camera3DFrame &frame, const std::string &base_value)
{
    require_centered_camera3d(frame, "julibrot3d");
    if (!near(frame.forward, {0.0, 0.0, -1.0}) || !near(frame.up, {0.0, 1.0, 0.0}))
    {
        throw std::runtime_error("Camera3D output 'julibrot3d' supports only a centered straight-on frame");
    }
    std::vector<double> values{parse_slash_doubles(base_value)};
    if (values.size() != 6U)
    {
        throw std::runtime_error("Camera3D output 'julibrot3d' source geometry must have six values");
    }
    values[5] = frame.distance;
    return format_slash_doubles(clean_path_components(values));
}

class Camera3DInterpolant : public Base
{
public:
    Camera3DInterpolant(const ResolvedTrack &track, int num_steps);
    ~Camera3DInterpolant() override = default;

    std::string step() override;

private:
    Camera3DFrame frame_at(int frame) const;

    Camera3DOutputKind m_output_kind{};
    Camera3DValueEvaluator m_eye;
    Camera3DValueEvaluator m_look_at;
    Camera3DValueEvaluator m_view_up;
    std::string m_base_value;
};

Camera3DInterpolant::Camera3DInterpolant(const ResolvedTrack &track, int num_steps) :
    Base(track.output_parameter, num_steps),
    m_output_kind(track.camera3d->output_kind),
    m_eye(track.camera3d->eye, num_steps),
    m_look_at(track.camera3d->look_at, num_steps),
    m_view_up(track.camera3d->view_up, num_steps),
    m_base_value(track.base_value)
{
}

Camera3DFrame Camera3DInterpolant::frame_at(int frame) const
{
    const Point3D eye{point3_from_values(m_eye.value_at(frame), "eye")};
    const Point3D look_at{point3_from_values(m_look_at.value_at(frame), "look-at")};
    const Point3D view_up{point3_from_values(m_view_up.value_at(frame), "view-up")};
    return camera3d_frame(eye, look_at, view_up, name());
}

std::string Camera3DInterpolant::step()
{
    const int frame{m_step};
    ++m_step;

    const Camera3DFrame camera{frame_at(frame)};
    if (m_output_kind == Camera3DOutputKind::ID_ROTATION)
    {
        return id_camera3d_rotation(camera);
    }
    if (m_output_kind == Camera3DOutputKind::ID_PERSPECTIVE)
    {
        return id_camera3d_perspective(camera);
    }
    if (m_output_kind == Camera3DOutputKind::ID_XYSHIFT)
    {
        return id_camera3d_xyshift(camera);
    }
    return julibrot_camera3d_geometry(camera, m_base_value);
}

class IntegerInterpolant : public Base
{
public:
    IntegerInterpolant(const ParameterMetadata &metadata, const std::vector<KeyframeConfig> &keys, Curve curve,
        std::string_view base_value, int num_steps);
    ~IntegerInterpolant() override = default;

    std::string step() override;

private:
    int m_from_frame{};
    int m_to_frame{};
    int m_from{};
    int m_to{};
    std::string m_base;
    Curve m_curve{};
    ExtrapolateMode m_extrapolate{};
};

IntegerInterpolant::IntegerInterpolant(const ParameterMetadata &metadata, const std::vector<KeyframeConfig> &keys,
    Curve curve, std::string_view base_value, int num_steps) :
    Base(metadata.name, num_steps),
    m_from_frame(keys[0].frame),
    m_to_frame(keys[1].frame),
    m_from(parse_integer(keys[0].value)),
    m_to(parse_integer(keys[1].value)),
    m_base(base_value),
    m_curve(curve),
    m_extrapolate(extrapolate_mode(metadata))
{
    validate_scalar_curve("integer", m_curve);
    validate_bounds(metadata, m_from);
    validate_bounds(metadata, m_to);
}

std::string IntegerInterpolant::step()
{
    const int frame{m_step};
    ++m_step;
    m_has_value = true;
    if (frame < m_from_frame || frame > m_to_frame)
    {
        if (m_extrapolate == ExtrapolateMode::BASE)
        {
            return m_base;
        }
        if (m_extrapolate == ExtrapolateMode::OMIT)
        {
            m_has_value = false;
            return {};
        }
    }
    const int sample_frame{extrapolated_frame(frame, m_from_frame, m_to_frame, m_extrapolate)};
    if (sample_frame <= m_from_frame)
    {
        return std::to_string(m_from);
    }
    if (sample_frame >= m_to_frame)
    {
        return std::to_string(m_to);
    }
    if (m_curve == Curve::HOLD || m_curve == Curve::STEP)
    {
        return std::to_string(m_from);
    }
    const double fraction{(sample_frame - m_from_frame) / static_cast<double>(m_to_frame - m_from_frame)};
    const double value{m_from + fraction * (m_to - m_from)};
    return std::to_string(static_cast<int>(std::lround(value)));
}

class DoubleInterpolant : public Base
{
public:
    DoubleInterpolant(const ParameterMetadata &metadata, const std::vector<KeyframeConfig> &keys, Curve curve,
        std::string_view base_value, int num_steps);
    ~DoubleInterpolant() override = default;

    std::string step() override;

private:
    int m_from_frame{};
    int m_to_frame{};
    double m_from{};
    double m_to{};
    std::string m_base;
    Curve m_curve{};
    ExtrapolateMode m_extrapolate{};
};

DoubleInterpolant::DoubleInterpolant(const ParameterMetadata &metadata, const std::vector<KeyframeConfig> &keys,
    Curve curve, std::string_view base_value, int num_steps) :
    Base(metadata.name, num_steps),
    m_from_frame(keys[0].frame),
    m_to_frame(keys[1].frame),
    m_from(parse_double(keys[0].value)),
    m_to(parse_double(keys[1].value)),
    m_base(base_value),
    m_curve(curve),
    m_extrapolate(extrapolate_mode(metadata))
{
    validate_scalar_curve("double", m_curve);
    validate_bounds(metadata, m_from);
    validate_bounds(metadata, m_to);
}

std::string DoubleInterpolant::step()
{
    const int frame{m_step};
    ++m_step;
    m_has_value = true;
    if (frame < m_from_frame || frame > m_to_frame)
    {
        if (m_extrapolate == ExtrapolateMode::BASE)
        {
            return m_base;
        }
        if (m_extrapolate == ExtrapolateMode::OMIT)
        {
            m_has_value = false;
            return {};
        }
    }
    const int sample_frame{extrapolated_frame(frame, m_from_frame, m_to_frame, m_extrapolate)};
    double value{m_from};
    if (sample_frame >= m_to_frame)
    {
        value = m_to;
    }
    else if (sample_frame > m_from_frame && m_curve != Curve::HOLD && m_curve != Curve::STEP)
    {
        const double fraction{(sample_frame - m_from_frame) / static_cast<double>(m_to_frame - m_from_frame)};
        value = m_from + fraction * (m_to - m_from);
    }
    return format_double(value);
}

class ParamsIntegerInterpolant : public Base
{
public:
    ParamsIntegerInterpolant(const ResolvedTrack &track, Curve curve, int num_steps);
    ~ParamsIntegerInterpolant() override = default;

    std::string step() override;

private:
    int m_from_frame{};
    int m_to_frame{};
    int m_slot{};
    int m_from{};
    int m_to{};
    std::vector<double> m_base_values;
    Curve m_curve{};
};

ParamsIntegerInterpolant::ParamsIntegerInterpolant(const ResolvedTrack &track, Curve curve, int num_steps) :
    Base(track.output_parameter, num_steps, track.slots),
    m_from_frame(track.keys[0].frame),
    m_to_frame(track.keys[1].frame),
    m_from(parse_integer(track.keys[0].value)),
    m_to(parse_integer(track.keys[1].value)),
    m_base_values(parse_slash_doubles(track.base_value)),
    m_curve(curve)
{
    if (track.slots.size() != 1U)
    {
        throw std::runtime_error("Track '" + track.parameter + "' requires exactly one params slot");
    }
    m_slot = track.slots[0];
    validate_scalar_curve("integer", m_curve);
    validate_bounds(track.metadata, m_from);
    validate_bounds(track.metadata, m_to);
    validate_params_slot(track.parameter, m_base_values, m_slot);
}

std::string ParamsIntegerInterpolant::step()
{
    const int frame{m_step};
    ++m_step;

    int value{m_from};
    if (frame >= m_to_frame)
    {
        value = m_to;
    }
    else if (frame > m_from_frame && m_curve != Curve::HOLD && m_curve != Curve::STEP)
    {
        const double fraction{(frame - m_from_frame) / static_cast<double>(m_to_frame - m_from_frame)};
        value = static_cast<int>(std::lround(m_from + fraction * (m_to - m_from)));
    }

    std::vector<double> values{m_base_values};
    values[static_cast<std::size_t>(m_slot)] = value;
    return format_slash_doubles(values);
}

class ParamsDoubleInterpolant : public Base
{
public:
    ParamsDoubleInterpolant(const ResolvedTrack &track, Curve curve, int num_steps);
    ~ParamsDoubleInterpolant() override = default;

    std::string step() override;

private:
    int m_from_frame{};
    int m_to_frame{};
    int m_slot{};
    double m_from{};
    double m_to{};
    std::vector<double> m_base_values;
    Curve m_curve{};
};

ParamsDoubleInterpolant::ParamsDoubleInterpolant(const ResolvedTrack &track, Curve curve, int num_steps) :
    Base(track.output_parameter, num_steps, track.slots),
    m_from_frame(track.keys[0].frame),
    m_to_frame(track.keys[1].frame),
    m_from(parse_double(track.keys[0].value)),
    m_to(parse_double(track.keys[1].value)),
    m_base_values(parse_slash_doubles(track.base_value)),
    m_curve(curve)
{
    if (track.slots.size() != 1U)
    {
        throw std::runtime_error("Track '" + track.parameter + "' requires exactly one params slot");
    }
    m_slot = track.slots[0];
    validate_scalar_curve("double", m_curve);
    validate_bounds(track.metadata, m_from);
    validate_bounds(track.metadata, m_to);
    validate_params_slot(track.parameter, m_base_values, m_slot);
}

std::string ParamsDoubleInterpolant::step()
{
    const int frame{m_step};
    ++m_step;

    double value{m_from};
    if (frame >= m_to_frame)
    {
        value = m_to;
    }
    else if (frame > m_from_frame && m_curve != Curve::HOLD && m_curve != Curve::STEP)
    {
        const double fraction{(frame - m_from_frame) / static_cast<double>(m_to_frame - m_from_frame)};
        value = m_from + fraction * (m_to - m_from);
    }

    std::vector<double> values{m_base_values};
    values[static_cast<std::size_t>(m_slot)] = value;
    return format_slash_doubles(values);
}

class ParamsComplexInterpolant : public Base
{
public:
    ParamsComplexInterpolant(const ResolvedTrack &track, Curve curve, int num_steps);
    ~ParamsComplexInterpolant() override = default;

    std::string step() override;

private:
    int m_from_frame{};
    int m_to_frame{};
    std::vector<int> m_slots;
    std::complex<double> m_from;
    std::complex<double> m_to;
    std::vector<double> m_base_values;
    Curve m_curve{};
};

ParamsComplexInterpolant::ParamsComplexInterpolant(const ResolvedTrack &track, Curve curve, int num_steps) :
    Base(track.output_parameter, num_steps, track.slots),
    m_from_frame(track.keys[0].frame),
    m_to_frame(track.keys[1].frame),
    m_slots(track.slots),
    m_from(parse_slash_pair(track.keys[0].value)),
    m_to(parse_slash_pair(track.keys[1].value)),
    m_base_values(parse_slash_doubles(track.base_value)),
    m_curve(curve)
{
    if (m_slots.size() != 2U)
    {
        throw std::runtime_error("Track '" + track.parameter + "' requires exactly two params slots");
    }
    validate_scalar_curve("complex", m_curve);
    validate_params_slot(track.parameter, m_base_values, m_slots[0]);
    validate_params_slot(track.parameter, m_base_values, m_slots[1]);
}

std::string ParamsComplexInterpolant::step()
{
    const int frame{m_step};
    ++m_step;

    std::complex<double> value{m_from};
    if (frame >= m_to_frame)
    {
        value = m_to;
    }
    else if (frame > m_from_frame && m_curve != Curve::HOLD && m_curve != Curve::STEP)
    {
        const double fraction{(frame - m_from_frame) / static_cast<double>(m_to_frame - m_from_frame)};
        value = m_from + fraction * (m_to - m_from);
    }

    std::vector<double> values{m_base_values};
    values[static_cast<std::size_t>(m_slots[0])] = value.real();
    values[static_cast<std::size_t>(m_slots[1])] = value.imag();
    return format_slash_doubles(values);
}

class NumericTupleInterpolant : public Base
{
public:
    NumericTupleInterpolant(
        const ParameterMetadata &metadata, const std::vector<KeyframeConfig> &keys, Curve curve, int num_steps);
    ~NumericTupleInterpolant() override = default;

    std::string step() override;

private:
    int m_from_frame{};
    int m_to_frame{};
    std::vector<double> m_from;
    std::vector<double> m_to;
    Curve m_curve{};
    ParameterMetadata m_metadata;
};

NumericTupleInterpolant::NumericTupleInterpolant(
    const ParameterMetadata &metadata, const std::vector<KeyframeConfig> &keys, Curve curve, int num_steps) :
    Base(metadata.name, num_steps),
    m_from_frame(keys[0].frame),
    m_to_frame(keys[1].frame),
    m_from(parse_slash_doubles(keys[0].value)),
    m_to(parse_slash_doubles(keys[1].value)),
    m_curve(curve),
    m_metadata(metadata)
{
    const std::size_t arity{static_cast<std::size_t>(tuple_arity(metadata))};
    if (m_from.size() != arity || m_to.size() != arity)
    {
        throw std::runtime_error(
            "Numeric tuple parameter '" + metadata.name + "' requires " + std::to_string(arity) + " values");
    }
    validate_scalar_curve("numeric-tuple", m_curve);
    for (double value : m_from)
    {
        validate_bounds(metadata, value);
    }
    for (double value : m_to)
    {
        validate_bounds(metadata, value);
    }
}

std::string NumericTupleInterpolant::step()
{
    const int frame{m_step};
    ++m_step;

    std::vector<double> values{m_from};
    if (frame >= m_to_frame)
    {
        values = m_to;
    }
    else if (frame > m_from_frame && m_curve != Curve::HOLD && m_curve != Curve::STEP)
    {
        const double fraction{(frame - m_from_frame) / static_cast<double>(m_to_frame - m_from_frame)};
        for (std::size_t i = 0; i < values.size(); ++i)
        {
            values[i] = m_from[i] + fraction * (m_to[i] - m_from[i]);
        }
    }
    normalize_vector(m_metadata, values);
    return format_slash_doubles(values);
}

class DiscreteInterpolant : public Base
{
public:
    DiscreteInterpolant(
        const ParameterMetadata &metadata, const std::vector<KeyframeConfig> &keys, Curve curve, int num_steps);
    ~DiscreteInterpolant() override = default;

    std::string step() override;

private:
    int m_to_frame{};
    std::string m_from;
    std::string m_to;
    Curve m_curve{};
};

DiscreteInterpolant::DiscreteInterpolant(
    const ParameterMetadata &metadata, const std::vector<KeyframeConfig> &keys, Curve curve, int num_steps) :
    Base(metadata.name, num_steps),
    m_to_frame(keys[1].frame),
    m_from(keys[0].value),
    m_to(keys[1].value),
    m_curve(curve)
{
    validate_discrete_curve(to_string(metadata.type), m_curve);
    if (has_validated_discrete_values(metadata.type))
    {
        validate_discrete_value(metadata, m_from);
        validate_discrete_value(metadata, m_to);
    }
}

std::string DiscreteInterpolant::step()
{
    const int frame{m_step};
    ++m_step;
    return frame >= m_to_frame ? m_to : m_from;
}

class DiscretePwmInterpolant : public Base
{
public:
    DiscretePwmInterpolant(const ResolvedTrack &track, int num_steps);
    ~DiscretePwmInterpolant() override = default;

    std::string step() override;

private:
    int m_from_frame{};
    int m_to_frame{};
    std::string m_a;
    std::string m_b;
    int m_window{};
    double m_from_mix{};
    double m_to_mix{};
};

DiscretePwmInterpolant::DiscretePwmInterpolant(const ResolvedTrack &track, int num_steps) :
    Base(track.output_parameter, num_steps, track.slots),
    m_from_frame(track.keys[0].frame),
    m_to_frame(track.keys[1].frame)
{
    if (!track.pwm)
    {
        throw std::runtime_error("PWM track '" + track.parameter + "' is missing pwm settings");
    }
    if (!track.slots.empty())
    {
        throw std::runtime_error("PWM track '" + track.parameter + "' cannot target slotted output");
    }
    if (track.pwm->window < 2)
    {
        throw std::runtime_error("PWM track '" + track.parameter + "' window must be at least 2");
    }

    m_a = track.pwm->a;
    m_b = track.pwm->b;
    m_window = track.pwm->window;
    m_from_mix = validate_mix(track.parameter, track.keys[0]);
    m_to_mix = validate_mix(track.parameter, track.keys[1]);
    validate_discrete_value(track.metadata, m_a);
    validate_discrete_value(track.metadata, m_b);
}

std::string DiscretePwmInterpolant::step()
{
    const int frame{m_step};
    ++m_step;

    double mix{m_from_mix};
    if (frame >= m_to_frame)
    {
        mix = m_to_mix;
    }
    else if (frame > m_from_frame)
    {
        const double fraction{(frame - m_from_frame) / static_cast<double>(m_to_frame - m_from_frame)};
        mix = m_from_mix + fraction * (m_to_mix - m_from_mix);
    }

    const int b_count{static_cast<int>(std::lround(mix * m_window))};
    if (b_count <= 0)
    {
        return m_a;
    }
    if (b_count >= m_window)
    {
        return m_b;
    }
    return positive_mod(frame - m_from_frame, m_window) < b_count ? m_b : m_a;
}

class FunctionEnumInterpolant : public Base
{
public:
    FunctionEnumInterpolant(const ResolvedTrack &track, Curve curve, int num_steps);
    ~FunctionEnumInterpolant() override = default;

    std::string step() override;

private:
    int m_from_frame{};
    int m_to_frame{};
    int m_slot{};
    std::string m_from;
    std::string m_to;
    std::vector<std::string> m_base_values;
    Curve m_curve{};
};

FunctionEnumInterpolant::FunctionEnumInterpolant(const ResolvedTrack &track, Curve curve, int num_steps) :
    Base(track.output_parameter, num_steps, track.slots),
    m_from_frame(track.keys[0].frame),
    m_to_frame(track.keys[1].frame),
    m_from(track.keys[0].value),
    m_to(track.keys[1].value),
    m_base_values(split_slash_strings(track.base_value)),
    m_curve(curve)
{
    if (track.slots.size() != 1U)
    {
        throw std::runtime_error("Track '" + track.parameter + "' requires exactly one function slot");
    }
    m_slot = track.slots[0];
    validate_discrete_curve("enum", m_curve);
    validate_enum_value(track.metadata, m_from);
    validate_enum_value(track.metadata, m_to);
    if (m_slot < 0)
    {
        throw std::runtime_error("Track '" + track.parameter + "' has an invalid function slot");
    }
    if (static_cast<std::size_t>(m_slot) >= m_base_values.size())
    {
        m_base_values.resize(static_cast<std::size_t>(m_slot) + 1U, "ident");
    }
}

std::string FunctionEnumInterpolant::step()
{
    const int frame{m_step};
    ++m_step;

    std::vector<std::string> values{m_base_values};
    values[static_cast<std::size_t>(m_slot)] = frame >= m_to_frame ? m_to : m_from;
    return format_slash_strings(values);
}

} // namespace

static InterpolantPtr create_path_interpolant(const ResolvedTrack &track, int num_steps)
{
    if (!track.path || !is_explicit_path(track.path->kind))
    {
        throw std::runtime_error("Track '" + track.parameter + "' has no supported path generator");
    }

    if (is_control_point_path(track.path->kind))
    {
        switch (track.metadata.type)
        {
        case ParameterType::COMPLEX:
        case ParameterType::NUMERIC_TUPLE:
        case ParameterType::POINT2:
        case ParameterType::POINT3:
        case ParameterType::VECTOR2:
        case ParameterType::VECTOR3:
            if (track.output_parameter == "params")
            {
                return std::make_shared<ParamsTuplePathInterpolant>(track, num_steps);
            }
            return std::make_shared<TuplePathInterpolant>(track, num_steps);
        default:
            break;
        }
        throw std::runtime_error(
            "Control point path track '" + track.parameter + "' requires a complex or tuple target");
    }

    switch (track.metadata.type)
    {
    case ParameterType::COMPLEX:
        if (track.output_parameter == "params")
        {
            return std::make_shared<ParamsComplexPathInterpolant>(track, num_steps);
        }
        return std::make_shared<ComplexPathInterpolant>(track, num_steps);
    case ParameterType::POINT2:
        return std::make_shared<Point2PathInterpolant>(track, num_steps);
    default:
        break;
    }
    throw std::runtime_error("Path track '" + track.parameter + "' requires a complex or point2 target");
}

InterpolantPtr create_interpolant(const ResolvedTrack &track, int num_steps)
{
    if (track.camera2d)
    {
        return std::make_shared<Camera2DInterpolant>(track, num_steps);
    }
    if (track.camera3d)
    {
        return std::make_shared<Camera3DInterpolant>(track, num_steps);
    }

    const ParameterMetadata &metadata{track.metadata};
    const std::vector<KeyframeConfig> &keys{track.keys};
    if (track.path && keys.empty() && is_explicit_path(track.path->kind))
    {
        return create_path_interpolant(track, num_steps);
    }
    validate_keyframes(metadata.name, keys, num_steps);
    if (track.mode == TrackMode::PWM)
    {
        if (!is_pwm_type(metadata.type))
        {
            throw std::runtime_error("PWM track '" + track.parameter + "' requires an enum, inside, or outside target");
        }
        validate_full_range(metadata.name, keys, num_steps);
        return std::make_shared<DiscretePwmInterpolant>(track, num_steps);
    }
    switch (metadata.type)
    {
    case ParameterType::CENTER_MAG:
        validate_full_range(metadata.name, keys, num_steps);
        return std::make_shared<CenterMagInterpolant>(metadata.name, keys[0].value, keys[1].value, num_steps);
    case ParameterType::COLOR_MAP:
        throw std::runtime_error("Color-map parameter '" + metadata.name + "' must use a color-map track");
    case ParameterType::COMPLEX:
    {
        validate_full_range(metadata.name, keys, num_steps);
        Curve curve{default_curve(metadata)};
        if (keys[1].curve)
        {
            curve = *keys[1].curve;
        }
        if (track.output_parameter == "params")
        {
            return std::make_shared<ParamsComplexInterpolant>(track, curve, num_steps);
        }
        break;
    }
    case ParameterType::CORNERS:
        validate_full_range(metadata.name, keys, num_steps);
        return std::make_shared<CornersInterpolant>(metadata.name, keys[0].value, keys[1].value, num_steps);
    case ParameterType::INTEGER:
    {
        Curve curve{default_curve(metadata)};
        if (keys[1].curve)
        {
            curve = *keys[1].curve;
        }
        if (track.output_parameter == "params")
        {
            validate_full_range(metadata.name, keys, num_steps);
            return std::make_shared<ParamsIntegerInterpolant>(track, curve, num_steps);
        }
        return std::make_shared<IntegerInterpolant>(metadata, keys, curve, track.base_value, num_steps);
    }
    case ParameterType::DOUBLE:
    {
        Curve curve{default_curve(metadata)};
        if (keys[1].curve)
        {
            curve = *keys[1].curve;
        }
        if (track.output_parameter == "params")
        {
            validate_full_range(metadata.name, keys, num_steps);
            return std::make_shared<ParamsDoubleInterpolant>(track, curve, num_steps);
        }
        return std::make_shared<DoubleInterpolant>(metadata, keys, curve, track.base_value, num_steps);
    }
    case ParameterType::INSIDE:
    case ParameterType::OUTSIDE:
    {
        validate_full_range(metadata.name, keys, num_steps);
        Curve curve{default_curve(metadata)};
        if (keys[1].curve)
        {
            curve = *keys[1].curve;
        }
        return std::make_shared<DiscreteInterpolant>(metadata, keys, curve, num_steps);
    }
    case ParameterType::NUMERIC_TUPLE:
    case ParameterType::POINT2:
    case ParameterType::POINT3:
    case ParameterType::VECTOR2:
    case ParameterType::VECTOR3:
    {
        validate_full_range(metadata.name, keys, num_steps);
        Curve curve{default_curve(metadata)};
        if (keys[1].curve)
        {
            curve = *keys[1].curve;
        }
        return std::make_shared<NumericTupleInterpolant>(metadata, keys, curve, num_steps);
    }
    case ParameterType::STRING:
    {
        validate_full_range(metadata.name, keys, num_steps);
        Curve curve{default_curve(metadata)};
        if (keys[1].curve)
        {
            curve = *keys[1].curve;
        }
        return std::make_shared<DiscreteInterpolant>(metadata, keys, curve, num_steps);
    }
    case ParameterType::ENUM:
    {
        validate_full_range(metadata.name, keys, num_steps);
        Curve curve{default_curve(metadata)};
        if (keys[1].curve)
        {
            curve = *keys[1].curve;
        }
        if (track.output_parameter == "function")
        {
            return std::make_shared<FunctionEnumInterpolant>(track, curve, num_steps);
        }
        return std::make_shared<DiscreteInterpolant>(metadata, keys, curve, num_steps);
    }
    }

    throw std::runtime_error(
        "Unknown track type '" + std::string{to_string(metadata.type)} + "' for parameter '" + metadata.name + "'");
}

} // namespace ParFile
