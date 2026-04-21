MITK JSON Scene Format {#MITKSceneJsonFormatPage}
======================

[TOC]

This page specifies the MITK JSON scene file format (`.mitkscene.json`). It is
an authoring-friendly, human-writable descriptor for MITK scene graphs that can
be consumed by mitk::SceneIO through mitk::SceneJsonReader.

The format is intentionally coherent with two other MITK JSON surfaces so that
external tools (Python scripts, REST clients) only need one mental model:

- The MITK REST API (property payloads, node descriptors).
- The @ref MITKSegmentationStackFormatPage "MITK MultiLabel Segmentation Stack format"
  (root `type` / `version`, underscore-prefixed meta keys, self-contained
  property JSON).

## Status and scope

- **Reader only.** Writing is not implemented; the existing XML + ZIP writer
  (mitk::SceneIO) remains the serialization format.
- **File extension:** `.mitkscene.json`.
- **MIME type name:** `application/vnd.mitk.scene.json`.
- **Category:** `MITK Scenes`.
- **Integration:** `mitk::SceneIO::LoadScene()` detects `.mitkscene.json` (and
  `index.json` inside a zipped `.mitk` archive) and routes it to
  mitk::SceneJsonReader. The JSON reader is **not** registered as a regular
  mitk::IFileReader, so `mitk::IOUtil::Load` does not load scenes; scenes go
  through `SceneIO` only, the same entry point as `.mitk` archives.

## Quick start

A minimal scene with one image:

```json
{
  "type": "org.mitk.scene",
  "version": 1,
  "nodes": [
    {
      "data": {"data_type": "Image", "_file": "patient.nrrd"},
      "properties": {"name": "Patient"}
    }
  ]
}
```

Load it:

```cpp
auto sceneIO = mitk::SceneIO::New();
sceneIO->LoadScene("/path/to/scene.mitkscene.json", storage);
```

Relative `_file` paths resolve against the directory of the scene file.

## Conventions

### Casing

Schema-defined field names use bare `snake_case` (`parent_uid`, `data_type`,
`context_properties`). This matches the REST API.

### Meta-key convention

Keys beginning with an underscore (`_`) are **meta / loader-steering**
information. Two distinct roles use the same prefix so authors can recognise
them at a glance:

- **Inside a property map** (a JSON object carrying user-defined property
  keys: node `properties`, an entry in `context_properties`, the `properties`
  of a data descriptor), `_`-prefixed keys never become MITK properties.
  They configure how the map is loaded (`_loadstyle`) or where the map comes
  from (`_file`). All other keys are property names whose values are
  self-contained JSON property values (see
  @ref MITKSceneJsonFormatPropertyValues below).
- **Inside structural objects** (the root, a node object, a data
  descriptor), the schema is fixed and `_`-prefixed keys mark those fields
  that steer the loader rather than describe a scene item. For example,
  `data._file` points the reader at an external binary to load, whereas
  `data.data_type` and `data.uid` are structural fields. Treating `_file`
  consistently across structural objects and property maps keeps the format
  uniform.

MITK property keys do not start with `_` in practice; the specification
reserves the underscore prefix for meta use in both roles.

## Top-level object

| Field      | Type   | Required | Description                                                                                                        |
|------------|--------|----------|--------------------------------------------------------------------------------------------------------------------|
| `type`     | string | yes      | Must be `"org.mitk.scene"`. Used for content-based detection.                                                      |
| `version`  | int    | yes      | Format version. Current: `1`. Unsupported versions produce an error.                                               |
| `metadata` | object | no       | Free-form metadata. Reserved key `description` (string). Additional keys are tolerated and ignored.                |
| `nodes`    | array  | yes      | Array of node objects. Order is not significant (two-pass topological resolution).                                  |

Unknown top-level keys produce a warning and are ignored (forward
compatibility across minor versions).

## Node object

| Field                | Type          | Required | Default        | Description                                                                              |
|----------------------|---------------|----------|----------------|------------------------------------------------------------------------------------------|
| `uid`                | string        | no       | auto-generated | Scene-local node UID. Referenced by other nodes' `parent_uid`.                           |
| `parent_uid`         | string / null | no       | `null`         | UID of the parent node, or `null` for a top-level node. Single parent.                   |
| `data`               | object        | no       | `null`         | Data payload descriptor (see below).                                                     |
| `properties`         | object        | no       | `null`         | Default-context property map (see @ref MITKSceneJsonFormatPropertyMaps).                 |
| `context_properties` | object        | no       | `{}`           | Map from renderer context name to property map.                                          |

### Single-parent rationale

The format permits **one** parent per node. mitk::DataStorage supports
multiple parents, but the MITK application (Data Manager, rendering) does not
use that capability. Exposing multiple parents in the authoring format would
invite scenes the application cannot render consistently. If multi-parent
scenes become a supported application use case later, a `parent_uids` array
can be introduced under a new format version.

## Data descriptor

The optional `data` object describes the BaseData loaded into the node.

| Field        | Type   | Required | Description                                                                                                                |
|--------------|--------|----------|----------------------------------------------------------------------------------------------------------------------------|
| `data_type`  | string | no       | Informative MITK class name, intended for authoring tools and human readers. Fully qualified (`"mitk::Image"`) is canonical; unqualified (`"Image"`) is accepted. The reader does **not** enforce that it matches the actually loaded BaseData class in v1; a future format version may tighten this. |
| `_file`      | string | yes*     | Path to the binary file, relative to the scene-file directory, or absolute. Underscored because it steers the loader.       |
| `uid`        | string | no       | Optional UID to assign to the loaded BaseData (applied via mitk::UIDManipulator).                                           |
| `properties` | object | no       | Optional data-level property map. Applied to the BaseData's own mitk::PropertyList.                                         |

*`_file` is currently the only supported data source. Alternative sources
(inline base64, URI, content-hash) are deliberately deferred; see the
implementation plan (section 9.4).

Because `data_type` is not enforced in v1, authors may omit it. Enforcing
consistency with the loaded BaseData would be a breaking change and is
therefore deferred to a future version bump.

## Property maps {#MITKSceneJsonFormatPropertyMaps}

A property map is a JSON object that lists properties to be applied to a
target (a node's default property list, a node's context-specific property
list, or a BaseData's property list).

Meta keys (v1):

| Key           | Type   | Default    | Description                                                                                                         |
|---------------|--------|------------|---------------------------------------------------------------------------------------------------------------------|
| `_loadstyle`  | string | `"modify"` | `"modify"` patches on top of mapper defaults. `"replace"` clears the list first (plain `PropertyList::Clear()`), then applies the listed keys. |
| `_file`       | string | -          | Load the property map from an external JSON file whose content is itself a property map (same schema, same meta convention). |

Rules:

- When `_file` is present in a property map, **no non-meta property keys may
  appear beside it**. Other `_`-meta keys (e.g. `_loadstyle`) are allowed and
  take precedence over the same meta key read from the external file.
- Unknown `_`-meta keys produce a warning and are ignored (forward compatible).

### Loadstyle semantics

For each property map (default or per-context):

- **`modify` (default):**
  1. The node is added to mitk::DataStorage and the mapper initializes
     defaults.
  2. Only explicitly listed properties are set / overwritten.
  3. All other properties remain as initialized by the mapper.

- **`replace`:**
  1. The target property list is cleared with a plain `PropertyList::Clear()`.
  2. All properties from the map are applied.

The `replace` behavior intentionally differs from the legacy XML reader,
which preserved a small set of mapper-assigned defaults (e.g. `LookupTable`,
`Image.Displayed Component`) when clearing. That exception list exists in
mitk::SceneReaderV1 only as a backwards-compat workaround for pre-fix XML
scene files, and does not apply to freshly authored JSON scenes. If a JSON
author wants those properties, they simply list them.

Loadstyle is **per property map**. A node may use `modify` in its default
context and `replace` in a renderer context (or vice versa). Data-level
properties honor `_loadstyle` analogously on the BaseData's property list.

### Property values {#MITKSceneJsonFormatPropertyValues}

Property values use the same self-contained JSON encoding as the REST API and
the Segmentation Stack format, produced and consumed by
mitk::ConvertPropertyToSelfContainedJson /
mitk::ConvertPropertyFromSelfContainedJson.

Two forms are accepted:

1. **Simple primitive -> inferred type.**
   - `"text"` -> mitk::StringProperty
   - `0.5` -> mitk::FloatProperty
   - `42` -> mitk::IntProperty
   - `true` -> mitk::BoolProperty
2. **Explicit tagged object.**
   ```json
   {"type": "ColorProperty", "value": [1.0, 0.0, 0.0]}
   ```
   `type` is the MITK property class name; `value` is its self-contained
   representation.

Use the explicit form whenever the inferred type would be wrong (e.g. to
disambiguate between mitk::IntProperty and mitk::FloatProperty for an integer
literal, or for composite property types).

### Context property maps

`context_properties` is a JSON object keyed by renderer context name (for
example `"stdmulti.widget0"`, `"stdmulti.widget1"`). Each value is a property
map with the same schema, including `_loadstyle` and `_file`. Using a map
(rather than an array of tagged entries) prevents duplicate context entries
structurally.

## Full example

```json
{
  "type": "org.mitk.scene",
  "version": 1,

  "metadata": {
    "description": "Example scene with a CT image and a segmentation"
  },

  "nodes": [
    {
      "uid": "node-ct",
      "parent_uid": null,

      "data": {
        "data_type": "mitk::Image",
        "_file": "brain.nrrd",
        "uid": "data-ct",
        "properties": {
          "modality": "CT"
        }
      },

      "properties": {
        "name": "CT scan",
        "visible": true,
        "opacity": 0.8,
        "color": {"type": "ColorProperty", "value": [1.0, 0.0, 0.0]}
      },

      "context_properties": {
        "stdmulti.widget0": {
          "_loadstyle": "modify",
          "opacity": 0.5
        },
        "stdmulti.widget1": {
          "_loadstyle": "replace",
          "_file": "widget1-props.json"
        }
      }
    },

    {
      "uid": "node-seg",
      "parent_uid": "node-ct",
      "data": {
        "data_type": "LabelSetImage",
        "_file": "brain-seg.nrrd"
      },
      "properties": {
        "name": "Segmentation"
      }
    }
  ]
}
```

## Errors and warnings

### Hard errors (scene load fails with mitk::Exception)

- Malformed JSON.
- Missing or wrong `type`; unsupported `version`.
- Missing `nodes` array.
- Duplicate `uid` in `nodes` - the message identifies the offending entries.
- `parent_uid` referring to a UID not present in `nodes` - the message
  identifies the dangling reference.
- Circular `parent_uid` chain - the message lists the cycle.
- `data._file` refers to a missing or unreadable file.
- `_file` in a property map refers to a missing / unreadable file, or the
  file's content is not a valid property map.
- Inline property keys present in a property map that also specifies `_file`.
- `_loadstyle` value other than `"modify"` or `"replace"`.

### Soft conditions (warnings, loading proceeds)

- Unknown `_`-meta keys in property maps.
- Unknown top-level keys in the root object or node object.
- Unqualified `data_type` is accepted silently (intentional ergonomic
  feature, not a warning).
- `metadata` keys other than `description`.

## Relationship to other MITK JSON formats

The property-map shape is **identical** to the body of a REST
`PATCH /api/v1/datastorage/nodes/{uid}/properties` request, except for the
optional `_loadstyle` meta key. A REST `PATCH` corresponds to
`"_loadstyle": "modify"` and a REST `PUT` corresponds to
`"_loadstyle": "replace"`, which means the same JSON can be used on both
surfaces with only the meta key as a boundary concern.

The root `type` / `version` shape, the underscore meta convention, and the
self-contained property JSON encoding are shared with the
@ref MITKSegmentationStackFormatPage "MultiLabel Segmentation Stack format".
