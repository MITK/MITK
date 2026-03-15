# Packaging and Installation {#PackagingPage}

[TOC]

## Overview

MITK uses CMake's native install and packaging infrastructure to produce redistributable packages on Windows, Linux, and macOS.
The system is built around a single **runtime dependency set** (`mitk_deps`) that collects all targets during configuration and resolves their transitive shared-library dependencies in one pass at install time.

The high-level flow is:

1. Each target creation site (`mitk_create_module()`, `mitk_create_executable()`, `mitkFunctionCreateBlueBerryApplication()`, `mitkFunctionInstallCTKPlugin()`) registers its target in the `mitk_deps` dependency set via `install(TARGETS ... RUNTIME_DEPENDENCY_SET mitk_deps)`.
2. At the end of the top-level `CMakeLists.txt`, `mitkInstallRules.cmake` is included. It installs special-case targets (CppMicroServices, Python), resolves all transitive dependencies, and deploys Qt.
3. CPack picks up the resulting install tree and produces platform-specific packages.

## Two-Phase Build Recap

MITK uses a SuperBuild pattern with two distinct CMake phases:

1. **SuperBuild phase** (`SuperBuild.cmake`): Downloads, configures, builds, and installs (most) external dependencies (ITK, VTK, CTK, DCMTK, Boost, Python3, pybind11, etc.) into an external project prefix (`ep/`). Each external project is configured with a shared `CMAKE_INSTALL_RPATH` so its libraries can find each other at runtime.

2. **MITK build phase** (top-level `CMakeLists.txt`): Builds MITK modules, plugins, and applications. Uses `find_package()` to locate the externals installed by the SuperBuild. All install rules target this phase.

External projects are built and installed into the SuperBuild prefix. The MITK build itself lives in `MITK-superbuild/MITK-build/`. Only the MITK build phase generates install/package rules — the SuperBuild phase does not install anything into the final package directly. Instead, the runtime dependency resolution in `mitkInstallRules.cmake` finds and copies external libraries from the SuperBuild prefix.

## Install Architecture

### The RUNTIME_DEPENDENCY_SET Approach

CMake 3.21+ provides `install(RUNTIME_DEPENDENCY_SET)`, which collects targets and resolves their transitive shared-library dependencies at install time. MITK uses a single dependency set named `mitk_deps`.

Every installable target registers with this set:

```cmake
install(TARGETS MyTarget
  RUNTIME_DEPENDENCY_SET mitk_deps
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION bin)
```

At the end of configuration, a single call in `mitkInstallRules.cmake` resolves all collected targets:

```cmake
install(RUNTIME_DEPENDENCY_SET mitk_deps
  PRE_EXCLUDE_REGEXES ...   # Skip Windows system DLLs
  POST_EXCLUDE_REGEXES ...  # Skip system libraries, Python, plugin dirs
  DIRECTORIES ${_search_dirs}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION bin
  FRAMEWORK DESTINATION bin)
```

This replaces the legacy approach of manually walking targets with `BundleUtilities` or hand-maintained install loops.

### Dependency Filtering

The single resolution call uses two layers of filtering:

**PRE_EXCLUDE_REGEXES** (matched against the dependency filename before searching):
- `api-ms-*`, `ext-ms-*` — Windows API sets
- `msvcp*`, `vcruntime*`, `concrt*`, `vcomp*`, `ucrtbase` — Visual C++ runtime (deployed separately via `InstallRequiredSystemLibraries`)

**POST_EXCLUDE_REGEXES** (matched against the full resolved path):
- `[/\\]Windows[/\\]` — Windows system directory (case-insensitive)
- `/usr/lib`, `/lib`, `/System` — Linux/macOS system libraries
- `python3[0-9]+.` — Python shared library (installed separately)
- `.*/plugins/.*` — Qt/CTK plugins (deployed by Qt deployment or CTK plugin install)

### Search Directories

The `DIRECTORIES` argument tells CMake where to look for dependencies. These are provided by `mitkFunctionGetLibrarySearchPaths()`, which collects:

- MITK runtime and library output directories
- External project prefix (`ep/bin`, `ep/lib`)
- OpenSSL binary directory
- Additional module library search paths (from `MITK_ADDITIONAL_LIBRARY_SEARCH_PATHS` global property)
- Python directories
- CTK library directories
- Qt library directory (via `qmake -query QT_INSTALL_LIBS` / `QT_INSTALL_BINS`)
- Per-external-project paths from `MITK-AdditionalLibPaths/*.cmake` files

### Ordering in CMakeLists.txt

The `include(mitkInstallRules)` call must appear **after all targets have been created** (Modules, Plugins, Applications, Examples) so that the dependency set is complete:

