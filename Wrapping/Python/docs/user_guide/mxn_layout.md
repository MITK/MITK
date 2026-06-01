# MxN layout DSL

`mitk.mxn.layout` is a typed Python DSL for constructing, inspecting,
transforming, and round-tripping the **multi-widget layout documents**
that drive `QmitkMxNMultiWidget`, the workbench's configurable multi-window
view.

The DSL targets layout schema v2.0. Documents round-trip byte-stable with
the JSON consumed by `QmitkMxNMultiWidget::ApplyLayout` and produced by
`QmitkMxNMultiWidget::SerializeLayout`. The package is pure Python and
has no Qt or MITK-runtime dependency, so it is usable in scripts and
notebooks without a built workbench.

## Mental model

A layout document is a tree:

- `Split` nodes (horizontal or vertical) subdivide the available area.
- `LayoutWindow` leaves are individual render windows, each with an
  anatomical `view_direction` (axial / sagittal / coronal / original)
  and an `id` that doubles as the routing identity used by REST URL
  paths and persisted state keys.

Cross-window synchronization is encoded through **groups** and **links**.
Each window declares one or more links (today only the `selection`
dimension) referencing a named `Group`. Windows linked to the same group
share their selection state at runtime.

Every public name (the dataclasses, the builders, the selector, the
preset registry, and the file-I/O helpers) is re-exported from
`mitk.mxn.layout` itself, so a single import covers the typical script.

## Build a layout

The fastest path is a built-in builder. Builders return a `Split` tree
that you wrap in an `MxNLayoutDocument`:

```python
from mitk.mxn.layout import MxNLayoutDocument, grid

doc = MxNLayoutDocument.create(
    root=grid(2, 3),
    name="2 x 3 grid",
)
print(doc.window_ids())
# ['mxn__widget0', 'mxn__widget1', ..., 'mxn__widget5']
```

Builders cover the common cases: `grid`, `three_up`,
`two_rows_each_direction`, `single_window`. They synthesise qualified
ids of the form `mxn__widget<i>`, matching the convention the engine
emits.

For named, registered layouts, use `preset`:

```python
from mitk.mxn.layout import list_presets, preset

print(list_presets())
# ['single', 'three-up', 'two-rows-each-direction']
doc = preset("three-up")
```

## Custom layouts

For anything the builders do not cover, compose `Split` and
`LayoutWindow` manually. Both validate their input on construction, so
malformed values are rejected immediately:

```python
from mitk.mxn.layout import (
    LayoutWindow,
    MxNLayoutDocument,
    Split,
    ViewDirection,
)

custom = Split.vertical(
    Split.horizontal(
        LayoutWindow.create(id="mxn__alpha", view_direction=ViewDirection.AXIAL),
        LayoutWindow.create(id="mxn__beta", view_direction="sagittal"),
    ),
    LayoutWindow.create(id="mxn__gamma", view_direction="coronal"),
)
doc = MxNLayoutDocument.create(root=custom, name="Hand-stitched")
```

Window ids must match the qualified form `<editor_name>__<bare_id>` (for
example `mxn__alpha`); a bare id like `alpha` raises `ValueError`. If
you stitched a tree with ad-hoc ids and want canonical, contiguous
numbering, call `with_default_ids()`:

```python
renumbered = doc.with_default_ids()
print(renumbered.window_ids())
# ['mxn__widget0', 'mxn__widget1', 'mxn__widget2']
```

```{warning}
Renaming an id that has already been served via REST or saved to a
session breaks every cached reference (persisted state, REST URL
bookmarks, per-renderer DataNode property keys). Use `with_default_ids`
on freshly built or freshly loaded documents, before they leave the
process.
```

## File I/O

Documents serialize to v2.0 JSON. The wire format matches what the
workbench consumes:

```python
from mitk.mxn.layout import load_preset, save_preset

save_preset("layout.json", doc)
restored = load_preset("layout.json")
assert restored == doc
```

For programmatic access to the JSON dict (for example to compose a
larger payload), call `to_json()` / `from_json()` directly:

```python
import json

payload = doc.to_json()
print(json.dumps(payload, indent=2))

doc_again = MxNLayoutDocument.from_json(payload)
```

## Inspecting and transforming layouts

`MxNWindowSelector` provides a pandas-style filter and bulk-transform
over a document's windows. Access it via `select_windows`:

```python
from mitk.mxn.layout import ViewDirection

# Read: collect every axial window.
axials = doc.select_windows.where(view_direction=ViewDirection.AXIAL).to_list()

# Update: rename every axial cell.
relabelled = doc.select_windows.where(view_direction="axial").with_display_name(
    lambda w: f"Axial ({w.id.split('__', 1)[1]})"
)

# Update: re-group every axial cell to a different selection group.
regrouped = doc.select_windows.where(view_direction="axial").link_to("row2")
```

The selector is immutable: every terminal returns a new document and
leaves the original untouched. On document sources, materialising a
previously-unknown group name via `link_to` auto-creates a `Group` entry
with default properties so the strict-mode invariant
(every link references a declared group) is preserved.

Filters compose AND-style with `where(...)`. Convenience shortcuts
(`by_id`, `by_display_name`, `by_view`) cover the common single-keyword
filters.

## Groups and links

For documents with multiple synchronization groups, declare them up
front so non-default properties (today: `select_all`) survive the
round trip:

```python
from mitk.mxn.layout import Group, MxNLayoutDocument, two_rows_each_direction

doc = MxNLayoutDocument.create(
    root=two_rows_each_direction(),
    groups=[
        Group("main", select_all=True),
        Group("row2", select_all=False),
    ],
    name="Two synchronized rows",
)
```

Passing `groups=None` (the default) auto-materialises one default
`Group` for every label any window references. The explicit form is
only needed when you want non-default group properties or you want to
declare a group up front even though no window links to it yet.

## See also

- The runnable script `mitk.mxn.layout.examples.01_getting_started`
  (next to the package source) walks through the same surface, end to
  end. The companion notebook of the same name renders the HTML repr.
- Schema reference: `Modules/QtWidgets/resource/mxn-layout-v2.schema.json`
  in the MITK source tree.
- {doc}`../api/index` (MxN layout DSL section) for the complete API.
