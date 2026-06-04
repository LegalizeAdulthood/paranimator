// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/ResolvedAnimation.h>

#include <gtest/gtest.h>

#include <stdexcept>
#include <string>
#include <string_view>

namespace
{

std::vector<std::string> id_functions()
{
    return {"sin", "cos", "tan", "cotan", "sinh", "cosh", "tanh", "cotanh", "exp", "log", "sqr", "recip", "ident",
        "cosxx", "flip", "conj", "zero", "one", "asin", "asinh", "acos", "acosh", "atan", "atanh", "sqrt", "abs",
        "cabs", "floor", "ceil", "trunc", "round"};
}

ParFile::ParameterCatalog catalog_data()
{
    ParFile::ParameterCatalog result{
        {{"center-mag", ParFile::ParameterType::CENTER_MAG, ParFile::ParameterFormat::SLASH, ParFile::Curve::GEOMETRIC,
             ParFile::ExtrapolateMode::CLAMP, {}, {}},
            {"corners", ParFile::ParameterType::CORNERS, ParFile::ParameterFormat::SLASH, ParFile::Curve::LINEAR,
                ParFile::ExtrapolateMode::CLAMP, {}, {}},
            {"rotation", ParFile::ParameterType::NUMERIC_TUPLE, ParFile::ParameterFormat::SLASH, ParFile::Curve::LINEAR,
                ParFile::ExtrapolateMode::CLAMP, {}, {}, {}, 3},
            {"perspective", ParFile::ParameterType::INTEGER, ParFile::ParameterFormat::RAW, ParFile::Curve::LINEAR,
                ParFile::ExtrapolateMode::CLAMP, {}, {}},
            {"xyshift", ParFile::ParameterType::NUMERIC_TUPLE, ParFile::ParameterFormat::SLASH, ParFile::Curve::LINEAR,
                ParFile::ExtrapolateMode::CLAMP, {}, {}, {}, 2},
            {"scalexyz", ParFile::ParameterType::NUMERIC_TUPLE, ParFile::ParameterFormat::SLASH, ParFile::Curve::LINEAR,
                ParFile::ExtrapolateMode::CLAMP, {}, {}, {}, 3},
            {"roughness", ParFile::ParameterType::INTEGER, ParFile::ParameterFormat::RAW, ParFile::Curve::LINEAR,
                ParFile::ExtrapolateMode::CLAMP, {}, {}},
            {"sphere", ParFile::ParameterType::ENUM, ParFile::ParameterFormat::RAW, ParFile::Curve::HOLD,
                ParFile::ExtrapolateMode::CLAMP, {}, {}, {"yes", "no", "y", "n"}},
            {"longitude", ParFile::ParameterType::NUMERIC_TUPLE, ParFile::ParameterFormat::SLASH,
                ParFile::Curve::LINEAR, ParFile::ExtrapolateMode::CLAMP, {}, {}, {}, 2},
            {"latitude", ParFile::ParameterType::NUMERIC_TUPLE, ParFile::ParameterFormat::SLASH, ParFile::Curve::LINEAR,
                ParFile::ExtrapolateMode::CLAMP, {}, {}, {}, 2},
            {"radius", ParFile::ParameterType::INTEGER, ParFile::ParameterFormat::RAW, ParFile::Curve::LINEAR,
                ParFile::ExtrapolateMode::CLAMP, 0.0, {}},
            {"stereo", ParFile::ParameterType::INTEGER, ParFile::ParameterFormat::RAW, ParFile::Curve::LINEAR,
                ParFile::ExtrapolateMode::CLAMP, 0.0, 4.0},
            {"interocular", ParFile::ParameterType::INTEGER, ParFile::ParameterFormat::RAW, ParFile::Curve::LINEAR,
                ParFile::ExtrapolateMode::CLAMP, {}, {}},
            {"converge", ParFile::ParameterType::INTEGER, ParFile::ParameterFormat::RAW, ParFile::Curve::LINEAR,
                ParFile::ExtrapolateMode::CLAMP, {}, {}},
            {"3dmode", ParFile::ParameterType::ENUM, ParFile::ParameterFormat::RAW, ParFile::Curve::HOLD,
                ParFile::ExtrapolateMode::CLAMP, {}, {}, {"monocular", "lefteye", "righteye", "red-blue"}},
            {"julibrot3d", ParFile::ParameterType::NUMERIC_TUPLE, ParFile::ParameterFormat::SLASH,
                ParFile::Curve::LINEAR, ParFile::ExtrapolateMode::CLAMP, {}, {}, {}, 6},
            {"julibroteyes", ParFile::ParameterType::DOUBLE, ParFile::ParameterFormat::RAW, ParFile::Curve::LINEAR,
                ParFile::ExtrapolateMode::CLAMP, {}, {}},
            {"julibrotfromto", ParFile::ParameterType::NUMERIC_TUPLE, ParFile::ParameterFormat::SLASH,
                ParFile::Curve::LINEAR, ParFile::ExtrapolateMode::CLAMP, {}, {}, {}, 4},
            {"maxiter", ParFile::ParameterType::INTEGER, ParFile::ParameterFormat::RAW, ParFile::Curve::LINEAR,
                ParFile::ExtrapolateMode::CLAMP, {}, {}}}};
    result.fractal_types.push_back({"julia",
        {{{0, "c-real",
              {"params[0]", ParFile::ParameterType::DOUBLE, ParFile::ParameterFormat::RAW, ParFile::Curve::LINEAR,
                  ParFile::ExtrapolateMode::CLAMP, {}, {}}},
             {1, "c-imag",
                 {"params[1]", ParFile::ParameterType::DOUBLE, ParFile::ParameterFormat::RAW, ParFile::Curve::LINEAR,
                     ParFile::ExtrapolateMode::CLAMP, {}, {}}}},
            {{"c",
                {"params.c", ParFile::ParameterType::COMPLEX, ParFile::ParameterFormat::SLASH_PAIR,
                    ParFile::Curve::LINEAR, ParFile::ExtrapolateMode::CLAMP, {}, {}},
                {0, 1}}}}});
    result.formula_entries.push_back({"MandelbrotMix4",
        {{{"bailout",
              {"MandelbrotMix4.bailout", ParFile::ParameterType::DOUBLE, ParFile::ParameterFormat::RAW,
                  ParFile::Curve::LINEAR, ParFile::ExtrapolateMode::CLAMP, {}, {}},
              {0}},
            {"bailout-copy",
                {"MandelbrotMix4.bailout-copy", ParFile::ParameterType::DOUBLE, ParFile::ParameterFormat::RAW,
                    ParFile::Curve::LINEAR, ParFile::ExtrapolateMode::CLAMP, {}, {}},
                {0}},
            {"scale factor",
                {"MandelbrotMix4.scale factor", ParFile::ParameterType::DOUBLE, ParFile::ParameterFormat::RAW,
                    ParFile::Curve::LINEAR, ParFile::ExtrapolateMode::CLAMP, {}, {}},
                {1}},
            {"c",
                {"MandelbrotMix4.c", ParFile::ParameterType::COMPLEX, ParFile::ParameterFormat::SLASH_PAIR,
                    ParFile::Curve::LINEAR, ParFile::ExtrapolateMode::CLAMP, {}, {}},
                {2, 3}}}},
        {{{"fn1",
              {"MandelbrotMix4.fn1", ParFile::ParameterType::ENUM, ParFile::ParameterFormat::RAW, ParFile::Curve::HOLD,
                  ParFile::ExtrapolateMode::CLAMP, {}, {}, id_functions()},
              0},
            {"fn2",
                {"MandelbrotMix4.fn2", ParFile::ParameterType::ENUM, ParFile::ParameterFormat::RAW,
                    ParFile::Curve::HOLD, ParFile::ExtrapolateMode::CLAMP, {}, {}, id_functions()},
                1}}}});
    return result;
}

ParFile::Config config_data()
{
    return {{}, {"source.par", "source"}, {"out", "frames.par", "frame-%04d", "frames.bat"}, 1, "F6", 3,
        {{"center-mag", {{0, "-0.5/0/1"}, {2, "-0.5/0/10"}}}}};
}

ParFile::ParSet source_set()
{
    return {"source", {{"center-mag", "-0.5/0/1"}, {"corners", "-3/-1/-2/2"}, {"maxiter", "100"}}};
}

ParFile::Config julia_config_data(std::string_view parameter)
{
    ParFile::Config result{config_data()};
    result.tracks = {{std::string{parameter}, {{0, "0/1"}, {2, "2/3"}}}};
    return result;
}

ParFile::ParSet julia_source_set()
{
    return {"source", {{"type", "julia"}, {"params", "0/1/52"}}};
}

ParFile::Config formula_config_data(std::string_view parameter)
{
    ParFile::Config result{config_data()};
    result.tracks = {{std::string{parameter}, {{0, "10"}, {2, "20"}}}};
    return result;
}

ParFile::Config camera2d_config_data(std::string_view output = "corners")
{
    ParFile::Camera2DConfig camera;
    camera.name = "camera";
    camera.output = output;
    camera.aspect = "source";
    camera.look_at = {ParFile::ParameterType::POINT2, false, {{0, "0/0"}, {2, "1/1"}}};
    camera.view_up = ParFile::Camera2DValueTrackConfig{ParFile::ParameterType::VECTOR2, true, {{0, "0/2"}, {2, "1/1"}}};
    camera.height = {ParFile::ParameterType::DOUBLE, false, {{0, "4"}, {2, "2", ParFile::Curve::GEOMETRIC}}};

    ParFile::TrackConfig track;
    track.parameter = "camera";
    track.kind = ParFile::TrackKind::CAMERA2D;
    track.camera2d = camera;

    ParFile::Config result{config_data()};
    result.tracks = {track};
    return result;
}

ParFile::Config id_3d_view_config_data()
{
    ParFile::Id3DViewConfig view;
    view.name = "view";
    view.outputs.rotation = "rotation";
    view.outputs.perspective = "perspective";
    view.outputs.xyshift = "xyshift";
    view.rotation =
        ParFile::Id3DViewValueTrackConfig{ParFile::ParameterType::NUMERIC_TUPLE, 3, {{0, "60/30/0"}, {2, "70/50/10"}}};
    view.perspective = ParFile::Id3DViewValueTrackConfig{ParFile::ParameterType::INTEGER, {}, {{0, "0"}, {2, "100"}}};
    view.xyshift =
        ParFile::Id3DViewValueTrackConfig{ParFile::ParameterType::NUMERIC_TUPLE, 2, {{0, "0/0"}, {2, "20/-10"}}};

    ParFile::TrackConfig track;
    track.parameter = "view";
    track.kind = ParFile::TrackKind::ID_3D_VIEW;
    track.id_3d_view = view;

    ParFile::Config result{config_data()};
    result.tracks = {track};
    return result;
}

ParFile::Config id_3d_view_more_config_data()
{
    ParFile::Id3DViewConfig view;
    view.name = "view";
    view.outputs.scalexyz = "scalexyz";
    view.outputs.stereo = "stereo";
    view.scalexyz = ParFile::Id3DViewValueTrackConfig{
        ParFile::ParameterType::NUMERIC_TUPLE, 3, {{0, "90/90/30"}, {2, "100/100/40"}}};
    view.stereo = ParFile::Id3DViewValueTrackConfig{ParFile::ParameterType::INTEGER, {}, {{0, "0"}, {2, "2"}}};

    ParFile::TrackConfig track;
    track.parameter = "view";
    track.kind = ParFile::TrackKind::ID_3D_VIEW;
    track.id_3d_view = view;

    ParFile::Config result{config_data()};
    result.tracks = {track};
    return result;
}

ParFile::Camera3DConfig camera3d_config_data(std::string_view eye_to = "10/0/0")
{
    ParFile::Camera3DConfig camera;
    camera.eye = ParFile::Camera3DValueTrackConfig{
        ParFile::ParameterType::POINT3, false, {{0, "0/0/24"}, {2, std::string{eye_to}}}};
    camera.look_at =
        ParFile::Camera3DValueTrackConfig{ParFile::ParameterType::POINT3, false, {{0, "0/0/0"}, {2, "0/0/0"}}};
    camera.view_up =
        ParFile::Camera3DValueTrackConfig{ParFile::ParameterType::VECTOR3, true, {{0, "0/2/0"}, {2, "0/2/0"}}};
    return camera;
}

ParFile::Config id_3d_view_camera_config_data()
{
    ParFile::Id3DViewConfig view;
    view.name = "view";
    view.outputs.rotation = "rotation";
    view.outputs.perspective = "perspective";
    view.outputs.xyshift = "xyshift";
    view.camera3d = camera3d_config_data();

    ParFile::TrackConfig track;
    track.parameter = "view";
    track.kind = ParFile::TrackKind::ID_3D_VIEW;
    track.id_3d_view = view;

    ParFile::Config result{config_data()};
    result.tracks = {track};
    return result;
}

ParFile::Config julibrot_view_config_data()
{
    ParFile::JulibrotViewConfig view;
    view.name = "view";
    view.outputs.mode = "3dmode";
    view.outputs.geometry = "julibrot3d";
    view.outputs.eyes = "julibroteyes";
    view.outputs.from_to = "julibrotfromto";
    view.mode =
        ParFile::JulibrotViewValueTrackConfig{ParFile::ParameterType::ENUM, {}, {{0, "monocular"}, {2, "lefteye"}}};
    view.geometry = ParFile::JulibrotViewValueTrackConfig{
        ParFile::ParameterType::NUMERIC_TUPLE, 6, {{0, "128/8/8/7/10/24"}, {2, "160/7/6/6/9/20"}}};
    view.eyes = ParFile::JulibrotViewValueTrackConfig{ParFile::ParameterType::DOUBLE, {}, {{0, "2.5"}, {2, "1"}}};
    view.from_to = ParFile::JulibrotViewValueTrackConfig{
        ParFile::ParameterType::NUMERIC_TUPLE, 4, {{0, "-0.83/-0.83/0.25/-0.25"}, {2, "-0.7/-0.9/0.2/-0.2"}}};

    ParFile::TrackConfig track;
    track.parameter = "view";
    track.kind = ParFile::TrackKind::JULIBROT_VIEW;
    track.julibrot_view = view;

    ParFile::Config result{config_data()};
    result.tracks = {track};
    return result;
}

ParFile::Config julibrot_view_camera_config_data()
{
    ParFile::JulibrotViewConfig view;
    view.name = "view";
    view.outputs.geometry = "julibrot3d";
    view.camera3d = camera3d_config_data("0/0/12");

    ParFile::TrackConfig track;
    track.parameter = "view";
    track.kind = ParFile::TrackKind::JULIBROT_VIEW;
    track.julibrot_view = view;

    ParFile::Config result{config_data()};
    result.tracks = {track};
    return result;
}

ParFile::ParSet julibrot_source_set()
{
    ParFile::ParSet result{source_set()};
    result.params.push_back({"julibrot3d", "128/8/8/7/10/24"});
    return result;
}

ParFile::ParSet formula_source_set()
{
    return {"source",
        {{"type", "formula"}, {"formulaname", "MandelbrotMix4"}, {"function", "sin/cos"},
            {"params", "0.05/3/-1.5/-2/0/0"}}};
}

} // namespace

