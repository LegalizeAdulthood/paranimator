# Mandel Demo Plan

## Rules

- One demo JSON file per slice. When the slice is complete, remove it
  from this plan and renumber the remaining slices.
- Base source stays `type=mandel`.
- Every demo is 60 seconds at 60 fps: `num-frames` is `3600`.
- Every demo uses `video` value `CF4`.
- The animation title is the parameter or effect named by the slice.
- Each demo carries that title and a short animation description. If the
  JSON schema lacks metadata support, add that support before the first
  demo JSON.
- Include `core-catalog.json`, `coloring-catalog.json`, and
  `id-3d-catalog.json` only when the slice needs them.

## Scope

The Mandel-specific animated parameter is `params.z0`, backed by slots
`params.z0-real` and `params.z0-imag`. This series also covers shared
catalog parameters that can affect a `type=mandel` render: viewport,
escape, coloring, orbit display, inversion, precision, and 3D
post-transform controls.

Selector-only or non-Mandel knobs are not demo targets: `type`, `reset`,
`formulafile`, `formulaname`, `function`, `ifs`, `ifs3d`, `ifsfile`,
`ismand`, `lfile`, `lname`, `miim`, `orbitname`, `3dmode`,
`julibrot3d`, `julibroteyes`, and `julibrotfromto`. If later evidence
shows visible `type=mandel` behavior for one of these, add it as a new
slice.

## Interpolation Coverage

- `geometric`: covered by `center-mag`.
- `linear`: covered by `corners`, `params.z0`, and numeric tracks.
- `hold`: covered by `invert`, `inside`, `outside`, and map names.
- `step`: covered by `passes`, `bailoutest`, and `logmode`.
- `pwm`: covered by `showorbit`, `truecolor`, and `sphere`.

## Slices

1. `01-center-mag.json`
   Title: `center-mag`.
   Description: Simple geometric zoom from the full Mandelbrot set into
   Seahorse Valley, with `maxiter` rising to preserve edge detail.
   Tracks: `center-mag`, `maxiter`.
   Coverage: `geometric`, `linear`.

2. `02-corners.json`
   Title: `corners`.
   Description: Linear pan and crop across the main cardioid using the
   rectangle form of `corners`.
   Tracks: `corners`.
   Coverage: `linear`.

3. `03-camera2d-path.json`
   Title: `camera2d path`.
   Description: Bezier camera move that glides through minibrots while
   writing `center-mag`.
   Tracks: `camera2d` `look-at`, `eye`, `view-up`, `height`.
   Coverage: `linear`, `bezier` path.

4. `04-camera2d-skew.json`
   Title: `camera2d skew`.
   Description: Camera roll and skew over a filament, with `aspectdrift`
   varying to show when Id accepts aspect changes.
   Tracks: `camera2d` `view-up`, `skew`, `height`, `aspectdrift`.
   Coverage: `linear`.

5. `05-invert.json`
   Title: `invert`.
   Description: The inversion circle grows from a hidden portal into a
   full transformed view, then snaps back at the end.
   Tracks: `invert`.
   Coverage: `linear`, `hold`.

6. `06-params-z0.json`
   Title: `params.z0`.
   Description: Lissajous perturbation of the Mandelbrot starting orbit,
   paired with `initorbit` to compare pixel start and explicit start.
   Tracks: `params.z0`, `initorbit`.
   Coverage: `linear`, `lissajous` path, `step`.

7. `07-colors.json`
   Title: `colors`.
   Description: Generated color table morph from deep blues to hot
   copper, emitted through `colors=@...`.
   Tracks: `colors`.
   Coverage: `linear`.

8. `08-color-map-effects.json`
   Title: `color-map effects`.
   Description: One generated palette demonstrates hue shift, brightness,
   contrast, gamma, saturation, reverse, remap, pulse, sparkle, mask
   blend, and ping-pong.
   Tracks: `colors` effects.
   Coverage: `linear`, `hold`, `step`.

9. `09-map-ranges.json`
   Title: `map and ranges`.
   Description: Hold-swapped source maps and keyed `ranges` turn smooth
   escape bands into deliberate stripes.
   Tracks: `map`, `ranges`.
   Coverage: `hold`.

10. `10-maxiter.json`
    Title: `maxiter`.
    Description: Iteration ceiling climbs through a deep zoom so the image
    visibly resolves from fog to filigree.
    Tracks: `maxiter`.
    Coverage: `linear`.

11. `11-bailout.json`
    Title: `bailout`.
    Description: Escape radius and bailout test change together, showing
    how different tests reshape exterior contours.
    Tracks: `bailout`, `bailoutest`.
    Coverage: `linear`, `step`.

