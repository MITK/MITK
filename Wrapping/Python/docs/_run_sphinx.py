"""Sphinx launcher used by the mitk_python_docs CMake target.

The compiled ``mitk`` extension links against MitkCore, ITK, VTK, Python,
Qt and external-project libraries that live elsewhere in the build tree.
On Windows, Python 3.8+ no longer consults ``PATH`` when resolving DLL
dependencies of native extension modules, so the only reliable way to
point the loader at those directories is :py:func:`os.add_dll_directory`.

This script accepts ``--dll-dir <path>`` flags (repeatable), registers
each existing directory via ``os.add_dll_directory`` on Windows, then
hands the remaining argv to ``sphinx.cmd.build.main`` in-process.

The dev-time ``mitk/__init__.py`` writes a similar set of
``os.add_dll_directory`` calls from the ``MITK_DLL_DIRECTORIES`` env var,
but Sphinx imports happen inside autodoc which runs in this same Python
process, so doing the setup here keeps the responsibility local to the
docs build.
"""

from __future__ import annotations

import argparse
import os
import sys


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__, allow_abbrev=False, add_help=False)
    parser.add_argument(
        "--dll-dir", action="append", default=[],
        help="Directory to register via os.add_dll_directory on Windows. "
             "Repeatable. Ignored on other platforms and for paths that "
             "do not exist.")
    args, sphinx_argv = parser.parse_known_args()

    if os.name == "nt":
        for path in args.dll_dir:
            if path and os.path.isdir(path):
                try:
                    os.add_dll_directory(path)
                except (FileNotFoundError, OSError):
                    pass

    from sphinx.cmd.build import main as sphinx_main
    return sphinx_main(sphinx_argv)


if __name__ == "__main__":
    sys.exit(main())
