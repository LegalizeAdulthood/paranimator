# ParAnimator Parameter Timeline Design

## Goal

Extend ParAnimator from a two-endpoint viewport interpolator into a
data-driven fractal animation sequencer.

The central idea is:

- one global frame clock
- one independent timeline per animated parameter
- parameter metadata loaded from JSON
- typed tracks for interpolation and formatting
- no hard-coded Iterated Dynamics parameter names in the animator

ParAnimator should not know that maxiter, params, colors, inside, or
lightsource exist. It should know how to animate declared parameter
types.

## Current Limitation

The existing model is essentially:

- load one source parameter set
- load one destination parameter set
- interpolate selected fields
- write one parameter set per frame

That works for center-mag and corners, but it does not scale to richer
animations. Interesting fractal animation requires independently timed
changes to formula parameters, iteration limits, color controls, lighting,
orbit traps, and rendering modes.

## Core Model

Each generated frame is:

- a copy of a base parameter set
- plus the value of every active parameter track at that frame
- plus batch/rendering parameters needed by Iterated Dynamics

Conceptually:

    frame = base_parameter_set

    for each track:
        frame[track.parameter] = track.value_at(frame_number)

    write frame

This preserves the current output style while replacing the old
interpolant list with a more general track list.

## Global Clock

The animation has one global frame range:

- Frame 0 through num-frames - 1.

Every track is evaluated against that same frame number. Tracks do not
need to share keyframe locations.

## Independent Parameter Timelines

Each parameter has its own timeline. This is important because unrelated
parameters should not be forced to change at the same frames.

Example conceptual timing:

| Parameter | Timing |
| --- | --- |
| `params` for `type=julia` | 0:A; 300:B; 900:A |
| `maxiter` | 0:100; 120:100; 500:2000; 900:2000 |
| `colors` | 0:`fire.map`; 900:`ice.map` |
| `lightsource` | 200:`-1/-1/1`; 700:`1/1/1` |

These are independent rhythms:

- Julia shape mutates over the shot.
- Iteration detail fades in after a delay.
- The colormap changes over the shot.
- Lighting sweep happens only in the middle.

A global keyframe list would make this awkward. Per-parameter timelines
make it direct.

## Parameter Catalog

The available parameters are configured by a JSON catalog. The catalog
associates each named Iterated Dynamics parameter with metadata.

The animator hard-codes parameter types, curves, parsers, formatters, and
validation rules. It does not hard-code Iterated Dynamics parameter names.

Example parameter catalog:

    {
      "parameters": {
        "center-mag": {
          "type": "center-mag",
          "description": "Viewport center and magnification",
          "default-curve": "geometric",
          "extrapolate": "clamp"
        },

        "corners": {
          "type": "corners",
          "description": "Viewport rectangle",
          "default-curve": "linear",
          "extrapolate": "clamp"
        },

        "maxiter": {
          "type": "integer",
          "min": 1,
          "max": 2147483647,
          "default-curve": "step",
          "rounding": "nearest",
          "extrapolate": "clamp"
        },

        "bailout": {
          "type": "double",
          "min": 0.0,
          "default-curve": "smoothstep",
          "extrapolate": "clamp"
        },

        "colors": {
          "type": "color-map",
          "format": "at-file",
          "default-curve": "smoothstep",
          "extrapolate": "clamp"
        },

        "inside": {
          "type": "inside",
          "values": [
            "maxiter",
            "zmag",
            "bof60",
            "bof61",
            "epsiloncross",
            "startrail",
            "period",
            "atan",
            "fmod"
          ],
          "default-curve": "hold",
          "extrapolate": "clamp",
          "min": 0,
          "max": 255
        },

        "outside": {
          "type": "outside",
          "values": [
            "iter",
            "real",
            "imag",
            "mult",
            "summ",
            "atan",
            "fmod",
            "tdis"
          ],
          "default-curve": "hold",
          "extrapolate": "clamp",
          "min": 0,
          "max": 255
        },

        "lightsource": {
          "type": "point3",
          "format": "slash",
          "default-curve": "smoothstep",
          "extrapolate": "clamp"
        }
      }
    }

## Catalog Metadata Fields

| Field Name | Meaning |
| --- | --- |
| `type` | Selects parser, interpolator, and formatter. |
| `description` | Human-readable parameter explanation. |
| `format` | Text syntax used in par files. |
| `default-curve` | Curve used by segments unless overridden. |
| `extrapolate` | Behavior outside the keyed range. |
| `min` | Lower validation limit; may clamp. |
| `max` | Upper validation limit; may clamp. |
| `arity` | Component count for tuple-like values. |
| `values` | Legal string values for enum, inside, and outside parameters. |
| `rounding` | Rule for producing integer-like values. |
| `units` | Unit hint: degrees, radians, raw, percent. |
| `normalize` | Normalize vector-like values after interpolation. |
| `aliases` | Alternative parameter names accepted in par files. |
| `required` | Parameter must exist in the base parameter set. |
| `write-when-unchanged` | Write value even when it matches base. |

## Type Versus Format

Type and format should be separate.

Do not make a separate type for every textual spelling.

Example:

    {
      "type": "complex",
      "format": "slash-pair"
    }

Possible formats:

| Type | Format |
| --- | --- |
| `integer` | `raw` |
| `double` | `raw` |
| `complex` | `slash-pair` |
| `numeric-tuple` | `slash` |
| `point2` | `slash` |
| `vector2` | `slash` |
| `point3` | `slash` |
| `vector3` | `slash` |
| `color` | `color-spec` |
| `color-map` | `at-file` |
| `angle` | `degrees` |
| `angle` | `radians` |

This lets typed interpolation stay separate from Id parameter formatting.

For Id parameter files, complex values use slash-separated real and
imaginary parts. The help documents examples such as
`params=-0.480/0.626` and `initorbit=nnn/nnn`. Comma-pair notation appears
in formula-language prose, not in parameter syntax.

## Fractal-Specific Params

`params=` is a slash-delimited vector, not one semantic parameter. Id
stores up to ten values in `g_params[]`. The active fractal type decides
which indexes exist and what each index means.

