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
"""Validate MxN v2.0 layout fixtures against `mxn-layout-v2.schema.json`.

Used as the `mitkMxNLayoutSchemaLint` CTest, gated on `MITK_USE_Python3`.
The CTest fixture `mitkMxNLayoutSchemaLintSetup` provisions a dedicated
venv with `jsonschema` installed, so the script can assume the
dependency is present when invoked from CTest. Catches drift between the
schema and the in-tree preset / test fixtures before it lands in a
release.

Usage:
    validate_layout_fixtures.py SCHEMA FIXTURE [FIXTURE ...]

Exit codes:
    0 - schema and all fixtures validated.
    1 - one or more fixtures failed validation, or `jsonschema` is not
        importable in the running interpreter.
"""

from __future__ import annotations

import json
import sys
from pathlib import Path


def main(argv: list) -> int:
    if len(argv) < 2:
        print(
            "usage: validate_layout_fixtures.py SCHEMA FIXTURE [FIXTURE ...]",
            file=sys.stderr,
        )
        return 1

    schema_path = Path(argv[0])
    fixture_paths = [Path(p) for p in argv[1:]]

    try:
        import jsonschema  # type: ignore
    except ImportError:
        print(
            "validate_layout_fixtures: 'jsonschema' is not installed in this "
            "Python environment. Install with "
            "`{0} -m pip install jsonschema` and re-run.".format(sys.executable),
            file=sys.stderr,
        )
        return 1

    try:
        schema = json.loads(schema_path.read_text(encoding="utf-8"))
    except OSError as e:
        print(
            f"validate_layout_fixtures: could not read schema {schema_path}: {e}",
            file=sys.stderr,
        )
        return 1
    except json.JSONDecodeError as e:
        print(
            f"validate_layout_fixtures: schema {schema_path} is not valid JSON: {e}",
            file=sys.stderr,
        )
        return 1

    validator = jsonschema.Draft202012Validator(schema)

    failed = 0
    for fixture in fixture_paths:
        try:
            doc = json.loads(fixture.read_text(encoding="utf-8"))
        except OSError as e:
            print(f"{fixture}: could not read: {e}", file=sys.stderr)
            failed += 1
            continue
        except json.JSONDecodeError as e:
            print(f"{fixture}: not valid JSON: {e}", file=sys.stderr)
            failed += 1
            continue

        errors = sorted(validator.iter_errors(doc), key=lambda e: list(e.absolute_path))
        if errors:
            failed += 1
            print(f"{fixture}: schema validation failed", file=sys.stderr)
            for err in errors:
                location = "/".join(str(p) for p in err.absolute_path) or "<root>"
                print(f"  - {err.message} at /{location}", file=sys.stderr)

    if failed:
        print(
            f"validate_layout_fixtures: {failed} fixture(s) failed validation.",
            file=sys.stderr,
        )
        return 1

    print(
        f"validate_layout_fixtures: validated {len(fixture_paths)} fixture(s) "
        f"against {schema_path.name}.",
    )
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
