// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/Interpolant.h>

#include <ParFile/ParFile.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/format.hpp>
#include <tweeny/tweeny.h>

#include <algorithm>
#include <complex>
#include <stdexcept>
#include <vector>

namespace ParFile
{

namespace
{

class Base : public Interpolant
{
public:
    Base() = default;
    Base(std::string_view name, int num_steps) :
        m_name(name),
        m_num_steps(num_steps)
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
    int m_num_steps{};
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
    const double fraction{(m_step - 1) / static_cast<double>(m_num_steps - 1)};
    const std::complex<double> center{m_from.center + fraction * (m_to.center - m_from.center)};
    const double mag{m_from.mag + fraction * (m_to.mag - m_from.mag)};
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

    std::complex<double> lower_left;
    std::complex<double> upper_right;
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
    text.reserve(4);
    boost::algorithm::split(text, value, [](char c) { return c == '/'; });
    std::vector<double> numbers(text.size());
    std::transform(text.begin(), text.end(), numbers.begin(), [](const std::string &text) { return std::stod(text); });
    lower_left.real(numbers[0]);
    upper_right.real(numbers[1]);
    lower_left.imag(numbers[2]);
    upper_right.imag(numbers[3]);
}

CornersInterpolant::CornersInterpolant(const std::string &from, const std::string &to, int num_steps) :
    Base("corners", num_steps),
    m_from(from),
    m_to(to)
{
}

std::string CornersInterpolant::step()
{
    ++m_step;
    const double fraction{(m_step - 1) / static_cast<double>(m_num_steps - 1)};
    const std::complex<double> ll{m_from.lower_left + fraction * (m_to.lower_left - m_from.lower_left)};
    const std::complex<double> ur{m_from.upper_right + fraction * (m_to.upper_right - m_from.upper_right)};
    return (boost::format("%.12g/%.12g/%.12g/%.12g") % ll.real() % ur.real() % ll.imag() % ur.imag()).str();
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
