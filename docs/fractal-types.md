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

- Convert prompt prefixes into type hints: `+` and `#` mean integer.
  These prefixes are internal code annotations; drop them from catalog
  descriptions. Unprefixed numeric prompts should start as double unless
  source audit proves an enum or string shape.
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

- `burning-ship` (`BURNING_SHIP`)
  - `params[0]`: Real portion of p1; default `0`.
  - `params[1]`: Imaginary portion of p1; default `0`.
  - `params[2]`: degree (2-5); default `2`.
  - `function=`: none.
