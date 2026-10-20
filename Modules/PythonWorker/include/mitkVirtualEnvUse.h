/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkVirtualEnvUse_h
#define mitkVirtualEnvUse_h

#include <MitkPythonWorkerExports.h>

#include <string>

namespace mitk
{
  /** \brief How this process uses a virtual environment. */
  enum class VirtualEnvUse
  {
    None,         /**< \brief Not in use; it can be updated or removed. */
    Worker,       /**< \brief A PythonWorker runs its interpreter. Ending the worker,
                              e.g. by uninitializing the tool, frees it. */
    LoadedModules /**< \brief A native library of it is loaded into this process. Only a
                              restart of the application frees it. */
  };

  /** \brief Tells how this process uses a virtual environment.
   *
   * An environment in use can be neither updated nor removed: on Windows its
   * files cannot be replaced, and on Linux and macOS replacing them would
   * break the code that runs from them. Loaded modules take precedence over a
   * worker, since ending the worker would not free the environment then.
   *
   * \param[in] venvName Name of the virtual environment.
   */
  MITKPYTHONWORKER_EXPORT VirtualEnvUse GetVirtualEnvUse(const std::string& venvName);
}

#endif
