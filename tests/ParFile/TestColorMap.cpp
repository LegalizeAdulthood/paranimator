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

void expect_color(const ParFile::RgbColor &color, int red, int green, int blue)
{
    EXPECT_EQ(red, color.red);
    EXPECT_EQ(green, color.green);
    EXPECT_EQ(blue, color.blue);
}

void expect_valid_color(const ParFile::RgbColor &color)
{
    EXPECT_GE(color.red, 0);
    EXPECT_LE(color.red, 255);
    EXPECT_GE(color.green, 0);
    EXPECT_LE(color.green, 255);
    EXPECT_GE(color.blue, 0);
    EXPECT_LE(color.blue, 255);
}

void expect_equal_maps(const ParFile::ColorMap &left, const ParFile::ColorMap &right)
{
    for (std::size_t i = 0; i < left.size(); ++i)
    {
        EXPECT_EQ(left[i].red, right[i].red);
        EXPECT_EQ(left[i].green, right[i].green);
        EXPECT_EQ(left[i].blue, right[i].blue);
    }
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

TEST(TestColorMap, contrastAmountOneLeavesMapUnchanged)
{
    const ParFile::ColorMap map{solid_map(64, 128, 192)};

    const ParFile::ColorMap result{ParFile::contrast_color_map(map, 1.0)};

    expect_color(result[0], 64, 128, 192);
}

TEST(TestColorMap, contrastScalesDistanceFromMidgray)
{
    const ParFile::ColorMap map{solid_map(64, 128, 192)};

    const ParFile::ColorMap result{ParFile::contrast_color_map(map, 0.5)};

    expect_color(result[0], 96, 128, 160);
}

TEST(TestColorMap, contrastClampsToValidRgbRange)
{
    const ParFile::ColorMap map{solid_map(0, 128, 255)};

    const ParFile::ColorMap result{ParFile::contrast_color_map(map, 3.0)};

    expect_color(result[0], 0, 128, 255);
}

TEST(TestColorMap, gammaAmountOneLeavesMapUnchanged)
{
    const ParFile::ColorMap map{solid_map(64, 128, 255)};

    const ParFile::ColorMap result{ParFile::gamma_color_map(map, 1.0)};

    expect_color(result[0], 64, 128, 255);
}

TEST(TestColorMap, gammaAppliesNonlinearIntensity)
{
    const ParFile::ColorMap map{solid_map(64, 128, 255)};

    const ParFile::ColorMap result{ParFile::gamma_color_map(map, 2.0)};

    expect_color(result[0], 16, 64, 255);
}

TEST(TestColorMap, gammaClampsToValidRgbRange)
{
    const ParFile::ColorMap map{solid_map(0, 128, 255)};

    const ParFile::ColorMap result{ParFile::gamma_color_map(map, 0.01)};

    expect_valid_color(result[0]);
}

TEST(TestColorMap, gammaRejectsNonPositiveAmount)
{
    const ParFile::ColorMap map{solid_map(64, 128, 255)};

    EXPECT_THROW(static_cast<void>(ParFile::gamma_color_map(map, 0.0)), std::runtime_error);
}

TEST(TestColorMap, hueShiftAmountZeroLeavesMapUnchanged)
{
    const ParFile::ColorMap map{solid_map(255, 0, 0)};

    const ParFile::ColorMap result{ParFile::hue_shift_color_map(map, 0.0)};

    expect_color(result[0], 255, 0, 0);
}

TEST(TestColorMap, hueShiftRotatesHue)
{
    const ParFile::ColorMap map{solid_map(255, 0, 0)};

    const ParFile::ColorMap result{ParFile::hue_shift_color_map(map, 120.0)};

    expect_color(result[0], 0, 255, 0);
}

TEST(TestColorMap, hueShiftClampsToValidRgbRange)
{
    const ParFile::ColorMap map{solid_map(255, 0, 0)};

    const ParFile::ColorMap result{ParFile::hue_shift_color_map(map, 765.0)};

    expect_valid_color(result[0]);
}

TEST(TestColorMap, saturationAmountOneLeavesMapUnchanged)
{
    const ParFile::ColorMap map{solid_map(255, 0, 0)};

    const ParFile::ColorMap result{ParFile::saturation_color_map(map, 1.0)};

    expect_color(result[0], 255, 0, 0);
}

TEST(TestColorMap, saturationScalesColorSaturation)
{
    const ParFile::ColorMap map{solid_map(255, 0, 0)};

    const ParFile::ColorMap result{ParFile::saturation_color_map(map, 0.0)};

    expect_color(result[0], 128, 128, 128);
}

TEST(TestColorMap, saturationClampsToValidRgbRange)
{
    const ParFile::ColorMap map{solid_map(255, 0, 0)};

    const ParFile::ColorMap result{ParFile::saturation_color_map(map, 100.0)};

    expect_valid_color(result[0]);
}

TEST(TestColorMap, pulseAffectsOnlySelectedRange)
{
    const ParFile::ColorMap map{indexed_map()};

    const ParFile::ColorMap result{ParFile::pulse_color_map(map, {2, 3}, {255, 255, 255}, 1.0)};

    expect_color(result[1], 1, 254, 1);
    expect_color(result[2], 255, 255, 255);
    expect_color(result[3], 255, 255, 255);
    expect_color(result[4], 4, 251, 4);
}

TEST(TestColorMap, maskBlendAffectsOnlySelectedRanges)
{
    const ParFile::ColorMap map{indexed_map()};
    const ParFile::ColorMap mask{solid_map(100, 120, 140)};

    const ParFile::ColorMap result{ParFile::mask_blend_color_map(map, mask, {{2, 3}, {6, 6}}, 1.0)};

    expect_color(result[1], 1, 254, 1);
    expect_color(result[2], 100, 120, 140);
    expect_color(result[3], 100, 120, 140);
    expect_color(result[4], 4, 251, 4);
    expect_color(result[6], 100, 120, 140);
}

TEST(TestColorMap, remapUsesIndexTable)
{
    const ParFile::ColorMap map{indexed_map()};
    std::vector<int> indices(ParFile::COLOR_MAP_SIZE);
    for (std::size_t i = 0; i < indices.size(); ++i)
    {
        indices[i] = static_cast<int>(i);
    }
    indices[0] = 2;
    indices[1] = 3;

    const ParFile::ColorMap result{ParFile::remap_color_map(map, indices)};

    expect_color(result[0], 2, 253, 2);
    expect_color(result[1], 3, 252, 3);
    expect_color(result[2], 2, 253, 2);
}

TEST(TestColorMap, sparkleIsSeededAndRepeatable)
{
    const ParFile::ColorMap map{solid_map(128, 128, 128)};

    const ParFile::ColorMap first{ParFile::sparkle_color_map(map, {2, 5}, 8675309, 32.0)};
    const ParFile::ColorMap second{ParFile::sparkle_color_map(map, {2, 5}, 8675309, 32.0)};

    expect_equal_maps(first, second);
    expect_color(first[1], 128, 128, 128);
    expect_color(first[6], 128, 128, 128);
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
