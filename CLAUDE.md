# CLAUDE.md

Guidance for Claude Code working in this repository. Specialised,
file-type- or subtree-specific rules live in `.claude/rules/` and
activate automatically when matching files are touched.

## Working style

- When asked for analysis, troubleshooting, design opinions, or
  similar: do not start implementing. Propose options and a
  recommendation, then wait for the go-ahead.
- Favour simplicity. Do not overengineer, do not add abstractions
  without a concrete need, work toward reducing technical debt.
- Be critical and direct, but stay constructive. Surface
  disagreements early rather than going along with a flawed plan.
- Educated guesses are fine once or twice. When they do not hold
  up empirically, switch to observation: add temporary logging or
  instrumentation before guessing again.
- C++ and CMake files must not use tabs for indentation and must
  end with a newline.
- Ask before committing anything.
- Never push without an explicit go-ahead for that specific push.

## What is MITK?

MITK (Medical Imaging Interaction Toolkit) is an open-source C++
framework for developing interactive medical image processing
software. It combines ITK (image processing) and VTK (visualization)
with an application framework built on Qt and BlueBerry plugins.
Maintained by the German Cancer Research Center (DKFZ);
BSD-3-Clause licensed.

### Prerequisites

| | Required |
| --- | --- |
| CMake | 3.31+ (Windows) / 3.28+ (Linux, macOS) |
| C++ | C++20 |
| Qt | 6.10+ |

Debug builds are incompatible with `MITK_USE_Python3=ON`.

## Build

MITK uses a CMake superbuild that downloads and builds all
dependencies, then builds MITK itself.

**Windows (Visual Studio):**
```bash
cmake -S . -B ../MITK-superbuild -G "Visual Studio 17 2022"
cmake --build ../MITK-superbuild --config Release -- -m
```

**Linux / macOS (Ninja):**
```bash
cmake -S . -B ../MITK-superbuild -G "Ninja" -D CMAKE_BUILD_TYPE=Release
cmake --build ../MITK-superbuild
```

Subsequent rebuilds of just MITK happen in the inner build tree (the
`-- -m` is an MSBuild flag that parallelises across targets; omit it when
building with Ninja):
```bash
cmake --build ../MITK-superbuild/MITK-build --config Release -- -m
```

### Build configurations

`MITK_BUILD_CONFIGURATION` selects a predefined feature set. The
full list lives in `CMake/BuildConfigurations/`. Commonly used:

| Value | Purpose |
| --- | --- |
| `WorkbenchRelease` | Default; full Workbench with Python |
| `WorkbenchReleaseNoPython` | Required for Debug builds |
| `All` | Everything, used by CI |
| `PythonWheel` | Headless build, used by `Wrapping/Python/wheel/build_wheel.py`; not for general development |

### Python versions

`MITK_Python3_VERSION` (default `3.12`) is the CPython embedded in
MITK; `MITK_Python3_WHEEL_VERSIONS` (default: all supported series)
are the ones the `mitk` wheel is built for, honoured only by the
`PythonWheel` configuration. Both are validated against the
committed table in `CMakeExternals/Python3/Versions.cmake`. See
`Documentation/Doxygen/3-DeveloperManual/Concepts/PythonInMITK.md`.

### Tests

Always run tests through `ctest` from the inner build tree. Do not
invoke the test-driver executables directly - they expect arguments
that ctest knows how to supply.

```bash
cd ../MITK-superbuild/MITK-build
ctest -C Release                 # all tests
ctest -C Release -R mitkImage    # tests matching a pattern
ctest -C Release -N              # list without running
```

Many tests read input from the MITK-Data repository, which the
superbuild checks out automatically; `MITK_DATA_DIR` in the inner
build tree points to it. When a test cannot find its input, check
there first.

### Documentation build

```bash
cmake --build ../MITK-superbuild/MITK-build --target doc
```

### Running applications

On Windows, launch via the generated start script (it sets up
`PATH`):
`MITK-superbuild/MITK-build/bin/startMitkWorkbench_release.bat`.
On Linux and macOS the binaries in the build tree run directly.

## Architecture

Two-phase build: `SuperBuild.cmake` builds dependencies (ITK, VTK,
CTK, DCMTK, Boost, Python3, pybind11, ...); the inner MITK build
produces modules, plugins, Python bindings, and applications.

