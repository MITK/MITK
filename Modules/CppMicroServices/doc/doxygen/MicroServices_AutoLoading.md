Auto Loading Modules    {#MicroServices_AutoLoading}
====================

Auto-loading of modules is a feature of CppMicroServices to manage the loading
of modules which would normally not be loaded at runtime because of missing link-time
dependencies.

In MITK, auto-loading is always enabled and cannot be disabled.

How It Works
------------

For each module being loaded, the following steps are taken:

 - If the module provides an activator, its ModuleActivator::Load() method is called.
 - The auto-load paths returned from ModuleSettings::GetAutoLoadPaths() are processed.
 - For each auto-load path, all modules in that path with the currently loaded module's
   auto-load directory appended are explicitly loaded.

See the ModuleSettings class for details about auto-load paths. The auto-load directory of
a module defaults to the module's library name, but can be customized using a `manifest.json`
file (see \ref MicroServices_ModuleProperties). For executables, the auto-load directory
defaults to the special value `main`. This allows third-party modules to be auto-loaded
during application start-up, without having to reference a special auto-load directory.

Environment Variables
---------------------

The following environment variables influence the runtime behavior:

 - *US_DISABLE_AUTOLOADING* If set, auto-loading of modules is disabled.
 - *US_AUTOLOAD_PATHS* A `:` (Unix) or `;` (Windows) separated list of paths from which modules
   should be auto-loaded.
