/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <usModuleRegistry.h>
#include <usModule.h>

namespace py = pybind11;

void InitCppMicroServices(py::module_& m)
{
  m.def("get_loaded_modules", []() {
    std::vector<std::string> names;
    for (auto* module : us::ModuleRegistry::GetLoadedModules())
      names.push_back(module->GetName());
    return names;
  },
  R"(Return the names of all currently loaded CppMicroServices modules.

In the standalone wheel, this is the list of MITK modules whose auto-load
shared libraries have been loaded by the CppMicroServices runtime
(typically IO readers/writers and similar plug-in services).

Returns:
    A list of module name strings.

Examples:
    >>> for name in mitk.get_loaded_modules():
    ...     print(name)
)");
}
