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

#include "mitkTimeGridHelper.h"

#include "itkExceptionObject.h"

bool mitk::TimeGridIsMonotonIncreasing(const mitk::ModelBase::TimeGridType timeGrid)
{
    const auto beginPos = timeGrid.begin();
    const auto endPos = timeGrid.end();
    for(mitk::ModelBase::TimeGridType::const_iterator posTime = beginPos; posTime != endPos; ++posTime)
    {
      if (posTime != beginPos && *(posTime-1)<*posTime) return false;
    }

    return true;
};

mitk::ModelBase::ModelResultType mitk::InterpolateSignalToNewTimeGrid(const ModelBase::ModelResultType& inputSignal, const ModelBase::TimeGridType& inputGrid, const ModelBase::TimeGridType& outputGrid)
{
    mitk::ModelBase::ModelResultType result(outputGrid.GetSize());
    if (! inputSignal.GetSize())
    {
      return result;
    }

    if (inputSignal.GetSize() != inputGrid.GetSize())
    {
      itkGenericExceptionMacro("Input signal and input time grid have not the same size.");
    }

    mitk::ModelBase::ModelResultType::ValueType lastValue = inputSignal[0];
    mitk::ModelBase::TimeGridType::ValueType lastTime = itk::NumericTraits<mitk::ModelBase::TimeGridType::ValueType>::NonpositiveMin();

    mitk::ModelBase::TimeGridType::const_iterator posITime = inputGrid.begin();
    mitk::ModelBase::ModelResultType::const_iterator posValue = inputSignal.begin();
    mitk::ModelBase::ModelResultType::iterator posResult = result.begin();

    for(mitk::ModelBase::TimeGridType::const_iterator posOTime = outputGrid.begin(); posOTime != outputGrid.end(); ++posResult, ++posOTime)
    {
        while(posITime!=inputGrid.end() && *posOTime > *posITime)
      { //forward in the input grid until the current output point
        //is between last and the current input point.
        lastValue = *posValue;
        lastTime = *posITime;
        ++posValue;
        ++posITime;
      }

      double weightLast = 1 - (*posOTime - lastTime)/(*posITime - lastTime);
      double weightNext = 1 - (*posITime - *posOTime)/(*posITime - lastTime);

      *posResult = weightLast * lastValue + weightNext * (*posValue);
    }

    return result;
};

mitk::ModelBase::TimeGridType
mitk::GenerateSupersampledTimeGrid(const mitk::ModelBase::TimeGridType& grid, const unsigned int samplingRate)
{
  unsigned int origGridSize = grid.size();

  mitk::ModelBase::TimeGridType interpolatedTimeGrid(((origGridSize - 1) * samplingRate) + 1);

  for (unsigned int t = 0; t < origGridSize - 1; ++t)
  {
    double delta = (grid[t + 1] - grid[t]) / samplingRate;

    for (unsigned int i = 0; i < samplingRate; ++i)
    {
      interpolatedTimeGrid[(t * samplingRate) + i] = grid[t] + i * delta;
    }
  }

  interpolatedTimeGrid[interpolatedTimeGrid.size() - 1] = grid[grid.size() - 1];

  return interpolatedTimeGrid;
};
