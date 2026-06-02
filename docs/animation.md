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

    frame 0 through num_frames - 1

Every track is evaluated against that same frame number. Tracks do not
need to share keyframe locations.

## Independent Parameter Timelines

Each parameter has its own timeline. This is important because unrelated
parameters should not be forced to change at the same frames.

Example conceptual timing:

    params for type=julia:
        frame 0      value A
        frame 300    value B
        frame 900    value A

    maxiter:
        frame 0      100
        frame 120    100
        frame 500    2000
        frame 900    2000

    colors:
        frame 0      fire.map
        frame 900    ice.map

    lightsource:
        frame 200    -1/-1/1
        frame 700    1/1/1

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

        "params": {
          "type": "complex",
          "description": "Julia constant for type=julia",
          "format": "slash_pair",
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

Useful metadata fields:

    type
    description
    format
    default_curve
    extrapolate
    min
    max
    modulus
    arity
    values
    rounding
    units
    normalize
    aliases
    required
    write_when_unchanged

Meanings:

    type
        Selects the parser, interpolator, and formatter.

    description
        Human-readable explanation of the parameter.

    format
        Describes the textual syntax used in the par file.

    default_curve
        Curve used by track segments unless overridden.

    extrapolate
        Behavior outside the track key range.

    min and max
        Validation limits. They may also be used for clamping.

    modulus
        Wrap range for cyclic numeric values.

    arity
        Number of components for tuple-like values.

    values
        Legal enum values.

    rounding
        How integer-like values are produced from continuous values.

    units
        Unit hint, such as degrees, radians, raw, or percent.

    normalize
        Whether vector-like values are normalized after interpolation.

    aliases
        Alternative parameter names accepted in par files.

    required
        Whether the parameter must exist in the base parameter set.

    write_when_unchanged
        Whether the generated frame should include the value even when it
        matches the base value.

## Type Versus Format

Type and format should be separate.

Do not make a separate type for every textual spelling.

Bad model:

    slash_complex
    comma_complex
    paren_complex

Better model:

    type: complex
    format: slash_pair

Possible formats:

    integer raw
    double raw
    complex slash_pair
    complex comma_pair
    numeric_tuple slash
    numeric_tuple comma
    point2 slash
    vector2 slash
    point3 slash
    vector3 slash
    color rgb_tuple
    colormap at_file
    angle degrees
    angle radians

This lets one complex interpolator support several textual syntaxes.

## Built-In Track Types

Minimum useful type set:

    string
    enum
    integer
    double
    complex
    numeric_tuple
    point2
    vector2
    point3
    vector3
    camera2d
    id_3d_view
    julibrot_view
    colormap
    center_mag
    corners
    rgb_color
    angle
    cyclic_integer
    cyclic_double

The animator may hard-code these types. That is a small type system, not
a list of Iterated Dynamics parameters.

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

    rotation
    perspective
    xyshift
    xyadjust
    scalexyz
    roughness
    sphere
    longitude
    latitude
    radius
    stereo
    interocular
    converge

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

    3dmode
    julibrot3d
    julibroteyes
    julibrotfromto

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

    z dots
    origin
    depth
    height
    width
    viewer distance

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

    key i - 1 to key i uses key i curve

Useful curves:

    linear
    step
    hold
    smoothstep
    smootherstep
    ease_in
    ease_out
    ease_in_out
    sine
    triangle
    sawtooth
    pulse
    ping_pong

Suggested behavior:

    hold
        Keep the previous key value until the next key is reached.

    step
        Jump at the destination key.

    linear
        Interpolate linearly in local segment time.

    smoothstep
        Ease in and ease out with a smooth polynomial.

    geometric
        Useful for positive scale or magnification values.

## Implementation Libraries

Do not use tweeny as the new animation engine. It is shaped around
object tweening, while ParAnimator needs frame-addressable typed tracks,
ID parameter formatting, virtual adapters, validation, and side effects
such as per-frame map files.

The new track engine should have a small local curve evaluator:

    local segment lookup by frame
    local easing functions
    local typed interpolation
    local extrapolation handling

This should replace the existing tweeny-backed interpolant path in the
first implementation slice. Once no source file includes tweeny, remove
tweeny from the dependency manifest.

Dependency guidance:

    Boost.JSON
        Keep for JSON parsing and serialization.

    Boost.Math interpolators
        Use behind path generators if Catmull-Rom, Bezier, Akima, PCHIP,
        or B-spline paths become useful.

    TinySpline
        Consider only if spline paths need control points, knots,
        arbitrary-dimensional splines, or arc-length sampling.

    GLM or local vector types
        Use for camera and 3D adapter math only if it reduces code.

    ImageMagick
        Keep as an external command-line renderer for layer composition.

Do not expose dependency-specific names or behavior in JSON. JSON names
belong to ParAnimator's track model.

## Extrapolation

Each track controls behavior before its first key and after its last key.

Useful extrapolation modes:

    clamp
        Use the nearest key value.

    base
        Use the value from the base parameter set.

    omit
        Do not write this parameter outside the keyed range.

    cycle
        Repeat the track.

    ping_pong
        Repeat the track forward and backward.

Safe default:

    clamp

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

    output-directory/par
        Generated par files.

    output-directory/map
        Generated map files.

    output-directory
        Generated batch scripts.

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

Layer fields:

    id
    source
    tracks
    opacity
    compose
    write_when_hidden

Meanings:

    id
        Stable identifier used in filenames and scoped track names.

    source
        Base parameter set for this layer.

    tracks
        Parameter timelines evaluated only for this layer.

    opacity
        Percent opacity. Animate opacity to 0 instead of inserting or
        deleting layers over time.

    compose
        ImageMagick compose operator used when this layer is placed over
        the current frame image.

    write_when_hidden
        Whether to render the layer even when evaluated opacity is 0.

    output.background
        Optional flatten color for final frame formats that do not keep
        alpha. If omitted, keep the composed frame alpha channel.

The compose value is an ImageMagick compositing operator name, such as:

    Over
    Multiply
    Screen
    Overlay
    HardLight
    SoftLight
    Darken
    Lighten
    Difference
    Plus
    Minus

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

    evaluate the source map
    apply each effect in order
    write output-directory/map/colors-0000.map
    return @colors-0000.map as the colors parameter value

Effect parameters may be constants or keyed scalar, tuple, color, or enum
tracks. This keeps timing local to the colormap track while reusing the
normal track interpolation machinery.

Core colormap effects:

    interpolate
        Blend two or more ID map files with keyed weights.

    sequence
        Step through map files, with optional crossfade frames.

    rotate
        Shift all palette indices by a keyed offset.

    rotate_range
        Shift only an inclusive index range.

    reverse
        Reverse the full map or one inclusive index range.

    ping_pong
        Oscillate an index range forward and backward.

    gradient
        Generate a map from keyed RGB color stops.

    hue_shift
        Rotate hue in HSL or HSV space.

    saturation
        Scale color saturation.

    brightness
        Scale color intensity.

    contrast
        Expand or compress color distance from midgray.

    gamma
        Apply nonlinear intensity shaping.

    posterize
        Reduce color levels to bands.

    remap
        Reindex the palette through a curve or lookup table.

    pulse
        Blend a range toward a keyed flash color.

    mask_blend
        Blend selected index ranges between maps.

    sparkle
        Apply seeded, bounded random color perturbations.

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

    parameter for ID tracks, or name for virtual tracks
    optional type override
    keyframes or path generator
    extrapolation behavior
    local options

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

    if frame is before the first key:
        apply before-range extrapolation

    if frame is after the last key:
        apply after-range extrapolation

    otherwise:
        find the enclosing key segment
        normalize local time to 0 through 1
        apply the segment curve
        interpolate typed values
        format the result in par-file syntax

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

    line
    circle
    ellipse
    lissajous
    spiral
    bezier
    catmull_rom
    constant
    ping_pong
    noise

A track can use keys, a path, or an expression. They all expose the same
runtime operation:

    value_at(frame)

## Formula-Specific Catalogs

Parameter metadata can be split into multiple files.

Suggested layout:

    parameters/core.json
    parameters/coloring.json
    parameters/3d.json
    parameters/fractals/mandel.json
    parameters/fractals/julia.json
    parameters/fractals/phoenix.json
    parameters/fractals/newton.json
    parameters/fractals/ifs.json

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

      "parameters": {
        "params": {
          "type": "complex",
          "format": "slash_pair",
          "default_curve": "linear"
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

    mix = 0.0
        Always emit value a.

    mix = 1.0
        Always emit value b.

    mix = 0.25
        Emit value b for about 25 percent of frames.

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

    a must be a legal enum value
    b must be a legal enum value
    off must be a legal enum value
    on must be a legal enum value
    a and b may be equal, but this should warn
    mix must be in the range 0 through 1 unless clamping is enabled
    duty must be in the range 0 through 1 unless clamping is enabled
    window must be at least 2

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

    regular
    ordered
    random_seeded
    low_discrepancy
    blue_noise

For reproducible builds, avoid unseeded randomness.

## Enum PWM Limitations

PWM approximates blending over time. A still frame is never blended. It
contains only one enum value.

If true per-pixel or spatial blending is desired, that belongs in the
renderer, not ParAnimator.

PWM works best for:

    coloring mode
    inside method
    outside method
    decomposition mode
    orbit trap mode
    palette selection
    rendering style
    overlay mode

PWM is usually bad for:

    fractal formula
    major coordinate interpretation
    symmetry mode
    precision mode
    rendering backend
    algorithm switch that changes image structure completely

If two enum values produce unrelated images, PWM becomes flicker rather
than interpolation.

## Validation

On startup:

    load parameter catalogs
    merge includes and overrides
    load the base parameter set
    load animation tracks
    for each track:
        look up parameter metadata
        validate keys and options
        construct the typed track

Errors should be specific.

Examples:

    Unknown animated parameter 'param'.
    No metadata exists for 'param'.
    Did you mean 'params'?

    Parameter 'maxiter' is type integer, but key at frame 120 has
    value 'abc'.

    Parameter 'inside' is enum, but value 'foo' is not listed.

    Parameter 'outside' uses PWM value 'atan', but 'atan' is not listed
    as a legal enum value.

## JSON File Replacement

The new animation JSON format replaces the existing JSON files. Do not
support old from/to/interpolate JSON as compatibility syntax.

Required behavior:

    load only the new source/tracks animation schema
    reject old from/to/interpolate JSON with a clear error
    provide examples that use only the new schema
    remove code paths that desugar old JSON into tracks

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
        std::optional<int> modulus;
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

## Priority Order

Implement in this order:

    1. local segment evaluator and easing table replacing tweeny
    2. parameter catalog loading
    3. integer, double, numeric_tuple, point2, vector2, point3, and
       vector3 tracks
    4. per-parameter key timelines
    5. complex tracks
    6. angle and cyclic tracks
    7. enum hold and enum step tracks
    8. enum PWM tracks
    9. colormap tracks, ID map reading, and map file writing
    10. basic colormap interpolation and rotation effects
    11. gradient and color adjustment colormap effects
    12. path generators
    13. camera2d virtual tracks
    14. id_3d_view and julibrot_view virtual tracks
    15. layer stacks
    16. ImageMagick compose operators and opacity
    17. formula-specific catalog files

This order gets useful behavior early while keeping the design open.

## Design Boundary

Hard-code:

    track types
    curves
    path generators
    format parsers
    validation rules
    colormap effect algorithms
    ID map file writer
    layer stack evaluation
    ImageMagick command generation

Do not hard-code:

    Iterated Dynamics parameter names
    formula-specific parameter lists
    legal enum values
    which parameters are animatable
    virtual adapter output parameter names
    default curves for individual parameters
    enum values used by PWM tracks
    generated colormap filenames
    source map filenames
    ParAnimator-specific blend aliases
    dependency-specific curve or track names

## Summary

The final design is:

    one global frame clock
    many independent parameter timelines
    each timeline has its own keys, curves, type, and extrapolation
    local frame-addressable curve evaluation replaces tweeny
    parameter names and metadata come from JSON catalogs
    the animator knows types, not Iterated Dynamics parameter names
    virtual adapters map planned views onto real ID parameters
    colormap tracks can apply effects, write per-frame ID map files, and
    emit colors=@file
    optional layer stacks render ID layer images and compose them with
    ImageMagick operators
    enum parameters are discrete by default
    enum PWM is an optional temporal dithering mode
    PWM tracks explicitly choose their a and b enum values

This turns ParAnimator into a data-driven parameter animation sequencer
rather than a viewport interpolation tool.
