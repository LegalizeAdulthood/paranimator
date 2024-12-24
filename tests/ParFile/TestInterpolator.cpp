// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/Interpolator.h>

#include <TestParFile/test.h>

#include <ParFile/Config.h>
#include <ParFile/Json.h>
#include <boost/json.hpp>

#include <gtest/gtest.h>

namespace
{

struct TestInterpolator : testing::Test
{
    ~TestInterpolator() override = default;

protected:
    void SetUp() override
    {
        m_json = ParFile::read_json(TestParFile::CONFIG_JSON).as_object();
        m_config = ParFile::Config{m_json};
        m_lerper = ParFile::Interpolator{m_config};
    }

    void add_expected_params(ParFile::ParSet &frame, const std::string &save_name);

    boost::json::object m_json;
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

TEST_F(TestInterpolator, firstFrameIsFrom)
{
    ParFile::ParSet expected{m_lerper.from()};
    expected.name = "frame-0001";
    add_expected_params(expected, expected.name + ".gif");
    ParFile::ParSet frame{m_lerper()};

    ASSERT_EQ( expected, frame );
}

TEST_F(TestInterpolator, lastFrameIsTo)
{
    m_json.at("num_frames").as_int64() = 2;
    m_config = ParFile::Config{m_json};
    m_lerper = ParFile::Interpolator{m_config};
    ParFile::ParSet expected{m_lerper.to()};
    expected.name = "frame-0002";
    add_expected_params(expected, expected.name + ".gif");
    ParFile::ParSet frame{m_lerper()};

    frame = m_lerper();

    ASSERT_EQ( expected, frame );
}

TEST_F(TestInterpolator, inbetweenFramesAreInterpolated)
{
    m_json.at("num_frames").as_int64() = 3;
    m_config = ParFile::Config{m_json};
    m_lerper = ParFile::Interpolator{m_config};
    ParFile::ParSet expected{m_lerper.to()};
    expected.name = "frame-0002";
    expected.params[2].value  = "-0.5/0/5.5";
    add_expected_params(expected, expected.name + ".gif");
    ParFile::ParSet frame{m_lerper()};

    frame = m_lerper();

    ASSERT_EQ( expected, frame );
}
