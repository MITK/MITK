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

#ifndef __MITK_TIME_GRID_HELPER_H_
#define __MITK_TIME_GRID_HELPER_H_

#include "mitkModelBase.h"

#include "MitkFitMIDataExports.h"

namespace mitk
{
  /* Checks if the time grid is monotone increasing (timeGrid[n] <= timeGrid[n+1]).
   * It is a precondition for the helper interpolate time grid.*/
  MITKFITMIDATA_EXPORT bool TimeGridIsMonotonIncreasing(const ModelBase::TimeGridType timeGrid);

  /* Helper function that interpolates a passed signal to a new time grid.
   * @pre The time grids must be monotone increasing. Use TimeGridIsMonotonIncreasing() to verify that.*/
  MITKFITMIDATA_EXPORT ModelBase::ModelResultType InterpolateSignalToNewTimeGrid(const ModelBase::ModelResultType& inputSignal, const ModelBase::TimeGridType& inputGrid, const ModelBase::TimeGridType& outputGrid);
}

#endif
