# To Do

## Catalogs

- Create complete catalog for all Id parameters
- Create complete catalog for all fractal types
  - Include specific interpretation of param or function
- Create complete catalog for all formulas in id.frm

### Parameters

- Core catalog:
  - finattract needs type "yes-no" `bool`: "yes", "y", "no", or "n"
  - ismand needs type "yes-no"
  - miim needs type "miim":
    `[bdw][lr]`/`double`/`double`/`double`/`double`/`double`; not
    documented
  - orbitcorners: not documented
  - orbitdrawmode: `function` not documented
  - params: needs type?
  - potential needs type "potential": maxcolor[/slope[/modulus[/16bit]]]]
  - proximity not documented
  - screencoords needs type "yes-no"
  - showdot missing
  - showorbits needs type "yes-no"
- Coloring catalog:
  - colors allows encoded spec?
  - decomp needs type `int`
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
