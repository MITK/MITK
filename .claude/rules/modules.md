---
paths:
  - Modules/**/*.h
  - Modules/**/*.cpp
  - Modules/**/*.cmake
  - Modules/**/CMakeLists.txt
---

# Modules

Modules are the core functional units. Register new ones in the
dependency order in `Modules/ModuleList.cmake`.

## Layout

```
ModuleName/
├── CMakeLists.txt       # calls mitk_create_module()
├── files.cmake          # CPP_FILES, H_FILES, RESOURCE_FILES, ...
├── include/             # public headers
├── src/                 # implementation
├── resource/            # embedded resources (XML, JSON, SVG)
├── autoload/<Subdir>/   # sub-modules auto-loaded with the parent
├── cmdapps/             # CLI applications shipped by the module
├── test/ or Testing/    # unit tests (prefer test/ for new code)
└── TestingHelper/       # helper sub-module for downstream tests
```

## CMake entry point

```cmake
mitk_create_module(
  DEPENDS PUBLIC MitkCore
  PACKAGE_DEPENDS PUBLIC ITK|IOImageBase VTK|RenderingOpenGL2
)
```

## `files.cmake`

Source lists live in `files.cmake`, not in `CMakeLists.txt`. Common
variables:

| Variable | Purpose |
| --- | --- |
| `CPP_FILES` | Implementation files |
| `H_FILES` | Public headers |
| `MOC_H_FILES` | Qt-MOC-processed headers |
| `UI_FILES` | Qt Designer `.ui` files |
| `RESOURCE_FILES` | Files embedded into the module (`resource/`) |
| `MODULE_TESTS`, `MODULE_RENDERING_TESTS`, `MODULE_IMAGE_TESTS` | Test sources (see "Tests" below) |

## File naming

Use the `mitk` prefix for headers and sources of core / algorithm
classes (`mitkImage.h`, `mitkImage.cpp`). The `Qmitk` prefix is
reserved for Qt widget files.

## Class macros

Defined in `Modules/Core/include/mitkCommon.h`:

| Macro | Use when |
| --- | --- |
| `mitkClassMacro(Class, Super)` | Standard; parent already overrides `GetNameOfClass`. |
| `mitkClassMacroItkParent(Class, Super)` | Parent is a raw ITK class. |
| `mitkClassMacroNoParent(Class)` | Root class with no superclass. |
| `mitkNewMacro1Param` ... `mitkNewMacro6Param` | `New()` factories for 1- to 6-argument constructors. |
| `mitkCloneMacro(Class)` | Implements `InternalClone()` for `BaseData::Clone()`. |

Dedicated exception types are declared with `mitkExceptionClassMacro`
(defined in `Modules/Core/include/mitkExceptionMacro.h`; see also
the usage example in `mitkException.h`). Prefer a distinct
exception subclass per error category so callers differentiate by
type, not by parsing messages.

## Tests

CppUnit-based, registered through:

```cmake
MITK_CREATE_MODULE_TESTS()
```

The macro is declared as `MITK_CREATE_MODULE_TESTS` in
`CMake/mitkMacroCreateModuleTests.cmake`. CMake is
case-insensitive, so the occasional lower-case
`mitk_create_module_tests()` callsite works, but upper-case is the
prevailing convention in this codebase.

Test class skeleton:

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

Beyond `MODULE_TESTS`, `files.cmake` also supports:

- `MODULE_RENDERING_TESTS` - rendering tests; run serially, use
  `xvfb` on Linux. Compared against reference PNGs in
  `${MITK_DATA_DIR}/RenderingTestData/ReferenceScreenshots/`.
- `MODULE_IMAGE_TESTS` - executed once per test image.

Data-driven tests use `mitkAddCustomModuleTest()` (or
`mitkAddCustomModuleRenderingTest()` for rendering) and pass file
paths from `${MITK_DATA_DIR}` as argv. Return exit code 77 from a
test to signal a runtime skip.

Run tests through `ctest`, never the test-driver `.exe` directly.

## Copyright header

Every new C++ or Python file must start with the header at
`Templates/copyright_header.txt`.

## CLI applications

When a module adds a CLI app under `cmdapps/`:

1. Ship `Modules/<Module>/documentation/Mitk<App>.md` describing it.
2. `\subpage` it from
   `Documentation/Doxygen/2-UserManual/MITKCmdAppsPage.dox` so it
   appears in the user-manual index.
