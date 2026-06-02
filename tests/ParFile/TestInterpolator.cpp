// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/Interpolator.h>

#include <TestParFile/test.h>

#include <ParFile/Config.h>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <fstream>
#include <iterator>
#include <stdexcept>

namespace
{

nlohmann::json read_json(const char *path)
{
    std::ifstream in{path};
    const std::string text{std::istreambuf_iterator<char>{in}, std::istreambuf_iterator<char>{}};
    return nlohmann::json::parse(text.begin(), text.end());
}

struct TestInterpolator : testing::Test
{
    ~TestInterpolator() override = default;

protected:
    void SetUp() override
    {
        m_json = read_json(TestParFile::CENTER_MAG_CONFIG_JSON);
        m_config = ParFile::Config{m_json.dump()};
        m_lerper = ParFile::Interpolator{m_config};
    }

    void add_expected_params(ParFile::ParSet &frame, const std::string &save_name);

    nlohmann::json m_json;
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
    m_json["num_frames"] = 2;
    m_json["tracks"][0]["keys"][1]["frame"] = 1;
    m_config = ParFile::Config{m_json.dump()};
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
    m_json["num_frames"] = 3;
    m_json["tracks"][0]["keys"][1]["frame"] = 2;
    m_config = ParFile::Config{m_json.dump()};
    m_lerper = ParFile::Interpolator{m_config};
    ParFile::ParSet expected{m_lerper.source()};
    expected.params[2].value = "-0.5/0/3.16228";
    expected.name = "frame-0002";
    add_expected_params(expected, expected.name + ".gif");
    ParFile::ParSet frame{m_lerper()};

    frame = m_lerper();

    ASSERT_EQ(expected, frame);
}

TEST_F(TestInterpolator, unknownAnimatedParameterRejected)
{
    m_json["tracks"][0]["parameter"] = "unknown";
    m_config = ParFile::Config{m_json.dump()};

    EXPECT_THROW(ParFile::Interpolator{m_config}, std::runtime_error);
}
