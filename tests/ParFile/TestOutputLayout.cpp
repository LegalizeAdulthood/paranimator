// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/OutputLayout.h>

#include <TestParFile/TestOutputDirectory.h>
#include <TestParFile/test.h>

#include <ParFile/Config.h>

#include <gtest/gtest.h>

#include <filesystem>

namespace
{

ParFile::Config config_data()
{
    return {{TestParFile::CORE_CATALOG_JSON},                                         //
        {TestParFile::FROM_PAR, "Mandel_Demo"},                                       //
        {TestParFile::test_output_directory().string(), TestParFile::TEST_OUTPUT_PAR, //
            TestParFile::TEST_OUTPUT_ENTRY, TestParFile::TEST_OUTPUT_SCRIPT},         //
        1,                                                                            //
        TestParFile::TEST_VIDEO_MODE,                                                 //
        60,                                                                           //
        {}};                                                                          //
}

} // namespace

TEST(TestOutputLayout, createLibraryDirectories)
{
    ParFile::Config config{config_data()};
    ParFile::OutputLayout layout{config};
    std::filesystem::remove_all(layout.directory());

    layout.create_directories();

    EXPECT_TRUE(std::filesystem::is_directory(layout.par_directory()));
    EXPECT_TRUE(std::filesystem::is_directory(layout.map_directory()));
}

TEST(TestOutputLayout, parFileIsUnderParDirectory)
{
    ParFile::Config config{config_data()};
    ParFile::OutputLayout layout{config};

    EXPECT_EQ(layout.par_directory() / TestParFile::TEST_OUTPUT_PAR, layout.par_file());
}

TEST(TestOutputLayout, parallelScriptFileIsIndexed)
{
    ParFile::Config config{config_data()};
    ParFile::OutputLayout layout{config};

    EXPECT_EQ(layout.directory() / "frames-1.bat", layout.script_file(1));
}

TEST(TestOutputLayout, createLayerAndFrameDirectories)
{
    ParFile::Config config{config_data()};
    config.output.frames = "frames/frame%04d.png";
    config.output.layers = "layers/layer-%s-%04d.gif";
    ParFile::OutputLayout layout{config};
    std::filesystem::remove_all(layout.directory());

    layout.create_directories();

    EXPECT_TRUE(std::filesystem::is_directory(layout.directory() / "frames"));
    EXPECT_TRUE(std::filesystem::is_directory(layout.directory() / "layers"));
}

TEST(TestOutputLayout, composeScriptFileIsUnderOutputDirectory)
{
    ParFile::Config config{config_data()};
    config.output.compose_script = "compose.bat";
    ParFile::OutputLayout layout{config};

    EXPECT_EQ(layout.directory() / "compose.bat", layout.compose_script_file());
}
