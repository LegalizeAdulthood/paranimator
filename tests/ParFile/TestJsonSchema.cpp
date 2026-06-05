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
#include <vector>

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

std::string config_with_layer_compose(std::string_view compose)
{
    return R"({
  "parameter-catalogs": [ "core-catalog.json" ],
  "output": {
    "directory": "out",
    "par": "frames.par",
    "entry": "layer-%s-%04d",
    "script": "frames.bat",
    "frames": "frames/frame%04d.png",
    "layers": "layers/layer-%s-%04d.gif",
    "compose-script": "compose.bat",
    "background": "black"
  },
  "video": "F6",
  "num-frames": 3,
  "layers": [
    {
      "id": "base",
      "source": { "file": "from.par", "name": "Mandel_Demo" },
      "compose": ")" +
        std::string{compose} +
        R"(",
      "tracks": [
        {
          "parameter": "maxiter",
          "keys": [
            { "frame": 0, "value": "100" },
            { "frame": 2, "value": "200" }
          ]
        }
      ]
    }
  ]
})";
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
          { "index": 255, "color": ")" +
        std::string{color} + R"(" }
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
        )" +
        std::string{effect} + R"(
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
    )" + std::string{track} +
        R"(
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

TEST(TestJsonSchema, singleLayerConfigAccepted)
{
    EXPECT_TRUE(validates_config_text(R"({
  "parameter-catalogs": [ "core-catalog.json" ],
  "output": {
    "directory": "out",
    "par": "frames.par",
    "entry": "frame-%04d",
    "script": "frames.bat"
  },
  "video": "F6",
  "num-frames": 3,
  "layers": [
    {
      "id": "base",
      "source": { "file": "from.par", "name": "Mandel_Demo" },
      "tracks": [
        {
          "parameter": "maxiter",
          "keys": [
            { "frame": 0, "value": "100" },
            { "frame": 2, "value": "200" }
          ]
        }
      ]
    }
  ]
})"));
}

TEST(TestJsonSchema, singleLayerRejectsTopLevelTracks)
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
      "parameter": "maxiter",
      "keys": [
        { "frame": 0, "value": "100" },
        { "frame": 2, "value": "200" }
      ]
    }
  ],
  "layers": [
    {
      "id": "base",
      "source": { "file": "from.par", "name": "Mandel_Demo" },
      "tracks": [
        {
          "parameter": "maxiter",
          "keys": [
            { "frame": 0, "value": "100" },
            { "frame": 2, "value": "200" }
          ]
        }
      ]
    }
  ]
})"));
}

TEST(TestJsonSchema, multiLayerConfigAccepted)
{
    EXPECT_TRUE(validates_config_text(R"({
  "parameter-catalogs": [ "core-catalog.json" ],
  "output": {
    "directory": "out",
    "par": "frames.par",
    "entry": "layer-%s-%04d",
    "script": "frames.bat"
  },
  "video": "F6",
  "num-frames": 3,
  "layers": [
    {
      "id": "base",
      "source": { "file": "from.par", "name": "Mandel_Demo" },
      "tracks": [
        {
          "parameter": "maxiter",
          "keys": [
            { "frame": 0, "value": "100" },
            { "frame": 2, "value": "200" }
          ]
        }
      ]
    },
    {
      "id": "detail",
      "source": { "file": "from.par", "name": "Julia_Demo" },
      "tracks": [
        {
          "parameter": "maxiter",
          "keys": [
            { "frame": 0, "value": "300" },
            { "frame": 2, "value": "500" }
          ]
        }
      ]
    }
  ]
})"));
}

TEST(TestJsonSchema, layerOpacityAccepted)
{
    EXPECT_TRUE(validates_config_text(R"({
  "parameter-catalogs": [ "core-catalog.json" ],
  "output": {
    "directory": "out",
    "par": "frames.par",
    "entry": "layer-%s-%04d",
    "script": "frames.bat"
  },
  "video": "F6",
  "num-frames": 3,
  "layers": [
    {
      "id": "base",
      "source": { "file": "from.par", "name": "Mandel_Demo" },
      "opacity": {
        "keys": [
          { "frame": 0, "value": 0 },
          { "frame": 2, "value": 100 }
        ]
      },
      "write-when-hidden": true,
      "tracks": [
        {
          "parameter": "maxiter",
          "keys": [
            { "frame": 0, "value": "100" },
            { "frame": 2, "value": "200" }
          ]
        }
      ]
    }
  ]
})"));
}

