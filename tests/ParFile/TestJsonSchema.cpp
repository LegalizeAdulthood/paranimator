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

std::string config_with_gradient_stop_color(std::string_view color)
{
    return R"({
  "parameter-catalogs": [ "core-catalog.json" ],
  "source": { "file": "from.par", "name": "Mandel_Demo" },
  "output": {
    "directory": "out",
    "par": "frames.par",
    "entry": "frame-%04d",
    "script": "frames.bat"
  },
  "video": "F6",
  "num-frames": 1,
  "tracks": [
    {
      "parameter": "colors",
      "type": "color-map",
      "format": "at-file",
      "output": "colors-%04d.map",
      "source": {
        "kind": "gradient",
        "stops": [
          { "index": 0, "color": "black" },
          { "index": 255, "color": ")" + std::string{color} + R"(" }
        ]
      }
    }
  ]
})";
}

std::string config_with_color_map_effect(std::string_view effect)
{
    return R"({
  "parameter-catalogs": [ "core-catalog.json" ],
  "source": { "file": "from.par", "name": "Mandel_Demo" },
  "output": {
    "directory": "out",
    "par": "frames.par",
    "entry": "frame-%04d",
    "script": "frames.bat"
  },
  "video": "F6",
  "num-frames": 3,
  "tracks": [
    {
      "parameter": "colors",
      "type": "color-map",
      "format": "at-file",
      "output": "colors-%04d.map",
      "source": "base.map",
      "effects": [
        )" + std::string{effect} + R"(
      ]
    }
  ]
})";
}

std::string config_with_track(std::string_view track)
{
    return R"({
  "parameter-catalogs": [ "core-catalog.json" ],
  "source": { "file": "from.par", "name": "Mandel_Demo" },
  "output": {
    "directory": "out",
    "par": "frames.par",
    "entry": "frame-%04d",
    "script": "frames.bat"
  },
  "video": "F6",
  "num-frames": 3,
  "tracks": [
    )" + std::string{track} + R"(
  ]
})";
}

