// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/Config.h>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <stdexcept>

using Object = nlohmann::json;

namespace
{

Object valid_json()
{
    return Object{
        {"parameter-catalogs", Object::array({"core-catalog.json"})}, //
        {"source", Object{{"file", "foo.par"}, {"name", "foo"}}},     //
        {"output",
            Object{{"directory", "out"},    //
                {"par", "output.par"},      //
                {"entry", "frame-%04d"},    //
                {"script", "output.bat"}}}, //
        {"video", "F6"},                    //
        {"num-frames", 60},                 //
        {"tracks", Object::array()}         //
    };
}

ParFile::Config valid_config()
{
    return {{"core-catalog.json"},                         //
        {"foo.par", "foo"},                                //
        {"out", "output.par", "frame-%04d", "output.bat"}, //
        1,                                                 //
        "F6",                                              //
        60,                                                //
        {}};                                               //
}

Object identity_indices()
{
    Object result = Object::array();
    for (int i = 0; i < 256; ++i)
    {
        result.push_back(i);
    }
    return result;
}

void expect_invalid(const Object &json)
{
    EXPECT_THROW(static_cast<void>(ParFile::read_config(json.dump())), std::runtime_error);
}

} // namespace

TEST(TestConfig, minimumValid)
{
    ParFile::Config config{valid_config()};

    EXPECT_EQ("foo.par", config.source.file);
    ASSERT_EQ(1U, config.parameter_catalogs.size());
    EXPECT_EQ("core-catalog.json", config.parameter_catalogs[0]);
    EXPECT_EQ("foo", config.source.name);
    EXPECT_EQ("out", config.output.directory);
    EXPECT_EQ("output.par", config.output.par);
    EXPECT_EQ("frame-%04d", config.output.entry);
    EXPECT_EQ("output.bat", config.output.script);
    EXPECT_EQ(1, config.parallel);
    EXPECT_EQ("F6", config.video);
    EXPECT_EQ(60, config.num_frames);
    EXPECT_EQ(0U, config.tracks.size());
}

TEST(TestConfig, jsonDeserializesMinimumValid)
{
    const ParFile::Config config{ParFile::read_config(valid_json().dump())};

    EXPECT_EQ("foo.par", config.source.file);
    ASSERT_EQ(1U, config.parameter_catalogs.size());
    EXPECT_EQ("core-catalog.json", config.parameter_catalogs[0]);
    EXPECT_EQ("foo", config.source.name);
    EXPECT_EQ("out", config.output.directory);
    EXPECT_EQ("output.par", config.output.par);
    EXPECT_EQ("frame-%04d", config.output.entry);
    EXPECT_EQ("output.bat", config.output.script);
    EXPECT_EQ(1, config.parallel);
    EXPECT_EQ("F6", config.video);
    EXPECT_EQ(60, config.num_frames);
    EXPECT_TRUE(config.tracks.empty());
}

TEST(TestConfig, optionalParallelValid)
{
    Object json = valid_json();
    json["parallel"] = 20;

    const ParFile::Config config{ParFile::read_config(json.dump())};

    EXPECT_EQ(20, config.parallel);
}

TEST(TestConfig, missingParameterCatalogs)
{
    Object json = valid_json();
    json.erase("parameter-catalogs");

    expect_invalid(json);
}

TEST(TestConfig, missingSource)
{
    Object json = valid_json();
    json.erase("source");

    expect_invalid(json);
}

TEST(TestConfig, sourceMissingFile)
{
    Object json = valid_json();
    json.at("source").erase("file");

    expect_invalid(json);
}

TEST(TestConfig, sourceMissingName)
{
    Object json = valid_json();
    json.at("source").erase("name");

    expect_invalid(json);
}

TEST(TestConfig, missingOutput)
{
    Object json = valid_json();
    json.erase("output");

    expect_invalid(json);
}

TEST(TestConfig, outputMissingDirectory)
{
    Object json = valid_json();
    json.at("output").erase("directory");

    expect_invalid(json);
}

TEST(TestConfig, outputMissingPar)
{
    Object json = valid_json();
    json.at("output").erase("par");

    expect_invalid(json);
}

