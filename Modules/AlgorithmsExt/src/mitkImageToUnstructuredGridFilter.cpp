/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkImageToUnstructuredGridFilter.h>

#include <vtkPoints.h>
#include <vtkPolyVertex.h>
#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>

#include <itkImageRegionIterator.h>

#include <mitkImageAccessByItk.h>

mitk::ImageToUnstructuredGridFilter::ImageToUnstructuredGridFilter() : m_NumberOfExtractedPoints(0), m_Threshold(-0.1)
{
  this->m_UnstructGrid = mitk::UnstructuredGrid::New();
}

mitk::ImageToUnstructuredGridFilter::~ImageToUnstructuredGridFilter()
{
}

void mitk::ImageToUnstructuredGridFilter::GenerateData()
{
  mitk::UnstructuredGrid::Pointer unstructGrid = this->GetOutput();
  const mitk::Image *image = this->GetInput();

  if (image == nullptr || !image->IsInitialized())
  {
    MITK_ERROR << "Wrong input image set" << std::endl;
    return;
  }

  m_Geometry = image->GetGeometry();

  m_NumberOfExtractedPoints = 0;

  AccessByItk(image, ExtractPoints)
}

void mitk::ImageToUnstructuredGridFilter::SetInput(const mitk::Image *image)
{
  this->ProcessObject::SetNthInput(0, const_cast<mitk::Image *>(image));
}

const mitk::Image *mitk::ImageToUnstructuredGridFilter::GetInput(void) const
{
  if (this->GetNumberOfInputs() < 1)
  {
    MITK_ERROR << "No input set" << std::endl;
    return nullptr;
  }

  return static_cast<const mitk::Image *>(this->ProcessObject::GetInput(0));
}

mitk::Image *mitk::ImageToUnstructuredGridFilter::GetInput(void)
{
  if (this->GetNumberOfInputs() < 1)
  {
    MITK_ERROR << "No input set" << std::endl;
    return nullptr;
  }

  return static_cast<mitk::Image *>(this->ProcessObject::GetInput(0));
}

template <typename TPixel, unsigned int VImageDimension>
void mitk::ImageToUnstructuredGridFilter::ExtractPoints(const itk::Image<TPixel, VImageDimension> *image)
{
  typedef itk::Image<TPixel, VImageDimension> InputImageType;
  typename itk::ImageRegionConstIterator<InputImageType> it(image, image->GetRequestedRegion());

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

  it.GoToBegin();
  while (!it.IsAtEnd())
  {
    if (it.Get() >= m_Threshold)
    {
      mitk::Point3D imagePoint;
      mitk::Point3D worldPoint;

      imagePoint[0] = it.GetIndex()[0];
      imagePoint[1] = it.GetIndex()[1];
      imagePoint[2] = it.GetIndex()[2];

      m_Geometry->IndexToWorld(imagePoint, worldPoint);

      points->InsertNextPoint(worldPoint[0], worldPoint[1], worldPoint[2]);
      m_NumberOfExtractedPoints++;
    }
    ++it;
  }

  vtkSmartPointer<vtkPolyVertex> verts = vtkSmartPointer<vtkPolyVertex>::New();

  verts->GetPointIds()->SetNumberOfIds(m_NumberOfExtractedPoints);
  for (int i = 0; i < m_NumberOfExtractedPoints; i++)
  {
    verts->GetPointIds()->SetId(i, i);
  }

  vtkSmartPointer<vtkUnstructuredGrid> uGrid = vtkSmartPointer<vtkUnstructuredGrid>::New();
  uGrid->Allocate(1);

  uGrid->InsertNextCell(verts->GetCellType(), verts->GetPointIds());
  uGrid->SetPoints(points);

  m_UnstructGrid->SetVtkUnstructuredGrid(uGrid);
}

void mitk::ImageToUnstructuredGridFilter::SetThreshold(double threshold)
{
  this->m_Threshold = threshold;
}

double mitk::ImageToUnstructuredGridFilter::GetThreshold()
{
  return this->m_Threshold;
}

void mitk::ImageToUnstructuredGridFilter::GenerateOutputInformation()
{
  mitk::Image::ConstPointer inputImage = this->GetInput();

  m_UnstructGrid = this->GetOutput();

  itkDebugMacro(<< "GenerateOutputInformation()");

  if (inputImage.IsNull())
    return;
}
