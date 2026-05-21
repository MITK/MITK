# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## IMPORTANT General Rules

- When working on complex tasks, think out loud, revisit earlier conclusions, step back when needed, and keep the broader context in mind.
- Making educated guesses is fine once or twice, but if they do not hold up, verify them empirically instead: add temporary logging or instrumentation to observe what is actually happening during execution.

## What is MITK?

MITK (Medical Imaging Interaction Toolkit) is an open-source C++ framework for developing interactive medical image processing software. It combines ITK (Insight Toolkit) and VTK (Visualization Toolkit) with an application framework built on Qt and BlueBerry plugins.

## Build Commands

MITK uses CMake with a superbuild pattern that downloads and builds all dependencies first.

### Prerequisites

- **CMake**: 3.31+ on Windows, 3.28+ on Linux/macOS
- **C++ standard**: C++20
- **Qt**: 6.10 minimum
- **Debug + Python**: Debug builds are incompatible with `MITK_USE_Python3=ON`

### Initial Build (superbuild)

Note that the initial superbuild also includes the MITK build.

**Windows (Visual Studio):**
```bash
cmake -S . -B ../MITK-superbuild -G "Visual Studio 17 2022"
cmake --build ../MITK-superbuild --config Release -- -m
```

**Linux/macOS (Ninja):**
```bash
cmake -S . -B ../MITK-superbuild -G "Ninja" -D CMAKE_BUILD_TYPE=Release
cmake --build ../MITK-superbuild
```

### Key CMake Variables on superbuild level

- `MITK_BUILD_CONFIGURATION`: Selects a predefined feature set. Available values are listed in `CMake/BuildConfigurations/`. The most common configurations are:

- `WorkbenchRelease` (default)
- `WorkbenchReleaseNoPython` (without Python, enabling Debug build configuration)
- `All` (build everything, used by our CI)

Other commonly used options:

| Option | Default | Purpose |
| --- | --- | --- |
| `MITK_EXTENSION_DIRS` | — | Additional dirs for downstream/extension projects |

### Build After Initial Configuration

Once the superbuild completes, the actual MITK build is in `MITK-superbuild/MITK-build/`:
```bash
cmake --build ../MITK-superbuild/MITK-build --config Release
```

### Running Tests

```bash
cd ../MITK-superbuild/MITK-build
ctest -C Release                           # Run all tests
ctest -C Release -R mitkImage              # Run tests matching pattern
ctest -C Release -N                        # List tests without running
```

### Building Documentation

Requires Doxygen at configure time (`DOXYGEN_FOUND`). Developer documentation source lives in `Documentation/Doxygen/3-DeveloperManual/` (`.dox` and `.md` files).

```bash
cmake --build ../MITK-superbuild/MITK-build --target doc
```

### Running Applications (Windows)

After building, launch via the generated start scripts which sets up `PATH`:

- `MITK-superbuild/MITK-build/bin/startMitkWorkbench_release.bat`

On Linux and macOS, applications in the build tree do not need or use start scripts.

## Architecture Overview

### Two-Phase Build System

1. **superbuild phase** (`SuperBuild.cmake`): Downloads and builds most external dependencies (ITK, VTK, CTK, DCMTK, Boost, Python3, pybind11, and more). External dependencies are managed in `CMakeExternals/` and listed in `CMakeExternals/ExternalProjectList.cmake`.
2. **MITK build phase**: Builds MITK modules, plugins, Python bindings, and applications.

### Module System

Modules are the core functional units in `Modules/`. They must be listed in dependency order in `Modules/ModuleList.cmake` (> 60 modules). Key tiers:

- **Foundation**: `Log`, `CppMicroServices`, `Core` (base data structures: Image, Surface, PointSet, BaseData, BaseGeometry)
- **Intermediate**: `AlgorithmsExt`, `DICOM`, `Multilabel`, `Segmentation`
- **UI**: `QtWidgets`, `SegmentationUI` (standalone applications live in `Applications/`, not here)

**Module structure:**
```
ModuleName/
├── CMakeLists.txt       # calls mitk_create_module()
├── files.cmake          # lists CPP_FILES, H_FILES, RESOURCE_FILES, UI_FILES
├── include/             # public headers
├── src/                 # implementation
├── resource/            # embedded resources (XML, JSON, SVG) declared via RESOURCE_FILES
├── autoload/<Subdir>/   # sub-modules that auto-load with a parent module
├── cmdapps/             # command-line applications shipped with the module
├── test/ or Testing/    # unit tests (both naming conventions exist in the codebase, prefer test/)
└── TestingHelper/       # helper sub-module for downstream test use (rarely used)
```

**Creating a module** (`CMakeLists.txt`):
```cmake
mitk_create_module(
  DEPENDS PUBLIC MitkCore
  PACKAGE_DEPENDS PUBLIC ITK|IOImageBase VTK|RenderingOpenGL2
)
```

### Plugin System (BlueBerry Framework)

Plugins extend applications without modifying core code. Located in `Plugins/`, they use an Eclipse-based OSGi model.

**Plugin structure:**
```
org.mitk.gui.qt.myview/
├── CMakeLists.txt
├── files.cmake           # SRC_CPP_FILES, INTERNAL_CPP_FILES, MOC_H_FILES, UI_FILES,
│                         # CACHED_RESOURCE_FILES, QRC_FILES, MOC_H_FILES
├── manifest_headers.cmake
├── plugin.xml            # defines extension points
├── target_libraries.cmake
├── src/
│   └── internal/         # non-exported internals; PluginActivator lives here
├── resources/            # Qt resources and icons (plural, distinct from module's resource/)
└── documentation/
    └── UserManual/       # feeds Qt Help (.qch) generation
```

