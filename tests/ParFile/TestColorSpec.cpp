// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/ColorSpec.h>

#include <gtest/gtest.h>

#include <stdexcept>

namespace
{

void expect_color(const ParFile::RgbColor &color, int red, int green, int blue)
{
    EXPECT_EQ(red, color.red);
    EXPECT_EQ(green, color.green);
    EXPECT_EQ(blue, color.blue);
}

} // namespace

TEST(TestColorSpec, unprefixedRgbMatchesExplicitRgb)
{
    const ParFile::RgbColor unprefixed{ParFile::parse_color_spec("1/2/3")};
    const ParFile::RgbColor explicit_rgb{ParFile::parse_color_spec("rgb:1/2/3")};

    expect_color(unprefixed, explicit_rgb.red, explicit_rgb.green, explicit_rgb.blue);
}

TEST(TestColorSpec, cssNamedColorsConvertToRgb)
{
    expect_color(ParFile::parse_color_spec("RebeccaPurple"), 102, 51, 153);
    expect_color(ParFile::parse_color_spec("darkgrey"), 169, 169, 169);
    expect_color(ParFile::parse_color_spec("cyan"), 0, 255, 255);
}

TEST(TestColorSpec, hsvConvertsToRgb)
{
    expect_color(ParFile::parse_color_spec("hsv:120/1/1"), 0, 255, 0);
    expect_color(ParFile::parse_color_spec("hsv:20/1/1"), 255, 85, 0);
}

TEST(TestColorSpec, hslConvertsToRgb)
{
    expect_color(ParFile::parse_color_spec("hsl:240/1/0.5"), 0, 0, 255);
    expect_color(ParFile::parse_color_spec("hsl:0/0/1"), 255, 255, 255);
}

TEST(TestColorSpec, unknownColorSpacePrefixRejected)
{
    EXPECT_THROW(static_cast<void>(ParFile::parse_color_spec("cmyk:0/0/0/0")), std::runtime_error);
}

TEST(TestColorSpec, cssSpecialKeywordsRejected)
{
    EXPECT_THROW(static_cast<void>(ParFile::parse_color_spec("currentColor")), std::runtime_error);
    EXPECT_THROW(static_cast<void>(ParFile::parse_color_spec("transparent")), std::runtime_error);
    EXPECT_THROW(static_cast<void>(ParFile::parse_color_spec("Canvas")), std::runtime_error);
}

TEST(TestColorSpec, outOfRangeRgbComponentsRejected)
{
    EXPECT_THROW(static_cast<void>(ParFile::parse_color_spec("rgb:256/0/0")), std::runtime_error);
    EXPECT_THROW(static_cast<void>(ParFile::parse_color_spec("rgb:-1/0/0")), std::runtime_error);
}

TEST(TestColorSpec, outOfRangeHsvComponentsRejected)
{
    EXPECT_THROW(static_cast<void>(ParFile::parse_color_spec("hsv:361/1/1")), std::runtime_error);
    EXPECT_THROW(static_cast<void>(ParFile::parse_color_spec("hsv:0/1.1/1")), std::runtime_error);
    EXPECT_THROW(static_cast<void>(ParFile::parse_color_spec("hsv:0/1/-0.1")), std::runtime_error);
}

TEST(TestColorSpec, outOfRangeHslComponentsRejected)
{
    EXPECT_THROW(static_cast<void>(ParFile::parse_color_spec("hsl:361/1/0.5")), std::runtime_error);
    EXPECT_THROW(static_cast<void>(ParFile::parse_color_spec("hsl:0/1.1/0.5")), std::runtime_error);
    EXPECT_THROW(static_cast<void>(ParFile::parse_color_spec("hsl:0/1/-0.1")), std::runtime_error);
}
