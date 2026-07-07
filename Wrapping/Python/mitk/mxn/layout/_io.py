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

"""File I/O helpers for layout JSON documents.

The module intentionally stays thin: the heavy lifting lives in
``MxNLayoutDocument.from_json`` / ``MxNLayoutDocument.to_json``.
"""

from __future__ import annotations

import json
from collections.abc import Mapping
from pathlib import Path
from typing import Any

from ._model import MxNLayoutDocument


def load_preset(path: str | Path) -> MxNLayoutDocument:
    """Read a JSON file from disk and parse it via
    :meth:`MxNLayoutDocument.from_json`.

    Up-front version mismatch raises a clear ``ValueError`` before any
    structural parsing -- see ``MxNLayoutDocument.from_json``.
    """
    text = Path(path).read_text(encoding="utf-8")
    return MxNLayoutDocument.from_json(text)


def save_preset(
    path: str | Path,
    doc: MxNLayoutDocument | Mapping[str, Any],
) -> None:
    """Write ``doc`` as JSON to disk.

    Accepts either a typed :class:`MxNLayoutDocument` (preferred) or a
    raw mapping previously produced by :meth:`MxNLayoutDocument.to_json`.
    For a typed document, calls :meth:`MxNLayoutDocument.to_json` first.
    Output uses ``indent=2`` and explicit (schema-aligned) key ordering;
    ``sort_keys`` is **not** enabled because we want ``version, name?,
    groups, root`` rather than alphabetic order.
    """
    payload: Mapping[str, Any]
    if isinstance(doc, MxNLayoutDocument):
        payload = doc.to_json()
    elif isinstance(doc, Mapping):
        payload = doc
    else:
        raise ValueError(
            f"doc must be MxNLayoutDocument or Mapping, got {type(doc).__name__}"
        )
    Path(path).write_text(
        json.dumps(payload, indent=2, sort_keys=False, ensure_ascii=False) + "\n",
        encoding="utf-8",
    )
