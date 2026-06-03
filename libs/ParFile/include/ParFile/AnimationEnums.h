// SPDX-License-Identifier: GPL-3.0-only
//
#pragma once

#include <string_view>

namespace ParFile
{

enum class ParameterType
{
    CENTER_MAG,
    COMPLEX,
    CORNERS,
    DOUBLE,
    ENUM,
    INTEGER
};

enum class ParameterFormat
{
    RAW,
    SLASH,
    SLASH_LIST,
    SLASH_PAIR
};

enum class Curve
{
    GEOMETRIC,
    HOLD,
    LINEAR,
    STEP
};

enum class ExtrapolateMode
{
    BASE,
    CLAMP,
    CYCLE,
    OMIT,
    PING_PONG
};

ParameterType parse_parameter_type(std::string_view text);
ParameterFormat parse_parameter_format(std::string_view text);
Curve parse_curve(std::string_view text);
ExtrapolateMode parse_extrapolate_mode(std::string_view text);

std::string_view to_string(ParameterType value);
std::string_view to_string(ParameterFormat value);
std::string_view to_string(Curve value);
std::string_view to_string(ExtrapolateMode value);

} // namespace ParFile
