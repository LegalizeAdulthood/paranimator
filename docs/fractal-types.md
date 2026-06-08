# Fractal Type Catalog Plan

Source:

`D:\src\legalize\id\iterated-dynamics\libid\fractals\fractalp.cpp`

## Scope

- Catalog built-in, non-formula fractal types that expose at least one
  `params=` slot in `g_fractal_specific` or `g_more_fractal_params`.
- Skip the formula fractal type; formula entries are planned separately in
  `docs/formulas.md`.
- Skip `test`; it is a developer harness for new built-in fractal types
  and has no useful paranimator behavior.
- Skip fractal types that have no `params=` slots. For included types,
  still record whether `FractalFlags::TRIGn` exposes `function=` slots.
- Use the active Id CMake configuration. `RANDOM_RUN` is not defined by
  `ID_TARGET_DEFINITIONS`, target compile definitions, or headers, so the
  active `julia_inverse` entry is the `#else` branch.
- Use `type_has_param` behavior: empty prompt strings are absent, the
  first four prompts map to `params[0]` through `params[3]`, and
  `g_more_fractal_params` maps to `params[4]` through `params[9]`.

## Catalog Rules

- Convert prompt prefixes into type hints: `+` means integer and `#`
  means U32. These prefixes are internal code annotations; drop them
  from catalog descriptions. Unprefixed numeric prompts should start as
  double unless source audit proves an enum or string shape.
- Name obvious real/imag pairs as complex groups while retaining
  addressable scalar slots.
- Keep source prompt text in comments or tests until semantic names are
  confirmed from the fractal implementation.
- For `FractalFlags::TRIGn`, expose `n` function slots backed by the
  shared `id-functions` enum. Do not add function-only fractal types to
  this plan.
- For `g_more_fractal_params` entries, catalog the extra slots as normal
  `params=` slots after the first four.
- Add completeness tests that compare this plan against `fractalp.cpp` and
  state explicit omissions.

## Work Items

Each item names the Id type, slots to catalog, default source values,
extra slots when present, and function slots when present. Remove an
item after the catalog entry and focused tests land.

- `complexnewton` (`COMPLEX_NEWTON`)
  - `params[0]`: Real part of Degree; default `3`.
  - `params[1]`: Imag part of Degree; default `0`.
  - `params[2]`: Real part of Root; default `1`.
  - `params[3]`: Imag part of Root; default `0`.
  - `function=`: none.

- `complexbasin` (`COMPLEX_BASIN`)
  - `params[0]`: Real part of Degree; default `3`.
  - `params[1]`: Imag part of Degree; default `0`.
  - `params[2]`: Real part of Root; default `1`.
  - `params[3]`: Imag part of Root; default `0`.
  - `function=`: none.

- `cmplxmarksmand` (`COMPLEX_MARKS_MAND`)
  - `params[0]`: Real Perturbation of Z(0); default `0`.
  - `params[1]`: Imaginary Perturbation of Z(0); default `0`.
  - `params[2]`: Real part of Exponent; default `1`.
  - `params[3]`: Imag part of Exponent; default `0`.
  - `function=`: none.

- `cmplxmarksjul` (`COMPLEX_MARKS_JUL`)
  - `params[0]`: Real Part of Parameter; default `0.3`.
  - `params[1]`: Imaginary Part of Parameter; default `0.6`.
  - `params[2]`: Real part of Exponent; default `1`.
  - `params[3]`: Imag part of Exponent; default `0`.
  - `function=`: none.

- `lambda` (`LAMBDA`)
  - `params[0]`: Real Part of Parameter; default `0.85`.
  - `params[1]`: Imaginary Part of Parameter; default `0.6`.
  - `function=`: none.

- `barnsleym1` (`BARNSLEY_M1`)
  - `params[0]`: Real Perturbation of Z(0); default `0`.
  - `params[1]`: Imaginary Perturbation of Z(0); default `0`.
  - `function=`: none.

- `barnsleyj1` (`BARNSLEY_J1`)
  - `params[0]`: Real Part of Parameter; default `0.6`.
  - `params[1]`: Imaginary Part of Parameter; default `1.1`.
  - `function=`: none.

- `barnsleym2` (`BARNSLEY_M2`)
  - `params[0]`: Real Perturbation of Z(0); default `0`.
  - `params[1]`: Imaginary Perturbation of Z(0); default `0`.
  - `function=`: none.

