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

#include <mitkUnstructuredGridClusteringFilter.h>

#include <vtkSmartPointer.h>
#include <vtkPoints.h>
#include <vtkUnstructuredGrid.h>


mitk::UnstructuredGridClusteringFilter::UnstructuredGridClusteringFilter()
{
  this->m_UnstructGrid = mitk::UnstructuredGrid::New();
}

mitk::UnstructuredGridClusteringFilter::~UnstructuredGridClusteringFilter(){}

void mitk::UnstructuredGridClusteringFilter::GenerateOutputInformation()
{
  mitk::UnstructuredGrid::Pointer inputImage = const_cast<mitk::UnstructuredGrid*>(this->GetInput());
  if(inputImage.IsNull()) return;

  m_UnstructGrid = this->GetOutput();
}
