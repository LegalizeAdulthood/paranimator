#include <ParFile/ParFile.h>

#include <gtest/gtest.h>

TEST(TestParameter, emptyComparesEqual)
{
    ParFile::Parameter lhs;
    ParFile::Parameter rhs;

    EXPECT_EQ(lhs, rhs);
}

TEST(TestParameter, differentNames)
{
    ParFile::Parameter lhs{"foo", ""};
    ParFile::Parameter rhs{"bar", ""};

    EXPECT_NE(lhs, rhs);
}

TEST(TestParameter, differentValues)
{
    ParFile::Parameter lhs{"foo", "1"};
    ParFile::Parameter rhs{"foo", "2"};

    EXPECT_NE(lhs, rhs);
}

TEST(TestParameter, sameParameter)
{
    ParFile::Parameter lhs{"foo", "1"};
    ParFile::Parameter rhs{"foo", "1"};

    EXPECT_EQ(lhs, rhs);
}

TEST(TestParameter, streamInsert)
{
    std::ostringstream str;
    ParFile::Parameter par{"foo", "1"};

    str << par;

    EXPECT_EQ("foo=1", str.str());
}

TEST(TestParameter, toString)
{
    ParFile::Parameter par{"foo", "1"};

    const std::string str{to_string(par)};

    EXPECT_EQ("foo=1", str);
}

TEST(TestParameter, emptyValueToString)
{
    ParFile::Parameter par{"foo", {}};

    const std::string str{to_string(par)};

    EXPECT_EQ("foo", str);
}

TEST(TestParameter, emptyValueInsert)
{
    std::ostringstream str;
    ParFile::Parameter par{"foo", ""};

    str << par;

    EXPECT_EQ("foo", str.str());
}
