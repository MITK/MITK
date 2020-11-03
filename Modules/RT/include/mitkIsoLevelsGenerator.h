/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef __ISO_LEVELS_GENERATOR_H
#define __ISO_LEVELS_GENERATOR_H

#include "mitkIsoDoseLevelCollections.h"

#include "MitkRTExports.h"

namespace mitk
{
  IsoDoseLevelSet::Pointer MITKRT_EXPORT GenerateIsoLevels_Virtuos();
}

#endif
