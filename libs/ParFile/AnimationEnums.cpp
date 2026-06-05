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
    if (text == "color-map")
    {
        return ParameterType::COLOR_MAP;
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
    if (text == "function-list")
    {
        return ParameterType::FUNCTION_LIST;
    }
    if (text == "yes-no")
    {
        return ParameterType::YES_NO;
    }
    if (text == "inside")
    {
        return ParameterType::INSIDE;
    }
    if (text == "integer")
    {
        return ParameterType::INTEGER;
    }
    if (text == "numeric-tuple")
    {
        return ParameterType::NUMERIC_TUPLE;
    }
    if (text == "outside")
    {
        return ParameterType::OUTSIDE;
    }
    if (text == "point2")
    {
        return ParameterType::POINT2;
    }
    if (text == "point3")
    {
        return ParameterType::POINT3;
    }
    if (text == "string")
    {
        return ParameterType::STRING;
    }
    if (text == "vector2")
    {
        return ParameterType::VECTOR2;
    }
    if (text == "vector3")
    {
        return ParameterType::VECTOR3;
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

TrackMode parse_track_mode(std::string_view text)
{
    if (text == "keyframes")
    {
        return TrackMode::KEYFRAMES;
    }
    if (text == "pwm")
    {
        return TrackMode::PWM;
    }
    throw std::runtime_error("Unknown track mode '" + std::string{text} + "'");
}

TrackKind parse_track_kind(std::string_view text)
{
    if (text == "parameter")
    {
        return TrackKind::PARAMETER;
    }
    if (text == "camera2d")
    {
        return TrackKind::CAMERA2D;
    }
    if (text == "color-map")
    {
        return TrackKind::COLOR_MAP;
    }
    if (text == "id-3d-view")
    {
        return TrackKind::ID_3D_VIEW;
    }
    if (text == "julibrot-view")
    {
        return TrackKind::JULIBROT_VIEW;
    }
    throw std::runtime_error("Unknown track kind '" + std::string{text} + "'");
}

TrackFormat parse_track_format(std::string_view text)
{
    if (text == "at-file")
    {
        return TrackFormat::AT_FILE;
    }
    throw std::runtime_error("Unknown track format '" + std::string{text} + "'");
}

std::string_view to_string(ParameterType value)
{
    switch (value)
    {
    case ParameterType::CENTER_MAG:
        return "center-mag";
    case ParameterType::COLOR_MAP:
        return "color-map";
    case ParameterType::COMPLEX:
        return "complex";
    case ParameterType::CORNERS:
        return "corners";
    case ParameterType::DOUBLE:
        return "double";
    case ParameterType::ENUM:
        return "enum";
    case ParameterType::FUNCTION_LIST:
        return "function-list";
    case ParameterType::INSIDE:
        return "inside";
    case ParameterType::INTEGER:
        return "integer";
    case ParameterType::NUMERIC_TUPLE:
        return "numeric-tuple";
    case ParameterType::OUTSIDE:
        return "outside";
    case ParameterType::POINT2:
        return "point2";
    case ParameterType::POINT3:
        return "point3";
    case ParameterType::STRING:
        return "string";
    case ParameterType::VECTOR2:
        return "vector2";
    case ParameterType::VECTOR3:
        return "vector3";
    case ParameterType::YES_NO:
        return "yes-no";
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

std::string_view to_string(TrackMode value)
{
    switch (value)
    {
    case TrackMode::KEYFRAMES:
        return "keyframes";
    case TrackMode::PWM:
        return "pwm";
    }
    return {};
}

std::string_view to_string(TrackKind value)
{
    switch (value)
    {
    case TrackKind::PARAMETER:
        return "parameter";
    case TrackKind::CAMERA2D:
        return "camera2d";
    case TrackKind::COLOR_MAP:
        return "color-map";
    case TrackKind::ID_3D_VIEW:
        return "id-3d-view";
    case TrackKind::JULIBROT_VIEW:
        return "julibrot-view";
    }
    return {};
}

std::string_view to_string(TrackFormat value)
{
    switch (value)
    {
    case TrackFormat::AT_FILE:
        return "at-file";
    }
    return {};
}

} // namespace ParFile
