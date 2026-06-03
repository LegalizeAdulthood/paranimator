// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/Interpolant.h>

#include <ParFile/Config.h>
#include <ParFile/ParameterCatalog.h>

#include <gtest/gtest.h>

#include <optional>
#include <utility>
#include <vector>

namespace
{

ParFile::ParameterMetadata metadata(const std::string &name, ParFile::ParameterType type,
    std::optional<double> min = {}, std::optional<double> max = {},
    ParFile::ExtrapolateMode extrapolate = ParFile::ExtrapolateMode::CLAMP,
    ParFile::Curve default_curve = ParFile::Curve::LINEAR)
{
    return {name, type, ParFile::ParameterFormat::SLASH, default_curve, extrapolate, min, max};
}

std::vector<ParFile::KeyframeConfig> keyframes(const std::string &from, const std::string &to, int num_steps)
{
    return {{0, from}, {num_steps - 1, to}};
}

std::vector<ParFile::KeyframeConfig> keyframes(
    const std::string &from, const std::string &to, ParFile::Curve curve, int num_steps)
{
    return {{0, from}, {num_steps - 1, to, curve}};
}

ParFile::KeyframeConfig pwm_keyframe(int frame, double mix)
{
    ParFile::KeyframeConfig result;
    result.frame = frame;
    result.mix = mix;
    return result;
}

std::vector<ParFile::KeyframeConfig> pwm_keyframes(double from, double to, int num_steps)
{
    return {pwm_keyframe(0, from), pwm_keyframe(num_steps - 1, to)};
}

ParFile::ResolvedTrack resolved_track(const std::string &name, ParFile::ParameterType type,
    const std::vector<ParFile::KeyframeConfig> &keys, const std::string &base_value)
{
    return {name, metadata(name, type), base_value, keys, name, {}};
}

ParFile::ResolvedTrack resolved_track(const ParFile::ParameterMetadata &parameter_metadata,
    const std::vector<ParFile::KeyframeConfig> &keys, const std::string &base_value)
{
    return {parameter_metadata.name, parameter_metadata, base_value, keys, parameter_metadata.name, {}};
}

ParFile::ResolvedTrack resolved_params_track(const std::string &name, ParFile::ParameterType type,
    const std::vector<ParFile::KeyframeConfig> &keys, const std::string &base_value, std::vector<int> slots)
{
    return {name, metadata(name, type), base_value, keys, "params", std::move(slots)};
}

ParFile::ResolvedTrack resolved_pwm_track(const ParFile::ParameterMetadata &parameter_metadata, const std::string &a,
    const std::string &b, int window, double from, double to, int num_steps)
{
    ParFile::ResolvedTrack result{resolved_track(parameter_metadata, pwm_keyframes(from, to, num_steps), a)};
    result.mode = ParFile::TrackMode::PWM;
    result.pwm = ParFile::PwmConfig{a, b, window};
    return result;
}

ParFile::ParameterMetadata tuple_metadata(const std::string &name, int arity)
{
    ParFile::ParameterMetadata result{metadata(name, ParFile::ParameterType::NUMERIC_TUPLE)};
    result.arity = arity;
    return result;
}

ParFile::ParameterMetadata enum_metadata(const std::string &name)
{
    ParFile::ParameterMetadata result{
        metadata(name, ParFile::ParameterType::ENUM, {}, {}, ParFile::ExtrapolateMode::CLAMP, ParFile::Curve::HOLD)};
    result.format = ParFile::ParameterFormat::RAW;
    result.values = {"bof60", "zmag", "epsiloncross", "startrail"};
    return result;
}

ParFile::ParameterMetadata inside_metadata(const std::string &name)
{
    ParFile::ParameterMetadata result{metadata(
        name, ParFile::ParameterType::INSIDE, 0.0, 255.0, ParFile::ExtrapolateMode::CLAMP, ParFile::Curve::HOLD)};
    result.format = ParFile::ParameterFormat::RAW;
    result.values = {"maxiter", "zmag", "bof60", "bof61", "epsiloncross", "startrail", "period", "atan", "fmod"};
    return result;
}

ParFile::ParameterMetadata outside_metadata(const std::string &name)
{
    ParFile::ParameterMetadata result{metadata(
        name, ParFile::ParameterType::OUTSIDE, 0.0, 255.0, ParFile::ExtrapolateMode::CLAMP, ParFile::Curve::HOLD)};
    result.format = ParFile::ParameterFormat::RAW;
    result.values = {"iter", "real", "imag", "mult", "summ", "atan", "fmod", "tdis"};
    return result;
}

std::vector<std::string> id_functions()
{
    return {"sin", "cos", "tan", "cotan", "sinh", "cosh", "tanh", "cotanh", "exp", "log", "sqr", "recip", "ident",
        "cosxx", "flip", "conj", "zero", "one", "asin", "asinh", "acos", "acosh", "atan", "atanh", "sqrt", "abs",
        "cabs", "floor", "ceil", "trunc", "round"};
}

ParFile::ResolvedTrack resolved_function_track(
    const std::string &name, const std::vector<ParFile::KeyframeConfig> &keys, const std::string &base_value, int slot)
{
    ParFile::ParameterMetadata function_metadata{name, ParFile::ParameterType::ENUM, ParFile::ParameterFormat::RAW,
        ParFile::Curve::HOLD, ParFile::ExtrapolateMode::CLAMP, {}, {}, id_functions()};
    return {name, function_metadata, base_value, keys, "function", {slot}};
}

ParFile::InterpolantPtr create_interpolant(
    const std::string &name, ParFile::ParameterType type, const std::string &from, const std::string &to, int num_steps)
{
    return ParFile::create_interpolant(resolved_track(name, type, keyframes(from, to, num_steps), from), num_steps);
}

ParFile::InterpolantPtr create_interpolant(const std::string &name, ParFile::ParameterType type,
    const std::vector<ParFile::KeyframeConfig> &keys, int num_steps)
{
    return ParFile::create_interpolant(resolved_track(name, type, keys, keys[0].value), num_steps);
}

ParFile::InterpolantPtr create_interpolant(
    const ParFile::ParameterMetadata &parameter_metadata, const std::string &from, const std::string &to, int num_steps)
{
    const std::vector<ParFile::KeyframeConfig> keys{keyframes(from, to, num_steps)};
    return ParFile::create_interpolant(resolved_track(parameter_metadata, keys, from), num_steps);
}

ParFile::InterpolantPtr create_interpolant(const ParFile::ParameterMetadata &parameter_metadata,
    const std::string &from, const std::string &to, ParFile::Curve curve, int num_steps)
{
    const std::vector<ParFile::KeyframeConfig> keys{keyframes(from, to, curve, num_steps)};
    return ParFile::create_interpolant(resolved_track(parameter_metadata, keys, from), num_steps);
}

} // namespace