Id's `type_has_param()` checks the first four parameter names from the
fractal-specific table, then checks extra names from
`g_more_fractal_params`. For `type=formula`, unused formula parameters
are suppressed. Id's `put_fractal_params()` writes one `params=` command
through the highest parameter index used by the active type.

ParAnimator should model `params` as a context-scoped parameter vector.
The context comes from:

- `type` in the source par entry
- `orbitname` or orbit type for Julibrot
- `formulaname` plus user catalog overrides for formulas
- the layer source, when rendering layered animations

Catalogs may describe slots and named groups for fractal types whose
`params=` values have stable type-specific meaning:

    {
      "fractal-types": {
        "julia": {
          "params": {
            "format": "slash-list",
            "slots": [
              { "index": 0, "name": "c-real", "type": "double" },
              { "index": 1, "name": "c-imag", "type": "double" }
            ],
            "groups": {
              "c": {
                "type": "complex",
                "slots": [ 0, 1 ],
                "format": "slash-pair"
              }
            }
          }
        }
      }
    }

For `type=formula`, the assignment of `params=` values to formula
variables is fixed by Id and is not catalog metadata:

| Variable | Values |
| --- | --- |
| `p1` | `params[0]` and `params[1]` |
| `p2` | `params[2]` and `params[3]` |
| `p3` | `params[4]` and `params[5]` |
| `p4` | `params[6]` and `params[7]` |

These variable names cannot be changed. Formula authors often use `p1`,
`p2`, `p3`, and `p4` as complex values. They also often use the real and
imaginary pieces as unrelated knobs. ParAnimator should not infer those
meanings from the formula source. User or bundled metadata should attach
human-readable names that describe how the formula entry uses the fixed
`p1` through `p4` variables:

    {
      "formula-entries": {
        "MandelbrotMix4": {
          "params": {
            "knobs": {
              "bailout": {
                "type": "real",
                "variable": "p1.real"
              },
              "scale factor": {
                "type": "real",
                "variable": "p1.imag"
              },
              "c": {
                "type": "complex",
                "variable": "p2"
              }
            }
          },
          "functions": {
            "fn1": { "type": "enum", "values": "id-functions" },
            "fn2": { "type": "enum", "values": "id-functions" },
            "fn3": { "type": "enum", "values": "id-functions" },
            "fn4": { "type": "enum", "values": "id-functions" }
          }
        }
      }
    }

Tracks may target formula-entry knobs such as `MandelbrotMix4.bailout`,
`MandelbrotMix4["scale factor"]`, or `MandelbrotMix4.c`. These names are
ParAnimator metadata describing the formula entry's use of fixed Id
variables. Id never sees them, and the formula source still refers only to
`p1`, `p2`, `p3`, and `p4`.

Each formula params knob is keyed by its human-readable name and declares:

| Field | Meaning |
| --- | --- |
| `type` | `integer`, `real`, or `complex`. |
| `variable` | Fixed formula variable or component being described. |

Integer and real knobs may bind to `pN.real` or `pN.imag`, where
1 <= N <= 4. Complex knobs may bind only to `pN`, where 1 <= N <= 4.
The metadata annotates the fixed binding; it does not create or rename
formula variables.

When a source has `type=formula`, ParAnimator resolves the active
`formulaname` value and treats it as the formula entry name. Params knob
metadata is looked up from the matching `formula-entries` entry. If no
metadata exists for that entry name, only raw `params.p1`,
`params.p1.real`, and related `p1` through `p4` variable targets are
available.

If two tracks write overlapping formula variables or components,
validation must reject the animation unless the overlap is exactly the
same declared knob. This keeps `params.p1`, `params.p1.real`, and
formula-specific knobs from fighting over the same generated `params=`
value.

Formula entry metadata may also expose fixed function keys backed by
`function=fn1/fn2/fn3/fn4`. The key names are always `fn1`, `fn2`, `fn3`,
and `fn4`; formula metadata cannot rename them. Tracks target formula
entry keys such as `MandelbrotMix4.fn1`. Each function key is an enum
using the fixed `id-functions` value set. The writer starts from the
source par entry or Id reset defaults, applies function key updates, and
emits one slash-delimited `function=` assignment through the highest
required function key.

The fixed `id-functions` enum contains the function names recognized by
Id: `sin`, `cos`, `tan`, `cotan`, `sinh`, `cosh`, `tanh`, `cotanh`,
`exp`, `log`, `sqr`, `recip`, `ident`, `cosxx`, `flip`, `conj`, `zero`,
`one`, `asin`, `asinh`, `acos`, `acosh`, `atan`, `atanh`, `sqrt`, `abs`,
`cabs`, `floor`, `ceil`, `trunc`, and `round`.

Id leaves omitted `function=` values unchanged. ParAnimator should still
compose from known base/default values before writing, so generated
frames do not depend on prior process state.

For non-formula params vectors, tracks may target the whole vector, an
indexed slot such as `params[0]`, or a named group such as `params.c`. The
writer starts from the source par entry's base `params`, applies all slot
and group updates, then emits one slash-delimited `params=` assignment
through the highest required slot. This preserves untouched values. Do not
emit partial `params` assignments, because Id treats omitted values as
zero after a `params=` command.

For the first implementation, a layer's fractal type must be stable when
it has params tracks. If `type` is animated and the reachable types do not
share the same params schema, validation must reject the animation. Use
separate layers or separate animations for those cases.

## Built-In Track Types

Minimum useful interpolated track type set:

- `enum`
- `inside`
- `integer`
- `double`
- `complex`
- `numeric-tuple`
- `outside`
- `point2`
- `vector2`
- `point3`
- `vector3`
- `camera2d`
- `id-3d-view`
- `julibrot-view`
- `color-map`
- `center-mag`
- `corners`
- `color`
- `angle`

The animator may hard-code these types. That is a small type system, not
a list of Iterated Dynamics parameters.

String parameters are discrete keyframed values, not interpolated values.
Use strings for held selector, entry-name, and filename parameters that
choose Id resources or metadata context. These are arbitrary Id strings,
such as entry names in formula, L-system, IFS, and orbit files.

