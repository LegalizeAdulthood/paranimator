// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/Interpolant.h>

#include <ParFile/ParFile.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/format.hpp>

#include <algorithm>
#include <cmath>
#include <complex>
#include <cstddef>
#include <stdexcept>
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
    CenterMagInterpolant(const std::string &from, const std::string &to, int num_steps);
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

CenterMagInterpolant::CenterMagInterpolant(const std::string &from, const std::string &to, int num_steps) :
    Base("center-mag", num_steps),
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
    CornersInterpolant(const std::string &from, const std::string &to, int num_steps);
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

CornersInterpolant::CornersInterpolant(const std::string &from, const std::string &to, int num_steps) :
    Base("corners", num_steps),
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

} // namespace

InterpolantPtr create_interpolant(
    const std::string &name, const std::string &from, const std::string &to, int num_steps)
{
    if (name == "center-mag")
    {
        return std::make_shared<CenterMagInterpolant>(from, to, num_steps);
    }
    if (name == "corners")
    {
        return std::make_shared<CornersInterpolant>(from, to, num_steps);
    }

    throw std::runtime_error("Unknown interpolant '" + name + "'");
}

} // namespace ParFile
