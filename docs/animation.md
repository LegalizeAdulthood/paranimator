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

- Frame 0 through num_frames - 1.

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
          "type": "center_mag",
          "description": "Viewport center and magnification",
          "default_curve": "geometric",
          "extrapolate": "clamp"
        },

        "corners": {
          "type": "corners",
          "description": "Viewport rectangle",
          "default_curve": "linear",
          "extrapolate": "clamp"
        },

        "maxiter": {
          "type": "integer",
          "min": 1,
          "max": 2147483647,
          "default_curve": "step",
          "rounding": "nearest",
          "extrapolate": "clamp"
        },

        "bailout": {
          "type": "double",
          "min": 0.0,
          "default_curve": "smoothstep",
          "extrapolate": "clamp"
        },

        "colors": {
          "type": "colormap",
          "format": "at_file",
          "default_curve": "smoothstep",
          "extrapolate": "clamp"
        },

        "inside": {
          "type": "enum",
          "values": [ "bof60", "zmag", "epscross", "startrail" ],
          "default_curve": "hold",
          "extrapolate": "clamp"
        },

        "outside": {
          "type": "enum",
          "default_curve": "hold",
          "extrapolate": "clamp"
        },

        "lightsource": {
          "type": "point3",
          "format": "slash",
          "default_curve": "smoothstep",
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
| `default_curve` | Curve used by segments unless overridden. |
| `extrapolate` | Behavior outside the keyed range. |
| `min` | Lower validation limit; may clamp. |
| `max` | Upper validation limit; may clamp. |
| `arity` | Component count for tuple-like values. |
| `values` | Legal enum values. |
| `rounding` | Rule for producing integer-like values. |
| `units` | Unit hint: degrees, radians, raw, percent. |
| `normalize` | Normalize vector-like values after interpolation. |
| `aliases` | Alternative parameter names accepted in par files. |
| `required` | Parameter must exist in the base parameter set. |
| `write_when_unchanged` | Write value even when it matches base. |

## Type Versus Format

Type and format should be separate.

Do not make a separate type for every textual spelling.

Example:

    {
      "type": "complex",
      "format": "slash_pair"
    }

Possible formats:

| Type | Format |
| --- | --- |
| `integer` | `raw` |
| `double` | `raw` |
| `complex` | `slash_pair` |
| `numeric_tuple` | `slash` |
| `point2` | `slash` |
| `vector2` | `slash` |
| `point3` | `slash` |
| `vector3` | `slash` |
| `color` | `rgb_tuple` |
| `colormap` | `at_file` |
| `angle` | `degrees` |
| `angle` | `radians` |

This lets typed interpolation stay separate from ID parameter formatting.

For ID parameter files, complex values use slash-separated real and
imaginary parts. The help documents examples such as
`params=-0.480/0.626` and `initorbit=nnn/nnn`. Comma-pair notation appears
in formula-language prose, not in parameter syntax.

## Fractal-Specific Params

`params=` is a slash-delimited vector, not one semantic parameter. ID
stores up to ten values in `g_params[]`. The active fractal type decides
which indexes exist and what each index means.

ID's `type_has_param()` checks the first four parameter names from the
fractal-specific table, then checks extra names from
`g_more_fractal_params`. For `type=formula`, unused formula parameters
are suppressed. ID's `put_fractal_params()` writes one `params=` command
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
      "fractal_types": {
        "julia": {
          "params": {
            "format": "slash_list",
            "slots": [
              { "index": 0, "name": "c_real", "type": "double" },
              { "index": 1, "name": "c_imag", "type": "double" }
            ],
            "groups": {
              "c": {
                "type": "complex",
                "slots": [ 0, 1 ],
                "format": "slash_pair"
              }
            }
          }
        }
      }
    }

For `type=formula`, the assignment of `params=` values to formula
variables is fixed by ID and is not catalog metadata:

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
      "formula_entries": {
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
            "fn1": { "type": "enum", "values": "id_functions" },
            "fn2": { "type": "enum", "values": "id_functions" },
            "fn3": { "type": "enum", "values": "id_functions" },
            "fn4": { "type": "enum", "values": "id_functions" }
          }
        }
      }
    }

