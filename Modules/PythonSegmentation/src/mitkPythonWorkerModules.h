/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPythonWorkerModules_h
#define mitkPythonWorkerModules_h

#include <mitkBaseGeometry.h>
#include <mitkPythonWorker.h>

#include <nlohmann/json.hpp>

#include <exception>
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

  /**
   * \brief Returns the index-to-world transform of a geometry as the four rows of a 4x4 matrix.
   *
   * The workers derive the affine of an image from it.
   */
  nlohmann::json GetIndexToWorld(const BaseGeometry* geometry);

  /**
   * \brief Returns an mitk::Exception with the given message, for a callback
   *        that reports how work off the GUI thread ended.
   */
  std::exception_ptr MakeExceptionPtr(const std::string& message);
}

#endif