- `Modules/` - 60+ functional modules; build order in
  `Modules/ModuleList.cmake`.
- `Plugins/` - BlueBerry / OSGi plugins that extend applications.
- `Applications/` - standalone apps (MitkWorkbench, FlowBench).
- `Wrapping/Python/` - pybind11 bindings.
- `CMake/`, `CMakeExternals/` - build macros and external-dependency
  definitions.
- `Examples/` - tutorial and example code (first steps, BlueBerry
  examples).
- `Utilities/` - bundled third-party code; MITK conventions
  (copyright header, style) do not apply there.

### Data model

All data inherits from `BaseData`: `Image` (multi-channel,
multi-timepoint), `Surface` (VTK polydata), `PointSet`, with
`BaseGeometry` (and its concrete subclasses) carrying spatial /
temporal metadata.

The runtime model adds:

- **DataStorage** - central tree holding `DataNode`s, each wrapping
  a `BaseData` with properties.
- **Mapper** - converts data to VTK rendering commands.
- **CppMicroServices** - OSGi-style service registry used across
  several modules.

## C++ coding conventions

- Const-correctness aggressively: const methods, const arguments,
  const locals where it does not impair clarity.
- Avoid `const_cast`. Ask before using it.
- Use `this->` when calling member methods of the same class.
- Do not assume defaults. Base layers throw on unresolvable input;
  higher layers decide what to do.
- `itk::SmartPointer<>` is for storage and ownership. In function
  signatures, take the narrowest interface type that captures what
  the function actually uses (e.g. `IPropertyProvider*` instead of
  `BaseData*`); use raw pointers unless ownership transfers.
- Closed value sets (sex M/F, decay strategy, ...) belong in an
  `enum class` at the earliest interface boundary, never as
  `char`/`string`.
- Prefer current C++ idioms (`[[maybe_unused]]`, structured
  bindings, ...) over older workarounds.
- Reuse > reinvent. Default-private; promote to protected or public
  only when needed. Static helpers that do not touch instance state
  can live in an anonymous namespace inside the `.cpp`.
- When a test fails because the assertion is wrong, fix the test -
  do not patch productive code to make a flawed assertion pass.
- No decorative unicode in code or doc comments.
- Doxygen: block-style `/** ... */` comments only; never `///`.
- Every new C++ or Python file must start with the project
  copyright header from `Templates/copyright_header.txt`.

File naming: core / algorithm files use the `mitk` prefix
(`mitkImage.h`); Qt widget files use the `Qmitk` prefix
(`QmitkRenderWindow.h`).

## Documentation and comments

- A comment earns its place by explaining *why*: the non-obvious
  constraint, the subtle invariant, the trade-off the reader cannot
  see from the code itself. Restating *what* the code already says
  is noise. Stating *why* in terms of "why was it changed" belongs
  in the commit message, not in the code comment.
  Default to writing no comment.
- Doc comments on public APIs describe contract and intent (what
  callers rely on), not an implementation walkthrough.
- Do not reference artifacts that will not live in the repo - plans,
  design docs, chat threads, "see chapter X of the proposal". Once
  those artifacts move or disappear, the reference is dead noise.
  If something is load-bearing, inline it here or in the relevant
  source file.
- The same rules apply to commit messages, PR descriptions, and
  in-tree Markdown / `.dox` documentation: be helpful, on point,
  and add insight the code does not already convey.

## Commit messages

Follow the seven rules at https://cbea.ms/git-commit/:

- Separate subject from body with a blank line
- Limit the subject line to 50 characters
- Capitalize the subject; do not end it with a period
- Use imperative mood in the subject
- Wrap the body at 72 characters
- Use the body to explain what and why, not how

Put issue-tracker references at the bottom of the body:

```
Resolves: #123
See also: #456, #789
```

Sign-off (`git commit -s`) is required only for external
contributors. See `CONTRIBUTING.md`.

## Pointers

- Style guide:
  `Documentation/Doxygen/3-DeveloperManual/Starting/GettingToKnow/StyleGuideAndNotes.dox`
- Contribution workflow: `CONTRIBUTING.md`
- Copyright header: `Templates/copyright_header.txt`
- Build configurations: `CMake/BuildConfigurations/`
- External dependency definitions:
  `CMakeExternals/ExternalProjectList.cmake`
