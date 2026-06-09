// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/ParameterCatalog.h>

#include <TestParFile/test.h>

#include <gtest/gtest.h>

#include <algorithm>
#include <fstream>
#include <iterator>
#include <optional>
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

ParFile::ParameterCatalog coloring_catalog()
{
    return ParFile::read_parameter_catalog(read_text(TestParFile::COLORING_CATALOG_JSON));
}

ParFile::ParameterCatalog id_3d_catalog()
{
    return ParFile::read_parameter_catalog(read_text(TestParFile::ID_3D_CATALOG_JSON));
}

ParFile::ParameterCatalog formula_catalog()
{
    return ParFile::read_parameter_catalog(read_text(TestParFile::FORMULA_CATALOG_JSON));
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

std::string fractal_function_catalog_text(std::string_view function)
{
    return "{\"parameters\":{},\"fractal-types\":{\"foo\":{\"functions\":{\"fn1\":{" + std::string{function} + "}}}}}";
}

struct FormulaEntryMetadataCase
{
    const char *name;
};

constexpr FormulaEntryMetadataCase FORMULA_ENTRY_METADATA_CASES[]{
    {"DAFrm01"},
    {"DAFrm07"},
    {"Larry"},
};

struct FormulaFunctionMetadataCase
{
    const char *formula_name;
    const char *name;
    int slot;
};

struct ParamsSlotMetadataCase
{
    const char *fractal_type;
    int slot;
    const char *name;
    ParFile::ParameterType type;
    ParFile::Curve default_curve;
    std::optional<double> min;
    std::optional<double> max;
};

struct FunctionSlotMetadataCase
{
    const char *fractal_type;
    int slot;
    const char *name;
};

struct ParamsGroupMetadataCase
{
    const char *fractal_type;
    const char *name;
    const char *metadata_name;
    int first_slot;
    int second_slot;
};

constexpr FormulaFunctionMetadataCase FORMULA_FUNCTION_METADATA_CASES[]{
    {"DAFrm01", "fn1", 0},
    {"DAFrm07", "fn1", 0},
    {"Larry", "fn1", 0},
    {"Larry", "fn2", 1},
};

const ParamsSlotMetadataCase PARAMS_SLOT_METADATA_CASES[]{
    {"ant", 0, "rule-string", ParFile::ParameterType::INTEGER, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"ant", 1, "max-points", ParFile::ParameterType::INTEGER, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"ant", 2, "ant-count", ParFile::ParameterType::INTEGER, ParFile::Curve::LINEAR, 1.0, 256.0},
    {"ant", 3, "ant-type", ParFile::ParameterType::INTEGER, ParFile::Curve::HOLD, 1.0, 2.0},
    {"ant", 4, "wrap", ParFile::ParameterType::INTEGER, ParFile::Curve::HOLD, 0.0, 1.0},
    {"ant", 5, "random-seed-mode", ParFile::ParameterType::INTEGER, ParFile::Curve::HOLD, 0.0, 1.0},
    {"bifurcation", 0, "filter-cycles", ParFile::ParameterType::INTEGER, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"bifurcation", 1, "seed-population", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"bif+sinpi", 0, "filter-cycles", ParFile::ParameterType::INTEGER, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"bif+sinpi", 1, "seed-population", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"bif=sinpi", 0, "filter-cycles", ParFile::ParameterType::INTEGER, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"bif=sinpi", 1, "seed-population", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"biflambda", 0, "filter-cycles", ParFile::ParameterType::INTEGER, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"biflambda", 1, "seed-population", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"bifmay", 0, "filter-cycles", ParFile::ParameterType::INTEGER, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"bifmay", 1, "seed-population", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"bifmay", 2, "beta", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"bifstewart", 0, "filter-cycles", ParFile::ParameterType::INTEGER, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"bifstewart", 1, "seed-population", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"burning-ship", 0, "p1-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"burning-ship", 1, "p1-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"burning-ship", 2, "degree", ParFile::ParameterType::INTEGER, ParFile::Curve::LINEAR, 2.0, 5.0},
    {"cellular", 0, "initial-string", ParFile::ParameterType::INTEGER, ParFile::Curve::LINEAR, -1.0, std::nullopt},
    {"cellular", 1, "rule", ParFile::ParameterType::INTEGER, ParFile::Curve::LINEAR, 0.0, std::nullopt},
    {"cellular", 2, "cellular-type", ParFile::ParameterType::INTEGER, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"cellular", 3, "starting-row", ParFile::ParameterType::INTEGER, ParFile::Curve::LINEAR, 0.0, std::nullopt},
    {"chip", 0, "a", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"chip", 1, "b", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"chip", 2, "c", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"circle", 0, "magnification", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"diffusion", 0, "border-size", ParFile::ParameterType::INTEGER, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"diffusion", 1, "diffusion-type", ParFile::ParameterType::INTEGER, ParFile::Curve::HOLD, 0.0, 2.0},
    {"diffusion", 2, "color-change-rate", ParFile::ParameterType::INTEGER, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"dividebrot5", 0, "a", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"dividebrot5", 1, "b", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"escher_julia", 0, "parameter-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"escher_julia", 1, "parameter-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"fn(z)+fn(pix)", 0, "z0-perturbation-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"fn(z)+fn(pix)", 1, "z0-perturbation-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"fn(z)+fn(pix)", 2, "fn2-coefficient-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"fn(z)+fn(pix)", 3, "fn2-coefficient-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"fn+fn", 0, "fn1-coefficient-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"fn+fn", 1, "fn1-coefficient-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"fn+fn", 2, "fn2-coefficient-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"fn+fn", 3, "fn2-coefficient-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"fn*z+z", 0, "fn1-coefficient-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"fn*z+z", 1, "fn1-coefficient-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"fn*z+z", 2, "second-term-coefficient-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"fn*z+z", 3, "second-term-coefficient-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"frothybasin", 0, "mapping-pass-count", ParFile::ParameterType::INTEGER, ParFile::Curve::HOLD, 1.0, 2.0},
    {"frothybasin", 1, "alternate-color-shading", ParFile::ParameterType::INTEGER, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"frothybasin", 2, "c-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"ifs", 0, "coloring-method", ParFile::ParameterType::INTEGER, ParFile::Curve::HOLD, 0.0, 1.0},
    {"ifs3d", 0, "coloring-method", ParFile::ParameterType::INTEGER, ParFile::Curve::HOLD, 0.0, 1.0},
    {"kamtorus", 0, "angle", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"kamtorus", 1, "step-size", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"kamtorus", 2, "stop-value", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"kamtorus", 3, "points-per-orbit", ParFile::ParameterType::INTEGER, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"kamtorus3d", 0, "angle", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"kamtorus3d", 1, "step-size", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"kamtorus3d", 2, "stop-value", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"kamtorus3d", 3, "points-per-orbit", ParFile::ParameterType::INTEGER, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"julfn+exp", 0, "c-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"julfn+exp", 1, "c-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"julfn+zsqrd", 0, "c-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"julfn+zsqrd", 1, "c-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"lambdafn", 0, "c-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"lambdafn", 1, "c-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"latoocarfian", 0, "a", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"latoocarfian", 1, "b", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"latoocarfian", 2, "c", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"latoocarfian", 3, "d", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"lorenz", 0, "time-step", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"lorenz", 1, "a", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"lorenz", 2, "b", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"lorenz", 3, "c", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"lorenz3d", 0, "time-step", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"lorenz3d", 1, "a", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"lorenz3d", 2, "b", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"lorenz3d", 3, "c", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"lorenz3d1", 0, "time-step", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"lorenz3d1", 1, "a", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"lorenz3d1", 2, "b", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"lorenz3d1", 3, "c", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"lorenz3d3", 0, "time-step", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"lorenz3d3", 1, "a", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"lorenz3d3", 2, "b", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"lorenz3d3", 3, "c", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"lorenz3d4", 0, "time-step", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"lorenz3d4", 1, "a", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"lorenz3d4", 2, "b", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"lorenz3d4", 3, "c", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"lsystem", 0, "order", ParFile::ParameterType::INTEGER, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"lyapunov", 0, "order", ParFile::ParameterType::INTEGER, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"lyapunov", 1, "seed-population", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"lyapunov", 2, "filter-cycles", ParFile::ParameterType::INTEGER, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"dynamic", 0, "interval-count", ParFile::ParameterType::INTEGER, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"dynamic", 1, "time-step", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"dynamic", 2, "a", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"dynamic", 3, "b", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"hypercomplex", 0, "unused-0", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"hypercomplex", 1, "unused-1", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"hypercomplex", 2, "cj", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"hypercomplex", 3, "ck", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"hypercomplexj", 0, "c1", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"hypercomplexj", 1, "ci", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"hypercomplexj", 2, "cj", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"hypercomplexj", 3, "ck", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"hypercomplexj", 4, "zj", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"hypercomplexj", 5, "zk", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"icons", 0, "lambda", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"icons", 1, "alpha", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"icons", 2, "beta", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"icons", 3, "gamma", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"icons", 4, "omega", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"icons", 5, "symmetry-degree", ParFile::ParameterType::INTEGER, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"icons3d", 0, "lambda", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"icons3d", 1, "alpha", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"icons3d", 2, "beta", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"icons3d", 3, "gamma", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"icons3d", 4, "omega", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"icons3d", 5, "symmetry-degree", ParFile::ParameterType::INTEGER, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"quat", 0, "unused-0", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"quat", 1, "unused-1", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"quat", 2, "cj", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"quat", 3, "ck", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"quatjul", 0, "c1", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"quatjul", 1, "ci", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"quatjul", 2, "cj", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"quatjul", 3, "ck", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"quatjul", 4, "zj", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"quatjul", 5, "zk", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"rossler3d", 0, "time-step", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"rossler3d", 1, "a", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"rossler3d", 2, "b", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"rossler3d", 3, "c", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"henon", 0, "a", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"henon", 1, "b", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"hopalong", 0, "a", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"hopalong", 1, "b", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"hopalong", 2, "c", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"martin", 0, "a", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"pickover", 0, "a", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"pickover", 1, "b", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"pickover", 2, "c", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"pickover", 3, "d", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"gingerbreadman", 0, "initial-x", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"gingerbreadman", 1, "initial-y", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"mandel", 0, "z0-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"mandel", 1, "z0-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"mandel(fn||fn)", 0, "z0-perturbation-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"mandel(fn||fn)", 1, "z0-perturbation-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"mandel(fn||fn)", 2, "function-shift", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"mandel4", 0, "z0-perturbation-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"mandel4", 1, "z0-perturbation-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"mandelbrotmix4", 0, "p1-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"mandelbrotmix4", 1, "p1-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"mandelbrotmix4", 2, "p2-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"mandelbrotmix4", 3, "p2-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"mandelbrotmix4", 4, "p3-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"mandelbrotmix4", 5, "p3-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"mandelcloud", 0, "interval-count", ParFile::ParameterType::INTEGER, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"mandelfn", 0, "z0-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"mandelfn", 1, "z0-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"mandellambda", 0, "z0-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"mandellambda", 1, "z0-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"mandphoenix", 0, "z0-perturbation-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"mandphoenix", 1, "z0-perturbation-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"mandphoenix", 2, "degree", ParFile::ParameterType::INTEGER, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"mandphoenixclx", 0, "z0-perturbation-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"mandphoenixclx", 1, "z0-perturbation-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"mandphoenixclx", 2, "p2-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"mandphoenixclx", 3, "p2-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"mandphoenixclx", 4, "degree", ParFile::ParameterType::INTEGER, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"spider", 0, "z0-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"spider", 1, "z0-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"tetrate", 0, "z0-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"tetrate", 1, "z0-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"magnet1m", 0, "z0-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"magnet1m", 1, "z0-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"magnet1j", 0, "c-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"magnet1j", 1, "c-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"magnet2m", 0, "z0-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"magnet2m", 1, "z0-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"magnet2j", 0, "c-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"magnet2j", 1, "c-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"manfn+exp", 0, "z0-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"manfn+exp", 1, "z0-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"manfn+zsqrd", 0, "z0-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"manfn+zsqrd", 1, "z0-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"manlam(fn||fn)", 0, "z0-perturbation-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"manlam(fn||fn)", 1, "z0-perturbation-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"manlam(fn||fn)", 2, "function-shift", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"barnsleym1", 0, "z0-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"barnsleym1", 1, "z0-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"barnsleyj1", 0, "c-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"barnsleyj1", 1, "c-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"barnsleym2", 0, "z0-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"barnsleym2", 1, "z0-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"barnsleyj2", 0, "c-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"barnsleyj2", 1, "c-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"barnsleym3", 0, "z0-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"barnsleym3", 1, "z0-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"barnsleyj3", 0, "c-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"barnsleyj3", 1, "c-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"manowar", 0, "z0-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"manowar", 1, "z0-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"manowarj", 0, "c-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"manowarj", 1, "c-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"manzpower", 0, "z0-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"manzpower", 1, "z0-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"manzpower", 2, "exponent-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"manzpower", 3, "exponent-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"cmplxmarksmand", 0, "z0-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"cmplxmarksmand", 1, "z0-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"cmplxmarksmand", 2, "exponent-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"cmplxmarksmand", 3, "exponent-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"manzzpwr", 0, "z0-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"manzzpwr", 1, "z0-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"manzzpwr", 2, "exponent-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"marksmandel", 0, "z0-perturbation-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"marksmandel", 1, "z0-perturbation-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"marksmandel", 2, "exponent-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"marksmandelpwr", 0, "z0-perturbation-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"marksmandelpwr", 1, "z0-perturbation-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"tim's_error", 0, "z0-perturbation-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"tim's_error", 1, "z0-perturbation-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"newtbasin", 0, "degree", ParFile::ParameterType::INTEGER, ParFile::Curve::LINEAR, 2.0, std::nullopt},
    {"newtbasin", 1, "stripes", ParFile::ParameterType::DOUBLE, ParFile::Curve::HOLD, std::nullopt, std::nullopt},
    {"newton", 0, "degree", ParFile::ParameterType::INTEGER, ParFile::Curve::LINEAR, 2.0, std::nullopt},
    {"phoenix", 0, "p1-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"phoenix", 1, "p2-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"phoenix", 2, "degree", ParFile::ParameterType::INTEGER, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"phoenixcplx", 0, "p1-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"phoenixcplx", 1, "p1-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"phoenixcplx", 2, "p2-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"phoenixcplx", 3, "p2-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"phoenixcplx", 4, "degree", ParFile::ParameterType::INTEGER, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"halley", 0, "order", ParFile::ParameterType::INTEGER, ParFile::Curve::LINEAR, 2.0, std::nullopt},
    {"halley", 1, "relaxation-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"halley", 2, "epsilon", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"halley", 3, "relaxation-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"complexbasin", 0, "degree-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"complexbasin", 1, "degree-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"complexbasin", 2, "root-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"complexbasin", 3, "root-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"complexnewton", 0, "degree-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"complexnewton", 1, "degree-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"complexnewton", 2, "root-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"complexnewton", 3, "root-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"cmplxmarksjul", 0, "c-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"cmplxmarksjul", 1, "c-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"cmplxmarksjul", 2, "exponent-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"cmplxmarksjul", 3, "exponent-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"marksjulia", 0, "c-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"marksjulia", 1, "c-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"marksjulia", 2, "exponent-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"julia", 0, "c-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"julia", 1, "c-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"julia(fn||fn)", 0, "c-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"julia(fn||fn)", 1, "c-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"julia(fn||fn)", 2, "function-shift", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"julia_inverse", 0, "parameter-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"julia_inverse", 1, "parameter-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"julia_inverse", 2, "max-hits-per-pixel", ParFile::ParameterType::INTEGER, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"julia4", 0, "c-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"julia4", 1, "c-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"julzpower", 0, "c-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"julzpower", 1, "c-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"julzpower", 2, "exponent-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"julzpower", 3, "exponent-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"julzzpwr", 0, "c-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"julzzpwr", 1, "c-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"julzzpwr", 2, "exponent-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"lambda", 0, "c-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"lambda", 1, "c-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"lambda(fn||fn)", 0, "c-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"lambda(fn||fn)", 1, "c-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"lambda(fn||fn)", 2, "function-shift", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"plasma", 0, "graininess", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, 0.0, 100.0},
    {"plasma", 1, "algorithm", ParFile::ParameterType::INTEGER, ParFile::Curve::HOLD, 0.0, 1.0},
    {"plasma", 2, "random-seed-mode", ParFile::ParameterType::INTEGER, ParFile::Curve::HOLD, 0.0, 1.0},
    {"plasma", 3, "save-pot-file", ParFile::ParameterType::INTEGER, ParFile::Curve::HOLD, 0.0, 1.0},
    {"popcorn", 0, "step-size-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"popcorn", 1, "step-size-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"popcorn", 2, "c-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"popcorn", 3, "c-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"popcornjul", 0, "step-size-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"popcornjul", 1, "step-size-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt,
        std::nullopt},
    {"popcornjul", 2, "c-real", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"popcornjul", 3, "c-imag", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"quadruptwo", 0, "a", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"quadruptwo", 1, "b", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"quadruptwo", 2, "c", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"threeply", 0, "a", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"threeply", 1, "b", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"threeply", 2, "c", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"volterra-lotka", 0, "h", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
    {"volterra-lotka", 1, "p", ParFile::ParameterType::DOUBLE, ParFile::Curve::LINEAR, std::nullopt, std::nullopt},
};

constexpr FunctionSlotMetadataCase FUNCTION_SLOT_METADATA_CASES[]{
    {"bifurcation", 0, "fn1"},
    {"bif+sinpi", 0, "fn1"},
    {"bif=sinpi", 0, "fn1"},
    {"biflambda", 0, "fn1"},
    {"bifstewart", 0, "fn1"},
    {"dynamic", 0, "fn1"},
    {"fn(z)+fn(pix)", 0, "fn1"},
    {"fn(z)+fn(pix)", 1, "fn2"},
    {"fn+fn", 0, "fn1"},
    {"fn+fn", 1, "fn2"},
    {"fn*z+z", 0, "fn1"},
    {"hypercomplex", 0, "fn1"},
    {"hypercomplexj", 0, "fn1"},
    {"julfn+exp", 0, "fn1"},
    {"julfn+zsqrd", 0, "fn1"},
    {"julia(fn||fn)", 0, "fn1"},
    {"julia(fn||fn)", 1, "fn2"},
    {"lambda(fn||fn)", 0, "fn1"},
    {"lambda(fn||fn)", 1, "fn2"},
    {"lambdafn", 0, "fn1"},
    {"latoocarfian", 0, "fn1"},
    {"latoocarfian", 1, "fn2"},
    {"latoocarfian", 2, "fn3"},
    {"latoocarfian", 3, "fn4"},
    {"mandel(fn||fn)", 0, "fn1"},
    {"mandel(fn||fn)", 1, "fn2"},
    {"mandelbrotmix4", 0, "fn1"},
    {"mandelfn", 0, "fn1"},
    {"manfn+exp", 0, "fn1"},
    {"manfn+zsqrd", 0, "fn1"},
    {"manlam(fn||fn)", 0, "fn1"},
    {"manlam(fn||fn)", 1, "fn2"},
    {"marksmandelpwr", 0, "fn1"},
    {"tim's_error", 0, "fn1"},
    {"popcorn", 0, "fn1"},
    {"popcorn", 1, "fn2"},
    {"popcorn", 2, "fn3"},
    {"popcorn", 3, "fn4"},
    {"popcornjul", 0, "fn1"},
    {"popcornjul", 1, "fn2"},
    {"popcornjul", 2, "fn3"},
    {"popcornjul", 3, "fn4"},
};

constexpr ParamsGroupMetadataCase PARAMS_GROUP_METADATA_CASES[]{
    {"fn(z)+fn(pix)", "z0-perturbation", "params.z0-perturbation", 0, 1},
    {"fn(z)+fn(pix)", "fn2-coefficient", "params.fn2-coefficient", 2, 3},
    {"fn+fn", "fn1-coefficient", "params.fn1-coefficient", 0, 1},
    {"fn+fn", "fn2-coefficient", "params.fn2-coefficient", 2, 3},
    {"fn*z+z", "fn1-coefficient", "params.fn1-coefficient", 0, 1},
    {"fn*z+z", "second-term-coefficient", "params.second-term-coefficient", 2, 3},
    {"complexbasin", "degree", "params.degree", 0, 1},
    {"complexbasin", "root", "params.root", 2, 3},
    {"complexnewton", "degree", "params.degree", 0, 1},
    {"complexnewton", "root", "params.root", 2, 3},
    {"burning-ship", "p1", "params.p1", 0, 1},
    {"halley", "relaxation", "params.relaxation", 1, 3},
    {"phoenixcplx", "p1", "params.p1", 0, 1},
    {"phoenixcplx", "p2", "params.p2", 2, 3},
    {"cmplxmarksjul", "c", "params.c", 0, 1},
    {"cmplxmarksjul", "exponent", "params.exponent", 2, 3},
    {"marksjulia", "c", "params.c", 0, 1},
    {"julfn+exp", "c", "params.c", 0, 1},
    {"julfn+zsqrd", "c", "params.c", 0, 1},
    {"julia", "c", "params.c", 0, 1},
    {"julia(fn||fn)", "c", "params.c", 0, 1},
    {"julia_inverse", "parameter", "params.parameter", 0, 1},
    {"escher_julia", "parameter", "params.parameter", 0, 1},
    {"julia4", "c", "params.c", 0, 1},
    {"julzpower", "c", "params.c", 0, 1},
    {"julzpower", "exponent", "params.exponent", 2, 3},
    {"julzzpwr", "c", "params.c", 0, 1},
    {"lambda", "c", "params.c", 0, 1},
    {"lambda(fn||fn)", "c", "params.c", 0, 1},
    {"barnsleyj1", "c", "params.c", 0, 1},
    {"barnsleyj2", "c", "params.c", 0, 1},
    {"barnsleyj3", "c", "params.c", 0, 1},
    {"magnet1j", "c", "params.c", 0, 1},
    {"magnet2j", "c", "params.c", 0, 1},
    {"lambdafn", "c", "params.c", 0, 1},
    {"manowarj", "c", "params.c", 0, 1},
    {"mandel", "z0", "params.z0", 0, 1},
    {"mandel(fn||fn)", "z0-perturbation", "params.z0-perturbation", 0, 1},
    {"mandel4", "z0-perturbation", "params.z0-perturbation", 0, 1},
    {"mandelbrotmix4", "p1", "params.p1", 0, 1},
    {"mandelbrotmix4", "p2", "params.p2", 2, 3},
    {"mandelbrotmix4", "p3", "params.p3", 4, 5},
    {"mandelfn", "z0", "params.z0", 0, 1},
    {"mandellambda", "z0", "params.z0", 0, 1},
    {"mandphoenix", "z0-perturbation", "params.z0-perturbation", 0, 1},
    {"mandphoenixclx", "z0-perturbation", "params.z0-perturbation", 0, 1},
    {"mandphoenixclx", "p2", "params.p2", 2, 3},
    {"spider", "z0", "params.z0", 0, 1},
    {"tetrate", "z0", "params.z0", 0, 1},
    {"magnet1m", "z0", "params.z0", 0, 1},
    {"magnet2m", "z0", "params.z0", 0, 1},
    {"manfn+exp", "z0", "params.z0", 0, 1},
    {"manfn+zsqrd", "z0", "params.z0", 0, 1},
    {"manlam(fn||fn)", "z0-perturbation", "params.z0-perturbation", 0, 1},
    {"barnsleym1", "z0", "params.z0", 0, 1},
    {"barnsleym2", "z0", "params.z0", 0, 1},
    {"barnsleym3", "z0", "params.z0", 0, 1},
    {"manowar", "z0", "params.z0", 0, 1},
    {"cmplxmarksmand", "exponent", "params.exponent", 2, 3},
    {"cmplxmarksmand", "z0", "params.z0", 0, 1},
    {"manzpower", "exponent", "params.exponent", 2, 3},
    {"manzpower", "z0", "params.z0", 0, 1},
    {"manzzpwr", "z0", "params.z0", 0, 1},
    {"marksmandel", "z0-perturbation", "params.z0-perturbation", 0, 1},
    {"marksmandelpwr", "z0-perturbation", "params.z0-perturbation", 0, 1},
    {"tim's_error", "z0-perturbation", "params.z0-perturbation", 0, 1},
    {"popcorn", "c", "params.c", 2, 3},
    {"popcorn", "step-size", "params.step-size", 0, 1},
    {"popcornjul", "c", "params.c", 2, 3},
    {"popcornjul", "step-size", "params.step-size", 0, 1},
};

std::string test_parameter_name(std::string text)
{
    std::string result;
    for (const char ch : text)
    {
        const bool valid{(ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || ch == '_'};
        if (valid)
        {
            result.push_back(ch);
        }
        else if (ch == '+')
        {
            result += "_plus_";
        }
        else if (ch == '=')
        {
            result += "_eq_";
        }
        else
        {
            result.push_back('_');
        }
    }
    return result;
}

std::string formula_entry_metadata_test_name(const ::testing::TestParamInfo<FormulaEntryMetadataCase> &info)
{
    return test_parameter_name(info.param.name);
}

std::string params_slot_metadata_test_name(const ::testing::TestParamInfo<ParamsSlotMetadataCase> &info)
{
    return test_parameter_name(
        std::string{info.param.fractal_type} + "_" + std::to_string(info.param.slot) + "_" + info.param.name);
}

std::string function_slot_metadata_test_name(const ::testing::TestParamInfo<FunctionSlotMetadataCase> &info)
{
    return test_parameter_name(
        std::string{info.param.fractal_type} + "_" + std::to_string(info.param.slot) + "_" + info.param.name);
}

std::string params_group_metadata_test_name(const ::testing::TestParamInfo<ParamsGroupMetadataCase> &info)
{
    return test_parameter_name(std::string{info.param.fractal_type} + "_" + info.param.name);
}

std::string formula_function_metadata_test_name(const ::testing::TestParamInfo<FormulaFunctionMetadataCase> &info)
{
    return test_parameter_name(std::string{info.param.formula_name} + "_" + info.param.name);
}

ParFile::ParameterMetadata read_metadata(std::string_view metadata)
{
    return ParFile::read_parameter_catalog(catalog_text(metadata)).metadata("x");
}

} // namespace

TEST(TestParameterCatalog, validCatalogJsonDeserializesAllCoreParameters)
{
    const ParFile::ParameterCatalog catalog{core_catalog()};

    EXPECT_EQ(42U, catalog.parameters.size());
    EXPECT_EQ(101U, catalog.fractal_types.size());
    EXPECT_EQ(0U, catalog.formula_entries.size());
}

TEST(TestParameterCatalog, typeMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{core_catalog()};
    const ParFile::ParameterMetadata &metadata{catalog.metadata("type")};

    EXPECT_EQ("type", metadata.name);
    EXPECT_EQ(ParFile::ParameterType::ENUM, metadata.type);
    ASSERT_TRUE(metadata.format);
    EXPECT_EQ(ParFile::ParameterFormat::RAW, *metadata.format);
    ASSERT_TRUE(metadata.default_curve);
    EXPECT_EQ(ParFile::Curve::HOLD, *metadata.default_curve);
    ASSERT_TRUE(metadata.extrapolate);
    EXPECT_EQ(ParFile::ExtrapolateMode::CLAMP, *metadata.extrapolate);
    EXPECT_NE(metadata.values.end(), std::find(metadata.values.begin(), metadata.values.end(), "mandel"));
    EXPECT_NE(metadata.values.end(), std::find(metadata.values.begin(), metadata.values.end(), "julia"));
    EXPECT_NE(metadata.values.end(), std::find(metadata.values.begin(), metadata.values.end(), "formula"));
    EXPECT_NE(metadata.values.end(), std::find(metadata.values.begin(), metadata.values.end(), "julibrot"));
    EXPECT_NE(metadata.values.end(), std::find(metadata.values.begin(), metadata.values.end(), "mandelbrotmix4"));
    EXPECT_NE(metadata.values.end(), std::find(metadata.values.begin(), metadata.values.end(), "tim's_error"));
    EXPECT_EQ(110U, metadata.values.size());
}

TEST(TestParameterCatalog, coloringCatalogDeclaresColors)
{
    const ParFile::ParameterCatalog catalog{coloring_catalog()};
    const ParFile::ParameterMetadata &metadata{catalog.metadata("colors")};

    EXPECT_EQ("colors", metadata.name);
    EXPECT_EQ(ParFile::ParameterType::COLOR_MAP, metadata.type);
    ASSERT_TRUE(metadata.format);
    EXPECT_EQ(ParFile::ParameterFormat::RAW, *metadata.format);
    ASSERT_TRUE(metadata.default_curve);
    EXPECT_EQ(ParFile::Curve::HOLD, *metadata.default_curve);
    ASSERT_TRUE(metadata.extrapolate);
    EXPECT_EQ(ParFile::ExtrapolateMode::CLAMP, *metadata.extrapolate);
}

TEST(TestParameterCatalog, coloringCatalogIncludesSavedImageParameters)
{
    const ParFile::ParameterCatalog catalog{coloring_catalog()};
    const ParFile::ParameterMetadata &mode{catalog.metadata("logmode")};
    const ParFile::ParameterMetadata &decomposition{catalog.metadata("decomp")};
    const ParFile::ParameterMetadata &no_bof{catalog.metadata("nobof")};
    const ParFile::ParameterMetadata &old_demm_colors{catalog.metadata("olddemmcolors")};
    const ParFile::ParameterMetadata &ranges{catalog.metadata("ranges")};
    const ParFile::ParameterMetadata &distance_estimator{catalog.metadata("distest")};
    const ParFile::ParameterMetadata &true_color{catalog.metadata("truecolor")};
    const ParFile::ParameterMetadata &true_mode{catalog.metadata("truemode")};

    EXPECT_EQ(11U, catalog.parameters.size());
    EXPECT_EQ(ParFile::ParameterType::ENUM, mode.type);
    EXPECT_NE(mode.values.end(), std::find(mode.values.begin(), mode.values.end(), "fly"));
    EXPECT_NE(mode.values.end(), std::find(mode.values.begin(), mode.values.end(), "table"));
    EXPECT_EQ(ParFile::ParameterType::INTEGER, decomposition.type);
    EXPECT_EQ(ParFile::ParameterType::YES_NO, no_bof.type);
    EXPECT_EQ(ParFile::ParameterType::YES_NO, old_demm_colors.type);
    EXPECT_EQ(ParFile::ParameterType::STRING, ranges.type);
    EXPECT_EQ(ParFile::ParameterType::INTEGER_TUPLE, distance_estimator.type);
    ASSERT_TRUE(distance_estimator.arity);
    EXPECT_EQ(2, *distance_estimator.arity);
    EXPECT_EQ(ParFile::ParameterType::YES_NO, true_color.type);
    EXPECT_EQ(ParFile::ParameterType::ENUM, true_mode.type);
}

TEST(TestParameterCatalog, nonSavedImageControlsAreNotCataloged)
{
    const ParFile::ParameterCatalog core{core_catalog()};
    const ParFile::ParameterCatalog coloring{coloring_catalog()};
    const ParFile::ParameterCatalog id_3d{id_3d_catalog()};
    const char *names[]{"askvideo", "fastrestore", "viewwindows", "virtual", "recordcolors", "cyclerange", "cyclelimit",
        "textcolors", "hertz", "sound", "volume", "attenuate", "polyphony", "wavetype", "attack", "decay", "sustain",
        "srelease", "scalemap", "orbitsave", "orbitsavename"};

    for (const char *name : names)
    {
        EXPECT_THROW(core.metadata(name), std::runtime_error);
        EXPECT_THROW(coloring.metadata(name), std::runtime_error);
        EXPECT_THROW(id_3d.metadata(name), std::runtime_error);
    }
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

TEST(TestParameterCatalog, yesNoMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{core_catalog()};
    const ParFile::ParameterMetadata &metadata{catalog.metadata("showorbit")};

    EXPECT_EQ("showorbit", metadata.name);
    EXPECT_EQ(ParFile::ParameterType::YES_NO, metadata.type);
    ASSERT_TRUE(metadata.format);
    EXPECT_EQ(ParFile::ParameterFormat::RAW, *metadata.format);
    ASSERT_TRUE(metadata.default_curve);
    EXPECT_EQ(ParFile::Curve::HOLD, *metadata.default_curve);
    ASSERT_TRUE(metadata.extrapolate);
    EXPECT_EQ(ParFile::ExtrapolateMode::CLAMP, *metadata.extrapolate);
    EXPECT_TRUE(metadata.values.empty());
}

TEST(TestParameterCatalog, showdotMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{core_catalog()};
    const ParFile::ParameterMetadata &metadata{catalog.metadata("showdot")};

    EXPECT_EQ("showdot", metadata.name);
    EXPECT_EQ(ParFile::ParameterType::STRING, metadata.type);
    ASSERT_TRUE(metadata.format);
    EXPECT_EQ(ParFile::ParameterFormat::SLASH, *metadata.format);
    ASSERT_TRUE(metadata.default_curve);
    EXPECT_EQ(ParFile::Curve::HOLD, *metadata.default_curve);
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

TEST(TestParameterCatalog, functionMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{core_catalog()};
    const ParFile::ParameterMetadata &metadata{catalog.metadata("function")};

    EXPECT_EQ("function", metadata.name);
    EXPECT_EQ(ParFile::ParameterType::FUNCTION_LIST, metadata.type);
    ASSERT_TRUE(metadata.format);
    EXPECT_EQ(ParFile::ParameterFormat::SLASH_LIST, *metadata.format);
    ASSERT_TRUE(metadata.default_curve);
    EXPECT_EQ(ParFile::Curve::HOLD, *metadata.default_curve);
    ASSERT_TRUE(metadata.extrapolate);
    EXPECT_EQ(ParFile::ExtrapolateMode::CLAMP, *metadata.extrapolate);
    ASSERT_EQ(31U, metadata.values.size());
    EXPECT_NE(metadata.values.end(), std::find(metadata.values.begin(), metadata.values.end(), "sin"));
    EXPECT_NE(metadata.values.end(), std::find(metadata.values.begin(), metadata.values.end(), "round"));
}

TEST(TestParameterCatalog, orbitdrawmodeFunctionValueIsOrbitModeEnum)
{
    const ParFile::ParameterCatalog catalog{core_catalog()};
    const ParFile::ParameterMetadata &metadata{catalog.metadata("orbitdrawmode")};

    EXPECT_EQ(ParFile::ParameterType::ENUM, metadata.type);
    EXPECT_NE(metadata.values.end(), std::find(metadata.values.begin(), metadata.values.end(), "function"));
}

TEST(TestParameterCatalog, xyshiftMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{id_3d_catalog()};
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

TEST(TestParameterCatalog, id3DViewMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{id_3d_catalog()};
    const ParFile::ParameterMetadata &rotation{catalog.metadata("rotation")};
    const ParFile::ParameterMetadata &perspective{catalog.metadata("perspective")};
    const ParFile::ParameterMetadata &scalexyz{catalog.metadata("scalexyz")};
    const ParFile::ParameterMetadata &sphere{catalog.metadata("sphere")};
    const ParFile::ParameterMetadata &longitude{catalog.metadata("longitude")};
    const ParFile::ParameterMetadata &stereo{catalog.metadata("stereo")};
    const ParFile::ParameterMetadata &light_source{catalog.metadata("lightsource")};
    const ParFile::ParameterMetadata &ray{catalog.metadata("ray")};

    EXPECT_EQ(42U, catalog.parameters.size());
    EXPECT_EQ(ParFile::ParameterType::NUMERIC_TUPLE, rotation.type);
    ASSERT_TRUE(rotation.arity);
    EXPECT_EQ(3, *rotation.arity);
    EXPECT_EQ(ParFile::ParameterType::INTEGER, perspective.type);
    ASSERT_TRUE(perspective.format);
    EXPECT_EQ(ParFile::ParameterFormat::RAW, *perspective.format);
    EXPECT_EQ(ParFile::ParameterType::NUMERIC_TUPLE, scalexyz.type);
    ASSERT_TRUE(scalexyz.arity);
    EXPECT_EQ(3, *scalexyz.arity);
    EXPECT_EQ(ParFile::ParameterType::YES_NO, sphere.type);
    EXPECT_TRUE(sphere.values.empty());
    EXPECT_EQ(ParFile::ParameterType::NUMERIC_TUPLE, longitude.type);
    ASSERT_TRUE(longitude.arity);
    EXPECT_EQ(2, *longitude.arity);
    EXPECT_EQ(ParFile::ParameterType::INTEGER, stereo.type);
    ASSERT_TRUE(stereo.min);
    ASSERT_TRUE(stereo.max);
    EXPECT_EQ(0.0, *stereo.min);
    EXPECT_EQ(4.0, *stereo.max);
    EXPECT_EQ(ParFile::ParameterType::NUMERIC_TUPLE, light_source.type);
    ASSERT_TRUE(light_source.arity);
    EXPECT_EQ(3, *light_source.arity);
    EXPECT_EQ(ParFile::ParameterType::STRING, ray.type);
}

TEST(TestParameterCatalog, julibrotViewMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{id_3d_catalog()};
    const ParFile::ParameterMetadata &mode{catalog.metadata("3dmode")};
    const ParFile::ParameterMetadata &geometry{catalog.metadata("julibrot3d")};
    const ParFile::ParameterMetadata &eyes{catalog.metadata("julibroteyes")};
    const ParFile::ParameterMetadata &from_to{catalog.metadata("julibrotfromto")};

    EXPECT_EQ(ParFile::ParameterType::ENUM, mode.type);
    ASSERT_EQ(4U, mode.values.size());
    EXPECT_EQ("monocular", mode.values[0]);
    EXPECT_EQ("red-blue", mode.values[3]);
    EXPECT_EQ(ParFile::ParameterType::NUMERIC_TUPLE, geometry.type);
    ASSERT_TRUE(geometry.arity);
    EXPECT_EQ(6, *geometry.arity);
    EXPECT_EQ(ParFile::ParameterType::DOUBLE, eyes.type);
    ASSERT_TRUE(eyes.format);
    EXPECT_EQ(ParFile::ParameterFormat::RAW, *eyes.format);
    EXPECT_EQ(ParFile::ParameterType::NUMERIC_TUPLE, from_to.type);
    ASSERT_TRUE(from_to.arity);
    EXPECT_EQ(4, *from_to.arity);
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
    EXPECT_EQ(1, *metadata.min);
    EXPECT_EQ(2100000000, *metadata.max);
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
    EXPECT_NE(metadata.values.end(), std::find(metadata.values.begin(), metadata.values.end(), "maxiter"));
    EXPECT_NE(metadata.values.end(), std::find(metadata.values.begin(), metadata.values.end(), "bof60"));
    EXPECT_NE(metadata.values.end(), std::find(metadata.values.begin(), metadata.values.end(), "epsiloncross"));
    EXPECT_EQ(metadata.values.end(), std::find(metadata.values.begin(), metadata.values.end(), "real"));
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
    ASSERT_TRUE(metadata.min);
    ASSERT_TRUE(metadata.max);
    EXPECT_EQ(0, *metadata.min);
    EXPECT_EQ(255, *metadata.max);
    ASSERT_EQ(8U, metadata.values.size());
    EXPECT_NE(metadata.values.end(), std::find(metadata.values.begin(), metadata.values.end(), "iter"));
    EXPECT_NE(metadata.values.end(), std::find(metadata.values.begin(), metadata.values.end(), "tdis"));
    EXPECT_EQ(metadata.values.end(), std::find(metadata.values.begin(), metadata.values.end(), "maxiter"));
}

TEST(TestParameterCatalog, fillcolorMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{core_catalog()};
    const ParFile::ParameterMetadata &metadata{catalog.metadata("fillcolor")};

    EXPECT_EQ(ParFile::ParameterType::INTEGER_OR_ENUM, metadata.type);
    ASSERT_TRUE(metadata.min);
    EXPECT_EQ(0, *metadata.min);
    ASSERT_EQ(1U, metadata.values.size());
    EXPECT_EQ("normal", metadata.values[0]);
}

TEST(TestParameterCatalog, initorbitMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{core_catalog()};
    const ParFile::ParameterMetadata &metadata{catalog.metadata("initorbit")};

    EXPECT_EQ(ParFile::ParameterType::NUMERIC_TUPLE_OR_ENUM, metadata.type);
    ASSERT_TRUE(metadata.arity);
    EXPECT_EQ(2, *metadata.arity);
    ASSERT_EQ(1U, metadata.values.size());
    EXPECT_EQ("pixel", metadata.values[0]);
}

TEST(TestParameterCatalog, invertMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{core_catalog()};
    const ParFile::ParameterMetadata &metadata{catalog.metadata("invert")};

    EXPECT_EQ(ParFile::ParameterType::NUMERIC_TUPLE, metadata.type);
    ASSERT_TRUE(metadata.arity);
    EXPECT_EQ(3, *metadata.arity);
}

TEST(TestParameterCatalog, mathtoleranceMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{core_catalog()};
    const ParFile::ParameterMetadata &metadata{catalog.metadata("mathtolerance")};

    EXPECT_EQ(ParFile::ParameterType::NUMERIC_TUPLE, metadata.type);
    ASSERT_TRUE(metadata.arity);
    EXPECT_EQ(2, *metadata.arity);
}

TEST(TestParameterCatalog, miimMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{core_catalog()};
    const ParFile::ParameterMetadata &metadata{catalog.metadata("miim")};

    EXPECT_EQ(ParFile::ParameterType::MIIM, metadata.type);
    ASSERT_TRUE(metadata.format);
    EXPECT_EQ(ParFile::ParameterFormat::SLASH, *metadata.format);
    ASSERT_TRUE(metadata.default_curve);
    EXPECT_EQ(ParFile::Curve::HOLD, *metadata.default_curve);
}

TEST(TestParameterCatalog, potentialMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{core_catalog()};
    const ParFile::ParameterMetadata &metadata{catalog.metadata("potential")};

    EXPECT_EQ(ParFile::ParameterType::POTENTIAL, metadata.type);
    ASSERT_TRUE(metadata.format);
    EXPECT_EQ(ParFile::ParameterFormat::SLASH, *metadata.format);
    ASSERT_TRUE(metadata.default_curve);
    EXPECT_EQ(ParFile::Curve::LINEAR, *metadata.default_curve);
}

TEST(TestParameterCatalog, distestMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{coloring_catalog()};
    const ParFile::ParameterMetadata &metadata{catalog.metadata("distest")};

    EXPECT_EQ(ParFile::ParameterType::INTEGER_TUPLE, metadata.type);
    ASSERT_TRUE(metadata.arity);
    EXPECT_EQ(2, *metadata.arity);
}

TEST(TestParameterCatalog, decompMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{coloring_catalog()};
    const ParFile::ParameterMetadata &metadata{catalog.metadata("decomp")};

    EXPECT_EQ(ParFile::ParameterType::INTEGER, metadata.type);
    ASSERT_TRUE(metadata.format);
    EXPECT_EQ(ParFile::ParameterFormat::RAW, *metadata.format);
    ASSERT_TRUE(metadata.default_curve);
    EXPECT_EQ(ParFile::Curve::LINEAR, *metadata.default_curve);
    ASSERT_TRUE(metadata.extrapolate);
    EXPECT_EQ(ParFile::ExtrapolateMode::CLAMP, *metadata.extrapolate);
    ASSERT_TRUE(metadata.min);
    EXPECT_EQ(2, *metadata.min);
    ASSERT_TRUE(metadata.max);
    EXPECT_EQ(256, *metadata.max);
}

TEST(TestParameterCatalog, truemodeMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{coloring_catalog()};
    const ParFile::ParameterMetadata &metadata{catalog.metadata("truemode")};

    EXPECT_EQ(ParFile::ParameterType::ENUM, metadata.type);
    ASSERT_TRUE(metadata.format);
    EXPECT_EQ(ParFile::ParameterFormat::RAW, *metadata.format);
    ASSERT_TRUE(metadata.default_curve);
    EXPECT_EQ(ParFile::Curve::HOLD, *metadata.default_curve);
    ASSERT_TRUE(metadata.extrapolate);
    EXPECT_EQ(ParFile::ExtrapolateMode::CLAMP, *metadata.extrapolate);
    ASSERT_EQ(2U, metadata.values.size());
    EXPECT_EQ("def", metadata.values[0]);
    EXPECT_EQ("iter", metadata.values[1]);
}

TEST(TestParameterCatalog, passesMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{core_catalog()};
    const ParFile::ParameterMetadata &metadata{catalog.metadata("passes")};

    EXPECT_EQ(ParFile::ParameterType::ENUM, metadata.type);
    EXPECT_NE(metadata.values.end(), std::find(metadata.values.begin(), metadata.values.end(), "1"));
    EXPECT_NE(metadata.values.end(), std::find(metadata.values.begin(), metadata.values.end(), "d"));
    EXPECT_NE(metadata.values.end(), std::find(metadata.values.begin(), metadata.values.end(), "g6"));
}

TEST(TestParameterCatalog, periodicityMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{core_catalog()};
    const ParFile::ParameterMetadata &metadata{catalog.metadata("periodicity")};

    EXPECT_EQ(ParFile::ParameterType::INTEGER_OR_ENUM, metadata.type);
    EXPECT_NE(metadata.values.end(), std::find(metadata.values.begin(), metadata.values.end(), "no"));
    EXPECT_NE(metadata.values.end(), std::find(metadata.values.begin(), metadata.values.end(), "show"));
    EXPECT_NE(metadata.values.end(), std::find(metadata.values.begin(), metadata.values.end(), "yes"));
    ASSERT_TRUE(metadata.min);
    EXPECT_EQ(-255, *metadata.min);
    ASSERT_TRUE(metadata.max);
    EXPECT_EQ(255, *metadata.max);
}

TEST(TestParameterCatalog, logmapMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{coloring_catalog()};
    const ParFile::ParameterMetadata &metadata{catalog.metadata("logmap")};

    EXPECT_EQ(ParFile::ParameterType::INTEGER_OR_ENUM, metadata.type);
    EXPECT_NE(metadata.values.end(), std::find(metadata.values.begin(), metadata.values.end(), "no"));
    EXPECT_NE(metadata.values.end(), std::find(metadata.values.begin(), metadata.values.end(), "old"));
    EXPECT_NE(metadata.values.end(), std::find(metadata.values.begin(), metadata.values.end(), "yes"));
}

TEST(TestParameterCatalog, typedCatalogFindsMetadataByName)
{
    const ParFile::ParameterCatalog catalog{typed_catalog()};
    const ParFile::ParameterMetadata &metadata{catalog.metadata("maxiter")};

    EXPECT_EQ(ParFile::ParameterType::INTEGER, metadata.type);
}

class ParamsSlotMetadataTest : public ::testing::TestWithParam<ParamsSlotMetadataCase>
{
};

TEST_P(ParamsSlotMetadataTest, paramsSlotMetadataLoads)
{
    const ParamsSlotMetadataCase &expected{GetParam()};
    const ParFile::ParameterCatalog catalog{core_catalog()};
    const ParFile::ParamsSlotMetadata &slot{catalog.params_slot(expected.fractal_type, expected.slot)};

    EXPECT_EQ(expected.slot, slot.index);
    EXPECT_EQ(expected.name, slot.name);
    EXPECT_EQ("params[" + std::to_string(expected.slot) + "]", slot.metadata.name);
    EXPECT_EQ(expected.type, slot.metadata.type);
    ASSERT_TRUE(slot.metadata.format);
    EXPECT_EQ(ParFile::ParameterFormat::RAW, *slot.metadata.format);
    ASSERT_TRUE(slot.metadata.default_curve);
    EXPECT_EQ(expected.default_curve, *slot.metadata.default_curve);
    ASSERT_TRUE(slot.metadata.extrapolate);
    EXPECT_EQ(ParFile::ExtrapolateMode::CLAMP, *slot.metadata.extrapolate);

    if (expected.min)
    {
        ASSERT_TRUE(slot.metadata.min);
        EXPECT_DOUBLE_EQ(*expected.min, *slot.metadata.min);
    }
    else
    {
        EXPECT_FALSE(slot.metadata.min);
    }

    if (expected.max)
    {
        ASSERT_TRUE(slot.metadata.max);
        EXPECT_DOUBLE_EQ(*expected.max, *slot.metadata.max);
    }
    else
    {
        EXPECT_FALSE(slot.metadata.max);
    }
}

INSTANTIATE_TEST_SUITE_P(TestParameterCatalog, ParamsSlotMetadataTest, ::testing::ValuesIn(PARAMS_SLOT_METADATA_CASES),
    params_slot_metadata_test_name);

class FunctionSlotMetadataTest : public ::testing::TestWithParam<FunctionSlotMetadataCase>
{
};

TEST_P(FunctionSlotMetadataTest, functionSlotMetadataLoads)
{
    const FunctionSlotMetadataCase &expected{GetParam()};
    const ParFile::ParameterCatalog catalog{core_catalog()};
    const ParFile::FunctionSlotMetadata &function{catalog.function_slot(expected.fractal_type, expected.slot)};

    EXPECT_EQ(expected.name, function.name);
    EXPECT_EQ("function[" + std::to_string(expected.slot) + "]", function.metadata.name);
    EXPECT_EQ(ParFile::ParameterType::ENUM, function.metadata.type);
    EXPECT_EQ(expected.slot, function.slot);
    ASSERT_TRUE(function.metadata.format);
    EXPECT_EQ(ParFile::ParameterFormat::RAW, *function.metadata.format);
    ASSERT_TRUE(function.metadata.default_curve);
    EXPECT_EQ(ParFile::Curve::HOLD, *function.metadata.default_curve);
    ASSERT_TRUE(function.metadata.extrapolate);
    EXPECT_EQ(ParFile::ExtrapolateMode::CLAMP, *function.metadata.extrapolate);
    EXPECT_NE(function.metadata.values.end(),
        std::find(function.metadata.values.begin(), function.metadata.values.end(), "sin"));
    EXPECT_NE(function.metadata.values.end(),
        std::find(function.metadata.values.begin(), function.metadata.values.end(), "round"));
}

INSTANTIATE_TEST_SUITE_P(TestParameterCatalog, FunctionSlotMetadataTest,
    ::testing::ValuesIn(FUNCTION_SLOT_METADATA_CASES), function_slot_metadata_test_name);

class ParamsGroupMetadataTest : public ::testing::TestWithParam<ParamsGroupMetadataCase>
{
};

TEST_P(ParamsGroupMetadataTest, paramsGroupMetadataLoads)
{
    const ParamsGroupMetadataCase &expected{GetParam()};
    const ParFile::ParameterCatalog catalog{core_catalog()};
    const ParFile::ParamsGroupMetadata &group{catalog.params_group(expected.fractal_type, expected.name)};

    EXPECT_EQ(expected.name, group.name);
    EXPECT_EQ(expected.metadata_name, group.metadata.name);
    EXPECT_EQ(ParFile::ParameterType::COMPLEX, group.metadata.type);
    ASSERT_TRUE(group.metadata.format);
    EXPECT_EQ(ParFile::ParameterFormat::SLASH_PAIR, *group.metadata.format);
    ASSERT_EQ(2U, group.slots.size());
    EXPECT_EQ(expected.first_slot, group.slots[0]);
    EXPECT_EQ(expected.second_slot, group.slots[1]);
}

INSTANTIATE_TEST_SUITE_P(TestParameterCatalog, ParamsGroupMetadataTest,
    ::testing::ValuesIn(PARAMS_GROUP_METADATA_CASES), params_group_metadata_test_name);

class FormulaEntryMetadataTest : public ::testing::TestWithParam<FormulaEntryMetadataCase>
{
};

TEST_P(FormulaEntryMetadataTest, formulaCatalogUsesIdFrmEntry)
{
    const FormulaEntryMetadataCase &expected{GetParam()};
    const ParFile::ParameterCatalog catalog{formula_catalog()};

    ASSERT_EQ(std::size(FORMULA_ENTRY_METADATA_CASES), catalog.formula_entries.size());

    const auto is_formula{
        [&](const ParFile::FormulaEntryMetadata &metadata) { return metadata.name == expected.name; }};
    EXPECT_NE(catalog.formula_entries.end(),
        std::find_if(catalog.formula_entries.begin(), catalog.formula_entries.end(), is_formula));
}

INSTANTIATE_TEST_SUITE_P(TestParameterCatalog, FormulaEntryMetadataTest,
    ::testing::ValuesIn(FORMULA_ENTRY_METADATA_CASES), formula_entry_metadata_test_name);

TEST(TestParameterCatalog, formulaParamsBailoutKnobMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{formula_catalog()};
    const ParFile::FormulaParamsKnobMetadata &knob{catalog.formula_params_knob("Larry", "bailout")};

    EXPECT_EQ("bailout", knob.name);
    EXPECT_EQ("Larry.bailout", knob.metadata.name);
    EXPECT_EQ(ParFile::ParameterType::DOUBLE, knob.metadata.type);
    ASSERT_TRUE(knob.metadata.format);
    EXPECT_EQ(ParFile::ParameterFormat::RAW, *knob.metadata.format);
    ASSERT_EQ(1U, knob.slots.size());
    EXPECT_EQ(2, knob.slots[0]);
}

TEST(TestParameterCatalog, formulaParamsComplexKnobMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{formula_catalog()};
    const ParFile::FormulaParamsKnobMetadata &knob{catalog.formula_params_knob("Larry", "fractal parameter")};

    EXPECT_EQ("fractal parameter", knob.name);
    EXPECT_EQ("Larry.fractal parameter", knob.metadata.name);
    EXPECT_EQ(ParFile::ParameterType::COMPLEX, knob.metadata.type);
    ASSERT_TRUE(knob.metadata.format);
    EXPECT_EQ(ParFile::ParameterFormat::SLASH_PAIR, *knob.metadata.format);
    ASSERT_EQ(2U, knob.slots.size());
    EXPECT_EQ(0, knob.slots[0]);
    EXPECT_EQ(1, knob.slots[1]);
}

TEST(TestParameterCatalog, formulaParamsIntegerKnobMetadataLoads)
{
    const ParFile::ParameterCatalog catalog{
        ParFile::read_parameter_catalog(formula_catalog_text(R"("type":"integer","variable":"p3.real")"))};
    const ParFile::FormulaParamsKnobMetadata &knob{catalog.formula_params_knob("foo", "x")};

    EXPECT_EQ(ParFile::ParameterType::INTEGER, knob.metadata.type);
    ASSERT_EQ(1U, knob.slots.size());
    EXPECT_EQ(4, knob.slots[0]);
}

class FormulaFunctionMetadataTest : public ::testing::TestWithParam<FormulaFunctionMetadataCase>
{
};

TEST_P(FormulaFunctionMetadataTest, formulaFunctionMetadataLoads)
{
    const FormulaFunctionMetadataCase &expected{GetParam()};
    const ParFile::ParameterCatalog catalog{formula_catalog()};
    const ParFile::FormulaFunctionMetadata &function{catalog.formula_function(expected.formula_name, expected.name)};

    EXPECT_EQ(expected.name, function.name);
    EXPECT_EQ(std::string{expected.formula_name} + "." + expected.name, function.metadata.name);
    EXPECT_EQ(ParFile::ParameterType::ENUM, function.metadata.type);
    EXPECT_EQ(expected.slot, function.slot);
    ASSERT_TRUE(function.metadata.format);
    EXPECT_EQ(ParFile::ParameterFormat::RAW, *function.metadata.format);
    ASSERT_TRUE(function.metadata.default_curve);
    EXPECT_EQ(ParFile::Curve::HOLD, *function.metadata.default_curve);
    EXPECT_NE(function.metadata.values.end(),
        std::find(function.metadata.values.begin(), function.metadata.values.end(), "sin"));
    EXPECT_NE(function.metadata.values.end(),
        std::find(function.metadata.values.begin(), function.metadata.values.end(), "round"));
}

INSTANTIATE_TEST_SUITE_P(TestParameterCatalog, FormulaFunctionMetadataTest,
    ::testing::ValuesIn(FORMULA_FUNCTION_METADATA_CASES), formula_function_metadata_test_name);

TEST(TestParameterCatalog, legalParameterTypeStringsDecode)
{
    EXPECT_EQ(ParFile::ParameterType::CENTER_MAG, read_metadata(R"("type":"center-mag")").type);
    EXPECT_EQ(ParFile::ParameterType::COLOR_MAP, read_metadata(R"("type":"color-map")").type);
    EXPECT_EQ(ParFile::ParameterType::COMPLEX, read_metadata(R"("type":"complex")").type);
    EXPECT_EQ(ParFile::ParameterType::CORNERS, read_metadata(R"("type":"corners")").type);
    EXPECT_EQ(ParFile::ParameterType::DOUBLE, read_metadata(R"("type":"double")").type);
    EXPECT_EQ(ParFile::ParameterType::ENUM, read_metadata(R"("type":"enum","values":["a"])").type);
    EXPECT_EQ(
        ParFile::ParameterType::FUNCTION_LIST, read_metadata(R"("type":"function-list","values":"id-functions")").type);
    EXPECT_EQ(ParFile::ParameterType::YES_NO, read_metadata(R"("type":"yes-no")").type);
    EXPECT_EQ(ParFile::ParameterType::INSIDE, read_metadata(R"("type":"inside","values":["maxiter"])").type);
    EXPECT_EQ(ParFile::ParameterType::INTEGER, read_metadata(R"("type":"integer")").type);
    EXPECT_EQ(
        ParFile::ParameterType::INTEGER_OR_ENUM, read_metadata(R"("type":"integer-or-enum","values":["a"])").type);
    EXPECT_EQ(ParFile::ParameterType::INTEGER_TUPLE, read_metadata(R"("type":"integer-tuple")").type);
    EXPECT_EQ(ParFile::ParameterType::MIIM, read_metadata(R"("type":"miim")").type);
    EXPECT_EQ(ParFile::ParameterType::NUMERIC_TUPLE, read_metadata(R"("type":"numeric-tuple")").type);
    EXPECT_EQ(ParFile::ParameterType::NUMERIC_TUPLE_OR_ENUM,
        read_metadata(R"("type":"numeric-tuple-or-enum","values":["a"])").type);
    EXPECT_EQ(ParFile::ParameterType::OUTSIDE, read_metadata(R"("type":"outside","values":["iter"])").type);
    EXPECT_EQ(ParFile::ParameterType::POINT2, read_metadata(R"("type":"point2")").type);
    EXPECT_EQ(ParFile::ParameterType::POINT3, read_metadata(R"("type":"point3")").type);
    EXPECT_EQ(ParFile::ParameterType::POTENTIAL, read_metadata(R"("type":"potential")").type);
    EXPECT_EQ(ParFile::ParameterType::STRING, read_metadata(R"("type":"string")").type);
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

TEST(TestParameterCatalog, optionalDescriptionDecodes)
{
    const ParFile::ParameterMetadata described{read_metadata(R"("type":"integer","description":"Help text.")")};
    const ParFile::ParameterMetadata undescribed{read_metadata(R"("type":"integer")")};

    EXPECT_EQ("Help text.", described.description);
    EXPECT_TRUE(undescribed.description.empty());
}

TEST(TestParameterCatalog, optionalDescriptionLoadsForAllMetadataShapes)
{
    const ParFile::ParameterCatalog catalog{ParFile::read_parameter_catalog(R"({
  "parameters": {
    "x": { "type": "integer", "description": "Top parameter help." }
  },
  "fractal-types": {
    "foo": {
      "params": {
        "slots": [
          { "index": 0, "name": "real", "type": "double", "description": "Slot help." },
          { "index": 1, "name": "imag", "type": "double" }
        ],
        "groups": {
          "c": {
            "type": "complex",
            "description": "Group help.",
            "slots": [ 0, 1 ]
          }
        }
      },
      "functions": {
        "fn1": {
          "type": "enum",
          "values": "id-functions",
          "description": "Fractal function help."
        }
      }
    }
  },
  "formula-entries": {
    "bar": {
      "params": {
        "knobs": {
          "knob": {
            "type": "real",
            "variable": "p1.real",
            "description": "Knob help."
          }
        }
      },
      "functions": {
        "fn1": {
          "type": "enum",
          "values": "id-functions",
          "description": "Formula function help."
        }
      }
    }
  }
})")};

    EXPECT_EQ("Top parameter help.", catalog.metadata("x").description);
    EXPECT_EQ("Slot help.", catalog.params_slot("foo", 0).metadata.description);
    EXPECT_EQ("Group help.", catalog.params_group("foo", "c").metadata.description);
    EXPECT_EQ("Fractal function help.", catalog.function_slot("foo", 0).metadata.description);
    EXPECT_EQ("Knob help.", catalog.formula_params_knob("bar", "knob").metadata.description);
    EXPECT_EQ("Formula function help.", catalog.formula_function("bar", "fn1").metadata.description);
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
    EXPECT_THROW(ParFile::read_parameter_catalog(catalog_text(R"("type":"integer-or-enum")")), std::runtime_error);
    EXPECT_THROW(
        ParFile::read_parameter_catalog(catalog_text(R"("type":"numeric-tuple-or-enum")")), std::runtime_error);
    EXPECT_THROW(ParFile::read_parameter_catalog(catalog_text(R"("type":"outside")")), std::runtime_error);
    EXPECT_NO_THROW(ParFile::read_parameter_catalog(catalog_text(R"("type":"yes-no")")));
    EXPECT_NO_THROW(ParFile::read_parameter_catalog(catalog_text(R"("type":"string")")));
}

TEST(TestParameterCatalog, valuesRequireDiscreteType)
{
    EXPECT_THROW(
        ParFile::read_parameter_catalog(catalog_text(R"("type":"integer","values":["a"])")), std::runtime_error);
    EXPECT_THROW(
        ParFile::read_parameter_catalog(catalog_text(R"("type":"yes-no","values":["yes"])")), std::runtime_error);
}

TEST(TestParameterCatalog, invalidEnumValuesRejected)
{
    EXPECT_THROW(ParFile::read_parameter_catalog(catalog_text(R"("type":"enum","values":"a")")), std::runtime_error);
    EXPECT_THROW(
        ParFile::read_parameter_catalog(catalog_text(R"("type":"enum","values":["a", 1])")), std::runtime_error);
}

TEST(TestParameterCatalog, invalidFunctionListValuesRejected)
{
    EXPECT_THROW(ParFile::read_parameter_catalog(catalog_text(R"("type":"function-list")")), std::runtime_error);
    EXPECT_THROW(ParFile::read_parameter_catalog(catalog_text(R"("type":"function-list","values":["sin"])")),
        std::runtime_error);
    EXPECT_THROW(ParFile::read_parameter_catalog(catalog_text(R"("type":"function-list","values":"unknown")")),
        std::runtime_error);
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

TEST(TestParameterCatalog, unknownFunctionSlotRejected)
{
    EXPECT_THROW(core_catalog().function_slot("mandelfn", 1), std::runtime_error);
}

TEST(TestParameterCatalog, unknownFormulaParamsKnobRejected)
{
    EXPECT_THROW(formula_catalog().formula_params_knob("Larry", "unknown"), std::runtime_error);
}

TEST(TestParameterCatalog, unknownFormulaFunctionRejected)
{
    EXPECT_THROW(formula_catalog().formula_function("Larry", "fn5"), std::runtime_error);
}

TEST(TestParameterCatalog, unknownFormulaFunctionValuesRejected)
{
    EXPECT_THROW(ParFile::read_parameter_catalog(formula_function_catalog_text(R"("type":"enum","values":"unknown")")),
        std::runtime_error);
}

TEST(TestParameterCatalog, unknownFractalFunctionValuesRejected)
{
    EXPECT_THROW(ParFile::read_parameter_catalog(fractal_function_catalog_text(R"("type":"enum","values":"unknown")")),
        std::runtime_error);
}

TEST(TestParameterCatalog, invalidFormulaFunctionNameRejected)
{
    EXPECT_THROW(
        ParFile::read_parameter_catalog("{\"parameters\":{},\"formula-entries\":{\"foo\":{\"functions\":{\"fn5\":{"
                                        "\"type\":\"enum\",\"values\":\"id-functions\"}}}}}"),
        std::runtime_error);
}

TEST(TestParameterCatalog, invalidFractalFunctionNameRejected)
{
    EXPECT_THROW(
        ParFile::read_parameter_catalog("{\"parameters\":{},\"fractal-types\":{\"foo\":{\"functions\":{\"fn5\":{"
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