std::string remap_indices_json()
{
    std::string result{"["};
    for (int i = 0; i < 256; ++i)
    {
        if (i != 0)
        {
            result += ",";
        }
        result += std::to_string(i);
    }
    result += "]";
    return result;
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

TEST(TestJsonSchema, enumMetadataAccepted)
{
    EXPECT_TRUE(validates_parameter_catalog_text(catalog_with_metadata(R"("type":"enum","values":["bof60","zmag"])")));
}

TEST(TestJsonSchema, insideAndOutsideMetadataAccepted)
{
    EXPECT_TRUE(validates_parameter_catalog_text(
        catalog_with_metadata(R"("type":"inside","values":["bof60"],"min":0,"max":255)")));
    EXPECT_TRUE(validates_parameter_catalog_text(
        catalog_with_metadata(R"("type":"outside","values":["iter"],"min":0,"max":255)")));
}

TEST(TestJsonSchema, tupleAliasMetadataAccepted)
{
    EXPECT_TRUE(validates_parameter_catalog_text(catalog_with_metadata(R"("type":"point3")")));
    EXPECT_TRUE(validates_parameter_catalog_text(catalog_with_metadata(R"("type":"vector3","normalize":true)")));
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

TEST(TestJsonSchema, invalidMetadataNormalizeRejected)
{
    EXPECT_FALSE(validates_parameter_catalog_text(catalog_with_metadata(R"("type":"vector3","normalize":"true")")));
}

TEST(TestJsonSchema, invalidEnumMetadataRejected)
{
    EXPECT_FALSE(validates_parameter_catalog_text(catalog_with_metadata(R"("type":"enum")")));
    EXPECT_FALSE(validates_parameter_catalog_text(catalog_with_metadata(R"("type":"inside")")));
    EXPECT_FALSE(validates_parameter_catalog_text(catalog_with_metadata(R"("type":"outside")")));
    EXPECT_FALSE(validates_parameter_catalog_text(catalog_with_metadata(R"("type":"enum","values":"id-functions")")));
    EXPECT_FALSE(validates_parameter_catalog_text(catalog_with_metadata(R"("type":"enum","values":["a",1])")));
    EXPECT_FALSE(validates_parameter_catalog_text(catalog_with_metadata(R"("type":"integer","values":["a"])")));
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

TEST(TestJsonSchema, pwmTrackAccepted)
{
    EXPECT_TRUE(validates_config_text(R"({
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
      "parameter": "inside",
      "mode": "pwm",
      "a": "bof60",
      "b": "zmag",
      "window": 2,
      "keys": [
        { "frame": 0, "mix": 0.0 },
        { "frame": 1, "mix": 1.0 }
      ]
    }
  ]
})"));
}

TEST(TestJsonSchema, pathTrackAccepted)
{
    EXPECT_TRUE(validates_config_text(
        config_with_track(R"({"parameter":"maxiter","path":{"kind":"constant","value":"321"}})")));
    EXPECT_TRUE(validates_config_text(
        config_with_track(R"({"parameter":"maxiter","path":{"kind":"line","from":"100","to":"200"}})")));
    EXPECT_TRUE(validates_config_text(config_with_track(
        R"({"parameter":"params.c","path":{"kind":"circle","center":"0/0","radius":1,"turns":1,"phase":90}})")));
    EXPECT_TRUE(validates_config_text(config_with_track(
        R"({"parameter":"look-at","path":{"kind":"ellipse","center":"0/0","x-radius":2,"y-radius":1}})")));
    EXPECT_TRUE(validates_config_text(config_with_track(R"({
      "parameter": "params.c",
      "path": {
        "kind": "lissajous",
        "center": "0/0",
        "x-radius": 2,
        "y-radius": 1,
        "x-frequency": 3,
        "y-frequency": 2,
        "phase": 45
      }
    })")));
    EXPECT_TRUE(validates_config_text(config_with_track(R"({
      "parameter": "params.c",
      "path": {
        "kind": "spiral",
        "center": "0/0",
        "from-radius": 1,
        "to-radius": 3,
        "turns": 1
      }
    })")));
    EXPECT_TRUE(validates_config_text(config_with_track(R"({
      "parameter": "params.c",
      "path": {
        "kind": "bezier",
        "control-points": [ "0/1", "2/3", "4/5" ]
      }
    })")));
    EXPECT_TRUE(validates_config_text(config_with_track(R"({
      "parameter": "params.c",
      "path": {
        "kind": "catmull-rom",
        "control-points": [ "0/0", "1/2", "3/2", "4/0" ]
      }
    })")));
}

TEST(TestJsonSchema, pathTrackRejectsKeysAndPath)
{
    EXPECT_FALSE(validates_config_text(config_with_track(R"({
      "parameter": "maxiter",
      "path": { "kind": "constant", "value": "321" },
      "keys": [
        { "frame": 0, "value": "100" },
        { "frame": 2, "value": "200" }
      ]
    })")));
}

TEST(TestJsonSchema, unknownPathKindRejected)
{
    EXPECT_FALSE(validates_config_text(
        config_with_track(R"({"parameter":"maxiter","path":{"kind":"unknown","value":"321"}})")));
}

TEST(TestJsonSchema, pathTrackRejectsInvalidRadius)
{
    EXPECT_FALSE(validates_config_text(
        config_with_track(R"({"parameter":"params.c","path":{"kind":"circle","center":"0/0","radius":-1}})")));
    EXPECT_FALSE(validates_config_text(config_with_track(
        R"({"parameter":"look-at","path":{"kind":"ellipse","center":"0/0","x-radius":1,"y-radius":-1}})")));
    EXPECT_FALSE(validates_config_text(config_with_track(R"({
      "parameter": "params.c",
      "path": {
        "kind": "lissajous",
        "center": "0/0",
        "x-radius": 1,
        "y-radius": 1,
        "x-frequency": 0,
        "y-frequency": 1
      }
    })")));
    EXPECT_FALSE(validates_config_text(config_with_track(
        R"({"parameter":"params.c","path":{"kind":"spiral","center":"0/0","from-radius":1,"to-radius":-1}})")));
    EXPECT_FALSE(validates_config_text(config_with_track(
        R"({"parameter":"params.c","path":{"kind":"bezier","control-points":["0/1"]}})")));
    EXPECT_FALSE(validates_config_text(config_with_track(
        R"({"parameter":"params.c","path":{"kind":"catmull-rom","control-points":["0/0","1/1","2/2"]}})")));
}

TEST(TestJsonSchema, unknownTrackModeRejected)
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
      "parameter": "inside",
      "mode": "unknown",
      "a": "bof60",
      "b": "zmag",
      "window": 2,
      "keys": [
        { "frame": 0, "mix": 0.0 },
        { "frame": 1, "mix": 1.0 }
      ]
    }
  ]
})"));
}

TEST(TestJsonSchema, pwmWindowBelowTwoRejected)
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
      "parameter": "inside",
      "mode": "pwm",
      "a": "bof60",
      "b": "zmag",
      "window": 1,
      "keys": [
        { "frame": 0, "mix": 0.0 },
        { "frame": 1, "mix": 1.0 }
      ]
    }
  ]
})"));
}