- `barnsleyj2` (`BARNSLEY_J2`)
  - `params[0]`: Real Part of Parameter; default `0.6`.
  - `params[1]`: Imaginary Part of Parameter; default `1.1`.
  - `function=`: none.

- `barnsleym3` (`BARNSLEY_M3`)
  - `params[0]`: Real Perturbation of Z(0); default `0`.
  - `params[1]`: Imaginary Perturbation of Z(0); default `0`.
  - `function=`: none.

- `barnsleyj3` (`BARNSLEY_J3`)
  - `params[0]`: Real Part of Parameter; default `0.1`.
  - `params[1]`: Imaginary Part of Parameter; default `0.36`.
  - `function=`: none.

- `mandellambda` (`MANDEL_LAMBDA`)
  - `params[0]`: Real Perturbation of Z(0); default `0`.
  - `params[1]`: Imaginary Perturbation of Z(0); default `0`.
  - `function=`: none.

- `gingerbreadman` (`GINGERBREAD`)
  - `params[0]`: Initial x; default `-.1`.
  - `params[1]`: Initial y; default `0`.
  - `function=`: none.

- `diffusion` (`DIFFUSION`)
  - `params[0]`: +Border size; default `10`.
  - `params[1]`: +Type (0=Central,1=Falling,2=Square Cavity); default `0`.
  - `params[2]`: +Color change rate (0=Random); default `0`.
  - `function=`: none.

- `spider` (`SPIDER`)
  - `params[0]`: Real Perturbation of Z(0); default `0`.
  - `params[1]`: Imaginary Perturbation of Z(0); default `0`.
  - `function=`: none.

- `tetrate` (`TETRATE`)
  - `params[0]`: Real Perturbation of Z(0); default `0`.
  - `params[1]`: Imaginary Perturbation of Z(0); default `0`.
  - `function=`: none.

- `magnet1m` (`MAGNET_1M`)
  - `params[0]`: Real Perturbation of Z(0); default `0`.
  - `params[1]`: Imaginary Perturbation of Z(0); default `0`.
  - `function=`: none.

- `magnet1j` (`MAGNET_1J`)
  - `params[0]`: Real Part of Parameter; default `0`.
  - `params[1]`: Imaginary Part of Parameter; default `0`.
  - `function=`: none.

- `magnet2m` (`MAGNET_2M`)
  - `params[0]`: Real Perturbation of Z(0); default `0`.
  - `params[1]`: Imaginary Perturbation of Z(0); default `0`.
  - `function=`: none.

- `magnet2j` (`MAGNET_2J`)
  - `params[0]`: Real Part of Parameter; default `0`.
  - `params[1]`: Imaginary Part of Parameter; default `0`.
  - `function=`: none.

- `biflambda` (`BIF_LAMBDA`)
  - `params[0]`: +Filter Cycles; default `1000.0`.
  - `params[1]`: Seed Population; default `0.66`.
  - `function=`: `fn1`.

- `bif+sinpi` (`BIF_PLUS_SIN_PI`)
  - `params[0]`: +Filter Cycles; default `1000.0`.
  - `params[1]`: Seed Population; default `0.66`.
  - `function=`: `fn1`.

- `bif=sinpi` (`BIF_EQ_SIN_PI`)
  - `params[0]`: +Filter Cycles; default `1000.0`.
  - `params[1]`: Seed Population; default `0.66`.
  - `function=`: `fn1`.

- `popcornjul` (`POPCORN_JUL`)
  - `params[0]`: Step size (real); default `0.05`.
  - `params[1]`: Step size (imaginary); default `0`.
  - `params[2]`: Constant C (real); default `3.00`.
  - `params[3]`: Constant C (imaginary); default `0`.
  - `function=`: `fn1/fn2/fn3/fn4`.

- `lsystem` (`L_SYSTEM`)
  - `params[0]`: +Order; default `2`.
  - `function=`: none.

- `manowarj` (`MAN_O_WAR_J`)
  - `params[0]`: Real Part of Parameter; default `0`.
  - `params[1]`: Imaginary Part of Parameter; default `0`.
  - `function=`: none.

- `fn(z)+fn(pix)` (`FN_PLUS_FN_PIX`)
  - `params[0]`: Real Perturbation of Z(0); default `0`.
  - `params[1]`: Imaginary Perturbation of Z(0); default `0`.
  - `params[2]`: Real Coefficient Second Function; default `1`.
  - `params[3]`: Imag Coefficient Second Function; default `0`.
  - `function=`: `fn1/fn2`.

