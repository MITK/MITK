#!/usr/bin/env python3

"""Build a redistributable Python wheel from a completed MITK build.

This script stages the mitk_python_bindings target and auto-load modules
into a wheel-compatible layout, generates wheel metadata, packs the wheel,
and runs a platform-specific delocator to bundle all native dependencies.

Usage:
  python build_wheel.py --build-dir <MITK-build>

Prerequisites:
  - MITK must be fully built (SuperBuild completed, PythonWheel configuration)
  - The 'wheel' package must be installed: pip install wheel
  - Platform delocator must be installed:
      Windows: pip install delvewheel
      Linux:   pip install auditwheel (+ patchelf)
      macOS:   pip install delocate
"""

import argparse
import hashlib
import os
import platform
import re
import shutil
import subprocess
import sys
import sysconfig
import tempfile
from base64 import urlsafe_b64encode
from pathlib import Path


def get_platform_tag():
    """Return the wheel platform tag for the current platform.

    On Linux, detects the system glibc version and uses the corresponding
    manylinux tag (e.g., glibc 2.39 -> manylinux_2_39). When building in
    a manylinux container, this automatically produces the correct tag.
    """
    system = platform.system()
    machine = platform.machine().lower()

    if system == "Windows":
        return f"win_{machine}"
    elif system == "Linux":
        libc_name, libc_ver = platform.libc_ver()
        if libc_name == "glibc" and libc_ver:
            major, minor = libc_ver.split(".")[:2]
            return f"manylinux_{major}_{minor}_{machine}"
        return f"linux_{machine}"
    elif system == "Darwin":
        ver = platform.mac_ver()[0]
        major, minor = ver.split(".")[:2]
        return f"macosx_{major}_{minor}_{machine}"
    else:
        raise RuntimeError(f"Unsupported platform: {system}")


def get_python_tag():
    """Return the Python version tag (e.g., 'cp312')."""
    impl = "cp"  # CPython
    ver = f"{sys.version_info.major}{sys.version_info.minor}"
    return f"{impl}{ver}"


def get_abi_tag():
    """Return the ABI tag (e.g., 'cp312')."""
    return get_python_tag()


def get_source_dir(build_dir):
    """Read CMAKE_HOME_DIRECTORY (the MITK source dir) from CMakeCache.txt."""
    cache_file = Path(build_dir) / "CMakeCache.txt"
    for line in cache_file.read_text().splitlines():
        match = re.match(r"^CMAKE_HOME_DIRECTORY:INTERNAL=(.+)$", line)
        if match:
            return Path(match.group(1))
    return None


def compute_version(source_dir, base_version):
    """Return a PEP 440 version derived from git state.

    If HEAD is at a tag, use it (stripping a leading 'v'). Otherwise return
    ``base_version + '+g<shorthash>'`` as a PEP 440 local version label.
    Falls back to base_version if git is unavailable or source_dir is not a
    git repository (e.g. source tarball, missing .git, no git on PATH).
    """
    if source_dir is None or not (source_dir / ".git").exists():
        return base_version
    try:
        tags = subprocess.run(
            ["git", "-C", str(source_dir), "tag", "--points-at", "HEAD"],
            capture_output=True, text=True, check=True,
        ).stdout.strip().splitlines()
        if tags:
            tag = tags[0]
            return tag[1:] if tag.startswith("v") else tag
        short = subprocess.run(
            ["git", "-C", str(source_dir), "rev-parse", "--short", "HEAD"],
            capture_output=True, text=True, check=True,
        ).stdout.strip()
        if short:
            return f"{base_version}+g{short}"
    except (subprocess.CalledProcessError, FileNotFoundError):
        pass
    return base_version


def cmake_install_wheel_component(build_dir, staging_dir, cmake_command="cmake"):
    """Run cmake --install to stage the wheel component."""
    cmd = [
        cmake_command,
        "--install", str(build_dir),
        "--component", "wheel",
        "--prefix", str(staging_dir),
    ]
    if platform.system() == "Windows":
        cmd.extend(["--config", "Release"])

    print(f"Staging wheel component: {' '.join(cmd)}")
    subprocess.check_call(cmd)


def get_mitk_version(build_dir):
    """Extract MITK version from the CMake cache."""
    cache_file = Path(build_dir) / "CMakeCache.txt"
    if not cache_file.exists():
        raise FileNotFoundError(f"CMakeCache.txt not found in {build_dir}")

    version_vars = {}
    for line in cache_file.read_text().splitlines():
        for var in ("CMAKE_PROJECT_VERSION_MAJOR", "CMAKE_PROJECT_VERSION_MINOR",
                    "CMAKE_PROJECT_VERSION_PATCH"):
            match = re.match(rf"^{var}:STATIC=(.+)$", line)
            if match:
                version_vars[var] = match.group(1)

    if len(version_vars) < 3:
        raise RuntimeError(f"Could not find MITK version in {cache_file}")

    return (
        f"{version_vars['CMAKE_PROJECT_VERSION_MAJOR']}"
        f".{version_vars['CMAKE_PROJECT_VERSION_MINOR']}"
        f".{version_vars['CMAKE_PROJECT_VERSION_PATCH']}"
    )


