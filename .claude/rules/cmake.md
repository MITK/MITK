---
paths:
  - CMake/**/*.cmake
  - CMake/**/CMakeLists.txt
  - CMakeExternals/**/*.cmake
  - SuperBuild.cmake
  - CMakeLists.txt
---

# Build-system internals

This rule covers the build system itself: the macros under
`CMake/`, the external-project recipes under `CMakeExternals/`,
the superbuild, and the top-level `CMakeLists.txt`. For
consumer `CMakeLists.txt` and `files.cmake` files inside `Modules/`
or `Plugins/`, use the respective subtree rules.

## Layout

- `CMake/` - MITK's own CMake macros and helpers, including the
  `mitk_create_module` / `mitk_create_plugin` /
  `MITK_CREATE_MODULE_TESTS` definitions.
- `CMake/BuildConfigurations/` - predefined
  `MITK_BUILD_CONFIGURATION` presets (see the table in root
  `CLAUDE.md`).
- `CMake/PackageDepends/` - `find_package` wrappers for
  third-party libraries.
- `CMakeExternals/` - `ExternalProject_Add` recipes; the master
  list is `CMakeExternals/ExternalProjectList.cmake`.
- `SuperBuild.cmake` - orchestrates the superbuild phase.

## Conventions

- New external dependencies: add a recipe under `CMakeExternals/`
  and register it in `ExternalProjectList.cmake`.
- New build configurations: add a `.cmake` file under
  `CMake/BuildConfigurations/`; document it in the root
  `CLAUDE.md` table when it is meant for general use.
- Boost is bundled (legacy reasons), but prefer the C++ standard
  library when an equivalent exists.
- CMake commands are case-insensitive, but follow the prevailing
  casing of the surrounding code when extending an existing file.
