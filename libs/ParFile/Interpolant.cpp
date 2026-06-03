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
#include <stdexcept>
#include <string>
#include <string_view>
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
    return kind == PathKind::CIRCLE || kind == PathKind::ELLIPSE;
}

double clean_path_component(double value)
{
    return std::abs(value) < 1.0e-12 ? 0.0 : value;
}

std::string format_slash_pair(const std::complex<double> &value)
{
    return format_slash_doubles({clean_path_component(value.real()), clean_path_component(value.imag())});
}

class PlanarPathEvaluator
{
public:
    PlanarPathEvaluator(const PathConfig &path, int num_steps);

    std::complex<double> value_at(int frame) const;

private:
    std::complex<double> m_center;
    double m_x_radius{};
    double m_y_radius{};
    double m_turns{};
    double m_phase{};
    int m_num_steps{};
};

PlanarPathEvaluator::PlanarPathEvaluator(const PathConfig &path, int num_steps) :
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
    }
    else if (path.kind == PathKind::ELLIPSE)
    {
        m_x_radius = path.x_radius;
        m_y_radius = path.y_radius;
    }
    else
    {
        throw std::runtime_error("Planar path requires a circle or ellipse path");
    }
}

std::complex<double> PlanarPathEvaluator::value_at(int frame) const
{
    constexpr double PI{3.141592653589793238462643383279502884};
    const double fraction{frame / static_cast<double>(m_num_steps - 1)};
    const double radians{(m_phase + 360.0 * m_turns * fraction) * PI / 180.0};
    return {m_center.real() + std::cos(radians) * m_x_radius, m_center.imag() + std::sin(radians) * m_y_radius};
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
};

CenterMag::CenterMag(const std::string &value)
{
    std::vector<std::string> value_text;
    boost::algorithm::split(value_text, value, [](char c) { return c == '/'; });
    if (value_text.size() < 3)
    {
        throw std::runtime_error("Insufficient values for center-mag parameter; need 3, have " +
            std::to_string(value_text.size()) + " in '" + value + "'");
    }
    std::vector<double> values(value_text.size());
    std::transform(
        value_text.begin(), value_text.end(), values.begin(), [](const std::string &text) { return std::stod(text); });
    center = std::complex<double>(values[0], values[1]);
    mag = values[2];
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
    const std::complex<double> center{m_segment.linear_value_at(m_step, m_from.center, m_to.center)};
    double mag;
    if (m_from.mag > 0.0 && m_to.mag > 0.0)
    {
        mag = m_segment.geometric_value_at(m_step, m_from.mag, m_to.mag);
    }
    else
    {
        // Fallback to linear interpolation for non-positive magnifications
        mag = m_segment.linear_value_at(m_step, m_from.mag, m_to.mag);
    }
    return (boost::format("%g/%g/%g") % center.real() % center.imag() % mag).str();
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
    validate_discrete_value(metadata, m_from);
    validate_discrete_value(metadata, m_to);
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
    if (!track.path || !is_planar_path(track.path->kind))
    {
        throw std::runtime_error("Track '" + track.parameter + "' has no supported path generator");
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
    const ParameterMetadata &metadata{track.metadata};
    const std::vector<KeyframeConfig> &keys{track.keys};
    if (track.path && is_planar_path(track.path->kind))
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
