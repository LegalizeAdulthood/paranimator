// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/JsonSchema.h>

#include <TestParFile/test.h>

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <iterator>

namespace
{

std::string read_text(const char *path)
{
    std::ifstream in{path};
    return {std::istreambuf_iterator<char>{in}, std::istreambuf_iterator<char>{}};
}

bool validates_config_file(const char *path)
{
    return ParFile::validate_json_schema(read_text(TestParFile::CONFIG_SCHEMA_JSON), read_text(path));
}

bool validates_parameter_catalog_file(const char *path)
{
    return ParFile::validate_json_schema(
        read_text(TestParFile::PARAMETER_CATALOG_SCHEMA_JSON), read_text(path));
}

} // namespace

TEST(TestJsonSchema, schemaPathStable)
{
    const std::filesystem::path path{TestParFile::CONFIG_SCHEMA_JSON};

    EXPECT_EQ("config.schema.json", path.filename().string());
    EXPECT_TRUE(std::filesystem::exists(path));
}

TEST(TestJsonSchema, validConfigFilesPass)
{
    EXPECT_TRUE(validates_config_file(TestParFile::CENTER_MAG_CONFIG_JSON));
    EXPECT_TRUE(validates_config_file(TestParFile::CORNERS_CONFIG_JSON));
    EXPECT_TRUE(validates_config_file(TestParFile::DATA_CONFIG_JSON));
}

TEST(TestJsonSchema, parameterCatalogSchemaPathStable)
{
    const std::filesystem::path path{TestParFile::PARAMETER_CATALOG_SCHEMA_JSON};

    EXPECT_EQ("parameter-catalog.schema.json", path.filename().string());
    EXPECT_TRUE(std::filesystem::exists(path));
}

TEST(TestJsonSchema, validParameterCatalogPasses)
{
    EXPECT_TRUE(validates_parameter_catalog_file(TestParFile::VIEWPORT_CATALOG_JSON));
}

TEST(TestJsonSchema, missingMetadataTypeRejected)
{
    EXPECT_FALSE(validates_parameter_catalog_file(TestParFile::INVALID_MISSING_METADATA_TYPE_JSON));
}

TEST(TestJsonSchema, invalidOutputDirectoryTypeRejected)
{
    EXPECT_FALSE(validates_config_file(TestParFile::INVALID_OUTPUT_DIRECTORY_CONFIG_JSON));
}
