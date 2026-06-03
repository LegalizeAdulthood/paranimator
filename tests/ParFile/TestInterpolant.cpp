// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/Interpolant.h>

#include <ParFile/Config.h>
#include <ParFile/ParameterCatalog.h>

#include <gtest/gtest.h>

#include <optional>

namespace
{

ParFile::ParameterMetadata metadata(const std::string &name, const std::string &type, std::optional<double> min = {},
    std::optional<double> max = {}, const std::string &extrapolate = "clamp")
{
    return {name, type, "slash", "linear", extrapolate, min, max};
}

std::vector<ParFile::KeyframeConfig> keyframes(const std::string &from, const std::string &to, int num_steps)
{
    return {{0, from}, {num_steps - 1, to}};
}

std::vector<ParFile::KeyframeConfig> keyframes(
    const std::string &from, const std::string &to, const std::string &curve, int num_steps)
{
    return {{0, from}, {num_steps - 1, to, curve}};
}

ParFile::InterpolantPtr create_interpolant(
    const std::string &name, const std::string &type, const std::string &from, const std::string &to, int num_steps)
{
    return ParFile::create_interpolant(metadata(name, type), keyframes(from, to, num_steps), num_steps, from);
}

ParFile::InterpolantPtr create_interpolant(
    const std::string &name, const std::string &type, const std::vector<ParFile::KeyframeConfig> &keys, int num_steps)
{
    return ParFile::create_interpolant(metadata(name, type), keys, num_steps, keys[0].value);
}

ParFile::InterpolantPtr create_interpolant(
    const ParFile::ParameterMetadata &parameter_metadata, const std::string &from, const std::string &to, int num_steps)
{
    return ParFile::create_interpolant(parameter_metadata, keyframes(from, to, num_steps), num_steps, from);
}

} // namespace

TEST(TestInterpolant, centerMag)
{
    const std::string from{"-0.5/0.0/1.0"};
    const std::string to{"-0.5/0.0/10.0"};
    const int num_steps{3};

    ParFile::InterpolantPtr interpolant{create_interpolant("center-mag", "center-mag", from, to, num_steps)};

    ASSERT_TRUE(interpolant);
    ASSERT_EQ("center-mag", interpolant->name());
}

TEST(TestInterpolant, centerMagFrom)
{
    const std::string from{"-0.5/0/1"};
    const std::string to{"-0.5/0/10"};
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{create_interpolant("center-mag", "center-mag", from, to, num_steps)};

    const std::string value{interpolant->step()};

    EXPECT_EQ(from, value);
}

TEST(TestInterpolant, centerMagMagnificationIsGeometric)
{
    const std::string from{"-0.5/0.0/1.0"};
    const std::string to{"-0.5/0.0/10.0"};
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{create_interpolant("center-mag", "center-mag", from, to, num_steps)};
    static_cast<void>(interpolant->step());

    const std::string value{interpolant->step()};

    EXPECT_EQ("-0.5/0/3.16228", value);
}

TEST(TestInterpolant, centerMagCenterFraction)
{
    const std::string from{"-1/-2/1"};
    const std::string to{"1/2/1"};
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{create_interpolant("center-mag", "center-mag", from, to, num_steps)};
    static_cast<void>(interpolant->step());

    const std::string value{interpolant->step()};

    EXPECT_EQ("0/0/1", value);
}

TEST(TestInterpolant, centerMagTo)
{
    const std::string from{"-0.5/0/1"};
    const std::string to{"-0.5/0/10"};
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{create_interpolant("center-mag", "center-mag", from, to, num_steps)};
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

    ParFile::InterpolantPtr interpolant{create_interpolant("corners", "corners", from, to, num_steps)};

    ASSERT_TRUE(interpolant);
    ASSERT_EQ("corners", interpolant->name());
}

TEST(TestInterpolant, cornersFrom)
{
    const std::string from{"-3.570101/-0.0499383/-1.320061/1.320061"};
    const std::string to{"-1.8101050271/-1.8099342992/-6.37250230799e-05/6.4320896203e-05"};
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{create_interpolant("corners", "corners", from, to, num_steps)};

    const std::string value{interpolant->step()};

    EXPECT_EQ(from, value);
}