Concrete string parameters include:

- `formulaname`
- `lname`
- `ifs`
- `orbitname`
- `formulafile`
- `lfile`
- `ifsfile`
- `filename`
- `savename`

For animation planning, the important string selectors are
`formulaname`, which selects formula entry metadata, and `orbitname`,
which selects Julibrot orbit context. String parameters may appear in
keyframes, but they must use hold or step behavior. Do not interpolate
strings.

## Color Specifications

Color specifications are strings used anywhere the animation format names
a concrete color, such as gradient stops, flash colors, and flatten
backgrounds. CSS named colors are accepted case-insensitively when they
name a fixed sRGB color. Exclude special CSS keywords that do not name one
specific color, such as `currentColor`, `transparent`, and system colors.
Use the W3C CSS Color Module Level 4 named-colors table as the reference
for the name-to-sRGB mapping. There is no canonical machine-readable form
of that table. Implementations should check in a static table derived from
the W3C table, with the source URL and retrieval date recorded in a
comment. Do not scrape the spec at runtime.

A bare slash tuple is RGB and means `rgb:red/green/blue`. The explicit
prefixes are:

| Prefix | Components |
| --- | --- |
| `rgb:` | `red/green/blue`, integers from 0 through 255. |
| `hsv:` | `hue/saturation/value`, hue degrees and normalized floats. |
| `hsl:` | `hue/saturation/lightness`, hue degrees and normalized floats. |

The prefix is optional and defaults to `rgb:`. Thus `255/40/0` and
`rgb:255/40/0` are the same color. HSV and HSL values are converted to
RGB before writing Id map files or passing a color to a backend. Hue is
validated from 0 through 360, and saturation, value, and lightness are
validated from 0 through 1.

The inside and outside types are discrete Id coloring values. A value may
be either one declared method string or an integer colormap index. Catalog
metadata should declare the accepted method strings and set numeric bounds
for color indexes, normally 0 through 255.

Inside and outside are distinct types because they accept different method
sets. `inside` accepts methods such as `maxiter`, `bof60`, and `zmag`.
`outside` accepts methods such as `iter`, `real`, `imag`, and `summ`.
Both accept numeric color-index values such as `0` and `255`.

## 3D Point And Vector Parameters

Some Iterated Dynamics parameters are one par-file parameter whose value
is a 3-component floating-point tuple. Treat these as atomic tracks, not
as three unrelated scalar tracks.

Example:

    {
      "parameters": {
        "lightsource": {
          "type": "point3",
          "format": "slash",
          "default-curve": "smoothstep",
          "extrapolate": "clamp"
        }
      }
    }

The point3 and vector3 types are convenience aliases over numeric-tuple
with arity 3. They use the numeric-tuple parser, interpolate each
component independently, and format one value back into the original
tuple syntax.

Use point3 for positions. Use vector3 for directions. A vector3 metadata
entry may set normalize to true when the value must remain a unit vector
after interpolation.

## Virtual 2D Camera Tracks

Iterated Dynamics does not have a 2D camera object. It has viewport
parameters such as center-mag and corners. A camera2d track is therefore
a virtual planning track. It evaluates camera curves, then writes one
catalog-declared viewport parameter.

Example:

    {
      "name": "camera",
      "type": "camera2d",
      "output": "corners",
      "aspect": "source",
      "look-at": {
        "type": "point2",
        "keys": [
          { "frame": 0,   "value": "-0.5/0.0" },
          { "frame": 300, "value": "-0.75/0.1" }
        ]
      },
      "view-up": {
        "type": "vector2",
        "normalize": true,
        "keys": [
          { "frame": 0,   "value": "0/1" },
          { "frame": 300, "value": "0.25/1" }
        ]
      },
      "height": {
        "type": "double",
        "keys": [
          { "frame": 0,   "value": 3.0 },
          { "frame": 300, "value": 0.1, "curve": "geometric" }
        ]
      }
    }

At each frame:

    look = evaluate look-at point2 track
    up = normalize(evaluate view-up vector2 track)
    right = perpendicular clockwise from up
    height = evaluate height track
    width = height * aspect

    lower_left = look - right * width / 2 - up * height / 2
    lower_right = look + right * width / 2 - up * height / 2
    upper_left = look - right * width / 2 + up * height / 2

The output names a catalog parameter. It is not a hard-coded Iterated
Dynamics parameter name in the animator.

For output corners, the output parameter metadata must have type corners.
Format the computed points into the corners syntax supported by the
target renderer.

For output center-mag, the output parameter metadata must have type
center-mag. Require the camera to be axis-aligned with the normal view-up
vector. Reject rotated camera2d output to center-mag with a specific
error rather than silently dropping orientation.

The camera2d track lets the animator plan look-at, view-up, and height as
independent curves while still writing only normal Iterated Dynamics
parameters.

## Id 3D Viewing Adapters

Iterated Dynamics does not expose one general 3D camera model. It exposes
several parameter families. Use virtual adapters that evaluate planned
curves, then write real catalog-declared parameters.

The adapter type is hard-coded. The output parameter names are not.

## Id Euler 3D View Adapter

The id-3d-view adapter targets Id's Euler-style 3D view controls. This is
the right adapter for Id's general 3D viewing parameters and for 3D
orbital types such as lorenz3d and ifs3d.

It writes catalog-declared outputs such as:

- `rotation`
- `perspective`
- `xyshift`
- `xyadjust`
- `scalexyz`
- `roughness`
- `sphere`
- `longitude`
- `latitude`
- `radius`
- `stereo`
- `interocular`
- `converge`

Example:

    {
      "name": "view",
      "type": "id-3d-view",
      "outputs": {
        "rotation": "rotation",
        "perspective": "perspective",
        "xyshift": "xyshift",
        "scalexyz": "scalexyz"
      },
      "rotation": {
        "type": "numeric-tuple",
        "arity": 3,
        "rounding": "nearest",
        "keys": [
          { "frame": 0,   "value": "60/30/0" },
          { "frame": 300, "value": "70/45/5" }
        ]
      },
      "perspective": {
        "type": "integer",
        "keys": [
          { "frame": 0,   "value": 0 },
          { "frame": 300, "value": 150 }
        ]
      },
      "xyshift": {
        "type": "numeric-tuple",
        "arity": 2,
        "rounding": "nearest",
        "keys": [
          { "frame": 0,   "value": "0/0" },
          { "frame": 300, "value": "20/-5" }
        ]
      }
    }

