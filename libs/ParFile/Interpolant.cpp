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
    Base(std::string_view name) :
        m_name(name)
    {
    }
    ~Base() override = default;

    const std::string &name() const override
    {
        return m_name;
    }
    
private:
    std::string m_name;
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
    int m_step{};
    int m_num_steps;
};

CenterMagInterpolant::CenterMagInterpolant(const std::string &from, const std::string &to, int num_steps) :
    Base("center-mag"),
    m_from(from),
    m_to(to),
    m_num_steps(num_steps)
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

} // namespace

InterpolantPtr create_interpolant(
    const std::string &name, const std::string &from, const std::string &to, int num_steps)
{
    if (name == "center-mag")
    {
        return std::make_shared<CenterMagInterpolant>(from, to, num_steps);
    }

    throw std::runtime_error("Unknown interpolant '" + name + "'");
}

} // namespace ParFile