TEST(TestConfig, outputMissingEntry)
{
    Object json = valid_json();
    json.at("output").erase("entry");

    expect_invalid(json);
}

TEST(TestConfig, outputMissingScript)
{
    Object json = valid_json();
    json.at("output").erase("script");

    expect_invalid(json);
}

TEST(TestConfig, missingVideo)
{
    Object json = valid_json();
    json.erase("video");

    expect_invalid(json);
}

TEST(TestConfig, missingNumFrames)
{
    Object json = valid_json();
    json.erase("num-frames");

    expect_invalid(json);
}

TEST(TestConfig, missingTracks)
{
    Object json = valid_json();
    json.erase("tracks");

    expect_invalid(json);
}

TEST(TestConfig, oneTrackValid)
{
    ParFile::Config data{valid_config()};
    data.tracks = {{"center-mag", {{0, "-0.5/0/1"}, {2, "-0.5/0/10", ParFile::Curve::HOLD}}}};

    ParFile::Config config{data};

    ASSERT_EQ(1U, config.tracks.size());
    EXPECT_EQ("center-mag", config.tracks[0].parameter);
    ASSERT_EQ(2U, config.tracks[0].keys.size());
    EXPECT_EQ(0, config.tracks[0].keys[0].frame);
    EXPECT_EQ("-0.5/0/1", config.tracks[0].keys[0].value);
    EXPECT_FALSE(config.tracks[0].keys[0].curve);
    EXPECT_EQ(2, config.tracks[0].keys[1].frame);
    EXPECT_EQ("-0.5/0/10", config.tracks[0].keys[1].value);
    ASSERT_TRUE(config.tracks[0].keys[1].curve);
    EXPECT_EQ(ParFile::Curve::HOLD, *config.tracks[0].keys[1].curve);
}

TEST(TestConfig, jsonDeserializesOneTrack)
{
    Object json = valid_json();
    json["tracks"] = Object::array({Object{{"parameter", "center-mag"},
        {"keys",
            Object::array({Object{{"frame", 0}, {"value", "-0.5/0/1"}},
                Object{{"frame", 2}, {"value", "-0.5/0/10"}, {"curve", "hold"}}})}}});

    const ParFile::Config config{ParFile::read_config(json.dump())};

    ASSERT_EQ(1U, config.tracks.size());
    EXPECT_EQ("center-mag", config.tracks[0].parameter);
    ASSERT_EQ(2U, config.tracks[0].keys.size());
    EXPECT_EQ(0, config.tracks[0].keys[0].frame);
    EXPECT_EQ("-0.5/0/1", config.tracks[0].keys[0].value);
    EXPECT_FALSE(config.tracks[0].keys[0].curve);
    EXPECT_EQ(2, config.tracks[0].keys[1].frame);
    EXPECT_EQ("-0.5/0/10", config.tracks[0].keys[1].value);
    ASSERT_TRUE(config.tracks[0].keys[1].curve);
    EXPECT_EQ(ParFile::Curve::HOLD, *config.tracks[0].keys[1].curve);
}

TEST(TestConfig, jsonDeserializesConstantPathTrack)
{
    Object json = valid_json();
    json["tracks"] = Object::array(
        {Object{{"parameter", "maxiter"}, {"path", Object{{"kind", "constant"}, {"value", "321"}}}}});

    const ParFile::Config config{ParFile::read_config(json.dump())};

    ASSERT_EQ(1U, config.tracks.size());
    EXPECT_EQ("maxiter", config.tracks[0].parameter);
    ASSERT_EQ(2U, config.tracks[0].keys.size());
    EXPECT_EQ(0, config.tracks[0].keys[0].frame);
    EXPECT_EQ("321", config.tracks[0].keys[0].value);
    EXPECT_FALSE(config.tracks[0].keys[0].curve);
    EXPECT_EQ(59, config.tracks[0].keys[1].frame);
    EXPECT_EQ("321", config.tracks[0].keys[1].value);
    ASSERT_TRUE(config.tracks[0].keys[1].curve);
    EXPECT_EQ(ParFile::Curve::HOLD, *config.tracks[0].keys[1].curve);
}

