#!/usr/bin/env python3

import argparse
import json
import re
import subprocess
import sys
import urllib.request

parser = argparse.ArgumentParser(description="Find latest NumPy wheel for specific macOS version")
parser.add_argument("--arch", help="CPU arch: x86_64 or arm64", default="arm64")
parser.add_argument("--osx-target", help="CMAKE_OSX_DEPLOYMENT_TARGET, e.g. 12.0", default="12.0")
args = parser.parse_args()

# Parse macOS version from wheel filename
def parse_osx_version(tag):
    m = re.search(r"macosx_(\d+)_(\d+)_", tag)
    if not m:
        return None
    return (int(m.group(1)), int(m.group(2)))

# Convert version string to tuple of integers
def version_tuple(v):
    parts = []
    for x in v.split("."):
        m = re.match(r"(\d+)", x)
        if m:
            parts.append(int(m.group(1)))
        else:
            parts.append(0)
    return tuple(parts)

target_tuple = tuple(int(x) for x in args.osx_target.split("."))

# Fetch NumPy release info from PyPI
with urllib.request.urlopen("https://pypi.org/pypi/numpy/json") as resp:
    data = json.load(resp)

# Sort releases descending and find latest <3.0
py_tag = f"cp{sys.version_info.major}{sys.version_info.minor}"
best_wheel = None

for version in sorted(data["releases"].keys(), key=version_tuple, reverse=True):
    vtuple = version_tuple(version)
    if vtuple[0] >= 3:
        continue

    # Find best wheel in this release
    best_wheel_in_release = None
    best_osx_version_in_release = (0, 0)

    for f in data["releases"][version]:
        fname = f["filename"]
        if not fname.endswith(".whl"):
            continue
        if py_tag not in fname or args.arch not in fname:
            continue
        osx_version = parse_osx_version(fname)
        if not osx_version or osx_version > target_tuple:
            continue
        if osx_version > best_osx_version_in_release:
            best_wheel_in_release = f
            best_osx_version_in_release = osx_version

    if best_wheel_in_release:
        best_wheel = best_wheel_in_release
        break  # Stop at latest release with a matching wheel

if not best_wheel:
    sys.exit(1)

# Upgrade pip and install
subprocess.check_call([sys.executable, "-m", "pip", "install", "--no-warn-script-location", "--upgrade", "pip"])
subprocess.check_call([sys.executable, "-m", "pip", "install", "--no-warn-script-location", best_wheel["url"]])