TEST(TestResolvedAnimation, unknownAnimatedParameterRejected)
{
    ParFile::Config config{config_data()};
    config.tracks[0].parameter = "unknown";

    EXPECT_THROW(ParFile::resolve_animation(config, catalog_data(), source_set()), std::runtime_error);
}

TEST(TestResolvedAnimation, missingSourceParameterRejected)
{
    ParFile::ParSet source{source_set()};
    source.params.erase(source.params.begin());

    EXPECT_THROW(ParFile::resolve_animation(config_data(), catalog_data(), source), std::runtime_error);
}

TEST(TestResolvedAnimation, resolvedTrackCarriesParameterMetadataBaseValueAndKeys)
{
    const ParFile::ResolvedAnimation animation{ParFile::resolve_animation(config_data(), catalog_data(), source_set())};

    EXPECT_EQ("frame-%04d", animation.frame_name);
    EXPECT_EQ("F6", animation.video);
    EXPECT_EQ(3, animation.num_frames);
    EXPECT_EQ("source", animation.source.name);
    ASSERT_EQ(1U, animation.tracks.size());

    const ParFile::ResolvedTrack &track{animation.tracks[0]};
    EXPECT_EQ("center-mag", track.parameter);
    EXPECT_EQ("center-mag", track.metadata.name);
    EXPECT_EQ(ParFile::ParameterType::CENTER_MAG, track.metadata.type);
    EXPECT_EQ("-0.5/0/1", track.base_value);
    ASSERT_EQ(2U, track.keys.size());
    EXPECT_EQ(0, track.keys[0].frame);
    EXPECT_EQ("-0.5/0/1", track.keys[0].value);
    EXPECT_EQ(2, track.keys[1].frame);
    EXPECT_EQ("-0.5/0/10", track.keys[1].value);
}

