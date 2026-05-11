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
"""Provision the venv that `mitkMxNLayoutSchemaLint` runs in.

Creates a dedicated virtualenv under the build tree and makes
`jsonschema` importable from it. Running into a dedicated venv keeps
the host Python untouched and lets the lint test treat a missing
dependency as a hard failure rather than a silent skip on CI.

The venv is created with ``with_pip=False`` so that Python's
``ensurepip`` step is not invoked inside the venv interpreter. On
macOS Jenkins agents the bundled ``ensurepip`` has been observed to
abort with SIGABRT (likely related to framework-Python re-exec or
code-signing of the freshly created venv interpreter), which is
unrelated to anything we need. Instead we use the host Python's pip
with ``--target`` to install ``jsonschema`` directly into the venv's
``site-packages``; the venv interpreter then picks it up at import
time without ever needing its own pip.

Wired up as a CTest fixture (FIXTURES_SETUP) that
`mitkMxNLayoutSchemaLint` requires; CTest runs this once per `ctest`
invocation and reuses the venv on subsequent runs.

Usage:
    setup_schema_lint_env.py VENV_DIR

Exit codes:
    0 - venv is ready and `jsonschema` importable.
    1 - venv creation or dependency install failed.
"""

from __future__ import annotations

import subprocess
import sys
import venv
from pathlib import Path


def _venv_python(venv_dir: Path) -> Path:
    if sys.platform == "win32":
        return venv_dir / "Scripts" / "python.exe"
    return venv_dir / "bin" / "python"


def _venv_site_packages(python: Path) -> Path:
    """Return the venv interpreter's purelib directory.

    Asking the interpreter itself avoids hard-coding the
    Lib/site-packages (Windows) vs lib/pythonX.Y/site-packages (POSIX)
    layout difference and is robust against future Python releases.
    """
    result = subprocess.run(
        [str(python), "-c", "import sysconfig;print(sysconfig.get_paths()['purelib'])"],
        capture_output=True,
        text=True,
        check=True,
    )
    return Path(result.stdout.strip())


def main(argv: list) -> int:
    if len(argv) != 1:
        print("usage: setup_schema_lint_env.py VENV_DIR", file=sys.stderr)
        return 1

    venv_dir = Path(argv[0])
    python = _venv_python(venv_dir)

    if not python.is_file():
        print(f"setup_schema_lint_env: creating venv at {venv_dir}")
        try:
            venv.EnvBuilder(with_pip=False, clear=False).create(str(venv_dir))
        except Exception as exc:
            print(f"setup_schema_lint_env: venv creation failed: {exc}", file=sys.stderr)
            return 1

    if not python.is_file():
        print(
            f"setup_schema_lint_env: expected interpreter not found at {python}",
            file=sys.stderr,
        )
        return 1

    # Fast path: dependency already installed - skip the pip round-trip.
    probe = subprocess.run([str(python), "-c", "import jsonschema"], capture_output=True)
    if probe.returncode == 0:
        print("setup_schema_lint_env: jsonschema already present.")
        return 0

    try:
        site_packages = _venv_site_packages(python)
    except subprocess.CalledProcessError as exc:
        print(
            f"setup_schema_lint_env: could not determine venv site-packages: {exc}",
            file=sys.stderr,
        )
        return 1

    print(f"setup_schema_lint_env: installing jsonschema into {site_packages}")
    install = subprocess.run(
        [
            sys.executable,
            "-m",
            "pip",
            "install",
            "--disable-pip-version-check",
            "--target",
            str(site_packages),
            "jsonschema",
        ],
    )
    if install.returncode != 0:
        print("setup_schema_lint_env: pip install jsonschema failed.", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
