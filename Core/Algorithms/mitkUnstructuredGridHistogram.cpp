/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision: 4509 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <vtkUnstructuredGrid.h>

#include <vtkPointData.h>
#include <vtkIdList.h>

#include "mitkUnstructuredGridHistogram.h"


mitk::UnstructuredGridHistogram::UnstructuredGridHistogram()
{
}

mitk::UnstructuredGridHistogram::~UnstructuredGridHistogram()
{
}

void mitk::UnstructuredGridHistogram::Initialize(mitk::UnstructuredGrid* ugrid)
{

  const int numBins = 20;

  vtkUnstructuredGrid* vtkUGrid = ugrid->GetVtkUnstructuredGrid();
  
  //std::cout << "initializing histogram\n";
  //vtkUGrid->PrintSelf(cout, 0);
  const vtkIdType numPoints = vtkUGrid->GetNumberOfPoints();
  double* range = vtkUGrid->GetScalarRange();
   
  SizeType size;
  MeasurementVectorType lowerBound;
  MeasurementVectorType upperBound;
  
  size.Fill(numBins);
  lowerBound[0] = range[0];
  upperBound[0] = range[1];
  double length = upperBound[0] - lowerBound[0];
  
  this->Superclass::Initialize(size, lowerBound, upperBound);
  
  vtkDataArray* pointData = vtkUGrid->GetPointData()->GetScalars();
  vtkIdList* cellIds = vtkIdList::New();
  for (vtkIdType pointId = 0; pointId < numPoints; pointId++) {
    vtkUGrid->GetPointCells(pointId, cellIds);
    double numIds = (double)cellIds->GetNumberOfIds();
    double scalar = pointData->GetComponent(pointId, 0);
    //std::cout << "scalar value: " << scalar;
    int bin = numBins - 1;
    if (scalar != upperBound[0])
      bin = (int)(((double)numBins)*(scalar-lowerBound[0])/length);
    //std::cout << " bin: " << bin << std::endl;
    this->IncreaseFrequency(bin, scalar/numIds);
    cellIds->Reset();
  }
  cellIds->Delete();
}
