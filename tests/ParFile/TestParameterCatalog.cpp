// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/ParameterCatalog.h>

#include <TestParFile/test.h>

#include <gtest/gtest.h>

#include <algorithm>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>

namespace
{

std::string read_text(const char *path)
{
    std::ifstream in{path};
    return {std::istreambuf_iterator<char>{in}, std::istreambuf_iterator<char>{}};
}

ParFile::ParameterCatalog core_catalog()
{
    return ParFile::read_parameter_catalog(read_text(TestParFile::CORE_CATALOG_JSON));
}

ParFile::ParameterCatalog typed_catalog()
{
    return {{{"center-mag", ParFile::ParameterType::CENTER_MAG, ParFile::ParameterFormat::SLASH,
                 ParFile::Curve::GEOMETRIC, ParFile::ExtrapolateMode::CLAMP, {}, {}},
        {"maxiter", ParFile::ParameterType::INTEGER, ParFile::ParameterFormat::RAW, ParFile::Curve::LINEAR,
            ParFile::ExtrapolateMode::CLAMP, {}, {}}}};
}

std::string catalog_text(std::string_view metadata)
{
    return "{\"parameters\":{\"x\":{" + std::string{metadata} + "}}}";
}

std::string formula_catalog_text(std::string_view knob)
{
    return "{\"parameters\":{},\"formula-entries\":{\"foo\":{\"params\":{\"knobs\":{\"x\":{" + std::string{knob} +
        "}}}}}}";
}

std::string formula_function_catalog_text(std::string_view function)
{
    return "{\"parameters\":{},\"formula-entries\":{\"foo\":{\"functions\":{\"fn1\":{" + std::string{function} +
        "}}}}}";
}

ParFile::ParameterMetadata read_metadata(std::string_view metadata)
{
    return ParFile::read_parameter_catalog(catalog_text(metadata)).metadata("x");
}

} // namespace

TEST(TestParameterCatalog, validCatalogJsonDeserializesAllCoreParameters)
{
    const ParFile::ParameterCatalog catalog{core_catalog()};

    EXPECT_EQ(7U, catalog.parameters.size());
    EXPECT_EQ(1U, catalog.fractal_types.size());
    EXPECT_EQ(1U, catalog.formula_entries.size());
}

TEST(TestParameterCatalog, centerMagMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{core_catalog()};
    const ParFile::ParameterMetadata &metadata{catalog.metadata("center-mag")};

    EXPECT_EQ("center-mag", metadata.name);
    EXPECT_EQ(ParFile::ParameterType::CENTER_MAG, metadata.type);
    ASSERT_TRUE(metadata.format);
    EXPECT_EQ(ParFile::ParameterFormat::SLASH, *metadata.format);
    ASSERT_TRUE(metadata.default_curve);
    EXPECT_EQ(ParFile::Curve::GEOMETRIC, *metadata.default_curve);
    ASSERT_TRUE(metadata.extrapolate);
    EXPECT_EQ(ParFile::ExtrapolateMode::CLAMP, *metadata.extrapolate);
}

TEST(TestParameterCatalog, cornersMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{core_catalog()};
    const ParFile::ParameterMetadata &metadata{catalog.metadata("corners")};

    EXPECT_EQ("corners", metadata.name);
    EXPECT_EQ(ParFile::ParameterType::CORNERS, metadata.type);
    ASSERT_TRUE(metadata.format);
    EXPECT_EQ(ParFile::ParameterFormat::SLASH, *metadata.format);
    ASSERT_TRUE(metadata.default_curve);
    EXPECT_EQ(ParFile::Curve::LINEAR, *metadata.default_curve);
    ASSERT_TRUE(metadata.extrapolate);
    EXPECT_EQ(ParFile::ExtrapolateMode::CLAMP, *metadata.extrapolate);
}

