MxN Layout File Format (v2.0) {#MxNLayoutFormatPage}
=============================

[TOC]

This page is the human-friendly walkthrough of the JSON layout file format used
by the MxN multi-widget editor. The on-disk preset files
(`Modules/QtWidgets/resource/mxnLayout_*.json`), the documents accepted by
`QmitkMxNMultiWidget::ApplyLayout`, and the documents emitted by
`QmitkMxNMultiWidget::SerializeLayout` all share this single format.

The closed, normative reference is the JSON Schema
`Modules/QtWidgets/resource/mxn-layout-v2.schema.json` (Draft 2020-12). When
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
- Per-window **placement state** (window name, view direction, links into
  synchronization groups).
- Per-group **persisted state** (today: the `select_all` UX-mode of the
  selection bundle).

A layout document does **not** capture global rendering state (selected
position, current time step, camera) or the loaded data set. Those are owned
by other parts of the system. Loading a layout into a session that already has
data nodes simply applies the new geometry; the data stays.

## Top-level shape

```json
{
  "version": "2.0",
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
        "name": "widget0",
        "view_direction": "axial",
        "links": { "selection": "main" },
        "size": 1
      }
    ]
  }
}
```

- `version`: must be the exact string `"2.0"`. The C++ loader rejects any
  other value. v1.x files are upgraded out-of-band — see "Migrating from
  v1.x" below.
- `name`: optional human-readable preset name. Pure metadata; ignored by the
  loader and not used for routing.
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
  "name": "widget0",
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

- `name`: a unique identifier within the document. Used as the engine-side
  bare name; the editor qualifies it at load time as `<editorName>.<name>`
  when registering with the rendering manager.
- `view_direction`: which anatomical plane the window shows. The closed enum
  is `"axial"`, `"sagittal"`, `"coronal"`, `"original"` (lowercase). Unknown
  strings throw at load time; there is no silent fallback.
- `links`: per-dimension synchronization references. v2.0 has one dimension,
  `selection`; v3.0 will add more dimensions here additively (zoom, time,
  crosshair, ...). Every window must declare `links.selection` explicitly --
  there are no implicit singletons.

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

Per-group persisted state lives once at the top level:

```json
"groups": {
  "main": { "select_all": true },
  "row2": { "select_all": false }
}
```

v2.0 declares one such property: `select_all` (the selection bundle's UX
mode — whether the group displays every data node or a curated subset).
v3.0 dimensions that need persisted per-group state add their properties to
the same group entry additively.

There is no per-cell `select_all`. The setting belongs to the group, not to
any one of its members.

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
  "version": "2.0",
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
          { "type": "window", "name": "widget0", "view_direction": "axial",    "links": { "selection": "main" }, "size": 1 },
          { "type": "window", "name": "widget1", "view_direction": "sagittal", "links": { "selection": "main" }, "size": 1 },
          { "type": "window", "name": "widget2", "view_direction": "coronal",  "links": { "selection": "main" }, "size": 1 }
        ]
      },
      {
        "type": "split",
        "orientation": "horizontal",
        "size": 1,
        "children": [
          { "type": "window", "name": "widget3", "view_direction": "axial",    "links": { "selection": "row2" }, "size": 1 },
          { "type": "window", "name": "widget4", "view_direction": "sagittal", "links": { "selection": "row2" }, "size": 1 },
          { "type": "window", "name": "widget5", "view_direction": "coronal",  "links": { "selection": "row2" }, "size": 1 }
        ]
      }
    ]
  }
}
```

The result is a 2x3 grid with equal weights everywhere. Row 1
(`widget0..widget2`) shares the selection bundle named `main`; row 2
(`widget3..widget5`) shares its own bundle named `row2`. Changing the
selection in any row-1 cell propagates only to the other two row-1 cells;
row 2 is independent.

To make the top row twice as tall as the bottom row, change the outer
children's sizes to `2` and `1` (or omit one and leave the other at `2`,
since the omitted child defaults to `1`).

## Group seeding at load

When a layout is applied, each group's runtime synchronized state for
per-renderer node properties (today: per-node `visible`, per-node `layer`;
future v3 keys analogously) is seeded from the cell that appears first in
document order whose `links.<dim>` names that group. After seeding, every
other group member is normalised to the seed cell's values for those keys.

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
example above, `widget0` is the seed for `main` and `widget3` is the seed
for `row2`.

## Window names

Within a document, window names must be unique. The schema enforces
URL-segment safety (alphanumeric, underscore, dot, hyphen) so a name can
drop into a future REST URL without escaping.

The recommended default for tool-generated layouts is `widget<i>` where
`<i>` is the leaf's pre-order traversal index (0-based, contiguous).
`SerializeLayout` writes this form. Hand-authored presets may use any
unique URL-segment-safe string (e.g. `"alpha"`, `"upper_left"`).

The loader registers each window under the qualified name
`<editorName>.<bareName>`. For the default editor the prefix is `mxn.`, so a
JSON `"name": "widget0"` lands as `mxn.widget0` in the rendering manager.
On serialize the prefix is stripped again.

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

## Closed schema and forward compatibility

The schema is closed: `additionalProperties: false` everywhere, including
inside `links` and inside `groups` entries. This is deliberate. v2.0 only
knows the `selection` synchronization dimension and the `select_all` group
property. A future v3.0 schema bump will add further dimensions inside
`links` (e.g. `zoom`, `time`, `crosshair`) and possibly further per-group
properties inside group entries. Older loaders presented with a v3.0
document refuse it loudly at the version check — by design. A loader that
cannot honour the synchronization should not silently load the file and
drop part of the contract.

The mental-model break between v2 and v3 is "more dimension keys inside
`links`, more group properties inside group entries" — never "selection got
reshaped". Tooling that learns the v2 link/groups shape continues to work in
v3 without modification.

## Where to look in the source

- Normative schema: `Modules/QtWidgets/resource/mxn-layout-v2.schema.json`
- Default in-tree preset: `Modules/QtWidgets/resource/mxnLayout_twoRowsEachDirection.json`
- Migration script: `Modules/QtWidgets/resource/migrate-mxn-layout-v1-to-v2.py`
- Loader / serializer: `QmitkMxNMultiWidget::ApplyLayout` and
  `QmitkMxNMultiWidget::SerializeLayout`
- Runtime synchronization: see @ref MxNConceptPage