**Creating a plugin:**
```cmake
mitk_create_plugin(
  EXPORT_DIRECTIVE MY_PLUGIN_EXPORT
  MODULE_DEPENDS MitkQtWidgets
)
```

### Data Model Hierarchy

All data objects inherit from `BaseData`:
- **Image** (via SlicedData): Multi-channel, multi-timepoint image data
- **Surface**: VTK polydata wrapper
- **PointSet**: Collection of points with properties
- **BaseGeometry**: Spatial/temporal metadata

### Python Bindings

Located in `Wrapping/Python/mitk/`. Uses pybind11. Key files:
- `Module.cpp`: Entry point with `PYBIND11_MODULE`
- `Image.cpp`, `Geometries.cpp`, `Points.cpp`: Type bindings
- `SmartPointer.h`: ITK smart pointer integration

Python wheel builds require `MITK_BUILD_CONFIGURATION=PythonWheel` (headless, no Qt/BlueBerry) and use `Wrapping/Python/wheel/build_wheel.py` with a platform delocator.

## Testing Patterns

Tests use CppUnit with MITK macros:
```cpp
class mitkMyTestSuite : public mitk::TestFixture {
  CPPUNIT_TEST_SUITE(mitkMyTestSuite);
  MITK_TEST(MyTest_Success);
  CPPUNIT_TEST_SUITE_END();

  void MyTest_Success() {
    CPPUNIT_ASSERT(condition);
  }
};
MITK_TEST_SUITE_REGISTRATION(mitkMyTest)
```

In test `CMakeLists.txt`:
```cmake
mitk_create_module_tests()
```

**Additional test patterns:**

- **Data-driven tests**: Use `mitkAddCustomModuleTest()` and pass file paths from `${MITK_DATA_DIR}` as argv. Most non-trivial tests use this. `MITK_DATA_DIR` points to the external `MITK-Data` repository cloned by the superbuild.
- **Rendering tests**: `mitkAddCustomModuleRenderingTest()` — compares rendered output against reference PNGs in `MITK_DATA_DIR/RenderingTestData/ReferenceScreenshots/`. These run serially.
- **Runtime skip**: Return exit code 77 (`SKIP_RETURN_CODE 77`) for tests that detect missing preconditions.

## Key Dependencies

- **ITK**: Image processing algorithms
- **VTK**: 3D rendering and visualization
- **Qt6**: GUI framework (not part of the superbuild)
- **CTK**: Extended Qt widgets and plugin support
- **Boost**: Utilities (try to use C++ Standard Library instead if possible)
- **pybind11**: Python bindings
- **MITK-Data**: External test data repository (cloned by superbuild, referenced as `${MITK_DATA_DIR}`)

## Code Conventions

- MITK classes deriving from ITK use `itk::SmartPointer<>` for memory management.
- Use the appropriate class macro (defined in `Modules/Core/include/mitkCommon.h`):
  - `mitkClassMacro(Class, Super)` — standard; parent already overrides `GetNameOfClass`
  - `mitkClassMacroItkParent(Class, Super)` — parent is a raw ITK class
  - `mitkClassMacroNoParent(Class)` — root classes with no superclass
  - `mitkNewMacro1Param` .. `mitkNewMacro6Param` — `New()` factories for constructors with 1-6 arguments
  - `mitkCloneMacro(Class)` — complements `itkCloneMacro(Class)` and implements `InternalClone()` for `BaseData::Clone()`
- **Copyright header**: Every new C++ and Python file must begin with the project copyright header. Template: `Templates/copyright_header.txt`.
- **File naming**: Core and algorithm files use the `mitk` prefix (e.g., `mitkImage.h`); Qt widget files use the `Qmitk` prefix (e.g., `QmitkRenderWindow.h`).
- Follow the MITK Style Guide: `Documentation/Doxygen/3-DeveloperManual/Starting/GettingToKnow/StyleGuideAndNotes.dox`. See also `CONTRIBUTING.md` at the repo root for the contribution workflow. Signing off commits is only required for external contributors.

## Commit messages

Great commit message are a priceless source for meta information about code, context and decisions that would be otherwise lost. Put effort into commit messages but keep them as short as possible and as long as necessary.

Follow the seven rules of great commit messages (https://cbea.ms/git-commit/):
- Separate subject from body with a blank line
- Limit the subject line to 50 characters
- Capitalize the subject line
- Do not end the subject line with a period
- Use the imperative mood in the subject line
- Wrap the body at 72 characters
- Use the body to explain what and why vs. how

This is an example commit message:

```
Summarize changes in around 50 characters or less

More detailed explanatory text, if necessary. Wrap it to about 72
characters or so. In some contexts, the first line is treated as the
subject of the commit and the rest of the text as the body. The
blank line separating the summary from the body is critical (unless
you omit the body entirely); various tools like `log`, `shortlog`
and `rebase` can get confused if you run the two together.

Explain the problem that this commit is solving. Focus on why you
are making this change as opposed to how (the code explains that).
Are there side effects or other unintuitive consequences of this
change? Here's the place to explain them.

Further paragraphs come after blank lines.

 - Bullet points are okay, too

 - Typically a hyphen or asterisk is used for the bullet, preceded
   by a single space, with blank lines in between, but conventions
   vary here

If you use an issue tracker, put references to them at the bottom,
like this:

Resolves: #123
See also: #456, #789
```

## Important Directories

- `CMake/`: Build macros and utilities
- `CMakeExternals/`: External dependency definitions
- `CMake/BuildConfigurations/`: Predefined build configuration sets
- `Applications/`: Standalone apps (MitkWorkbench, FlowBench)
- `Documentation/`: Doxygen configuration and developer manual (`.dox` files)
- `Templates/`: File templates including the mandatory copyright header
