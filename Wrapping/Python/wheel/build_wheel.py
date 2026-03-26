#!/usr/bin/env python3

"""Build a redistributable Python wheel from a completed MITK build.

This script stages the mitk_python_bindings target and auto-load modules
into a wheel-compatible layout, generates wheel metadata, packs the wheel,
and runs a platform-specific delocator to bundle all native dependencies.

Usage:
  python build_wheel.py --build-dir <MITK-build> --output-dir dist/

Prerequisites:
  - MITK must be fully built (SuperBuild completed)
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


def pep440_version(version_string):
    """Convert MITK version string to PEP 440 compliant version.

    MITK uses "major.minor.99-shortid" for development versions.
    PEP 440 does not allow dashes, so we convert to "major.minor.99.devN".
    """
    if "-" in version_string:
        base, suffix = version_string.split("-", 1)
        # Use a hash of the suffix as a numeric dev identifier
        dev_num = int(hashlib.sha1(suffix.encode()).hexdigest()[:6], 16)
        return f"{base}.dev{dev_num}"
    return version_string


def cmake_install_wheel_component(build_dir, staging_dir, config="Release"):
    """Run cmake --install to stage the wheel component."""
    cmd = [
        "cmake",
        "--install", str(build_dir),
        "--component", "wheel",
        "--prefix", str(staging_dir),
    ]
    if platform.system() == "Windows":
        cmd.extend(["--config", config])

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
        env["DYLD_LIBRARY_PATH"] = ":".join(search_paths) + ":" + env.get("DYLD_LIBRARY_PATH", "")
        cmd = [
            sys.executable, "-m", "delocate", "repair",
            str(wheel_path),
            "-w", str(output_dir),
        ]
    else:
        raise RuntimeError(f"Unsupported platform: {system}")

    print(f"Repairing wheel: {' '.join(cmd)}")
    env_to_use = env if system != "Windows" else None
    subprocess.check_call(cmd, env=env_to_use)

    # Find the repaired wheel
    repaired = list(Path(output_dir).glob("mitk-*.whl"))
    if not repaired:
        raise RuntimeError("No repaired wheel found")
    return repaired[-1]


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
        default="dist",
        help="Directory for the output wheel (default: dist/)",
    )
    parser.add_argument(
        "--config",
        default="Release",
        help="Build configuration (default: Release)",
    )
    parser.add_argument(
        "--skip-repair",
        action="store_true",
        help="Skip the delocator step (for debugging)",
    )
    args = parser.parse_args()

    build_dir = Path(args.build_dir).resolve()
    output_dir = Path(args.output_dir).resolve()

    if not build_dir.is_dir():
        print(f"Error: build directory not found: {build_dir}", file=sys.stderr)
        return 1

    # Get version
    version = pep440_version(get_mitk_version(build_dir))
    print(f"MITK version: {version}")

    with tempfile.TemporaryDirectory() as tmpdir:
        staging_dir = Path(tmpdir) / "staging"
        staging_dir.mkdir()

        # Stage wheel component
        cmake_install_wheel_component(build_dir, staging_dir, args.config)

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
