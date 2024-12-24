// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/ParFile.h>

#include <gtest/gtest.h>

TEST(TestParSet, emptyComparesEqual)
{
    ParFile::ParSet lhs;
    ParFile::ParSet rhs;

    EXPECT_EQ(lhs, rhs);
}

TEST(TestParSet, differentName)
{
    ParFile::ParSet lhs{"foo", {}};
    ParFile::ParSet rhs{"bar", {}};

    EXPECT_NE(lhs, rhs);
}

TEST(TestParSet, sameNameDifferentNumParams)
{
    ParFile::ParSet lhs{"foo", {{"foo", ""}}};
    ParFile::ParSet rhs{"bar", {{"foo", ""}, {"bar", ""}}};

    EXPECT_NE(lhs, rhs);
}

TEST(TestParSet, sameNameDifferentParamNames)
{
    ParFile::ParSet lhs{"foo", {{"foo", "1"}}};
    ParFile::ParSet rhs{"foo", {{"bar", "1"}}};

    EXPECT_NE(lhs, rhs);
}

TEST(TestParSet, sameNameDifferentParamValues)
{
    ParFile::ParSet lhs{"foo", {{"foo", "1"}}};
    ParFile::ParSet rhs{"foo", {{"foo", "2"}}};

    EXPECT_NE(lhs, rhs);
}

TEST(TestParSet, equal)
{
    ParFile::ParSet lhs{"foo", {{"foo", "1"}}};
    ParFile::ParSet rhs{"foo", {{"foo", "1"}}};

    EXPECT_EQ(lhs, rhs);
}

TEST(TestParSet, emptyInsert)
{
    std::ostringstream str;
    ParFile::ParSet empty;

    str << empty;

    EXPECT_TRUE(str.str().empty());
}

TEST(TestParSet, emptyToString)
{
    ParFile::ParSet empty;

    const std::string str{to_string(empty)};

    EXPECT_TRUE(str.empty());
}

TEST(TestParSet, nameNoParamsInsert)
{
    std::ostringstream str;
    ParFile::ParSet name_only{"foo", {}};

    str << name_only;

    EXPECT_EQ(R"par(foo {
}
)par",
        str.str());
}

TEST(TestParSet, nameNoParamsToString)
{
    ParFile::ParSet name_only{"foo", {}};

    const std::string str{to_string(name_only)};

    EXPECT_EQ(R"par(foo {
}
)par",
        str);
}

TEST(TestParSet, paramsInsert)
{
    std::ostringstream str;
    ParFile::ParSet name_only{"foo", {{"foo", "1"}, {"bar", "2"}}};

    str << name_only;

    EXPECT_EQ(R"par(foo {
    foo=1
    bar=2
}
)par",
        str.str());
}

TEST(TestParSet, paramsToString)
{
    ParFile::ParSet name_only{"foo", {{"foo", "1"}, {"bar", "2"}}};

    const std::string str{to_string(name_only)};

    EXPECT_EQ(R"par(foo {
    foo=1
    bar=2
}
)par",
        str);
}

TEST(TestParSet, foldLinesToString)
{
    ParFile::ParSet long_value{"foo",
        {{"colors",
             "DUH"
             "<2>EXJFZKFYK"
             "<29>121000110"
             "<30>rZ0"
             "<30>220000011"
             "<13>0OL0QM2RM"
             "<14>ZhZ"
             "<30>222000110"
             "<30>zkK"
             "<30>221000010"
             "<25>CTG"},
            {"bar", "2"}}};

    const std::string str{to_string(long_value)};

    EXPECT_EQ(R"par(foo {
    colors=DUH<2>EXJFZKFYK<29>121000110<30>rZ0<30>220000011<13>0OL0QM\
        2RM<14>ZhZ<30>222000110<30>zkK<30>221000010<25>CTG
    bar=2
}
)par", str);
}
