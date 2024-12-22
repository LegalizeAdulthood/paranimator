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
