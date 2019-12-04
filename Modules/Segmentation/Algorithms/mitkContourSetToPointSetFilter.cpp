/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkContourSetToPointSetFilter.h"

#include <mitkInteractionConst.h>
#include <mitkPointOperation.h>

mitk::ContourSetToPointSetFilter::ContourSetToPointSetFilter()
{
  OutputType::Pointer output = dynamic_cast<OutputType *>(this->MakeOutput(0).GetPointer());
  this->SetNumberOfRequiredInputs(1);
  this->SetNumberOfIndexedOutputs(1);
  this->SetNthOutput(0, output.GetPointer());
  m_Frequency = 5;
}

mitk::ContourSetToPointSetFilter::~ContourSetToPointSetFilter()
{
}

void mitk::ContourSetToPointSetFilter::GenerateOutputInformation()
{
}

void mitk::ContourSetToPointSetFilter::GenerateData()
{
  auto *input = (mitk::ContourSet *)(this->GetInput());
  mitk::PointSet::Pointer output = this->GetOutput();

  mitk::ContourSet::ContourVectorType contourVec = input->GetContours();
  auto contourIt = contourVec.begin();
  unsigned int pointId = 0;

  while (contourIt != contourVec.end())
  {
    auto *nextContour = (mitk::Contour *)(*contourIt).second;

    mitk::Contour::InputType idx = nextContour->GetContourPath()->StartOfInput();
    mitk::Contour::OutputType point;
    mitk::Contour::InputType end = nextContour->GetContourPath()->EndOfInput();
    if (end > 50000)
      end = 0;

    while (idx <= end)
    {
      point = nextContour->GetContourPath()->Evaluate(idx);
      Contour::BoundingBoxType::PointType p;
      p.CastFrom(point);
      mitk::PointOperation popInsert(mitk::OpINSERT, p, pointId++);
      mitk::PointOperation popDeactivate(mitk::OpDESELECTPOINT, p, pointId++);
      output->ExecuteOperation(&popInsert);
      output->ExecuteOperation(&popDeactivate);

      idx += m_Frequency;
    }
    contourIt++;
  }
}

const mitk::ContourSet *mitk::ContourSetToPointSetFilter::GetInput(void)
{
  if (this->GetNumberOfInputs() < 1)
  {
    return nullptr;
  }

  return static_cast<const mitk::ContourSet *>(this->BaseDataSource::GetInput(0));
}

void mitk::ContourSetToPointSetFilter::SetInput(const mitk::ContourSet *input)
{
  // Process object is not const-correct so the const_cast is required here
  this->BaseDataSource::SetNthInput(0, const_cast<mitk::ContourSet *>(input));
}