TEST(TestInterpolant, cornersFraction)
{
    const std::string from{"-3.570101/-0.0499383/-1.320061/1.320061"};
    const std::string to{"-1.8101050271/-1.8099342992/-6.37250230799e-05/6.4320896203e-05"};
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{create_interpolant("corners", "corners", from, to, num_steps)};
    static_cast<void>(interpolant->step());

    const std::string value{interpolant->step()};

    EXPECT_EQ("-2.69010301355/-0.9299362996/-0.660062362512/0.660062660448", value);
}

TEST(TestInterpolant, cornersTo)
{
    const std::string from{"-3.570101/-0.0499383/-1.320061/1.320061"};
    const std::string to{"-1.8101050271/-1.8099342992/-6.37250230799e-05/6.4320896203e-05"};
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{create_interpolant("corners", "corners", from, to, num_steps)};
    static_cast<void>(interpolant->step());
    static_cast<void>(interpolant->step());

    const std::string value{interpolant->step()};

    EXPECT_EQ(to, value);
}

TEST(TestInterpolant, cornersSixValueFraction)
{
    const std::string from{"0/10/0/10/1/2"};
    const std::string to{"10/20/10/20/3/4"};
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{create_interpolant("corners", "corners", from, to, num_steps)};
    static_cast<void>(interpolant->step());

    const std::string value{interpolant->step()};

    EXPECT_EQ("5/15/5/15/2/3", value);
}

TEST(TestInterpolant, cornersMismatchedArityRejected)
{
    const std::string from{"0/10/0/10"};
    const std::string to{"10/20/10/20/3/4"};
    const int num_steps{3};

    EXPECT_THROW(create_interpolant("corners", "corners", from, to, num_steps), std::runtime_error);
}

TEST(TestInterpolant, cornersInvalidArityRejected)
{
    const std::string from{"0/10/0/10/1"};
    const std::string to{"10/20/10/20/3"};
    const int num_steps{3};

    EXPECT_THROW(create_interpolant("corners", "corners", from, to, num_steps), std::runtime_error);
}

TEST(TestInterpolant, integerFrom)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{create_interpolant("maxiter", "integer", "100", "200", num_steps)};

    const std::string value{interpolant->step()};

    EXPECT_EQ("100", value);
}

TEST(TestInterpolant, integerTo)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{create_interpolant("maxiter", "integer", "100", "200", num_steps)};
    static_cast<void>(interpolant->step());
    static_cast<void>(interpolant->step());

    const std::string value{interpolant->step()};

    EXPECT_EQ("200", value);
}

TEST(TestInterpolant, integerFraction)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{create_interpolant("maxiter", "integer", "100", "200", num_steps)};
    static_cast<void>(interpolant->step());

    const std::string value{interpolant->step()};

    EXPECT_EQ("150", value);
}

TEST(TestInterpolant, integerRoundingIsStable)
{
    const int num_steps{4};
    ParFile::InterpolantPtr interpolant{create_interpolant("maxiter", "integer", "0", "2", num_steps)};
    static_cast<void>(interpolant->step());

    const std::string value{interpolant->step()};

    EXPECT_EQ("1", value);
}

TEST(TestInterpolant, integerClampExtrapolation)
{
    const int num_steps{4};
    const std::vector<ParFile::KeyframeConfig> keys{{1, "10"}, {2, "20"}};
    ParFile::InterpolantPtr interpolant{create_interpolant("maxiter", "integer", keys, num_steps)};

    EXPECT_EQ("10", interpolant->step());
    EXPECT_EQ("10", interpolant->step());
    EXPECT_EQ("20", interpolant->step());
    EXPECT_EQ("20", interpolant->step());
}

TEST(TestInterpolant, integerBaseExtrapolation)
{
    const int num_steps{4};
    const std::vector<ParFile::KeyframeConfig> keys{{1, "100"}, {2, "200"}};
    ParFile::InterpolantPtr interpolant{
        ParFile::create_interpolant(metadata("maxiter", "integer", {}, {}, "base"), keys, num_steps, "678")};

    EXPECT_EQ("678", interpolant->step());
    EXPECT_TRUE(interpolant->has_value());
    EXPECT_EQ("100", interpolant->step());
    EXPECT_TRUE(interpolant->has_value());
    EXPECT_EQ("200", interpolant->step());
    EXPECT_TRUE(interpolant->has_value());
    EXPECT_EQ("678", interpolant->step());
    EXPECT_TRUE(interpolant->has_value());
}

