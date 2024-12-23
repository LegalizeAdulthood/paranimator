// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/ParFile.h>

#include <gtest/gtest.h>

#include <sstream>

TEST(TestParFile, construct)
{
    std::stringstream contents;
    ParFile::ParFilePtr par_file{ParFile::create_par_file(contents)};

    EXPECT_TRUE(par_file);
}

TEST(TestParFile, empty)
{
    std::stringstream contents;
    ParFile::ParFilePtr par_file{ParFile::create_par_file(contents)};

    EXPECT_TRUE(par_file->empty());
    EXPECT_EQ(par_file->cbegin(), par_file->cend());
    EXPECT_EQ(par_file->begin(), par_file->end());
}

TEST(TestParFile, notEmpty)
{
    std::stringstream contents{"Name {}"};
    ParFile::ParFilePtr par_file{ParFile::create_par_file(contents)};

    EXPECT_FALSE(par_file->empty());
    EXPECT_NE(par_file->cbegin(), par_file->cend());
    EXPECT_NE(par_file->begin(), par_file->end());
}

TEST(TestParFile, parSetName)
{
    std::stringstream contents{"Name {}"};
    ParFile::ParFilePtr par_file{ParFile::create_par_file(contents)};

    EXPECT_EQ("Name", par_file->cbegin()->name);
}

TEST(TestParFile, parSetFoo)
{
    std::stringstream contents{"Foo {}"};
    ParFile::ParFilePtr par_file{ParFile::create_par_file(contents)};

    EXPECT_EQ("Foo", par_file->cbegin()->name);
}

TEST(TestParFile, multipleParSetNames)
{
    std::stringstream contents{"Foo {}\nBar {}\n"};
    ParFile::ParFilePtr par_file{ParFile::create_par_file(contents)};

    auto it = par_file->begin();
    EXPECT_EQ("Foo", it->name);
    EXPECT_EQ("Bar", (++it)->name);
}

TEST(TestParFile, parSetHasParams)
{
    std::stringstream contents{"Foo {\n"
                               "    type=mandel\n"
                               "}"};
    ParFile::ParFilePtr par_file{ParFile::create_par_file(contents)};

    ASSERT_FALSE(par_file->empty());
    const ParFile::ParSet &set{*par_file->begin()};
    EXPECT_EQ("Foo", set.name);
    EXPECT_FALSE(set.params.empty());
}

TEST(TestParFile, parSetParamNameValue)
{
    std::stringstream contents{"Foo {\n"
                               "    type=mandel\n"
                               "}"};
    ParFile::ParFilePtr par_file{ParFile::create_par_file(contents)};

    ASSERT_FALSE(par_file->empty());
    const ParFile::ParSet &set{*par_file->begin()};
    EXPECT_EQ("Foo", set.name);
    ASSERT_FALSE(set.params.empty());
    EXPECT_EQ("type", set.params.back().name);
    EXPECT_EQ("mandel", set.params.back().value);
}

TEST(TestParFile, parSetMultipleParams)
{
    std::stringstream contents{"Foo {\n"
                               "    type=mandel video=F6\n"
                               "}"};
    ParFile::ParFilePtr par_file{ParFile::create_par_file(contents)};

    ASSERT_FALSE(par_file->empty());
    const ParFile::ParSet &set{*par_file->begin()};
    EXPECT_EQ("Foo", set.name);
    ASSERT_EQ(2u, set.params.size());
    EXPECT_EQ("type", set.params[0].name);
    EXPECT_EQ("mandel", set.params[0].value);
    EXPECT_EQ("video", set.params[1].name);
    EXPECT_EQ("F6", set.params[1].value);
}

TEST(TestParFile, parSetParamContinued)
{
    std::stringstream contents{"Foo {\n"
                               "    type=mandel video=\\\n"
                               "    F6\n"
                               "}"};
    ParFile::ParFilePtr par_file{ParFile::create_par_file(contents)};

    ASSERT_FALSE(par_file->empty());
    const ParFile::ParSet &set{*par_file->begin()};
    EXPECT_EQ("Foo", set.name);
    ASSERT_EQ(2u, set.params.size());
    EXPECT_EQ("type", set.params[0].name);
    EXPECT_EQ("mandel", set.params[0].value);
    EXPECT_EQ("video", set.params[1].name);
    EXPECT_EQ("F6", set.params[1].value);
}

