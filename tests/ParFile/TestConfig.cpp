// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/Config.h>

#include <gtest/gtest.h>

using Object = boost::json::object;

TEST(TestConfig, valid)
{
    const Object json{
        {"from", Object{{"file", "foo.par"}, {"name", "foo"}}}, //
        {"to", Object{{"file", "bar.par"}, {"name", "bar"}}},   //
        {"output", "output.par"},                               //
        {"script", "output.bat"},                               //
        {"frame", "frame-%04d"},                                //
        {"num_frames", 60}                                      //
    };

    ParFile::Config config{json};

    EXPECT_EQ("foo.par", config.from().file);
    EXPECT_EQ("foo", config.from().name);
    EXPECT_EQ("bar.par", config.to().file);
    EXPECT_EQ("bar", config.to().name);
    EXPECT_EQ("output.par", config.output());
    EXPECT_EQ("output.bat", config.script());
    EXPECT_EQ("frame-%04d", config.frame());
    EXPECT_EQ(60, config.num_frames());
}

TEST(TestConfig, missingFrom)
{
    const Object json{
        {"to", Object{{"file", "bar.par"}, {"name", "bar"}}}, //
        {"output", "output.par"},                             //
        {"script", "output.bat"},                             //
        {"frame", "frame-%04d"},                              //
        {"num_frames", 60}                                    //
    };

    EXPECT_THROW(ParFile::Config{json}, std::runtime_error);
}

TEST(TestConfig, fromMissingFile)
{
    const Object json{
        {"from", Object{{"name", "foo"}}},                    //
        {"to", Object{{"file", "bar.par"}, {"name", "bar"}}}, //
        {"output", "output.par"},                             //
        {"script", "output.bat"},                             //
        {"frame", "frame-%04d"},                              //
        {"num_frames", 60}                                    //
    };

    EXPECT_THROW(ParFile::Config{json}, std::runtime_error);
}

TEST(TestConfig, fromMissingName)
{
    const Object json{
        {"from", Object{{"file", "foo.par"}}},                //
        {"to", Object{{"file", "bar.par"}, {"name", "bar"}}}, //
        {"output", "output.par"},                             //
        {"script", "output.bat"},                             //
        {"frame", "frame-%04d"},                              //
        {"num_frames", 60}                                    //
    };

    EXPECT_THROW(ParFile::Config{json}, std::runtime_error);
}

TEST(TestConfig, missingTo)
{
    const Object json{
        {"from", Object{{"file", "foo.par"}, {"name", "foo"}}}, //
        {"output", "output.par"},                               //
        {"script", "output.bat"},                               //
        {"frame", "frame-%04d"},                                //
        {"num_frames", 60}                                      //
    };

    EXPECT_THROW(ParFile::Config{json}, std::runtime_error);
}

TEST(TestConfig, missingOutput)
{
    const Object json{
        {"from", Object{{"file", "foo.par"}, {"name", "foo"}}}, //
        {"to", Object{{"file", "bar.par"}, {"name", "bar"}}},   //
        {"script", "output.bat"},                               //
        {"frame", "frame-%04d"},                                //
        {"num_frames", 60}                                      //
    };

    EXPECT_THROW(ParFile::Config{json}, std::runtime_error);
}

TEST(TestConfig, missingScript)
{
    const Object json{
        {"from", Object{{"file", "foo.par"}, {"name", "foo"}}}, //
        {"to", Object{{"file", "bar.par"}, {"name", "bar"}}},   //
        {"output", "output.par"},                               //
        {"frame", "frame-%04d"},                                //
        {"num_frames", 60}                                      //
    };

    EXPECT_THROW(ParFile::Config{json}, std::runtime_error);
}

TEST(TestConfig, missingFrame)
{
    const Object json{
        {"from", Object{{"file", "foo.par"}, {"name", "foo"}}}, //
        {"to", Object{{"file", "bar.par"}, {"name", "bar"}}},   //
        {"output", "output.par"},                               //
        {"script", "output.bat"},                               //
        {"num_frames", 60}                                      //
    };

    EXPECT_THROW(ParFile::Config{json}, std::runtime_error);
}

TEST(TestConfig, missingNumFrames)
{
    const Object json{
        {"from", Object{{"file", "foo.par"}, {"name", "foo"}}}, //
        {"to", Object{{"file", "bar.par"}, {"name", "bar"}}},   //
        {"output", "output.par"},                               //
        {"script", "output.bat"},                               //
        {"frame", "frame-%04d"}                                 //
    };

    EXPECT_THROW(ParFile::Config{json}, std::runtime_error);
}