TEST(TestJsonSchema, colorMapTrackAccepted)
{
    EXPECT_TRUE(validates_config_text(R"({
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
      "parameter": "colors",
      "type": "color-map",
      "format": "at-file",
      "output": "colors-%04d.map",
      "keys": [
        { "frame": 0, "value": "fire.map" },
        { "frame": 1, "value": "ice.map" }
      ]
    }
  ]
})"));
}

TEST(TestJsonSchema, colorMapEffectTrackAccepted)
{
    EXPECT_TRUE(validates_config_text(R"({
  "parameter-catalogs": [ "core-catalog.json" ],
  "source": { "file": "from.par", "name": "Mandel_Demo" },
  "output": {
    "directory": "out",
    "par": "frames.par",
    "entry": "frame-%04d",
    "script": "frames.bat"
  },
  "video": "F6",
  "num-frames": 5,
  "tracks": [
    {
      "parameter": "colors",
      "type": "color-map",
      "format": "at-file",
      "output": "colors-%04d.map",
      "source": "base.map",
      "effects": [
        { "kind": "reverse", "range": [ 2, 5 ] },
        {
          "kind": "ping-pong",
          "range": [ 2, 5 ],
          "offset": {
            "keys": [
              { "frame": 0, "value": 0 },
              { "frame": 4, "value": 4 }
            ]
          }
        }
      ]
    }
  ]
})"));
}

TEST(TestJsonSchema, colorMapBrightnessEffectAccepted)
{
    EXPECT_TRUE(validates_config_text(R"({
  "parameter-catalogs": [ "core-catalog.json" ],
  "source": { "file": "from.par", "name": "Mandel_Demo" },
  "output": {
    "directory": "out",
    "par": "frames.par",
    "entry": "frame-%04d",
    "script": "frames.bat"
  },
  "video": "F6",
  "num-frames": 3,
  "tracks": [
    {
      "parameter": "colors",
      "type": "color-map",
      "format": "at-file",
      "output": "colors-%04d.map",
      "source": "base.map",
      "effects": [
        {
          "kind": "brightness",
          "amount": {
            "keys": [
              { "frame": 0, "value": 1.0 },
              { "frame": 2, "value": 2.0 }
            ]
          }
        }
      ]
    }
  ]
})"));
}

TEST(TestJsonSchema, colorMapBrightnessEffectRequiresAmount)
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
  "num-frames": 3,
  "tracks": [
    {
      "parameter": "colors",
      "type": "color-map",
      "format": "at-file",
      "output": "colors-%04d.map",
      "source": "base.map",
      "effects": [
        { "kind": "brightness" }
      ]
    }
  ]
})"));
}

