#!/usr/bin/env python3

import ssl
import re
import sys
import argparse

def extract_major_minor(version_string: str) -> str:
    """
    Extracts the major.minor OpenSSL version from a version string.

    Example:
        "OpenSSL 3.0.16 11 Feb 2025" -> "3.0"
    """
    match = re.search(r"(\d+\.\d+)", version_string)
    if not match:
        return ""
    return match.group(1)

parser = argparse.ArgumentParser(description="Check OpenSSL version used by Python Standalone Builds")
parser.add_argument("--expected", required=True, help="Expected OpenSSL version (found by CMake)")
args = parser.parse_args()

expected_version = extract_major_minor(args.expected)

if not expected_version:
    sys.stderr.write(f"ERROR: Could not parse expected OpenSSL version from '{args.expected}'\n")
    sys.exit(1)

actual_version = extract_major_minor(ssl.OPENSSL_VERSION)

if not actual_version:
    sys.stderr.write(f"ERROR: Could not parse Python's OpenSSL version from '{ssl.OPENSSL_VERSION}'\n")
    sys.exit(1)


if actual_version != expected_version:
    sys.stderr.write(
        "ERROR: OpenSSL version mismatch detected!\n"
        f"  Found by CMake:           v{expected_version}\n"
        f"  Python Standalone Builds: v{actual_version}\n"
        "Python Standalone Builds and MITK must bundle identical major and minor OpenSSL versions.\n"
    )
    sys.exit(1)

print(f"OK: Python Standalone Builds uses OpenSSL v{actual_version} (matches expected version).")
