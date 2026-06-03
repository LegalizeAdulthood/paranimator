// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/ColorMap.h>

#include <gtest/gtest.h>

#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

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

ParFile::ColorMap indexed_map()
{
    ParFile::ColorMap result;
    for (std::size_t i = 0; i < result.size(); ++i)
    {
        result[i] = {static_cast<int>(i), 255 - static_cast<int>(i), static_cast<int>(i % 64U)};
    }
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

TEST(TestColorMap, brightnessScalesEachRgbComponent)
{
    const ParFile::ColorMap map{solid_map(10, 20, 30)};

    const ParFile::ColorMap result{ParFile::brightness_color_map(map, 1.5)};

    EXPECT_EQ(15, result[0].red);
    EXPECT_EQ(30, result[0].green);
    EXPECT_EQ(45, result[0].blue);
}

TEST(TestColorMap, brightnessClampsToValidRgbRange)
{
    const ParFile::ColorMap map{solid_map(200, 220, 240)};

    const ParFile::ColorMap result{ParFile::brightness_color_map(map, 2.0)};

    EXPECT_EQ(255, result[0].red);
    EXPECT_EQ(255, result[0].green);
    EXPECT_EQ(255, result[0].blue);

    const ParFile::ColorMap dark_result{ParFile::brightness_color_map(map, -1.0)};

    EXPECT_EQ(0, dark_result[0].red);
    EXPECT_EQ(0, dark_result[0].green);
    EXPECT_EQ(0, dark_result[0].blue);
}

TEST(TestColorMap, brightnessAmountOneLeavesMapUnchanged)
{
    const ParFile::ColorMap map{indexed_map()};

    const ParFile::ColorMap result{ParFile::brightness_color_map(map, 1.0)};

    for (std::size_t i = 0; i < result.size(); ++i)
    {
        EXPECT_EQ(map[i].red, result[i].red);
        EXPECT_EQ(map[i].green, result[i].green);
        EXPECT_EQ(map[i].blue, result[i].blue);
    }
}

TEST(TestColorMap, gradientTwoStopsAccepted)
{
    const ParFile::ColorMap map{
        ParFile::gradient_color_map({{0, {0, 0, 0}}, {255, {255, 255, 255}}})};

    EXPECT_EQ(0, map[0].red);
    EXPECT_EQ(128, map[128].red);
    EXPECT_EQ(255, map[255].red);
}

TEST(TestColorMap, gradientAdjacentStopPairsInterpolateEachInterval)
{
    const ParFile::ColorMap map{
        ParFile::gradient_color_map({{0, {0, 0, 0}}, {2, {10, 0, 0}}, {4, {10, 10, 0}}})};

    EXPECT_EQ(5, map[1].red);
    EXPECT_EQ(0, map[1].green);
    EXPECT_EQ(10, map[3].red);
    EXPECT_EQ(5, map[3].green);
}

TEST(TestColorMap, gradientInvalidStopsAreRejected)
{
    EXPECT_THROW(
        static_cast<void>(ParFile::gradient_color_map({{0, {0, 0, 0}}})), std::runtime_error);
    EXPECT_THROW(static_cast<void>(
                     ParFile::gradient_color_map({{2, {0, 0, 0}}, {1, {255, 255, 255}}})),
        std::runtime_error);
    EXPECT_THROW(static_cast<void>(
                     ParFile::gradient_color_map({{-1, {0, 0, 0}}, {255, {255, 255, 255}}})),
        std::runtime_error);
}

TEST(TestColorMap, rotatePositiveOffsetWrapsPaletteEntries)
{
    const ParFile::ColorMap map{indexed_map()};

    const ParFile::ColorMap result{ParFile::rotate_color_map(map, 1)};

    EXPECT_EQ(255, result[0].red);
    EXPECT_EQ(0, result[1].red);
    EXPECT_EQ(1, result[2].red);
}

TEST(TestColorMap, rotateNegativeOffsetWrapsPaletteEntries)
{
    const ParFile::ColorMap map{indexed_map()};

    const ParFile::ColorMap result{ParFile::rotate_color_map(map, -1)};

    EXPECT_EQ(1, result[0].red);
    EXPECT_EQ(2, result[1].red);
    EXPECT_EQ(0, result[255].red);
}

TEST(TestColorMap, rotateZeroOffsetLeavesMapUnchanged)
{
    const ParFile::ColorMap map{indexed_map()};

    const ParFile::ColorMap result{ParFile::rotate_color_map(map, 0)};

    for (std::size_t i = 0; i < result.size(); ++i)
    {
        EXPECT_EQ(map[i].red, result[i].red);
        EXPECT_EQ(map[i].green, result[i].green);
        EXPECT_EQ(map[i].blue, result[i].blue);
    }
}

TEST(TestColorMap, rotateRangeOnlySelectedRangeRotates)
{
    const ParFile::ColorMap map{indexed_map()};

    const ParFile::ColorMap result{ParFile::rotate_color_map_range(map, 2, 5, 1)};

    EXPECT_EQ(1, result[1].red);
    EXPECT_EQ(5, result[2].red);
    EXPECT_EQ(2, result[3].red);
    EXPECT_EQ(3, result[4].red);
    EXPECT_EQ(4, result[5].red);
    EXPECT_EQ(6, result[6].red);
}

TEST(TestColorMap, rotateRangeEntriesOutsideRangeAreUnchanged)
{
    const ParFile::ColorMap map{indexed_map()};

    const ParFile::ColorMap result{ParFile::rotate_color_map_range(map, 10, 12, -1)};

    EXPECT_EQ(9, result[9].red);
    EXPECT_EQ(11, result[10].red);
    EXPECT_EQ(12, result[11].red);
    EXPECT_EQ(10, result[12].red);
    EXPECT_EQ(13, result[13].red);
}

TEST(TestColorMap, rotateRangeInvalidRangesAreRejected)
{
    const ParFile::ColorMap map{indexed_map()};

    EXPECT_THROW(ParFile::rotate_color_map_range(map, 5, 4, 1), std::runtime_error);
    EXPECT_THROW(ParFile::rotate_color_map_range(map, -1, 4, 1), std::runtime_error);
    EXPECT_THROW(ParFile::rotate_color_map_range(map, 0, 256, 1), std::runtime_error);
}

TEST(TestColorMap, reverseFullMapFlipsAllEntries)
{
    const ParFile::ColorMap map{indexed_map()};

    const ParFile::ColorMap result{ParFile::reverse_color_map(map)};

    EXPECT_EQ(255, result[0].red);
    EXPECT_EQ(254, result[1].red);
    EXPECT_EQ(0, result[255].red);
}

TEST(TestColorMap, reverseRangeFlipsOnlySelectedRange)
{
    const ParFile::ColorMap map{indexed_map()};

    const ParFile::ColorMap result{ParFile::reverse_color_map_range(map, 2, 5)};

    EXPECT_EQ(1, result[1].red);
    EXPECT_EQ(5, result[2].red);
    EXPECT_EQ(4, result[3].red);
    EXPECT_EQ(3, result[4].red);
    EXPECT_EQ(2, result[5].red);
    EXPECT_EQ(6, result[6].red);
}

TEST(TestColorMap, reverseRangeInvalidRangesAreRejected)
{
    const ParFile::ColorMap map{indexed_map()};

    EXPECT_THROW(ParFile::reverse_color_map_range(map, 5, 4), std::runtime_error);
    EXPECT_THROW(ParFile::reverse_color_map_range(map, -1, 4), std::runtime_error);
    EXPECT_THROW(ParFile::reverse_color_map_range(map, 0, 256), std::runtime_error);
}

TEST(TestColorMap, pingPongRangeAlternatesForwardAndBackwardOffsets)
{
    const ParFile::ColorMap map{indexed_map()};

    const ParFile::ColorMap forward{ParFile::ping_pong_color_map_range(map, 2, 5, 1)};
    const ParFile::ColorMap backward{ParFile::ping_pong_color_map_range(map, 2, 5, 5)};

    EXPECT_EQ(5, forward[2].red);
    EXPECT_EQ(2, forward[3].red);
    EXPECT_EQ(5, backward[2].red);
    EXPECT_EQ(2, backward[3].red);
}

TEST(TestColorMap, pingPongFullMapReturnsToStartAfterFullCycle)
{
    const ParFile::ColorMap map{indexed_map()};

    const ParFile::ColorMap result{ParFile::ping_pong_color_map(map, 510)};

    EXPECT_EQ(0, result[0].red);
    EXPECT_EQ(1, result[1].red);
    EXPECT_EQ(255, result[255].red);
}

TEST(TestColorMap, pingPongRangeInvalidRangesAreRejected)
{
    const ParFile::ColorMap map{indexed_map()};

    EXPECT_THROW(ParFile::ping_pong_color_map_range(map, 5, 4, 1), std::runtime_error);
    EXPECT_THROW(ParFile::ping_pong_color_map_range(map, -1, 4, 1), std::runtime_error);
    EXPECT_THROW(ParFile::ping_pong_color_map_range(map, 0, 256, 1), std::runtime_error);
}

TEST(TestColorMap, sequenceSelectedMapChangesAtExpectedFrame)
{
    const std::vector<ParFile::ColorMapSequenceEntry> sequence{{0, solid_map(1, 2, 3)}, {3, solid_map(4, 5, 6)}};

    const ParFile::ColorMap before{ParFile::sequence_color_map(sequence, 2, 0)};
    const ParFile::ColorMap after{ParFile::sequence_color_map(sequence, 3, 0)};

    EXPECT_EQ(1, before[0].red);
    EXPECT_EQ(4, after[0].red);
}

TEST(TestColorMap, sequenceCrossfadeUsesInterpolation)
{
    const std::vector<ParFile::ColorMapSequenceEntry> sequence{{0, solid_map(0, 10, 20)}, {4, solid_map(10, 20, 30)}};

    const ParFile::ColorMap result{ParFile::sequence_color_map(sequence, 3, 2)};

    EXPECT_EQ(5, result[0].red);
    EXPECT_EQ(15, result[0].green);
    EXPECT_EQ(25, result[0].blue);
}

TEST(TestColorMap, sequenceMissingMapsAreRejected)
{
    const std::vector<ParFile::ColorMapSequenceEntry> sequence;

    EXPECT_THROW(ParFile::sequence_color_map(sequence, 0, 0), std::runtime_error);
}