TEST(TestInterpolant, centerMag)
{
    const std::string from{"-0.5/0.0/1.0"};
    const std::string to{"-0.5/0.0/10.0"};
    const int num_steps{3};

    ParFile::InterpolantPtr interpolant{
        create_interpolant("center-mag", ParFile::ParameterType::CENTER_MAG, from, to, num_steps)};

    ASSERT_TRUE(interpolant);
    ASSERT_EQ("center-mag", interpolant->name());
}

TEST(TestInterpolant, centerMagFrom)
{
    const std::string from{"-0.5/0/1"};
    const std::string to{"-0.5/0/10"};
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{
        create_interpolant("center-mag", ParFile::ParameterType::CENTER_MAG, from, to, num_steps)};

    const std::string value{interpolant->step()};

    EXPECT_EQ(from, value);
}

TEST(TestInterpolant, centerMagMagnificationIsGeometric)
{
    const std::string from{"-0.5/0.0/1.0"};
    const std::string to{"-0.5/0.0/10.0"};
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{
        create_interpolant("center-mag", ParFile::ParameterType::CENTER_MAG, from, to, num_steps)};
    static_cast<void>(interpolant->step());

    const std::string value{interpolant->step()};

    EXPECT_EQ("-0.5/0/3.16228", value);
}

TEST(TestInterpolant, centerMagCenterFraction)
{
    const std::string from{"-1/-2/1"};
    const std::string to{"1/2/1"};
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{
        create_interpolant("center-mag", ParFile::ParameterType::CENTER_MAG, from, to, num_steps)};
    static_cast<void>(interpolant->step());

    const std::string value{interpolant->step()};

    EXPECT_EQ("0/0/1", value);
}