TEST(TestJsonSchema, colorMapAdjustmentEffectsAccepted)
{
    const std::string amount{R"("amount":{"keys":[{"frame":0,"value":1.0},{"frame":2,"value":2.0}]})"};

    EXPECT_TRUE(validates_config_text(config_with_color_map_effect(R"({"kind":"gamma",)" + amount + "}")));
    EXPECT_TRUE(validates_config_text(config_with_color_map_effect(R"({"kind":"contrast",)" + amount + "}")));
    EXPECT_TRUE(validates_config_text(config_with_color_map_effect(R"({"kind":"saturation",)" + amount + "}")));
    EXPECT_TRUE(validates_config_text(config_with_color_map_effect(R"({"kind":"hue-shift",)" + amount + "}")));
}

TEST(TestJsonSchema, colorMapAdjustmentEffectsRequireAmount)
{
    EXPECT_FALSE(validates_config_text(config_with_color_map_effect(R"({"kind":"gamma"})")));
    EXPECT_FALSE(validates_config_text(config_with_color_map_effect(R"({"kind":"contrast"})")));
    EXPECT_FALSE(validates_config_text(config_with_color_map_effect(R"({"kind":"saturation"})")));
    EXPECT_FALSE(validates_config_text(config_with_color_map_effect(R"({"kind":"hue-shift"})")));
}

TEST(TestJsonSchema, colorMapGammaEffectRequiresPositiveAmount)
{
    EXPECT_FALSE(validates_config_text(config_with_color_map_effect(
        R"({"kind":"gamma","amount":{"keys":[{"frame":0,"value":0.0},{"frame":2,"value":1.0}]}})")));
}

TEST(TestJsonSchema, colorMapMaskedEffectsAccepted)
{
    const std::string unit_amount{R"("amount":{"keys":[{"frame":0,"value":0.0},{"frame":2,"value":1.0}]})"};
    const std::string byte_amount{R"("amount":{"keys":[{"frame":0,"value":0.0},{"frame":2,"value":32.0}]})"};

    EXPECT_TRUE(validates_config_text(
        config_with_color_map_effect(R"({"kind":"pulse","range":[0,1],"color":"white",)" + unit_amount + "}")));
    EXPECT_TRUE(validates_config_text(config_with_color_map_effect(
        R"({"kind":"mask-blend","ranges":[[0,1]],"source":"mask.map",)" + unit_amount + "}")));
    EXPECT_TRUE(validates_config_text(
        config_with_color_map_effect(R"({"kind":"remap","indices":)" + remap_indices_json() + "}")));
    EXPECT_TRUE(validates_config_text(
        config_with_color_map_effect(R"({"kind":"sparkle","range":[0,1],"seed":1234,)" + byte_amount + "}")));
}

TEST(TestJsonSchema, colorMapMaskedEffectsRejectInvalidRequiredFields)
{
    const std::string unit_amount{R"("amount":{"keys":[{"frame":0,"value":0.0},{"frame":2,"value":1.0}]})"};
    const std::string byte_amount{R"("amount":{"keys":[{"frame":0,"value":0.0},{"frame":2,"value":32.0}]})"};

    EXPECT_FALSE(validates_config_text(
        config_with_color_map_effect(R"({"kind":"pulse","color":"white",)" + unit_amount + "}")));
    EXPECT_FALSE(validates_config_text(config_with_color_map_effect(
        R"({"kind":"mask-blend","ranges":[],"source":"mask.map",)" + unit_amount + "}")));
    EXPECT_FALSE(validates_config_text(config_with_color_map_effect(R"({"kind":"remap","indices":[0,1]})")));
    EXPECT_FALSE(validates_config_text(
        config_with_color_map_effect(R"({"kind":"sparkle","range":[0,1],)" + byte_amount + "}")));
}

