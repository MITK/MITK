/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkExtractTimeGrid.h"

mitk::ModelBase::TimeGridType mitk::ExtractTimeGrid(const Image* image)
{
  mitk::ModelBase::TimeGridType result;
  if (image)
  {
    result = ExtractTimeGrid(image->GetTimeGeometry());
  }
  return result;
};

mitk::ModelBase::TimeGridType mitk::ExtractTimeGrid(const TimeGeometry* geometry)
{
  mitk::ModelBase::TimeGridType result;
  if (geometry)
  {
    unsigned int i = 0;
    std::vector<double> tempGrid;
    while(geometry->IsValidTimeStep(i))
    {
      tempGrid.push_back(geometry->TimeStepToTimePoint(i)/1000.0);
      ++i;
    }

    result.SetSize(tempGrid.size());

    mitk::ModelBase::TimeGridType::iterator destPost = result.begin();

    for(std::vector<double>::iterator sourcePos = tempGrid.begin(); sourcePos != tempGrid.end(); ++sourcePos, ++destPost)
    {
      *destPost = *sourcePos;
    }
  }

  return result;
};
