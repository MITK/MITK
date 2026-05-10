# `mitk.mxn.layout` examples

User-facing pedagogy for the layout DSL. These are **examples**, not test
fixtures: they live next to the package source so a developer who edits
the DSL is reminded to update them.

## Files

- **`01_getting_started.py`** -- runnable plain-Python tour. Executed as
  part of the `mitkPythonBindingsTest` CTest entry via pytest (the test
  function `test_example_script_runs` in
  `Modules/Python/test/pytest/test_layout/test_examples_smoke.py`); CI
  fails if any covered surface regresses.
- **`01_getting_started.ipynb`** -- same content as a Jupyter notebook so
  the `_repr_html_` output is visible when browsing the repo. Not
  executed in CI; re-execute and commit before merging changes that
  touch the DSL surface. (Editing the `.py` and `.ipynb` in lockstep is
  the maintenance contract; both files cross-reference each other in
  their headers.)

## What the example covers

End-to-end, both files demonstrate:

1. Building a 2 x 3 grid layout with `grid(...)` + `MxNLayoutDocument.create(...)`.
2. Composing a custom layout with `Split.horizontal(...)` / `Split.vertical(...)`,
   then renumbering ids cleanly via `with_default_ids()`.
3. Round-tripping the document via `save_preset` -> `load_preset`
   (using a temp file to avoid an external on-disk dependency).
4. Renaming a cell's display label with
   `MxNWindowSelector.with_display_name(...)`.
5. Re-grouping windows via `selector.where(view_direction="axial").link_to("row2")`
   and observing the document's groups registry materialise the new entry.
6. Triggering a validation error (passing a bare-id to `LayoutWindow.create`
   raises `ValueError`).

## Why next to the package, not under `Examples/`

`Examples/` is the C++ tutorial directory; mixing Python pedagogy in is
a category error. `Modules/Python/test/` is for verifying behaviour, not
teaching it. Co-locating examples with the package source ties their
lifecycle directly to the DSL implementation.

The wheel does not ship these examples -- the `install(... PATTERN
"examples" EXCLUDE ...)` rule in `Wrapping/Python/mitk/CMakeLists.txt`
keeps them out. Wheel users who want them fetch from the source repo.