TEST(TestInterpolant, centerMagTo)
{
    const std::string from{"-0.5/0/1"};
    const std::string to{"-0.5/0/10"};
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{
        create_interpolant("center-mag", ParFile::ParameterType::CENTER_MAG, from, to, num_steps)};
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

    ParFile::InterpolantPtr interpolant{
        create_interpolant("corners", ParFile::ParameterType::CORNERS, from, to, num_steps)};

    ASSERT_TRUE(interpolant);
    ASSERT_EQ("corners", interpolant->name());
}

TEST(TestInterpolant, cornersFrom)
{
    const std::string from{"-3.570101/-0.0499383/-1.320061/1.320061"};
    const std::string to{"-1.8101050271/-1.8099342992/-6.37250230799e-05/6.4320896203e-05"};
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{
        create_interpolant("corners", ParFile::ParameterType::CORNERS, from, to, num_steps)};

    const std::string value{interpolant->step()};

    EXPECT_EQ(from, value);
}

TEST(TestInterpolant, cornersFraction)
{
    const std::string from{"-3.570101/-0.0499383/-1.320061/1.320061"};
    const std::string to{"-1.8101050271/-1.8099342992/-6.37250230799e-05/6.4320896203e-05"};
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{
        create_interpolant("corners", ParFile::ParameterType::CORNERS, from, to, num_steps)};
    static_cast<void>(interpolant->step());

    const std::string value{interpolant->step()};

    EXPECT_EQ("-2.69010301355/-0.9299362996/-0.660062362512/0.660062660448", value);
}

TEST(TestInterpolant, cornersTo)
{
    const std::string from{"-3.570101/-0.0499383/-1.320061/1.320061"};
    const std::string to{"-1.8101050271/-1.8099342992/-6.37250230799e-05/6.4320896203e-05"};
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{
        create_interpolant("corners", ParFile::ParameterType::CORNERS, from, to, num_steps)};
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
    ParFile::InterpolantPtr interpolant{
        create_interpolant("corners", ParFile::ParameterType::CORNERS, from, to, num_steps)};
    static_cast<void>(interpolant->step());

    const std::string value{interpolant->step()};

    EXPECT_EQ("5/15/5/15/2/3", value);
}

TEST(TestInterpolant, cornersMismatchedArityRejected)
{
    const std::string from{"0/10/0/10"};
    const std::string to{"10/20/10/20/3/4"};
    const int num_steps{3};

    EXPECT_THROW(
        create_interpolant("corners", ParFile::ParameterType::CORNERS, from, to, num_steps), std::runtime_error);
}

TEST(TestInterpolant, cornersInvalidArityRejected)
{
    const std::string from{"0/10/0/10/1"};
    const std::string to{"10/20/10/20/3"};
    const int num_steps{3};

    EXPECT_THROW(
        create_interpolant("corners", ParFile::ParameterType::CORNERS, from, to, num_steps), std::runtime_error);
}

TEST(TestInterpolant, integerFrom)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{
        create_interpolant("maxiter", ParFile::ParameterType::INTEGER, "100", "200", num_steps)};

    const std::string value{interpolant->step()};

    EXPECT_EQ("100", value);
}

TEST(TestInterpolant, integerTo)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{
        create_interpolant("maxiter", ParFile::ParameterType::INTEGER, "100", "200", num_steps)};
    static_cast<void>(interpolant->step());
    static_cast<void>(interpolant->step());

    const std::string value{interpolant->step()};

    EXPECT_EQ("200", value);
}

TEST(TestInterpolant, integerFraction)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{
        create_interpolant("maxiter", ParFile::ParameterType::INTEGER, "100", "200", num_steps)};
    static_cast<void>(interpolant->step());

    const std::string value{interpolant->step()};

    EXPECT_EQ("150", value);
}

TEST(TestInterpolant, integerRoundingIsStable)
{
    const int num_steps{4};
    ParFile::InterpolantPtr interpolant{
        create_interpolant("maxiter", ParFile::ParameterType::INTEGER, "0", "2", num_steps)};
    static_cast<void>(interpolant->step());

    const std::string value{interpolant->step()};

    EXPECT_EQ("1", value);
}

