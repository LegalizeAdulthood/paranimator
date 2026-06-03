// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/AnimationEnums.h>

#include <stdexcept>
#include <string>
#include <string_view>

namespace ParFile
{

ParameterType parse_parameter_type(std::string_view text)
{
    if (text == "center-mag")
    {
        return ParameterType::CENTER_MAG;
    }
    if (text == "corners")
    {
        return ParameterType::CORNERS;
    }
    if (text == "complex")
    {
        return ParameterType::COMPLEX;
    }
    if (text == "double")
    {
        return ParameterType::DOUBLE;
    }
    if (text == "enum")
    {
        return ParameterType::ENUM;
    }
    if (text == "integer")
    {
        return ParameterType::INTEGER;
    }
    if (text == "numeric-tuple")
    {
        return ParameterType::NUMERIC_TUPLE;
    }
    throw std::runtime_error("Unknown parameter type '" + std::string{text} + "'");
}

ParameterFormat parse_parameter_format(std::string_view text)
{
    if (text == "raw")
    {
        return ParameterFormat::RAW;
    }
    if (text == "slash")
    {
        return ParameterFormat::SLASH;
    }
    if (text == "slash-list")
    {
        return ParameterFormat::SLASH_LIST;
    }
    if (text == "slash-pair")
    {
        return ParameterFormat::SLASH_PAIR;
    }
    throw std::runtime_error("Unknown parameter format '" + std::string{text} + "'");
}

Curve parse_curve(std::string_view text)
{
    if (text == "geometric")
    {
        return Curve::GEOMETRIC;
    }
    if (text == "hold")
    {
        return Curve::HOLD;
    }
    if (text == "linear")
    {
        return Curve::LINEAR;
    }
    if (text == "step")
    {
        return Curve::STEP;
    }
    throw std::runtime_error("Unknown curve '" + std::string{text} + "'");
}

ExtrapolateMode parse_extrapolate_mode(std::string_view text)
{
    if (text == "base")
    {
        return ExtrapolateMode::BASE;
    }
    if (text == "clamp")
    {
        return ExtrapolateMode::CLAMP;
    }
    if (text == "cycle")
    {
        return ExtrapolateMode::CYCLE;
    }
    if (text == "omit")
    {
        return ExtrapolateMode::OMIT;
    }
    if (text == "ping-pong")
    {
        return ExtrapolateMode::PING_PONG;
    }
    throw std::runtime_error("Unknown extrapolate mode '" + std::string{text} + "'");
}

std::string_view to_string(ParameterType value)
{
    switch (value)
    {
    case ParameterType::CENTER_MAG:
        return "center-mag";
    case ParameterType::COMPLEX:
        return "complex";
    case ParameterType::CORNERS:
        return "corners";
    case ParameterType::DOUBLE:
        return "double";
    case ParameterType::ENUM:
        return "enum";
    case ParameterType::INTEGER:
        return "integer";
    case ParameterType::NUMERIC_TUPLE:
        return "numeric-tuple";
    }
    return {};
}

std::string_view to_string(ParameterFormat value)
{
    switch (value)
    {
    case ParameterFormat::RAW:
        return "raw";
    case ParameterFormat::SLASH:
        return "slash";
    case ParameterFormat::SLASH_LIST:
        return "slash-list";
    case ParameterFormat::SLASH_PAIR:
        return "slash-pair";
    }
    return {};
}

std::string_view to_string(Curve value)
{
    switch (value)
    {
    case Curve::GEOMETRIC:
        return "geometric";
    case Curve::HOLD:
        return "hold";
    case Curve::LINEAR:
        return "linear";
    case Curve::STEP:
        return "step";
    }
    return {};
}

std::string_view to_string(ExtrapolateMode value)
{
    switch (value)
    {
    case ExtrapolateMode::BASE:
        return "base";
    case ExtrapolateMode::CLAMP:
        return "clamp";
    case ExtrapolateMode::CYCLE:
        return "cycle";
    case ExtrapolateMode::OMIT:
        return "omit";
    case ExtrapolateMode::PING_PONG:
        return "ping-pong";
    }
    return {};
}

} // namespace ParFile