- `marksmandelpwr` (`MARKS_MANDEL_PWR`)
  - `params[0]`: Real Perturbation of Z(0); default `0`.
  - `params[1]`: Imaginary Perturbation of Z(0); default `0`.
  - `function=`: `fn1`.

- `tim's_error` (`TIMS_ERROR`)
  - `params[0]`: Real Perturbation of Z(0); default `0`.
  - `params[1]`: Imaginary Perturbation of Z(0); default `0`.
  - `function=`: `fn1`.

- `bifstewart` (`BIF_STEWART`)
  - `params[0]`: +Filter Cycles; default `1000.0`.
  - `params[1]`: Seed Population; default `0.66`.
  - `function=`: `fn1`.

- `hopalong` (`HOPALONG`)
  - `params[0]`: a; default `.4`.
  - `params[1]`: b; default `1`.
  - `params[2]`: c; default `0`.
  - `function=`: none.

- `circle` (`CIRCLE`)
  - `params[0]`: magnification; default `200000L`.
  - `function=`: none.

- `martin` (`MARTIN`)
  - `params[0]`: a; default `3.14`.
  - `function=`: none.

- `lyapunov` (`LYAPUNOV`)
  - `params[0]`: +Order (integer); default `0`.
  - `params[1]`: Population Seed; default `0.5`.
  - `params[2]`: +Filter Cycles; default `0`.
  - `function=`: none.

- `lorenz3d1` (`LORENZ_3D1`)
  - `params[0]`: Time Step; default `.02`.
  - `params[1]`: a; default `5`.
  - `params[2]`: b; default `15`.
  - `params[3]`: c; default `1`.
  - `function=`: none.

- `lorenz3d3` (`LORENZ_3D3`)
  - `params[0]`: Time Step; default `.02`.
  - `params[1]`: a; default `10`.
  - `params[2]`: b; default `28`.
  - `params[3]`: c; default `2.66`.
  - `function=`: none.

- `lorenz3d4` (`LORENZ_3D4`)
  - `params[0]`: Time Step; default `.02`.
  - `params[1]`: a; default `10`.
  - `params[2]`: b; default `28`.
  - `params[3]`: c; default `2.66`.
  - `function=`: none.

- `lambda(fn||fn)` (`LAMBDA_FN_FN`)
  - `params[0]`: Real Part of Parameter; default `1`.
  - `params[1]`: Imaginary Part of Parameter; default `0.1`.
  - `params[2]`: Function Shift Value; default `1`.
  - `function=`: `fn1/fn2`.

- `julia(fn||fn)` (`JUL_FN_FN`)
  - `params[0]`: Real Part of Parameter; default `0`.
  - `params[1]`: Imaginary Part of Parameter; default `0`.
  - `params[2]`: Function Shift Value; default `8`.
  - `function=`: `fn1/fn2`.

- `manlam(fn||fn)` (`MAN_LAM_FN_FN`)
  - `params[0]`: Real Perturbation of Z(0); default `0`.
  - `params[1]`: Imaginary Perturbation of Z(0); default `0`.
  - `params[2]`: Function Shift Value; default `10`.
  - `function=`: `fn1/fn2`.

- `mandel(fn||fn)` (`MAN_FN_FN`)
  - `params[0]`: Real Perturbation of Z(0); default `0`.
  - `params[1]`: Imaginary Perturbation of Z(0); default `0`.
  - `params[2]`: Function Shift Value; default `0.5`.
  - `function=`: `fn1/fn2`.

- `bifmay` (`BIF_MAY`)
  - `params[0]`: +Filter Cycles; default `300.0`.
  - `params[1]`: Seed Population; default `0.9`.
  - `params[2]`: Beta >= 2; default `5`.
  - `function=`: none.

- `halley` (`HALLEY`)
  - `params[0]`: +Order (integer > 1); default `6`.
  - `params[1]`: Real Relaxation coefficient; default `1.0`.
  - `params[2]`: Epsilon; default `0.0001`.
  - `params[3]`: Imag Relaxation coefficient; default `0`.
  - `function=`: none.

