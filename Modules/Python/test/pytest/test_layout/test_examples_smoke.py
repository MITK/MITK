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

"""Smoke test for the in-tree DSL examples.

Runs each example script end-to-end so that any regression in a covered
DSL surface fails CI. Lives inside the pytest suite executed by the C++
``mitkPythonBindingsTest`` driver; that driver embeds Python in a process
that has already loaded the MITK/ITK/VTK/Qt runtime DLLs, so ``import
mitk`` resolves cleanly on Windows without bespoke ``PATH`` plumbing.

Skipped in wheel-test CI runs where the source tree is not on disk.
"""

from __future__ import annotations

import runpy
from pathlib import Path

import pytest


def _find_repo_root() -> Path | None:
    here = Path(__file__).resolve()
    for ancestor in (here, *here.parents):
        if (ancestor / "Wrapping" / "Python" / "mitk" / "mxn" / "layout" / "examples").is_dir():
            return ancestor
    return None


def _examples_dir() -> Path | None:
    root = _find_repo_root()
    if root is None:
        return None
    return root / "Wrapping" / "Python" / "mitk" / "mxn" / "layout" / "examples"


@pytest.mark.parametrize("script_name", ["01_getting_started.py"])
def test_example_script_runs(script_name, capsys):
    examples = _examples_dir()
    if examples is None:
        pytest.skip("source tree not available (wheel-test environment)")
    script = examples / script_name
    if not script.is_file():
        pytest.skip(f"example script not found: {script}")

    runpy.run_path(str(script), run_name="__main__")

    captured = capsys.readouterr()
    assert "Done." in captured.out
