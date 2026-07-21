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

"""Parallel launcher for fetch_one.cmake.

Runs the cross-platform per-module fetch cmake script concurrently, one worker
per module. BoostProvision.cmake uses this when a Python interpreter is
available; the pure-CMake fallback runs the same fetch_one.cmake sequentially.

usage: fetch_boost.py <boost_src> <list_file> <fetch_one.cmake> <cmake> [jobs]
  list_file: lines of "module|url|sha"
"""

import concurrent.futures
import subprocess
import sys


def main():
    boost_src, list_file, fetch_one, cmake = sys.argv[1:5]
    jobs = int(sys.argv[5]) if len(sys.argv) > 5 else 8

    items = []
    with open(list_file, encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            if line:
                items.append(line.split('|'))

    def run(item):
        module, url, sha = item
        r = subprocess.run(
            [cmake, '-DBOOST_SRC=' + boost_src, '-DMODULE=' + module,
             '-DBOOST_URL=' + url, '-DBOOST_SHA=' + sha, '-P', fetch_one],
            capture_output=True, text=True)
        return module, r.returncode, r.stderr

    rc = 0
    with concurrent.futures.ThreadPoolExecutor(max_workers=jobs) as ex:
        for module, code, err in ex.map(run, items):
            if code != 0:
                sys.stderr.write('FAIL %s\n%s\n' % (module, err))
                rc = 1
    sys.exit(rc)


if __name__ == '__main__':
    main()