- `dynamic` (`DYNAMIC`)
  - `params[0]`: +# of intervals (<0 = connect); default `50`.
  - `params[1]`: time step (<0 = Euler); default `.1`.
  - `params[2]`: a; default `1`.
  - `params[3]`: b; default `3`.
  - `function=`: `fn1`.

- `quat` (`QUAT`)
  - `params[0]`: notused; default `0`.
  - `params[1]`: notused; default `0`.
  - `params[2]`: cj; default `0`.
  - `params[3]`: ck; default `0`.
  - `function=`: none.

- `quatjul` (`QUAT_JUL`)
  - `params[0]`: c1; default `-.745`.
  - `params[1]`: ci; default `0`.
  - `params[2]`: cj; default `.113`.
  - `params[3]`: ck; default `.05`.
  - extra `params[4]`: zj; default `0`.
  - extra `params[5]`: zk; default `0`.
  - `function=`: none.

- `cellular` (`CELLULAR`)
  - `params[0]`: #Initial String | 0 = Random | -1 = Reuse Last Random;
    default `11.0`.
  - `params[1]`: #Rule = # of digits (see below) | 0 = Random; default
    `3311100320.0`.
  - `params[2]`: +Type (see below); default `41.0`.
  - `params[3]`: #Starting Row Number; default `0`.
  - `function=`: none.

- `julia_inverse` (`INVERSE_JULIA`)
  - `params[0]`: Real Part of Parameter; default `-0.11`.
  - `params[1]`: Imaginary Part of Parameter; default `0.6557`.
  - `params[2]`: Max Hits per Pixel; default `4`.
  - `function=`: none.

- `mandelcloud` (`MANDEL_CLOUD`)
  - `params[0]`: +# of intervals (<0 = connect); default `50`.
  - `function=`: none.

- `phoenix` (`PHOENIX`)
  - `params[0]`: Real portion of p1; default `0.56667`.
  - `params[1]`: Real portion of p2; default `-0.5`.
  - `params[2]`: Degree = 0 | >= 2 | <= -3; default `0`.
  - `function=`: none.

- `mandphoenix` (`MAND_PHOENIX`)
  - `params[0]`: Real Perturbation of Z(0); default `0.0`.
  - `params[1]`: Imaginary Perturbation of Z(0); default `0.0`.
  - `params[2]`: Degree = 0 | >= 2 | <= -3; default `0`.
  - `function=`: none.

- `hypercomplex` (`HYPER_CMPLX`)
  - `params[0]`: notused; default `0`.
  - `params[1]`: notused; default `0`.
  - `params[2]`: cj; default `0`.
  - `params[3]`: ck; default `0`.
  - `function=`: `fn1`.

- `hypercomplexj` (`HYPER_CMPLX_J`)
  - `params[0]`: c1; default `-.745`.
  - `params[1]`: ci; default `0`.
  - `params[2]`: cj; default `.113`.
  - `params[3]`: ck; default `.05`.
  - extra `params[4]`: zj; default `0`.
  - extra `params[5]`: zk; default `0`.
  - `function=`: `fn1`.

- `frothybasin` (`FROTHY_BASIN`)
  - `params[0]`: +Apply mapping once (1) or twice (2); default `1`.
  - `params[1]`: +Enter non-zero value for alternate color shading; default
    `0`.
  - `params[2]`: A (imaginary part of C); default `1.028713768218725`.
  - `function=`: none.

- `mandel4` (`MANDEL4`)
  - `params[0]`: Real Perturbation of Z(0); default `0`.
  - `params[1]`: Imaginary Perturbation of Z(0); default `0`.
  - `function=`: none.

- `julia4` (`JULIA4`)
  - `params[0]`: Real Part of Parameter; default `0.6`.
  - `params[1]`: Imaginary Part of Parameter; default `0.55`.
  - `function=`: none.

- `marksmandel` (`MARKS_MANDEL`)
  - `params[0]`: Real Perturbation of Z(0); default `0`.
  - `params[1]`: Imaginary Perturbation of Z(0); default `0`.
  - `params[2]`: Real part of Exponent; default `1`.
  - `function=`: none.

- `marksjulia` (`MARKS_JULIA`)
  - `params[0]`: Real Part of Parameter; default `0.1`.
  - `params[1]`: Imaginary Part of Parameter; default `0.9`.
  - `params[2]`: Real part of Exponent; default `1`.
  - `function=`: none.

