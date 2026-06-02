// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/JsonSchema.h>

#include <TestParFile/test.h>

#include <gtest/gtest.h>

#include <filesystem>

namespace
{

bool validates_config_file(const char *path)
{
    return ParFile::validate_json_schema(TestParFile::CONFIG_SCHEMA_JSON, path);
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

TEST(TestJsonSchema, invalidOutputDirectoryTypeRejected)
{
    EXPECT_FALSE(validates_config_file(TestParFile::INVALID_OUTPUT_DIRECTORY_CONFIG_JSON));
}
