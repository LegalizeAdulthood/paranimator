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

ParAnimator should not know that maxiter, julia, inside, colors, or
lightx exist. It should know how to animate declared parameter types.

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

    julia parameter:
        frame 0      value A
        frame 300    value B
        frame 900    value A

    maxiter:
        frame 0      100
        frame 120    100
        frame 500    2000
        frame 900    2000

    palette offset:
        frame 0      0
        frame 900    256

    light angle:
        frame 200    0 degrees
        frame 700    360 degrees

These are independent rhythms:

- Julia shape mutates over the shot.
- Iteration detail fades in after a delay.
- Palette phase rotates continuously.
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

        "julia": {
          "type": "complex",
          "format": "slash_pair",
          "default_curve": "smoothstep",
          "extrapolate": "clamp"
        },

        "colors": {
          "type": "cyclic_integer",
          "modulus": 256,
          "default_curve": "linear",
          "extrapolate": "cycle"
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

        "lightx": {
          "type": "double",
          "default_curve": "smoothstep"
        },

        "lighty": {
          "type": "double",
          "default_curve": "smoothstep"
        },

        "lightz": {
          "type": "double",
          "default_curve": "smoothstep"
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
    values
    rounding
    units
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
        Wrap range for cyclic values such as palette offsets.

    values
        Legal enum values.

    rounding
        How integer-like values are produced from continuous values.

    units
        Unit hint, such as degrees, radians, raw, or percent.

    aliases
        Alternative parameter names or old spellings.

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
    tuple slash
    tuple comma
    color rgb_tuple
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
    center_mag
    corners
    rgb_color
    angle
    cyclic_integer
    cyclic_double

The animator may hard-code these types. That is a small type system, not
a list of Iterated Dynamics parameters.

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

      "output": "frames.par",
      "script": "render.cmd",
      "frame": "frame%04d",
      "video": "yes",
      "num_frames": 900,

      "tracks": [
        {
          "parameter": "julia",
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
          "keys": [
            { "frame": 0,   "value": 0 },
            { "frame": 900, "value": 256, "curve": "linear" }
          ]
        }
      ]
    }

The track does not need to repeat the type if the catalog declares it.

## Track Structure

A track has:

    parameter name
    optional type override
    keyframes or path generator
    extrapolation behavior
    local options

Conceptual C++ interface:

    class Track
    {
    public:
        std::string parameter() const;
        std::string value_at(int frame) const;
    };

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
      "parameter": "julia",
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
        "my_custom_param": {
          "type": "numeric_tuple",
          "arity": 3,
          "format": "slash",
          "default_curve": "linear"
        }
      }
    }

This lets local experiments define custom metadata without modifying the
default catalog.

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

    Unknown animated parameter 'julac'.
    No metadata exists for 'julac'.
    Did you mean 'julia'?

    Parameter 'maxiter' is type integer, but key at frame 120 has
    value 'abc'.

    Parameter 'inside' is enum, but value 'foo' is not listed.

    Parameter 'outside' uses PWM value 'atan', but 'atan' is not listed
    as a legal enum value.

## Backward Compatibility

The existing from/to/interpolate config can remain as compatibility
syntax.

Old-style concept:

    {
      "from": { "file": "a.par", "name": "a" },
      "to":   { "file": "b.par", "name": "b" },
      "interpolate": [ "center-mag" ],
      "num_frames": 300
    }

Internally, desugar this to:

    {
      "source": { "file": "a.par", "name": "a" },
      "tracks": [
        {
          "parameter": "center-mag",
          "type": "center_mag",
          "keys": [
            { "frame": 0,   "value": "value from a" },
            { "frame": 299, "value": "value from b" }
          ]
        }
      ]
    }

The old model becomes a shorthand for a one-track, two-key animation.

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
            set frame parameter to track value at frame_number

        append batch parameters
        append savename parameter
        append overwrite parameter
        append video parameter

        write frame to output par file
        write script command for frame

This preserves the current ParAnimator workflow while allowing much richer
animation.

## Priority Order

Implement in this order:

    1. parameter catalog loading
    2. integer, double, and numeric_tuple tracks
    3. per-parameter key timelines
    4. easing curves
    5. complex tracks
    6. angle and cyclic tracks
    7. enum hold and enum step tracks
    8. enum PWM tracks
    9. color tracks
    10. path generators
    11. formula-specific catalog files
    12. compatibility desugaring for old from/to configs

This order gets useful behavior early while keeping the design open.

## Design Boundary

Hard-code:

    track types
    curves
    path generators
    format parsers
    validation rules

Do not hard-code:

    Iterated Dynamics parameter names
    formula-specific parameter lists
    legal enum values
    which parameters are animatable
    default curves for individual parameters
    enum values used by PWM tracks

## Summary

The final design is:

    one global frame clock
    many independent parameter timelines
    each timeline has its own keys, curves, type, and extrapolation
    parameter names and metadata come from JSON catalogs
    the animator knows types, not Iterated Dynamics parameter names
    enum parameters are discrete by default
    enum PWM is an optional temporal dithering mode
    PWM tracks explicitly choose their a and b enum values

This turns ParAnimator into a data-driven parameter animation sequencer
rather than a viewport interpolation tool.
