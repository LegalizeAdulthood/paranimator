// SPDX-License-Identifier: GPL-3.0-only
//
#pragma once

#include <string_view>

namespace ParFile
{

enum class ParameterType
{
    CENTER_MAG,
    COLOR_MAP,
    COMPLEX,
    CORNERS,
    DOUBLE,
    ENUM,
    INSIDE,
    INTEGER,
    NUMERIC_TUPLE,
    OUTSIDE,
    POINT2,
    POINT3,
    STRING,
    VECTOR2,
    VECTOR3
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

enum class TrackMode
{
    KEYFRAMES,
    PWM
};

enum class TrackKind
{
    PARAMETER,
    CAMERA2D,
    COLOR_MAP,
    ID_3D_VIEW,
    JULIBROT_VIEW
};

enum class TrackFormat
{
    AT_FILE
};

ParameterType parse_parameter_type(std::string_view text);
ParameterFormat parse_parameter_format(std::string_view text);
Curve parse_curve(std::string_view text);
ExtrapolateMode parse_extrapolate_mode(std::string_view text);
TrackMode parse_track_mode(std::string_view text);
TrackKind parse_track_kind(std::string_view text);
TrackFormat parse_track_format(std::string_view text);

std::string_view to_string(ParameterType value);
std::string_view to_string(ParameterFormat value);
std::string_view to_string(Curve value);
std::string_view to_string(ExtrapolateMode value);
std::string_view to_string(TrackMode value);
std::string_view to_string(TrackKind value);
std::string_view to_string(TrackFormat value);

} // namespace ParFile