def write_dist_info(staging_dir, package_name, version):
    """Write wheel metadata (METADATA, WHEEL, top_level.txt, RECORD)."""
    dist_info_dir = staging_dir / f"{package_name}-{version}.dist-info"
    dist_info_dir.mkdir(parents=True, exist_ok=True)

    python_tag = get_python_tag()
    abi_tag = get_abi_tag()
    platform_tag = get_platform_tag()

    # METADATA
    (dist_info_dir / "METADATA").write_text(
        f"Metadata-Version: 2.4\n"
        f"Name: {package_name}\n"
        f"Version: {version}\n"
        f"Summary: Python bindings for the Medical Imaging Interaction Toolkit (MITK)\n"
        f"License: BSD-3-Clause\n"
        f"Requires-Python: >={sys.version_info.major}.{sys.version_info.minor}\n"
        f"Requires-Dist: numpy>=2.0\n"
        f"Project-URL: Homepage, https://www.mitk.org\n"
        f"Project-URL: Repository, https://github.com/MITK/MITK\n"
    )

    # WHEEL
    (dist_info_dir / "WHEEL").write_text(
        f"Wheel-Version: 1.0\n"
        f"Generator: mitk-build-wheel\n"
        f"Root-Is-Purelib: false\n"
        f"Tag: {python_tag}-{abi_tag}-{platform_tag}\n"
    )

    # top_level.txt
    (dist_info_dir / "top_level.txt").write_text("mitk\n")

    # RECORD is written last (after all files are staged)
    return dist_info_dir


def write_record(staging_dir, dist_info_dir):
    """Write the RECORD file with SHA256 hashes of all wheel contents."""
    record_path = dist_info_dir / "RECORD"
    lines = []

    for root, _dirs, files in os.walk(staging_dir):
        for fname in files:
            fpath = Path(root) / fname
            rel = fpath.relative_to(staging_dir).as_posix()

            if rel.endswith("RECORD"):
                lines.append(f"{rel},,\n")
                continue

            data = fpath.read_bytes()
            digest = urlsafe_b64encode(
                hashlib.sha256(data).digest()
            ).rstrip(b"=").decode("ascii")
            size = len(data)
            lines.append(f"{rel},sha256={digest},{size}\n")

    record_path.write_text("".join(lines))


def pack_wheel(staging_dir, output_dir):
    """Pack the staging directory into a .whl file."""
    output_dir.mkdir(parents=True, exist_ok=True)
    cmd = [sys.executable, "-m", "wheel", "pack", str(staging_dir), "-d", str(output_dir)]
    print(f"Packing wheel: {' '.join(cmd)}")
    subprocess.check_call(cmd)

    # Find the produced wheel
    wheels = list(output_dir.glob("mitk-*.whl"))
    if not wheels:
        raise RuntimeError("No wheel produced")
    return wheels[0]


def get_library_search_paths(build_dir):
    """Collect library search paths for the delocator."""
    build_dir = Path(build_dir)
    paths = []

    # MITK build output (Windows: bin/, Linux/macOS: lib/)
    bin_dir = build_dir / "bin"
    if (bin_dir / "Release").is_dir():
        paths.append(str(bin_dir / "Release"))
    elif bin_dir.is_dir():
        paths.append(str(bin_dir))

    lib_dir = build_dir / "lib"
    if lib_dir.is_dir():
        paths.append(str(lib_dir))

    # SuperBuild external project libraries
    superbuild_dir = build_dir.parent
    ep_lib = superbuild_dir / "ep" / "lib"
    if ep_lib.is_dir():
        paths.append(str(ep_lib))
    ep_bin = superbuild_dir / "ep" / "bin"
    if ep_bin.is_dir():
        paths.append(str(ep_bin))

    # Python libraries (for the delocator to exclude)
    python_dir = build_dir / "python"
    if python_dir.is_dir():
        paths.append(str(python_dir))
        if platform.system() == "Windows":
            paths.append(str(python_dir / "Lib" / "site-packages" / "mitk"))
        else:
            pyver = f"python{sys.version_info.major}.{sys.version_info.minor}"
            paths.append(str(python_dir / "lib" / pyver / "site-packages" / "mitk"))

    return paths