TEST(TestInterpolant, integerClampExtrapolation)
{
    const int num_steps{4};
    const std::vector<ParFile::KeyframeConfig> keys{{1, "10"}, {2, "20"}};
    ParFile::InterpolantPtr interpolant{
        create_interpolant("maxiter", ParFile::ParameterType::INTEGER, keys, num_steps)};

    EXPECT_EQ("10", interpolant->step());
    EXPECT_EQ("10", interpolant->step());
    EXPECT_EQ("20", interpolant->step());
    EXPECT_EQ("20", interpolant->step());
}

TEST(TestInterpolant, integerBaseExtrapolation)
{
    const int num_steps{4};
    const std::vector<ParFile::KeyframeConfig> keys{{1, "100"}, {2, "200"}};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant(
        resolved_track(
            metadata("maxiter", ParFile::ParameterType::INTEGER, {}, {}, ParFile::ExtrapolateMode::BASE), keys, "678"),
        num_steps)};

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
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant(
        resolved_track(
            metadata("maxiter", ParFile::ParameterType::INTEGER, {}, {}, ParFile::ExtrapolateMode::CYCLE), keys, "678"),
        num_steps)};

    EXPECT_EQ("300", interpolant->step());
    EXPECT_EQ("100", interpolant->step());
    EXPECT_EQ("200", interpolant->step());
    EXPECT_EQ("300", interpolant->step());
    EXPECT_EQ("100", interpolant->step());
}

TEST(TestInterpolant, integerHoldCurve)
{
    const int num_steps{4};
    ParFile::InterpolantPtr interpolant{create_interpolant("maxiter", ParFile::ParameterType::INTEGER,
        keyframes("100", "200", ParFile::Curve::HOLD, num_steps), num_steps)};

    EXPECT_EQ("100", interpolant->step());
    EXPECT_EQ("100", interpolant->step());
    EXPECT_EQ("100", interpolant->step());
    EXPECT_EQ("200", interpolant->step());
}

TEST(TestInterpolant, integerStepCurve)
{
    const int num_steps{4};
    ParFile::InterpolantPtr interpolant{create_interpolant("maxiter", ParFile::ParameterType::INTEGER,
        keyframes("100", "200", ParFile::Curve::STEP, num_steps), num_steps)};

    EXPECT_EQ("100", interpolant->step());
    EXPECT_EQ("100", interpolant->step());
    EXPECT_EQ("100", interpolant->step());
    EXPECT_EQ("200", interpolant->step());
}

TEST(TestInterpolant, integerGeometricCurveRejected)
{
    const int num_steps{4};

    EXPECT_THROW(create_interpolant("maxiter", ParFile::ParameterType::INTEGER,
                     keyframes("100", "200", ParFile::Curve::GEOMETRIC, num_steps), num_steps),
        std::runtime_error);
}

TEST(TestInterpolant, doubleFraction)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{
        create_interpolant("bailout", ParFile::ParameterType::DOUBLE, "1.25", "2.5", num_steps)};
    static_cast<void>(interpolant->step());

    const std::string value{interpolant->step()};

    EXPECT_EQ("1.875", value);
}

TEST(TestInterpolant, doublePreservesFractionalEndpoints)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{
        create_interpolant("bailout", ParFile::ParameterType::DOUBLE, "1.25", "2.5", num_steps)};

    EXPECT_EQ("1.25", interpolant->step());
    static_cast<void>(interpolant->step());
    EXPECT_EQ("2.5", interpolant->step());
}

TEST(TestInterpolant, doubleMinimumRejected)
{
    const int num_steps{3};

    EXPECT_THROW(create_interpolant(metadata("bailout", ParFile::ParameterType::DOUBLE, 1.0), "0.5", "2.5", num_steps),
        std::runtime_error);
}

TEST(TestInterpolant, doubleMaximumRejected)
{
    const int num_steps{3};

    EXPECT_THROW(
        create_interpolant(metadata("bailout", ParFile::ParameterType::DOUBLE, {}, 2.0), "1.25", "2.5", num_steps),
        std::runtime_error);
}

