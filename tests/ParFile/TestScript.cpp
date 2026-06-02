// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/Script.h>

#include <TestParFile/test.h>

#include <ParFile/Config.h>

#include <gtest/gtest.h>

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

TEST(TestScript, construct)
{
    ParFile::Config config{read_text(TestParFile::CENTER_MAG_CONFIG_JSON)};

    ParFile::Script script{config};
}

TEST(TestScript, commandForFrame)
{
    ParFile::Config config{read_text(TestParFile::CENTER_MAG_CONFIG_JSON)};
    ParFile::Script script{config};

    const std::string commands{script.commands("frame-0001")};

    EXPECT_EQ(std::string{"start/wait id batch=yes librarydirs="} + TestParFile::TEST_OUTPUT_DIRECTORY +
            " @" + TestParFile::TEST_OUTPUT_PAR + "/frame-0001\n"
            "if errorlevel 1 exit /b 1\n",
        commands);
}
