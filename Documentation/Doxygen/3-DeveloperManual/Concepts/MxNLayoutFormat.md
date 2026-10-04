MxN Layout File Format (v3.0) {#MxNLayoutFormatPage}
=============================

[TOC]

This page is the human-friendly walkthrough of the JSON layout file format used
by the MxN multi-widget editor. The on-disk preset files
(`Modules/QtWidgets/resource/mxnLayout_*.json`), the documents accepted by
`QmitkMxNMultiWidget::ApplyLayout`, and the documents emitted by
`QmitkMxNMultiWidget::SerializeLayout` all share this single format.

The closed, normative reference is the JSON Schema
`Modules/QtWidgets/resource/mxn-layout-v3.schema.json` (Draft 2020-12); v2.0
documents, which remain loadable, are described by
`Modules/QtWidgets/resource/mxn-layout-v2.schema.json`. When
something below is ambiguous or contradicts the schema, the schema wins. This
page focuses on intent, examples, and the everyday parts of the format; the
schema covers the closed list of accepted enum values, exact regex patterns,
and the `additionalProperties: false` closure rules.

Related runtime concepts are not duplicated here; see the @ref MxNConceptPage
"MxN Multi-Widget concept page" for `QmitkMxNMultiWidget`,
`QmitkSynchronizedNodeSelectionWidget`, and `QmitkSynchronizedWidgetConnector`,
which together implement the runtime side of the synchronization the layout
document describes.

## What the format describes — and what it does not

A layout document captures:

- The **splitter tree** that arranges render windows on screen (rows, columns,
  nesting, splitter weights).
- Per-window **placement state** (window id, optional display name, view
  direction, links into synchronization groups).
- Per-group **persisted state**: the `select_all` UX-mode of the selection
  bundle, and an optional display name and color.

A layout document does **not** capture global rendering state (selected
position, current time step, camera) or the loaded data set. Those are owned
by other parts of the system. Loading a layout into a session that already has
data nodes simply applies the new geometry; the data stays.

## Top-level shape

```json
{
  "version": "3.0",
  "name": "Optional preset name",
  "groups": {
    "main": { "select_all": true }
  },
  "root": {
    "type": "split",
    "orientation": "horizontal",
    "children": [
      {
        "type": "window",
        "id": "mxn__widget0",
        "name": "Tumor axial",
        "view_direction": "axial",
        "links": { "selection": "main" },
        "size": 1
      }
    ]
  }
}
```

- `version`: the exact string `"3.0"` or `"2.0"`. The C++ loader rejects any
  other value, and `SerializeLayout` always writes `"3.0"`. A v2.0 document is
  a v3.0 document that only uses the selection dimension and loads unchanged
  (see "Versions and closure" below). v1.x files are upgraded out-of-band —
  see "Migrating from v1.x" below.
- `name`: optional human-readable preset name. Pure metadata; ignored by the
  loader and not used for routing. (The window leaves carry an optional
  `name` of their own, used the same way — see "Window identity and display
  label" below.)
- `groups`: optional. When present, authoritative. See "Lazy vs. strict mode"
  below.
- `root`: required. The root of the splitter tree. Always a `split`, even for
  a single window (which is then a one-child split).

## Tree nodes: `split` and `window`

A node in the tree is one of two kinds, distinguished by `type`:

```json
{
  "type": "split",
  "orientation": "horizontal" | "vertical",
  "size": 1,              // omitted on the document root; optional elsewhere
  "children": [ <node>, ... ]
}
```

```json
{
  "type": "window",
  "id": "mxn__widget0",
  "name": "Tumor axial",  // optional display label; omit if absent
  "view_direction": "axial",
  "links": { "selection": "<groupName>" },
  "size": 1               // optional; defaults to 1 when omitted
}
```

A `split` divides its area among its children along one axis. `horizontal`
arranges children left-to-right; `vertical` arranges them top-to-bottom. (Same
semantics as Qt's `QSplitter::Horizontal` / `QSplitter::Vertical`.) Children
must be a non-empty array. The document root has no parent and therefore no
`size`.

A `window` is a leaf render-window. Every window must declare:

- `id`: the window's identity. A unique identifier within the document, in
  the canonical fully-qualified form `<editor_name>__<bare_id>`. The same
  string is used verbatim as the engine-side render-window name registered
  with the rendering manager, as the URL path segment in REST sub-resources,
  as the per-renderer DataNode property context key, and in persisted
  session state. The loader does not prepend or strip a prefix at any
  boundary. Schema-enforced shape:
  `^[A-Za-z][A-Za-z0-9.-]*__[A-Za-z0-9_.-]+$`. The editor-name segment
  contains no `_`, the namespace delimiter is the literal `__`, and the
  bare-id segment uses the existing URL-segment-safe alphabet.
- `view_direction`: which anatomical plane the window shows. The closed enum
  is `"axial"`, `"sagittal"`, `"coronal"`, `"original"` (lowercase). Unknown
  strings throw at load time; there is no silent fallback.
- `links`: per-dimension synchronization references. v2.0 has one dimension,
  `selection`; v3.0 adds seven navigation and appearance dimensions (see
  "Navigation and appearance links" below). Every window must declare
  `links.selection` explicitly -- there are no implicit singletons. The other
  dimensions are optional; a window without one is unsynchronized on it.

A window may additionally declare:

- `name`: optional human-readable display label. Free-form (no pattern
  constraint, not required to be unique within the document). Pure metadata;
  the loader does not use it for routing, addressing, persisted-state
  keying, or REST URL construction (those all use `id`). Omit the field
  entirely if the cell has no display name — empty strings are rejected.

### `size` is a ratio, not pixels

`size` on a child is a splitter weight. Only the **ratio between siblings**
matters; Qt redistributes weights proportionally on resize. Absolute values
are NOT pixel measurements -- prefer small numbers (e.g. `1`, `2`, `3`) over
screenshot-derived pixel counts. `[size: 1, size: 1, size: 1]` and
`[size: 100, size: 100, size: 100]` produce the exact same layout.

`size` is **optional**. When omitted, the cell takes a default weight of `1`.
Mixed-defined siblings compute as ratios:

- `[size: 3, default, default]` -> `3:1:1` (first cell is 3/5 of the row)
- `[size: 2, size: 1]`          -> `2:1` (first cell is 2/3 of the row)
- `[default, default, default]` -> `1:1:1` (equal split)

`size` must be `>= 1` if present (the schema rejects `0` and the C++ loader
throws on `< 1`). The format has no first-class way to hide a cell while
keeping it in the tree; `size: 0` is not a valid stand-in.

## Sync groups: cells reference, properties live in `groups`

Cells share runtime state by linking to a group name:

```json
"links": { "selection": "main" }
```

Two cells with the same `links.selection` value are mutually linked: they
share the connector that mediates selection list, visibility, and stack
order. Group names are arbitrary URL-segment-safe strings (alphanumeric,
underscore, dot, hyphen). The same group name can be used across multiple
dimensions in v3 (e.g. `"selection": "main", "zoom": "main"`); group names
live in a single namespace and dimensions are orthogonal.

One selection group is the *default group*: the one every fresh cell joins
and an unlinked selection reverts to. A selection group named `main`
becomes the default when the document declares one; otherwise the
alphabetically first selection group referenced by any cell takes that role.

Per-group persisted state lives once at the top level:

```json
"groups": {
  "main": { "select_all": true },
  "row2": { "select_all": false }
}
```

The properties of a group entry:

- `select_all` (the selection bundle's UX mode: whether the group displays
  every data node or a curated subset). Optional, defaults to `true`. It
  applies to every window whose `links.selection` names the group, including
  windows that join it later.
- `color` (v3.0): optional hue as `"#RRGGBB"`, shown verbatim by the sync
  furniture. Without it the editor assigns a default hue by group creation
  order. A malformed value is ignored with a warning, never rejected.
- `name` (v3.0): optional display name, shown instead of the group id on
  every surface. The id (the dict key) stays the URL-safe identity.

Groups referenced only by navigation dimensions may use an empty entry `{}`;
such a group exists while some window links it and goes with its last link.
A declared group is kept as a group of its own (shown by the layout editor
even while empty and written back by `SerializeLayout` with its
`select_all`) when no window references it or when its entry carries
`select_all`.

There is no per-cell `select_all`. The setting belongs to the group, not to
any one of its members.

## Navigation and appearance links

v3.0 adds seven dimensions beside `selection`, each linking a window into a
group independently of the others:

| Dimension | What the group shares | Offset |
| --- | --- | --- |
| `slice` | the slice position | integer, in displayed slices |
| `zoom` | the zoom factor | number > 0, a multiplicative factor |
| `pan` | the in-plane position | `[x, y]`, in world mm along the window's own plane axes |
| `crosshair` | the selected world position | none |
| `orientation` | the view direction | none |
| `windowing` | level/window | none |
| `lut` | the colormap | none |

Every dimension accepts the bare group name. `slice`, `zoom` and `pan` also
accept an object form carrying an offset:

```json
"links": {
  "selection": "main",
  "slice": { "target": "nav", "offset": -1 },
  "zoom": "nav"
}
```

Every member's offset, the first member's included, is relative to one
common group reference, so only the differences between the offsets carry
meaning: offsets of `-1`, `0` and `+1` and offsets of `0`, `+1` and `+2`
describe the same layout. The group's *seed* is its first member in document
order. When the layout is applied, the seed stays where it is and defines
the reference (its live state minus its own offset); every other member
converges to the reference combined with its own offset. From then on the
group moves together, which keeps the offsets. A member clamped at the end of
its slice range or camera bounds can lose its offset; the editor's
re-converge action restores it. For slices, re-converge takes the reference
from the members that sit strictly inside their slice range, following the
reference most of them agree on, so a clamped member is moved back rather
than the group shifted to it. Only when every member sits on the first or
last slice of its range does the seed define the reference, clamped or not.

A slice offset counts displayed slices: the index the navigator shows, which
follows the image's own index axis for the view direction and can run
opposite to the slice stepper. Offsets of `-1`, `0` and `+1` on three members
therefore show the previous, the same and the next slice, whatever the
view's stepping direction.

A pan offset and every shared pan move are applied in the axes of each
window's own plane. They therefore point the same way in the patient only
across windows that show the same plane; across planes the in-plane vector is
applied as is.

`orientation` aligns a joining window to its group's view direction.
`windowing` and `lut` relay changes to every member but do not converge a
joining window, which keeps its own value until the group's next change.
`time` is reserved as a future key and rejected by v3.0.

## Lazy vs. strict mode

The top-level `groups` dict is optional:

- **Strict (recommended for tool output and REST GET responses).** `groups`
  is present. Every label appearing in any cell's `links` must be a key in
  `groups`; missing labels throw at load time. This catches typos and tool
  drift early.
- **Lazy (for hand-authoring).** `groups` is omitted. Every referenced label
  becomes an implicit group with property defaults (`select_all: true`).
  Cells still declare `links.selection` explicitly.

Per-cell `links` is **always** required — laziness applies only to the
group-properties block at the top, never to per-cell linking.

`QmitkMxNMultiWidget::SerializeLayout` always emits strict mode (full
`groups` dict, every property explicit), so a serialize → apply round trip
is stable and reviewer-friendly.

## A worked two-row example

```json
{
  "version": "3.0",
  "name": "Two rows; row 2 is its own selection group",
  "groups": {
    "main": { "select_all": true  },
    "row2": { "select_all": false }
  },
  "root": {
    "type": "split",
    "orientation": "vertical",
    "children": [
      {
        "type": "split",
        "orientation": "horizontal",
        "size": 1,
        "children": [
          { "type": "window", "id": "mxn__widget0", "name": "Row 1 - Axial",    "view_direction": "axial",    "links": { "selection": "main" }, "size": 1 },
          { "type": "window", "id": "mxn__widget1", "view_direction": "sagittal", "links": { "selection": "main" }, "size": 1 },
          { "type": "window", "id": "mxn__widget2", "view_direction": "coronal",  "links": { "selection": "main" }, "size": 1 }
        ]
      },
      {
        "type": "split",
        "orientation": "horizontal",
        "size": 1,
        "children": [
          { "type": "window", "id": "mxn__widget3", "view_direction": "axial",    "links": { "selection": "row2" }, "size": 1 },
          { "type": "window", "id": "mxn__widget4", "view_direction": "sagittal", "links": { "selection": "row2" }, "size": 1 },
          { "type": "window", "id": "mxn__widget5", "view_direction": "coronal",  "links": { "selection": "row2" }, "size": 1 }
        ]
      }
    ]
  }
}
```

The result is a 2x3 grid with equal weights everywhere. Row 1
(`mxn__widget0..mxn__widget2`) shares the selection bundle named `main`;
row 2 (`mxn__widget3..mxn__widget5`) shares its own bundle named `row2`.
Changing the selection in any row-1 cell propagates only to the other two
row-1 cells; row 2 is independent.

To make the top row twice as tall as the bottom row, change the outer
children's sizes to `2` and `1` (or omit one and leave the other at `2`,
since the omitted child defaults to `1`).

## Group seeding at load

When a layout is applied, each selection group's runtime synchronized state
for per-renderer node properties (per-node `visible` and `layer`) is seeded
from the cell that appears first in document order whose `links.selection`
names that group. After seeding, every other group member is normalised to
the seed cell's values for those keys. The navigation dimensions use the
same seed rule to decide which window stays put while the others converge
(see "Navigation and appearance links" above).

- **Document order** = pre-order traversal of the splitter tree (splits'
  `children` arrays in array order).
- **Group-scoped persisted properties** stored in the top-level `groups`
  dict (e.g. `select_all`) are not seeded from cells — the value declared
  there wins outright.
- **Selection-list membership** is a runtime concept not encoded in the
  layout. It is seeded from the data storage's non-helper, non-hidden
  nodes.

If a hand-author wants a specific cell to be the seed, they list that cell
first among the group's members in the layout document. In the worked
example above, `mxn__widget0` is the seed for `main` and `mxn__widget3` is
the seed for `row2`.

## Window identity and display label

Each window leaf carries a required identity (`id`) and an optional display
label (`name`). The two are deliberately separate fields so that renaming
the human-facing label never invalidates persisted references.

**`id` (identity).** The `id` is the **single canonical string** for a
window across every artifact in the system: the layout JSON, the REST URL,
the engine's render-window registration with the rendering manager, the
per-renderer DataNode property context key, and any persisted session-state
reference. The loader does not prepend or strip a prefix at any boundary;
what the document holds is what every other surface sees.

The qualified-id form is `<editor_name>__<bare_id>`:

- `<editor_name>` matches `^[A-Za-z][A-Za-z0-9.-]*$` (no `_`, so the
  first-`__` split is unambiguous).
- `__` is the literal namespace delimiter.
- `<bare_id>` matches `^[A-Za-z0-9_.-]+$` (URL-segment-safe; may itself
  contain further `__` substrings, since split is by *first* occurrence).

The combined regex is
`^[A-Za-z][A-Za-z0-9.-]*__[A-Za-z0-9_.-]+$`. Within a document, ids must
be unique.

The schema enforces structural shape only. The C++ loader additionally
enforces that `<editor_name>` matches the loading editor's `multiWidgetName`
(default `mxn`); a layout written for a different editor instance is
rejected up-front with a message naming the offending id. Editor-name
constructor inputs that contain `_` or otherwise violate the editor-name
regex are rejected at editor construction time, not at load time.

The recommended default for tool-generated layouts is `mxn__widget<i>`
where `<i>` is the leaf's pre-order traversal index (0-based, contiguous).
`SerializeLayout` writes this form. Hand-authored presets may use any
unique bare-id segment (e.g. `mxn__alpha`, `mxn__upper_left`) as long as
the qualified id is URL-segment-safe and unique within the document.

Renaming an id breaks every cached reference to it (persisted sessions,
REST clients holding URLs, scene-file context keys); treat it as
permanent.

**`name` (display label).** Optional, free-form, not required to be unique.
Holds whatever string a user-facing surface should show for the cell —
"Tumor axial", "Reference T1", "Comparison view 2". Pure metadata: the
loader does not use it for routing, addressing, persisted-state keying, or
REST URL construction. The schema rejects an empty `name`; tools should omit
the field entirely instead of emitting `""`. Tools that auto-generate
layouts (`SerializeLayout`, the migration script) leave `name` unset by
default; hand-authors and UIs that surface a "rename window" action populate
it.

The convention is symmetric across the document: the top-level optional
`name` is the display label of the *preset*, and a per-window optional
`name` is the display label of *that window*. Both are pure metadata; both
can be safely renamed at any time.

## Migrating from v1.x

Files written by older MITK releases use a different shape (top-level
`content` array, integer `synchGroup`, capitalised view directions, per-cell
`selectAll`). The current loader **does not** read v1.x documents directly;
attempting to load one surfaces an error message that points at a one-shot
conversion tool.

To upgrade a v1.x file:

```bash
python3 Modules/QtWidgets/resource/migrate-mxn-layout-v1-to-v2.py <file.json>
```

The script writes a v2.0 document to standard output (or to the path given
via `-o`) that the editor's "Load layout" action accepts unchanged. It is a
pure stdlib Python 3 script with no required external dependencies; if
`jsonschema` is importable in the running interpreter, the script also
validates its own output against the v2 schema before writing.

The migration is intentionally one-shot. There is no in-memory v1.x
translation in the C++ loader — the rationale is that custom MxN presets
are rare and an out-of-band script is cheaper to maintain than a permanent
in-process compatibility shim.

## Versions and closure

The schemas are closed: `additionalProperties: false` everywhere, including
inside `links`, link objects and `groups` entries. This is deliberate: a
loader that cannot honour a synchronization should not silently load the
file and drop part of the contract.

v2.0 to v3.0 is additive: more dimension keys inside `links`, the object
form for offsets, and the cosmetic `color` / `name` group properties. The
selection link and `select_all` keep their v2 shape, so tooling that learned
the v2 shape keeps working. The loader reads both versions in one code path;
there is no mandatory migration. A v2-only loader refuses a `"3.0"` document
at its version check, correctly, since it cannot honour the extra
dimensions.

The C++ loader runs no JSON-schema validator. It enforces the closure of
`links` at runtime, and only for `"3.0"` documents: an unknown link key, an
unknown modifier, or an offset on a dimension that does not take one (or of
the wrong type) throws. `"2.0"` documents keep their historical leniency, so
unknown link keys in them are ignored and existing v2 files are never newly
rejected. Unknown keys inside a group entry are tolerated in both versions.

## Where to look in the source

- Normative schema: `Modules/QtWidgets/resource/mxn-layout-v3.schema.json`
  (v2.0 documents: `Modules/QtWidgets/resource/mxn-layout-v2.schema.json`)
- Default in-tree preset: `Modules/QtWidgets/resource/mxnLayout_twoRowsEachDirection.json`
- Migration script: `Modules/QtWidgets/resource/migrate-mxn-layout-v1-to-v2.py`
- Loader / serializer: `QmitkMxNMultiWidget::ApplyLayout` and
  `QmitkMxNMultiWidget::SerializeLayout`
- Runtime synchronization: see @ref MxNConceptPage
