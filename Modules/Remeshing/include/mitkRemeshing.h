/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRemeshing_h
#define mitkRemeshing_h

#include <mitkSurface.h>
#include <MitkRemeshingExports.h>

namespace mitk
{
  namespace Remeshing
  {
    MITKREMESHING_EXPORT Surface::Pointer Decimate(Surface::ConstPointer input);
  }
}

#endif
