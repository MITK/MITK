/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPythonSegmentationUI.h>

void mitk::PythonSegmentationUI::EnforceLinkage()
{
  // Nothing here - just forces inclusion and static init of this module.
  // Since it only contains runtime dependencies it would be ignored by
  // the linker otherwise.
}