TEST(TestInterpolant, doubleOmitExtrapolation)
{
    const int num_steps{4};
    const std::vector<ParFile::KeyframeConfig> keys{{1, "1.5"}, {2, "2.5"}};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant(
        resolved_track(
            metadata("bailout", ParFile::ParameterType::DOUBLE, {}, {}, ParFile::ExtrapolateMode::OMIT), keys, "1.25"),
        num_steps)};

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
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant(
        resolved_track(metadata("bailout", ParFile::ParameterType::DOUBLE, {}, {}, ParFile::ExtrapolateMode::PING_PONG),
            keys, "1.25"),
        num_steps)};

    EXPECT_EQ("2", interpolant->step());
    EXPECT_EQ("1", interpolant->step());
    EXPECT_EQ("2", interpolant->step());
    EXPECT_EQ("3", interpolant->step());
    EXPECT_EQ("2", interpolant->step());
}

TEST(TestInterpolant, numericTupleTwoValueFraction)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{create_interpolant(tuple_metadata("xyshift", 2), "0/1", "10/11", num_steps)};

    EXPECT_EQ("0/1", interpolant->step());
    EXPECT_EQ("5/6", interpolant->step());
    EXPECT_EQ("10/11", interpolant->step());
}

TEST(TestInterpolant, numericTupleThreeValueFraction)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{
        create_interpolant(tuple_metadata("lightsource", 3), "0/10/20", "10/20/30", num_steps)};

    EXPECT_EQ("0/10/20", interpolant->step());
    EXPECT_EQ("5/15/25", interpolant->step());
    EXPECT_EQ("10/20/30", interpolant->step());
}

TEST(TestInterpolant, numericTupleWrongArityRejected)
{
    const int num_steps{3};

    EXPECT_THROW(create_interpolant(tuple_metadata("xyshift", 3), "0/1", "10/11", num_steps), std::runtime_error);
}

TEST(TestInterpolant, numericTupleMissingArityRejected)
{
    const int num_steps{3};
    const ParFile::ParameterMetadata parameter_metadata{metadata("xyshift", ParFile::ParameterType::NUMERIC_TUPLE)};

    EXPECT_THROW(create_interpolant(parameter_metadata, "0/1", "10/11", num_steps), std::runtime_error);
}

TEST(TestInterpolant, point3WritesThreeValueTuple)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{
        create_interpolant(metadata("lightsource", ParFile::ParameterType::POINT3), "0/10/20", "10/20/30", num_steps)};

    EXPECT_EQ("0/10/20", interpolant->step());
    EXPECT_EQ("5/15/25", interpolant->step());
    EXPECT_EQ("10/20/30", interpolant->step());
}

TEST(TestInterpolant, vector3NormalizesWhenRequested)
{
    const int num_steps{3};
    ParFile::ParameterMetadata parameter_metadata{metadata("view-up", ParFile::ParameterType::VECTOR3)};
    parameter_metadata.normalize = true;
    ParFile::InterpolantPtr interpolant{create_interpolant(parameter_metadata, "10/0/0", "0/10/0", num_steps)};

    EXPECT_EQ("1/0/0", interpolant->step());
    EXPECT_EQ("0.707106781187/0.707106781187/0", interpolant->step());
    EXPECT_EQ("0/1/0", interpolant->step());
}

TEST(TestInterpolant, point3DoesNotNormalize)
{
    const int num_steps{3};
    ParFile::ParameterMetadata parameter_metadata{metadata("look-at", ParFile::ParameterType::POINT3)};
    parameter_metadata.normalize = true;
    ParFile::InterpolantPtr interpolant{create_interpolant(parameter_metadata, "10/0/0", "0/10/0", num_steps)};

    EXPECT_EQ("10/0/0", interpolant->step());
    EXPECT_EQ("5/5/0", interpolant->step());
    EXPECT_EQ("0/10/0", interpolant->step());
}

TEST(TestInterpolant, enumHold)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{create_interpolant(enum_metadata("inside"), "bof60", "zmag", num_steps)};

    EXPECT_EQ("bof60", interpolant->step());
    EXPECT_EQ("bof60", interpolant->step());
    EXPECT_EQ("zmag", interpolant->step());
}

TEST(TestInterpolant, enumStep)
{
    const int num_steps{4};
    ParFile::InterpolantPtr interpolant{
        create_interpolant(enum_metadata("inside"), "bof60", "zmag", ParFile::Curve::STEP, num_steps)};

    EXPECT_EQ("bof60", interpolant->step());
    EXPECT_EQ("bof60", interpolant->step());
    EXPECT_EQ("bof60", interpolant->step());
    EXPECT_EQ("zmag", interpolant->step());
}