TEST(TestJsonSchema, colorMapMaskedEffectAmountsRejectOutOfRangeValues)
{
    EXPECT_FALSE(validates_config_text(config_with_color_map_effect(
        R"({"kind":"pulse","range":[0,1],"color":"white","amount":{"keys":[{"frame":0,"value":0.0},{"frame":2,"value":1.1}]}})")));
    EXPECT_FALSE(validates_config_text(config_with_color_map_effect(
        R"({"kind":"sparkle","range":[0,1],"seed":1234,"amount":{"keys":[{"frame":0,"value":0.0},{"frame":2,"value":256.0}]}})")));
}

TEST(TestJsonSchema, colorMapGradientSourceAccepted)
{
    EXPECT_TRUE(validates_config_text(R"({
  "parameter-catalogs": [ "core-catalog.json" ],
  "source": { "file": "from.par", "name": "Mandel_Demo" },
  "output": {
    "directory": "out",
    "par": "frames.par",
    "entry": "frame-%04d",
    "script": "frames.bat"
  },
  "video": "F6",
  "num-frames": 1,
  "tracks": [
    {
      "parameter": "colors",
      "type": "color-map",
      "format": "at-file",
      "output": "colors-%04d.map",
      "source": {
        "kind": "gradient",
        "stops": [
          { "index": 0, "color": "black" },
          { "index": 255, "color": "white" }
        ]
      }
    }
  ]
})"));
}

TEST(TestJsonSchema, colorMapGradientSourceRequiresTwoStops)
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
  "num-frames": 1,
  "tracks": [
    {
      "parameter": "colors",
      "type": "color-map",
      "format": "at-file",
      "output": "colors-%04d.map",
      "source": {
        "kind": "gradient",
        "stops": [
          { "index": 0, "color": "black" }
        ]
      }
    }
  ]
})"));
}

TEST(TestJsonSchema, colorMapGradientSourceColorSpecificationsAccepted)
{
    EXPECT_TRUE(validates_config_text(config_with_gradient_stop_color("white")));
    EXPECT_TRUE(validates_config_text(config_with_gradient_stop_color("rgb:255/40/0")));
    EXPECT_TRUE(validates_config_text(config_with_gradient_stop_color("255/40/0")));
    EXPECT_TRUE(validates_config_text(config_with_gradient_stop_color("hsv:20/1/1")));
    EXPECT_TRUE(validates_config_text(config_with_gradient_stop_color("hsl:60/1/0.5")));
}

TEST(TestJsonSchema, malformedGradientColorSpecificationRejected)
{
    EXPECT_FALSE(validates_config_text(config_with_gradient_stop_color("rgb:300/0/0")));
    EXPECT_FALSE(validates_config_text(config_with_gradient_stop_color("rgb:1/2")));
    EXPECT_FALSE(validates_config_text(config_with_gradient_stop_color("cmyk:0/0/0/0")));
}

TEST(TestJsonSchema, invalidColorMapEffectRangeRejected)
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
      "parameter": "colors",
      "type": "color-map",
      "format": "at-file",
      "output": "colors-%04d.map",
      "source": "base.map",
      "effects": [
        { "kind": "reverse", "range": [ 0, 256 ] }
      ]
    }
  ]
})"));
}

TEST(TestJsonSchema, unknownTrackTypeRejected)
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
      "parameter": "colors",
      "type": "unknown",
      "format": "at-file",
      "output": "colors-%04d.map",
      "keys": [
        { "frame": 0, "value": "fire.map" },
        { "frame": 1, "value": "ice.map" }
      ]
    }
  ]
})"));
}

TEST(TestJsonSchema, unknownColorMapFormatRejected)
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
      "parameter": "colors",
      "type": "color-map",
      "format": "unknown",
      "output": "colors-%04d.map",
      "keys": [
        { "frame": 0, "value": "fire.map" },
        { "frame": 1, "value": "ice.map" }
      ]
    }
  ]
})"));
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
