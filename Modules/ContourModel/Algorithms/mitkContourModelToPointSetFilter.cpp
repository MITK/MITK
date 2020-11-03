/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkContourModelToPointSetFilter.h"
#include <mitkInteractionConst.h>
#include <mitkPointOperation.h>

mitk::ContourModelToPointSetFilter::ContourModelToPointSetFilter()
{
  itk::DataObject::Pointer output = this->MakeOutput(0);
  this->SetNumberOfRequiredInputs(1);
  this->SetNumberOfRequiredOutputs(1);
  this->SetNthOutput(0, output.GetPointer());
}
mitk::ContourModelToPointSetFilter::~ContourModelToPointSetFilter()
{
}

void mitk::ContourModelToPointSetFilter::GenerateData()
{
  mitk::ContourModel::Pointer inputContour = static_cast<mitk::ContourModel *>(this->GetInput(0));
  mitk::ContourModelToPointSetFilter::OutputType *outputPointSet = this->GetOutput();

  auto it = inputContour->IteratorBegin();
  auto end = inputContour->IteratorEnd();
  unsigned int pointId = 0;

  std::size_t timestep = inputContour->GetTimeSteps();

  for (std::size_t i = 0; i < timestep; i++)
  {
    while (it <= end)
    {
      mitk::Point3D p = (*it)->Coordinates;
      mitk::PointOperation popInsert(
        mitk::OpINSERT, inputContour->GetTimeGeometry()->TimeStepToTimePoint(timestep), p, pointId++, false);

      outputPointSet->ExecuteOperation(&popInsert);

      it++;
    }
  }
}