```cmake
add_subdirectory(Utilities)
add_subdirectory(Modules)
# ... Plugins, Applications, Examples ...
include(mitkInstallRules)  # Must come last
```

## Target Types and Their Install Rules

### Modules (Shared Libraries)

Created by `mitk_create_module()` in `mitkFunctionCreateModule.cmake`. Non-static, non-executable modules are installed to `bin/`:

```cmake
install(TARGETS ${MODULE_TARGET}
  RUNTIME_DEPENDENCY_SET mitk_deps
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION bin)
```

### Autoload Modules

Modules that specify `AUTOLOAD_WITH <ParentModule>` are installed into a subdirectory named after the parent module:

```cmake
install(TARGETS ${MODULE_TARGET}
  RUNTIME_DEPENDENCY_SET mitk_deps
  RUNTIME DESTINATION bin/${MODULE_AUTOLOAD_WITH}
  LIBRARY DESTINATION bin/${MODULE_AUTOLOAD_WITH})
```

At runtime, CppMicroServices automatically loads these modules when the parent module is loaded.

On Linux, autoload modules get `INSTALL_RPATH "$ORIGIN/.."` so they can find libraries in the parent `bin/` directory.

### Plugins (CTK/BlueBerry)

MITK plugins are installed by `mitkFunctionInstallCTKPlugin()`, called from `mitkFunctionCreatePlugin()`. Non-imported plugins register with the dependency set:

```cmake
install(TARGETS ${_install_target}
  RUNTIME_DEPENDENCY_SET mitk_deps
  RUNTIME DESTINATION bin/plugins
  LIBRARY DESTINATION bin/plugins)
```

Third-party (imported) CTK plugins are installed via `install(FILES ...)` since they are not CMake targets in the current project. On Linux, their RPATH is set post-install using `file(RPATH_SET)`.

Plugins get `INSTALL_RPATH "$ORIGIN/.."` on Linux to resolve libraries in `bin/`.

### Executables

Created by `mitk_create_executable()` (in `mitkMacroCreateExecutable.cmake`), which wraps `mitk_create_module()` with the `EXECUTABLE` option. Executables are installed to `bin/` with a wrapper script:

```cmake
install(TARGETS ${EXECUTABLE_TARGET}
  RUNTIME_DEPENDENCY_SET mitk_deps
  RUNTIME DESTINATION bin)
```

Command-line apps (created via `mitkFunctionCreateCommandLineApp()`) follow the same path but their wrapper scripts go into `apps/` instead of the install root.

### BlueBerry Applications

Created by `mitkFunctionCreateBlueBerryApplication()`. These are Qt-based GUI applications that additionally:

- Set the `MITK_DEPLOY_QT` target property to `TRUE`
- Install third-party CTK plugins via `mitkFunctionInstallThirdPartyCTKPlugins()`
- Create and install provisioning files
- Get a platform-specific wrapper script

```cmake
install(TARGETS ${_APP_NAME}
  RUNTIME_DEPENDENCY_SET mitk_deps
  RUNTIME DESTINATION bin
  BUNDLE DESTINATION .)
```

### CppMicroServices

The CppMicroServices library is a special case. It is built via `usMacroCreateModule()` (not `mitk_create_module()`), is not tracked in `MITK_MODULE_TARGETS`, and has `US_NO_INSTALL=1` to disable its own install rules. It must be explicitly installed in `mitkInstallRules.cmake`:

```cmake
install(TARGETS CppMicroServices
  RUNTIME_DEPENDENCY_SET mitk_deps
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION bin
  PUBLIC_HEADER DESTINATION include/CppMicroServices EXCLUDE_FROM_ALL
  PRIVATE_HEADER DESTINATION include/CppMicroServices EXCLUDE_FROM_ALL)
```

The `EXCLUDE_FROM_ALL` on header destinations suppresses warnings about the unused header install components.

See also the dedicated \ref CppMicroServicesResourcesSection section below for how CppMicroServices resource embedding affects packaging.

### Python

When `MITK_USE_Python3` is enabled, the entire Python distribution from the build tree is copied into the package, and the MITK Python bindings (`mitk_python_bindings` target) are installed into the appropriate site-packages directory:

```cmake
install(DIRECTORY "${MITK_BINARY_DIR}/python/"
  DESTINATION ${_python_dest}
  USE_SOURCE_PERMISSIONS)

install(TARGETS mitk_python_bindings
  RUNTIME_DEPENDENCY_SET mitk_deps
  RUNTIME DESTINATION ${_python_dest}/${_rel_sitearch}/mitk
  LIBRARY DESTINATION ${_python_dest}/${_rel_sitearch}/mitk)
```

