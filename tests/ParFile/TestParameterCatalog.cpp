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

ParFile::ParameterCatalog viewport_catalog()
{
    return ParFile::ParameterCatalog{read_text(TestParFile::VIEWPORT_CATALOG_JSON)};
}

} // namespace

TEST(TestParameterCatalog, centerMagMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{viewport_catalog()};
    const ParFile::ParameterMetadata &metadata{catalog.metadata("center-mag")};

    EXPECT_EQ("center-mag", metadata.name);
    EXPECT_EQ("center_mag", metadata.type);
    EXPECT_EQ("slash", metadata.format);
    EXPECT_EQ("geometric", metadata.default_curve);
    EXPECT_EQ("clamp", metadata.extrapolate);
}

TEST(TestParameterCatalog, cornersMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{viewport_catalog()};
    const ParFile::ParameterMetadata &metadata{catalog.metadata("corners")};

    EXPECT_EQ("corners", metadata.name);
    EXPECT_EQ("corners", metadata.type);
    EXPECT_EQ("slash", metadata.format);
    EXPECT_EQ("linear", metadata.default_curve);
    EXPECT_EQ("clamp", metadata.extrapolate);
}

TEST(TestParameterCatalog, unknownAnimatedParameterRejected)
{
    EXPECT_THROW(viewport_catalog().metadata("unknown"), std::runtime_error);
}

TEST(TestParameterCatalog, missingMetadataTypeRejected)
{
    EXPECT_THROW(
        ParFile::ParameterCatalog{read_text(TestParFile::INVALID_MISSING_METADATA_TYPE_JSON)},
        std::runtime_error);
}
