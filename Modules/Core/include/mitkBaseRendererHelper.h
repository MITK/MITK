/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKBASERENDERERHELPER_H
#define MITKBASERENDERERHELPER_H

#include <mitkBaseRenderer.h>
#include <mitkTimeGeometry.h>

namespace mitk
{
  namespace BaseRendererHelper
  {
    /*
     * \brief
     */
    MITKCORE_EXPORT bool IsRendererAlignedWithSegmentation(BaseRenderer* renderer, const TimeGeometry* timeGeometry);

  } // namespace BaseRendererHelper
} // namespace mitk

#endif // MITKBASERENDERERHELPER_H
