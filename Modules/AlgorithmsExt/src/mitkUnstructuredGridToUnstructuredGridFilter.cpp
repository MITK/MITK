/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkUnstructuredGridToUnstructuredGridFilter.h>

#include <vtkPoints.h>
#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>

#include <itkImageRegionIterator.h>

#include <mitkImageAccessByItk.h>

mitk::UnstructuredGridToUnstructuredGridFilter::UnstructuredGridToUnstructuredGridFilter()
{
  this->m_UnstructGrid = mitk::UnstructuredGrid::New();
}

mitk::UnstructuredGridToUnstructuredGridFilter::~UnstructuredGridToUnstructuredGridFilter()
{
}

void mitk::UnstructuredGridToUnstructuredGridFilter::SetInput(const mitk::UnstructuredGrid *grid)
{
  this->ProcessObject::SetNthInput(0, const_cast<mitk::UnstructuredGrid *>(grid));
}

void mitk::UnstructuredGridToUnstructuredGridFilter::CreateOutputsForAllInputs(unsigned int /*idx*/)
{
  this->SetNumberOfIndexedOutputs(this->GetNumberOfIndexedInputs());
  for (unsigned int idx = 0; idx < this->GetNumberOfIndexedInputs(); ++idx)
  {
    if (this->GetOutput(idx) == nullptr)
    {
      mitk::UnstructuredGrid::Pointer newOutputX = mitk::UnstructuredGrid::New();
      this->SetNthOutput(idx, newOutputX);
    }
  }
  this->Modified();
}

void mitk::UnstructuredGridToUnstructuredGridFilter::SetInput(unsigned int idx, const mitk::UnstructuredGrid *grid)
{
  if (this->GetInput(idx) != grid)
  {
    this->SetNthInput(idx, const_cast<mitk::UnstructuredGrid *>(grid));
    this->CreateOutputsForAllInputs(idx);
    this->Modified();
  }
}

const mitk::UnstructuredGrid *mitk::UnstructuredGridToUnstructuredGridFilter::GetInput(void)
{
  if (this->GetNumberOfInputs() < 1)
    return nullptr;

  return static_cast<const mitk::UnstructuredGrid *>(this->ProcessObject::GetInput(0));
}

const mitk::UnstructuredGrid *mitk::UnstructuredGridToUnstructuredGridFilter::GetInput(unsigned int idx)
{
  if (this->GetNumberOfInputs() < 1)
    return nullptr;

  return static_cast<const mitk::UnstructuredGrid *>(this->ProcessObject::GetInput(idx));
}

void mitk::UnstructuredGridToUnstructuredGridFilter::GenerateOutputInformation()
{
  mitk::UnstructuredGrid::ConstPointer inputImage = this->GetInput();

  m_UnstructGrid = this->GetOutput();

  itkDebugMacro(<< "GenerateOutputInformation()");

  if (inputImage.IsNull())
    return;
}
