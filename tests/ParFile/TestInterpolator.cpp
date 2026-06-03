// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/Interpolator.h>

#include <TestParFile/test.h>

#include <ParFile/ColorMap.h>
#include <ParFile/Config.h>

#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

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
        {{"center-mag", {{0, "-0.5/0/1"}, {59, "-0.5/0/10"}}}}};              //
}

struct TestInterpolator : testing::Test
{
    ~TestInterpolator() override = default;

protected:
    void SetUp() override
    {
        m_config_data = config_data();
        m_config = m_config_data;
        m_lerper = ParFile::Interpolator{m_config};
    }

    void add_expected_params(ParFile::ParSet &frame, const std::string &save_name);

    ParFile::Config m_config_data;
    ParFile::Config m_config{};
    ParFile::Interpolator m_lerper{};
};

void TestInterpolator::add_expected_params(ParFile::ParSet &expected, const std::string &save_name)
{
    expected.params.push_back({"batch", "yes"});
    expected.params.push_back({"savename", save_name});
    expected.params.push_back({"overwrite", "yes"});
    expected.params.push_back({"video", TestParFile::TEST_VIDEO_MODE});
}

void set_param(ParFile::ParSet &par_set, const std::string &name, const std::string &value)
{
    const auto it{std::find_if(par_set.params.begin(), par_set.params.end(),
        [&](const ParFile::Parameter &param) { return param.name == name; })};
    ASSERT_NE(par_set.params.end(), it);
    it->value = value;
}

ParFile::ColorMap solid_color(int red, int green, int blue)
{
    ParFile::ColorMap result;
    for (ParFile::RgbColor &color : result)
    {
        color = {red, green, blue};
    }
    return result;
}

void write_map_file(const std::filesystem::path &path, const ParFile::ColorMap &map)
{
    std::filesystem::create_directories(path.parent_path());
    std::ofstream out{path.string().c_str()};
    ParFile::write_color_map(out, map);
}

ParFile::ColorMap read_map_file(const std::filesystem::path &path)
{
    std::ifstream in{path.string().c_str()};
    return ParFile::read_color_map(in);
}

} // namespace

TEST_F(TestInterpolator, firstFrameCopiesSource)
{
    ParFile::ParSet expected{m_lerper.source()};
    expected.params[2].value = "-0.5/0/1";
    expected.name = "frame-0001";
    add_expected_params(expected, expected.name + ".gif");
    ParFile::ParSet frame{m_lerper()};

    ASSERT_EQ(expected, frame);
}

TEST_F(TestInterpolator, lastFrameIsTrackEndValue)
{
    m_config_data.num_frames = 2;
    m_config_data.tracks[0].keys[1].frame = 1;
    m_config = m_config_data;
    m_lerper = ParFile::Interpolator{m_config};
    ParFile::ParSet expected{m_lerper.source()};
    expected.params[2].value = "-0.5/0/10";
    expected.name = "frame-0002";
    add_expected_params(expected, expected.name + ".gif");
    ParFile::ParSet frame{m_lerper()};

    frame = m_lerper();

    ASSERT_EQ(expected, frame);
}

TEST_F(TestInterpolator, inbetweenFramesAreInterpolated)
{
    m_config_data.num_frames = 3;
    m_config_data.tracks[0].keys[1].frame = 2;
    m_config = m_config_data;
    m_lerper = ParFile::Interpolator{m_config};
    ParFile::ParSet expected{m_lerper.source()};
    expected.params[2].value = "-0.5/0/3.16228";
    expected.name = "frame-0002";
    add_expected_params(expected, expected.name + ".gif");
    ParFile::ParSet frame{m_lerper()};

    frame = m_lerper();

    ASSERT_EQ(expected, frame);
}

TEST_F(TestInterpolator, paramsTrackWritesOneParamsAssignment)
{
    m_config_data.source.name = "Julia_Demo";
    m_config_data.num_frames = 3;
    m_config_data.tracks = {{"params.c", {{0, "0/1"}, {2, "2/3"}}}};
    m_config = m_config_data;
    m_lerper = ParFile::Interpolator{m_config};
    ParFile::ParSet expected{m_lerper.source()};
    set_param(expected, "params", "1/2");
    expected.name = "frame-0002";
    add_expected_params(expected, expected.name + ".gif");
    ParFile::ParSet frame{m_lerper()};

    frame = m_lerper();

    ASSERT_EQ(expected, frame);
}

TEST_F(TestInterpolator, formulaParamsTracksMergeOneParamsAssignment)
{
    m_config_data.source.name = "Formula_Demo";
    m_config_data.num_frames = 3;
    m_config_data.tracks = {
        {"MandelbrotMix4.bailout", {{0, "10"}, {2, "20"}}}, {"MandelbrotMix4.c", {{0, "-1/-2"}, {2, "-3/-4"}}}};
    m_config = m_config_data;
    m_lerper = ParFile::Interpolator{m_config};
    ParFile::ParSet expected{m_lerper.source()};
    set_param(expected, "params", "15/3/-2/-3/0/0");
    expected.name = "frame-0002";
    add_expected_params(expected, expected.name + ".gif");
    ParFile::ParSet frame{m_lerper()};

    frame = m_lerper();

    ASSERT_EQ(expected, frame);
}