TEST(TestConfig, jsonDeserializesLinePathTrack)
{
    Object json = valid_json();
    json["tracks"] = Object::array(
        {Object{{"parameter", "params.c"}, {"path", Object{{"kind", "line"}, {"from", "0/1"}, {"to", "2/3"}}}}});

    const ParFile::Config config{ParFile::read_config(json.dump())};

    ASSERT_EQ(1U, config.tracks.size());
    EXPECT_EQ("params.c", config.tracks[0].parameter);
    ASSERT_EQ(2U, config.tracks[0].keys.size());
    EXPECT_EQ(0, config.tracks[0].keys[0].frame);
    EXPECT_EQ("0/1", config.tracks[0].keys[0].value);
    EXPECT_FALSE(config.tracks[0].keys[0].curve);
    EXPECT_EQ(59, config.tracks[0].keys[1].frame);
    EXPECT_EQ("2/3", config.tracks[0].keys[1].value);
    ASSERT_TRUE(config.tracks[0].keys[1].curve);
    EXPECT_EQ(ParFile::Curve::LINEAR, *config.tracks[0].keys[1].curve);
}

TEST(TestConfig, jsonDeserializesCirclePathTrack)
{
    Object json = valid_json();
    json["tracks"] = Object::array({Object{{"parameter", "params.c"},
        {"path",
            Object{{"kind", "circle"}, {"center", "0/0"}, {"radius", 1.5}, {"turns", 2.0}, {"phase", 90.0}}}}});

    const ParFile::Config config{ParFile::read_config(json.dump())};

    ASSERT_EQ(1U, config.tracks.size());
    ASSERT_TRUE(config.tracks[0].path);
    EXPECT_EQ(ParFile::PathKind::CIRCLE, config.tracks[0].path->kind);
    EXPECT_EQ("0/0", config.tracks[0].path->center);
    EXPECT_DOUBLE_EQ(1.5, config.tracks[0].path->radius);
    EXPECT_DOUBLE_EQ(2.0, config.tracks[0].path->turns);
    EXPECT_DOUBLE_EQ(90.0, config.tracks[0].path->phase);
    EXPECT_TRUE(config.tracks[0].keys.empty());
}

TEST(TestConfig, jsonDeserializesEllipsePathTrack)
{
    Object json = valid_json();
    json["tracks"] = Object::array({Object{{"parameter", "look-at"},
        {"path", Object{{"kind", "ellipse"}, {"center", "0/0"}, {"x-radius", 2.0}, {"y-radius", 1.0}}}}});

    const ParFile::Config config{ParFile::read_config(json.dump())};

    ASSERT_EQ(1U, config.tracks.size());
    ASSERT_TRUE(config.tracks[0].path);
    EXPECT_EQ(ParFile::PathKind::ELLIPSE, config.tracks[0].path->kind);
    EXPECT_EQ("0/0", config.tracks[0].path->center);
    EXPECT_DOUBLE_EQ(2.0, config.tracks[0].path->x_radius);
    EXPECT_DOUBLE_EQ(1.0, config.tracks[0].path->y_radius);
    EXPECT_DOUBLE_EQ(1.0, config.tracks[0].path->turns);
    EXPECT_DOUBLE_EQ(0.0, config.tracks[0].path->phase);
    EXPECT_TRUE(config.tracks[0].keys.empty());
}

TEST(TestConfig, jsonDeserializesLissajousPathTrack)
{
    Object json = valid_json();
    json["tracks"] = Object::array({Object{{"parameter", "params.c"},
        {"path",
            Object{{"kind", "lissajous"},
                {"center", "0/0"},
                {"x-radius", 2.0},
                {"y-radius", 1.0},
                {"x-frequency", 3.0},
                {"y-frequency", 2.0},
                {"phase", 45.0}}}}});

    const ParFile::Config config{ParFile::read_config(json.dump())};

    ASSERT_EQ(1U, config.tracks.size());
    ASSERT_TRUE(config.tracks[0].path);
    EXPECT_EQ(ParFile::PathKind::LISSAJOUS, config.tracks[0].path->kind);
    EXPECT_DOUBLE_EQ(2.0, config.tracks[0].path->x_radius);
    EXPECT_DOUBLE_EQ(1.0, config.tracks[0].path->y_radius);
    EXPECT_DOUBLE_EQ(3.0, config.tracks[0].path->x_frequency);
    EXPECT_DOUBLE_EQ(2.0, config.tracks[0].path->y_frequency);
    EXPECT_DOUBLE_EQ(45.0, config.tracks[0].path->phase);
    EXPECT_TRUE(config.tracks[0].keys.empty());
}