TEST(TestParameterCatalog, xyshiftMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{core_catalog()};
    const ParFile::ParameterMetadata &metadata{catalog.metadata("xyshift")};

    EXPECT_EQ("xyshift", metadata.name);
    EXPECT_EQ(ParFile::ParameterType::NUMERIC_TUPLE, metadata.type);
    ASSERT_TRUE(metadata.format);
    EXPECT_EQ(ParFile::ParameterFormat::SLASH, *metadata.format);
    ASSERT_TRUE(metadata.arity);
    EXPECT_EQ(2, *metadata.arity);
    ASSERT_TRUE(metadata.default_curve);
    EXPECT_EQ(ParFile::Curve::LINEAR, *metadata.default_curve);
    ASSERT_TRUE(metadata.extrapolate);
    EXPECT_EQ(ParFile::ExtrapolateMode::CLAMP, *metadata.extrapolate);
}

TEST(TestParameterCatalog, maxiterMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{core_catalog()};
    const ParFile::ParameterMetadata &metadata{catalog.metadata("maxiter")};

    EXPECT_EQ("maxiter", metadata.name);
    EXPECT_EQ(ParFile::ParameterType::INTEGER, metadata.type);
    ASSERT_TRUE(metadata.format);
    EXPECT_EQ(ParFile::ParameterFormat::RAW, *metadata.format);
    ASSERT_TRUE(metadata.default_curve);
    EXPECT_EQ(ParFile::Curve::LINEAR, *metadata.default_curve);
    ASSERT_TRUE(metadata.extrapolate);
    EXPECT_EQ(ParFile::ExtrapolateMode::CLAMP, *metadata.extrapolate);
}

TEST(TestParameterCatalog, bailoutMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{core_catalog()};
    const ParFile::ParameterMetadata &metadata{catalog.metadata("bailout")};

    EXPECT_EQ("bailout", metadata.name);
    EXPECT_EQ(ParFile::ParameterType::DOUBLE, metadata.type);
    ASSERT_TRUE(metadata.format);
    EXPECT_EQ(ParFile::ParameterFormat::RAW, *metadata.format);
    ASSERT_TRUE(metadata.default_curve);
    EXPECT_EQ(ParFile::Curve::LINEAR, *metadata.default_curve);
    ASSERT_TRUE(metadata.extrapolate);
    EXPECT_EQ(ParFile::ExtrapolateMode::CLAMP, *metadata.extrapolate);
    ASSERT_TRUE(metadata.min);
    ASSERT_TRUE(metadata.max);
    EXPECT_EQ(0, *metadata.min);
    EXPECT_EQ(1000, *metadata.max);
}

TEST(TestParameterCatalog, insideMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{core_catalog()};
    const ParFile::ParameterMetadata &metadata{catalog.metadata("inside")};

    EXPECT_EQ("inside", metadata.name);
    EXPECT_EQ(ParFile::ParameterType::INSIDE, metadata.type);
    ASSERT_TRUE(metadata.format);
    EXPECT_EQ(ParFile::ParameterFormat::RAW, *metadata.format);
    ASSERT_TRUE(metadata.default_curve);
    EXPECT_EQ(ParFile::Curve::HOLD, *metadata.default_curve);
    ASSERT_TRUE(metadata.extrapolate);
    EXPECT_EQ(ParFile::ExtrapolateMode::CLAMP, *metadata.extrapolate);
    ASSERT_TRUE(metadata.min);
    ASSERT_TRUE(metadata.max);
    EXPECT_EQ(0, *metadata.min);
    EXPECT_EQ(255, *metadata.max);
    ASSERT_EQ(9U, metadata.values.size());
    EXPECT_NE(metadata.values.end(), std::find(metadata.values.begin(), metadata.values.end(), "bof60"));
    EXPECT_NE(metadata.values.end(), std::find(metadata.values.begin(), metadata.values.end(), "epsiloncross"));
}

TEST(TestParameterCatalog, outsideMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{core_catalog()};
    const ParFile::ParameterMetadata &metadata{catalog.metadata("outside")};

    EXPECT_EQ("outside", metadata.name);
    EXPECT_EQ(ParFile::ParameterType::OUTSIDE, metadata.type);
    ASSERT_TRUE(metadata.format);
    EXPECT_EQ(ParFile::ParameterFormat::RAW, *metadata.format);
    ASSERT_TRUE(metadata.default_curve);
    EXPECT_EQ(ParFile::Curve::HOLD, *metadata.default_curve);
    ASSERT_EQ(8U, metadata.values.size());
    EXPECT_NE(metadata.values.end(), std::find(metadata.values.begin(), metadata.values.end(), "iter"));
    EXPECT_NE(metadata.values.end(), std::find(metadata.values.begin(), metadata.values.end(), "tdis"));
}