Tracks may target formula-entry knobs such as `MandelbrotMix4.bailout`,
`MandelbrotMix4["scale factor"]`, or `MandelbrotMix4.c`. These names are
ParAnimator metadata describing the formula entry's use of fixed ID
variables. ID never sees them, and the formula source still refers only to
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
metadata is looked up from the matching `formula_entries` entry. If no
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
using the fixed `id_functions` value set. The writer starts from the
source par entry or ID reset defaults, applies function key updates, and
emits one slash-delimited `function=` assignment through the highest
required function key.

The fixed `id_functions` enum contains the function names recognized by
ID: `sin`, `cos`, `tan`, `cotan`, `sinh`, `cosh`, `tanh`, `cotanh`,
`exp`, `log`, `sqr`, `recip`, `ident`, `cosxx`, `flip`, `conj`, `zero`,
`one`, `asin`, `asinh`, `acos`, `acosh`, `atan`, `atanh`, `sqrt`, `abs`,
`cabs`, `floor`, `ceil`, `trunc`, and `round`.

ID leaves omitted `function=` values unchanged. ParAnimator should still
compose from known base/default values before writing, so generated
frames do not depend on prior process state.

For non-formula params vectors, tracks may target the whole vector, an
indexed slot such as `params[0]`, or a named group such as `params.c`. The
writer starts from the source par entry's base `params`, applies all slot
and group updates, then emits one slash-delimited `params=` assignment
through the highest required slot. This preserves untouched values. Do not
emit partial `params` assignments, because ID treats omitted values as
zero after a `params=` command.

For the first implementation, a layer's fractal type must be stable when
it has params tracks. If `type` is animated and the reachable types do not
share the same params schema, validation must reject the animation. Use
separate layers or separate animations for those cases.

## Built-In Track Types

Minimum useful interpolated track type set:

- `enum`
- `integer`
- `double`
- `complex`
- `numeric_tuple`
- `point2`
- `vector2`
- `point3`
- `vector3`
- `camera2d`
- `id_3d_view`
- `julibrot_view`
- `colormap`
- `center_mag`
- `corners`
- `rgb_color`
- `angle`

The animator may hard-code these types. That is a small type system, not
a list of Iterated Dynamics parameters.

String parameters are discrete keyframed values, not interpolated values.
Use strings for held selector, entry-name, and filename parameters that
choose ID resources or metadata context. These are arbitrary ID strings,
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
          "default_curve": "smoothstep",
          "extrapolate": "clamp"
        }
      }
    }

The point3 and vector3 types are convenience aliases over numeric_tuple
with arity 3. They use the numeric_tuple parser, interpolate each
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
      "look_at": {
        "type": "point2",
        "keys": [
          { "frame": 0,   "value": "-0.5/0.0" },
          { "frame": 300, "value": "-0.75/0.1" }
        ]
      },
      "view_up": {
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

    look = evaluate look_at point2 track
    up = normalize(evaluate view_up vector2 track)
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
center_mag. Require the camera to be axis-aligned with the normal view-up
vector. Reject rotated camera2d output to center-mag with a specific
error rather than silently dropping orientation.

The camera2d track lets the animator plan look_at, view_up, and height as
independent curves while still writing only normal Iterated Dynamics
parameters.

## ID 3D Viewing Adapters

Iterated Dynamics does not expose one general 3D camera model. It exposes
several parameter families. Use virtual adapters that evaluate planned
curves, then write real catalog-declared parameters.

The adapter type is hard-coded. The output parameter names are not.

## ID Euler 3D View Adapter

The id_3d_view adapter targets ID's Euler-style 3D view controls. This is
the right adapter for ID's general 3D viewing parameters and for 3D
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
      "type": "id_3d_view",
      "outputs": {
        "rotation": "rotation",
        "perspective": "perspective",
        "xyshift": "xyshift",
        "scalexyz": "scalexyz"
      },
      "rotation": {
        "type": "numeric_tuple",
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
        "type": "numeric_tuple",
        "arity": 2,
        "rounding": "nearest",
        "keys": [
          { "frame": 0,   "value": "0/0" },
          { "frame": 300, "value": "20/-5" }
        ]
      }
    }

