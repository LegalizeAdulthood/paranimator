// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/Script.h>

#include <TestParFile/test.h>

#include <ParFile/Config.h>

#include <gtest/gtest.h>

namespace
{

ParFile::Config config_data()
{
    return {{TestParFile::CORE_CATALOG_JSON},                                 //
        {TestParFile::FROM_PAR, "Mandel_Demo"},                               //
        {TestParFile::TEST_OUTPUT_DIRECTORY, TestParFile::TEST_OUTPUT_PAR,    //
            TestParFile::TEST_OUTPUT_ENTRY, TestParFile::TEST_OUTPUT_SCRIPT}, //
        1,                                                                    //
        TestParFile::TEST_VIDEO_MODE,                                         //
        60,                                                                   //
        {}};                                                                  //
}

} // namespace

TEST(TestScript, construct)
{
    ParFile::Config config{config_data()};

    ParFile::Script script{config};
}

TEST(TestScript, commandForFrame)
{
    ParFile::Config config{config_data()};
    ParFile::Script script{config};

    const std::string commands{script.commands("frame-0001")};

    EXPECT_EQ(std::string{"start/wait id batch=yes librarydirs="} + TestParFile::TEST_OUTPUT_DIRECTORY + " @" +
            TestParFile::TEST_OUTPUT_PAR +
            "/frame-0001\n"
            "if errorlevel 1 exit /b 1\n",
        commands);
}
