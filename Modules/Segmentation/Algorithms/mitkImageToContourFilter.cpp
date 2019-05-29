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

#include "mitkImageToContourFilter.h"
#include "mitkImageAccessByItk.h"
#include "mitkImageCast.h"
#include "mitkVtkRepresentationProperty.h"
#include "vtkLinearTransform.h"
#include "vtkMatrix4x4.h"
#include "vtkProperty.h"
#include "vtkSmartPointer.h"

#include <itkConstantPadImageFilter.h>

mitk::ImageToContourFilter::ImageToContourFilter()
{
  this->m_UseProgressBar = false;
  this->m_ProgressStepSize = 1;
  this->m_SliceGeometry = nullptr;
}

mitk::ImageToContourFilter::~ImageToContourFilter()
{
}

void mitk::ImageToContourFilter::GenerateData()
{
  mitk::Image::ConstPointer sliceImage = ImageToSurfaceFilter::GetInput();

  if (!sliceImage)
  {
    MITK_ERROR << "mitk::ImageToContourFilter: No input available. Please set the input!" << std::endl;
    itkExceptionMacro("mitk::ImageToContourFilter: No input available. Please set the input!");
    return;
  }

  if (sliceImage->GetDimension() > 2 || sliceImage->GetDimension() < 2)
  {
    MITK_ERROR << "mitk::ImageToImageFilter::GenerateData() works only with 2D images. Please assure that your input "
                  "image is 2D!"
               << std::endl;
    itkExceptionMacro(
      "mitk::ImageToImageFilter::GenerateData() works only with 2D images. Please assure that your input image is 2D!");
    return;
  }

  m_SliceGeometry = sliceImage->GetGeometry();

  AccessFixedDimensionByItk(sliceImage, Itk2DContourExtraction, 2);

  // Setting progressbar
  if (this->m_UseProgressBar)
    mitk::ProgressBar::GetInstance()->Progress(this->m_ProgressStepSize);
}

template <typename TPixel, unsigned int VImageDimension>
void mitk::ImageToContourFilter::Itk2DContourExtraction(const itk::Image<TPixel, VImageDimension> *sliceImage)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::ContourExtractor2DImageFilter<ImageType> ContourExtractor;

  typedef itk::ConstantPadImageFilter<ImageType, ImageType> PadFilterType;
  typename PadFilterType::Pointer padFilter = PadFilterType::New();
  typename ImageType::SizeType lowerExtendRegion;
  lowerExtendRegion[0] = 1;
  lowerExtendRegion[1] = 1;

  typename ImageType::SizeType upperExtendRegion;
  upperExtendRegion[0] = 1;
  upperExtendRegion[1] = 1;

  /*
   * We need to pad here, since the ITK contour extractor fails if the
   * segmentation touches more than one image edge.
   * By padding the image for one row at each edge we overcome this issue
   */
  padFilter->SetInput(sliceImage);
  padFilter->SetConstant(0);
  padFilter->SetPadLowerBound(lowerExtendRegion);
  padFilter->SetPadUpperBound(upperExtendRegion);

  typename ContourExtractor::Pointer contourExtractor = ContourExtractor::New();
  contourExtractor->SetInput(padFilter->GetOutput());
  contourExtractor->SetContourValue(0.5);

  contourExtractor->Update();

  unsigned int foundPaths = contourExtractor->GetNumberOfOutputs();

  vtkSmartPointer<vtkPolyData> contourSurface = vtkSmartPointer<vtkPolyData>::New();
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> polygons = vtkSmartPointer<vtkCellArray>::New();

  unsigned int pointId(0);

  for (unsigned int i = 0; i < foundPaths; i++)
  {
    const ContourPath *currentPath = contourExtractor->GetOutput(i)->GetVertexList();

    vtkSmartPointer<vtkPolygon> polygon = vtkSmartPointer<vtkPolygon>::New();
    polygon->GetPointIds()->SetNumberOfIds(currentPath->Size());

    Point3D currentPoint;
    Point3D currentWorldPoint;

    for (unsigned int j = 0; j < currentPath->Size(); j++)
    {
      currentPoint[0] = currentPath->ElementAt(j)[0];
      currentPoint[1] = currentPath->ElementAt(j)[1];
      currentPoint[2] = 0;

      m_SliceGeometry->IndexToWorld(currentPoint, currentWorldPoint);

      points->InsertPoint(pointId, currentWorldPoint[0], currentWorldPoint[1], currentWorldPoint[2]);
      polygon->GetPointIds()->SetId(j, pointId);
      pointId++;

    } // for2

    polygons->InsertNextCell(polygon);

  } // for1

  contourSurface->SetPoints(points);
  contourSurface->SetPolys(polygons);
  contourSurface->BuildLinks();
  Surface::Pointer finalSurface = this->GetOutput();

  finalSurface->SetVtkPolyData(contourSurface);
}

void mitk::ImageToContourFilter::GenerateOutputInformation()
{
  Superclass::GenerateOutputInformation();
}

void mitk::ImageToContourFilter::SetUseProgressBar(bool status)
{
  this->m_UseProgressBar = status;
}

void mitk::ImageToContourFilter::SetProgressStepSize(unsigned int stepSize)
{
  this->m_ProgressStepSize = stepSize;
}