TEST(TestConfig, jsonDeserializesSpiralPathTrack)
{
    Object json = valid_json();
    json["tracks"] = Object::array({Object{{"parameter", "params.c"},
        {"path",
            Object{
                {"kind", "spiral"}, {"center", "0/0"}, {"from-radius", 1.0}, {"to-radius", 3.0}, {"turns", 2.0}}}}});

    const ParFile::Config config{ParFile::read_config(json.dump())};

    ASSERT_EQ(1U, config.tracks.size());
    ASSERT_TRUE(config.tracks[0].path);
    EXPECT_EQ(ParFile::PathKind::SPIRAL, config.tracks[0].path->kind);
    EXPECT_DOUBLE_EQ(1.0, config.tracks[0].path->from_radius);
    EXPECT_DOUBLE_EQ(3.0, config.tracks[0].path->to_radius);
    EXPECT_DOUBLE_EQ(2.0, config.tracks[0].path->turns);
    EXPECT_DOUBLE_EQ(0.0, config.tracks[0].path->phase);
    EXPECT_TRUE(config.tracks[0].keys.empty());
}

TEST(TestConfig, jsonRejectsTrackWithKeysAndPath)
{
    Object json = valid_json();
    json["tracks"] = Object::array({Object{{"parameter", "maxiter"},
        {"path", Object{{"kind", "constant"}, {"value", "321"}}},
        {"keys", Object::array({Object{{"frame", 0}, {"value", "100"}}, Object{{"frame", 59}, {"value", "200"}}})}}});

    expect_invalid(json);
}

TEST(TestConfig, jsonRejectsUnknownPathKind)
{
    Object json = valid_json();
    json["tracks"] =
        Object::array({Object{{"parameter", "maxiter"}, {"path", Object{{"kind", "unknown"}, {"value", "321"}}}}});

    expect_invalid(json);
}

TEST(TestConfig, jsonRejectsInvalidPathFrequencyOrRadius)
{
    Object json = valid_json();
    json["tracks"] = Object::array({Object{{"parameter", "params.c"},
        {"path",
            Object{{"kind", "lissajous"},
                {"center", "0/0"},
                {"x-radius", 1.0},
                {"y-radius", 1.0},
                {"x-frequency", 0.0},
                {"y-frequency", 1.0}}}}});
    expect_invalid(json);

    json["tracks"] = Object::array({Object{{"parameter", "params.c"},
        {"path", Object{{"kind", "spiral"}, {"center", "0/0"}, {"from-radius", 1.0}, {"to-radius", -1.0}}}}});
    expect_invalid(json);
}

TEST(TestConfig, jsonDeserializesPwmTrack)
{
    Object json = valid_json();
    json["tracks"] =
        Object::array({Object{{"parameter", "inside"}, {"mode", "pwm"}, {"a", "bof60"}, {"b", "zmag"}, {"window", 8},
            {"keys", Object::array({Object{{"frame", 0}, {"mix", 0.0}}, Object{{"frame", 59}, {"mix", 1.0}}})}}});

    const ParFile::Config config{ParFile::read_config(json.dump())};

    ASSERT_EQ(1U, config.tracks.size());
    EXPECT_EQ("inside", config.tracks[0].parameter);
    EXPECT_EQ(ParFile::TrackMode::PWM, config.tracks[0].mode);
    ASSERT_TRUE(config.tracks[0].pwm);
    EXPECT_EQ("bof60", config.tracks[0].pwm->a);
    EXPECT_EQ("zmag", config.tracks[0].pwm->b);
    EXPECT_EQ(8, config.tracks[0].pwm->window);
    ASSERT_EQ(2U, config.tracks[0].keys.size());
    ASSERT_TRUE(config.tracks[0].keys[0].mix);
    ASSERT_TRUE(config.tracks[0].keys[1].mix);
    EXPECT_DOUBLE_EQ(0.0, *config.tracks[0].keys[0].mix);
    EXPECT_DOUBLE_EQ(1.0, *config.tracks[0].keys[1].mix);
}

