---
paths:
  - '**/*.dox'
  - Documentation/**/*.md
  - Modules/**/documentation/*.md
---

# Manual prose

Doxygen comment style (block `/** ... */`, never `///`) is enforced
project-wide and lives in the root `CLAUDE.md`. This file covers
the manual prose surface.

## Where the prose lives

- `.dox` (Doxygen) and `.md` (Markdown) files under
  `Documentation/Doxygen/` - the user manual in `2-UserManual/`,
  the developer manual in `3-DeveloperManual/`.
- Per-module documentation under
  `Modules/<Module>/documentation/`.
- Plugin user manuals under
  `Plugins/<plugin>/documentation/UserManual/`, compiled into the Qt
  Help shipped with the application.
- The build target `doc` produces the HTML output.

## Images

The aliases are defined in `Documentation/doxygen.conf.in` and
`Documentation/doxygen_plugin_manual.conf.in`; the contributor-facing
description is `3-DeveloperManual/Starting/GettingToKnow/DocumentationGuide.dox`.

- Never use `\image` directly; use one of the macros, which also emit
  the pdf variant.
- `\imageMacro{file, "caption", cm}` sizes only the pdf; HTML shows the
  image at native size.
- For screenshots use `\imageMacroEx{file, "caption", cm, px}`: the
  fourth argument is the HTML width (e.g. `360px`). Existing pages use
  `10` cm with 320-640 px.
- Arguments are comma-separated: escape a comma in the caption as `\,`,
  or the macro silently breaks ("unknown command" warning).
- Doxygen resolves image names globally: prefix them with the plugin or
  view name (`QmitkMyView_Overview.png`), never a generic name.
- After editing, build the `doc` target and compare the warning count
  with the baseline.

## CLI-application registration

When a module ships a new CLI application:

1. Add `Modules/<Module>/documentation/Mitk<App>.md` describing it.
2. Add a `\subpage` entry in
   `Documentation/Doxygen/2-UserManual/MITKCmdAppsPage.dox` so the
   page appears in the user-manual index.