TEST(TestParameterCatalog, typedCatalogFindsMetadataByName)
{
    const ParFile::ParameterCatalog catalog{typed_catalog()};
    const ParFile::ParameterMetadata &metadata{catalog.metadata("maxiter")};

    EXPECT_EQ(ParFile::ParameterType::INTEGER, metadata.type);
}

TEST(TestParameterCatalog, juliaParamsSlotMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{core_catalog()};
    const ParFile::ParamsSlotMetadata &slot{catalog.params_slot("julia", 0)};

    EXPECT_EQ(0, slot.index);
    EXPECT_EQ("c-real", slot.name);
    EXPECT_EQ("params[0]", slot.metadata.name);
    EXPECT_EQ(ParFile::ParameterType::DOUBLE, slot.metadata.type);
    ASSERT_TRUE(slot.metadata.format);
    EXPECT_EQ(ParFile::ParameterFormat::RAW, *slot.metadata.format);
}

TEST(TestParameterCatalog, juliaParamsGroupMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{core_catalog()};
    const ParFile::ParamsGroupMetadata &group{catalog.params_group("julia", "c")};

    EXPECT_EQ("c", group.name);
    EXPECT_EQ("params.c", group.metadata.name);
    EXPECT_EQ(ParFile::ParameterType::COMPLEX, group.metadata.type);
    ASSERT_TRUE(group.metadata.format);
    EXPECT_EQ(ParFile::ParameterFormat::SLASH_PAIR, *group.metadata.format);
    ASSERT_EQ(2U, group.slots.size());
    EXPECT_EQ(0, group.slots[0]);
    EXPECT_EQ(1, group.slots[1]);
}

TEST(TestParameterCatalog, formulaParamsBailoutKnobMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{core_catalog()};
    const ParFile::FormulaParamsKnobMetadata &knob{catalog.formula_params_knob("MandelbrotMix4", "bailout")};

    EXPECT_EQ("bailout", knob.name);
    EXPECT_EQ("MandelbrotMix4.bailout", knob.metadata.name);
    EXPECT_EQ(ParFile::ParameterType::DOUBLE, knob.metadata.type);
    ASSERT_TRUE(knob.metadata.format);
    EXPECT_EQ(ParFile::ParameterFormat::RAW, *knob.metadata.format);
    ASSERT_EQ(1U, knob.slots.size());
    EXPECT_EQ(0, knob.slots[0]);
}

TEST(TestParameterCatalog, formulaParamsScaleFactorKnobMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{core_catalog()};
    const ParFile::FormulaParamsKnobMetadata &knob{catalog.formula_params_knob("MandelbrotMix4", "scale factor")};

    EXPECT_EQ("scale factor", knob.name);
    EXPECT_EQ("MandelbrotMix4.scale factor", knob.metadata.name);
    EXPECT_EQ(ParFile::ParameterType::DOUBLE, knob.metadata.type);
    ASSERT_EQ(1U, knob.slots.size());
    EXPECT_EQ(1, knob.slots[0]);
}

TEST(TestParameterCatalog, formulaParamsComplexKnobMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{core_catalog()};
    const ParFile::FormulaParamsKnobMetadata &knob{catalog.formula_params_knob("MandelbrotMix4", "c")};

    EXPECT_EQ("c", knob.name);
    EXPECT_EQ("MandelbrotMix4.c", knob.metadata.name);
    EXPECT_EQ(ParFile::ParameterType::COMPLEX, knob.metadata.type);
    ASSERT_TRUE(knob.metadata.format);
    EXPECT_EQ(ParFile::ParameterFormat::SLASH_PAIR, *knob.metadata.format);
    ASSERT_EQ(2U, knob.slots.size());
    EXPECT_EQ(2, knob.slots[0]);
    EXPECT_EQ(3, knob.slots[1]);
}