The adapter may offer eye, look-at, and view-up as planning inputs only
when they can be converted to Id's x/y/z rotation, perspective, and shift
controls. If the requested camera motion needs an unsupported target,
roll, projection, or center of interest, reject it with a clear error.

The general 3D view supports the broader output set. Orbital 3D types
such as lorenz3d and ifs3d support a smaller set: rotation,
perspective, xyshift, and stereo controls. Validate against the selected
target.

## Julibrot View Adapter

The julibrot-view adapter targets Julibrot's slice and stereo renderer. It
does not use Id's general rotation parameters.

It writes catalog-declared outputs such as:

- `3dmode`
- `julibrot3d`
- `julibroteyes`
- `julibrotfromto`

Example:

    {
      "name": "julibrot-view",
      "type": "julibrot-view",
      "outputs": {
        "mode": "3dmode",
        "geometry": "julibrot3d",
        "eyes": "julibroteyes",
        "from-to": "julibrotfromto"
      },
      "mode": {
        "type": "enum",
        "keys": [
          { "frame": 0, "value": "monocular" }
        ]
      },
      "geometry": {
        "type": "numeric-tuple",
        "arity": 6,
        "keys": [
          { "frame": 0,   "value": "128/8/8/7/10/24" },
          { "frame": 300, "value": "160/7/6/6/9/20" }
        ]
      },
      "eyes": {
        "type": "double",
        "keys": [
          { "frame": 0,   "value": 2.5 },
          { "frame": 300, "value": 1.0 }
        ]
      },
      "from-to": {
        "type": "numeric-tuple",
        "arity": 4,
        "keys": [
          { "frame": 0,   "value": "-0.83/-0.83/0.25/-0.25" },
          { "frame": 300, "value": "-0.7/-0.9/0.2/-0.2" }
        ]
      }
    }

The six geometry components are:

- z dots
- origin
- depth
- height
- width
- viewer distance

Julibrot has no arbitrary view-up, roll, or look-at camera. If a planned
camera path asks for those, the adapter must reject it unless the request
can be expressed by Julibrot's origin, depth, screen size, viewer
distance, eye separation, and from/to slice parameters.

The orbitname parameter belongs to the Julibrot fractal setup, but it is
not a view control. Animate it as a normal string or enum track when
needed.

## Curves

Each segment between two keys has a curve. The curve may be specified on
the destination key or on the segment itself.

If specified on the destination key:

- Key i - 1 to key i uses key i curve.

Useful curves:

- `linear`
- `step`
- `hold`
- `smoothstep`
- `smootherstep`
- `ease-in`
- `ease-out`
- `ease-in-out`
- `sine`
- `triangle`
- `sawtooth`
- `pulse`
- `ping-pong`

Suggested behavior:

| Curve | Behavior |
| --- | --- |
| `hold` | Keep the previous key value until the next key is reached. |
| `step` | Jump at the destination key. |
| `linear` | Interpolate linearly in local segment time. |
| `smoothstep` | Ease in and ease out with a smooth polynomial. |
| `geometric` | Useful for positive scale or magnification values. |

## Implementation Libraries

Do not use tweeny as the new animation engine. It is shaped around
object tweening, while ParAnimator needs frame-addressable typed tracks,
Id parameter formatting, virtual adapters, validation, and side effects
such as per-frame map files.

The new track engine should have a small local curve evaluator:

- local segment lookup by frame
- local easing functions
- local typed interpolation
- local extrapolation handling

This should replace the existing tweeny-backed interpolant path in the
first implementation slice. Once no source file includes tweeny, remove
tweeny from the dependency manifest.

Dependency guidance:

| Library | Guidance |
| --- | --- |
| `Boost.JSON` | JSON parsing and serialization. |
| `Boost.Math` interpolators | Do not use for core tracks; evaluate only at the Catmull-Rom or spline path slice. |
| `TinySpline` | Consider at the same path slice only if paths need knots or arc-length sampling. |
| `GLM` or local vector types | Camera and 3D adapter math only if it reduces code. |
| `ImageMagick` | External command-line renderer for layer composition. |

If Boost.Math or TinySpline is used, hide it behind `PathGenerator`.

Do not expose dependency-specific names or behavior in JSON. JSON names
belong to ParAnimator's track model.

## Extrapolation

Each track controls behavior before its first key and after its last key.

Useful extrapolation modes:

| Mode | Behavior |
| --- | --- |
| `clamp` | Use the nearest key value. |
| `base` | Use the value from the base parameter set. |
| `omit` | Do not write this parameter outside the keyed range. |
| `cycle` | Repeat the track. |
| `ping-pong` | Repeat the track forward and backward. |

Safe default:

- `clamp`

## Animation File

The animation file references the parameter catalog and defines tracks.

Example:

    {
      "parameter-catalogs": [
        "parameters/core.json",
        "parameters/coloring.json",
        "parameters/fractals/julia.json"
      ],

      "source": {
        "file": "examples.par",
        "name": "base"
      },

      "output": {
        "directory": "out/julia-pan",
        "par": "frames.par",
        "entry": "frame%04d",
        "script": "render.bat",
        "frames": "frame%04d.png"
      },
      "video": "yes",
      "num-frames": 900,

      "tracks": [
        {
          "parameter": "params",
          "keys": [
            { "frame": 0,   "value": "-0.12/0.75" },
            { "frame": 300, "value": "-0.16/0.72" },
            { "frame": 900, "value": "-0.12/0.75" }
          ]
        },

        {
          "parameter": "maxiter",
          "keys": [
            { "frame": 0,   "value": 100 },
            { "frame": 120, "value": 100,  "curve": "hold" },
            { "frame": 500, "value": 2000, "curve": "ease-out" },
            { "frame": 900, "value": 2000, "curve": "hold" }
          ]
        },

        {
          "parameter": "colors",
          "type": "color-map",
          "format": "at-file",
          "output": "colors-%04d.map",
          "keys": [
            { "frame": 0,   "value": "fire.map" },
            { "frame": 900, "value": "ice.map" }
          ]
        }
      ]
    }