TEST(TestInterpolant, integerCycleExtrapolation)
{
    const int num_steps{5};
    const std::vector<ParFile::KeyframeConfig> keys{{1, "100"}, {3, "300"}};
    ParFile::InterpolantPtr interpolant{
        ParFile::create_interpolant(metadata("maxiter", "integer", {}, {}, "cycle"), keys, num_steps, "678")};

    EXPECT_EQ("300", interpolant->step());
    EXPECT_EQ("100", interpolant->step());
    EXPECT_EQ("200", interpolant->step());
    EXPECT_EQ("300", interpolant->step());
    EXPECT_EQ("100", interpolant->step());
}

TEST(TestInterpolant, integerHoldCurve)
{
    const int num_steps{4};
    ParFile::InterpolantPtr interpolant{
        create_interpolant("maxiter", "integer", keyframes("100", "200", "hold", num_steps), num_steps)};

    EXPECT_EQ("100", interpolant->step());
    EXPECT_EQ("100", interpolant->step());
    EXPECT_EQ("100", interpolant->step());
    EXPECT_EQ("200", interpolant->step());
}

TEST(TestInterpolant, integerStepCurve)
{
    const int num_steps{4};
    ParFile::InterpolantPtr interpolant{
        create_interpolant("maxiter", "integer", keyframes("100", "200", "step", num_steps), num_steps)};

    EXPECT_EQ("100", interpolant->step());
    EXPECT_EQ("100", interpolant->step());
    EXPECT_EQ("100", interpolant->step());
    EXPECT_EQ("200", interpolant->step());
}

TEST(TestInterpolant, integerUnknownCurveRejected)
{
    const int num_steps{4};

    EXPECT_THROW(create_interpolant("maxiter", "integer", keyframes("100", "200", "unknown", num_steps), num_steps),
        std::runtime_error);
}

TEST(TestInterpolant, doubleFraction)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{create_interpolant("bailout", "double", "1.25", "2.5", num_steps)};
    static_cast<void>(interpolant->step());

    const std::string value{interpolant->step()};

    EXPECT_EQ("1.875", value);
}

TEST(TestInterpolant, doublePreservesFractionalEndpoints)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{create_interpolant("bailout", "double", "1.25", "2.5", num_steps)};

    EXPECT_EQ("1.25", interpolant->step());
    static_cast<void>(interpolant->step());
    EXPECT_EQ("2.5", interpolant->step());
}

TEST(TestInterpolant, doubleMinimumRejected)
{
    const int num_steps{3};

    EXPECT_THROW(create_interpolant(metadata("bailout", "double", 1.0), "0.5", "2.5", num_steps), std::runtime_error);
}

TEST(TestInterpolant, doubleMaximumRejected)
{
    const int num_steps{3};

    EXPECT_THROW(
        create_interpolant(metadata("bailout", "double", {}, 2.0), "1.25", "2.5", num_steps), std::runtime_error);
}

TEST(TestInterpolant, doubleOmitExtrapolation)
{
    const int num_steps{4};
    const std::vector<ParFile::KeyframeConfig> keys{{1, "1.5"}, {2, "2.5"}};
    ParFile::InterpolantPtr interpolant{
        ParFile::create_interpolant(metadata("bailout", "double", {}, {}, "omit"), keys, num_steps, "1.25")};

    static_cast<void>(interpolant->step());
    EXPECT_FALSE(interpolant->has_value());
    EXPECT_EQ("1.5", interpolant->step());
    EXPECT_TRUE(interpolant->has_value());
    EXPECT_EQ("2.5", interpolant->step());
    EXPECT_TRUE(interpolant->has_value());
    static_cast<void>(interpolant->step());
    EXPECT_FALSE(interpolant->has_value());
}

TEST(TestInterpolant, doublePingPongExtrapolation)
{
    const int num_steps{5};
    const std::vector<ParFile::KeyframeConfig> keys{{1, "1"}, {3, "3"}};
    ParFile::InterpolantPtr interpolant{
        ParFile::create_interpolant(metadata("bailout", "double", {}, {}, "ping-pong"), keys, num_steps, "1.25")};

    EXPECT_EQ("2", interpolant->step());
    EXPECT_EQ("1", interpolant->step());
    EXPECT_EQ("2", interpolant->step());
    EXPECT_EQ("3", interpolant->step());
    EXPECT_EQ("2", interpolant->step());
}