12. `12-precision.json`
    Title: `bfdigits`.
    Description: Deep zoom precision handoff, with arbitrary precision
    digits, math tolerance, and periodicity checking changing in stages.
    Tracks: `bfdigits`, `mathtolerance`, `periodicity`.
    Coverage: `linear`, `step`.

13. `13-inside-outside.json`
    Title: `inside and outside`.
    Description: Interior and exterior coloring methods switch through
    named modes while `proximity` and `decomp` animate numeric emphasis.
    Tracks: `inside`, `outside`, `proximity`, `decomp`.
    Coverage: `hold`, `linear`.

14. `14-potential-logmap.json`
    Title: `potential`.
    Description: Continuous potential coloring fades in while logarithmic
    mapping and log mode step through their visible regimes.
    Tracks: `potential`, `logmap`, `logmode`.
    Coverage: `linear`, `step`.

15. `15-distest.json`
    Title: `distest`.
    Description: Distance estimator bands sweep across the boundary, with
    legacy DEM colors and BOF handling toggled for contrast.
    Tracks: `distest`, `olddemmcolors`, `nobof`.
    Coverage: `linear`, `step`.

16. `16-biomorph.json`
    Title: `biomorph`.
    Description: Biomorph color and fill color move from subtle outline to
    saturated interior accent.
    Tracks: `biomorph`, `fillcolor`.
    Coverage: `linear`, `hold`.

17. `17-truecolor.json`
    Title: `truecolor`.
    Description: Truecolor output controls switch on and change mode while
    the palette remains stable.
    Tracks: `truecolor`, `truemode`.
    Coverage: `pwm`, `step`.

18. `18-orbit-display.json`
    Title: `showorbit`.
    Description: Orbit plotting fades in by PWM while passes and screen
    coordinate locking switch around a moving viewport.
    Tracks: `showorbit`, `passes`, `screencoords`.
    Coverage: `pwm`, `step`.

19. `19-orbit-drawing.json`
    Title: `orbitdrawmode`.
    Description: Orbit rectangle, point cadence, delay, draw mode, and dot
    marker animate over the same Mandelbrot view.
    Tracks: `orbitcorners`, `orbitdrawmode`, `orbitinterval`,
    `orbitdelay`, `showdot`.
    Coverage: `linear`, `hold`, `step`.

20. `20-compatibility-controls.json`
    Title: `symmetry`.
    Description: Symmetry, finite attractor search, and random seed vary
    as a stability pass for catalog-accepted Mandel controls.
    Tracks: `symmetry`, `finattract`, `rseed`.
    Coverage: `hold`, `linear`.

21. `21-id-3d-view.json`
    Title: `rotation`.
    Description: Mandelbrot image becomes a low-relief 3D surface with
    camera rotation, perspective, xy shift, scale, and roughness changes.
    Tracks: `3d`, `rotation`, `perspective`, `xyshift`, `scalexyz`,
    `roughness`.
    Coverage: `linear`, `step`.

22. `22-id-3d-preview.json`
    Title: `preview`.
    Description: Preview and bounding-box controls toggle while coarse
    sampling and fill type change during a terrain spin.
    Tracks: `preview`, `showbox`, `coarse`, `filltype`, `brief`.
    Coverage: `linear`, `step`.

23. `23-lightsource.json`
    Title: `lightsource`.
    Description: Light sweeps across the Mandelbrot terrain while full
    color, haze, smoothing, waterline, randomization, grayscale, and
    background controls shift.
    Tracks: `lightsource`, `ambient`, `bright`, `background`, `haze`,
    `fullcolor`, `smoothing`, `randomize`, `usegrayscale`, `waterline`.
    Coverage: `linear`, `step`.

24. `24-stereo-crop.json`
    Title: `stereo`.
    Description: Stereo separation and convergence increase, with crop,
    monitor width, stereo width, interocular, and xy adjustment changing.
    Tracks: `stereo`, `interocular`, `stereowidth`, `converge`,
    `monitorwidth`, `crop`, `xyadjust`.
    Coverage: `linear`, `step`.

25. `25-sphere.json`
    Title: `sphere`.
    Description: Spherical projection turns on, then radius, latitude, and
    longitude sweep the Mandelbrot image around a globe.
    Tracks: `sphere`, `radius`, `latitude`, `longitude`.
    Coverage: `pwm`, `linear`.

26. `26-3d-output-controls.json`
    Title: `3D output controls`.
    Description: Output-facing 3D controls change in a gentle terrain shot
    so generated side effects can be inspected without changing the base
    Mandelbrot animation.
    Tracks: `filename`, `ray`, `rds`, `rds-texture`, `targa_overlay`,
    `transparent`.
    Coverage: `hold`, `step`.

## Done Per Slice

- JSON validates against the config schema.
- Generated par output contains 3600 frame entries.
- Generated render script uses `video=CF4`.
- Title and description match the demonstrated parameter or effect.
- First, middle, and last frame render without Id errors.