TEST(TestResolvedAnimation, camera2dResolvesOutputMetadataAndSourceAspect)
{
    const ParFile::ResolvedAnimation animation{
        ParFile::resolve_animation(camera2d_config_data(), catalog_data(), source_set())};

    ASSERT_EQ(1U, animation.tracks.size());
    const ParFile::ResolvedTrack &track{animation.tracks[0]};
    EXPECT_EQ("camera", track.parameter);
    EXPECT_EQ("corners", track.output_parameter);
    EXPECT_EQ(ParFile::ParameterType::CORNERS, track.metadata.type);
    EXPECT_EQ("-3/-1/-2/2", track.base_value);
    ASSERT_TRUE(track.camera2d);
    EXPECT_DOUBLE_EQ(0.5, track.camera2d->aspect);
    EXPECT_EQ("camera.look-at", track.camera2d->look_at.metadata.name);
    EXPECT_EQ(ParFile::ParameterType::POINT2, track.camera2d->look_at.metadata.type);
    ASSERT_TRUE(track.camera2d->view_up);
    EXPECT_EQ(ParFile::ParameterType::VECTOR2, track.camera2d->view_up->metadata.type);
    EXPECT_TRUE(track.camera2d->view_up->metadata.normalize);
    EXPECT_EQ(ParFile::ParameterType::DOUBLE, track.camera2d->height.metadata.type);
}

