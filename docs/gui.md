# GUI Timeline Design

## Decision

Build a reusable timeline editor library. Do not couple the reusable
pieces to ParAnimator or to Iterated Dynamics terminology.

The library should use neutral names:

- `TimelineCore`
- `TimelineWx`
- `TimelineTests`

The public CMake targets should be namespaced:

- `Timeline::Core`
- `Timeline::Wx`

The public C++ API should live in namespace `Timeline`.

ParAnimator-specific code belongs outside the reusable library. Use an
adapter layer for ParAnimator configuration, serialization, validation,
and command wiring.

Likely ParAnimator-facing names:

- `ParAnimatorTimelineAdapter`
- `ParAnimatorTimelineFrame`
- `ParAnimatorTimelineSerialization`

## Boundary

Domain state owns truth. wxWidgets owns pixels and events.

`TimelineCore` should have no wxWidgets dependency. It should model the
timeline in terms of frames, tracks, keys, values, selection, and edit
commands. It should be testable without a GUI.

`TimelineWx` should render and edit a `TimelineCore` model. It should
translate mouse, keyboard, scroll, zoom, and focus events into model
commands.

## Core Model

Use integer frame numbers as the timeline coordinate. Do not use wall
time or floating point time values as primary keys.

Core model responsibilities:

- global frame range
- independent tracks
- typed key values
- selected keys and tracks
- edit commands for add, move, delete, and update
- snapping and frame clamping rules
- curve and extrapolation metadata
- undoable command records

The model should match the animation engine's shape: frame-addressable
typed tracks. The GUI must not invent a separate timeline truth that then
has to be reconciled with generated animation data.

## wxWidgets Control

The reusable wxWidgets layer should be embeddable as a control, not as an
application frame.

Expected pieces:

- timeline panel
- ruler
- stacked track lanes
- key glyph rendering
- curve preview rendering
- playhead rendering
- horizontal and vertical scrolling
- zoom controls
- hit testing
- keyboard navigation
- mouse selection and dragging

The app that embeds the control owns menus, documents, persistence, and
preview playback policy.

## Third-Party Evaluation

`adct-the-experimenter/timeline-track-editor` was evaluated as a possible
wxWidgets timeline control.

Conclusion: do not adopt it as a production control.

Useful ideas:

- scrolled timeline window
- ruler drawing with `wxDC`
- stacked track panels
- playback timer concept

Blocking issues:

- It is a demo application, not a reusable control library.
- CMake builds an executable, not an installable or linkable library.
- The top-level timeline container is a `wxFrame`, not an embeddable
  `wxPanel` or `wxWindow`.
- Timeline dimensions, time range, and timer resolution are global
  compile-time constants in `parameters.h`.
- Track keys are stored by floating point time values.
- It has no model/view split.
- It has no event or command model for edits.
- It has no serialization boundary suitable for ParAnimator configs.
- It has no undo, selection model, key dragging, zoom model, or snapping
  model.
- Playback mutates an external `double *`, which does not fit typed
  frame-addressed tracks.
- The build is Unix-biased and links directly against `openal.so` and
  `sndfile.so`.
- Audio support pulls in dependencies that are unrelated to the core
  timeline editor.
- There are no tests.
- There are no releases.
- The last observed commit was `2019-12-18`.
- Public adoption is small: 8 stars and 0 forks at evaluation time.

Borrow concepts only. Do not copy the code into the project.
