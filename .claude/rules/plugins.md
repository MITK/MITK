---
paths:
  - Plugins/**/*.h
  - Plugins/**/*.cpp
  - Plugins/**/*.cmake
  - Plugins/**/*.xml
  - Plugins/**/CMakeLists.txt
---

# BlueBerry plugins

Plugins extend applications via the BlueBerry framework (an
Eclipse-style OSGi model). They live under `Plugins/`.

## Layout

```
org.mitk.gui.qt.myview/
├── CMakeLists.txt
├── files.cmake           # SRC_CPP_FILES, INTERNAL_CPP_FILES,
│                         # MOC_H_FILES, UI_FILES,
│                         # CACHED_RESOURCE_FILES, QRC_FILES
├── manifest_headers.cmake
├── plugin.xml            # extension-point declarations
├── target_libraries.cmake
├── src/
│   └── internal/         # non-exported internals;
│                         # PluginActivator lives here
├── resources/            # Qt resources and icons (plural;
│                         # distinct from a module's resource/)
└── documentation/
    └── UserManual/       # feeds Qt Help (.qch) generation
```

## CMake entry point

```cmake
mitk_create_plugin(
  EXPORT_DIRECTIVE MY_PLUGIN_EXPORT
  MODULE_DEPENDS MitkQtWidgets
)
```

## `files.cmake`

Source lists live in `files.cmake`. Plugin-specific variables:

| Variable | Purpose |
| --- | --- |
| `SRC_CPP_FILES` | Exported plugin sources |
| `INTERNAL_CPP_FILES` | Non-exported internals (under `src/internal/`) |
| `MOC_H_FILES` | Qt-MOC-processed headers |
| `UI_FILES` | Qt Designer `.ui` files |
| `CACHED_RESOURCE_FILES` | Plugin manifest resources |
| `QRC_FILES` | Qt resource collection files |

## Conventions

- Class and file names use the `Qmitk` prefix for everything visible
  in Qt (`QmitkMyView.h`).
- `PluginActivator` and other non-exported types belong in
  `src/internal/`.
- `plugin.xml` is the discovery surface; keep it in sync with the
  exposed extension points.
- User-facing documentation lives in `documentation/UserManual/` and
  is compiled into the Qt Help file shipped with the application.
