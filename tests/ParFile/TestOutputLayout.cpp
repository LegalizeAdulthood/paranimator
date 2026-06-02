// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/OutputLayout.h>

#include <TestParFile/test.h>

#include <ParFile/Config.h>
#include <ParFile/Json.h>

#include <gtest/gtest.h>

#include <filesystem>

TEST(TestOutputLayout, createLibraryDirectories)
{
    boost::json::object json{ParFile::read_json(TestParFile::CENTER_MAG_CONFIG_JSON).as_object()};
    ParFile::Config config{json};
    ParFile::OutputLayout layout{config};
    std::filesystem::remove_all(layout.directory());

    layout.create_directories();

    EXPECT_TRUE(std::filesystem::is_directory(layout.par_directory()));
    EXPECT_TRUE(std::filesystem::is_directory(layout.map_directory()));
}

TEST(TestOutputLayout, parFileIsUnderParDirectory)
{
    boost::json::object json{ParFile::read_json(TestParFile::CENTER_MAG_CONFIG_JSON).as_object()};
    ParFile::Config config{json};
    ParFile::OutputLayout layout{config};

    EXPECT_EQ(layout.par_directory() / TestParFile::TEST_OUTPUT_PAR, layout.par_file());
}

TEST(TestOutputLayout, parallelScriptFileIsIndexed)
{
    boost::json::object json{ParFile::read_json(TestParFile::CENTER_MAG_CONFIG_JSON).as_object()};
    ParFile::Config config{json};
    ParFile::OutputLayout layout{config};

    EXPECT_EQ(layout.directory() / "frames-1.bat", layout.script_file(1));
}