TEST(TestResolvedAnimation, camera2dSourceAspectUsesIdSixValueCorners)
{
    ParFile::ParSet source{source_set()};
    source.params[1].value = "-1/1/-2/2/-1/-2";

    const ParFile::ResolvedAnimation animation{
        ParFile::resolve_animation(camera2d_config_data(), catalog_data(), source)};

    ASSERT_EQ(1U, animation.tracks.size());
    ASSERT_TRUE(animation.tracks[0].camera2d);
    EXPECT_DOUBLE_EQ(0.5, animation.tracks[0].camera2d->aspect);
}

TEST(TestResolvedAnimation, camera2dResolvesEyeTrack)
{
    ParFile::Config config{camera2d_config_data("center-mag")};
    ASSERT_TRUE(config.tracks[0].camera2d);
    config.tracks[0].camera2d->view_up.reset();
    config.tracks[0].camera2d->eye =
        ParFile::Camera2DValueTrackConfig{ParFile::ParameterType::POINT2, false, {{0, "1/0"}, {2, "0/1"}}};

    const ParFile::ResolvedAnimation animation{ParFile::resolve_animation(config, catalog_data(), source_set())};

    ASSERT_EQ(1U, animation.tracks.size());
    const ParFile::ResolvedTrack &track{animation.tracks[0]};
    ASSERT_TRUE(track.camera2d);
    EXPECT_FALSE(track.camera2d->view_up);
    ASSERT_TRUE(track.camera2d->eye);
    EXPECT_EQ("camera.eye", track.camera2d->eye->metadata.name);
    EXPECT_EQ(ParFile::ParameterType::POINT2, track.camera2d->eye->metadata.type);
    EXPECT_FALSE(track.camera2d->eye->metadata.normalize);
    ASSERT_EQ(2U, track.camera2d->eye->keys.size());
    EXPECT_EQ("1/0", track.camera2d->eye->keys[0].value);
}