TEST(TestInterpolant, enumUnknownValueRejected)
{
    const int num_steps{3};

    EXPECT_THROW(create_interpolant(enum_metadata("inside"), "bof60", "unknown", num_steps), std::runtime_error);
}

TEST(TestInterpolant, enumLinearCurveRejected)
{
    const int num_steps{3};
    const ParFile::ParameterMetadata parameter_metadata{enum_metadata("inside")};

    EXPECT_THROW(
        ParFile::create_interpolant(
            resolved_track(parameter_metadata, keyframes("bof60", "zmag", ParFile::Curve::LINEAR, num_steps), "bof60"),
            num_steps),
        std::runtime_error);
}

TEST(TestInterpolant, enumPwmMixZeroEmitsA)
{
    const int num_steps{4};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant(
        resolved_pwm_track(enum_metadata("inside"), "bof60", "zmag", 2, 0.0, 0.0, num_steps), num_steps)};

    EXPECT_EQ("bof60", interpolant->step());
    EXPECT_EQ("bof60", interpolant->step());
    EXPECT_EQ("bof60", interpolant->step());
    EXPECT_EQ("bof60", interpolant->step());
}

TEST(TestInterpolant, insideMethodHold)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{create_interpolant(inside_metadata("inside"), "bof60", "zmag", num_steps)};

    EXPECT_EQ("bof60", interpolant->step());
    EXPECT_EQ("bof60", interpolant->step());
    EXPECT_EQ("zmag", interpolant->step());
}

TEST(TestInterpolant, insideMethodStep)
{
    const int num_steps{4};
    ParFile::InterpolantPtr interpolant{
        create_interpolant(inside_metadata("inside"), "bof60", "zmag", ParFile::Curve::STEP, num_steps)};

    EXPECT_EQ("bof60", interpolant->step());
    EXPECT_EQ("bof60", interpolant->step());
    EXPECT_EQ("bof60", interpolant->step());
    EXPECT_EQ("zmag", interpolant->step());
}

TEST(TestInterpolant, outsideMethodHold)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{create_interpolant(outside_metadata("outside"), "real", "tdis", num_steps)};

    EXPECT_EQ("real", interpolant->step());
    EXPECT_EQ("real", interpolant->step());
    EXPECT_EQ("tdis", interpolant->step());
}

TEST(TestInterpolant, outsideMethodStep)
{
    const int num_steps{4};
    ParFile::InterpolantPtr interpolant{
        create_interpolant(outside_metadata("outside"), "real", "tdis", ParFile::Curve::STEP, num_steps)};

    EXPECT_EQ("real", interpolant->step());
    EXPECT_EQ("real", interpolant->step());
    EXPECT_EQ("real", interpolant->step());
    EXPECT_EQ("tdis", interpolant->step());
}

TEST(TestInterpolant, insidePwmMixOneEmitsB)
{
    const int num_steps{4};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant(
        resolved_pwm_track(inside_metadata("inside"), "bof60", "zmag", 2, 1.0, 1.0, num_steps), num_steps)};

    EXPECT_EQ("zmag", interpolant->step());
    EXPECT_EQ("zmag", interpolant->step());
    EXPECT_EQ("zmag", interpolant->step());
    EXPECT_EQ("zmag", interpolant->step());
}

TEST(TestInterpolant, outsidePwmWindowBelowTwoRejected)
{
    const int num_steps{4};

    EXPECT_THROW(
        ParFile::create_interpolant(
            resolved_pwm_track(outside_metadata("outside"), "real", "tdis", 1, 0.0, 1.0, num_steps), num_steps),
        std::runtime_error);
}

TEST(TestInterpolant, insideColorIndexHold)
{
    const int num_steps{3};
    ParFile::InterpolantPtr interpolant{create_interpolant(inside_metadata("inside"), "0", "255", num_steps)};

    EXPECT_EQ("0", interpolant->step());
    EXPECT_EQ("0", interpolant->step());
    EXPECT_EQ("255", interpolant->step());
}

TEST(TestInterpolant, insideUnknownStringRejected)
{
    const int num_steps{3};

    EXPECT_THROW(create_interpolant(inside_metadata("inside"), "bof60", "unknown", num_steps), std::runtime_error);
}

