/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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
#include <vtkCellData.h>
#include <vtkIdList.h>

#include "mitkUnstructuredGridHistogram.h"



mitk::UnstructuredGridHistogram::~UnstructuredGridHistogram()
{
}

void mitk::UnstructuredGridHistogram::Initialize(mitk::UnstructuredGrid* ugrid)
{

  const int numBins = 20;

  vtkUnstructuredGrid* vtkUGrid = ugrid->GetVtkUnstructuredGrid();
  
  double* range = vtkUGrid->GetScalarRange();
   
  SizeType size;
  MeasurementVectorType lowerBound;
  MeasurementVectorType upperBound;
  
  size.Fill(numBins);
  lowerBound[0] = range[0];
  upperBound[0] = range[1];
  double length = upperBound[0] - lowerBound[0];
  
  this->Superclass::Initialize(size, lowerBound, upperBound);
  
  vtkDataArray* data;
  if (m_UsePointData) data = vtkUGrid->GetPointData()->GetScalars();
  else data = vtkUGrid->GetCellData()->GetScalars();
  
  if (data == 0) return;
  
  if (m_UsePointData)
  {
    vtkIdList* cellIds = vtkIdList::New();
    for (vtkIdType pointId = 0; pointId < vtkUGrid->GetNumberOfPoints(); pointId++) {
      vtkUGrid->GetPointCells(pointId, cellIds);
      double numIds = (double)cellIds->GetNumberOfIds();
      double scalar = data->GetComponent(pointId, 0);
      //MITK_INFO << "scalar value: " << scalar;
      int bin = numBins - 1;
      if (scalar != upperBound[0])
        bin = (int)(((double)numBins)*(scalar-lowerBound[0])/length);
      //MITK_INFO << " bin: " << bin << std::endl;
      this->IncreaseFrequency(bin, scalar/numIds);
      cellIds->Reset();
    }
    cellIds->Delete();
  }
  else
  {
    for (vtkIdType cellId = 0; cellId < vtkUGrid->GetNumberOfCells(); cellId++) {
      double scalar = data->GetComponent(cellId, 0);
      //MITK_INFO << "scalar value: " << scalar;
      int bin = numBins - 1;
      if (scalar != upperBound[0])
        bin = (int)(((double)numBins)*(scalar-lowerBound[0])/length);
      //MITK_INFO << " bin: " << bin << std::endl;
      this->IncreaseFrequency(bin, scalar);
    }
  }
}