TEST(TestResolvedAnimation, camera2dResolvesSkewTrack)
{
    ParFile::Config config{camera2d_config_data("center-mag")};
    ASSERT_TRUE(config.tracks[0].camera2d);
    config.tracks[0].camera2d->skew =
        ParFile::Camera2DValueTrackConfig{ParFile::ParameterType::DOUBLE, false, {{0, "0"}, {2, "10"}}};

    const ParFile::ResolvedAnimation animation{ParFile::resolve_animation(config, catalog_data(), source_set())};

    ASSERT_EQ(1U, animation.tracks.size());
    const ParFile::ResolvedTrack &track{animation.tracks[0]};
    ASSERT_TRUE(track.camera2d);
    ASSERT_TRUE(track.camera2d->skew);
    EXPECT_EQ("camera.skew", track.camera2d->skew->metadata.name);
    EXPECT_EQ(ParFile::ParameterType::DOUBLE, track.camera2d->skew->metadata.type);
    ASSERT_EQ(2U, track.camera2d->skew->keys.size());
    EXPECT_EQ("0", track.camera2d->skew->keys[0].value);
    EXPECT_EQ("10", track.camera2d->skew->keys[1].value);
}

TEST(TestResolvedAnimation, camera2dResolvesCenterMagOutputAndVideoAspect)
{
    const ParFile::ResolvedAnimation animation{
        ParFile::resolve_animation(camera2d_config_data("center-mag"), catalog_data(), source_set())};

    ASSERT_EQ(1U, animation.tracks.size());
    const ParFile::ResolvedTrack &track{animation.tracks[0]};
    EXPECT_EQ("camera", track.parameter);
    EXPECT_EQ("center-mag", track.output_parameter);
    EXPECT_EQ(ParFile::ParameterType::CENTER_MAG, track.metadata.type);
    EXPECT_EQ("-0.5/0/1", track.base_value);
    ASSERT_TRUE(track.camera2d);
    EXPECT_DOUBLE_EQ(4.0 / 3.0, track.camera2d->aspect);
    EXPECT_DOUBLE_EQ(1.0, track.camera2d->center_mag_x_mag_factor);
}

TEST(TestResolvedAnimation, camera2dCenterMagAspectUsesSourceXMagFactor)
{
    ParFile::ParSet source{source_set()};
    source.params[0].value = "-0.5/0/1/2";
    const ParFile::ResolvedAnimation animation{
        ParFile::resolve_animation(camera2d_config_data("center-mag"), catalog_data(), source)};

    ASSERT_EQ(1U, animation.tracks.size());
    const ParFile::ResolvedTrack &track{animation.tracks[0]};
    ASSERT_TRUE(track.camera2d);
    EXPECT_DOUBLE_EQ(2.0 / 3.0, track.camera2d->aspect);
    EXPECT_DOUBLE_EQ(2.0, track.camera2d->center_mag_x_mag_factor);
}

TEST(TestResolvedAnimation, camera2dCenterMagRejectsUnknownVideoShape)
{
    ParFile::Config config{camera2d_config_data("center-mag")};
    config.video = "unknown";

    EXPECT_THROW(ParFile::resolve_animation(config, catalog_data(), source_set()), std::runtime_error);
}

