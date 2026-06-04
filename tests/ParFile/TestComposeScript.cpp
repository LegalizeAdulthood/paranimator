// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/ComposeScript.h>

#include <ParFile/Config.h>
#include <ParFile/ScriptDialect.h>

#include <gtest/gtest.h>

#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace
{

ParFile::LayerConfig layer(std::string id)
{
    ParFile::LayerConfig result;
    result.id = std::move(id);
    result.source = {"source.par", result.id};
    return result;
}

ParFile::NumberTrackConfig opacity(double value)
{
    return {{{0, value}, {2, value}}};
}

ParFile::Config config_data()
{
    ParFile::Config result;
    result.output.directory = "output";
    result.output.par = "frames.par";
    result.output.entry = "layer-%s-%04d";
    result.output.script = "frames.bat";
    result.output.frames = "frames/frame%04d.png";
    result.output.layers = "layers/layer-%s-%04d.gif";
    result.output.compose_script = "compose.bat";
    result.video = "F6";
    result.num_frames = 3;
    result.layers = {layer("base"), layer("detail")};
    return result;
}

std::string layer_image(const std::string &file, const std::string &opacity)
{
    return std::string{ParFile::ScriptDialect::OPEN_GROUP} + " \"" + file +
        "\" -alpha set -channel A -evaluate multiply " + opacity + " +channel " +
        std::string{ParFile::ScriptDialect::CLOSE_GROUP};
}

std::string error_check()
{
    return std::string{ParFile::ScriptDialect::ERROR_CHECK};
}

} // namespace

TEST(TestComposeScript, commandsComposeLayersInStackOrder)
{
    ParFile::Config config{config_data()};
    config.layers[1].opacity = opacity(50.0);
    config.output.background = "black";
    const ParFile::ComposeScript script{config};

    EXPECT_EQ("magick " + layer_image("layers/layer-base-0002.gif", "1") + " " +
            layer_image("layers/layer-detail-0002.gif", "0.5") +
            " -compose Over -composite -background \"black\" -alpha remove -alpha off "
            "\"frames/frame0002.png\"\n" +
            error_check(),
        script.commands(1));
}

TEST(TestComposeScript, commandsMapNeutralOperatorsToImagemagickOperators)
{
    const std::vector<std::pair<ParFile::ComposeOperator, std::string>> cases{
        {ParFile::ComposeOperator::CLEAR, "Clear"},
        {ParFile::ComposeOperator::COPY, "Src"},
        {ParFile::ComposeOperator::DESTINATION, "Dst"},
        {ParFile::ComposeOperator::SOURCE_OVER, "Over"},
        {ParFile::ComposeOperator::DESTINATION_OVER, "Dst_Over"},
        {ParFile::ComposeOperator::SOURCE_IN, "Src_In"},
        {ParFile::ComposeOperator::DESTINATION_IN, "Dst_In"},
        {ParFile::ComposeOperator::SOURCE_OUT, "Src_Out"},
        {ParFile::ComposeOperator::DESTINATION_OUT, "Dst_Out"},
        {ParFile::ComposeOperator::SOURCE_ATOP, "Src_Atop"},
        {ParFile::ComposeOperator::DESTINATION_ATOP, "Dst_Atop"},
        {ParFile::ComposeOperator::XOR, "Xor"},
        {ParFile::ComposeOperator::ADD, "Plus"},
        {ParFile::ComposeOperator::SUBTRACT, "Minus_Src"},
        {ParFile::ComposeOperator::MULTIPLY, "Multiply"},
        {ParFile::ComposeOperator::DIVIDE, "Divide_Src"},
        {ParFile::ComposeOperator::MIN, "Min"},
        {ParFile::ComposeOperator::MAX, "Max"},
        {ParFile::ComposeOperator::DIFFERENCE, "Difference"},
        {ParFile::ComposeOperator::AVERAGE, "Average"},
        {ParFile::ComposeOperator::SCREEN, "Screen"},
        {ParFile::ComposeOperator::OVERLAY, "Overlay"},
    };

    for (const auto &[op, imagemagick_name] : cases)
    {
        ParFile::Config config{config_data()};
        config.layers[1].compose = op;
        const ParFile::ComposeScript script{config};

        EXPECT_NE(std::string::npos, script.commands(0).find("-compose " + imagemagick_name + " -composite"))
            << imagemagick_name;
    }
}

TEST(TestComposeScript, hiddenLayerIsSkipped)
{
    ParFile::Config config{config_data()};
    config.layers[1].opacity = opacity(0.0);
    const ParFile::ComposeScript script{config};

    EXPECT_EQ("magick " + layer_image("layers/layer-base-0001.gif", "1") +
            " \"frames/frame0001.png\"\n" + error_check(),
        script.commands(0));
}

TEST(TestComposeScript, writeWhenHiddenKeepsZeroOpacityLayer)
{
    ParFile::Config config{config_data()};
    config.layers[1].opacity = opacity(0.0);
    config.layers[1].write_when_hidden = true;
    const ParFile::ComposeScript script{config};

    EXPECT_EQ("magick " + layer_image("layers/layer-base-0001.gif", "1") + " " +
            layer_image("layers/layer-detail-0001.gif", "0") +
            " -compose Over -composite \"frames/frame0001.png\"\n" + error_check(),
        script.commands(0));
}

TEST(TestComposeScript, prologueRunsFromScriptDirectory)
{
    const ParFile::ComposeScript script{config_data()};

    EXPECT_EQ(std::string{ParFile::ScriptDialect::PROLOGUE}, script.prologue());
    EXPECT_EQ(std::string{ParFile::ScriptDialect::EPILOGUE}, script.epilogue());
}
