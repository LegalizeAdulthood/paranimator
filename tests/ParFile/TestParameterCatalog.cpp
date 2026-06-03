// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/ParameterCatalog.h>

#include <TestParFile/test.h>

#include <gtest/gtest.h>

#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>

namespace
{

std::string read_text(const char *path)
{
    std::ifstream in{path};
    return {std::istreambuf_iterator<char>{in}, std::istreambuf_iterator<char>{}};
}

ParFile::ParameterCatalog core_catalog()
{
    return ParFile::read_parameter_catalog(read_text(TestParFile::CORE_CATALOG_JSON));
}

ParFile::ParameterCatalog typed_catalog()
{
    return {{{"center-mag", ParFile::ParameterType::CENTER_MAG, ParFile::ParameterFormat::SLASH,
                 ParFile::Curve::GEOMETRIC, ParFile::ExtrapolateMode::CLAMP, {}, {}},
        {"maxiter", ParFile::ParameterType::INTEGER, ParFile::ParameterFormat::RAW, ParFile::Curve::LINEAR,
            ParFile::ExtrapolateMode::CLAMP, {}, {}}}};
}

std::string catalog_text(std::string_view metadata)
{
    return "{\"parameters\":{\"x\":{" + std::string{metadata} + "}}}";
}

ParFile::ParameterMetadata read_metadata(std::string_view metadata)
{
    return ParFile::read_parameter_catalog(catalog_text(metadata)).metadata("x");
}

} // namespace

TEST(TestParameterCatalog, validCatalogJsonDeserializesAllCoreParameters)
{
    const ParFile::ParameterCatalog catalog{core_catalog()};

    EXPECT_EQ(4U, catalog.parameters.size());
    EXPECT_EQ(1U, catalog.fractal_types.size());
}

TEST(TestParameterCatalog, centerMagMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{core_catalog()};
    const ParFile::ParameterMetadata &metadata{catalog.metadata("center-mag")};

    EXPECT_EQ("center-mag", metadata.name);
    EXPECT_EQ(ParFile::ParameterType::CENTER_MAG, metadata.type);
    ASSERT_TRUE(metadata.format);
    EXPECT_EQ(ParFile::ParameterFormat::SLASH, *metadata.format);
    ASSERT_TRUE(metadata.default_curve);
    EXPECT_EQ(ParFile::Curve::GEOMETRIC, *metadata.default_curve);
    ASSERT_TRUE(metadata.extrapolate);
    EXPECT_EQ(ParFile::ExtrapolateMode::CLAMP, *metadata.extrapolate);
}

TEST(TestParameterCatalog, cornersMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{core_catalog()};
    const ParFile::ParameterMetadata &metadata{catalog.metadata("corners")};

    EXPECT_EQ("corners", metadata.name);
    EXPECT_EQ(ParFile::ParameterType::CORNERS, metadata.type);
    ASSERT_TRUE(metadata.format);
    EXPECT_EQ(ParFile::ParameterFormat::SLASH, *metadata.format);
    ASSERT_TRUE(metadata.default_curve);
    EXPECT_EQ(ParFile::Curve::LINEAR, *metadata.default_curve);
    ASSERT_TRUE(metadata.extrapolate);
    EXPECT_EQ(ParFile::ExtrapolateMode::CLAMP, *metadata.extrapolate);
}

TEST(TestParameterCatalog, maxiterMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{core_catalog()};
    const ParFile::ParameterMetadata &metadata{catalog.metadata("maxiter")};

    EXPECT_EQ("maxiter", metadata.name);
    EXPECT_EQ(ParFile::ParameterType::INTEGER, metadata.type);
    ASSERT_TRUE(metadata.format);
    EXPECT_EQ(ParFile::ParameterFormat::RAW, *metadata.format);
    ASSERT_TRUE(metadata.default_curve);
    EXPECT_EQ(ParFile::Curve::LINEAR, *metadata.default_curve);
    ASSERT_TRUE(metadata.extrapolate);
    EXPECT_EQ(ParFile::ExtrapolateMode::CLAMP, *metadata.extrapolate);
}

TEST(TestParameterCatalog, bailoutMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{core_catalog()};
    const ParFile::ParameterMetadata &metadata{catalog.metadata("bailout")};

    EXPECT_EQ("bailout", metadata.name);
    EXPECT_EQ(ParFile::ParameterType::DOUBLE, metadata.type);
    ASSERT_TRUE(metadata.format);
    EXPECT_EQ(ParFile::ParameterFormat::RAW, *metadata.format);
    ASSERT_TRUE(metadata.default_curve);
    EXPECT_EQ(ParFile::Curve::LINEAR, *metadata.default_curve);
    ASSERT_TRUE(metadata.extrapolate);
    EXPECT_EQ(ParFile::ExtrapolateMode::CLAMP, *metadata.extrapolate);
    ASSERT_TRUE(metadata.min);
    ASSERT_TRUE(metadata.max);
    EXPECT_EQ(0, *metadata.min);
    EXPECT_EQ(1000, *metadata.max);
}

TEST(TestParameterCatalog, typedCatalogFindsMetadataByName)
{
    const ParFile::ParameterCatalog catalog{typed_catalog()};
    const ParFile::ParameterMetadata &metadata{catalog.metadata("maxiter")};

    EXPECT_EQ(ParFile::ParameterType::INTEGER, metadata.type);
}

TEST(TestParameterCatalog, juliaParamsSlotMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{core_catalog()};
    const ParFile::ParamsSlotMetadata &slot{catalog.params_slot("julia", 0)};

    EXPECT_EQ(0, slot.index);
    EXPECT_EQ("c-real", slot.name);
    EXPECT_EQ("params[0]", slot.metadata.name);
    EXPECT_EQ(ParFile::ParameterType::DOUBLE, slot.metadata.type);
    ASSERT_TRUE(slot.metadata.format);
    EXPECT_EQ(ParFile::ParameterFormat::RAW, *slot.metadata.format);
}

