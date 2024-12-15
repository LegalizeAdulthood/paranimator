#include <ParFile/ParFile.h>

#include <gtest/gtest.h>

#include <sstream>

TEST(TestParFile, construct)
{
    std::stringstream contents;
    ParFile::ParFilePtr par_file{ParFile::create(contents)};

    EXPECT_TRUE(par_file);
}

TEST(TestParFile, empty)
{
    std::stringstream contents;
    ParFile::ParFilePtr par_file{ParFile::create(contents)};

    EXPECT_TRUE(par_file->empty());
    EXPECT_EQ(par_file->cbegin(), par_file->cend());
    EXPECT_EQ(par_file->begin(), par_file->end());
}

TEST(TestParFile, notEmpty)
{
    std::stringstream contents{"Name {}"};
    ParFile::ParFilePtr par_file{ParFile::create(contents)};

    EXPECT_FALSE(par_file->empty());
    EXPECT_NE(par_file->cbegin(), par_file->cend());
    EXPECT_NE(par_file->begin(), par_file->end());
}

TEST(TestParFile, parSetName)
{
    std::stringstream contents{"Name {}"};
    ParFile::ParFilePtr par_file{ParFile::create(contents)};

    EXPECT_EQ("Name", par_file->cbegin()->name);
}

TEST(TestParFile, parSetFoo)
{
    std::stringstream contents{"Foo {}"};
    ParFile::ParFilePtr par_file{ParFile::create(contents)};

    EXPECT_EQ("Foo", par_file->cbegin()->name);
}

TEST(TestParFile, multipleParSetNames)
{
    std::stringstream contents{"Foo {}\nBar {}\n"};
    ParFile::ParFilePtr par_file{ParFile::create(contents)};

    auto it = par_file->begin();
    EXPECT_EQ("Foo", it->name);
    EXPECT_EQ("Bar", (++it)->name);
}
