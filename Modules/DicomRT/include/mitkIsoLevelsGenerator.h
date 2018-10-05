/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef __ISO_LEVELS_GENERATOR_H
#define __ISO_LEVELS_GENERATOR_H

#include "mitkIsoDoseLevelCollections.h"

#include "MitkDicomRTExports.h"

namespace mitk
{
  IsoDoseLevelSet::Pointer MITKDICOMRT_EXPORT GenerateIsoLevels_Virtuos();
}

#endif