TEST(TestResolvedAnimation, id3DViewResolvesToOutputTracks)
{
    const ParFile::ResolvedAnimation animation{
        ParFile::resolve_animation(id_3d_view_config_data(), catalog_data(), source_set())};

    ASSERT_EQ(3U, animation.tracks.size());
    EXPECT_EQ("view.rotation", animation.tracks[0].parameter);
    EXPECT_EQ("rotation", animation.tracks[0].metadata.name);
    EXPECT_EQ(ParFile::ParameterType::NUMERIC_TUPLE, animation.tracks[0].metadata.type);
    EXPECT_EQ("rotation", animation.tracks[0].output_parameter);
    EXPECT_EQ("60/30/0", animation.tracks[0].base_value);
    EXPECT_EQ("view.perspective", animation.tracks[1].parameter);
    EXPECT_EQ("perspective", animation.tracks[1].output_parameter);
    EXPECT_EQ(ParFile::ParameterType::INTEGER, animation.tracks[1].metadata.type);
    EXPECT_EQ("0", animation.tracks[1].base_value);
    EXPECT_EQ("view.xyshift", animation.tracks[2].parameter);
    EXPECT_EQ("xyshift", animation.tracks[2].output_parameter);
    EXPECT_EQ(ParFile::ParameterType::NUMERIC_TUPLE, animation.tracks[2].metadata.type);
    EXPECT_EQ("0/0", animation.tracks[2].base_value);
}

TEST(TestResolvedAnimation, id3DViewResolvesAdditionalOutputTracks)
{
    const ParFile::ResolvedAnimation animation{
        ParFile::resolve_animation(id_3d_view_more_config_data(), catalog_data(), source_set())};

    ASSERT_EQ(2U, animation.tracks.size());
    EXPECT_EQ("view.scalexyz", animation.tracks[0].parameter);
    EXPECT_EQ("scalexyz", animation.tracks[0].output_parameter);
    EXPECT_EQ(ParFile::ParameterType::NUMERIC_TUPLE, animation.tracks[0].metadata.type);
    ASSERT_TRUE(animation.tracks[0].metadata.arity);
    EXPECT_EQ(3, *animation.tracks[0].metadata.arity);
    EXPECT_EQ("90/90/30", animation.tracks[0].base_value);
    EXPECT_EQ("view.stereo", animation.tracks[1].parameter);
    EXPECT_EQ("stereo", animation.tracks[1].output_parameter);
    EXPECT_EQ(ParFile::ParameterType::INTEGER, animation.tracks[1].metadata.type);
    ASSERT_TRUE(animation.tracks[1].metadata.max);
    EXPECT_EQ(4.0, *animation.tracks[1].metadata.max);
}

TEST(TestResolvedAnimation, id3DViewCameraResolvesGeneratedOutputs)
{
    const ParFile::ResolvedAnimation animation{
        ParFile::resolve_animation(id_3d_view_camera_config_data(), catalog_data(), source_set())};

    ASSERT_EQ(3U, animation.tracks.size());
    EXPECT_EQ("view.rotation", animation.tracks[0].parameter);
    EXPECT_EQ("rotation", animation.tracks[0].output_parameter);
    ASSERT_TRUE(animation.tracks[0].camera3d);
    EXPECT_EQ(ParFile::Camera3DOutputKind::ID_ROTATION, animation.tracks[0].camera3d->output_kind);
    EXPECT_EQ("view.camera3d.eye", animation.tracks[0].camera3d->eye.metadata.name);
    EXPECT_EQ(ParFile::ParameterType::POINT3, animation.tracks[0].camera3d->eye.metadata.type);
    EXPECT_EQ("0/0/24", animation.tracks[0].camera3d->eye.keys[0].value);
    EXPECT_EQ("view.perspective", animation.tracks[1].parameter);
    EXPECT_EQ("perspective", animation.tracks[1].output_parameter);
    ASSERT_TRUE(animation.tracks[1].camera3d);
    EXPECT_EQ(ParFile::Camera3DOutputKind::ID_PERSPECTIVE, animation.tracks[1].camera3d->output_kind);
    EXPECT_EQ("view.xyshift", animation.tracks[2].parameter);
    EXPECT_EQ("xyshift", animation.tracks[2].output_parameter);
    ASSERT_TRUE(animation.tracks[2].camera3d);
    EXPECT_EQ(ParFile::Camera3DOutputKind::ID_XYSHIFT, animation.tracks[2].camera3d->output_kind);
}

TEST(TestResolvedAnimation, id3DViewRejectsWrongOutputMetadata)
{
    ParFile::Config config{id_3d_view_more_config_data()};
    config.tracks[0].id_3d_view->outputs.scalexyz = "maxiter";

    EXPECT_THROW(ParFile::resolve_animation(config, catalog_data(), source_set()), std::runtime_error);
}