TEST(TestJsonSchema, layerComposeOperatorsAccepted)
{
    const std::vector<std::string> operators{"clear", "copy", "destination", "source-over", "destination-over",
        "source-in", "destination-in", "source-out", "destination-out", "source-atop", "destination-atop", "xor", "add",
        "subtract", "multiply", "divide", "min", "max", "difference", "average", "screen", "overlay"};

    for (const std::string &op : operators)
    {
        EXPECT_TRUE(validates_config_text(config_with_layer_compose(op))) << op;
    }
}

TEST(TestJsonSchema, layerOutputRejectsNonGifExtension)
{
    EXPECT_FALSE(validates_config_text(R"({
  "parameter-catalogs": [ "core-catalog.json" ],
  "output": {
    "directory": "out",
    "par": "frames.par",
    "entry": "layer-%s-%04d",
    "script": "frames.bat",
    "frames": "frames/frame%04d.png",
    "layers": "layers/layer-%s-%04d.png",
    "compose-script": "compose.bat"
  },
  "video": "F6",
  "num-frames": 1,
  "layers": [
    {
      "id": "base",
      "source": { "file": "from.par", "name": "Mandel_Demo" },
      "tracks": []
    }
  ]
})"));
}

TEST(TestJsonSchema, layerComposeRejectsUnsupportedOperator)
{
    EXPECT_FALSE(validates_config_text(config_with_layer_compose("hard-light")));
}

TEST(TestJsonSchema, layerComposeRejectsImagemagickOperatorNames)
{
    EXPECT_FALSE(validates_config_text(config_with_layer_compose("Dst_Over")));
    EXPECT_FALSE(validates_config_text(config_with_layer_compose("Over")));
}