The track does not need to repeat the type if the catalog declares it.

## Output Layout

The animation file specifies one output directory. ParAnimator writes
generated Id library files under that directory:

| Path | Contents |
| --- | --- |
| `output-directory/par` | Generated par files. |
| `output-directory/map` | Generated map files. |
| `output-directory` | Generated batch scripts. |

Rendered frame and layer images may use separate configured directories,
but par files and map files must use Id's library layout.

output.par is the generated par filename under output-directory/par.
output.entry is the generated par entry name pattern.

Batch commands and colormap values reference par and map files by
filename only. Do not write generated map paths into colors values, and
do not write generated par paths into Id @ arguments.

Example generated colors value:

    colors=@colors-0042.map

Example batch command shape:

    id batch=yes librarydirs=out/julia-pan @frames.par/frame0042

This uses Id's @par/name syntax. Here par is frames.par, a generated file
in output-directory/par, and name is frame0042, an entry in that file.

The batch file passes librarydirs pointing at the animation output
directory. Id then locates generated par files in the par subdirectory
and generated map files in the map subdirectory.

Source par and map names are filenames too. ParAnimator may resolve them
using its own configured library search, but generated output should not
embed those paths.

## Layered Animation Files

An animation may define a layer stack instead of a single source
parameter set. Each layer renders an Iterated Dynamics image for the
current frame. The final animation frame is produced by compositing those
layer images with ImageMagick.

Layers are evaluated from bottom to top.

Example:

    {
      "parameter-catalogs": [
        "parameters/core.json",
        "parameters/coloring.json"
      ],

      "output": {
        "directory": "out/layered-shot",
        "par": "layers.par",
        "entry": "layer-%s-%04d",
        "frames": "frames/frame%04d.png",
        "layers": "layer-%s-%04d.png",
        "script": "render.bat",
        "compose-script": "compose.bat",
        "background": "black"
      },

      "num-frames": 900,
      "fps": 30,
      "video": "yes",

      "layers": [
        {
          "id": "base",
          "source": {
            "file": "examples.par",
            "name": "base"
          },
          "opacity": {
            "keys": [
              { "frame": 0, "value": 100 }
            ]
          },
          "compose": "source-over",
          "tracks": [
            {
              "parameter": "center-mag",
              "keys": [
                { "frame": 0,   "value": "-0.5/0/1" },
                { "frame": 900, "value": "-0.7/0.1/64" }
              ]
            }
          ]
        },

        {
          "id": "glow",
          "source": {
            "file": "examples.par",
            "name": "glow"
          },
          "opacity": {
            "keys": [
              { "frame": 0,   "value": 0 },
              { "frame": 120, "value": 65 },
              { "frame": 900, "value": 20 }
            ]
          },
          "compose": "screen",
          "tracks": [
            {
              "parameter": "maxiter",
              "keys": [
                { "frame": 0,   "value": 100 },
                { "frame": 900, "value": 1200 }
              ]
            }
          ]
        }
      ]
    }

Layer field meanings:

| Field | Meaning |
| --- | --- |
| `id` | Stable identifier used in filenames and scoped track names. |
| `source` | Base parameter set for this layer. |
| `tracks` | Parameter timelines evaluated only for this layer. |
| `opacity` | Percent opacity; animate to 0 instead of inserting or deleting layers over time. |
| `compose` | Backend-neutral layer operator applied over the current frame image. |
| `write-when-hidden` | Whether to render the layer even when evaluated opacity is 0. |
| `output.background` | Optional flatten color specification for final frame formats that do not keep alpha. |

If `output.background` is omitted, keep the composed frame alpha channel.

The compose value is a ParAnimator operator name, not an ImageMagick
operator name. Backends map these names to their native command syntax.
Operator enum strings are lower-case ASCII. Multiword names use hyphens.

Porter-Duff operators:

| Name | Meaning |
| --- | --- |
| `clear` | Output transparent black. |
| `copy` | Replace destination with source. |
| `destination` | Keep destination unchanged. |
| `source-over` | Source over destination. |
| `destination-over` | Destination over source. |
| `source-in` | Source kept only where destination alpha exists. |
| `destination-in` | Destination kept only where source alpha exists. |
| `source-out` | Source kept only where destination alpha is absent. |
| `destination-out` | Destination kept only where source alpha is absent. |
| `source-atop` | Source atop destination, keeping destination alpha. |
| `destination-atop` | Destination atop source, keeping source alpha. |
| `xor` | Source and destination outside their overlap. |

Math binary operators:

| Name | Meaning |
| --- | --- |
| `add` | Add source and destination channels. |
| `subtract` | Subtract source from destination channels. |
| `multiply` | Multiply normalized source and destination channels. |
| `divide` | Divide destination channels by source channels. |
| `min` | Keep the smaller channel value. |
| `max` | Keep the larger channel value. |

Other useful neutral operators:

| Name | Meaning |
| --- | --- |
| `difference` | Absolute channel difference. |
| `average` | Average source and destination channels. |
| `screen` | Inverse multiply; useful for glow and light layers. |
| `overlay` | Multiply dark areas and screen light areas. |

Validate operator names against the ParAnimator enum. ImageMagick support
is checked by the ImageMagick adapter that maps these names to native
operators.

The layer stack has no separate post-render geometry stage. A layer may
animate normal Id parameters, including viewport and virtual camera
tracks, but the composition step only controls opacity and the neutral
compose operator.

The layer system does not read external animation files or emulate their
blending vocabulary. The goal is similar layered rendering behavior using
Iterated Dynamics and ImageMagick.

## Colormap Tracks

Id color animation may require a generated map file for each frame. Model
that as a normal track for the real Id colors parameter, with a colormap
type that writes a side file and returns an at-file value.

