/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPythonSegmentationUI_h
#define mitkPythonSegmentationUI_h

#include <MitkPythonSegmentationUIExports.h>

/** \brief Namespace for the PythonSegmentationUI module.
 *
 * \sa QmitknnInteractiveToolGUI, QmitkPipInstallDialog
 */
namespace mitk::PythonSegmentationUI
{
  /** \brief Forces the linker to include this module.
   *
   * Since this module only contains runtime dependencies (e.g., tool GUI
   * registrations via static initialization), it would be discarded by the
   * linker without an explicit symbol reference. Call this function from a
   * dependent module to ensure the PythonSegmentationUI module is linked.
   */
  MITKPYTHONSEGMENTATIONUI_EXPORT void EnforceLinkage();
}

#endif
