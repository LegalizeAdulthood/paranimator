# To Do

## Catalogs

- Create complete catalog for all Id parameters
- Create complete catalog for all fractal types
  - Include specific interpretation of param or function
- Create complete catalog for all formulas in id.frm

### Parameters

- Core catalog:
  - fillcolor needs type "fill-color": "normal" or `int` >= 0
  - finattract needs type "yes-no" `bool`: "yes", "y", "no", or "n"
  - function needs type "function-list": values must be one of specific enum list
  - initorbit needs type "init-orbit": "pixel" or `double`/`double`
  - inside: allows integer value?
  - invert needs type "invert": radius/x/y
  - ismand needs type "yes-no"
  - mathtolerance needs type "math-tolerance": `double`/`double`
  - miim needs type "miim": `[bdw][lr]`/`double`/`double`/`double`/`double`/`double`;
    not documented
  - orbitcorners: not documented
  - orbitdrawmode: `function` not documented
  - outside: allows integer value?
  - params: needs type?
  - passes needs type "enum": `1`, `2`, `3`, `g`, `g1`, `g2`, `g3`, `g4`, `g5`, `g6`, `b`, `t`, `s`, `o`, `p`
  - periodicity needs type "periodicity": `no`, `show`, or `int`
  - potential needs type "potential": maxcolor[/slope[/modulus[/16bit]]]]
  - proximity not documented
  - screencoords needs type "yes-no"
  - showdot missing
  - showorbits needs type "yes-no"
- Coloring catalog:
  - colors allows encoded spec?
  - decomp needs type `int`
  - distest needs type "distest": `double`/`double`
  - logmap needs type "logmap": yes, no, or `int`
  - nobof needs type "yes-no"
  - olddemmcolors needs type "yes-no"
  - truecolor needs type "yes-no"
  - truemode needs type "enum": `def`, `iter`
- Formula catalog:
  - Only document parameters for formulas in id.frm

## Color

- Specify RGB colors as floats
- Specify gamma?

## Platforms

- Linux support
  - Output bash scripts, not batch scripts

## Documentation

- User documentation
- Demonstration config.json for all features

## Code

- GUI Timeline editor