The simplest colormap track interpolates between map files:

    {
      "parameter": "colors",
      "type": "color-map",
      "format": "at-file",
      "output": "colors-%04d.map",
      "keys": [
        { "frame": 0,   "value": "fire.map" },
        { "frame": 300, "value": "ice.map" }
      ]
    }

This is shorthand for an interpolate effect.

For richer animation, use a source map plus an ordered effect list:

    {
      "parameter": "colors",
      "type": "color-map",
      "format": "at-file",
      "output": "colors-%04d.map",
      "source": "base.map",
      "effects": [
        {
          "kind": "rotate-range",
          "range": [32, 127],
          "offset": {
            "keys": [
              { "frame": 0,   "value": 0 },
              { "frame": 300, "value": 96 }
            ]
          }
        },
        {
          "kind": "brightness",
          "amount": {
            "keys": [
              { "frame": 0,   "value": 1.0 },
              { "frame": 150, "value": 1.4 },
              { "frame": 300, "value": 1.0 }
            ]
          }
        }
      ]
    }

At each frame:

- Evaluate the source map.
- Apply each effect in order.
- Write `output-directory/map/colors-0000.map`.
- Return `@colors-0000.map` as the colors parameter value.

Effect parameters may be constants or keyed scalar, tuple, color
specification, or enum tracks. This keeps timing local to the colormap
track while reusing the normal track interpolation machinery.

Core colormap effects:

| Effect | Meaning |
| --- | --- |
| `interpolate` | Blend two or more Id map files with keyed weights. |
| `sequence` | Step through map files, with optional crossfade frames. |
| `rotate` | Shift all palette indices by a keyed offset. |
| `rotate-range` | Shift only an inclusive index range. |
| `reverse` | Reverse the full map or one inclusive index range. |
| `ping-pong` | Oscillate an index range forward and backward. |
| `gradient` | Generate a map from keyed color stops. |
| `hue-shift` | Rotate hue in HSL or HSV space. |
| `saturation` | Scale color saturation. |
| `brightness` | Scale color intensity. |
| `contrast` | Expand or compress color distance from midgray. |
| `gamma` | Apply nonlinear intensity shaping. |
| `posterize` | Reduce color levels to bands. |
| `remap` | Reindex the palette through a curve or lookup table. |
| `pulse` | Blend a range toward a keyed flash color. |
| `mask-blend` | Blend selected index ranges between maps. |
| `sparkle` | Apply seeded, bounded random color perturbations. |

Gradient sources accept two or more stops. Each adjacent stop pair defines
one interpolation interval.

Example generated map:

    {
      "parameter": "colors",
      "type": "color-map",
      "format": "at-file",
      "output": "gradient-%04d.map",
      "source": {
        "kind": "gradient",
        "stops": [
          { "index": 0,   "color": "black" },
          { "index": 64,  "color": "hsv:20/1/1" },
          { "index": 128, "color": "hsl:60/1/0.5" },
          { "index": 255, "color": "hsl:0/0/1" }
        ]
      },
      "effects": [
        {
          "kind": "hue-shift",
          "amount": {
            "keys": [
              { "frame": 0,   "value": 0 },
              { "frame": 300, "value": 360 }
            ]
          }
        }
      ]
    }

The colormap writer should emit an Id-compatible map file. Validation
must reject mismatched palette sizes or malformed color entries.
Stochastic effects such as sparkle must require an explicit seed so
renders are repeatable.

Use colors=@file for per-frame map files. The map parameter is a real Id
parameter too, but it is better suited to selecting a default map than to
recording the frame-local palette in generated par entries.

## Track Structure

A track has:

- a `parameter` for Id tracks, or a `name` for virtual tracks
- an optional type override
- keyframes or a path generator
- extrapolation behavior
- local options

Normal tracks write one Id par-file parameter named by parameter. Virtual
tracks use name instead of parameter. Virtual tracks such as camera2d may
write another catalog parameter named by a local output option. Virtual
adapters such as id-3d-view and julibrot-view may write multiple catalog
parameters.

Conceptual C++ interface:

    class Track
    {
    public:
        std::string parameter() const;
        std::string value_at(int frame) const;
    };

The implementation may generalize this to return a list of parameter
assignments when virtual tracks are added.

A track evaluates as follows:

1. If `frame` is before the first key, apply before-range
   extrapolation.
2. If `frame` is after the last key, apply after-range extrapolation.
3. Otherwise, find the enclosing key segment.
4. Normalize local time to 0 through 1.
5. Apply the segment curve.
6. Interpolate typed values.
7. Format the result in par-file syntax.

## Path Generators

Path generators belong to parameter tracks. They are not global animation
features.

Example:

    {
      "parameter": "params",
      "type": "complex",
      "path": {
        "kind": "circle",
        "from-frame": 0,
        "to-frame": 900,
        "center": "-0.12/0.75",
        "radius": 0.04,
        "turns": 1
      }
    }

Useful path kinds:

- `line`
- `circle`
- `ellipse`
- `lissajous`
- `spiral`
- `bezier`
- `catmull-rom`
- `constant`
- `ping-pong`
- `noise`

A track can use keys, a path, or an expression. They all expose the same
runtime operation:

    value_at(frame)

## Formula-Specific Catalogs

Parameter metadata can be split into multiple files.

Suggested layout:

- `parameters/core.json`
- `parameters/coloring.json`
- `parameters/3d.json`
- `parameters/fractals/mandel.json`
- `parameters/fractals/julia.json`
- `parameters/fractals/phoenix.json`
- `parameters/fractals/newton.json`
- `parameters/fractals/ifs.json`

The animation file includes only the catalogs it needs.

This is useful because Iterated Dynamics has many formula-specific
parameters. ParAnimator should not require a source change every time
another parameter becomes useful for animation.

## User Catalog Overrides

Allow user catalogs to include and override other catalogs.

Example:

    {
      "include": [
        "id-default-parameters.json"
      ],

      "fractal-types": {
        "julia": {
          "params": {
            "groups": {
              "c": {
                "type": "complex",
                "slots": [ 0, 1 ],
                "format": "slash-pair",
                "default-curve": "linear"
              }
            }
          }
        }
      }
    }

