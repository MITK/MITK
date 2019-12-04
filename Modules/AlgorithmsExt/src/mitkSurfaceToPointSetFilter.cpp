/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSurfaceToPointSetFilter.h"

// mitk headers
#include "mitkPointSet.h"

// itk headers
#include "itkMatrix.h"
#include "itkVector.h"

// vtk headers
#include <vtkCleanPolyData.h>
#include <vtkPolyData.h>

mitk::SurfaceToPointSetFilter::SurfaceToPointSetFilter()
{
  this->SetNumberOfIndexedOutputs(1);
}

mitk::SurfaceToPointSetFilter::~SurfaceToPointSetFilter()
{
}

void mitk::SurfaceToPointSetFilter::GenerateData()
{
  if (m_InputSurface.IsNull())
  {
    this->SetNthOutput(0, nullptr);
    m_ErrorMessage = "Error in SurfaceToPointSetFilter: Input is not set correctly.";
    return;
  }
  mitk::PointSet::Pointer result = mitk::PointSet::New();

  vtkPolyData *points = this->m_InputSurface->GetVtkPolyData();
  vtkCleanPolyData *cleaner = vtkCleanPolyData::New();
  cleaner->PieceInvariantOff();
  cleaner->ConvertLinesToPointsOff();
  cleaner->ConvertPolysToLinesOff();
  cleaner->ConvertStripsToPolysOff();
  cleaner->PointMergingOn();
  cleaner->SetInputData(points);
  cleaner->Update();

  vtkPolyData *mergedPoints = cleaner->GetOutput();

  // generate filter output
  for (int i = 0; i < mergedPoints->GetNumberOfPoints(); i++)
  {
    mitk::Point3D currentPoint;
    currentPoint[0] = mergedPoints->GetPoint(i)[0];
    currentPoint[1] = mergedPoints->GetPoint(i)[1];
    currentPoint[2] = mergedPoints->GetPoint(i)[2];
    result->InsertPoint(i, currentPoint);
  }
  this->SetNthOutput(0, result);
  //-------------------------

  cleaner->Delete();
}

void mitk::SurfaceToPointSetFilter::SetInput(mitk::Surface::Pointer InputSurface)
{
  m_InputSurface = InputSurface;
  this->Modified();
}

std::string mitk::SurfaceToPointSetFilter::GetErrorMessage()
{
  return this->m_ErrorMessage;
}
