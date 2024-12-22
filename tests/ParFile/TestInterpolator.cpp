#include <ParFile/Interpolator.h>

#include <TestParFile/test.h>

#include <ParFile/Config.h>
#include <ParFile/Json.h>
#include <boost/json.hpp>

#include <gtest/gtest.h>

TEST(TestInterpolator, construct)
{
    boost::json::value json{ParFile::read_json(TestParFile::CONFIG_JSON)};
    ParFile::Config config{json.as_object()};

    ParFile::Interpolator lerper{config};
}

TEST(TestInterpolator, firstFrameIsFrom)
{
    boost::json::value json{ParFile::read_json(TestParFile::CONFIG_JSON)};
    ParFile::Config config{json.as_object()};
    ParFile::Interpolator lerper{config};
    ParFile::ParSet expected{lerper.from()};
    expected.name = "frame-0001";
    expected.params.push_back({"batch", "yes"});
    expected.params.push_back({"savename", "frame-0001.gif"});

    ParFile::ParSet frame{lerper()};

    ASSERT_EQ( expected, frame );
}