TEST(TestConfig, jsonDeserializesColorMapTrack)
{
    Object json = valid_json();
    json["tracks"] = Object::array(
        {Object{{"parameter", "colors"}, {"type", "color-map"}, {"format", "at-file"}, {"output", "colors-%04d.map"},
            {"keys",
                Object::array(
                    {Object{{"frame", 0}, {"value", "fire.map"}}, Object{{"frame", 59}, {"value", "ice.map"}}})}}});

    const ParFile::Config config{ParFile::read_config(json.dump())};

    ASSERT_EQ(1U, config.tracks.size());
    EXPECT_EQ("colors", config.tracks[0].parameter);
    EXPECT_EQ(ParFile::TrackKind::COLOR_MAP, config.tracks[0].kind);
    ASSERT_TRUE(config.tracks[0].color_map);
    EXPECT_EQ(ParFile::TrackFormat::AT_FILE, config.tracks[0].color_map->format);
    EXPECT_EQ("colors-%04d.map", config.tracks[0].color_map->output);
    ASSERT_EQ(2U, config.tracks[0].keys.size());
    EXPECT_EQ("fire.map", config.tracks[0].keys[0].value);
    EXPECT_EQ("ice.map", config.tracks[0].keys[1].value);
}

TEST(TestConfig, jsonDeserializesColorMapEffectTrack)
{
    Object json = valid_json();
    json["tracks"] =
        Object::array({Object{{"parameter", "colors"},
            {"type", "color-map"},
            {"format", "at-file"},
            {"output", "colors-%04d.map"},
            {"source", "base.map"},
            {"effects",
                Object::array({Object{{"kind", "reverse"}, {"range", Object::array({2, 5})}},
                    Object{{"kind", "ping-pong"},
                        {"range", Object::array({2, 5})},
                        {"offset",
                            Object{{"keys",
                                Object::array({Object{{"frame", 0}, {"value", 0.0}},
                                    Object{{"frame", 4}, {"value", 4.0}}})}}}}})}}});

    const ParFile::Config config{ParFile::read_config(json.dump())};

    ASSERT_EQ(1U, config.tracks.size());
    EXPECT_EQ(ParFile::TrackKind::COLOR_MAP, config.tracks[0].kind);
    ASSERT_TRUE(config.tracks[0].color_map);
    ASSERT_TRUE(config.tracks[0].color_map->source);
    EXPECT_EQ("base.map", *config.tracks[0].color_map->source);
    ASSERT_EQ(2U, config.tracks[0].color_map->effects.size());
    EXPECT_EQ(ParFile::ColorMapEffectKind::REVERSE, config.tracks[0].color_map->effects[0].kind);
    ASSERT_TRUE(config.tracks[0].color_map->effects[0].range);
    EXPECT_EQ(2, config.tracks[0].color_map->effects[0].range->first);
    EXPECT_EQ(5, config.tracks[0].color_map->effects[0].range->last);
    EXPECT_EQ(ParFile::ColorMapEffectKind::PING_PONG, config.tracks[0].color_map->effects[1].kind);
    ASSERT_TRUE(config.tracks[0].color_map->effects[1].offset);
    ASSERT_EQ(2U, config.tracks[0].color_map->effects[1].offset->keys.size());
    EXPECT_EQ(4.0, config.tracks[0].color_map->effects[1].offset->keys[1].value);
}

