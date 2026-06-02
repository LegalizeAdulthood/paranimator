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
    Base(const Base &rhs) = delete;
    Base &operator=(const Base &rhs) = delete;
    Base &operator=(Base &&rhs) = delete;
    ~Base() override = default;

    const std::string &name() const override
    {
        return m_name;
    }

protected:
    std::string m_name;
    int m_step{};
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
    std::vector<std::string> text;
    boost::algorithm::split(text, value, [](char c) { return c == '/'; });
    if (text.size() != 4U && text.size() != 6U)
    {
        throw std::runtime_error(
            "Corners parameter must have 4 or 6 values; have " + std::to_string(text.size()) + " in '" + value + "'");
    }
    values.resize(text.size());
    std::transform(text.begin(), text.end(), values.begin(), [](const std::string &item) { return std::stod(item); });
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
        result += (boost::format("%.12g") % value).str();
    }
    return result;
}

class IntegerInterpolant : public Base
{
public:
    IntegerInterpolant(std::string_view name, const std::vector<KeyframeConfig> &keys, int num_steps);
    ~IntegerInterpolant() override = default;

    std::string step() override;

private:
    int m_from_frame{};
    int m_to_frame{};
    int m_from{};
    int m_to{};
};

IntegerInterpolant::IntegerInterpolant(std::string_view name, const std::vector<KeyframeConfig> &keys, int num_steps) :
    Base(name, num_steps),
    m_from_frame(keys[0].frame),
    m_to_frame(keys[1].frame),
    m_from(parse_integer(keys[0].value)),
    m_to(parse_integer(keys[1].value))
{
}

std::string IntegerInterpolant::step()
{
    const int frame{m_step};
    ++m_step;
    if (frame <= m_from_frame)
    {
        return std::to_string(m_from);
    }
    if (frame >= m_to_frame)
    {
        return std::to_string(m_to);
    }
    const double fraction{(frame - m_from_frame) / static_cast<double>(m_to_frame - m_from_frame)};
    const double value{m_from + fraction * (m_to - m_from)};
    return std::to_string(static_cast<int>(std::lround(value)));
}

} // namespace

InterpolantPtr create_interpolant(
    const ParameterMetadata &metadata, const std::vector<KeyframeConfig> &keys, int num_steps)
{
    validate_keyframes(metadata.name, keys, num_steps);
    if (metadata.type == "center_mag")
    {
        validate_full_range(metadata.name, keys, num_steps);
        return std::make_shared<CenterMagInterpolant>(metadata.name, keys[0].value, keys[1].value, num_steps);
    }
    if (metadata.type == "corners")
    {
        validate_full_range(metadata.name, keys, num_steps);
        return std::make_shared<CornersInterpolant>(metadata.name, keys[0].value, keys[1].value, num_steps);
    }
    if (metadata.type == "integer")
    {
        return std::make_shared<IntegerInterpolant>(metadata.name, keys, num_steps);
    }

    throw std::runtime_error("Unknown track type '" + metadata.type + "' for parameter '" + metadata.name + "'");
}

} // namespace ParFile
