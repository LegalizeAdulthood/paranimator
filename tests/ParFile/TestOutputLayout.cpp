// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/OutputLayout.h>

#include <TestParFile/test.h>

#include <ParFile/Config.h>

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <iterator>

namespace
{

std::string read_text(const char *path)
{
    std::ifstream in{path};
    return {std::istreambuf_iterator<char>{in}, std::istreambuf_iterator<char>{}};
}

} // namespace

TEST(TestOutputLayout, createLibraryDirectories)
{
    ParFile::Config config{read_text(TestParFile::CENTER_MAG_CONFIG_JSON)};
    ParFile::OutputLayout layout{config};
    std::filesystem::remove_all(layout.directory());

    layout.create_directories();

    EXPECT_TRUE(std::filesystem::is_directory(layout.par_directory()));
    EXPECT_TRUE(std::filesystem::is_directory(layout.map_directory()));
}

TEST(TestOutputLayout, parFileIsUnderParDirectory)
{
    ParFile::Config config{read_text(TestParFile::CENTER_MAG_CONFIG_JSON)};
    ParFile::OutputLayout layout{config};

    EXPECT_EQ(layout.par_directory() / TestParFile::TEST_OUTPUT_PAR, layout.par_file());
}

TEST(TestOutputLayout, parallelScriptFileIsIndexed)
{
    ParFile::Config config{read_text(TestParFile::CENTER_MAG_CONFIG_JSON)};
    ParFile::OutputLayout layout{config};

    EXPECT_EQ(layout.directory() / "frames-1.bat", layout.script_file(1));
}
