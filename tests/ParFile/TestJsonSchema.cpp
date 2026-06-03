// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/JsonSchema.h>

#include <TestParFile/test.h>

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <string_view>

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

bool validates_config_text(std::string_view json)
{
    return ParFile::validate_json_schema(read_text(TestParFile::CONFIG_SCHEMA_JSON), json);
}

bool validates_parameter_catalog_file(const char *path)
{
    return ParFile::validate_json_schema(read_text(TestParFile::PARAMETER_CATALOG_SCHEMA_JSON), read_text(path));
}

bool validates_parameter_catalog_text(std::string_view json)
{
    return ParFile::validate_json_schema(read_text(TestParFile::PARAMETER_CATALOG_SCHEMA_JSON), json);
}

std::string catalog_with_metadata(std::string_view metadata)
{
    return "{\"parameters\":{\"x\":{" + std::string{metadata} + "}}}";
}

std::string catalog_with_formula_knob(std::string_view metadata)
{
    return "{\"parameters\":{},\"formula-entries\":{\"foo\":{\"params\":{\"knobs\":{\"x\":{" + std::string{metadata} +
        "}}}}}}";
}

std::string catalog_with_formula_function(std::string_view metadata)
{
    return "{\"parameters\":{},\"formula-entries\":{\"foo\":{\"functions\":{\"fn1\":{" + std::string{metadata} +
        "}}}}}";
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
    EXPECT_TRUE(validates_config_file(TestParFile::MAXITER_CONFIG_JSON));
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
    EXPECT_TRUE(validates_parameter_catalog_file(TestParFile::CORE_CATALOG_JSON));
}

TEST(TestJsonSchema, missingMetadataTypeRejected)
{
    EXPECT_FALSE(validates_parameter_catalog_file(TestParFile::INVALID_MISSING_METADATA_TYPE_JSON));
}

TEST(TestJsonSchema, unknownMetadataTypeRejected)
{
    EXPECT_FALSE(validates_parameter_catalog_text(catalog_with_metadata(R"("type":"unknown")")));
}

TEST(TestJsonSchema, numericTupleMetadataAccepted)
{
    EXPECT_TRUE(validates_parameter_catalog_text(catalog_with_metadata(R"("type":"numeric-tuple","arity":3)")));
}

TEST(TestJsonSchema, unknownMetadataFormatRejected)
{
    EXPECT_FALSE(validates_parameter_catalog_text(catalog_with_metadata(R"("type":"integer","format":"unknown")")));
}

TEST(TestJsonSchema, unknownMetadataDefaultCurveRejected)
{
    EXPECT_FALSE(
        validates_parameter_catalog_text(catalog_with_metadata(R"("type":"integer","default-curve":"unknown")")));
}

TEST(TestJsonSchema, unknownMetadataExtrapolateRejected)
{
    EXPECT_FALSE(
        validates_parameter_catalog_text(catalog_with_metadata(R"("type":"integer","extrapolate":"unknown")")));
}

TEST(TestJsonSchema, baseMetadataExtrapolateAccepted)
{
    EXPECT_TRUE(validates_parameter_catalog_text(catalog_with_metadata(R"("type":"integer","extrapolate":"base")")));
}

TEST(TestJsonSchema, omitMetadataExtrapolateAccepted)
{
    EXPECT_TRUE(validates_parameter_catalog_text(catalog_with_metadata(R"("type":"integer","extrapolate":"omit")")));
}

TEST(TestJsonSchema, cycleMetadataExtrapolateAccepted)
{
    EXPECT_TRUE(validates_parameter_catalog_text(catalog_with_metadata(R"("type":"integer","extrapolate":"cycle")")));
}

TEST(TestJsonSchema, pingPongMetadataExtrapolateAccepted)
{
    EXPECT_TRUE(
        validates_parameter_catalog_text(catalog_with_metadata(R"("type":"integer","extrapolate":"ping-pong")")));
}

TEST(TestJsonSchema, invalidMetadataMinimumTypeRejected)
{
    EXPECT_FALSE(validates_parameter_catalog_text(catalog_with_metadata(R"("type":"double","min":"0")")));
}

TEST(TestJsonSchema, invalidMetadataMaximumTypeRejected)
{
    EXPECT_FALSE(validates_parameter_catalog_text(catalog_with_metadata(R"("type":"double","max":"1000")")));
}

TEST(TestJsonSchema, invalidMetadataArityRejected)
{
    EXPECT_FALSE(validates_parameter_catalog_text(catalog_with_metadata(R"("type":"numeric-tuple","arity":0)")));
    EXPECT_FALSE(validates_parameter_catalog_text(catalog_with_metadata(R"("type":"numeric-tuple","arity":"3")")));
}

TEST(TestJsonSchema, invalidFormulaParamsVariableRejected)
{
    EXPECT_FALSE(validates_parameter_catalog_text(catalog_with_formula_knob(R"("type":"real","variable":"p1")")));
    EXPECT_FALSE(
        validates_parameter_catalog_text(catalog_with_formula_knob(R"("type":"complex","variable":"p1.real")")));
}

TEST(TestJsonSchema, invalidFormulaFunctionValuesRejected)
{
    EXPECT_FALSE(
        validates_parameter_catalog_text(catalog_with_formula_function(R"("type":"enum","values":"unknown")")));
}

TEST(TestJsonSchema, invalidFormulaFunctionNameRejected)
{
    EXPECT_FALSE(validates_parameter_catalog_text(R"({
  "parameters": {},
  "formula-entries": {
    "foo": {
      "functions": {
        "fn5": { "type": "enum", "values": "id-functions" }
      }
    }
  }
})"));
}

TEST(TestJsonSchema, invalidOutputDirectoryTypeRejected)
{
    EXPECT_FALSE(validates_config_file(TestParFile::INVALID_OUTPUT_DIRECTORY_CONFIG_JSON));
}

TEST(TestJsonSchema, unknownKeyCurveRejected)
{
    EXPECT_FALSE(validates_config_text(R"({
  "parameter-catalogs": [ "core-catalog.json" ],
  "source": { "file": "from.par", "name": "Mandel_Demo" },
  "output": {
    "directory": "out",
    "par": "frames.par",
    "entry": "frame-%04d",
    "script": "frames.bat"
  },
  "video": "F6",
  "num-frames": 2,
  "tracks": [
    {
      "parameter": "maxiter",
      "keys": [
        { "frame": 0, "value": "100" },
        { "frame": 1, "value": "200", "curve": "unknown" }
      ]
    }
  ]
})"));
}
