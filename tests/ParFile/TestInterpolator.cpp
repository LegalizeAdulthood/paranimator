// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/Interpolator.h>

#include <TestParFile/test.h>

#include <ParFile/Config.h>

#include <gtest/gtest.h>

#include <algorithm>
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

TEST_F(TestInterpolator, unknownAnimatedParameterRejected)
{
    m_config_data.tracks[0].parameter = "unknown";
    m_config = m_config_data;

    EXPECT_THROW(ParFile::Interpolator{m_config}, std::runtime_error);
}
