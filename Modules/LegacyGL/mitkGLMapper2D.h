/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkGLMapper2D_h
#define mitkGLMapper2D_h

#include "mitkGLMapper.h"

namespace mitk
{
  // typedef allows integration of mappers into the new mapper architecture
  // \deprecatedSince{2013_03} Use GLMapper instead
  DEPRECATED(typedef GLMapper GLMapper2D);

} // namespace mitk

#endif
