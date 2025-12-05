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
    ASSERT_EQ("center-mag", interpolant->name());
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

    EXPECT_EQ("-0.5/0/3.16228", value);
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

TEST(TestInterpolant, corners)
{
    const std::string from{"-3.570101/-0.0499383/-1.320061/1.320061"};
    const std::string to{"-1.8101050271/-1.8099342992/-6.37250230799e-05/6.4320896203e-05"};
    const int num_steps{3};

    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant("corners", from, to, num_steps)};

    ASSERT_TRUE(interpolant);
    ASSERT_EQ("corners", interpolant->name());
}

TEST(TestInterpolant, cornersFrom)
{
    const std::string from{"-3.570101/-0.0499383/-1.320061/1.320061"};
    const std::string to{"-1.8101050271/-1.8099342992/-6.37250230799e-05/6.4320896203e-05"};
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant("corners", from, to, num_steps)};

    const std::string value{interpolant->step()};

    EXPECT_EQ(from, value);
}

TEST(TestInterpolant, cornersFraction)
{
    const std::string from{"-3.570101/-0.0499383/-1.320061/1.320061"};
    const std::string to{"-1.8101050271/-1.8099342992/-6.37250230799e-05/6.4320896203e-05"};
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant("corners", from, to, num_steps)};
    static_cast<void>(interpolant->step());

    const std::string value{interpolant->step()};

    EXPECT_EQ("-2.69010301355/-0.9299362996/-0.660062362512/0.660062660448", value);
}

TEST(TestInterpolant, cornersTo)
{
    const std::string from{"-3.570101/-0.0499383/-1.320061/1.320061"};
    const std::string to{"-1.8101050271/-1.8099342992/-6.37250230799e-05/6.4320896203e-05"};
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant("corners", from, to, num_steps)};
    static_cast<void>(interpolant->step());
    static_cast<void>(interpolant->step());

    const std::string value{interpolant->step()};

    EXPECT_EQ(to, value);
}
