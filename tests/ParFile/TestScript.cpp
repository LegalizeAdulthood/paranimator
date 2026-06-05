// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/Script.h>

#include <TestParFile/TestOutputDirectory.h>
#include <TestParFile/test.h>

#include <ParFile/Config.h>
#include <ParFile/ScriptDialect.h>

#include <gtest/gtest.h>

#include <fmt/format.h>

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

std::string script_path(std::filesystem::path path)
{
    path.make_preferred();
    return path.string();
}

std::string make_directory_command(const std::string &directory)
{
    return fmt::format(fmt::runtime(std::string{ParFile::ScriptDialect::MAKE_DIRECTORY_FORMAT}), directory) +
        std::string{ParFile::ScriptDialect::ERROR_CHECK};
}

std::string move_command(const std::string &source, const std::string &destination)
{
    return fmt::format(fmt::runtime(std::string{ParFile::ScriptDialect::MOVE_FORMAT}), source, destination) +
        std::string{ParFile::ScriptDialect::ERROR_CHECK};
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

    EXPECT_EQ(std::string{ParFile::ScriptDialect::RENDER_EXECUTABLE} +
            " batch=yes overwrite=yes savename=frame-0001.gif savedir=. librarydirs=. video=" +
            TestParFile::TEST_VIDEO_MODE + " @" + TestParFile::TEST_OUTPUT_PAR + "/frame-0001\n" +
            std::string{ParFile::ScriptDialect::ERROR_CHECK},
        commands);
}

TEST(TestScript, commandForLayerMovesSavedImage)
{
    ParFile::Config config{config_data()};
    config.output.layers = "layers/layer-%s-%04d.gif";
    ParFile::Script script{config};

    const std::string commands{script.layer_commands("layer-base-0001", "base", 0)};

    EXPECT_EQ(std::string{ParFile::ScriptDialect::RENDER_EXECUTABLE} +
            " batch=yes overwrite=yes savename=layer-base-0001.gif savedir=. librarydirs=. video=" +
            TestParFile::TEST_VIDEO_MODE + " @" + TestParFile::TEST_OUTPUT_PAR + "/layer-base-0001\n" +
            std::string{ParFile::ScriptDialect::ERROR_CHECK} +
            move_command(script_path(std::filesystem::path{"image"} / "layer-base-0001.gif"),
                script_path("layers/layer-base-0001.gif")),
        commands);
}

TEST(TestScript, prologueRunsFromScriptDirectory)
{
    ParFile::Config config{config_data()};
    ParFile::Script script{config};

    EXPECT_EQ(std::string{ParFile::ScriptDialect::PROLOGUE}, script.prologue());
    EXPECT_EQ(std::string{ParFile::ScriptDialect::EPILOGUE}, script.epilogue());
}

TEST(TestScript, prologueCreatesLayerDirectory)
{
    ParFile::Config config{config_data()};
    config.output.layers = "layers/layer-%s-%04d.gif";
    ParFile::Script script{config};

    EXPECT_EQ(std::string{ParFile::ScriptDialect::PROLOGUE} + make_directory_command("layers"), script.prologue());
}