def repair_wheel(wheel_path, output_dir, search_paths):
    """Run the platform-specific delocator to bundle native dependencies."""
    system = platform.system()

    if system == "Windows":
        add_path = ";".join(search_paths)
        cmd = [
            sys.executable, "-m", "delvewheel", "repair",
            str(wheel_path),
            "--add-path", add_path,
            "--no-mangle-all",  # CppMicroServices appends zip resources to DLLs (overlay)
            "--analyze-existing",  # trace deps of auto-load DLLs already in the wheel
            "--wheel-dir", str(output_dir),
        ]
    elif system == "Linux":
        # Set LD_LIBRARY_PATH for auditwheel to find libraries
        env = os.environ.copy()
        env["LD_LIBRARY_PATH"] = ":".join(search_paths) + ":" + env.get("LD_LIBRARY_PATH", "")
        cmd = [
            sys.executable, "-m", "auditwheel", "repair",
            str(wheel_path),
            "--plat", get_platform_tag(),
            "-w", str(output_dir),
        ]
    elif system == "Darwin":
        env = os.environ.copy()
        # delocate resolves @rpath via each dylib's own LC_RPATH, which CMake
        # rewrites to @loader_path/.. on install. Provide the build-tree
        # search paths via DYLD_FALLBACK_LIBRARY_PATH so delocate can locate
        # external dependencies the staged dylibs no longer point to directly.
        env["DYLD_LIBRARY_PATH"] = ":".join(search_paths) + ":" + env.get("DYLD_LIBRARY_PATH", "")
        env["DYLD_FALLBACK_LIBRARY_PATH"] = ":".join(search_paths) + ":" + env.get("DYLD_FALLBACK_LIBRARY_PATH", "")
        # -v emits one log line per copied library. Without it delocate is
        # silent for 10+ minutes on a slow Intel host, indistinguishable
        # from a hang.
        cmd = [
            sys.executable, "-m", "delocate.cmd.delocate_wheel",
            "-v",
            str(wheel_path),
            "-w", str(output_dir),
        ]
    else:
        raise RuntimeError(f"Unsupported platform: {system}")

    # Snapshot existing wheels so we can identify the one produced by this
    # invocation. Stale wheels from previous builds may share the directory
    # (output_dir is typically MITK-build, not a temp dir), and auditwheel
    # may rewrite the platform tag so the output filename can differ from
    # wheel_path.name. Tracking name+mtime handles both cases.
    before = {p: p.stat().st_mtime for p in output_dir.glob("mitk-*.whl")}

    print(f"Repairing wheel: {' '.join(cmd)}")
    env_to_use = env if system != "Windows" else None
    subprocess.check_call(cmd, env=env_to_use)

    new_or_updated = [
        p for p in output_dir.glob("mitk-*.whl")
        if p not in before or p.stat().st_mtime > before[p]
    ]
    if not new_or_updated:
        raise RuntimeError("No repaired wheel produced")
    return max(new_or_updated, key=lambda p: p.stat().st_mtime)


def main():
    parser = argparse.ArgumentParser(
        description="Build a redistributable Python wheel from a completed MITK build."
    )
    parser.add_argument(
        "--build-dir",
        required=True,
        help="Path to MITK-build directory (not the superbuild root)",
    )
    parser.add_argument(
        "--output-dir",
        default=None,
        help="Directory for the output wheel (default: build-dir)",
    )
    parser.add_argument(
        "--cmake",
        default="cmake",
        help="Path to cmake executable (default: cmake)",
    )
    parser.add_argument(
        "--skip-repair",
        action="store_true",
        help="Skip the delocator step (for debugging)",
    )
    args = parser.parse_args()

    build_dir = Path(args.build_dir).resolve()
    output_dir = Path(args.output_dir).resolve() if args.output_dir else build_dir

    if not build_dir.is_dir():
        print(f"Error: build directory not found: {build_dir}", file=sys.stderr)
        return 1

    # Get version: tag at HEAD if present, otherwise base + '+g<shorthash>'
    base_version = get_mitk_version(build_dir)
    source_dir = get_source_dir(build_dir)
    version = compute_version(source_dir, base_version)
    print(f"MITK version: {version}")

    with tempfile.TemporaryDirectory() as tmpdir:
        staging_dir = Path(tmpdir) / "staging"
        staging_dir.mkdir()

        # Stage wheel component
        cmake_install_wheel_component(build_dir, staging_dir, args.cmake)

        # Verify staging
        mitk_pkg = staging_dir / "mitk"
        if not mitk_pkg.is_dir():
            print("Error: cmake --install did not produce mitk/ directory", file=sys.stderr)
            return 1

        # Write dist-info
        dist_info_dir = write_dist_info(staging_dir, "mitk", version)
        write_record(staging_dir, dist_info_dir)

        # Pack raw wheel
        raw_dir = Path(tmpdir) / "raw"
        raw_wheel = pack_wheel(staging_dir, raw_dir)
        print(f"Raw wheel: {raw_wheel}")

        if args.skip_repair:
            # Just copy the raw wheel to output
            output_dir.mkdir(parents=True, exist_ok=True)
            shutil.copy2(raw_wheel, output_dir)
            print(f"Output (unrepaired): {output_dir / raw_wheel.name}")
        else:
            # Repair with platform delocator
            search_paths = get_library_search_paths(build_dir)
            print(f"Library search paths: {search_paths}")
            repaired = repair_wheel(raw_wheel, output_dir, search_paths)
            print(f"Output: {repaired}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