- `icons` (`ICON`)
  - `params[0]`: Lambda; default `-2.34`.
  - `params[1]`: Alpha; default `2.0`.
  - `params[2]`: Beta; default `0.2`.
  - `params[3]`: Gamma; default `0.1`.
  - extra `params[4]`: Omega; default `0`.
  - extra `params[5]`: +Degree of symmetry; default `3`.
  - `function=`: none.

- `icons3d` (`ICON_3D`)
  - `params[0]`: Lambda; default `-2.34`.
  - `params[1]`: Alpha; default `2.0`.
  - `params[2]`: Beta; default `0.2`.
  - `params[3]`: Gamma; default `0.1`.
  - extra `params[4]`: Omega; default `0`.
  - extra `params[5]`: +Degree of symmetry; default `3`.
  - `function=`: none.

- `phoenixcplx` (`PHOENIX_CPLX`)
  - `params[0]`: Real portion of p1; default `0.2`.
  - `params[1]`: Imaginary portion of p1; default `0`.
  - `params[2]`: Real portion of p2; default `0.3`.
  - `params[3]`: Imaginary portion of p2; default `0`.
  - extra `params[4]`: Degree = 0 | >= 2 | <= -3; default `0`.
  - `function=`: none.

- `mandphoenixclx` (`MAND_PHOENIX_CPLX`)
  - `params[0]`: Real Perturbation of Z(0); default `0`.
  - `params[1]`: Imaginary Perturbation of Z(0); default `0`.
  - `params[2]`: Real portion of p2; default `0.5`.
  - `params[3]`: Imaginary portion of p2; default `0`.
  - extra `params[4]`: Degree = 0 | >= 2 | <= -3; default `0`.
  - `function=`: none.

- `ant` (`ANT`)
  - `params[0]`: #Rule String (1's and non-1's, 0 rand); default `1100`.
  - `params[1]`: #Maxpts; default `1.0E9`.
  - `params[2]`: +Numants (max 256); default `1`.
  - `params[3]`: +Ant type (1 or 2); default `1`.
  - extra `params[4]`: +Wrap?; default `1`.
  - extra `params[5]`: +Random Seed Value (0 = Random, 1 = Reuse Last);
    default `0`.
  - `function=`: none.

- `chip` (`CHIP`)
  - `params[0]`: a; default `-15`.
  - `params[1]`: b; default `-19`.
  - `params[2]`: c; default `1`.
  - `function=`: none.

- `quadruptwo` (`QUADRUP_TWO`)
  - `params[0]`: a; default `34`.
  - `params[1]`: b; default `1`.
  - `params[2]`: c; default `5`.
  - `function=`: none.

- `threeply` (`THREEPLY`)
  - `params[0]`: a; default `-55`.
  - `params[1]`: b; default `-1`.
  - `params[2]`: c; default `-42`.
  - `function=`: none.

- `volterra-lotka` (`VL`)
  - `params[0]`: h; default `0.739`.
  - `params[1]`: p; default `0.739`.
  - `function=`: none.

- `escher_julia` (`ESCHER`)
  - `params[0]`: Real Part of Parameter; default `0.32`.
  - `params[1]`: Imaginary Part of Parameter; default `0.043`.
  - `function=`: none.

- `latoocarfian` (`LATOO`)
  - `params[0]`: a; default `-0.966918`.
  - `params[1]`: b; default `2.879879`.
  - `params[2]`: c; default `0.765145`.
  - `params[3]`: d; default `0.744728`.
  - `function=`: `fn1/fn2/fn3/fn4`.

- `dividebrot5` (`DIVIDE_BROT5`)
  - `params[0]`: a; default `2.0`.
  - `params[1]`: b; default `0.0`.
  - `function=`: none.

- `mandelbrotmix4` (`MANDELBROT_MIX4`)
  - `params[0]`: Real portion of p1; default `0.05`.
  - `params[1]`: Imaginary portion of p1; default `3`.
  - `params[2]`: Real portion of p2; default `-1.5`.
  - `params[3]`: Imaginary portion of p2; default `-2`.
  - extra `params[4]`: Real portion of p3; default `0`.
  - extra `params[5]`: Imaginary portion of p3; default `0`.
  - `function=`: `fn1`.

- `burning-ship` (`BURNING_SHIP`)
  - `params[0]`: Real portion of p1; default `0`.
  - `params[1]`: Imaginary portion of p1; default `0`.
  - `params[2]`: degree (2-5); default `2`.
  - `function=`: none.
