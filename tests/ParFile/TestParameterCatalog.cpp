// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/ParameterCatalog.h>

#include <TestParFile/test.h>

#include <gtest/gtest.h>

#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace
{

std::string read_text(const char *path)
{
    std::ifstream in{path};
    return {std::istreambuf_iterator<char>{in}, std::istreambuf_iterator<char>{}};
}

ParFile::ParameterCatalog core_catalog()
{
    return ParFile::ParameterCatalog{read_text(TestParFile::CORE_CATALOG_JSON)};
}

} // namespace

TEST(TestParameterCatalog, centerMagMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{core_catalog()};
    const ParFile::ParameterMetadata &metadata{catalog.metadata("center-mag")};

    EXPECT_EQ("center-mag", metadata.name);
    EXPECT_EQ("center-mag", metadata.type);
    EXPECT_EQ("slash", metadata.format);
    EXPECT_EQ("geometric", metadata.default_curve);
    EXPECT_EQ("clamp", metadata.extrapolate);
}

TEST(TestParameterCatalog, cornersMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{core_catalog()};
    const ParFile::ParameterMetadata &metadata{catalog.metadata("corners")};

    EXPECT_EQ("corners", metadata.name);
    EXPECT_EQ("corners", metadata.type);
    EXPECT_EQ("slash", metadata.format);
    EXPECT_EQ("linear", metadata.default_curve);
    EXPECT_EQ("clamp", metadata.extrapolate);
}

TEST(TestParameterCatalog, maxiterMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{core_catalog()};
    const ParFile::ParameterMetadata &metadata{catalog.metadata("maxiter")};

    EXPECT_EQ("maxiter", metadata.name);
    EXPECT_EQ("integer", metadata.type);
    EXPECT_EQ("raw", metadata.format);
    EXPECT_EQ("linear", metadata.default_curve);
    EXPECT_EQ("clamp", metadata.extrapolate);
}

TEST(TestParameterCatalog, bailoutMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{core_catalog()};
    const ParFile::ParameterMetadata &metadata{catalog.metadata("bailout")};

    EXPECT_EQ("bailout", metadata.name);
    EXPECT_EQ("double", metadata.type);
    EXPECT_EQ("raw", metadata.format);
    EXPECT_EQ("linear", metadata.default_curve);
    EXPECT_EQ("clamp", metadata.extrapolate);
    ASSERT_TRUE(metadata.min);
    ASSERT_TRUE(metadata.max);
    EXPECT_EQ(0, *metadata.min);
    EXPECT_EQ(1000, *metadata.max);
}

TEST(TestParameterCatalog, unknownAnimatedParameterRejected)
{
    EXPECT_THROW(core_catalog().metadata("unknown"), std::runtime_error);
}

TEST(TestParameterCatalog, missingMetadataTypeRejected)
{
    EXPECT_THROW(
        ParFile::ParameterCatalog{read_text(TestParFile::INVALID_MISSING_METADATA_TYPE_JSON)}, std::runtime_error);
}