This lets local experiments refine metadata for real Id parameters without
modifying the default catalog.

## Discrete Parameters

Enum, inside, and outside parameters are discrete by default.

Default discrete behavior should be hold or step, not numeric
interpolation.

Example metadata:

    {
      "parameters": {
        "inside": {
          "type": "inside",
          "values": [ "bof60", "zmag", "epsiloncross", "startrail" ],
          "default-curve": "hold",
          "min": 0,
          "max": 255
        }
      }
    }

Example track:

    {
      "parameter": "inside",
      "keys": [
        { "frame": 0,   "value": "bof60" },
        { "frame": 120, "value": "zmag" }
      ]
    }

This produces one legal discrete value per frame. The same track type may
also hold numeric color-index values such as `0` or `255`.

## Discrete PWM

Discrete values can be temporally dithered using a PWM-like track mode.

The value itself is not continuous. The choice of emitted value over
frames becomes a discretized signal whose duty cycle approximates a
continuous blend.

Use this only when the two discrete values produce visually related
results.

Example:

    {
      "parameter": "inside",
      "mode": "pwm",
      "a": "bof60",
      "b": "zmag",
      "window": 8,
      "keys": [
        { "frame": 0,   "mix": 0.0 },
        { "frame": 120, "mix": 1.0 }
      ]
    }

Semantics:

| Mix | Behavior |
| --- | --- |
| `0.0` | Always emit value `a`. |
| `1.0` | Always emit value `b`. |
| `0.25` | Emit value `b` for about 25 percent of frames. |

The user explicitly specifies which discrete values are used for the PWM
off/on pair. Do not infer the pair from catalog value order.

The names a and b are preferred over off and on because these are not
electrical signals. If the PWM analogy should be explicit, off and on are
acceptable aliases.

Example with explicit off and on:

    {
      "parameter": "outside",
      "mode": "pwm",
      "off": "real",
      "on": "atan",
      "window": 12,
      "keys": [
        { "frame": 0,   "duty": 0.0 },
        { "frame": 300, "duty": 0.5 },
        { "frame": 600, "duty": 1.0 }
      ]
    }

Validation rules:

- `a` must be a legal discrete value.
- `b` must be a legal discrete value.
- `off` must be a legal discrete value.
- `on` must be a legal discrete value.
- `a` and `b` may be equal, but this should warn.
- `mix` must be in the range 0 through 1 unless clamping is enabled.
- `duty` must be in the range 0 through 1 unless clamping is
  enabled.
- `window` must be at least 2.

Simple evaluation:

    mix = evaluate_mix_track(frame)
    phase = dither_phase(frame)

    if phase < mix:
        return b
    else:
        return a

For less obvious flicker, avoid simple alternating modulo patterns. Use a
deterministic low-discrepancy sequence, ordered dithering, or a seeded
blue-noise-like pattern.

Possible distribution modes:

- `regular`
- `ordered`
- `random-seeded`
- `low-discrepancy`
- `blue-noise`

For reproducible builds, avoid unseeded randomness.

## Discrete PWM Limitations

PWM approximates blending over time. A still frame is never blended. It
contains only one discrete value.

If true per-pixel or spatial blending is desired, that belongs in the
renderer, not ParAnimator.

PWM works best for:

- coloring mode
- inside method
- outside method
- decomposition mode
- orbit trap mode
- palette selection
- rendering style
- overlay mode

PWM is usually bad for:

- fractal formula
- major coordinate interpretation
- symmetry mode
- precision mode
- rendering backend
- algorithm switch that changes image structure completely

If two discrete values produce unrelated images, PWM becomes flicker
rather than interpolation.

## Validation

On startup:

1. Load parameter catalogs.
2. Merge includes and overrides.
3. Load the base parameter set.
4. Load animation tracks.
5. For each track, look up parameter metadata, validate keys and
   options, and construct the typed track.

Errors should be specific.

Examples:

- `Unknown animated parameter 'param'.`
- `No metadata exists for 'param'.`
- `Did you mean 'params'?`
- `Parameter 'maxiter' is type integer, but key at frame 120 has value
  'abc'.`
- `Parameter 'inside' is inside, but value 'foo' is neither a
  declared method nor a color index.`
- `Parameter 'outside' uses PWM value 'atan', but 'atan' is not listed
  as a legal discrete value.`

## JSON File Replacement

The new animation JSON format replaces the existing JSON files. Do not
support old from/to/interpolate JSON as compatibility syntax.

Required behavior:

- load only the new source/tracks animation schema
- reject old from/to/interpolate JSON with a clear error
- provide examples that use only the new schema
- remove code paths that desugar old JSON into tracks

The old model is not a shorthand. Existing animation JSON files must be
rewritten as new source/tracks files before they are used.

## Suggested Internal Classes

Sketch:

    struct ParameterMetadata
    {
        std::string name;
        std::string type;
        std::string format;
        std::string default_curve;
        std::string extrapolate;

        std::optional<double> min;
        std::optional<double> max;
        std::optional<int> arity;
        bool normalize{false};

        std::vector<std::string> discrete_values;
    };

    struct Keyframe
    {
        int frame;
        std::string value;
        std::string curve;
    };

    class Track
    {
    public:
        std::string parameter() const;
        std::string value_at(int frame) const;
    };

    TrackPtr create_track(
        ParameterMetadata const& metadata,
        TrackSpec const& spec,
        std::string const& base_value,
        int num_frames);

The current interpolant list becomes a track list:

    std::vector<TrackPtr> m_tracks;

The current hard-coded interpolant factory becomes a type-based track
factory.

## Render Loop Shape

The generated frame loop should remain simple:

    for frame_number in 0 through num_frames - 1:
        frame = base_parameter_set

        for each track:
            apply track assignments at frame_number

        write map side files to output-directory/map
        append batch parameters
        append savename parameter
        append overwrite parameter
        append video parameter

        write frame entry to output-directory/par/<output.par>
        write script command using librarydirs and @par/name

