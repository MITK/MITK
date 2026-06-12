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
- The build target `doc` produces the HTML output.

## CLI-application registration

When a module ships a new CLI application:

1. Add `Modules/<Module>/documentation/Mitk<App>.md` describing it.
2. Add a `\subpage` entry in
   `Documentation/Doxygen/2-UserManual/MITKCmdAppsPage.dox` so the
   page appears in the user-manual index.