TEST(TestInterpolant, insideColorIndexOutOfRangeRejected)
{
    const int num_steps{3};

    EXPECT_THROW(create_interpolant(inside_metadata("inside"), "0", "256", num_steps), std::runtime_error);
}

TEST(TestInterpolant, insideLinearCurveRejected)
{
    const int num_steps{3};
    const ParFile::ParameterMetadata parameter_metadata{inside_metadata("inside")};

    EXPECT_THROW(
        ParFile::create_interpolant(
            resolved_track(parameter_metadata, keyframes("bof60", "zmag", ParFile::Curve::LINEAR, num_steps), "bof60"),
            num_steps),
        std::runtime_error);
}

TEST(TestInterpolant, paramsComplexInterpolatesSlashPair)
{
    const int num_steps{3};
    const std::vector<ParFile::KeyframeConfig> keys{{0, "0/1"}, {2, "2/3"}};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant(
        resolved_params_track("params.c", ParFile::ParameterType::COMPLEX, keys, "0/1/52", {0, 1}), num_steps)};

    ASSERT_TRUE(interpolant);
    EXPECT_EQ("params", interpolant->name());
    ASSERT_EQ(2U, interpolant->output_slots().size());
    EXPECT_EQ(0, interpolant->output_slots()[0]);
    EXPECT_EQ(1, interpolant->output_slots()[1]);
    EXPECT_EQ("0/1/52", interpolant->step());
    EXPECT_EQ("1/2/52", interpolant->step());
    EXPECT_EQ("2/3/52", interpolant->step());
}

TEST(TestInterpolant, paramsSlotPreservesOtherSlots)
{
    const int num_steps{3};
    const std::vector<ParFile::KeyframeConfig> keys{{0, "2"}, {2, "4"}};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant(
        resolved_params_track("params[0]", ParFile::ParameterType::DOUBLE, keys, "0/1", {0}), num_steps)};

    ASSERT_TRUE(interpolant);
    EXPECT_EQ("params", interpolant->name());
    ASSERT_EQ(1U, interpolant->output_slots().size());
    EXPECT_EQ(0, interpolant->output_slots()[0]);
    EXPECT_EQ("2/1", interpolant->step());
    EXPECT_EQ("3/1", interpolant->step());
    EXPECT_EQ("4/1", interpolant->step());
}

TEST(TestInterpolant, paramsIntegerInterpolatesAndRoundsSlot)
{
    const int num_steps{4};
    const std::vector<ParFile::KeyframeConfig> keys{{0, "0"}, {3, "2"}};
    ParFile::InterpolantPtr interpolant{ParFile::create_interpolant(
        resolved_params_track("MandelbrotMix4.iterations", ParFile::ParameterType::INTEGER, keys, "0/1/0", {2}),
        num_steps)};

    ASSERT_TRUE(interpolant);
    EXPECT_EQ("params", interpolant->name());
    EXPECT_EQ("0/1/0", interpolant->step());
    EXPECT_EQ("0/1/1", interpolant->step());
    EXPECT_EQ("0/1/1", interpolant->step());
    EXPECT_EQ("0/1/2", interpolant->step());
}

TEST(TestInterpolant, formulaFunctionAcceptsLegalIdFunctions)
{
    const int num_steps{3};
    const std::vector<ParFile::KeyframeConfig> keys{{0, "tan"}, {2, "log"}};
    ParFile::InterpolantPtr interpolant{
        ParFile::create_interpolant(resolved_function_track("MandelbrotMix4.fn2", keys, "sin/cos", 1), num_steps)};

    ASSERT_TRUE(interpolant);
    EXPECT_EQ("function", interpolant->name());
    ASSERT_EQ(1U, interpolant->output_slots().size());
    EXPECT_EQ(1, interpolant->output_slots()[0]);
    EXPECT_EQ("sin/tan", interpolant->step());
    EXPECT_EQ("sin/tan", interpolant->step());
    EXPECT_EQ("sin/log", interpolant->step());
}

TEST(TestInterpolant, formulaFunctionRejectsUnknownIdFunction)
{
    const int num_steps{3};
    const std::vector<ParFile::KeyframeConfig> keys{{0, "tan"}, {2, "unknown"}};

    EXPECT_THROW(
        ParFile::create_interpolant(resolved_function_track("MandelbrotMix4.fn2", keys, "sin/cos", 1), num_steps),
        std::runtime_error);
}