TEST(TestConfig, jsonDeserializesColorMapBrightnessEffect)
{
    Object json = valid_json();
    json["tracks"] = Object::array({Object{{"parameter", "colors"},
        {"type", "color-map"},
        {"format", "at-file"},
        {"output", "colors-%04d.map"},
        {"source", "base.map"},
        {"effects",
            Object::array({Object{{"kind", "brightness"},
                {"amount",
                    Object{{"keys",
                        Object::array({Object{{"frame", 0}, {"value", 1.0}},
                            Object{{"frame", 4}, {"value", 2.0}}})}}}}})}}});

    const ParFile::Config config{ParFile::read_config(json.dump())};

    ASSERT_EQ(1U, config.tracks.size());
    ASSERT_TRUE(config.tracks[0].color_map);
    ASSERT_EQ(1U, config.tracks[0].color_map->effects.size());
    const ParFile::ColorMapEffectConfig &effect{config.tracks[0].color_map->effects[0]};
    EXPECT_EQ(ParFile::ColorMapEffectKind::BRIGHTNESS, effect.kind);
    ASSERT_TRUE(effect.amount);
    ASSERT_EQ(2U, effect.amount->keys.size());
    EXPECT_EQ(2.0, effect.amount->keys[1].value);
}

TEST(TestConfig, jsonDeserializesColorMapAdjustmentEffects)
{
    Object json = valid_json();
    json["tracks"] = Object::array({Object{{"parameter", "colors"},
        {"type", "color-map"},
        {"format", "at-file"},
        {"output", "colors-%04d.map"},
        {"source", "base.map"},
        {"effects",
            Object::array({Object{{"kind", "gamma"},
                              {"amount",
                                  Object{{"keys",
                                      Object::array({Object{{"frame", 0}, {"value", 1.0}},
                                          Object{{"frame", 4}, {"value", 2.0}}})}}}},
                Object{{"kind", "contrast"},
                    {"amount",
                        Object{{"keys",
                            Object::array({Object{{"frame", 0}, {"value", 1.0}},
                                Object{{"frame", 4}, {"value", 0.5}}})}}}},
                Object{{"kind", "saturation"},
                    {"amount",
                        Object{{"keys",
                            Object::array({Object{{"frame", 0}, {"value", 1.0}},
                                Object{{"frame", 4}, {"value", 0.0}}})}}}},
                Object{{"kind", "hue-shift"},
                    {"amount",
                        Object{{"keys",
                            Object::array({Object{{"frame", 0}, {"value", 0.0}},
                                Object{{"frame", 4}, {"value", 120.0}}})}}}}})}}});

    const ParFile::Config config{ParFile::read_config(json.dump())};

    ASSERT_EQ(1U, config.tracks.size());
    ASSERT_TRUE(config.tracks[0].color_map);
    ASSERT_EQ(4U, config.tracks[0].color_map->effects.size());
    EXPECT_EQ(ParFile::ColorMapEffectKind::GAMMA, config.tracks[0].color_map->effects[0].kind);
    EXPECT_EQ(ParFile::ColorMapEffectKind::CONTRAST, config.tracks[0].color_map->effects[1].kind);
    EXPECT_EQ(ParFile::ColorMapEffectKind::SATURATION, config.tracks[0].color_map->effects[2].kind);
    EXPECT_EQ(ParFile::ColorMapEffectKind::HUE_SHIFT, config.tracks[0].color_map->effects[3].kind);
    ASSERT_TRUE(config.tracks[0].color_map->effects[3].amount);
    EXPECT_EQ(120.0, config.tracks[0].color_map->effects[3].amount->keys[1].value);
}