TEST(TestParameterCatalog, formulaParamsIntegerKnobMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{core_catalog()};
    const ParFile::FormulaParamsKnobMetadata &knob{catalog.formula_params_knob("MandelbrotMix4", "iterations")};

    EXPECT_EQ(ParFile::ParameterType::INTEGER, knob.metadata.type);
    ASSERT_EQ(1U, knob.slots.size());
    EXPECT_EQ(4, knob.slots[0]);
}

TEST(TestParameterCatalog, formulaFunctionMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{core_catalog()};
    const ParFile::FormulaFunctionMetadata &function{catalog.formula_function("MandelbrotMix4", "fn1")};

    EXPECT_EQ("fn1", function.name);
    EXPECT_EQ("MandelbrotMix4.fn1", function.metadata.name);
    EXPECT_EQ(ParFile::ParameterType::ENUM, function.metadata.type);
    EXPECT_EQ(0, function.slot);
    ASSERT_TRUE(function.metadata.format);
    EXPECT_EQ(ParFile::ParameterFormat::RAW, *function.metadata.format);
    ASSERT_TRUE(function.metadata.default_curve);
    EXPECT_EQ(ParFile::Curve::HOLD, *function.metadata.default_curve);
    EXPECT_NE(function.metadata.values.end(),
        std::find(function.metadata.values.begin(), function.metadata.values.end(), "sin"));
    EXPECT_NE(function.metadata.values.end(),
        std::find(function.metadata.values.begin(), function.metadata.values.end(), "round"));
}

TEST(TestParameterCatalog, legalParameterTypeStringsDecode)
{
    EXPECT_EQ(ParFile::ParameterType::CENTER_MAG, read_metadata(R"("type":"center-mag")").type);
    EXPECT_EQ(ParFile::ParameterType::COMPLEX, read_metadata(R"("type":"complex")").type);
    EXPECT_EQ(ParFile::ParameterType::CORNERS, read_metadata(R"("type":"corners")").type);
    EXPECT_EQ(ParFile::ParameterType::DOUBLE, read_metadata(R"("type":"double")").type);
    EXPECT_EQ(ParFile::ParameterType::ENUM, read_metadata(R"("type":"enum","values":["a"])").type);
    EXPECT_EQ(ParFile::ParameterType::INSIDE, read_metadata(R"("type":"inside","values":["maxiter"])").type);
    EXPECT_EQ(ParFile::ParameterType::INTEGER, read_metadata(R"("type":"integer")").type);
    EXPECT_EQ(ParFile::ParameterType::NUMERIC_TUPLE, read_metadata(R"("type":"numeric-tuple")").type);
    EXPECT_EQ(ParFile::ParameterType::OUTSIDE, read_metadata(R"("type":"outside","values":["iter"])").type);
    EXPECT_EQ(ParFile::ParameterType::POINT2, read_metadata(R"("type":"point2")").type);
    EXPECT_EQ(ParFile::ParameterType::POINT3, read_metadata(R"("type":"point3")").type);
    EXPECT_EQ(ParFile::ParameterType::VECTOR2, read_metadata(R"("type":"vector2")").type);
    EXPECT_EQ(ParFile::ParameterType::VECTOR3, read_metadata(R"("type":"vector3")").type);
}

TEST(TestParameterCatalog, legalParameterFormatStringsDecode)
{
    EXPECT_EQ(ParFile::ParameterFormat::RAW, *read_metadata(R"("type":"integer","format":"raw")").format);
    EXPECT_EQ(ParFile::ParameterFormat::SLASH, *read_metadata(R"("type":"integer","format":"slash")").format);
    EXPECT_EQ(ParFile::ParameterFormat::SLASH_LIST, *read_metadata(R"("type":"integer","format":"slash-list")").format);
    EXPECT_EQ(ParFile::ParameterFormat::SLASH_PAIR, *read_metadata(R"("type":"integer","format":"slash-pair")").format);
}

TEST(TestParameterCatalog, legalCurveStringsDecode)
{
    EXPECT_EQ(
        ParFile::Curve::GEOMETRIC, *read_metadata(R"("type":"integer","default-curve":"geometric")").default_curve);
    EXPECT_EQ(ParFile::Curve::HOLD, *read_metadata(R"("type":"integer","default-curve":"hold")").default_curve);
    EXPECT_EQ(ParFile::Curve::LINEAR, *read_metadata(R"("type":"integer","default-curve":"linear")").default_curve);
    EXPECT_EQ(ParFile::Curve::STEP, *read_metadata(R"("type":"integer","default-curve":"step")").default_curve);
}