TEST(TestResolvedAnimation, julibrotViewResolvesToOutputTracks)
{
    const ParFile::ResolvedAnimation animation{
        ParFile::resolve_animation(julibrot_view_config_data(), catalog_data(), source_set())};

    ASSERT_EQ(4U, animation.tracks.size());
    EXPECT_EQ("view.mode", animation.tracks[0].parameter);
    EXPECT_EQ("3dmode", animation.tracks[0].output_parameter);
    EXPECT_EQ(ParFile::ParameterType::ENUM, animation.tracks[0].metadata.type);
    EXPECT_EQ("monocular", animation.tracks[0].base_value);
    EXPECT_EQ("view.geometry", animation.tracks[1].parameter);
    EXPECT_EQ("julibrot3d", animation.tracks[1].output_parameter);
    EXPECT_EQ(ParFile::ParameterType::NUMERIC_TUPLE, animation.tracks[1].metadata.type);
    ASSERT_TRUE(animation.tracks[1].metadata.arity);
    EXPECT_EQ(6, *animation.tracks[1].metadata.arity);
    EXPECT_EQ("128/8/8/7/10/24", animation.tracks[1].base_value);
    EXPECT_EQ("view.eyes", animation.tracks[2].parameter);
    EXPECT_EQ("julibroteyes", animation.tracks[2].output_parameter);
    EXPECT_EQ(ParFile::ParameterType::DOUBLE, animation.tracks[2].metadata.type);
    EXPECT_EQ("2.5", animation.tracks[2].base_value);
    EXPECT_EQ("view.from-to", animation.tracks[3].parameter);
    EXPECT_EQ("julibrotfromto", animation.tracks[3].output_parameter);
    EXPECT_EQ(ParFile::ParameterType::NUMERIC_TUPLE, animation.tracks[3].metadata.type);
    ASSERT_TRUE(animation.tracks[3].metadata.arity);
    EXPECT_EQ(4, *animation.tracks[3].metadata.arity);
}

TEST(TestResolvedAnimation, julibrotViewCameraResolvesGeneratedGeometry)
{
    const ParFile::ResolvedAnimation animation{
        ParFile::resolve_animation(julibrot_view_camera_config_data(), catalog_data(), julibrot_source_set())};

    ASSERT_EQ(1U, animation.tracks.size());
    EXPECT_EQ("view.geometry", animation.tracks[0].parameter);
    EXPECT_EQ("julibrot3d", animation.tracks[0].output_parameter);
    EXPECT_EQ("128/8/8/7/10/24", animation.tracks[0].base_value);
    ASSERT_TRUE(animation.tracks[0].camera3d);
    EXPECT_EQ(ParFile::Camera3DOutputKind::JULIBROT_GEOMETRY, animation.tracks[0].camera3d->output_kind);
    EXPECT_EQ("view.camera3d.view-up", animation.tracks[0].camera3d->view_up.metadata.name);
    EXPECT_TRUE(animation.tracks[0].camera3d->view_up.metadata.normalize);
}

TEST(TestResolvedAnimation, julibrotViewCameraRequiresSourceGeometry)
{
    EXPECT_THROW(ParFile::resolve_animation(julibrot_view_camera_config_data(), catalog_data(), source_set()),
        std::runtime_error);
}

TEST(TestResolvedAnimation, julibrotViewRejectsWrongOutputMetadata)
{
    ParFile::Config config{julibrot_view_config_data()};
    config.tracks[0].julibrot_view->outputs.geometry = "maxiter";

    EXPECT_THROW(ParFile::resolve_animation(config, catalog_data(), source_set()), std::runtime_error);
}

TEST(TestResolvedAnimation, juliaParamsGroupResolvesToParamsSlots)
{
    const ParFile::ResolvedAnimation animation{
        ParFile::resolve_animation(julia_config_data("params.c"), catalog_data(), julia_source_set())};

    ASSERT_EQ(1U, animation.tracks.size());
    const ParFile::ResolvedTrack &track{animation.tracks[0]};
    EXPECT_EQ("params.c", track.parameter);
    EXPECT_EQ("params.c", track.metadata.name);
    EXPECT_EQ(ParFile::ParameterType::COMPLEX, track.metadata.type);
    EXPECT_EQ("0/1/52", track.base_value);
    EXPECT_EQ("params", track.output_parameter);
    ASSERT_EQ(2U, track.slots.size());
    EXPECT_EQ(0, track.slots[0]);
    EXPECT_EQ(1, track.slots[1]);
}