TEST(TestParFile, parSetParamComment)
{
    std::stringstream contents{"Foo { ; First comment\n"
                               "    type=mandel video=F6 ; Second comment\n"
                               "}"};
    ParFile::ParFilePtr par_file{ParFile::create_par_file(contents)};

    ASSERT_FALSE(par_file->empty());
    const ParFile::ParSet &set{*par_file->begin()};
    EXPECT_EQ("Foo", set.name);
    ASSERT_EQ(2u, set.params.size());
    EXPECT_EQ("type", set.params[0].name);
    EXPECT_EQ("mandel", set.params[0].value);
    EXPECT_EQ("video", set.params[1].name);
    EXPECT_EQ("F6", set.params[1].value);
}

constexpr std::string_view s_demo_par{
    R"par(; SPDX-License-Identifier: GPL-3.0-only
;
Mandel_Demo        { ; PAR for initialization of Fractint demo
   reset type=mandel corners=-2.5/1.5/-1.5/1.5 params=0/0 inside=0
   sound=no maxiter=678
}

trunc_demo        { ; PAR for initialization of Fractint demo (New in 19.4)
   reset type=lambda(fn||fn) function=conj/trunc
   center-mag=-4.44089e-015/3.55271e-015/0.2339956
   params=-0.7398119122257053/1.28140703517588/52 float=y maxiter=255
   bailoutest=or inside=0 decomp=256 sound=no
   colors=DUH<2>EXJFZKFYK<29>121000110<30>rZ0<30>220000011<13>0OL0QM2RM<14>\
   ZhZ<30>222000110<30>zkK<30>221000010<25>CTG cyclerange=0/255
}
)par"};

TEST(TestParFile, parSetMandelDemo)
{
    std::stringstream contents{s_demo_par.data()};
    ParFile::ParFilePtr par_file{ParFile::create_par_file(contents)};

    ASSERT_EQ(2U, par_file->size());
    const ParFile::ParSet &set{*par_file->begin()};
    EXPECT_EQ("Mandel_Demo", set.name);
    ASSERT_EQ(7U, set.params.size());
    EXPECT_EQ("reset", set.params[0].name);
    EXPECT_EQ("type", set.params[1].name);
    EXPECT_EQ("mandel", set.params[1].value);
    EXPECT_EQ("corners", set.params[2].name);
    EXPECT_EQ("params", set.params[3].name);
    EXPECT_EQ("inside", set.params[4].name);
    EXPECT_EQ("sound", set.params[5].name);
    EXPECT_EQ("maxiter", set.params[6].name);
}

TEST(TestParFile, parSetTruncDemo)
{
    std::stringstream contents{s_demo_par.data()};
    ParFile::ParFilePtr par_file{ParFile::create_par_file(contents)};

    ASSERT_EQ(2U, par_file->size());
    auto set_iter{par_file->begin()};
    ++set_iter;
    const ParFile::ParSet &set{*set_iter};
    EXPECT_EQ("trunc_demo", set.name);
    ASSERT_EQ(13U, set.params.size());
    auto param{set.params.begin()};
    EXPECT_EQ("reset", param->name);
    ++param;
    EXPECT_EQ("type", param->name);
    EXPECT_EQ("lambda(fn||fn)", param->value);
    EXPECT_EQ("function", (++param)->name);
    EXPECT_EQ("center-mag", (++param)->name);
    EXPECT_EQ("params", (++param)->name);
    EXPECT_EQ("float", (++param)->name);
    EXPECT_EQ("maxiter", (++param)->name);
    EXPECT_EQ("bailoutest", (++param)->name);
    EXPECT_EQ("inside", (++param)->name);
    EXPECT_EQ("decomp", (++param)->name);
    EXPECT_EQ("sound", (++param)->name);
    EXPECT_EQ("colors", (++param)->name);
    EXPECT_EQ("DUH<2>EXJFZKFYK<29>121000110<30>rZ0<30>220000011<13>0OL0QM2RM<14>"
              "ZhZ<30>222000110<30>zkK<30>221000010<25>CTG",
        param->value);
    EXPECT_EQ("cyclerange", (++param)->name);
}
