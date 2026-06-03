// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/ResolvedAnimation.h>

#include <gtest/gtest.h>

#include <stdexcept>

namespace
{

ParFile::ParameterCatalog catalog_data()
{
    return {{{"center-mag", ParFile::ParameterType::CENTER_MAG, ParFile::ParameterFormat::SLASH,
                 ParFile::Curve::GEOMETRIC, ParFile::ExtrapolateMode::CLAMP, {}, {}},
        {"maxiter", ParFile::ParameterType::INTEGER, ParFile::ParameterFormat::RAW, ParFile::Curve::LINEAR,
            ParFile::ExtrapolateMode::CLAMP, {}, {}}}};
}

ParFile::Config config_data()
{
    return {{}, {"source.par", "source"}, {"out", "frames.par", "frame-%04d", "frames.bat"}, 1, "F6", 3,
        {{"center-mag", {{0, "-0.5/0/1"}, {2, "-0.5/0/10"}}}}};
}

ParFile::ParSet source_set()
{
    return {"source", {{"center-mag", "-0.5/0/1"}, {"maxiter", "100"}}};
}

} // namespace

TEST(TestResolvedAnimation, unknownAnimatedParameterRejected)
{
    ParFile::Config config{config_data()};
    config.tracks[0].parameter = "unknown";

    EXPECT_THROW(ParFile::resolve_animation(config, catalog_data(), source_set()), std::runtime_error);
}

TEST(TestResolvedAnimation, missingSourceParameterRejected)
{
    ParFile::ParSet source{source_set()};
    source.params.erase(source.params.begin());

    EXPECT_THROW(ParFile::resolve_animation(config_data(), catalog_data(), source), std::runtime_error);
}

TEST(TestResolvedAnimation, resolvedTrackCarriesParameterMetadataBaseValueAndKeys)
{
    const ParFile::ResolvedAnimation animation{ParFile::resolve_animation(config_data(), catalog_data(), source_set())};

    EXPECT_EQ("frame-%04d", animation.frame_name);
    EXPECT_EQ("F6", animation.video);
    EXPECT_EQ(3, animation.num_frames);
    EXPECT_EQ("source", animation.source.name);
    ASSERT_EQ(1U, animation.tracks.size());

    const ParFile::ResolvedTrack &track{animation.tracks[0]};
    EXPECT_EQ("center-mag", track.parameter);
    EXPECT_EQ("center-mag", track.metadata.name);
    EXPECT_EQ(ParFile::ParameterType::CENTER_MAG, track.metadata.type);
    EXPECT_EQ("-0.5/0/1", track.base_value);
    ASSERT_EQ(2U, track.keys.size());
    EXPECT_EQ(0, track.keys[0].frame);
    EXPECT_EQ("-0.5/0/1", track.keys[0].value);
    EXPECT_EQ(2, track.keys[1].frame);
    EXPECT_EQ("-0.5/0/10", track.keys[1].value);
}