This preserves the current ParAnimator workflow while allowing much richer
animation.

Layered animations use the same track evaluation per layer, followed by
ImageMagick composition:

    for frame_number in 0 through num_frames - 1:
        for each layer from bottom to top:
            frame = layer.base_parameter_set

            for each layer track:
                apply track assignments at frame_number

            write map side files to output-directory/map

            if opacity is 0 and write-when-hidden is false:
                skip layer render
            else:
                append batch parameters
                append layer savename parameter
                write layer entry to output-directory/par/<output.par>
                write Id command using librarydirs and @par/name

        start with a transparent canvas

        for each rendered layer from bottom to top:
            apply evaluated opacity to layer alpha
            composite layer with its neutral compose operator

        optionally flatten to output.background
        write final frame image

## Implementation Slices

Each slice should leave the program buildable, tested, and at least as
usable as before. The minimum viable product can now write new-format
Id library-compatible par and batch files for center-mag and corners.
Later slices broaden one behavior at a time while preserving that working
path.

When a slice is implemented, remove it from this section.

Each new slice should add or extend unit tests against application data
types wherever that is reasonable. Tests that intentionally exercise JSON
should stay JSON-focused: schema validation, deserialization, serialization
when added, and error messages for invalid JSON. Downstream behavior tests
should prefer typed data once the typed seam exists. Keep
`paranimator_test` integration fixtures for end-to-end behavior and
regressions in generated par files.

Schema enums may stay duplicated while only two schemas need them. If
another schema needs the same enum, or if an enum grows enough that drift
is likely, add a shared schema file and external `$ref` loader support.
Every new schema object, field, and enum or const value must include a
`description` string when the schema element is added.




### 1. Add Camera2D Center-Mag Output

Add camera2d output to center-mag for axis-aligned cameras.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- axis-aligned camera writes expected center-mag.
- rotated camera targeting center-mag is rejected.
- aspect handling matches the source image shape.

### 2. Add Basic Id 3D View Adapter

Add id-3d-view output for rotation, perspective, and xyshift.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- rotation writes a 3-value slash tuple.
- perspective writes an integer value.
- xyshift writes a 2-value slash tuple.

### 3. Add More Id 3D View Outputs

Add scalexyz, roughness, sphere, longitude, latitude, radius, stereo,
interocular, and converge outputs.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- scalexyz writes three values.
- stereo controls write legal values.
- unsupported target outputs are rejected.

### 4. Add Julibrot View Adapter

Add julibrot-view output for 3dmode, julibrot3d, julibroteyes, and
julibrotfromto.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- 3dmode writes one legal enum value.
- julibrot3d writes six components.
- arbitrary look-at or view-up requests are rejected.

### 5. Add Single-Layer Stack

Allow animations to define one layer. It should behave like the existing
single-source animation but use the layer schema.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- one layer writes one par entry per frame.
- layer tracks apply to that layer.
- duplicate layer ids are rejected.

### 6. Add Multi-Layer Rendering

Allow multiple layers to render separate Id images before composition.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- layers are evaluated from bottom to top.
- each layer applies only its own tracks.
- generated layer entry names include layer id and frame number.

### 7. Add Layer Opacity

Add layer opacity evaluation and hidden-layer skipping.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- opacity 0 skips rendering by default.
- write-when-hidden renders opacity 0 layers.
- opacity values outside 0 through 100 are rejected.

### 8. Add source-over Composition

Generate ImageMagick commands for the neutral `source-over` operator.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- generated commands compose layers in stack order.
- opacity is applied before composition.
- output.background adds a flatten step when configured.

### 9. Add More Neutral Compose Operators

Allow configured neutral compose operators and validate them. Map those
operators to ImageMagick names only inside the ImageMagick adapter.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- Porter-Duff operators such as `source-over` and `destination-over` are
  accepted.
- math operators such as `add`, `subtract`, `multiply`, `divide`, `min`,
  and `max` are accepted.
- useful blend operators such as `difference`, `average`, `screen`, and
  `overlay` are accepted.
- unsupported operators are rejected.
- ImageMagick-specific operator spellings are rejected in animation JSON.

### 10. Add Core Catalog Files

Add default catalogs for core Id parameters and coloring.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- core catalog declares type, maxiter, center-mag, and corners.
- coloring catalog declares colors as color-map.
- catalog inclusion fails clearly for missing files.

### 11. Add 3D And Formula Catalog Files

Add default catalogs for Id 3D viewing and selected formula families.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- 3D catalog declares rotation and julibrot3d arity.
- formula catalogs attach params knob and function key metadata to formula
  entry names.
- animation files include only the catalogs they need.

The intent is not to finish a large subsystem before anything runs. The
intent is to get a small valid Id animation working quickly, then keep
that path working while each later feature is added.

## Design Boundary

Hard-code:

- track types
- curves
- path generators
- format parsers
- validation rules
- colormap effect algorithms
- Id map file writer
- layer stack evaluation
- ImageMagick command generation

Do not hard-code:

- Iterated Dynamics parameter names
- formula-specific parameter lists
- legal discrete values
- which parameters are animatable
- virtual adapter output parameter names
- default curves for individual parameters
- discrete values used by PWM tracks
- generated colormap filenames
- source map filenames
- backend-specific compose operator names
- dependency-specific curve or track names

## Summary

The final design is:

- one global frame clock
- many independent parameter timelines
- each timeline has its own keys, curves, type, and extrapolation
- local frame-addressable curve evaluation replaces tweeny
- parameter names and metadata come from JSON catalogs
- the animator knows types, not Iterated Dynamics parameter names
- virtual adapters map planned views onto real Id parameters
- colormap tracks can apply effects, write per-frame Id map files, and
  emit `colors=@file`
- optional layer stacks render Id layer images and compose them with
  backend-neutral operators
- enum, inside, and outside parameters are discrete by default
- discrete PWM is an optional temporal dithering mode
- PWM tracks explicitly choose their `a` and `b` discrete values

This turns ParAnimator into a data-driven parameter animation sequencer
rather than a viewport interpolation tool.
