/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkTimeGridHelper_h
#define mitkTimeGridHelper_h

#include <mitkModelBase.h>

#include <MitkModelFitExports.h>

namespace mitk
{
  /**
   * \brief Checks if the time grid is monotonically increasing (timeGrid[n] <= timeGrid[n+1]).
   *
   * This is a precondition for the InterpolateSignalToNewTimeGrid helper function.
   */
  MITKMODELFIT_EXPORT bool TimeGridIsMonotonIncreasing(const ModelBase::TimeGridType timeGrid);

  /**
   * \brief Interpolates a signal to a new time grid.
   * \pre The time grids must be monotonically increasing. Use TimeGridIsMonotonIncreasing() to verify.
   */
  MITKMODELFIT_EXPORT ModelBase::ModelResultType InterpolateSignalToNewTimeGrid(const ModelBase::ModelResultType& inputSignal, const ModelBase::TimeGridType& inputGrid, const ModelBase::TimeGridType& outputGrid);

  /**
   * \brief Supersamples a time grid by a given rate, interpolating linearly between original time steps.
   * \param grid The original time grid.
   * \param samplingRate Number of samples between original time steps (including the preceding step).
   * A rate of 1 returns the original grid; a rate of 3 generates two additional steps between originals.
   */
  MITKMODELFIT_EXPORT ModelBase::TimeGridType GenerateSupersampledTimeGrid(const mitk::ModelBase::TimeGridType& grid, const unsigned int samplingRate);
}

#endif
