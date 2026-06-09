# Parameter Description Plan

## Goal

Add a user-facing `description` string to every existing catalog metadata
parameter. GUI tools and generated documentation can display this text as
help for animation targets.

## Sources

- Built-in Id parameters must use Id help files as the primary source.
- Built-in fractal `params=` slots must use the prompt strings in
  `fractalp.cpp` as primary description source material. Id help files
  are the second source when they add context.
- `fractalp.cpp` is also the source for built-in fractal defaults,
  function flags, and extra `params=` slots.
- Formula parameter knobs must use formula files as the primary source.
  Prefer formula comments, named variables, and nearby explanatory text.
- Function slots should use Id help for `function=` behavior and formula
  comments when a formula assigns special meaning to `fn1` through `fn4`.
- If the source has no usable prose, write a concise factual description
  from audited code behavior. Keep the wording conservative and tied to
  the audited source.

## Scope

- Top-level parameter metadata in `data/core-catalog.json`.
- Top-level parameter metadata in `data/coloring-catalog.json`.
- Top-level parameter metadata in `data/id-3d-catalog.json`.
- Fractal type `params.slots` metadata in `data/core-catalog.json`.
- Fractal type `params.groups` metadata in `data/core-catalog.json`.
- Fractal type `functions.fn*` metadata in `data/core-catalog.json`.
- Formula `params.knobs` metadata in `data/formula-catalog.json`.
- Formula `functions.fn*` metadata in `data/formula-catalog.json`.

Do not add descriptions to structural schema nodes, catalog containers, or
config schema fields in this plan. JSON Schema already has its own
`description` fields.

## Description Rules

- Describe what the parameter changes for the user, not its JSON type.
- Prefer one sentence.
- Keep terms consistent with Id help unless the catalog already exposes a
  clearer user-facing name.
- Mention units, ranges, or discrete modes when the source text makes them
  important.
- Do not repeat the parameter name as the whole description.
- Do not invent mathematical meaning for anonymous `a`, `b`, `c`, `p1`,
  or `p2` parameters. Use source text or describe them as formula or
  fractal coefficients.
- Keep descriptions stable under interpolation; avoid wording that only
  describes a single static value.

## Implementation Rules

- Add `std::string description` to `ParFile::ParameterMetadata`.
- Parse `description` from every JSON object that becomes
  `ParameterMetadata`.
- Keep `description` optional during the migration slices.
- After all catalog data is populated, make `description` required in the
  schema and loader for every metadata object that produces
  `ParameterMetadata`.
- Add tests that prove descriptions load for top-level parameters,
  fractal params slots, fractal params groups, function slots, formula
  knobs, and formula functions.
- Add a final completeness test that fails on any loaded metadata object
  with an empty description.

## Work Items

Each item is one reviewable slice. Remove an item after it lands.

- Top-level core parameters
  - Audit Id help for each `data/core-catalog.json` top-level parameter.
  - Add sourced descriptions to `parameters.*`.
  - Add focused catalog tests for representative entries.

- Coloring parameters
  - Audit Id help for `data/coloring-catalog.json`.
  - Add sourced descriptions to all coloring metadata parameters.
  - Add focused catalog tests for representative entries.

- 3D parameters
  - Audit Id help for `data/id-3d-catalog.json`.
  - Add sourced descriptions to all 3D metadata parameters.
  - Add focused catalog tests for representative entries.

- Core fractal params A through C
  - Audit Id help and `fractalp.cpp` for fractal types from `ant`
    through `complexnewton`.
  - Add descriptions to each params slot and params group.

- Core fractal params D through H
  - Audit Id help and `fractalp.cpp` for fractal types from `diffusion`
    through `hypercomplexj`.
  - Add descriptions to each params slot and params group.

- Core fractal params I through L
  - Audit Id help and `fractalp.cpp` for fractal types from `icons`
    through `lyapunov`.
  - Add descriptions to each params slot and params group.

- Core fractal params M
  - Audit Id help and `fractalp.cpp` for `magnet*`, `mandel*`,
    `man*`, `marks*`, and `martin`.
  - Add descriptions to each params slot and params group.

- Core fractal params N through P
  - Audit Id help and `fractalp.cpp` for fractal types from `newtbasin`
    through `popcornjul`.
  - Add descriptions to each params slot and params group.

- Core fractal params Q through Z
  - Audit Id help and `fractalp.cpp` for fractal types from
    `quadruptwo` through `volterra-lotka`.
  - Add descriptions to each params slot and params group.

- Core fractal function slots
  - Audit Id help for `function=` and `fractalp.cpp` where functions
    have special meaning.
  - Add descriptions to all built-in fractal `functions.fn*` metadata.

- Formula params and functions
  - Audit the formula files for every entry in
    `data/formula-catalog.json`.
  - Add descriptions to each formula params knob.
  - Add descriptions to formula function slots using formula comments
    when present.

- Enforce completeness
  - Make `description` required in every catalog schema metadata
    definition.
  - Make the loader reject missing or empty descriptions.
  - Add completeness tests that walk all loaded catalog metadata.
  - Remove the completed description item from `docs/todo.md`.