On macOS, the destination is `../Frameworks/Python.framework` (relative to the bundle's `Contents/` directory).

## Qt Deployment

Qt deployment is handled by `mitkFunctionDeployQt()` in `mitkFunctionDeployQt.cmake`. It runs **after** the runtime dependency resolution, so that tools like `windeployqt` can see all MITK DLLs already present in `bin/` and correctly trace their transitive Qt dependencies.

Only targets with the `MITK_DEPLOY_QT` property set to `TRUE` trigger Qt deployment. Currently, only BlueBerry applications set this property.

### Why Two Different Qt Deploy Functions?

Qt provides two deployment entry points:

- **qt_generate_deploy_app_script()** — designed for structured application layouts, specifically macOS `.app` bundles. It handles the `Contents/Frameworks/`, `Contents/PlugIns/`, `Contents/Resources/` layout automatically.
- **qt_generate_deploy_script()** — a lower-level function that generates a custom CMake install script. It accepts a `CONTENT` parameter with arbitrary CMake code, giving full control over deployment behavior.

MITK uses `qt_generate_deploy_app_script()` only on macOS (for `.app` bundles). On Windows and Linux, MITK uses `qt_generate_deploy_script()` with `qt_deploy_runtime_dependencies()` embedded in the `CONTENT` because:

1. There are no app bundles on these platforms — the install layout is a flat `bin/` directory.
2. Qt's default FHS-style paths (`lib/`, `libexec/`, `plugins/`) do not match MITK's portable `bin/`-centric layout.
3. Platform-specific options (e.g. `windeployqt` flags) must be passed through.

### Platform-Specific Behavior

**macOS**: Uses `qt_generate_deploy_app_script()` which handles the `.app` bundle layout natively.

**Windows**: Uses `qt_generate_deploy_script()` with `qt_deploy_runtime_dependencies()`. The `--no-opengl-sw` option is passed to `windeployqt` to skip the software OpenGL fallback. If OpenSSL is available, its root directory is passed via `--openssl-root`.

After deployment, a post-install step moves `resources/` and `translations/` directories from the install prefix root into `bin/`, because `windeployqt` places them at the prefix root level and provides no option to change this.

**Linux**: Uses `qt_generate_deploy_script()` with overridden `QT_DEPLOY_*` variables so that everything goes under `bin/` instead of the default FHS-style directories:

```cmake
set(QT_DEPLOY_BIN_DIR "bin")
set(QT_DEPLOY_LIB_DIR "bin")
set(QT_DEPLOY_LIBEXEC_DIR "bin")
set(QT_DEPLOY_PLUGINS_DIR "bin/plugins")
set(QT_DEPLOY_QML_DIR "bin/qml")
set(QT_DEPLOY_TRANSLATIONS_DIR "bin/translations")
set(QT_DEPLOY_DATA_DIR "bin")
```

Without these overrides, Qt would deploy libraries to `lib/`, executables to `libexec/`, plugins to `plugins/` at the prefix root, etc. — scattering files across the install tree and breaking MITK's RPATH-based portable layout.

These overrides also affect internal deployment hooks (e.g. the WebEngine deployment hook), which is critical: it ensures that `QtWebEngineProcess`, resource `.pak` files, and locale data all land under `bin/` rather than at the prefix root.

### The qt.conf Problem on Linux

After the Qt deploy script runs on Linux, `qt.conf` is overwritten. The reason: Qt's WebEngine deployment hook generates `qt.conf` with an **absolute staging path** as the prefix (e.g. `/tmp/cpack-staging/install/bin`). This makes the deployment non-relocatable.

The fixup overwrites it with a correct relative prefix:

```ini
[Paths]
Prefix = .
```

Since `qt.conf` lives in `bin/` and the prefix is `.`, Qt looks for plugins at `bin/plugins/`, QML modules at `bin/qml/`, etc. — all relative to `bin/`.

This fixup is fragile. If Qt's internal deployment hooks change their `qt.conf` generation behavior in a future version, the overwrite may need to be adjusted.

### Qt Version Differences

The Qt deployment CMake API evolves between versions, and some changes directly affect MITK's deployment strategy.

**Qt 6.9 and earlier**: Plugin deployment uses a pre-populated plugin list (`__QT_DEPLOY_PLUGINS`). The deployment system does a single pass of dependency resolution.

**Qt 6.10+**: A completely rewritten plugin discovery system that uses iterative resolution (up to 10 passes). It reads module JSON descriptor files (`Qt6InstallDescriptions/*.json`) to automatically determine which plugin types each Qt module requires. New parameters are available:

- `NO_PLUGINS` — skip plugin deployment entirely
- `INCLUDE_PLUGINS` / `EXCLUDE_PLUGINS` — filter by plugin name
- `INCLUDE_PLUGIN_TYPES` / `EXCLUDE_PLUGIN_TYPES` — filter by plugin category

These new parameters are forwarded to platform-specific tools (`windeployqt` flags on Windows, `macdeployqt` flags on macOS, CMake logic on Linux).

Other notable changes in Qt 6.10:
- The deprecated `FILENAME_VARIABLE` parameter has been removed from `qt_generate_deploy_app_script()` — only `OUTPUT_SCRIPT` works.
- New internal variables like `__QT_DEPLOY_SHARED_LIBRARY_SUFFIX` and `__QT_LIBINFIX` are injected into deploy scripts for the new plugin discovery logic.

MITK's current deployment code works with both Qt 6.9 and 6.10. However, the new plugin filtering parameters in Qt 6.10 could be used in the future to fine-tune which Qt plugins are included in packages.

## RPATH Configuration

RPATH is configured per platform to ensure that shared libraries can find each other at runtime.

### Global RPATH (MITK Build)

Set in the top-level `CMakeLists.txt` for all MITK targets:

```cmake
if(LINUX)
  set(CMAKE_INSTALL_RPATH "$ORIGIN;$ORIGIN/plugins")
endif()
```

This means most installed binaries look for libraries in their own directory and in `plugins/` relative to their location.

### SuperBuild RPATH (External Projects)

External projects are configured with a separate RPATH in `SuperBuild.cmake`:

```cmake
# macOS
set(_install_rpath "@loader_path/../lib")

# Linux
set(_install_rpath "$ORIGIN:$ORIGIN/../lib")
if(Qt6_DIR)
  set(_install_rpath "${_install_rpath}:${Qt6_DIR}/../..")
endif()
```

This ensures external project libraries can find each other within the SuperBuild prefix during the build. At package time, these RPATHs are irrelevant since the dependency resolver copies all needed libraries into `bin/`.

### Per-Target RPATH Overrides

| Target type | RPATH (Linux) | Reason |
|---|---|---|
| Regular modules | `$ORIGIN;$ORIGIN/plugins` | Global default — find peers and plugins |
| Autoload modules | `$ORIGIN/..` | Installed in `bin/<parent>/`, must find libs in `bin/` |
| Plugins | `$ORIGIN/..` | Installed in `bin/plugins/`, must find libs in `bin/` |
| Imported CTK plugins | `$ORIGIN/..` (via `file(RPATH_SET)`) | Same as plugins, but set post-install since they are imported targets |
| Executables | `$ORIGIN;$ORIGIN/plugins` | Global default |

On **macOS**, `@loader_path` is used instead of `$ORIGIN`.

On **Windows**, RPATH does not apply. DLLs are found via the executable's directory and `PATH`.

## Wrapper Scripts

Wrapper scripts launch MITK executables from the install root. They exist because executables are installed in `bin/` but the user runs them from the install root (or a start menu shortcut).

### Windows

| Script | Used by | Behavior |
|---|---|---|
| `RunInstalledWin32App.bat` | BlueBerry apps | `start "" /B "%~dp0bin\%~n0.exe" -style windowsvista %*` — launches detached with Qt style |
| `RunInstalledApp.bat` | Regular executables | `"%~dp0bin\%~n0.exe" %*` — direct launch from root |
| `RunInstalledCmdLineApp.bat` | Command-line apps | `"%~dp0..\bin\%~n0.exe" %*` — launch from `apps/` subdirectory |

### Linux

| Script | Used by | Behavior |
|---|---|---|
| `RunInstalledApp.sh` | Regular executables and BlueBerry apps | Sets `LD_LIBRARY_PATH` to include `python/lib`, then launches `bin/<name>` |
| `RunInstalledCmdLineApp.sh` | Command-line apps | Same but resolves paths from `apps/` subdirectory |

On Linux, the wrapper scripts also prepend `python/lib` to `LD_LIBRARY_PATH`, which is necessary for the Python shared library to be found at runtime.

## CPack Configuration

CPack configuration is in `mitkSetupCPack.cmake`.

### Generators

| Platform | Generator(s) |
|---|---|
| Windows | ZIP (always), NSIS (optional, if `makensis` is found) |
| macOS | DragNDrop |
| Linux | TGZ |

### Versioning and Naming

The package version is derived from `MITK_REVISION_DESC` (a git-based version string). If not available, it falls back to `MITK_VERSION_MAJOR.MINOR.PATCH`.

The package filename follows the pattern:
```
MITK-<version>-<platform-arch>
```

Platform-arch examples: `windows-x86_64`, `ubuntu-22.04-x86_64`, `macos-14.0-arm64`.

On Linux, the distribution name and version are read from `/etc/os-release`.

### NSIS Settings

- Install root defaults to `$PROGRAMFILES64` on 64-bit Windows
- Start menu shortcuts are collected in `MITK_CPACK_PACKAGE_EXECUTABLES` by `mitkFunctionCreateBlueBerryApplication()`

### Strip Policy

Stripping is **disabled** globally (`CPACK_STRIP_FILES OFF`). See the \ref CppMicroServicesResourcesSection section for the detailed explanation of why this is necessary and how it could be improved in the future.

### macOS Post-Build Script

On macOS, CPack runs `FixMacOSInstaller.cmake` (generated from `FixMacOSInstaller.cmake.in`) as a post-build step. This script:

1. Converts the Python directory into a proper `Python.framework` (required for code signing)
2. Fixes `@executable_path` references in the mitk Python module to use `@loader_path`
3. Signs the app bundle with `codesign`

## Library Search Paths

`mitkFunctionGetLibrarySearchPaths()` serves a dual purpose:

1. **Build time**: Provides directories for `PATH` / `LD_LIBRARY_PATH` in batch scripts and test runners
2. **Install time**: Provides `DIRECTORIES` for `install(RUNTIME_DEPENDENCY_SET)` to locate dependencies

The function collects paths from:

| Source | Paths |
|---|---|
| MITK build tree | Runtime and library output directories |
| External project prefix | `ep/bin`, `ep/lib` |
| OpenSSL | Derived from `IMPORTED_LOCATION` of `OpenSSL::SSL` |
| Module additional libs | `MITK_ADDITIONAL_LIBRARY_SEARCH_PATHS` global property |
| Python | `python/` (Windows) or `python/lib` (Linux/macOS) |
| CTK | `CTK_LIBRARY_DIRS` and per-library dirs |
| Qt | `QT_INSTALL_BINS` (Windows) or `QT_INSTALL_LIBS` (Linux/macOS) |
| SuperBuild external projects | `MITK-AdditionalLibPaths/*.cmake` files (via `mitkFunctionAddLibrarySearchPaths()`) |

External projects register their library paths via `mitkFunctionAddLibrarySearchPaths()`, which writes per-configuration path info into `MITK-AdditionalLibPaths/<name>.cmake` files. These are picked up automatically by `mitkFunctionGetLibrarySearchPaths()`.

## Directory Layout

### Windows / Linux

```
<install-root>/
├── MitkWorkbench.bat/.sh          # Wrapper scripts (install root)
├── apps/
│   └── MitkFileConverter.bat/.sh  # Command-line app wrappers
├── bin/
│   ├── MitkWorkbench.exe          # Executables
│   ├── MitkFileConverter.exe
│   ├── MitkWorkbench.provisioning # Provisioning files
│   ├── *.dll / *.so               # MITK modules + resolved dependencies
│   ├── mitk.ico, mitk.bmp         # Icons (Windows only)
│   ├── qt.conf                    # Qt configuration
│   ├── CppMicroServices.dll/.so   # CppMicroServices library
│   ├── MitkCore/                  # Autoload module subdirectories
│   │   └── MitkPreloadPython.dll/.so
│   ├── plugins/                   # CTK/BlueBerry and Qt plugins
│   │   ├── org_mitk_gui_qt_*.dll/.so
│   │   ├── org_blueberry_*.dll/.so
│   │   ├── imageformats/          # Qt plugin subdirectories
│   │   └── platforms/
│   ├── resources/                  # Qt WebEngine resources
│   │   └── *.pak
│   └── translations/              # Qt translations
└── python/                         # Python distribution (if enabled)
    ├── bin/ or python.exe
    └── lib/
        └── python3.X/
            └── site-packages/
                └── mitk/           # MITK Python bindings
```

### macOS

```
<install-root>/
└── MitkWorkbench.app/
    └── Contents/
        ├── MacOS/
        │   ├── MitkWorkbench       # Main executable
        │   ├── *.dylib             # Shared libraries
        │   └── plugins/            # CTK/BlueBerry plugins
        ├── Frameworks/
        │   ├── Qt*.framework/      # Qt frameworks
        │   └── Python.framework/   # Python (converted from directory)
        │       └── Versions/
        │           ├── A/
        │           │   ├── Python  # Python shared library
        │           │   ├── bin/
        │           │   ├── lib/
        │           │   ├── Headers/
        │           │   └── Resources/
        │           │       └── Info.plist
        │           └── Current -> A
        ├── PlugIns/                # Qt plugins
        └── Resources/
            └── icon.icns
```

## External Project Special Cases

### Boost DLL Relocation (Windows)

Boost builds its DLLs into `lib/`. A post-install step (`Boost-post_install-WIN32.cmake`) moves them to `bin/` so they are found alongside other DLLs:

```cmake
file(GLOB boost_dlls boost_*.dll)
execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ../bin)
foreach(boost_dll ${boost_dlls})
  execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${boost_dll} ../bin)
  execute_process(COMMAND ${CMAKE_COMMAND} -E remove ${boost_dll})
endforeach()
```

### Boost macOS RPATH

Boost does not follow the common practice of using `@rpath` for inter-library references on macOS. A post-install step (`Boost-post_install-APPLE.cmake`) fixes this with `install_name_tool`:

```cmake
foreach(boost_dylib ${boost_dylibs})
  execute_process(COMMAND install_name_tool -id @rpath/${boost_dylib} ${boost_dylib})
  foreach(other_boost_dylib ${boost_dylibs})
    execute_process(COMMAND install_name_tool -change ${other_boost_dylib}
      @rpath/${other_boost_dylib} ${boost_dylib})
  endforeach()
endforeach()
```

### External CMake Project Installation

`mitkFunctionInstallExternalCMakeProject()` allows running an external project's install script directly:

```cmake
function(mitkFunctionInstallExternalCMakeProject ep_name)
  ExternalProject_Get_Property(${ep_name} binary_dir)
  install(SCRIPT ${binary_dir}/cmake_install.cmake)
endfunction()
```

This is used for external projects that need their own install logic beyond what the dependency resolver handles.

## macOS-Specific Details

### App Bundles

On macOS, BlueBerry applications are built as `.app` bundles (`MACOSX_BUNDLE`). The bundle properties (name, identifier, version, icon) are set in `mitkFunctionCreateBlueBerryApplication()`.

### Python Framework Conversion

For code signing, macOS requires frameworks to follow a specific directory structure. The `FixMacOSInstaller.cmake` script converts the flat Python directory into `Python.framework` with the standard versioned layout:

```
Python.framework/
├── Versions/
│   ├── A/
│   │   ├── Python              # Renamed from lib/libpython3.X.dylib
│   │   ├── bin/
│   │   ├── lib/
│   │   │   └── libpython3.X.dylib -> ../Python  # Symlink back
│   │   ├── Headers/            # Moved from include/python3.X
│   │   ├── share/
│   │   └── Resources/
│   │       └── Info.plist
│   └── Current -> A
├── Headers -> Versions/Current/Headers
├── Python -> Versions/Current/Python
└── Resources -> Versions/Current/Resources
```

### Code Signing and Entitlements

All app bundles are signed with `codesign` using the identity specified by `MITK_CODESIGN_IDENTITY`. The entitlements file (`CMake/entitlements.plist`) grants:

- `com.apple.security.cs.disable-library-validation` — allows loading unsigned or differently-signed shared libraries (needed for plugins and Python modules)
- `com.apple.security.cs.allow-jit` — allows JIT compilation (needed for some Python operations)

### BundleUtilities and `@loader_path`

The `FixMacOSInstaller.cmake` script also fixes a problem caused by `BundleUtilities`: it rewrites all library dependency paths to `@executable_path/../MacOS`, which works for executables but breaks when the Python interpreter in `Frameworks/Python.framework/Versions/A/bin` tries to load the `mitk` package. The fix rewrites these paths to use `@loader_path` instead.

## Provisioning Files

Provisioning files tell the BlueBerry framework which plugins to load at startup. They use a simple text format:

```
READ file:///path/to/other.provisioning
START file:///path/to/plugin.so
```

`mitkFunctionCreateProvisioningFile()` creates two variants:

1. **Build-time** (`<AppName>.provisioning`): Uses absolute `file:///` URLs pointing to the build tree
2. **Install-time** (`<AppName>.provisioning.install`): Uses `@EXECUTABLE_DIR` placeholders that resolve relative to the executable at runtime

The install-time variant is installed to `bin/` and renamed to drop the `.install` suffix. For example:

```
START file://@EXECUTABLE_DIR/plugins/liborg_mitk_gui_qt_common.so
```

## CppMicroServices Resource Embedding {#CppMicroServicesResourcesSection}

CppMicroServices allows modules to embed resources (XML descriptors, icons, etc.) as ZIP archives inside the shared library file itself. This mechanism has a direct impact on packaging because it constrains whether binaries can be stripped.

### Two Embedding Modes: APPEND vs LINK

The `usFunctionEmbedResources()` function in `Modules/CppMicroServices/cmake/usFunctionEmbedResources.cmake` supports two modes:

**APPEND mode** (default on Windows and Linux):

The ZIP archive is literally appended as raw bytes **after** the end of the ELF/PE binary structure:

```cmake
add_custom_command(
  TARGET ${US_RESOURCE_TARGET}
  POST_BUILD
  COMMAND ${resource_compiler} --append $<TARGET_FILE:${US_RESOURCE_TARGET}> ${_zip_archive}
  ...)
```

The `usResourceCompiler` opens the shared library in binary append mode and writes the ZIP at the end. At runtime, CppMicroServices uses the miniz library to search for the ZIP central directory signature from the end of the file — standard ZIP behavior that works even when the ZIP is concatenated after other data.

**LINK mode** (default on macOS):

The ZIP archive is compiled into a proper object file section using platform-specific linker techniques:

| Platform | Technique | Section |
|---|---|---|
| macOS | `ld -r -sectcreate __TEXT us_resources <zip> stub.o` | `__TEXT/us_resources` |
| Linux | `ld -r -b binary` + `objcopy --rename-section .data=.rodata` | `.rodata` |
| Windows | Windows Resource Compiler (`.rc` file with `US_RESOURCE` type) | PE resource section |

The resulting `.o` or `.rc` file is linked into the target as a regular object file, making the ZIP part of the binary's official section table.

### Default Mode Selection

`usFunctionCheckResourceLinking.cmake` determines the defaults:

- **macOS**: LINK mode (required because `codesign` validates the binary structure and rejects appended data)
- **Windows**: APPEND mode (even though LINK is available via RC compiler)
- **Linux**: APPEND mode (even though LINK is available via `ld -r -b binary`)

The check sets `US_DEFAULT_RESOURCE_MODE` and `US_RESOURCE_LINKING_AVAILABLE` as cache variables.

### Why Stripping Breaks APPEND Mode

When `strip` processes an ELF binary, it rewrites the file based on the ELF headers' recorded sections. Everything beyond the official ELF structure — including the appended ZIP archive — is discarded. The stripped binary loads fine, but all CppMicroServices resources are gone, causing runtime failures when modules try to access their embedded resources.

LINK mode survives stripping because the ZIP is stored inside a proper ELF section (`.rodata`). The `strip` command preserves allocated sections like `.rodata` since they are needed at runtime.

### Current Impact and Future Improvement

Because MITK uses APPEND mode on Windows and Linux (the default), `CPACK_STRIP_FILES` must be set to `OFF` in `mitkSetupCPack.cmake`. This means **all** binaries in the package are unstripped, resulting in significantly larger packages.

A future improvement would be to switch CppMicroServices resource embedding to **LINK mode on all platforms**. This would:

1. Allow re-enabling `CPACK_STRIP_FILES ON` to strip debug symbols from all binaries
2. Eliminate the fragile dependency on file-append ordering
3. Align all platforms with the same embedding strategy

The trade-off documented in the CppMicroServices source is that LINK mode "may result in slower module initialization and bigger object files." In practice, the initialization overhead is negligible, and the object file size increase is far outweighed by the package size savings from being able to strip debug symbols.

To switch, set `US_DEFAULT_RESOURCE_MODE` to `"LINK"` for all platforms in `usFunctionCheckResourceLinking.cmake`, or pass `LINK` explicitly in each `usFunctionEmbedResources()` call.

## Changes from Legacy System

The current install system replaced several legacy approaches:

| Legacy | Current | Why |
|---|---|---|
| `BundleUtilities` / `fixup_bundle()` | `install(RUNTIME_DEPENDENCY_SET)` | `BundleUtilities` is deprecated in CMake 3.23+; the new approach is declarative and handles edge cases better |
| Central install loops walking `MITK_MODULE_TARGETS` | Per-target `install()` at creation site | Each target knows its own install destination; no need for a central loop to classify targets |
| `mitkFunctionInstallAutoLoadModules()` | Inline install in `mitk_create_module()` | Autoload install is now part of module creation; the legacy function has been removed |
| Manual DLL copying scripts | `RUNTIME_DEPENDENCY_SET` with `DIRECTORIES` | CMake resolves transitive dependencies automatically |
| Per-target Qt deployment calls | Centralized loop in `mitkInstallRules.cmake` | Qt deployment must run after all dependencies are in place |

## Known Issues and Future Work

- **CPACK_STRIP_FILES is OFF globally**: Packages are larger than necessary because all binaries retain debug symbols. The root cause is CppMicroServices APPEND-mode resource embedding. Switching to LINK mode (see \ref CppMicroServicesResourcesSection) would allow re-enabling stripping.

- **macOS autoload modules in Python**: The `FixMacOSInstaller.cmake` `@loader_path` fix does not cover autoload modules. Importing `mitk` in a standalone Python interpreter on macOS will not load autoload modules. Running Python as a subprocess of an MITK application works correctly.

- **Qt WebEngine `qt.conf` on Linux**: The Qt WebEngine deployment hook writes `qt.conf` with an absolute staging path. A post-install step overwrites it, but this is fragile and depends on the hook's behavior not changing across Qt versions.

- **Qt 6.10 plugin filtering**: Qt 6.10 introduced fine-grained plugin filtering (`INCLUDE_PLUGINS`, `EXCLUDE_PLUGINS`, `INCLUDE_PLUGIN_TYPES`, `EXCLUDE_PLUGIN_TYPES`). MITK does not use these yet. They could reduce package size by excluding unnecessary Qt plugins.

- **mitkFunctionInstallThirdPartyCTKPlugins MACOSX_BUNDLE_NAMES**: The function supports multiple bundles via `MACOSX_BUNDLE_NAMES`, but this is rarely used and adds complexity.

## File Reference

| File | Purpose |
|---|---|
| `CMake/mitkInstallRules.cmake` | Central install orchestration: CppMicroServices, Python, dependency resolution, Qt deployment loop |
| `CMake/mitkFunctionDeployQt.cmake` | `mitkFunctionDeployQt()` — Qt plugin, qt.conf, and WebEngine deployment |
| `CMake/mitkFunctionCreateModule.cmake` | `mitk_create_module()` — module install rules and RPATH overrides |
| `CMake/mitkMacroCreateExecutable.cmake` | `mitk_create_executable()` — executable install rules and wrapper scripts |
| `CMake/mitkFunctionCreateCommandLineApp.cmake` | `mitkFunctionCreateCommandLineApp()` — wraps `mitk_create_executable()` with MitkCommandLine dependency |
| `CMake/mitkFunctionCreateBlueBerryApplication.cmake` | `mitkFunctionCreateBlueBerryApplication()` — BB app creation, Qt deployment flag, provisioning |
| `CMake/mitkFunctionCreatePlugin.cmake` | `mitkFunctionCreatePlugin()` — plugin install rules and RPATH |
| `CMake/mitkFunctionInstallCTKPlugin.cmake` | `mitkFunctionInstallCTKPlugin()` — CTK plugin install with dependency set registration |
| `CMake/mitkFunctionInstallThirdPartyCTKPlugins.cmake` | `mitkFunctionInstallThirdPartyCTKPlugins()` — installs third-party CTK plugins for an application |
| `CMake/mitkFunctionInstallExternalCMakeProject.cmake` | `mitkFunctionInstallExternalCMakeProject()` — runs external project install scripts |
| `CMake/mitkFunctionGetLibrarySearchPaths.cmake` | `mitkFunctionGetLibrarySearchPaths()` — collects library search dirs for build and install time |
| `CMake/mitkFunctionAddLibrarySearchPaths.cmake` | `mitkFunctionAddLibrarySearchPaths()` — registers external project library paths |
| `CMake/mitkFunctionCreateProvisioningFile.cmake` | `mitkFunctionCreateProvisioningFile()` — creates build-time and install-time provisioning files |
| `CMake/mitkSetupCPack.cmake` | CPack generator selection, versioning, NSIS settings, strip policy |
| `CMake/FixMacOSInstaller.cmake.in` | macOS post-CPack script: Python framework conversion, `@loader_path` fix, code signing |
| `CMake/entitlements.plist` | macOS code signing entitlements |
| `CMake/RunInstalledApp.bat` | Windows wrapper for regular executables |
| `CMake/RunInstalledWin32App.bat` | Windows wrapper for BlueBerry (GUI) applications |
| `CMake/RunInstalledCmdLineApp.bat` | Windows wrapper for command-line apps |
| `CMake/RunInstalledApp.sh` | Linux wrapper for regular executables and BlueBerry apps |
| `CMake/RunInstalledCmdLineApp.sh` | Linux wrapper for command-line apps |
| `CMakeExternals/Boost-post_install-WIN32.cmake` | Moves Boost DLLs from `lib/` to `bin/` |
| `CMakeExternals/Boost-post_install-APPLE.cmake` | Fixes Boost inter-library references to use `@rpath` |
| `Modules/CppMicroServices/cmake/usFunctionEmbedResources.cmake` | APPEND and LINK mode resource embedding into shared libraries |
| `Modules/CppMicroServices/cmake/usFunctionAddResources.cmake` | Creates ZIP archives from resource files for embedding |
| `Modules/CppMicroServices/cmake/usFunctionCheckResourceLinking.cmake` | Platform capability detection for LINK mode; sets `US_DEFAULT_RESOURCE_MODE` |
| `SuperBuild.cmake` | SuperBuild RPATH configuration for external projects |