TEST_F(TestInterpolator, formulaFunctionTrackWritesOneFunctionAssignment)
{
    m_config_data.source.name = "Formula_Demo";
    m_config_data.num_frames = 3;
    m_config_data.tracks = {{"MandelbrotMix4.fn2", {{0, "tan"}, {2, "log"}}}};
    m_config = m_config_data;
    m_lerper = ParFile::Interpolator{m_config};
    ParFile::ParSet expected{m_lerper.source()};
    set_param(expected, "function", "sin/tan");
    expected.name = "frame-0002";
    add_expected_params(expected, expected.name + ".gif");
    ParFile::ParSet frame{m_lerper()};

    frame = m_lerper();

    ASSERT_EQ(expected, frame);
}

TEST_F(TestInterpolator, multipleTracksHaveIndependentKeys)
{
    m_config_data.num_frames = 3;
    m_config_data.tracks[0].keys[1].frame = 2;
    m_config_data.tracks.push_back({"maxiter", {{0, "100"}, {1, "200"}}});
    m_config = m_config_data;
    m_lerper = ParFile::Interpolator{m_config};
    ParFile::ParSet expected{m_lerper.source()};
    set_param(expected, "center-mag", "-0.5/0/3.16228");
    set_param(expected, "maxiter", "200");
    expected.name = "frame-0002";
    add_expected_params(expected, expected.name + ".gif");
    ParFile::ParSet frame{m_lerper()};

    frame = m_lerper();

    ASSERT_EQ(expected, frame);
}

TEST_F(TestInterpolator, colorMapTrackWritesGeneratedMapsAndAtFileValues)
{
    const std::filesystem::path root{std::filesystem::path{TestParFile::TEST_OUTPUT_DIRECTORY} / "color-map-track"};
    const std::filesystem::path output{root / "output"};
    const std::filesystem::path warm_map{root / "input" / "warm.map"};
    const std::filesystem::path cool_map{root / "input" / "cool.map"};
    std::filesystem::remove_all(root);
    write_map_file(warm_map, solid_color(1, 2, 3));
    write_map_file(cool_map, solid_color(4, 5, 6));
    m_config_data.output.directory = output.string();
    m_config_data.num_frames = 3;
    m_config_data.tracks = {{"colors", {{0, warm_map.string()}, {2, cool_map.string()}}, ParFile::TrackMode::KEYFRAMES,
        {}, ParFile::TrackKind::COLOR_MAP, ParFile::ColorMapConfig{ParFile::TrackFormat::AT_FILE, "colors-%04d.map"}}};
    m_config = m_config_data;
    m_lerper = ParFile::Interpolator{m_config};

    const ParFile::ParSet first_frame{m_lerper()};
    const ParFile::ParSet middle_frame{m_lerper()};
    const ParFile::ParSet last_frame{m_lerper()};

    const std::filesystem::path first_map{output / "map" / "colors-0001.map"};
    const std::filesystem::path middle_map{output / "map" / "colors-0002.map"};
    const std::filesystem::path last_map{output / "map" / "colors-0003.map"};
    EXPECT_TRUE(std::filesystem::exists(first_map));
    EXPECT_TRUE(std::filesystem::exists(middle_map));
    EXPECT_TRUE(std::filesystem::exists(last_map));
    const ParFile::ColorMap first{read_map_file(first_map)};
    const ParFile::ColorMap middle{read_map_file(middle_map)};
    const ParFile::ColorMap last{read_map_file(last_map)};
    EXPECT_EQ(1, first[0].red);
    EXPECT_EQ(2, first[0].green);
    EXPECT_EQ(3, first[0].blue);
    EXPECT_EQ(3, middle[0].red);
    EXPECT_EQ(4, middle[0].green);
    EXPECT_EQ(5, middle[0].blue);
    EXPECT_EQ(4, last[0].red);
    EXPECT_EQ(5, last[0].green);
    EXPECT_EQ(6, last[0].blue);
    const auto colors_value = [](const ParFile::ParSet &frame)
    {
        const auto it{std::find_if(frame.params.begin(), frame.params.end(),
            [](const ParFile::Parameter &param) { return param.name == "colors"; })};
        return it == frame.params.end() ? std::string{} : it->value;
    };
    EXPECT_EQ("@colors-0001.map", colors_value(first_frame));
    EXPECT_EQ("@colors-0002.map", colors_value(middle_frame));
    EXPECT_EQ("@colors-0003.map", colors_value(last_frame));
}

TEST_F(TestInterpolator, unknownAnimatedParameterRejected)
{
    m_config_data.tracks[0].parameter = "unknown";
    m_config = m_config_data;

    EXPECT_THROW(ParFile::Interpolator{m_config}, std::runtime_error);
}
