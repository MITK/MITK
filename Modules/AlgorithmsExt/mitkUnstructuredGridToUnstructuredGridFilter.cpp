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

#include <mitkUnstructuredGridToUnstructuredGridFilter.h>

#include <vtkSmartPointer.h>
#include <vtkPoints.h>
#include <vtkUnstructuredGrid.h>

#include <itkImageRegionIterator.h>

#include <mitkImageAccessByItk.h>


mitk::UnstructuredGridToUnstructuredGridFilter::UnstructuredGridToUnstructuredGridFilter()
{
  this->m_UnstructGrid = mitk::UnstructuredGrid::New();
}

mitk::UnstructuredGridToUnstructuredGridFilter::~UnstructuredGridToUnstructuredGridFilter(){}

void mitk::UnstructuredGridToUnstructuredGridFilter::SetInput(const mitk::UnstructuredGrid* image)
{
  this->ProcessObject::SetNthInput(0, const_cast< mitk::UnstructuredGrid* >( image ) );
}


const mitk::UnstructuredGrid* mitk::UnstructuredGridToUnstructuredGridFilter::GetInput(void)
{
  if (this->GetNumberOfInputs() < 1)
  {
    MITK_ERROR << "No input set" << std::endl;
    return 0;
  }

  return static_cast<const mitk::UnstructuredGrid* >( this->ProcessObject::GetInput(0) );
}


void mitk::UnstructuredGridToUnstructuredGridFilter::GenerateOutputInformation()
{
  mitk::UnstructuredGrid::ConstPointer inputImage = this->GetInput();

  m_UnstructGrid = this->GetOutput();

  itkDebugMacro(<<"GenerateOutputInformation()");

  if(inputImage.IsNull()) return;
}