TEST(TestJsonSchema, layerOpacityRejectsValuesOutsidePercentRange)
{
    const std::string before{R"({
  "parameter-catalogs": [ "core-catalog.json" ],
  "output": {
    "directory": "out",
    "par": "frames.par",
    "entry": "layer-%s-%04d",
    "script": "frames.bat"
  },
  "video": "F6",
  "num-frames": 3,
  "layers": [
    {
      "id": "base",
      "source": { "file": "from.par", "name": "Mandel_Demo" },
      "opacity": {
        "keys": [
          { "frame": 0, "value": )"};
    const std::string after{R"( },
          { "frame": 2, "value": 100 }
        ]
      },
      "tracks": [
        {
          "parameter": "maxiter",
          "keys": [
            { "frame": 0, "value": "100" },
            { "frame": 2, "value": "200" }
          ]
        }
      ]
    }
  ]
})"};

    EXPECT_FALSE(validates_config_text(before + "-1" + after));
    EXPECT_FALSE(validates_config_text(before + "101" + after));
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
    EXPECT_TRUE(validates_parameter_catalog_file(TestParFile::COLORING_CATALOG_JSON));
    EXPECT_TRUE(validates_parameter_catalog_file(TestParFile::ID_3D_CATALOG_JSON));
    EXPECT_TRUE(validates_parameter_catalog_file(TestParFile::FORMULA_CATALOG_JSON));
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

TEST(TestJsonSchema, colorMapMetadataAccepted)
{
    EXPECT_TRUE(validates_parameter_catalog_text(catalog_with_metadata(R"("type":"color-map","format":"raw")")));
}

TEST(TestJsonSchema, multipleParameterCatalogsAccepted)
{
    const std::string json{R"({
  "parameter-catalogs": [ "core-catalog.json", "coloring-catalog.json" ],
  "source": { "file": "source.par", "name": "Mandel_Demo" },
  "output": {
    "directory": "output",
    "par": "frames.par",
    "entry": "frame-%04d",
    "script": "frames.bat"
  },
  "video": "F6",
  "num-frames": 3,
  "tracks": [
    {
      "parameter": "maxiter",
      "keys": [
        { "frame": 0, "value": "100" },
        { "frame": 2, "value": "200" }
      ]
    }
  ]
})"};

    EXPECT_TRUE(validates_config_text(json));
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

TEST(TestJsonSchema, stringMetadataAccepted)
{
    EXPECT_TRUE(validates_parameter_catalog_text(catalog_with_metadata(R"("type":"string")")));
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
    EXPECT_FALSE(validates_parameter_catalog_text(catalog_with_metadata(R"("type":"string","values":["a"])")));
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

TEST(TestJsonSchema, camera2dTrackAccepted)
{
    EXPECT_TRUE(validates_config_text(config_with_track(R"({
      "name": "camera",
      "type": "camera2d",
      "output": "corners",
      "aspect": "source",
      "look-at": {
        "type": "point2",
        "keys": [
          { "frame": 0, "value": "0/0" },
          { "frame": 2, "value": "1/1" }
        ]
      },
      "view-up": {
        "type": "vector2",
        "normalize": true,
        "keys": [
          { "frame": 0, "value": "0/2" },
          { "frame": 2, "value": "1/1" }
        ]
      },
      "height": {
        "type": "double",
        "keys": [
          { "frame": 0, "value": 4 },
          { "frame": 2, "value": 2, "curve": "geometric" }
        ]
      }
    })")));
}

TEST(TestJsonSchema, camera2dCenterMagTrackAccepted)
{
    EXPECT_TRUE(validates_config_text(config_with_track(R"({
      "name": "camera",
      "type": "camera2d",
      "output": "center-mag",
      "aspect": "source",
      "look-at": {
        "type": "point2",
        "keys": [
          { "frame": 0, "value": "-0.5/0" },
          { "frame": 2, "value": "-0.25/0.5" }
        ]
      },
      "view-up": {
        "type": "vector2",
        "normalize": true,
        "keys": [
          { "frame": 0, "value": "0/1" },
          { "frame": 2, "value": "0/1" }
        ]
      },
      "height": {
        "type": "double",
        "keys": [
          { "frame": 0, "value": 3 },
          { "frame": 2, "value": 1.5, "curve": "geometric" }
        ]
      }
    })")));
}

TEST(TestJsonSchema, camera2dEyeTrackAccepted)
{
    EXPECT_TRUE(validates_config_text(config_with_track(R"({
      "name": "camera",
      "type": "camera2d",
      "output": "center-mag",
      "aspect": "source",
      "look-at": {
        "type": "point2",
        "keys": [
          { "frame": 0, "value": "0/0" },
          { "frame": 2, "value": "0/0" }
        ]
      },
      "eye": {
        "type": "point2",
        "path": {
          "kind": "circle",
          "center": "0/0",
          "radius": 1
        }
      },
      "height": {
        "type": "double",
        "keys": [
          { "frame": 0, "value": 3 },
          { "frame": 2, "value": 3 }
        ]
      }
    })")));
}

TEST(TestJsonSchema, camera2dSkewTrackAccepted)
{
    EXPECT_TRUE(validates_config_text(config_with_track(R"({
      "name": "camera",
      "type": "camera2d",
      "output": "center-mag",
      "aspect": "source",
      "look-at": {
        "type": "point2",
        "keys": [
          { "frame": 0, "value": "0/0" },
          { "frame": 2, "value": "0/0" }
        ]
      },
      "view-up": {
        "type": "vector2",
        "normalize": true,
        "keys": [
          { "frame": 0, "value": "0/1" },
          { "frame": 2, "value": "0/1" }
        ]
      },
      "height": {
        "type": "double",
        "keys": [
          { "frame": 0, "value": 3 },
          { "frame": 2, "value": 3 }
        ]
      },
      "skew": {
        "type": "double",
        "keys": [
          { "frame": 0, "value": 0 },
          { "frame": 2, "value": 10 }
        ]
      }
    })")));
}

TEST(TestJsonSchema, id3DViewTrackAccepted)
{
    EXPECT_TRUE(validates_config_text(config_with_track(R"({
      "name": "view",
      "type": "id-3d-view",
      "outputs": {
        "rotation": "rotation",
        "perspective": "perspective",
        "xyshift": "xyshift"
      },
      "rotation": {
        "type": "numeric-tuple",
        "arity": 3,
        "keys": [
          { "frame": 0, "value": "60/30/0" },
          { "frame": 2, "value": "70/50/10" }
        ]
      },
      "perspective": {
        "type": "integer",
        "keys": [
          { "frame": 0, "value": 0 },
          { "frame": 2, "value": 100 }
        ]
      },
      "xyshift": {
        "type": "numeric-tuple",
        "arity": 2,
        "keys": [
          { "frame": 0, "value": "0/0" },
          { "frame": 2, "value": "20/-10" }
        ]
      }
    })")));
}

TEST(TestJsonSchema, id3DViewMoreOutputsAccepted)
{
    EXPECT_TRUE(validates_config_text(config_with_track(R"({
      "name": "view",
      "type": "id-3d-view",
      "outputs": {
        "scalexyz": "scalexyz",
        "sphere": "sphere",
        "longitude": "longitude",
        "latitude": "latitude",
        "radius": "radius",
        "stereo": "stereo",
        "interocular": "interocular",
        "converge": "converge"
      },
      "scalexyz": {
        "type": "numeric-tuple",
        "arity": 3,
        "keys": [
          { "frame": 0, "value": "90/90/30" },
          { "frame": 2, "value": "100/100/40" }
        ]
      },
      "sphere": {
        "type": "yes-no",
        "keys": [
          { "frame": 0, "value": false },
          { "frame": 2, "value": true }
        ]
      },
      "longitude": {
        "type": "numeric-tuple",
        "arity": 2,
        "keys": [
          { "frame": 0, "value": "180/0" },
          { "frame": 2, "value": "270/-90" }
        ]
      },
      "latitude": {
        "type": "numeric-tuple",
        "arity": 2,
        "keys": [
          { "frame": 0, "value": "-90/90" },
          { "frame": 2, "value": "-45/45" }
        ]
      },
      "radius": {
        "type": "integer",
        "keys": [
          { "frame": 0, "value": 100 },
          { "frame": 2, "value": 120 }
        ]
      },
      "stereo": {
        "type": "integer",
        "keys": [
          { "frame": 0, "value": 0 },
          { "frame": 2, "value": 4 }
        ]
      },
      "interocular": {
        "type": "integer",
        "keys": [
          { "frame": 0, "value": 0 },
          { "frame": 2, "value": 8 }
        ]
      },
      "converge": {
        "type": "integer",
        "keys": [
          { "frame": 0, "value": 0 },
          { "frame": 2, "value": -2 }
        ]
      }
    })")));
}

TEST(TestJsonSchema, id3DViewCamera3DFrameAccepted)
{
    EXPECT_TRUE(validates_config_text(config_with_track(R"({
      "name": "view",
      "type": "id-3d-view",
      "outputs": {
        "rotation": "rotation",
        "perspective": "perspective",
        "xyshift": "xyshift"
      },
      "camera3d": {
        "eye": {
          "type": "point3",
          "keys": [
            { "frame": 0, "value": "0/0/24" },
            { "frame": 2, "value": "10/0/0" }
          ]
        },
        "look-at": {
          "type": "point3",
          "keys": [
            { "frame": 0, "value": "0/0/0" },
            { "frame": 2, "value": "0/0/0" }
          ]
        },
        "view-up": {
          "type": "vector3",
          "normalize": true,
          "keys": [
            { "frame": 0, "value": "0/2/0" },
            { "frame": 2, "value": "0/2/0" }
          ]
        }
      }
    })")));
}

TEST(TestJsonSchema, id3DViewCamera3DFrameRejectsMissingViewUp)
{
    EXPECT_FALSE(validates_config_text(config_with_track(R"({
      "name": "view",
      "type": "id-3d-view",
      "outputs": {
        "rotation": "rotation"
      },
      "camera3d": {
        "eye": {
          "type": "point3",
          "keys": [
            { "frame": 0, "value": "0/0/24" },
            { "frame": 2, "value": "10/0/0" }
          ]
        },
        "look-at": {
          "type": "point3",
          "keys": [
            { "frame": 0, "value": "0/0/0" },
            { "frame": 2, "value": "0/0/0" }
          ]
        }
      }
    })")));
}

TEST(TestJsonSchema, id3DViewStereoRejectsInvalidValue)
{
    EXPECT_FALSE(validates_config_text(config_with_track(R"({
      "name": "view",
      "type": "id-3d-view",
      "outputs": {
        "stereo": "stereo"
      },
      "stereo": {
        "type": "integer",
        "keys": [
          { "frame": 0, "value": 0 },
          { "frame": 2, "value": 5 }
        ]
      }
    })")));
}

TEST(TestJsonSchema, julibrotViewTrackAccepted)
{
    EXPECT_TRUE(validates_config_text(config_with_track(R"({
      "name": "view",
      "type": "julibrot-view",
      "outputs": {
        "mode": "3dmode",
        "geometry": "julibrot3d",
        "eyes": "julibroteyes",
        "from-to": "julibrotfromto"
      },
      "mode": {
        "type": "enum",
        "keys": [
          { "frame": 0, "value": "monocular" },
          { "frame": 2, "value": "lefteye" }
        ]
      },
      "geometry": {
        "type": "numeric-tuple",
        "arity": 6,
        "keys": [
          { "frame": 0, "value": "128/8/8/7/10/24" },
          { "frame": 2, "value": "160/7/6/6/9/20" }
        ]
      },
      "eyes": {
        "type": "double",
        "keys": [
          { "frame": 0, "value": 2.5 },
          { "frame": 2, "value": 1.0 }
        ]
      },
      "from-to": {
        "type": "numeric-tuple",
        "arity": 4,
        "keys": [
          { "frame": 0, "value": "-0.83/-0.83/0.25/-0.25" },
          { "frame": 2, "value": "-0.7/-0.9/0.2/-0.2" }
        ]
      }
    })")));
}

TEST(TestJsonSchema, julibrotViewCamera3DFrameAccepted)
{
    EXPECT_TRUE(validates_config_text(config_with_track(R"({
      "name": "view",
      "type": "julibrot-view",
      "outputs": {
        "geometry": "julibrot3d"
      },
      "camera3d": {
        "eye": {
          "type": "point3",
          "keys": [
            { "frame": 0, "value": "0/0/24" },
            { "frame": 2, "value": "0/0/12" }
          ]
        },
        "look-at": {
          "type": "point3",
          "keys": [
            { "frame": 0, "value": "0/0/0" },
            { "frame": 2, "value": "0/0/0" }
          ]
        },
        "view-up": {
          "type": "vector3",
          "normalize": true,
          "keys": [
            { "frame": 0, "value": "0/1/0" },
            { "frame": 2, "value": "0/1/0" }
          ]
        }
      }
    })")));
}

TEST(TestJsonSchema, julibrotViewRejectsInvalidCameraRequests)
{
    EXPECT_FALSE(validates_config_text(config_with_track(R"({
      "name": "view",
      "type": "julibrot-view",
      "outputs": {
        "mode": "3dmode"
      },
      "look-at": {
        "type": "point3",
        "keys": [
          { "frame": 0, "value": "0/0/0" },
          { "frame": 2, "value": "1/1/1" }
        ]
      },
      "mode": {
        "type": "enum",
        "keys": [
          { "frame": 0, "value": "monocular" },
          { "frame": 2, "value": "lefteye" }
        ]
      }
    })")));
    EXPECT_FALSE(validates_config_text(config_with_track(R"({
      "name": "view",
      "type": "julibrot-view",
      "outputs": {
        "mode": "3dmode"
      },
      "view-up": {
        "type": "vector3",
        "keys": [
          { "frame": 0, "value": "0/1/0" },
          { "frame": 2, "value": "0/1/0" }
        ]
      },
      "mode": {
        "type": "enum",
        "keys": [
          { "frame": 0, "value": "monocular" },
          { "frame": 2, "value": "lefteye" }
        ]
      }
    })")));
}

TEST(TestJsonSchema, julibrotViewRejectsInvalidMode)
{
    EXPECT_FALSE(validates_config_text(config_with_track(R"({
      "name": "view",
      "type": "julibrot-view",
      "outputs": {
        "mode": "3dmode"
      },
      "mode": {
        "type": "enum",
        "keys": [
          { "frame": 0, "value": "left" },
          { "frame": 2, "value": "right" }
        ]
      }
    })")));
}

TEST(TestJsonSchema, camera2dTrackRejectsInvalidShape)
{
    EXPECT_FALSE(validates_config_text(config_with_track(R"({
      "name": "camera",
      "type": "camera2d",
      "output": "corners",
      "aspect": "source",
      "look-at": {
        "type": "vector2",
        "keys": [
          { "frame": 0, "value": "0/0" },
          { "frame": 2, "value": "1/1" }
        ]
      },
      "view-up": {
        "type": "vector2",
        "keys": [
          { "frame": 0, "value": "0/1" },
          { "frame": 2, "value": "0/1" }
        ]
      },
      "height": {
        "type": "double",
        "keys": [
          { "frame": 0, "value": 4 },
          { "frame": 2, "value": 0 }
        ]
      }
    })")));
}

TEST(TestJsonSchema, camera2dTrackRejectsMissingOrientation)
{
    EXPECT_FALSE(validates_config_text(config_with_track(R"({
      "name": "camera",
      "type": "camera2d",
      "output": "corners",
      "aspect": "source",
      "look-at": {
        "type": "point2",
        "keys": [
          { "frame": 0, "value": "0/0" },
          { "frame": 2, "value": "1/1" }
        ]
      },
      "height": {
        "type": "double",
        "keys": [
          { "frame": 0, "value": 4 },
          { "frame": 2, "value": 4 }
        ]
      }
    })")));
}

TEST(TestJsonSchema, camera2dSkewTrackRejectsNonnumericValue)
{
    EXPECT_FALSE(validates_config_text(config_with_track(R"({
      "name": "camera",
      "type": "camera2d",
      "output": "center-mag",
      "aspect": "source",
      "look-at": {
        "type": "point2",
        "keys": [
          { "frame": 0, "value": "0/0" },
          { "frame": 2, "value": "0/0" }
        ]
      },
      "view-up": {
        "type": "vector2",
        "normalize": true,
        "keys": [
          { "frame": 0, "value": "0/1" },
          { "frame": 2, "value": "0/1" }
        ]
      },
      "height": {
        "type": "double",
        "keys": [
          { "frame": 0, "value": 3 },
          { "frame": 2, "value": 3 }
        ]
      },
      "skew": {
        "type": "double",
        "keys": [
          { "frame": 0, "value": 0 },
          { "frame": 2, "value": "10" }
        ]
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
    EXPECT_FALSE(
        validates_config_text(config_with_track(R"({"parameter":"maxiter","path":{"kind":"unknown","value":"321"}})")));
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
    EXPECT_FALSE(validates_config_text(
        config_with_track(R"({"parameter":"params.c","path":{"kind":"bezier","control-points":["0/1"]}})")));
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

    EXPECT_FALSE(
        validates_config_text(config_with_color_map_effect(R"({"kind":"pulse","color":"white",)" + unit_amount + "}")));
    EXPECT_FALSE(validates_config_text(
        config_with_color_map_effect(R"({"kind":"mask-blend","ranges":[],"source":"mask.map",)" + unit_amount + "}")));
    EXPECT_FALSE(validates_config_text(config_with_color_map_effect(R"({"kind":"remap","indices":[0,1]})")));
    EXPECT_FALSE(
        validates_config_text(config_with_color_map_effect(R"({"kind":"sparkle","range":[0,1],)" + byte_amount + "}")));
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
