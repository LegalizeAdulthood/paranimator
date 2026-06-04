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

    EXPECT_EQ(std::string{"start/wait id batch=yes overwrite=yes savename=frame-0001.gif savedir=. librarydirs=. "
                          "video="} +
            TestParFile::TEST_VIDEO_MODE + " @" + TestParFile::TEST_OUTPUT_PAR +
            "/frame-0001\n"
            "if errorlevel 1 exit /b 1\n",
        commands);
}

TEST(TestScript, commandForLayerMovesSavedImage)
{
    ParFile::Config config{config_data()};
    config.output.layers = "layers/layer-%s-%04d.png";
    ParFile::Script script{config};

    const std::string commands{script.layer_commands("layer-base-0001", "base", 0)};

    EXPECT_EQ(std::string{"start/wait id batch=yes overwrite=yes savename=layer-base-0001.png savedir=. "
                          "librarydirs=. video="} +
            TestParFile::TEST_VIDEO_MODE + " @" + TestParFile::TEST_OUTPUT_PAR +
            "/layer-base-0001\n"
            "if errorlevel 1 exit /b 1\n"
            "move /y \"image\\layer-base-0001.png\" \"layers\\layer-base-0001.png\"\n"
            "if errorlevel 1 exit /b 1\n",
        commands);
}

TEST(TestScript, prologueRunsFromScriptDirectory)
{
    ParFile::Config config{config_data()};
    ParFile::Script script{config};

    EXPECT_EQ("@echo off\n"
              "pushd \"%~dp0\"\n"
              "if errorlevel 1 exit /b 1\n",
        script.prologue());
    EXPECT_EQ("popd\n", script.epilogue());
}

TEST(TestScript, prologueCreatesLayerDirectory)
{
    ParFile::Config config{config_data()};
    config.output.layers = "layers/layer-%s-%04d.png";
    ParFile::Script script{config};

    EXPECT_EQ("@echo off\n"
              "pushd \"%~dp0\"\n"
              "if errorlevel 1 exit /b 1\n"
              "if not exist \"layers\" mkdir \"layers\"\n"
              "if errorlevel 1 exit /b 1\n",
        script.prologue());
}
