// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/ColorMap.h>

#include <gtest/gtest.h>

#include <sstream>
#include <stdexcept>
#include <string>

namespace
{

std::string valid_map_text()
{
    std::ostringstream out;
    for (int i = 0; i < 256; ++i)
    {
        out << i << ' ' << 255 - i << ' ' << i % 64;
        if (i == 0)
        {
            out << " SPDX-License-Identifier: GPL-3.0-only";
        }
        out << '\n';
    }
    return out.str();
}

ParFile::ColorMap solid_map(int red, int green, int blue)
{
    ParFile::ColorMap result;
    result.fill({red, green, blue});
    return result;
}

} // namespace

TEST(TestColorMap, reads256Entries)
{
    std::istringstream in{valid_map_text()};

    const ParFile::ColorMap map{ParFile::read_color_map(in)};

    EXPECT_EQ(0, map[0].red);
    EXPECT_EQ(255, map[0].green);
    EXPECT_EQ(0, map[0].blue);
    EXPECT_EQ(255, map[255].red);
    EXPECT_EQ(0, map[255].green);
    EXPECT_EQ(63, map[255].blue);
}

TEST(TestColorMap, rejectsMalformedRgbEntry)
{
    std::istringstream in{"0 1\n"};

    EXPECT_THROW(static_cast<void>(ParFile::read_color_map(in)), std::runtime_error);
}

TEST(TestColorMap, rejectsOutOfRangeRgbEntry)
{
    std::istringstream in{"0 1 256\n"};

    EXPECT_THROW(static_cast<void>(ParFile::read_color_map(in)), std::runtime_error);
}

TEST(TestColorMap, rejectsShortMap)
{
    std::istringstream in{"0 1 2\n"};

    EXPECT_THROW(static_cast<void>(ParFile::read_color_map(in)), std::runtime_error);
}

TEST(TestColorMap, rejectsLongMap)
{
    std::string text{valid_map_text()};
    text += "0 0 0\n";
    std::istringstream in{text};

    EXPECT_THROW(static_cast<void>(ParFile::read_color_map(in)), std::runtime_error);
}

TEST(TestColorMap, writesIdCompatibleRgbTriplets)
{
    ParFile::ColorMap map;
    for (int i = 0; i < 256; ++i)
    {
        map[static_cast<std::size_t>(i)] = {i, 255 - i, i % 64};
    }
    std::ostringstream out;

    ParFile::write_color_map(out, map);
    std::istringstream in{out.str()};
    const ParFile::ColorMap reread{ParFile::read_color_map(in)};

    EXPECT_EQ(0, reread[0].red);
    EXPECT_EQ(255, reread[0].green);
    EXPECT_EQ(0, reread[0].blue);
    EXPECT_EQ(255, reread[255].red);
    EXPECT_EQ(0, reread[255].green);
    EXPECT_EQ(63, reread[255].blue);
}

TEST(TestColorMap, writeRejectsOutOfRangeRgbEntry)
{
    ParFile::ColorMap map;
    map[0] = {0, 1, 256};
    std::ostringstream out;

    EXPECT_THROW(ParFile::write_color_map(out, map), std::runtime_error);
}

TEST(TestColorMap, interpolateBlendZeroReturnsFirstMap)
{
    const ParFile::ColorMap from{solid_map(1, 2, 3)};
    const ParFile::ColorMap to{solid_map(4, 5, 6)};

    const ParFile::ColorMap result{ParFile::interpolate_color_map(from, to, 0.0)};

    EXPECT_EQ(1, result[0].red);
    EXPECT_EQ(2, result[0].green);
    EXPECT_EQ(3, result[0].blue);
}

TEST(TestColorMap, interpolateBlendHalfAveragesEntries)
{
    const ParFile::ColorMap from{solid_map(1, 3, 5)};
    const ParFile::ColorMap to{solid_map(5, 7, 9)};

    const ParFile::ColorMap result{ParFile::interpolate_color_map(from, to, 0.5)};

    EXPECT_EQ(3, result[0].red);
    EXPECT_EQ(5, result[0].green);
    EXPECT_EQ(7, result[0].blue);
}

TEST(TestColorMap, interpolateBlendOneReturnsSecondMap)
{
    const ParFile::ColorMap from{solid_map(1, 2, 3)};
    const ParFile::ColorMap to{solid_map(4, 5, 6)};

    const ParFile::ColorMap result{ParFile::interpolate_color_map(from, to, 1.0)};

    EXPECT_EQ(4, result[0].red);
    EXPECT_EQ(5, result[0].green);
    EXPECT_EQ(6, result[0].blue);
}