TEST(TestParameterCatalog, legalExtrapolateStringsDecode)
{
    EXPECT_EQ(ParFile::ExtrapolateMode::BASE, *read_metadata(R"("type":"integer","extrapolate":"base")").extrapolate);
    EXPECT_EQ(ParFile::ExtrapolateMode::CLAMP, *read_metadata(R"("type":"integer","extrapolate":"clamp")").extrapolate);
    EXPECT_EQ(ParFile::ExtrapolateMode::CYCLE, *read_metadata(R"("type":"integer","extrapolate":"cycle")").extrapolate);
    EXPECT_EQ(ParFile::ExtrapolateMode::OMIT, *read_metadata(R"("type":"integer","extrapolate":"omit")").extrapolate);
    EXPECT_EQ(ParFile::ExtrapolateMode::PING_PONG,
        *read_metadata(R"("type":"integer","extrapolate":"ping-pong")").extrapolate);
}

TEST(TestParameterCatalog, optionalArityDecodes)
{
    const ParFile::ParameterMetadata metadata{read_metadata(R"("type":"numeric-tuple","arity":3)")};

    ASSERT_TRUE(metadata.arity);
    EXPECT_EQ(3, *metadata.arity);
}

TEST(TestParameterCatalog, tupleAliasArityDecodes)
{
    const ParFile::ParameterMetadata point2{read_metadata(R"("type":"point2")")};
    const ParFile::ParameterMetadata vector3{read_metadata(R"("type":"vector3")")};

    ASSERT_TRUE(point2.arity);
    EXPECT_EQ(2, *point2.arity);
    ASSERT_TRUE(vector3.arity);
    EXPECT_EQ(3, *vector3.arity);
}

TEST(TestParameterCatalog, optionalNormalizeDecodes)
{
    const ParFile::ParameterMetadata metadata{read_metadata(R"("type":"vector3","normalize":true)")};

    EXPECT_TRUE(metadata.normalize);
}

TEST(TestParameterCatalog, enumValuesDecode)
{
    const ParFile::ParameterMetadata metadata{read_metadata(R"("type":"enum","values":["a","b"])")};

    ASSERT_EQ(2U, metadata.values.size());
    EXPECT_EQ("a", metadata.values[0]);
    EXPECT_EQ("b", metadata.values[1]);
}

TEST(TestParameterCatalog, unknownParameterTypeStringRejected)
{
    EXPECT_THROW(ParFile::read_parameter_catalog(catalog_text(R"("type":"unknown")")), std::runtime_error);
}

TEST(TestParameterCatalog, unknownParameterFormatStringRejected)
{
    EXPECT_THROW(
        ParFile::read_parameter_catalog(catalog_text(R"("type":"integer","format":"unknown")")), std::runtime_error);
}

TEST(TestParameterCatalog, unknownCurveStringRejected)
{
    EXPECT_THROW(ParFile::read_parameter_catalog(catalog_text(R"("type":"integer","default-curve":"unknown")")),
        std::runtime_error);
}

TEST(TestParameterCatalog, unknownExtrapolateStringRejected)
{
    EXPECT_THROW(ParFile::read_parameter_catalog(catalog_text(R"("type":"integer","extrapolate":"unknown")")),
        std::runtime_error);
}

TEST(TestParameterCatalog, invalidArityRejected)
{
    EXPECT_THROW(
        ParFile::read_parameter_catalog(catalog_text(R"("type":"numeric-tuple","arity":0)")), std::runtime_error);
}

TEST(TestParameterCatalog, tupleAliasArityMismatchRejected)
{
    EXPECT_THROW(ParFile::read_parameter_catalog(catalog_text(R"("type":"point3","arity":2)")), std::runtime_error);
}

TEST(TestParameterCatalog, invalidNormalizeRejected)
{
    EXPECT_THROW(
        ParFile::read_parameter_catalog(catalog_text(R"("type":"vector3","normalize":"true")")), std::runtime_error);
}