TEST(TestResolvedAnimation, juliaParamsSlotResolvesToParamsSlot)
{
    ParFile::Config config{julia_config_data("params[0]")};
    config.tracks[0].keys = {{0, "2"}, {2, "4"}};
    const ParFile::ResolvedAnimation animation{ParFile::resolve_animation(config, catalog_data(), julia_source_set())};

    ASSERT_EQ(1U, animation.tracks.size());
    const ParFile::ResolvedTrack &track{animation.tracks[0]};
    EXPECT_EQ("params[0]", track.parameter);
    EXPECT_EQ("params[0]", track.metadata.name);
    EXPECT_EQ(ParFile::ParameterType::DOUBLE, track.metadata.type);
    EXPECT_EQ("params", track.output_parameter);
    ASSERT_EQ(1U, track.slots.size());
    EXPECT_EQ(0, track.slots[0]);
}

TEST(TestResolvedAnimation, juliaParamsSlot2Rejected)
{
    EXPECT_THROW(ParFile::resolve_animation(julia_config_data("params[2]"), catalog_data(), julia_source_set()),
        std::runtime_error);
}

TEST(TestResolvedAnimation, formulaParamsKnobResolvesFromActiveFormulaname)
{
    const ParFile::ResolvedAnimation animation{ParFile::resolve_animation(
        formula_config_data("MandelbrotMix4.bailout"), catalog_data(), formula_source_set())};

    ASSERT_EQ(1U, animation.tracks.size());
    const ParFile::ResolvedTrack &track{animation.tracks[0]};
    EXPECT_EQ("MandelbrotMix4.bailout", track.parameter);
    EXPECT_EQ("MandelbrotMix4.bailout", track.metadata.name);
    EXPECT_EQ(ParFile::ParameterType::DOUBLE, track.metadata.type);
    EXPECT_EQ("0.05/3/-1.5/-2/0/0", track.base_value);
    EXPECT_EQ("params", track.output_parameter);
    ASSERT_EQ(1U, track.slots.size());
    EXPECT_EQ(0, track.slots[0]);
}

TEST(TestResolvedAnimation, formulaParamsKnobWithSpacesResolvesFromActiveFormulaname)
{
    const ParFile::ResolvedAnimation animation{ParFile::resolve_animation(
        formula_config_data("MandelbrotMix4[\"scale factor\"]"), catalog_data(), formula_source_set())};

    ASSERT_EQ(1U, animation.tracks.size());
    const ParFile::ResolvedTrack &track{animation.tracks[0]};
    EXPECT_EQ("MandelbrotMix4.scale factor", track.metadata.name);
    ASSERT_EQ(1U, track.slots.size());
    EXPECT_EQ(1, track.slots[0]);
}

TEST(TestResolvedAnimation, formulaParamsComplexKnobResolvesToPNSlots)
{
    ParFile::Config config{formula_config_data("MandelbrotMix4.c")};
    config.tracks[0].keys = {{0, "-1/-2"}, {2, "-3/-4"}};
    const ParFile::ResolvedAnimation animation{
        ParFile::resolve_animation(config, catalog_data(), formula_source_set())};

    ASSERT_EQ(1U, animation.tracks.size());
    const ParFile::ResolvedTrack &track{animation.tracks[0]};
    EXPECT_EQ(ParFile::ParameterType::COMPLEX, track.metadata.type);
    ASSERT_EQ(2U, track.slots.size());
    EXPECT_EQ(2, track.slots[0]);
    EXPECT_EQ(3, track.slots[1]);
}

TEST(TestResolvedAnimation, overlappingFormulaParamsKnobsRejected)
{
    ParFile::Config config{formula_config_data("MandelbrotMix4.bailout")};
    config.tracks.push_back({"MandelbrotMix4.bailout-copy", {{0, "11"}, {2, "21"}}});

    EXPECT_THROW(ParFile::resolve_animation(config, catalog_data(), formula_source_set()), std::runtime_error);
}

TEST(TestResolvedAnimation, formulaFunctionKeyResolvesFromActiveFormulaname)
{
    ParFile::Config config{formula_config_data("MandelbrotMix4.fn2")};
    config.tracks[0].keys = {{0, "tan"}, {2, "log"}};
    const ParFile::ResolvedAnimation animation{
        ParFile::resolve_animation(config, catalog_data(), formula_source_set())};

    ASSERT_EQ(1U, animation.tracks.size());
    const ParFile::ResolvedTrack &track{animation.tracks[0]};
    EXPECT_EQ("MandelbrotMix4.fn2", track.parameter);
    EXPECT_EQ("MandelbrotMix4.fn2", track.metadata.name);
    EXPECT_EQ(ParFile::ParameterType::ENUM, track.metadata.type);
    EXPECT_EQ("sin/cos", track.base_value);
    EXPECT_EQ("function", track.output_parameter);
    ASSERT_EQ(1U, track.slots.size());
    EXPECT_EQ(1, track.slots[0]);
}