TEST(TestConfig, jsonDeserializesMaskedColorMapEffects)
{
    Object json = valid_json();
    json["tracks"] = Object::array({Object{{"parameter", "colors"},
        {"type", "color-map"},
        {"format", "at-file"},
        {"output", "colors-%04d.map"},
        {"source", "base.map"},
        {"effects",
            Object::array({Object{{"kind", "pulse"},
                              {"range", Object::array({2, 5})},
                              {"color", "white"},
                              {"amount",
                                  Object{{"keys",
                                      Object::array({Object{{"frame", 0}, {"value", 0.0}},
                                          Object{{"frame", 4}, {"value", 1.0}}})}}}},
                Object{{"kind", "mask-blend"},
                    {"ranges", Object::array({Object::array({0, 1}), Object::array({4, 5})})},
                    {"source", "mask.map"},
                    {"amount",
                        Object{{"keys",
                            Object::array({Object{{"frame", 0}, {"value", 0.0}},
                                Object{{"frame", 4}, {"value", 1.0}}})}}}},
                Object{{"kind", "remap"}, {"indices", identity_indices()}},
                Object{{"kind", "sparkle"},
                    {"range", Object::array({6, 7})},
                    {"seed", 1234},
                    {"amount",
                        Object{{"keys",
                            Object::array({Object{{"frame", 0}, {"value", 0.0}},
                                Object{{"frame", 4}, {"value", 32.0}}})}}}}})}}});

    const ParFile::Config config{ParFile::read_config(json.dump())};

    ASSERT_EQ(1U, config.tracks.size());
    ASSERT_TRUE(config.tracks[0].color_map);
    ASSERT_EQ(4U, config.tracks[0].color_map->effects.size());
    EXPECT_EQ(ParFile::ColorMapEffectKind::PULSE, config.tracks[0].color_map->effects[0].kind);
    ASSERT_TRUE(config.tracks[0].color_map->effects[0].range);
    EXPECT_EQ("white", *config.tracks[0].color_map->effects[0].color);
    EXPECT_EQ(ParFile::ColorMapEffectKind::MASK_BLEND, config.tracks[0].color_map->effects[1].kind);
    ASSERT_EQ(2U, config.tracks[0].color_map->effects[1].ranges.size());
    EXPECT_EQ("mask.map", *config.tracks[0].color_map->effects[1].source);
    EXPECT_EQ(ParFile::ColorMapEffectKind::REMAP, config.tracks[0].color_map->effects[2].kind);
    ASSERT_EQ(256U, config.tracks[0].color_map->effects[2].indices.size());
    EXPECT_EQ(ParFile::ColorMapEffectKind::SPARKLE, config.tracks[0].color_map->effects[3].kind);
    ASSERT_TRUE(config.tracks[0].color_map->effects[3].seed);
    EXPECT_EQ(1234, *config.tracks[0].color_map->effects[3].seed);
}

TEST(TestConfig, jsonDeserializesColorMapGradientSourceTrack)
{
    Object json = valid_json();
    json["tracks"] = Object::array({Object{{"parameter", "colors"},
        {"type", "color-map"},
        {"format", "at-file"},
        {"output", "colors-%04d.map"},
        {"source",
            Object{{"kind", "gradient"},
                {"stops",
                    Object::array(
                        {Object{{"index", 0}, {"color", "black"}}, Object{{"index", 255}, {"color", "white"}}})}}}}});

    const ParFile::Config config{ParFile::read_config(json.dump())};

    ASSERT_EQ(1U, config.tracks.size());
    ASSERT_TRUE(config.tracks[0].color_map);
    ASSERT_TRUE(config.tracks[0].color_map->gradient);
    ASSERT_EQ(2U, config.tracks[0].color_map->gradient->stops.size());
    EXPECT_EQ(0, config.tracks[0].color_map->gradient->stops[0].index);
    EXPECT_EQ("black", config.tracks[0].color_map->gradient->stops[0].color);
    EXPECT_EQ(255, config.tracks[0].color_map->gradient->stops[1].index);
    EXPECT_EQ("white", config.tracks[0].color_map->gradient->stops[1].color);
}

TEST(TestConfig, pwmWindowBelowTwoRejected)
{
    Object json = valid_json();
    json["tracks"] =
        Object::array({Object{{"parameter", "inside"}, {"mode", "pwm"}, {"a", "bof60"}, {"b", "zmag"}, {"window", 1},
            {"keys", Object::array({Object{{"frame", 0}, {"mix", 0.0}}, Object{{"frame", 59}, {"mix", 1.0}}})}}});

    expect_invalid(json);
}

TEST(TestConfig, unknownKeyCurveRejected)
{
    Object json = valid_json();
    json["tracks"] = Object::array({Object{{"parameter", "center-mag"},
        {"keys",
            Object::array({Object{{"frame", 0}, {"value", "-0.5/0/1"}},
                Object{{"frame", 2}, {"value", "-0.5/0/10"}, {"curve", "unknown"}}})}}});

    expect_invalid(json);
}