TEST(TestParameterCatalog, enumMissingValuesRejected)
{
    EXPECT_THROW(ParFile::read_parameter_catalog(catalog_text(R"("type":"enum")")), std::runtime_error);
    EXPECT_THROW(ParFile::read_parameter_catalog(catalog_text(R"("type":"inside")")), std::runtime_error);
    EXPECT_THROW(ParFile::read_parameter_catalog(catalog_text(R"("type":"outside")")), std::runtime_error);
}

TEST(TestParameterCatalog, valuesRequireDiscreteType)
{
    EXPECT_THROW(
        ParFile::read_parameter_catalog(catalog_text(R"("type":"integer","values":["a"])")), std::runtime_error);
}

TEST(TestParameterCatalog, invalidEnumValuesRejected)
{
    EXPECT_THROW(ParFile::read_parameter_catalog(catalog_text(R"("type":"enum","values":"a")")), std::runtime_error);
    EXPECT_THROW(
        ParFile::read_parameter_catalog(catalog_text(R"("type":"enum","values":["a", 1])")), std::runtime_error);
}

TEST(TestParameterCatalog, unknownAnimatedParameterRejected)
{
    EXPECT_THROW(typed_catalog().metadata("unknown"), std::runtime_error);
}

TEST(TestParameterCatalog, unknownParamsSlotRejected)
{
    EXPECT_THROW(core_catalog().params_slot("julia", 2), std::runtime_error);
}

TEST(TestParameterCatalog, unknownParamsGroupRejected)
{
    EXPECT_THROW(core_catalog().params_group("julia", "unknown"), std::runtime_error);
}

TEST(TestParameterCatalog, unknownFormulaParamsKnobRejected)
{
    EXPECT_THROW(core_catalog().formula_params_knob("MandelbrotMix4", "unknown"), std::runtime_error);
}

TEST(TestParameterCatalog, unknownFormulaFunctionRejected)
{
    EXPECT_THROW(core_catalog().formula_function("MandelbrotMix4", "fn5"), std::runtime_error);
}

TEST(TestParameterCatalog, unknownFormulaFunctionValuesRejected)
{
    EXPECT_THROW(ParFile::read_parameter_catalog(formula_function_catalog_text(R"("type":"enum","values":"unknown")")),
        std::runtime_error);
}

TEST(TestParameterCatalog, invalidFormulaFunctionNameRejected)
{
    EXPECT_THROW(
        ParFile::read_parameter_catalog("{\"parameters\":{},\"formula-entries\":{\"foo\":{\"functions\":{\"fn5\":{"
                                        "\"type\":\"enum\",\"values\":\"id-functions\"}}}}}"),
        std::runtime_error);
}

TEST(TestParameterCatalog, realFormulaParamsKnobRejectsComplexVariable)
{
    EXPECT_THROW(
        ParFile::read_parameter_catalog(formula_catalog_text(R"("type":"real","variable":"p1")")), std::runtime_error);
}

TEST(TestParameterCatalog, integerFormulaParamsKnobRejectsComplexVariable)
{
    EXPECT_THROW(ParFile::read_parameter_catalog(formula_catalog_text(R"("type":"integer","variable":"p1")")),
        std::runtime_error);
}

TEST(TestParameterCatalog, complexFormulaParamsKnobRejectsComponentVariable)
{
    EXPECT_THROW(ParFile::read_parameter_catalog(formula_catalog_text(R"("type":"complex","variable":"p1.real")")),
        std::runtime_error);
}

TEST(TestParameterCatalog, formulaParamsKnobRejectsP5Variable)
{
    EXPECT_THROW(ParFile::read_parameter_catalog(formula_catalog_text(R"("type":"real","variable":"p5.real")")),
        std::runtime_error);
    EXPECT_THROW(ParFile::read_parameter_catalog(formula_catalog_text(R"("type":"complex","variable":"p5")")),
        std::runtime_error);
}

TEST(TestParameterCatalog, missingMetadataTypeRejected)
{
    EXPECT_THROW(ParFile::read_parameter_catalog(read_text(TestParFile::INVALID_MISSING_METADATA_TYPE_JSON)),
        std::runtime_error);
}
