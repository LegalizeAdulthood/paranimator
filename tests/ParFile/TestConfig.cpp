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
        {"parameter_catalogs", Object::array({"viewport-catalog.json"})},       //
        {"source", Object{{"file", "foo.par"}, {"name", "foo"}}},       //
        {"output", Object{{"directory", "out"},                         //
                       {"par", "output.par"},                           //
                       {"entry", "frame-%04d"},                         //
                       {"script", "output.bat"}}},                      //
        {"video", "F6"},                                                //
        {"num_frames", 60},                                             //
        {"tracks", Object::array()}                                     //
    };
}

void expect_invalid(const Object &json)
{
    EXPECT_THROW(ParFile::Config{json.dump()}, std::runtime_error);
}

} // namespace

TEST(TestConfig, minimumValid)
{
    ParFile::Config config{valid_json().dump()};

    EXPECT_EQ("foo.par", config.source().file);
    ASSERT_EQ(1U, config.parameter_catalogs().size());
    EXPECT_EQ("viewport-catalog.json", config.parameter_catalogs()[0]);
    EXPECT_EQ("foo", config.source().name);
    EXPECT_EQ("out", config.output().directory);
    EXPECT_EQ("output.par", config.output().par);
    EXPECT_EQ("frame-%04d", config.output().entry);
    EXPECT_EQ("output.bat", config.output().script);
    EXPECT_EQ(1, config.parallel());
    EXPECT_EQ("F6", config.video());
    EXPECT_EQ(60, config.num_frames());
    EXPECT_EQ(0U, config.num_tracks());
}

TEST(TestConfig, optionalParallelValid)
{
    Object json{valid_json()};
    json["parallel"] = 20;

    ParFile::Config config{json.dump()};

    EXPECT_EQ(20, config.parallel());
}

TEST(TestConfig, missingParameterCatalogs)
{
    Object json{valid_json()};
    json.erase("parameter_catalogs");

    expect_invalid(json);
}

TEST(TestConfig, missingSource)
{
    Object json{valid_json()};
    json.erase("source");

    expect_invalid(json);
}

TEST(TestConfig, sourceMissingFile)
{
    Object json{valid_json()};
    json.at("source").erase("file");

    expect_invalid(json);
}

TEST(TestConfig, sourceMissingName)
{
    Object json{valid_json()};
    json.at("source").erase("name");

    expect_invalid(json);
}

TEST(TestConfig, missingOutput)
{
    Object json{valid_json()};
    json.erase("output");

    expect_invalid(json);
}

TEST(TestConfig, outputMissingDirectory)
{
    Object json{valid_json()};
    json.at("output").erase("directory");

    expect_invalid(json);
}

TEST(TestConfig, outputMissingPar)
{
    Object json{valid_json()};
    json.at("output").erase("par");

    expect_invalid(json);
}

TEST(TestConfig, outputMissingEntry)
{
    Object json{valid_json()};
    json.at("output").erase("entry");

    expect_invalid(json);
}

TEST(TestConfig, outputMissingScript)
{
    Object json{valid_json()};
    json.at("output").erase("script");

    expect_invalid(json);
}

TEST(TestConfig, missingVideo)
{
    Object json{valid_json()};
    json.erase("video");

    expect_invalid(json);
}

TEST(TestConfig, missingNumFrames)
{
    Object json{valid_json()};
    json.erase("num_frames");

    expect_invalid(json);
}

TEST(TestConfig, missingTracks)
{
    Object json{valid_json()};
    json.erase("tracks");

    expect_invalid(json);
}

TEST(TestConfig, oneTrackValid)
{
    Object json{valid_json()};
    json["tracks"] = Object::array({Object{
        {"parameter", "center-mag"},
        {"keys", Object::array({
            Object{{"frame", 0}, {"value", "-0.5/0/1"}},
            Object{{"frame", 2}, {"value", "-0.5/0/10"}}
        })}
    }});

    ParFile::Config config{json.dump()};

    ASSERT_EQ(1U, config.tracks().size());
    EXPECT_EQ("center-mag", config.tracks()[0].parameter);
    ASSERT_EQ(2U, config.tracks()[0].keys.size());
    EXPECT_EQ(0, config.tracks()[0].keys[0].frame);
    EXPECT_EQ("-0.5/0/1", config.tracks()[0].keys[0].value);
    EXPECT_EQ(2, config.tracks()[0].keys[1].frame);
    EXPECT_EQ("-0.5/0/10", config.tracks()[0].keys[1].value);
}
