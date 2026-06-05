// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/Interpolator.h>

#include <TestParFile/test.h>

#include <ParFile/ColorMap.h>
#include <ParFile/Config.h>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace
{

using Object = nlohmann::json;

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

ParFile::Config parsed_config(std::string_view source_name, int num_frames, const Object &track,
    std::vector<std::string> parameter_catalogs = {TestParFile::CORE_CATALOG_JSON})
{
    Object catalogs = Object::array();
    for (const std::string &catalog : parameter_catalogs)
    {
        catalogs.push_back(catalog);
    }

    const Object json{{"parameter-catalogs", catalogs},
        {"source", Object{{"file", TestParFile::FROM_PAR}, {"name", std::string{source_name}}}},
        {"output",
            Object{{"directory", TestParFile::TEST_OUTPUT_DIRECTORY}, {"par", TestParFile::TEST_OUTPUT_PAR},
                {"entry", TestParFile::TEST_OUTPUT_ENTRY}, {"script", TestParFile::TEST_OUTPUT_SCRIPT}}},
        {"video", TestParFile::TEST_VIDEO_MODE}, {"num-frames", num_frames}, {"tracks", Object::array({track})}};
    return ParFile::read_config(json.dump());
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

    ParFile::Config m_config_data;
    ParFile::Config m_config{};
    ParFile::Interpolator m_lerper{};
};

void set_param(ParFile::ParSet &par_set, const std::string &name, const std::string &value)
{
    const auto it{std::find_if(par_set.params.begin(), par_set.params.end(),
        [&](const ParFile::Parameter &param) { return param.name == name; })};
    ASSERT_NE(par_set.params.end(), it);
    it->value = value;
}

std::string parameter_value(const ParFile::ParSet &par_set, std::string_view name)
{
    const std::string key{name};
    const auto it{std::find_if(par_set.params.begin(), par_set.params.end(),
        [&](const ParFile::Parameter &param) { return param.name == key; })};
    return it == par_set.params.end() ? std::string{} : it->value;
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

ParFile::ColorMap indexed_color()
{
    ParFile::ColorMap result;
    for (std::size_t i = 0; i < result.size(); ++i)
    {
        result[i] = {static_cast<int>(i), 255 - static_cast<int>(i), static_cast<int>(i % 64U)};
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
    ParFile::ParSet frame{m_lerper()};

    ASSERT_EQ(expected, frame);
}

TEST_F(TestInterpolator, multipleCatalogFilesLoad)
{
    m_config_data.parameter_catalogs = {TestParFile::CORE_CATALOG_JSON, TestParFile::COLORING_CATALOG_JSON};
    m_lerper = ParFile::Interpolator{m_config_data};

    EXPECT_EQ("frame-0001", m_lerper().name);
}

TEST_F(TestInterpolator, missingCatalogFileFailsClearly)
{
    m_config_data.parameter_catalogs = {"missing-catalog.json"};

    try
    {
        m_lerper = ParFile::Interpolator{m_config_data};
        FAIL() << "Expected missing catalog to throw";
    }
    catch (const std::runtime_error &bang)
    {
        EXPECT_NE(std::string::npos, std::string{bang.what()}.find("missing-catalog.json"));
    }
}

TEST_F(TestInterpolator, id3DViewWritesRotationPerspectiveAndXyshift)
{
    const Object track{{"name", "view"}, {"type", "id-3d-view"},
        {"outputs", Object{{"rotation", "rotation"}, {"perspective", "perspective"}, {"xyshift", "xyshift"}}},
        {"rotation",
            Object{{"type", "numeric-tuple"}, {"arity", 3},
                {"keys",
                    Object::array({Object{{"frame", 0}, {"value", Object::array({60, 30, 0})}},
                        Object{{"frame", 2}, {"value", Object::array({70, 50, 10})}}})}}},
        {"perspective",
            Object{{"type", "integer"},
                {"keys", Object::array({Object{{"frame", 0}, {"value", 0}}, Object{{"frame", 2}, {"value", 100}}})}}},
        {"xyshift",
            Object{{"type", "numeric-tuple"}, {"arity", 2},
                {"keys",
                    Object::array({Object{{"frame", 0}, {"value", Object::array({0, 0})}},
                        Object{{"frame", 2}, {"value", Object::array({20, -10})}}})}}}};
    const ParFile::Config config{
        parsed_config("Mandel_Demo", 3, track, {TestParFile::CORE_CATALOG_JSON, TestParFile::ID_3D_CATALOG_JSON})};
    ParFile::Interpolator lerper{config};

    const ParFile::ParSet first{lerper()};
    EXPECT_EQ("60/30/0", parameter_value(first, "rotation"));
    EXPECT_EQ("0", parameter_value(first, "perspective"));
    EXPECT_EQ("0/0", parameter_value(first, "xyshift"));

    const ParFile::ParSet second{lerper()};
    EXPECT_EQ("65/40/5", parameter_value(second, "rotation"));
    EXPECT_EQ("50", parameter_value(second, "perspective"));
    EXPECT_EQ("10/-5", parameter_value(second, "xyshift"));
}

TEST_F(TestInterpolator, id3DViewWritesScaleSphereAndStereoControls)
{
    const Object track{{"name", "view"}, {"type", "id-3d-view"},
        {"outputs",
            Object{{"scalexyz", "scalexyz"}, {"sphere", "sphere"}, {"stereo", "stereo"}, {"interocular", "interocular"},
                {"converge", "converge"}}},
        {"scalexyz",
            Object{{"type", "numeric-tuple"}, {"arity", 3},
                {"keys",
                    Object::array({Object{{"frame", 0}, {"value", Object::array({90, 90, 30})}},
                        Object{{"frame", 2}, {"value", Object::array({100, 100, 40})}}})}}},
        {"sphere",
            Object{{"type", "yes-no"},
                {"keys",
                    Object::array({Object{{"frame", 0}, {"value", false}}, Object{{"frame", 2}, {"value", true}}})}}},
        {"stereo",
            Object{{"type", "integer"},
                {"keys", Object::array({Object{{"frame", 0}, {"value", 0}}, Object{{"frame", 2}, {"value", 2}}})}}},
        {"interocular",
            Object{{"type", "integer"},
                {"keys", Object::array({Object{{"frame", 0}, {"value", 0}}, Object{{"frame", 2}, {"value", 8}}})}}},
        {"converge",
            Object{{"type", "integer"},
                {"keys", Object::array({Object{{"frame", 0}, {"value", 0}}, Object{{"frame", 2}, {"value", -2}}})}}}};
    const ParFile::Config config{
        parsed_config("Mandel_Demo", 3, track, {TestParFile::CORE_CATALOG_JSON, TestParFile::ID_3D_CATALOG_JSON})};
    ParFile::Interpolator lerper{config};

    static_cast<void>(lerper());
    const ParFile::ParSet second{lerper()};
    EXPECT_EQ("95/95/35", parameter_value(second, "scalexyz"));
    EXPECT_EQ("no", parameter_value(second, "sphere"));
    EXPECT_EQ("1", parameter_value(second, "stereo"));
    EXPECT_EQ("4", parameter_value(second, "interocular"));
    EXPECT_EQ("-1", parameter_value(second, "converge"));
}

TEST_F(TestInterpolator, id3DViewRejectsIllegalStereoValue)
{
    const Object track{{"name", "view"}, {"type", "id-3d-view"}, {"outputs", Object{{"stereo", "stereo"}}},
        {"stereo",
            Object{{"type", "integer"},
                {"keys", Object::array({Object{{"frame", 0}, {"value", 0}}, Object{{"frame", 2}, {"value", 5}}})}}}};
    const ParFile::Config config{
        parsed_config("Mandel_Demo", 3, track, {TestParFile::CORE_CATALOG_JSON, TestParFile::ID_3D_CATALOG_JSON})};

    EXPECT_THROW(ParFile::Interpolator{config}, std::runtime_error);
}

TEST_F(TestInterpolator, julibrotViewWritesModeGeometryEyesAndFromTo)
{
    const Object track{{"name", "view"}, {"type", "julibrot-view"},
        {"outputs",
            Object{{"mode", "3dmode"}, {"geometry", "julibrot3d"}, {"eyes", "julibroteyes"},
                {"from-to", "julibrotfromto"}}},
        {"mode",
            Object{{"type", "enum"},
                {"keys",
                    Object::array(
                        {Object{{"frame", 0}, {"value", "monocular"}}, Object{{"frame", 2}, {"value", "lefteye"}}})}}},
        {"geometry",
            Object{{"type", "numeric-tuple"}, {"arity", 6},
                {"keys",
                    Object::array({Object{{"frame", 0}, {"value", Object::array({128, 8, 8, 7, 10, 24})}},
                        Object{{"frame", 2}, {"value", Object::array({160, 7, 6, 6, 9, 20})}}})}}},
        {"eyes",
            Object{{"type", "double"},
                {"keys", Object::array({Object{{"frame", 0}, {"value", 2.5}}, Object{{"frame", 2}, {"value", 1.0}}})}}},
        {"from-to",
            Object{{"type", "numeric-tuple"}, {"arity", 4},
                {"keys",
                    Object::array({Object{{"frame", 0}, {"value", Object::array({-0.83, -0.83, 0.25, -0.25})}},
                        Object{{"frame", 2}, {"value", Object::array({-0.7, -0.9, 0.2, -0.2})}}})}}}};
    const ParFile::Config config{
        parsed_config("Mandel_Demo", 3, track, {TestParFile::CORE_CATALOG_JSON, TestParFile::ID_3D_CATALOG_JSON})};
    ParFile::Interpolator lerper{config};

    const ParFile::ParSet first{lerper()};
    EXPECT_EQ("monocular", parameter_value(first, "3dmode"));

    const ParFile::ParSet second{lerper()};
    EXPECT_EQ("monocular", parameter_value(second, "3dmode"));
    EXPECT_EQ("144/7.5/7/6.5/9.5/22", parameter_value(second, "julibrot3d"));
    EXPECT_EQ("1.75", parameter_value(second, "julibroteyes"));
    EXPECT_EQ("-0.765/-0.865/0.225/-0.225", parameter_value(second, "julibrotfromto"));
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
    ParFile::ParSet frame{m_lerper()};

    frame = m_lerper();

    ASSERT_EQ(expected, frame);
}

TEST_F(TestInterpolator, constantPathReturnsSameValueEveryFrame)
{
    m_config = parsed_config(
        "Mandel_Demo", 3, Object{{"parameter", "maxiter"}, {"path", Object{{"kind", "constant"}, {"value", "321"}}}});
    m_lerper = ParFile::Interpolator{m_config};

    EXPECT_EQ("321", parameter_value(m_lerper(), "maxiter"));
    EXPECT_EQ("321", parameter_value(m_lerper(), "maxiter"));
    EXPECT_EQ("321", parameter_value(m_lerper(), "maxiter"));
}

TEST_F(TestInterpolator, linePathMatchesEquivalentKeyedLinearTrack)
{
    const Object path_track{
        {"parameter", "maxiter"}, {"path", Object{{"kind", "line"}, {"from", "100"}, {"to", "200"}}}};
    const Object keyed_track{{"parameter", "maxiter"},
        {"keys",
            Object::array({Object{{"frame", 0}, {"value", "100"}},
                Object{{"frame", 2}, {"value", "200"}, {"curve", "linear"}}})}};
    ParFile::Interpolator path_lerper{parsed_config("Mandel_Demo", 3, path_track)};
    ParFile::Interpolator keyed_lerper{parsed_config("Mandel_Demo", 3, keyed_track)};

    EXPECT_EQ(keyed_lerper(), path_lerper());
    EXPECT_EQ(keyed_lerper(), path_lerper());
    EXPECT_EQ(keyed_lerper(), path_lerper());
}

TEST_F(TestInterpolator, complexLinePathPreservesSlashPairFormatting)
{
    m_config = parsed_config("Julia_Demo", 3,
        Object{{"parameter", "params.c"}, {"path", Object{{"kind", "line"}, {"from", "0/1"}, {"to", "2/3"}}}});
    m_lerper = ParFile::Interpolator{m_config};

    EXPECT_EQ("0/1", parameter_value(m_lerper(), "params"));
    EXPECT_EQ("1/2", parameter_value(m_lerper(), "params"));
    EXPECT_EQ("2/3", parameter_value(m_lerper(), "params"));
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
    ParFile::ParSet frame{m_lerper()};

    frame = m_lerper();

    ASSERT_EQ(expected, frame);
}

TEST_F(TestInterpolator, formulaParamsTracksMergeOneParamsAssignment)
{
    m_config_data.source.name = "Formula_Demo";
    m_config_data.parameter_catalogs = {TestParFile::CORE_CATALOG_JSON, TestParFile::FORMULA_CATALOG_JSON};
    m_config_data.num_frames = 3;
    m_config_data.tracks = {
        {"MandelbrotMix4.bailout", {{0, "10"}, {2, "20"}}}, {"MandelbrotMix4.c", {{0, "-1/-2"}, {2, "-3/-4"}}}};
    m_config = m_config_data;
    m_lerper = ParFile::Interpolator{m_config};
    ParFile::ParSet expected{m_lerper.source()};
    set_param(expected, "params", "15/3/-2/-3/0/0");
    expected.name = "frame-0002";
    ParFile::ParSet frame{m_lerper()};

    frame = m_lerper();

    ASSERT_EQ(expected, frame);
}

TEST_F(TestInterpolator, formulaFunctionTrackWritesOneFunctionAssignment)
{
    m_config_data.source.name = "Formula_Demo";
    m_config_data.parameter_catalogs = {TestParFile::CORE_CATALOG_JSON, TestParFile::FORMULA_CATALOG_JSON};
    m_config_data.num_frames = 3;
    m_config_data.tracks = {{"MandelbrotMix4.fn2", {{0, "tan"}, {2, "log"}}}};
    m_config = m_config_data;
    m_lerper = ParFile::Interpolator{m_config};
    ParFile::ParSet expected{m_lerper.source()};
    set_param(expected, "function", "sin/tan");
    expected.name = "frame-0002";
    ParFile::ParSet frame{m_lerper()};

    frame = m_lerper();

    ASSERT_EQ(expected, frame);
}

TEST_F(TestInterpolator, functionListTrackWritesOneFunctionAssignment)
{
    m_config_data.source.name = "Formula_Demo";
    m_config_data.num_frames = 3;
    ParFile::KeyframeConfig first;
    first.frame = 0;
    first.value = ParFile::KeyframeConfig::Value::Array{"sin", "cos"};
    ParFile::KeyframeConfig last;
    last.frame = 2;
    last.value = ParFile::KeyframeConfig::Value::Array{"tan", "log"};
    m_config_data.tracks = {{"function", {first, last}}};
    m_config = m_config_data;
    m_lerper = ParFile::Interpolator{m_config};
    ParFile::ParSet expected{m_lerper.source()};
    set_param(expected, "function", "sin/cos");
    expected.name = "frame-0002";
    ParFile::ParSet frame{m_lerper()};

    frame = m_lerper();

    ASSERT_EQ(expected, frame);
}

TEST_F(TestInterpolator, functionSlotPwmTrackWritesOneFunctionAssignment)
{
    m_config_data.source.name = "Formula_Demo";
    m_config_data.num_frames = 4;
    ParFile::TrackConfig track;
    track.parameter = "function[1]";
    track.mode = ParFile::TrackMode::PWM;
    track.pwm = ParFile::PwmConfig{ParFile::PwmEndpointConfig{"tan"}, ParFile::PwmEndpointConfig{"log"}, 2};
    ParFile::KeyframeConfig first;
    first.frame = 0;
    first.mix = 0.0;
    ParFile::KeyframeConfig last;
    last.frame = 3;
    last.mix = 1.0;
    track.keys = {first, last};
    m_config_data.tracks = {track};
    m_config = m_config_data;
    m_lerper = ParFile::Interpolator{m_config};
    ParFile::ParSet expected{m_lerper.source()};
    set_param(expected, "function", "sin/log");
    expected.name = "frame-0003";
    ParFile::ParSet frame{m_lerper()};

    frame = m_lerper();
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

TEST_F(TestInterpolator, colorMapEffectTrackWritesGeneratedMapsAndAtFileValues)
{
    const std::filesystem::path root{std::filesystem::path{TestParFile::TEST_OUTPUT_DIRECTORY} / "color-map-effects"};
    const std::filesystem::path output{root / "output"};
    const std::filesystem::path base_map{root / "input" / "base.map"};
    std::filesystem::remove_all(root);
    write_map_file(base_map, indexed_color());
    ParFile::ColorMapEffectConfig reverse;
    reverse.kind = ParFile::ColorMapEffectKind::REVERSE;
    reverse.range = ParFile::ColorMapRangeConfig{2, 5};
    ParFile::ColorMapEffectConfig ping_pong;
    ping_pong.kind = ParFile::ColorMapEffectKind::PING_PONG;
    ping_pong.range = ParFile::ColorMapRangeConfig{2, 5};
    ping_pong.offset = ParFile::NumberTrackConfig{{{0, 0.0}, {4, 4.0}}};
    ParFile::ColorMapConfig color_map;
    color_map.format = ParFile::TrackFormat::AT_FILE;
    color_map.output = "colors-%04d.map";
    color_map.source = base_map.string();
    color_map.effects = {reverse, ping_pong};
    m_config_data.output.directory = output.string();
    m_config_data.num_frames = 5;
    m_config_data.tracks = {
        {"colors", {}, ParFile::TrackMode::KEYFRAMES, {}, ParFile::TrackKind::COLOR_MAP, color_map}};
    m_config = m_config_data;
    m_lerper = ParFile::Interpolator{m_config};

    const ParFile::ParSet first_frame{m_lerper()};
    static_cast<void>(m_lerper());
    const ParFile::ParSet middle_frame{m_lerper()};
    static_cast<void>(m_lerper());
    const ParFile::ParSet last_frame{m_lerper()};

    const ParFile::ColorMap first{read_map_file(output / "map" / "colors-0001.map")};
    const ParFile::ColorMap middle{read_map_file(output / "map" / "colors-0003.map")};
    const ParFile::ColorMap last{read_map_file(output / "map" / "colors-0005.map")};
    EXPECT_EQ(5, first[2].red);
    EXPECT_EQ(4, first[3].red);
    EXPECT_EQ(3, middle[2].red);
    EXPECT_EQ(2, middle[3].red);
    EXPECT_EQ(3, last[2].red);
    EXPECT_EQ(2, last[3].red);
    const auto colors_value = [](const ParFile::ParSet &frame)
    {
        const auto it{std::find_if(frame.params.begin(), frame.params.end(),
            [](const ParFile::Parameter &param) { return param.name == "colors"; })};
        return it == frame.params.end() ? std::string{} : it->value;
    };
    EXPECT_EQ("@colors-0001.map", colors_value(first_frame));
    EXPECT_EQ("@colors-0003.map", colors_value(middle_frame));
    EXPECT_EQ("@colors-0005.map", colors_value(last_frame));
}

TEST_F(TestInterpolator, colorMapGradientSourceWritesGeneratedMap)
{
    const std::filesystem::path root{std::filesystem::path{TestParFile::TEST_OUTPUT_DIRECTORY} / "color-map-gradient"};
    const std::filesystem::path output{root / "output"};
    std::filesystem::remove_all(root);
    ParFile::ColorMapConfig color_map;
    color_map.format = ParFile::TrackFormat::AT_FILE;
    color_map.output = "colors-%04d.map";
    color_map.gradient = ParFile::ColorMapGradientConfig{{{{0, "black"}, {2, "white"}}}};
    m_config_data.output.directory = output.string();
    m_config_data.num_frames = 1;
    m_config_data.tracks = {
        {"colors", {}, ParFile::TrackMode::KEYFRAMES, {}, ParFile::TrackKind::COLOR_MAP, color_map}};
    m_config = m_config_data;
    m_lerper = ParFile::Interpolator{m_config};

    const ParFile::ParSet frame{m_lerper()};

    const ParFile::ColorMap map{read_map_file(output / "map" / "colors-0001.map")};
    EXPECT_EQ(0, map[0].red);
    EXPECT_EQ(128, map[1].red);
    EXPECT_EQ(255, map[2].red);
    EXPECT_EQ(255, map[255].red);
    const auto it{std::find_if(frame.params.begin(), frame.params.end(),
        [](const ParFile::Parameter &param) { return param.name == "colors"; })};
    ASSERT_NE(frame.params.end(), it);
    EXPECT_EQ("@colors-0001.map", it->value);
}

TEST_F(TestInterpolator, colorMapBrightnessEffectWritesGeneratedMap)
{
    const std::filesystem::path root{
        std::filesystem::path{TestParFile::TEST_OUTPUT_DIRECTORY} / "color-map-brightness"};
    const std::filesystem::path output{root / "output"};
    const std::filesystem::path base_map{root / "input" / "base.map"};
    std::filesystem::remove_all(root);
    write_map_file(base_map, solid_color(100, 120, 200));
    ParFile::ColorMapEffectConfig brightness;
    brightness.kind = ParFile::ColorMapEffectKind::BRIGHTNESS;
    brightness.amount = ParFile::NumberTrackConfig{{{0, 1.0}, {2, 2.0}}};
    ParFile::ColorMapConfig color_map;
    color_map.format = ParFile::TrackFormat::AT_FILE;
    color_map.output = "colors-%04d.map";
    color_map.source = base_map.string();
    color_map.effects = {brightness};
    m_config_data.output.directory = output.string();
    m_config_data.num_frames = 3;
    m_config_data.tracks = {
        {"colors", {}, ParFile::TrackMode::KEYFRAMES, {}, ParFile::TrackKind::COLOR_MAP, color_map}};
    m_config = m_config_data;
    m_lerper = ParFile::Interpolator{m_config};

    static_cast<void>(m_lerper());
    const ParFile::ParSet middle_frame{m_lerper()};

    const ParFile::ColorMap middle{read_map_file(output / "map" / "colors-0002.map")};
    EXPECT_EQ(150, middle[0].red);
    EXPECT_EQ(180, middle[0].green);
    EXPECT_EQ(255, middle[0].blue);
    const auto it{std::find_if(middle_frame.params.begin(), middle_frame.params.end(),
        [](const ParFile::Parameter &param) { return param.name == "colors"; })};
    ASSERT_NE(middle_frame.params.end(), it);
    EXPECT_EQ("@colors-0002.map", it->value);
}

TEST_F(TestInterpolator, colorMapAdjustmentEffectsWriteGeneratedMap)
{
    const std::filesystem::path root{
        std::filesystem::path{TestParFile::TEST_OUTPUT_DIRECTORY} / "color-map-adjustments"};
    const std::filesystem::path output{root / "output"};
    const std::filesystem::path base_map{root / "input" / "base.map"};
    std::filesystem::remove_all(root);
    write_map_file(base_map, solid_color(255, 0, 0));
    ParFile::ColorMapEffectConfig gamma;
    gamma.kind = ParFile::ColorMapEffectKind::GAMMA;
    gamma.amount = ParFile::NumberTrackConfig{{{0, 2.0}, {2, 2.0}}};
    ParFile::ColorMapEffectConfig contrast;
    contrast.kind = ParFile::ColorMapEffectKind::CONTRAST;
    contrast.amount = ParFile::NumberTrackConfig{{{0, 2.0}, {2, 2.0}}};
    ParFile::ColorMapEffectConfig saturation;
    saturation.kind = ParFile::ColorMapEffectKind::SATURATION;
    saturation.amount = ParFile::NumberTrackConfig{{{0, 2.0}, {2, 2.0}}};
    ParFile::ColorMapEffectConfig hue_shift;
    hue_shift.kind = ParFile::ColorMapEffectKind::HUE_SHIFT;
    hue_shift.amount = ParFile::NumberTrackConfig{{{0, 120.0}, {2, 120.0}}};
    ParFile::ColorMapConfig color_map;
    color_map.format = ParFile::TrackFormat::AT_FILE;
    color_map.output = "colors-%04d.map";
    color_map.source = base_map.string();
    color_map.effects = {gamma, contrast, saturation, hue_shift};
    m_config_data.output.directory = output.string();
    m_config_data.num_frames = 3;
    m_config_data.tracks = {
        {"colors", {}, ParFile::TrackMode::KEYFRAMES, {}, ParFile::TrackKind::COLOR_MAP, color_map}};
    m_config = m_config_data;
    m_lerper = ParFile::Interpolator{m_config};

    static_cast<void>(m_lerper());
    const ParFile::ParSet middle_frame{m_lerper()};

    const ParFile::ColorMap middle{read_map_file(output / "map" / "colors-0002.map")};
    EXPECT_EQ(0, middle[0].red);
    EXPECT_EQ(255, middle[0].green);
    EXPECT_EQ(0, middle[0].blue);
    const auto it{std::find_if(middle_frame.params.begin(), middle_frame.params.end(),
        [](const ParFile::Parameter &param) { return param.name == "colors"; })};
    ASSERT_NE(middle_frame.params.end(), it);
    EXPECT_EQ("@colors-0002.map", it->value);
}

TEST_F(TestInterpolator, colorMapMaskedEffectsWriteGeneratedMap)
{
    const std::filesystem::path root{std::filesystem::path{TestParFile::TEST_OUTPUT_DIRECTORY} / "color-map-masked"};
    const std::filesystem::path output{root / "output"};
    const std::filesystem::path base_map{root / "input" / "base.map"};
    const std::filesystem::path mask_map{root / "input" / "mask.map"};
    std::filesystem::remove_all(root);
    write_map_file(base_map, indexed_color());
    write_map_file(mask_map, solid_color(100, 120, 140));
    std::vector<int> indices(ParFile::COLOR_MAP_SIZE);
    for (std::size_t i = 0; i < indices.size(); ++i)
    {
        indices[i] = static_cast<int>(i);
    }
    indices[2] = 5;
    ParFile::ColorMapEffectConfig remap;
    remap.kind = ParFile::ColorMapEffectKind::REMAP;
    remap.indices = indices;
    ParFile::ColorMapEffectConfig pulse;
    pulse.kind = ParFile::ColorMapEffectKind::PULSE;
    pulse.range = ParFile::ColorMapRangeConfig{2, 2};
    pulse.color = "white";
    pulse.amount = ParFile::NumberTrackConfig{{{0, 1.0}, {2, 1.0}}};
    ParFile::ColorMapEffectConfig mask_blend;
    mask_blend.kind = ParFile::ColorMapEffectKind::MASK_BLEND;
    mask_blend.ranges = {ParFile::ColorMapRangeConfig{3, 3}};
    mask_blend.source = mask_map.string();
    mask_blend.amount = ParFile::NumberTrackConfig{{{0, 1.0}, {2, 1.0}}};
    ParFile::ColorMapEffectConfig sparkle;
    sparkle.kind = ParFile::ColorMapEffectKind::SPARKLE;
    sparkle.range = ParFile::ColorMapRangeConfig{4, 4};
    sparkle.seed = 1234;
    sparkle.amount = ParFile::NumberTrackConfig{{{0, 0.0}, {2, 0.0}}};
    ParFile::ColorMapConfig color_map;
    color_map.format = ParFile::TrackFormat::AT_FILE;
    color_map.output = "colors-%04d.map";
    color_map.source = base_map.string();
    color_map.effects = {remap, pulse, mask_blend, sparkle};
    m_config_data.output.directory = output.string();
    m_config_data.num_frames = 3;
    m_config_data.tracks = {
        {"colors", {}, ParFile::TrackMode::KEYFRAMES, {}, ParFile::TrackKind::COLOR_MAP, color_map}};
    m_config = m_config_data;
    m_lerper = ParFile::Interpolator{m_config};

    static_cast<void>(m_lerper());
    const ParFile::ParSet middle_frame{m_lerper()};

    const ParFile::ColorMap middle{read_map_file(output / "map" / "colors-0002.map")};
    EXPECT_EQ(255, middle[2].red);
    EXPECT_EQ(255, middle[2].green);
    EXPECT_EQ(255, middle[2].blue);
    EXPECT_EQ(100, middle[3].red);
    EXPECT_EQ(120, middle[3].green);
    EXPECT_EQ(140, middle[3].blue);
    EXPECT_EQ(4, middle[4].red);
    EXPECT_EQ(251, middle[4].green);
    EXPECT_EQ(4, middle[4].blue);
    const auto it{std::find_if(middle_frame.params.begin(), middle_frame.params.end(),
        [](const ParFile::Parameter &param) { return param.name == "colors"; })};
    ASSERT_NE(middle_frame.params.end(), it);
    EXPECT_EQ("@colors-0002.map", it->value);
}

TEST_F(TestInterpolator, colorMapGradientSourceRejectsInvalidColorSpec)
{
    ParFile::ColorMapConfig color_map;
    color_map.format = ParFile::TrackFormat::AT_FILE;
    color_map.output = "colors-%04d.map";
    color_map.gradient = ParFile::ColorMapGradientConfig{{{{0, "black"}, {255, "rgb:300/0/0"}}}};
    m_config_data.num_frames = 1;
    m_config_data.tracks = {
        {"colors", {}, ParFile::TrackMode::KEYFRAMES, {}, ParFile::TrackKind::COLOR_MAP, color_map}};
    m_config = m_config_data;

    EXPECT_THROW(ParFile::Interpolator{m_config}, std::runtime_error);
}

TEST_F(TestInterpolator, unknownAnimatedParameterRejected)
{
    m_config_data.tracks[0].parameter = "unknown";
    m_config = m_config_data;

    EXPECT_THROW(ParFile::Interpolator{m_config}, std::runtime_error);
}
