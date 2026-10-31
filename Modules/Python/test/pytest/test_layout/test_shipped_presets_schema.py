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

"""The layout documents in the tree validate against the v3 schema, and both
schemas refuse what the loader refuses about `links.time` and the reserved
group `global`.

The documents are read as raw JSON: they are 3.0 documents, and the DSL
targets 2.0. Nothing here imports MITK.
"""

from __future__ import annotations

import json

import pytest


def _load(path):
    with open(path, encoding="utf-8") as f:
        return json.load(f)


def _validator(schema_path):
    if schema_path is None:
        pytest.skip("schema not available; run from source tree")
    jsonschema = pytest.importorskip("jsonschema")
    return jsonschema.Draft202012Validator(_load(schema_path))


def _window_doc(version, **links):
    """A one-window document whose window links `selection` to "main" plus `links`."""
    return {
        "version": version,
        "root": {
            "type": "split",
            "orientation": "horizontal",
            "children": [
                {
                    "type": "window",
                    "id": "mxn__w0",
                    "view_direction": "axial",
                    "links": {"selection": "main", **links},
                }
            ],
        },
    }


def test_every_in_tree_v3_document_validates_against_v3_schema(repo_root, v3_schema_path):
    validator = _validator(v3_schema_path)

    documents = []
    for preset in sorted(v3_schema_path.parent.glob("mxnLayout_*.json")):
        documents.append((preset.name, _load(preset)))
    demo_dir = repo_root / "Examples" / "Plugins" / "org.mitk.example.gui.mxneditor" / "resources"
    for demo in sorted(demo_dir.glob("MxNDemoLayout*.json")):
        documents.append((demo.name, _load(demo)))
    for example in validator.schema["examples"]:
        documents.append((f"schema example '{example.get('name', '')}'", example))
    openapi = _load(repo_root / "Modules" / "RESTAPI" / "resource" / "openapi.json")
    layout_get = openapi["paths"]["/rendering/editors/mxn/layout"]["get"]
    documents.append(
        ("openapi.json layout example", layout_get["responses"]["200"]["content"]["application/json"]["example"])
    )

    # Eight presets, two demo layouts, three schema examples, the REST example.
    assert len(documents) == 14, [name for name, _ in documents]

    failures = []
    for name, document in documents:
        for error in validator.iter_errors(document):
            failures.append(f"{name}: {error.message} at {list(error.absolute_path)}")
    assert not failures, "\n".join(failures)


@pytest.mark.parametrize(
    "time",
    ["global", {"target": "global"}],
    ids=["string", "object"],
)
def test_v3_accepts_time_global(v3_schema_path, time):
    validator = _validator(v3_schema_path)
    assert validator.is_valid(_window_doc("3.0", time=time))


def test_v3_strict_mode_needs_no_groups_entry_for_global(v3_schema_path):
    validator = _validator(v3_schema_path)
    document = _window_doc("3.0", time="global")
    document["groups"] = {"main": {"select_all": True}}
    assert validator.is_valid(document)


_OTHER_LINK_KEYS = ["selection", "pan", "zoom", "slice", "crosshair", "orientation", "windowing", "lut"]


def _with_groups(document, groups):
    document["groups"] = groups
    return document


def _v3_refused():
    """(name, refused document, the same document repaired) triples."""
    repaired = _window_doc("3.0", time="global")
    cases = [("missing time", _window_doc("3.0"), repaired)]
    for target in ["main", {"target": "nav"}, "Global"]:
        cases.append((f"time targets {target!r}", _window_doc("3.0", time=target), repaired))
    cases.append(("time with an offset", _window_doc("3.0", time={"target": "global", "offset": 1}), repaired))
    for key in _OTHER_LINK_KEYS:
        for target in ["global", {"target": "global"}, "Global"]:
            cases.append(
                (
                    f"{key} targets {target!r}",
                    _window_doc("3.0", **{"time": "global", key: target}),
                    _window_doc("3.0", **{"time": "global", key: "main"}),
                )
            )
    for group in ["global", "GLOBAL"]:
        cases.append(
            (
                f"groups declares {group!r}",
                _with_groups(_window_doc("3.0", time="global"), {"main": {"select_all": True}, group: {}}),
                _with_groups(_window_doc("3.0", time="global"), {"main": {"select_all": True}}),
            )
        )
    return cases


@pytest.mark.parametrize("case", _v3_refused(), ids=lambda case: case[0])
def test_v3_refuses(v3_schema_path, case):
    validator = _validator(v3_schema_path)
    _, document, repaired = case
    assert validator.is_valid(repaired), "the repaired document must validate, or the refusal proves nothing"
    assert not validator.is_valid(document)


def _v2_refused():
    """(name, refused document, the same document repaired) triples."""
    cases = []
    for group in ["global", "GLOBAL"]:
        document = _window_doc("2.0")
        document["root"]["children"][0]["links"]["selection"] = group
        cases.append((f"selection targets {group!r}", document, _window_doc("2.0")))
    for group in ["global", "Global"]:
        cases.append(
            (
                f"groups declares {group!r}",
                _with_groups(_window_doc("2.0"), {"main": {"select_all": True}, group: {"select_all": True}}),
                _with_groups(_window_doc("2.0"), {"main": {"select_all": True}}),
            )
        )
    return cases


@pytest.mark.parametrize("case", _v2_refused(), ids=lambda case: case[0])
def test_v2_refuses(schema_path, case):
    validator = _validator(schema_path)
    _, document, repaired = case
    assert validator.is_valid(repaired), "the repaired document must validate, or the refusal proves nothing"
    assert not validator.is_valid(document)
