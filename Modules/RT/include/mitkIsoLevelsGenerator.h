/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkIsoLevelsGenerator_h
#define mitkIsoLevelsGenerator_h

#include <mitkIsoDoseLevelCollections.h>

#include <MitkRTExports.h>

namespace mitk
{
  /**
   * \brief Generates a predefined set of iso dose levels following the Virtuos color scheme.
   *
   * Creates an IsoDoseLevelSet containing 18 dose iso levels ranging from 1% to 150%
   * of the reference dose, each with a distinct color and both iso line and color wash
   * visibility enabled. This serves as the default color preset for RT dose visualization.
   *
   * \return A smart pointer to the newly created IsoDoseLevelSet.
   *
   * \sa IsoDoseLevelSet
   * \sa IsoDoseLevel
   */
  IsoDoseLevelSet::Pointer MITKRT_EXPORT GenerateIsoLevels_Virtuos();
}

#endif