TEST(TestParameterCatalog, juliaParamsGroupMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{core_catalog()};
    const ParFile::ParamsGroupMetadata &group{catalog.params_group("julia", "c")};

    EXPECT_EQ("c", group.name);
    EXPECT_EQ("params.c", group.metadata.name);
    EXPECT_EQ(ParFile::ParameterType::COMPLEX, group.metadata.type);
    ASSERT_TRUE(group.metadata.format);
    EXPECT_EQ(ParFile::ParameterFormat::SLASH_PAIR, *group.metadata.format);
    ASSERT_EQ(2U, group.slots.size());
    EXPECT_EQ(0, group.slots[0]);
    EXPECT_EQ(1, group.slots[1]);
}

TEST(TestParameterCatalog, legalParameterTypeStringsDecode)
{
    EXPECT_EQ(ParFile::ParameterType::CENTER_MAG, read_metadata(R"("type":"center-mag")").type);
    EXPECT_EQ(ParFile::ParameterType::COMPLEX, read_metadata(R"("type":"complex")").type);
    EXPECT_EQ(ParFile::ParameterType::CORNERS, read_metadata(R"("type":"corners")").type);
    EXPECT_EQ(ParFile::ParameterType::DOUBLE, read_metadata(R"("type":"double")").type);
    EXPECT_EQ(ParFile::ParameterType::INTEGER, read_metadata(R"("type":"integer")").type);
}

TEST(TestParameterCatalog, legalParameterFormatStringsDecode)
{
    EXPECT_EQ(ParFile::ParameterFormat::RAW, *read_metadata(R"("type":"integer","format":"raw")").format);
    EXPECT_EQ(ParFile::ParameterFormat::SLASH, *read_metadata(R"("type":"integer","format":"slash")").format);
    EXPECT_EQ(ParFile::ParameterFormat::SLASH_LIST, *read_metadata(R"("type":"integer","format":"slash-list")").format);
    EXPECT_EQ(ParFile::ParameterFormat::SLASH_PAIR, *read_metadata(R"("type":"integer","format":"slash-pair")").format);
}

TEST(TestParameterCatalog, legalCurveStringsDecode)
{
    EXPECT_EQ(
        ParFile::Curve::GEOMETRIC, *read_metadata(R"("type":"integer","default-curve":"geometric")").default_curve);
    EXPECT_EQ(ParFile::Curve::HOLD, *read_metadata(R"("type":"integer","default-curve":"hold")").default_curve);
    EXPECT_EQ(ParFile::Curve::LINEAR, *read_metadata(R"("type":"integer","default-curve":"linear")").default_curve);
    EXPECT_EQ(ParFile::Curve::STEP, *read_metadata(R"("type":"integer","default-curve":"step")").default_curve);
}

TEST(TestParameterCatalog, legalExtrapolateStringsDecode)
{
    EXPECT_EQ(ParFile::ExtrapolateMode::BASE, *read_metadata(R"("type":"integer","extrapolate":"base")").extrapolate);
    EXPECT_EQ(ParFile::ExtrapolateMode::CLAMP, *read_metadata(R"("type":"integer","extrapolate":"clamp")").extrapolate);
    EXPECT_EQ(ParFile::ExtrapolateMode::CYCLE, *read_metadata(R"("type":"integer","extrapolate":"cycle")").extrapolate);
    EXPECT_EQ(ParFile::ExtrapolateMode::OMIT, *read_metadata(R"("type":"integer","extrapolate":"omit")").extrapolate);
    EXPECT_EQ(ParFile::ExtrapolateMode::PING_PONG,
        *read_metadata(R"("type":"integer","extrapolate":"ping-pong")").extrapolate);
}

TEST(TestParameterCatalog, unknownParameterTypeStringRejected)
{
    EXPECT_THROW(ParFile::read_parameter_catalog(catalog_text(R"("type":"unknown")")), std::runtime_error);
}

TEST(TestParameterCatalog, unknownParameterFormatStringRejected)
{
    EXPECT_THROW(
        ParFile::read_parameter_catalog(catalog_text(R"("type":"integer","format":"unknown")")), std::runtime_error);
}

TEST(TestParameterCatalog, unknownCurveStringRejected)
{
    EXPECT_THROW(ParFile::read_parameter_catalog(catalog_text(R"("type":"integer","default-curve":"unknown")")),
        std::runtime_error);
}

TEST(TestParameterCatalog, unknownExtrapolateStringRejected)
{
    EXPECT_THROW(ParFile::read_parameter_catalog(catalog_text(R"("type":"integer","extrapolate":"unknown")")),
        std::runtime_error);
}

TEST(TestParameterCatalog, unknownAnimatedParameterRejected)
{
    EXPECT_THROW(typed_catalog().metadata("unknown"), std::runtime_error);
}

TEST(TestParameterCatalog, unknownParamsSlotRejected)
{
    EXPECT_THROW(core_catalog().params_slot("julia", 2), std::runtime_error);
}

TEST(TestParameterCatalog, unknownParamsGroupRejected)
{
    EXPECT_THROW(core_catalog().params_group("julia", "unknown"), std::runtime_error);
}

TEST(TestParameterCatalog, missingMetadataTypeRejected)
{
    EXPECT_THROW(ParFile::read_parameter_catalog(read_text(TestParFile::INVALID_MISSING_METADATA_TYPE_JSON)),
        std::runtime_error);
}
