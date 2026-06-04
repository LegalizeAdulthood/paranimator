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

#ifdef _WIN32
    EXPECT_EQ(std::string{"start/wait id batch=yes overwrite=yes savename=frame-0001.gif savedir=. librarydirs=. "
                          "video="} +
            TestParFile::TEST_VIDEO_MODE + " @" + TestParFile::TEST_OUTPUT_PAR +
            "/frame-0001\n"
            "if errorlevel 1 exit /b 1\n",
        commands);
#else
    EXPECT_EQ(std::string{"id batch=yes overwrite=yes savename=frame-0001.gif savedir=. librarydirs=. video="} +
            TestParFile::TEST_VIDEO_MODE + " @" + TestParFile::TEST_OUTPUT_PAR + "/frame-0001\n",
        commands);
#endif
}

TEST(TestScript, commandForLayerMovesSavedImage)
{
    ParFile::Config config{config_data()};
    config.output.layers = "layers/layer-%s-%04d.gif";
    ParFile::Script script{config};

    const std::string commands{script.layer_commands("layer-base-0001", "base", 0)};

#ifdef _WIN32
    EXPECT_EQ(std::string{"start/wait id batch=yes overwrite=yes savename=layer-base-0001.gif savedir=. "
                          "librarydirs=. video="} +
            TestParFile::TEST_VIDEO_MODE + " @" + TestParFile::TEST_OUTPUT_PAR +
            "/layer-base-0001\n"
            "if errorlevel 1 exit /b 1\n"
            "move /y \"image\\layer-base-0001.gif\" \"layers\\layer-base-0001.gif\"\n"
            "if errorlevel 1 exit /b 1\n",
        commands);
#else
    EXPECT_EQ(std::string{"id batch=yes overwrite=yes savename=layer-base-0001.gif savedir=. librarydirs=. video="} +
            TestParFile::TEST_VIDEO_MODE + " @" + TestParFile::TEST_OUTPUT_PAR +
            "/layer-base-0001\n"
            "mv -f \"image/layer-base-0001.gif\" \"layers/layer-base-0001.gif\"\n",
        commands);
#endif
}

TEST(TestScript, prologueRunsFromScriptDirectory)
{
    ParFile::Config config{config_data()};
    ParFile::Script script{config};

#ifdef _WIN32
    EXPECT_EQ("@echo off\n"
              "pushd \"%~dp0\"\n"
              "if errorlevel 1 exit /b 1\n",
        script.prologue());
    EXPECT_EQ("popd\n", script.epilogue());
#else
    EXPECT_EQ("#!/usr/bin/env bash\n"
              "set -e\n"
              "pushd \"$(dirname \"$0\")\" >/dev/null\n",
        script.prologue());
    EXPECT_EQ("popd >/dev/null\n", script.epilogue());
#endif
}

TEST(TestScript, prologueCreatesLayerDirectory)
{
    ParFile::Config config{config_data()};
    config.output.layers = "layers/layer-%s-%04d.gif";
    ParFile::Script script{config};

#ifdef _WIN32
    EXPECT_EQ("@echo off\n"
              "pushd \"%~dp0\"\n"
              "if errorlevel 1 exit /b 1\n"
              "if not exist \"layers\" mkdir \"layers\"\n"
              "if errorlevel 1 exit /b 1\n",
        script.prologue());
#else
    EXPECT_EQ("#!/usr/bin/env bash\n"
              "set -e\n"
              "pushd \"$(dirname \"$0\")\" >/dev/null\n"
              "mkdir -p \"layers\"\n",
        script.prologue());
#endif
}
