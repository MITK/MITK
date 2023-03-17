/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkTimeGridHelper_h
#define mitkTimeGridHelper_h

#include "mitkModelBase.h"

#include "MitkModelFitExports.h"

namespace mitk
{
  /* Checks if the time grid is monotone increasing (timeGrid[n] <= timeGrid[n+1]).
   * It is a precondition for the helper interpolate time grid.*/
  MITKMODELFIT_EXPORT bool TimeGridIsMonotonIncreasing(const ModelBase::TimeGridType timeGrid);

  /* Helper function that interpolates a passed signal to a new time grid.
   * @pre The time grids must be monotone increasing. Use TimeGridIsMonotonIncreasing() to verify that.*/
  MITKMODELFIT_EXPORT ModelBase::ModelResultType InterpolateSignalToNewTimeGrid(const ModelBase::ModelResultType& inputSignal, const ModelBase::TimeGridType& inputGrid, const ModelBase::TimeGridType& outputGrid);

  /** Super sample passed time grid by a given supersampling rate and interpolates linear in between original time steps.
   * @param grid
   * @param samplingRate Defines how many samples should be generated between to original time steps (including the preceeding time step). E.g. a sampling rate of 1 will just returns the original grid untouched;
   a sampling rate of 3 will generate to aditional steps between to original steps.*/
  MITKMODELFIT_EXPORT ModelBase::TimeGridType GenerateSupersampledTimeGrid(const mitk::ModelBase::TimeGridType& grid, const unsigned int samplingRate);
}

#endif
