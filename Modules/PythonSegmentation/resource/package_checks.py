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

# Worker module for mitk::PythonPackage::PackageProbe, which answers questions
# about the packages of a virtual environment without importing them.


def has_module(name):
    """Whether a top-level module can be imported, without importing it."""
    import importlib.util

    try:
        return importlib.util.find_spec(name) is not None
    except Exception:
        return False


def has_distribution(name):
    """Whether a pip distribution is installed, by its metadata alone."""
    import importlib.metadata

    try:
        importlib.metadata.version(name)
        return True
    except Exception:
        return False


def check_version(distribution, minimum, specifier, check_for_update):
    """Compares the installed version of a distribution with a supported range.

    Reads the installed version with the standard library alone, so a missing
    'packaging' never hides an installed package. Reaches out to PyPI only once
    the installed version is known to be supported, so a too-old package is
    flagged at once and a missing network never delays the result. An empty
    'installed' in the result means the verdict is inconclusive.
    """
    import importlib.metadata

    result = {'installed': '', 'latest': '', 'below_minimum': False, 'update_available': False}

    try:
        installed = importlib.metadata.version(distribution)
    except Exception:
        return result

    try:
        from packaging.version import Version
        below_minimum = Version(installed) < Version(minimum)
    except Exception:
        # A missing 'packaging' or an unparsable version must not pass for up to
        # date, so the result stays inconclusive.
        return result

    result['installed'] = installed
    result['below_minimum'] = below_minimum

    if below_minimum or not check_for_update:
        return result

    try:
        import json
        import urllib.request
        from packaging.specifiers import SpecifierSet

        with urllib.request.urlopen(f'https://pypi.org/pypi/{distribution}/json', timeout=5) as response:
            data = json.load(response)

        supported = SpecifierSet(specifier)

        # PyPI keeps every version ever registered, including yanked or fileless
        # ones that pip would never install.
        candidates = [version for version, files in data['releases'].items()
                      if files and not all(file.get('yanked') for file in files)
                      and supported.contains(version, prereleases=False)]

        if candidates:
            latest = max(candidates, key=Version)
            result['latest'] = str(latest)
            result['update_available'] = Version(latest) > Version(installed)
    except Exception:
        pass

    return result
