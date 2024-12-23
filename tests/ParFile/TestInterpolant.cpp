// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/Interpolant.h>

#include <gtest/gtest.h>

TEST(TestInterpolant, centerMag)
{
    const std::string from{"-0.5/0.0/1.0"};
    const std::string to{"-0.5/0.0/10.0"};
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant("center-mag", from, to, num_steps)};

    ASSERT_TRUE(interpolant);
}

TEST(TestInterpolant, centerMagFrom)
{
    const std::string from{"-0.5/0/1"};
    const std::string to{"-0.5/0/10"};
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant("center-mag", from, to, num_steps)};

    const std::string value{interpolant->step()};

    EXPECT_EQ(from, value);
}

TEST(TestInterpolant, centerMagFraction)
{
    const std::string from{"-0.5/0.0/1.0"};
    const std::string to{"-0.5/0.0/10.0"};
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant("center-mag", from, to, num_steps)};
    static_cast<void>(interpolant->step());

    const std::string value{interpolant->step()};

    EXPECT_EQ("-0.5/0/5.5", value);
}

TEST(TestInterpolant, centerMagTo)
{
    const std::string from{"-0.5/0/1"};
    const std::string to{"-0.5/0/10"};
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant("center-mag", from, to, num_steps)};
    static_cast<void>(interpolant->step());
    static_cast<void>(interpolant->step());

    const std::string value{interpolant->step()};

    EXPECT_EQ(to, value);
}
