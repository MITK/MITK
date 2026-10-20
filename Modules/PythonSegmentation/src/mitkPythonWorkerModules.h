/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPythonWorkerModules_h
#define mitkPythonWorkerModules_h

#include <mitkPythonWorker.h>

#include <string>

namespace mitk
{
  /**
   * \brief Reads a Python module for a PythonWorker from the resources of MitkPythonSegmentation.
   *
   * The module is named after its file, e.g. "voxtell_bridge" for
   * "VoxTell/voxtell_bridge.py".
   *
   * MitkPythonWorker reads its own runtime the same way. The few lines are
   * kept twice on purpose: a shared factory would put CppMicroServices types
   * into the public API of MitkPythonWorker for one further user.
   *
   * \throw mitk::Exception if the resource does not exist.
   */
  PythonWorker::Module ReadWorkerModule(const std::string& resourcePath);
}

#endif