The adapter may offer eye, look_at, and view_up as planning inputs only
when they can be converted to ID's x/y/z rotation, perspective, and shift
controls. If the requested camera motion needs an unsupported target,
roll, projection, or center of interest, reject it with a clear error.

The general 3D view supports the broader output set. Orbital 3D types
such as lorenz3d and ifs3d support a smaller set: rotation,
perspective, xyshift, and stereo controls. Validate against the selected
target.

## Julibrot View Adapter

The julibrot_view adapter targets Julibrot's slice and stereo renderer. It
does not use ID's general rotation parameters.

It writes catalog-declared outputs such as:

- `3dmode`
- `julibrot3d`
- `julibroteyes`
- `julibrotfromto`

Example:

    {
      "name": "julibrot_view",
      "type": "julibrot_view",
      "outputs": {
        "mode": "3dmode",
        "geometry": "julibrot3d",
        "eyes": "julibroteyes",
        "from_to": "julibrotfromto"
      },
      "mode": {
        "type": "enum",
        "keys": [
          { "frame": 0, "value": "monocular" }
        ]
      },
      "geometry": {
        "type": "numeric_tuple",
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
      "from_to": {
        "type": "numeric_tuple",
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

Julibrot has no arbitrary view_up, roll, or look_at camera. If a planned
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
- `ease_in`
- `ease_out`
- `ease_in_out`
- `sine`
- `triangle`
- `sawtooth`
- `pulse`
- `ping_pong`

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
ID parameter formatting, virtual adapters, validation, and side effects
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
| `ping_pong` | Repeat the track forward and backward. |

Safe default:

- `clamp`

## Animation File

The animation file references the parameter catalog and defines tracks.

Example:

    {
      "parameter_catalogs": [
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
      "num_frames": 900,

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
            { "frame": 500, "value": 2000, "curve": "ease_out" },
            { "frame": 900, "value": 2000, "curve": "hold" }
          ]
        },

        {
          "parameter": "colors",
          "type": "colormap",
          "format": "at_file",
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
generated ID library files under that directory:

| Path | Contents |
| --- | --- |
| `output-directory/par` | Generated par files. |
| `output-directory/map` | Generated map files. |
| `output-directory` | Generated batch scripts. |

Rendered frame and layer images may use separate configured directories,
but par files and map files must use ID's library layout.

output.par is the generated par filename under output-directory/par.
output.entry is the generated par entry name pattern.

Batch commands and colormap values reference par and map files by
filename only. Do not write generated map paths into colors values, and
do not write generated par paths into ID @ arguments.

Example generated colors value:

    colors=@colors-0042.map

Example batch command shape:

    id batch=yes librarydirs=out/julia-pan @frames.par/frame0042

This uses ID's @par/name syntax. Here par is frames.par, a generated file
in output-directory/par, and name is frame0042, an entry in that file.

The batch file passes librarydirs pointing at the animation output
directory. ID then locates generated par files in the par subdirectory
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
      "parameter_catalogs": [
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
        "compose_script": "compose.bat",
        "background": "black"
      },

      "num_frames": 900,
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
          "compose": "Over",
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
          "compose": "Screen",
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
| `compose` | ImageMagick compose operator placed over the current frame image. |
| `write_when_hidden` | Whether to render the layer even when evaluated opacity is 0. |
| `output.background` | Optional flatten color for final frame formats that do not keep alpha. |

If `output.background` is omitted, keep the composed frame alpha channel.

The compose value is an ImageMagick compositing operator name, such as:

- `Over`
- `Multiply`
- `Screen`
- `Overlay`
- `HardLight`
- `SoftLight`
- `Darken`
- `Lighten`
- `Difference`
- `Plus`
- `Minus`

Do not invent ParAnimator-specific blend aliases. Validate compose
operators against the ImageMagick operators supported by the installed
toolchain.

The layer stack has no separate post-render geometry stage. A layer may
animate normal ID parameters, including viewport and virtual camera
tracks, but the composition step only controls opacity and ImageMagick
compose.

The layer system does not read external animation files or emulate their
blending vocabulary. The goal is similar layered rendering behavior using
Iterated Dynamics and ImageMagick.

## Colormap Tracks

ID color animation may require a generated map file for each frame. Model
that as a normal track for the real ID colors parameter, with a colormap
type that writes a side file and returns an at-file value.

The simplest colormap track interpolates between map files:

    {
      "parameter": "colors",
      "type": "colormap",
      "format": "at_file",
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
      "type": "colormap",
      "format": "at_file",
      "output": "colors-%04d.map",
      "source": "base.map",
      "effects": [
        {
          "kind": "rotate_range",
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

Effect parameters may be constants or keyed scalar, tuple, color, or enum
tracks. This keeps timing local to the colormap track while reusing the
normal track interpolation machinery.

Core colormap effects:

| Effect | Meaning |
| --- | --- |
| `interpolate` | Blend two or more ID map files with keyed weights. |
| `sequence` | Step through map files, with optional crossfade frames. |
| `rotate` | Shift all palette indices by a keyed offset. |
| `rotate_range` | Shift only an inclusive index range. |
| `reverse` | Reverse the full map or one inclusive index range. |
| `ping_pong` | Oscillate an index range forward and backward. |
| `gradient` | Generate a map from keyed RGB color stops. |
| `hue_shift` | Rotate hue in HSL or HSV space. |
| `saturation` | Scale color saturation. |
| `brightness` | Scale color intensity. |
| `contrast` | Expand or compress color distance from midgray. |
| `gamma` | Apply nonlinear intensity shaping. |
| `posterize` | Reduce color levels to bands. |
| `remap` | Reindex the palette through a curve or lookup table. |
| `pulse` | Blend a range toward a keyed flash color. |
| `mask_blend` | Blend selected index ranges between maps. |
| `sparkle` | Apply seeded, bounded random color perturbations. |

Example generated map:

    {
      "parameter": "colors",
      "type": "colormap",
      "format": "at_file",
      "output": "gradient-%04d.map",
      "source": {
        "kind": "gradient",
        "stops": [
          { "index": 0,   "color": "0/0/0" },
          { "index": 128, "color": "63/10/0" },
          { "index": 255, "color": "63/63/63" }
        ]
      },
      "effects": [
        {
          "kind": "hue_shift",
          "amount": {
            "keys": [
              { "frame": 0,   "value": 0 },
              { "frame": 300, "value": 360 }
            ]
          }
        }
      ]
    }

The colormap writer should emit an ID-compatible map file. Validation
must reject mismatched palette sizes or malformed color entries.
Stochastic effects such as sparkle must require an explicit seed so
renders are repeatable.

Use colors=@file for per-frame map files. The map parameter is a real ID
parameter too, but it is better suited to selecting a default map than to
recording the frame-local palette in generated par entries.

## Track Structure

A track has:

- a `parameter` for ID tracks, or a `name` for virtual tracks
- an optional type override
- keyframes or a path generator
- extrapolation behavior
- local options

Normal tracks write one ID par-file parameter named by parameter. Virtual
tracks use name instead of parameter. Virtual tracks such as camera2d may
write another catalog parameter named by a local output option. Virtual
adapters such as id_3d_view and julibrot_view may write multiple catalog
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
        "from_frame": 0,
        "to_frame": 900,
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
- `catmull_rom`
- `constant`
- `ping_pong`
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

      "fractal_types": {
        "julia": {
          "params": {
            "groups": {
              "c": {
                "type": "complex",
                "slots": [ 0, 1 ],
                "format": "slash_pair",
                "default_curve": "linear"
              }
            }
          }
        }
      }
    }

This lets local experiments refine metadata for real ID parameters without
modifying the default catalog.

## Enum Parameters

Enum parameters are discrete by default.

Default enum behavior should be hold or step, not numeric interpolation.

Example metadata:

    {
      "parameters": {
        "inside": {
          "type": "enum",
          "values": [ "bof60", "zmag", "epscross", "startrail" ],
          "default_curve": "hold"
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

This produces one legal enum value per frame.

## Enum PWM

Enum values can be temporally dithered using a PWM-like track mode.

The enum value itself is not continuous. The choice of enum value over
frames becomes a discretized signal whose duty cycle approximates a
continuous blend.

Use this only when the two enum values produce visually related results.

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

The user explicitly specifies which enum values are used for the PWM
off/on pair. Do not infer the pair from enum order.

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

- `a` must be a legal enum value.
- `b` must be a legal enum value.
- `off` must be a legal enum value.
- `on` must be a legal enum value.
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
- `random_seeded`
- `low_discrepancy`
- `blue_noise`

For reproducible builds, avoid unseeded randomness.

## Enum PWM Limitations

PWM approximates blending over time. A still frame is never blended. It
contains only one enum value.

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

If two enum values produce unrelated images, PWM becomes flicker rather
than interpolation.

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
- `Parameter 'inside' is enum, but value 'foo' is not listed.`
- `Parameter 'outside' uses PWM value 'atan', but 'atan' is not listed
  as a legal enum value.`

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

        std::vector<std::string> enum_values;
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

            if opacity is 0 and write_when_hidden is false:
                skip layer render
            else:
                append batch parameters
                append layer savename parameter
                write layer entry to output-directory/par/<output.par>
                write ID command using librarydirs and @par/name

        start with a transparent canvas

        for each rendered layer from bottom to top:
            apply evaluated opacity to layer alpha
            composite layer with its ImageMagick compose operator

        optionally flatten to output.background
        write final frame image

## Implementation Slices

Each slice should leave the program buildable, tested, and at least as
usable as before. Slices 2 through 5 are the minimum viable product:
new-format animations can write ID library-compatible par and batch
files for center-mag and corners. Later slices broaden one behavior at a
time while preserving that working path.

When a slice is implemented, remove it from this section.

### 1. Add Configuration JSON Schema

Create a JSON Schema for animation configuration files and wire tests to
validate existing config fixtures against it. The schema should cover the
current two-endpoint configuration format and the current output object
shape. Keep schema validation separate from runtime parsing.

Unit tests:

- valid sample and test configuration files pass schema validation.
- invalid output.directory type is rejected by schema validation.
- schema path is stable for test and tool use.

### 2. Parse The New Animation Envelope

Parse the new JSON envelope with source, output, video, num_frames, and an
empty tracks array. Reject the old from/to/interpolate format.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- a valid empty-track animation loads.
- old from/to/interpolate JSON is rejected.
- missing output.directory is rejected.

### 3. Load One Viewport Catalog

Load one catalog file and look up metadata for center-mag and corners.
Support type, format, default_curve, and extrapolate only in this slice.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- center-mag metadata loads from one catalog.
- corners metadata loads from one catalog.
- unknown animated parameter is rejected.
- missing metadata type is rejected.

### 4. Add Center-Mag Tracks

Add center_mag tracks to the new track engine. Use it to animate the real
ID center-mag parameter in the new JSON format and write a complete
generated par file plus render batch file.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- center-mag values are written in each generated par entry.
- center values interpolate linearly.
- magnification interpolates geometrically when positive.
- the generated batch uses librarydirs and @par/name.

### 5. Add Corners Tracks

Add corners tracks to the new track engine. Use it to animate the real ID
corners parameter in the new JSON format.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- 4-value corners interpolate all components.
- 6-value corners interpolate all components.
- mismatched corner arity is rejected.

MVP is complete after this slice.

### 6. Add Integer Tracks

Add one integer track with keyed linear interpolation and clamp
extrapolation. Use it to animate maxiter.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- maxiter values are written in each generated par entry.
- interpolation hits exact first and last key values.
- integer rounding is stable.

### 7. Add Hold And Step Curves

Add hold and step to the local curve evaluator. These curves apply to the
integer track.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- hold keeps the previous key value before the next key.
- step changes at the destination key.

### 8. Add Double Tracks

Add double tracks with min and max validation. Keep the same key and
curve machinery as integer tracks.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- bailout values preserve fractional input.
- min and max reject invalid key values.
- double tracks work in the generated par output.

### 9. Add Multiple Independent Tracks

Allow more than one normal ID parameter track in one animation. Tracks
may have different key frames.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- center-mag and maxiter can animate in the same file.
- each track uses its own key frames.
- generated par entries contain both assignments.

### 10. Add Base And Omit Extrapolation

Add base and omit extrapolation for existing integer and double tracks.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- base uses the value from the source par entry.
- omit writes no assignment outside the keyed range.
- clamp behavior from the MVP path still works.

### 11. Add Cycle And Ping-Pong Extrapolation

Add cycle and ping_pong extrapolation for existing scalar tracks.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- cycle maps frames back into the keyed range.
- ping_pong maps frames forward and backward.
- boundary frames are not duplicated incorrectly.

### 12. Add Type-Scoped Params Tracks

Add params as a type-scoped slash-list vector. For type=julia, support the
named complex group `params.c` over slots 0 and 1. Compose one `params=`
assignment from the source value plus track updates.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- type=julia resolves `params.c` to slots 0 and 1.
- `params.c` parses real and imaginary slash values.
- updating slot 0 preserves slot 1 from the source params.
- generated output writes one slash-delimited `params=` assignment.
- params slot 2 is rejected for a type with no slot 2.

### 13. Add Formula Entry Params Knobs

Add params knob metadata attached to formula entry names. The active
`formulaname` value selects the entry. Support integer, real, and complex
knobs that map to `p1` through `p4`.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- `formulaname=MandelbrotMix4` loads `formula_entries.MandelbrotMix4`.
- formula entry metadata names the `p1.real` use as `bailout`.
- formula entry metadata names the `p1.imag` use as `scale factor`.
- formula entry metadata names the `p2` use as `c`.
- real and integer knobs reject variables without `.real` or `.imag`.
- complex knobs reject variables with `.real` or `.imag`.
- formula params knobs reject `p5` variables.
- updating a formula knob preserves all untouched params values.
- overlapping formula knobs are rejected.

### 14. Add Formula Function Keys

Add formula-entry function metadata. Support enum keys named `fn1`
through `fn4` and write one composed `function=` assignment.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- formula entry metadata exposes `MandelbrotMix4.fn1`.
- `MandelbrotMix4.fn1` accepts legal ID function enum values.
- `MandelbrotMix4.fn1` rejects unknown function names.
- updating `fn2` preserves `fn1` from the source function value.
- generated output writes one slash-delimited `function=` assignment.

### 15. Add Numeric Tuple Tracks

Add numeric_tuple with metadata arity and slash formatting.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- a 2-value tuple writes a slash-delimited value.
- a 3-value tuple writes a slash-delimited value.
- wrong arity is rejected.

### 16. Add Point And Vector Aliases

Add point2, vector2, point3, and vector3 aliases over numeric_tuple.
Vector aliases support normalize=true.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- point3 writes a 3-value ID tuple.
- vector3 normalizes when requested.
- point aliases do not normalize.

### 17. Add Enum Hold Tracks

Add enum tracks with hold behavior.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- inside can hold bof60 then zmag.
- an enum value not listed in metadata is rejected.
- enum values are not numerically interpolated.

### 18. Add Enum Step Tracks

Add step behavior for enum tracks.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- inside changes at the destination key.
- hold behavior remains unchanged.
- missing enum values still fail validation.

### 19. Add Enum PWM Tracks

Add PWM mode for enum tracks using explicit a and b values.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- mix 0 emits only a.
- mix 1 emits only b.
- window values below 2 are rejected.

### 20. Read And Write ID Map Files

Add ID map file parsing and writing. Do not add animation effects yet.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- a 256-entry ID map file is parsed.
- malformed RGB entries are rejected.
- written map files use ID-compatible RGB values.

### 21. Add Static Colormap Tracks

Add colormap tracks that copy or emit one map per frame and return
colors=@filename. Generated maps are written under output-directory/map.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- colors track writes generated map files under map.
- colors assignment uses @filename only.
- source map filenames are not written as paths.

### 22. Add Colormap Interpolation

Add the interpolate colormap effect.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- blend 0 returns the first map.
- blend 0.5 averages matching entries.
- blend 1 returns the second map.

### 23. Add Colormap Rotation

Add the rotate colormap effect.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- positive offsets wrap palette entries.
- negative offsets wrap palette entries.
- offset 0 leaves the map unchanged.

### 24. Add Ranged Colormap Rotation

Add rotate_range for inclusive palette index ranges.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- only the selected range rotates.
- entries outside the range are unchanged.
- invalid ranges are rejected.

### 25. Add Colormap Sequence

Add the sequence effect for stepping through map filenames.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- the selected map changes at the expected frame.
- optional crossfade uses interpolation.
- missing sequence maps are rejected.

### 26. Add Colormap Reverse And Ping-Pong

Add reverse and ping_pong effects for whole maps and ranges.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- reverse flips the selected range.
- ping_pong alternates forward and backward offsets.
- invalid ranges are rejected.

### 27. Add Gradient Map Sources

Add generated gradient sources with indexed RGB stops.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- two stops fill all 256 entries.
- three stops interpolate each interval.
- RGB components outside 0 through 63 are rejected.

### 28. Add One Color Adjustment Effect

Add brightness as the first color adjustment effect.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- brightness scales each RGB component.
- values clamp to ID's 0 through 63 range.
- amount 1 leaves the map unchanged.

### 29. Add More Color Adjustment Effects

Add gamma, contrast, saturation, and hue_shift one at a time in one
reviewable change if the implementation is still small.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- each effect has one identity test.
- each effect has one non-identity test.
- each effect clamps output to ID's valid RGB range.

### 30. Add Masked Colormap Effects

Add pulse, mask_blend, remap, and seeded sparkle one at a time in one
reviewable change if the implementation is still small.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- pulse affects only the selected range.
- mask_blend affects only selected ranges.
- sparkle requires a seed and is repeatable.

### 31. Add Constant And Line Paths

Add constant and line path generators for scalar and complex tracks.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- constant returns the same value for every frame.
- line matches an equivalent keyed linear track.
- complex line paths preserve slash_pair formatting.

### 32. Add Circle And Ellipse Paths

Add circle and ellipse paths for complex and point tracks.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- circle returns to its start after one turn.
- ellipse uses independent x and y radii.
- phase changes the starting point.

### 33. Add Lissajous And Spiral Paths

Add lissajous and spiral path generators.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- lissajous is deterministic for the same phase and frequency.
- spiral radius changes over time.
- invalid frequency or radius values are rejected.

### 34. Add Bezier Paths

Add bezier path generation.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- bezier paths hit the first and last control points.
- too few control points are rejected.
- tuple-valued paths preserve arity.

### 35. Add Catmull-Rom Paths

Add catmull_rom path generation. This is the first point where
Boost.Math should be considered. Do not add it earlier. Keep it hidden
behind PathGenerator, and add it only if local code would be larger or
less clear. Consider TinySpline at the same point only if Catmull-Rom
needs richer spline features.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- paths pass through declared control points.
- too few control points are rejected.
- tuple-valued paths preserve arity.

### 36. Add Camera2D Corners Output

Add camera2d with look_at, view_up, and height curves targeting corners.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- axis-aligned camera writes expected corners.
- rotated camera writes expected third corner.
- view_up is normalized before output.

### 37. Add Camera2D Center-Mag Output

Add camera2d output to center-mag for axis-aligned cameras.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- axis-aligned camera writes expected center-mag.
- rotated camera targeting center-mag is rejected.
- aspect handling matches the source image shape.

### 38. Add Basic ID 3D View Adapter

Add id_3d_view output for rotation, perspective, and xyshift.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- rotation writes a 3-value slash tuple.
- perspective writes an integer value.
- xyshift writes a 2-value slash tuple.

### 39. Add More ID 3D View Outputs

Add scalexyz, roughness, sphere, longitude, latitude, radius, stereo,
interocular, and converge outputs.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- scalexyz writes three values.
- stereo controls write legal values.
- unsupported target outputs are rejected.

### 40. Add Julibrot View Adapter

Add julibrot_view output for 3dmode, julibrot3d, julibroteyes, and
julibrotfromto.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- 3dmode writes one legal enum value.
- julibrot3d writes six components.
- arbitrary look_at or view_up requests are rejected.

### 41. Add Single-Layer Stack

Allow animations to define one layer. It should behave like the existing
single-source animation but use the layer schema.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- one layer writes one par entry per frame.
- layer tracks apply to that layer.
- duplicate layer ids are rejected.

### 42. Add Multi-Layer Rendering

Allow multiple layers to render separate ID images before composition.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- layers are evaluated from bottom to top.
- each layer applies only its own tracks.
- generated layer entry names include layer id and frame number.

### 43. Add Layer Opacity

Add layer opacity evaluation and hidden-layer skipping.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- opacity 0 skips rendering by default.
- write_when_hidden renders opacity 0 layers.
- opacity values outside 0 through 100 are rejected.

### 44. Add ImageMagick Over Composition

Generate ImageMagick commands for Over composition.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- generated commands compose layers in stack order.
- opacity is applied before composition.
- output.background adds a flatten step when configured.

### 45. Add More ImageMagick Compose Operators

Allow configured ImageMagick compose operators and validate them.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- Screen and Multiply are accepted.
- unsupported operators are rejected.
- no ParAnimator-specific blend aliases are accepted.

### 46. Add Core Catalog Files

Add default catalogs for core ID parameters and coloring.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- core catalog declares type, maxiter, center-mag, and corners.
- coloring catalog declares colors as colormap.
- catalog inclusion fails clearly for missing files.

### 47. Add 3D And Formula Catalog Files

Add default catalogs for ID 3D viewing and selected formula families.

Schema work:

- create or update JSON schemas for fields or metadata JSON files added by
  this slice.

Unit tests:

- 3D catalog declares rotation and julibrot3d arity.
- formula catalogs attach params knob and function key metadata to formula
  entry names.
- animation files include only the catalogs they need.

The intent is not to finish a large subsystem before anything runs. The
intent is to get a small valid ID animation working quickly, then keep
that path working while each later feature is added.

## Design Boundary

Hard-code:

- track types
- curves
- path generators
- format parsers
- validation rules
- colormap effect algorithms
- ID map file writer
- layer stack evaluation
- ImageMagick command generation

Do not hard-code:

- Iterated Dynamics parameter names
- formula-specific parameter lists
- legal enum values
- which parameters are animatable
- virtual adapter output parameter names
- default curves for individual parameters
- enum values used by PWM tracks
- generated colormap filenames
- source map filenames
- ParAnimator-specific blend aliases
- dependency-specific curve or track names

## Summary

The final design is:

- one global frame clock
- many independent parameter timelines
- each timeline has its own keys, curves, type, and extrapolation
- local frame-addressable curve evaluation replaces tweeny
- parameter names and metadata come from JSON catalogs
- the animator knows types, not Iterated Dynamics parameter names
- virtual adapters map planned views onto real ID parameters
- colormap tracks can apply effects, write per-frame ID map files, and
  emit `colors=@file`
- optional layer stacks render ID layer images and compose them with
  ImageMagick operators
- enum parameters are discrete by default
- enum PWM is an optional temporal dithering mode
- PWM tracks explicitly choose their `a` and `b` enum values

This turns ParAnimator into a data-driven parameter animation sequencer
rather than a viewport interpolation tool.
