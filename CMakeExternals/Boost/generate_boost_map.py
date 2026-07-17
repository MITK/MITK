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

"""Generate the Boost module map used by MITK's slim Boost provisioning.

Run once per Boost release against a full boostorg/boost checkout (all submodules
present); the output (boost-deps.cmake) is committed so that build-time
provisioning needs no Python and no source scanning:

    git clone --recurse-submodules --branch boost-<x.y.0> \\
        https://github.com/boostorg/boost.git <checkout>
    python generate_boost_map.py <checkout> boost-deps.cmake

Per module it records the direct module dependencies, the upstream repo URL, the
release-pinned commit, and whether the module builds a compiled library.
Header-to-module resolution mirrors boostorg/boostdep depinst.py (BSL-1.0,
(c) Peter Dimov) so the closure matches what depinst would fetch.
"""

import os
import re
import subprocess
import sys

INCLUDE_RE = re.compile(r'[ \t]*#[ \t]*include[ \t]*["<](boost/[^">]*)[">]')
SRC_EXT = ('.cpp', '.cxx', '.cc', '.c', '.asm')


def read_gitmodules(root):
    """Return {path: absolute-boostorg-url}. In each section 'path' precedes 'url'."""
    urls = {}
    section_path = None
    with open(os.path.join(root, '.gitmodules'), encoding='utf-8') as f:
        for line in f:
            s = line.strip()
            m = re.match(r'path\s*=\s*(.*)$', s)
            if m:
                section_path = m.group(1)
                continue
            m = re.match(r'url\s*=\s*(.*)$', s)
            if m and section_path is not None:
                repo = os.path.basename(m.group(1))
                if repo.endswith('.git'):
                    repo = repo[:-4]
                urls[section_path] = 'https://github.com/boostorg/%s.git' % repo
                section_path = None
    return urls


def read_exceptions(path):
    x = {}
    module = None
    with open(path, encoding='utf-8') as f:
        for line in f:
            line = line.rstrip()
            m = re.match(r'(.*):$', line)
            if m:
                module = m.group(1).replace('~', '/')
            else:
                x[line.lstrip()] = module
    return x


def is_module(m, gm):
    return ('libs/' + m) in gm


def module_for_header(h, x, gm):
    if h in x:
        return x[h]
    for pat in (r'boost/([^\./]*)\.h[a-z]*$',
                r'boost/([^/]*/[^\./]*)\.h[a-z]*$',
                r'boost/([^/]*/[^/]*)/',
                r'boost/([^/]*)/'):
        m = re.match(pat, h)
        if m and is_module(m.group(1), gm):
            return m.group(1)
    return None


def scan_dir(d, x, gm, deps):
    for root, _dirs, files in os.walk(d):
        for fn in files:
            try:
                with open(os.path.join(root, fn), encoding='latin-1') as f:
                    for line in f:
                        m = INCLUDE_RE.match(line)
                        if m:
                            mod = module_for_header(m.group(1), x, gm)
                            if mod:
                                deps.add(mod)
            except OSError:
                pass


def has_sources(d):
    for _root, _dirs, files in os.walk(d):
        if any(fn.endswith(SRC_EXT) for fn in files):
            return True
    return False


def sha_for(root, path):
    r = subprocess.run(['git', '-C', root, 'rev-parse', 'HEAD:' + path],
                       capture_output=True, text=True)
    return r.stdout.strip() if r.returncode == 0 else None


def main():
    if len(sys.argv) != 3:
        sys.exit('usage: generate_boost_map.py <boost-root> <out.cmake>')
    root = os.path.abspath(sys.argv[1])
    out = os.path.abspath(sys.argv[2])
    os.chdir(root)

    urls = read_gitmodules('.')
    gm = list(urls.keys())
    exc_path = 'tools/boostdep/depinst/exceptions.txt'
    x = read_exceptions(exc_path) if os.path.exists(exc_path) else {}

    modules, deps_map, compiled, sha_map = [], {}, [], {}
    for path in gm:
        if not path.startswith('libs/'):
            continue
        name = path[len('libs/'):]
        incl, src = os.path.join(path, 'include'), os.path.join(path, 'src')
        if not os.path.isdir(incl) and not os.path.isdir(src):
            continue
        deps = set()
        if os.path.isdir(incl):
            scan_dir(incl, x, gm, deps)
        if os.path.isdir(src):
            scan_dir(src, x, gm, deps)
        deps.discard(name)
        sha = sha_for('.', path)
        if not sha or path not in urls:
            continue
        modules.append(name)
        deps_map[name] = sorted(deps)
        sha_map[name] = sha
        if os.path.isdir(src) and has_sources(src):
            compiled.append(name)

    def mangle(n):
        return n.replace('/', '__')

    with open(out, 'w', encoding='utf-8', newline='\n') as f:
        f.write('# Generated by generate_boost_map.py - do not edit.\n')
        f.write('# Boost module map: deps, upstream url, release-pinned commit,\n')
        f.write('# and the set of modules that build a compiled library.\n\n')
        for name in sorted(modules):
            mv = mangle(name)
            f.write('set(BOOST_MODULE_DEPS_{} "{}")\n'.format(mv, ';'.join(deps_map[name])))
            f.write('set(BOOST_MODULE_URL_{} "{}")\n'.format(mv, urls['libs/' + name]))
            f.write('set(BOOST_MODULE_SHA_{} "{}")\n'.format(mv, sha_map[name]))
        f.write('\nset(BOOST_ALL_MODULES "{}")\n'.format(';'.join(sorted(modules))))
        f.write('set(BOOST_COMPILED_MODULES "{}")\n'.format(';'.join(sorted(compiled))))

    print('modules:', len(modules), 'compiled:', len(compiled))


if __name__ == '__main__':
    main()
