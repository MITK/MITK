#!/usr/bin/env python3
# ============================================================================
#
# The Medical Imaging Interaction Toolkit (MITK)
#
# Copyright (c) German Cancer Research Center (DKFZ)
# All rights reserved.
#
# Use of this source code is governed by a 3-clause BSD license that can be
# found in the LICENSE file.
#
# ============================================================================
"""Migrate a v1.x MITK MxN layout document to the v2.0 schema.

The v2.0 format is described by `mxn-layout-v2.schema.json` (next to this
script). This tool consumes a v1.x layout document and emits a v2.0 document
that the QmitkMxNMultiWidget editor accepts via `ApplyLayout`.

Field mapping highlights:
- `size` is optional in v2 (default weight 1). v1 nodes that omit `size` are
  emitted as v2 nodes that also omit it; the loader and any schema-aware
  consumer fill in the default.
- `synchGroup` (the v1 typo) and `syncGroup` are both accepted; the integer
  is mapped to the v2 group label `'main'` for index 1 and `'g_<N>'`
  otherwise. Per-cell `selectAll` is folded into the group's `select_all`
  property (first-encounter wins).
- Window ids are emitted in the v2 canonical fully-qualified form
  `<editor_name>__widget<i>` (default editor name `mxn`, override with
  `--editor-name`). v1 layouts had no stable per-cell identity worth
  preserving, so `<i>` is the leaf's pre-order traversal index.

Usage:
    migrate-mxn-layout-v1-to-v2.py INPUT [-o OUTPUT] [--editor-name NAME]
                                         [--schema SCHEMA]

If `--schema` is omitted, the v2 schema is loaded from a file called
`mxn-layout-v2.schema.json` next to this script. When `jsonschema` is
importable, the produced document is validated against the schema before
being written; if it does not validate, the script exits non-zero so the user
never sees an invalid v2 document from this tool. When `jsonschema` is not
installed, validation is skipped with a warning on stderr.
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path
from typing import Any


# Default editor name produced by `QmitkMxNMultiWidget`. Used as the
# `<editor_name>` segment of qualified ids (`<editor_name>__widget<i>`)
# unless overridden via `--editor-name`.
DEFAULT_EDITOR_NAME = "mxn"

# Namespace delimiter between editor name and bare id segment in the
# canonical qualified-window-id form. Mirrors `kNamespaceDelimiter` in the
# C++ engine.
NAMESPACE_DELIMITER = "__"

# Editor-name shape constraint. Mirrors the C++ engine's `kEditorNamePattern`
# and the `<editor_name>` segment of the schema's id pattern: starts with a
# letter, no `_` (which would break the first-`__` split rule), only
# `[A-Za-z0-9.-]` afterwards.
_EDITOR_NAME_PATTERN = re.compile(r"^[A-Za-z][A-Za-z0-9.-]*$")


def _qualified_id(bare_id: str, editor_name: str = DEFAULT_EDITOR_NAME) -> str:
    """Build the canonical qualified id `<editor_name>__<bare_id>`."""
    return f"{editor_name}{NAMESPACE_DELIMITER}{bare_id}"


def _group_label(group_int: int) -> str:
    """Convention: integer 1 -> 'main', integer N -> 'g_<N>'."""
    return "main" if group_int == 1 else f"g_{group_int}"


def _read_group_int(node: dict, *, where: str) -> int:
    """Read either `synchGroup` (canonical) or `syncGroup` (typo).

    The original v1 preset shipped in MITK contained the typo
    `synchGroup`; some hand-authored files use the corrected `syncGroup`.
    Tolerate both.
    """
    if "synchGroup" in node and "syncGroup" in node:
        if node["synchGroup"] != node["syncGroup"]:
            print(
                f"warning: window at {where} has both 'synchGroup' and 'syncGroup' "
                f"with different values ({node['synchGroup']} vs {node['syncGroup']}); "
                f"taking 'synchGroup'.",
                file=sys.stderr,
            )
        return int(node["synchGroup"])
    if "synchGroup" in node:
        return int(node["synchGroup"])
    if "syncGroup" in node:
        return int(node["syncGroup"])
    print(
        f"warning: window at {where} has neither 'synchGroup' nor 'syncGroup'; "
        f"defaulting to group 1 ('main').",
        file=sys.stderr,
    )
    return 1


def _convert_node(
    node: dict,
    *,
    path: str,
    widget_counter: list,
    group_select_all: dict,
    editor_name: str,
) -> dict:
    """Recursively convert a v1 node to a v2 node.

    `widget_counter` is a single-element list used as a mutable integer counter
    (pre-order index across the whole tree); `group_select_all` records the
    `selectAll` state for each group encountered (first-encounter wins).
    `editor_name` becomes the `<editor_name>` segment of every emitted
    qualified id.
    """
    if not isinstance(node, dict):
        raise ValueError(f"v1 node at {path} is not an object: {node!r}")

    is_window = bool(node.get("isWindow", False))

    if is_window:
        idx = widget_counter[0]
        widget_counter[0] += 1

        view_direction = str(node.get("viewDirection", "")).strip().lower()
        if not view_direction:
            raise ValueError(
                f"v1 window at {path} is missing 'viewDirection' (or it is empty)."
            )

        group_int = _read_group_int(node, where=path)
        group_name = _group_label(group_int)

        select_all = bool(node.get("selectAll", True))
        if group_name in group_select_all:
            existing = group_select_all[group_name]
            if existing != select_all:
                print(
                    f"warning: window at {path} declares selectAll={select_all} for "
                    f"group {group_name!r}, but an earlier window in the same group "
                    f"declared selectAll={existing}; first-encounter value wins.",
                    file=sys.stderr,
                )
        else:
            group_select_all[group_name] = select_all

        out: dict = {
            "type": "window",
            "id": _qualified_id(f"widget{idx}", editor_name),
            "view_direction": view_direction,
            "links": {"selection": group_name},
        }
        # The optional v2 `name` (free-form display label) has no v1 source;
        # we deliberately do not synthesize one. Hand-author after migration.
        # `size` is optional in v2; the loader defaults to 1 when omitted.
        # Only the ratio between siblings matters at runtime, so a v1 source
        # that omitted `size` becomes a v2 output that also omits it.
        if "size" in node:
            out["size"] = int(node["size"])
        return out

    # Interior split node.
    children_raw = node.get("content", [])
    if not isinstance(children_raw, list) or not children_raw:
        raise ValueError(
            f"v1 split at {path} is missing a non-empty 'content' array."
        )

    vertical = bool(node.get("vertical", False))
    out_split: dict = {
        "type": "split",
        "orientation": "vertical" if vertical else "horizontal",
        "children": [
            _convert_node(
                child,
                path=f"{path}/content[{i}]",
                widget_counter=widget_counter,
                group_select_all=group_select_all,
                editor_name=editor_name,
            )
            for i, child in enumerate(children_raw)
        ],
    }
    # `size` is optional in v2 (default weight 1). Pass through if the v1
    # source had one; otherwise let the v2 loader's default apply.
    if "size" in node:
        out_split["size"] = int(node["size"])
    return out_split


def migrate(v1_doc: dict, *, editor_name: str = DEFAULT_EDITOR_NAME) -> dict:
    """Convert a parsed v1.x layout document to a v2.0 document.

    `editor_name` becomes the `<editor_name>` segment of every emitted
    qualified id and must match the loading editor's `multiWidgetName`
    (default `mxn`).
    """
    if not isinstance(v1_doc, dict):
        raise ValueError("v1 document must be a JSON object at the top level.")

    if not _EDITOR_NAME_PATTERN.match(editor_name):
        raise ValueError(
            f"editor name {editor_name!r} does not match {_EDITOR_NAME_PATTERN.pattern!r}: "
            f"editor names must start with a letter, contain no '_', and use only "
            f"the alphabet [A-Za-z0-9.-]."
        )

    version = str(v1_doc.get("version", ""))
    if not version.startswith("1."):
        raise ValueError(
            f"input does not look like a v1.x layout file "
            f"(top-level 'version' is {version!r}, expected '1.x')."
        )

    widget_counter = [0]
    group_select_all: dict = {}
    root = _convert_node(
        v1_doc,
        path="$",
        widget_counter=widget_counter,
        group_select_all=group_select_all,
        editor_name=editor_name,
    )
    # Strip a 'size' from the root - the v2 schema's root has no parent.
    root.pop("size", None)

    out: dict = {"version": "2.0"}
    if "name" in v1_doc:
        out["name"] = v1_doc["name"]
    out["groups"] = {
        name: {"select_all": sel} for name, sel in group_select_all.items()
    }
    out["root"] = root
    return out


def _validate_against_schema(doc: dict, schema_path: Path) -> None:
    """Validate `doc` against the v2 schema if `jsonschema` is available.

    On ImportError: warn and continue. On unreadable schema file: print and
    exit 3 (a missing schema next to the script is a packaging defect, not a
    user error). On ValidationError: print and exit 3.
    """
    try:
        import jsonschema  # type: ignore
    except ImportError:
        print(
            "warning: jsonschema is not installed; output not validated against "
            f"{schema_path}. Install with `pip install jsonschema` to enable "
            "this safety check.",
            file=sys.stderr,
        )
        return

    try:
        schema = json.loads(schema_path.read_text(encoding="utf-8"))
    except OSError as e:
        print(f"error: could not read schema {schema_path}: {e}", file=sys.stderr)
        sys.exit(3)

    validator = jsonschema.Draft202012Validator(schema)
    errors = sorted(validator.iter_errors(doc), key=lambda e: list(e.absolute_path))
    if errors:
        print(
            f"error: migration produced output that does not validate against "
            f"{schema_path}:",
            file=sys.stderr,
        )
        for err in errors:
            location = "/".join(str(p) for p in err.absolute_path) or "<root>"
            print(f"  {err.message} at /{location}", file=sys.stderr)
        sys.exit(3)


def main(argv: list) -> int:
    parser = argparse.ArgumentParser(
        description="Migrate a v1.x MITK MxN layout document to v2.0.",
    )
    parser.add_argument(
        "input",
        type=Path,
        help="Path to the v1.x JSON layout document to convert.",
    )
    parser.add_argument(
        "-o",
        "--output",
        type=Path,
        default=None,
        help="Output path for the v2.0 document; stdout if omitted.",
    )
    parser.add_argument(
        "--editor-name",
        default=DEFAULT_EDITOR_NAME,
        help=(
            "MxN editor name to embed in the qualified id of every window "
            f"(default {DEFAULT_EDITOR_NAME!r}). Must match the loading editor's "
            "`multiWidgetName`."
        ),
    )
    parser.add_argument(
        "--schema",
        type=Path,
        default=Path(__file__).resolve().parent / "mxn-layout-v2.schema.json",
        help=(
            "Path to mxn-layout-v2.schema.json. Defaults to a file with that "
            "name next to this script. Used to validate the produced document "
            "when 'jsonschema' is installed."
        ),
    )
    args = parser.parse_args(argv)

    try:
        v1_doc = json.loads(args.input.read_text(encoding="utf-8"))
    except OSError as e:
        print(f"error: could not read {args.input}: {e}", file=sys.stderr)
        return 1
    except json.JSONDecodeError as e:
        print(f"error: {args.input} is not valid JSON: {e}", file=sys.stderr)
        return 1

    try:
        v2_doc = migrate(v1_doc, editor_name=args.editor_name)
    except ValueError as e:
        print(f"error: {e}", file=sys.stderr)
        return 2

    _validate_against_schema(v2_doc, args.schema)

    serialized = json.dumps(v2_doc, indent=2) + "\n"
    if args.output is None:
        sys.stdout.write(serialized)
    else:
        try:
            args.output.write_text(serialized, encoding="utf-8")
        except OSError as e:
            print(f"error: could not write {args.output}: {e}", file=sys.stderr)
            return 1

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
