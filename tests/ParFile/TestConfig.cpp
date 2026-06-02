// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/Config.h>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

using Object = nlohmann::json;

namespace
{

Object valid_json()
{
    return Object{
        {"from", Object{{"file", "foo.par"}, {"name", "foo"}}},         //
        {"to", Object{{"file", "bar.par"}, {"name", "bar"}}},           //
        {"interpolate", Object::array({"center-mag"})},                 //
        {"output", Object{{"directory", "out"},                         //
                       {"par", "output.par"},                           //
                       {"entry", "frame-%04d"},                         //
                       {"script", "output.bat"}}},                      //
        {"video", "F6"},                                                //
        {"num_frames", 60}                                              //
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

    EXPECT_EQ("foo.par", config.from().file);
    EXPECT_EQ("foo", config.from().name);
    EXPECT_EQ("bar.par", config.to().file);
    EXPECT_EQ("bar", config.to().name);
    ASSERT_EQ(1U, config.interpolate().size());
    EXPECT_EQ("center-mag", config.interpolate()[0]);
    EXPECT_EQ("out", config.output().directory);
    EXPECT_EQ("output.par", config.output().par);
    EXPECT_EQ("frame-%04d", config.output().entry);
    EXPECT_EQ("output.bat", config.output().script);
    EXPECT_EQ(1, config.parallel());
    EXPECT_EQ("F6", config.video());
    EXPECT_EQ(60, config.num_frames());
}

TEST(TestConfig, optionalParallelValid)
{
    Object json{valid_json()};
    json["parallel"] = 20;

    ParFile::Config config{json.dump()};

    EXPECT_EQ(20, config.parallel());
}

TEST(TestConfig, missingFrom)
{
    Object json{valid_json()};
    json.erase("from");

    expect_invalid(json);
}

TEST(TestConfig, fromMissingFile)
{
    Object json{valid_json()};
    json.at("from").erase("file");

    expect_invalid(json);
}

TEST(TestConfig, fromMissingName)
{
    Object json{valid_json()};
    json.at("from").erase("name");

    expect_invalid(json);
}

TEST(TestConfig, missingTo)
{
    Object json{valid_json()};
    json.erase("to");

    expect_invalid(json);
}

TEST(TestConfig, missingInterpolate)
{
    Object json{valid_json()};
    json.erase("interpolate");

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
