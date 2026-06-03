// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/ResolvedAnimation.h>

#include <gtest/gtest.h>

#include <stdexcept>
#include <string>
#include <string_view>

namespace
{

ParFile::ParameterCatalog catalog_data()
{
    ParFile::ParameterCatalog result{
        {{"center-mag", ParFile::ParameterType::CENTER_MAG, ParFile::ParameterFormat::SLASH, ParFile::Curve::GEOMETRIC,
             ParFile::ExtrapolateMode::CLAMP, {}, {}},
            {"maxiter", ParFile::ParameterType::INTEGER, ParFile::ParameterFormat::RAW, ParFile::Curve::LINEAR,
                ParFile::ExtrapolateMode::CLAMP, {}, {}}}};
    result.fractal_types.push_back({"julia",
        {{{0, "c-real",
              {"params[0]", ParFile::ParameterType::DOUBLE, ParFile::ParameterFormat::RAW, ParFile::Curve::LINEAR,
                  ParFile::ExtrapolateMode::CLAMP, {}, {}}},
             {1, "c-imag",
                 {"params[1]", ParFile::ParameterType::DOUBLE, ParFile::ParameterFormat::RAW, ParFile::Curve::LINEAR,
                     ParFile::ExtrapolateMode::CLAMP, {}, {}}}},
            {{"c",
                {"params.c", ParFile::ParameterType::COMPLEX, ParFile::ParameterFormat::SLASH_PAIR,
                    ParFile::Curve::LINEAR, ParFile::ExtrapolateMode::CLAMP, {}, {}},
                {0, 1}}}}});
    result.formula_entries.push_back({"MandelbrotMix4",
        {{{"bailout",
              {"MandelbrotMix4.bailout", ParFile::ParameterType::DOUBLE, ParFile::ParameterFormat::RAW,
                  ParFile::Curve::LINEAR, ParFile::ExtrapolateMode::CLAMP, {}, {}},
              {0}},
            {"bailout-copy",
                {"MandelbrotMix4.bailout-copy", ParFile::ParameterType::DOUBLE, ParFile::ParameterFormat::RAW,
                    ParFile::Curve::LINEAR, ParFile::ExtrapolateMode::CLAMP, {}, {}},
                {0}},
            {"scale factor",
                {"MandelbrotMix4.scale factor", ParFile::ParameterType::DOUBLE, ParFile::ParameterFormat::RAW,
                    ParFile::Curve::LINEAR, ParFile::ExtrapolateMode::CLAMP, {}, {}},
                {1}},
            {"c",
                {"MandelbrotMix4.c", ParFile::ParameterType::COMPLEX, ParFile::ParameterFormat::SLASH_PAIR,
                    ParFile::Curve::LINEAR, ParFile::ExtrapolateMode::CLAMP, {}, {}},
                {2, 3}}}}});
    return result;
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

ParFile::Config julia_config_data(std::string_view parameter)
{
    ParFile::Config result{config_data()};
    result.tracks = {{std::string{parameter}, {{0, "0/1"}, {2, "2/3"}}}};
    return result;
}

ParFile::ParSet julia_source_set()
{
    return {"source", {{"type", "julia"}, {"params", "0/1/52"}}};
}

ParFile::Config formula_config_data(std::string_view parameter)
{
    ParFile::Config result{config_data()};
    result.tracks = {{std::string{parameter}, {{0, "10"}, {2, "20"}}}};
    return result;
}

ParFile::ParSet formula_source_set()
{
    return {"source", {{"type", "formula"}, {"formulaname", "MandelbrotMix4"}, {"params", "0.05/3/-1.5/-2/0/0"}}};
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

TEST(TestResolvedAnimation, juliaParamsGroupResolvesToParamsSlots)
{
    const ParFile::ResolvedAnimation animation{
        ParFile::resolve_animation(julia_config_data("params.c"), catalog_data(), julia_source_set())};

    ASSERT_EQ(1U, animation.tracks.size());
    const ParFile::ResolvedTrack &track{animation.tracks[0]};
    EXPECT_EQ("params.c", track.parameter);
    EXPECT_EQ("params.c", track.metadata.name);
    EXPECT_EQ(ParFile::ParameterType::COMPLEX, track.metadata.type);
    EXPECT_EQ("0/1/52", track.base_value);
    EXPECT_EQ("params", track.output_parameter);
    ASSERT_EQ(2U, track.slots.size());
    EXPECT_EQ(0, track.slots[0]);
    EXPECT_EQ(1, track.slots[1]);
}

TEST(TestResolvedAnimation, juliaParamsSlotResolvesToParamsSlot)
{
    ParFile::Config config{julia_config_data("params[0]")};
    config.tracks[0].keys = {{0, "2"}, {2, "4"}};
    const ParFile::ResolvedAnimation animation{ParFile::resolve_animation(config, catalog_data(), julia_source_set())};

    ASSERT_EQ(1U, animation.tracks.size());
    const ParFile::ResolvedTrack &track{animation.tracks[0]};
    EXPECT_EQ("params[0]", track.parameter);
    EXPECT_EQ("params[0]", track.metadata.name);
    EXPECT_EQ(ParFile::ParameterType::DOUBLE, track.metadata.type);
    EXPECT_EQ("params", track.output_parameter);
    ASSERT_EQ(1U, track.slots.size());
    EXPECT_EQ(0, track.slots[0]);
}

TEST(TestResolvedAnimation, juliaParamsSlot2Rejected)
{
    EXPECT_THROW(ParFile::resolve_animation(julia_config_data("params[2]"), catalog_data(), julia_source_set()),
        std::runtime_error);
}

TEST(TestResolvedAnimation, formulaParamsKnobResolvesFromActiveFormulaname)
{
    const ParFile::ResolvedAnimation animation{ParFile::resolve_animation(
        formula_config_data("MandelbrotMix4.bailout"), catalog_data(), formula_source_set())};

    ASSERT_EQ(1U, animation.tracks.size());
    const ParFile::ResolvedTrack &track{animation.tracks[0]};
    EXPECT_EQ("MandelbrotMix4.bailout", track.parameter);
    EXPECT_EQ("MandelbrotMix4.bailout", track.metadata.name);
    EXPECT_EQ(ParFile::ParameterType::DOUBLE, track.metadata.type);
    EXPECT_EQ("0.05/3/-1.5/-2/0/0", track.base_value);
    EXPECT_EQ("params", track.output_parameter);
    ASSERT_EQ(1U, track.slots.size());
    EXPECT_EQ(0, track.slots[0]);
}

TEST(TestResolvedAnimation, formulaParamsKnobWithSpacesResolvesFromActiveFormulaname)
{
    const ParFile::ResolvedAnimation animation{ParFile::resolve_animation(
        formula_config_data("MandelbrotMix4[\"scale factor\"]"), catalog_data(), formula_source_set())};

    ASSERT_EQ(1U, animation.tracks.size());
    const ParFile::ResolvedTrack &track{animation.tracks[0]};
    EXPECT_EQ("MandelbrotMix4.scale factor", track.metadata.name);
    ASSERT_EQ(1U, track.slots.size());
    EXPECT_EQ(1, track.slots[0]);
}

TEST(TestResolvedAnimation, formulaParamsComplexKnobResolvesToPNSlots)
{
    ParFile::Config config{formula_config_data("MandelbrotMix4.c")};
    config.tracks[0].keys = {{0, "-1/-2"}, {2, "-3/-4"}};
    const ParFile::ResolvedAnimation animation{
        ParFile::resolve_animation(config, catalog_data(), formula_source_set())};

    ASSERT_EQ(1U, animation.tracks.size());
    const ParFile::ResolvedTrack &track{animation.tracks[0]};
    EXPECT_EQ(ParFile::ParameterType::COMPLEX, track.metadata.type);
    ASSERT_EQ(2U, track.slots.size());
    EXPECT_EQ(2, track.slots[0]);
    EXPECT_EQ(3, track.slots[1]);
}

TEST(TestResolvedAnimation, overlappingFormulaParamsKnobsRejected)
{
    ParFile::Config config{formula_config_data("MandelbrotMix4.bailout")};
    config.tracks.push_back({"MandelbrotMix4.bailout-copy", {{0, "11"}, {2, "21"}}});

    EXPECT_THROW(ParFile::resolve_animation(config, catalog_data(), formula_source_set()), std::runtime_error);
}
