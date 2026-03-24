/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkImageToContourFilter.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>
#include <mitkVtkRepresentationProperty.h>
#include <vtkLinearTransform.h>
#include <vtkMatrix4x4.h>
#include <vtkProperty.h>
#include <vtkSmartPointer.h>

#include <itkConstantPadImageFilter.h>
#include <itkUnaryGeneratorImageFilter.h>

#include <mitkNumericConstants.h>

mitk::ImageToContourFilter::ImageToContourFilter()
  : m_SliceGeometry(nullptr),
    m_UseProgressBar(false),
    m_ProgressStepSize(1),
    m_ContourValue(1.0)
{
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
  using ImageType = itk::Image<TPixel, VImageDimension>;
  using BinaryFilter = itk::UnaryGeneratorImageFilter<ImageType, ImageType>;
  using PadFilter = itk::ConstantPadImageFilter<ImageType, ImageType>;
  using ContourExtractorFilter = itk::ContourExtractor2DImageFilter<ImageType>;

  // Create a binary mask

  auto binaryFilter = BinaryFilter::New();

  binaryFilter->SetInput(sliceImage);
  binaryFilter->SetFunctor([this](TPixel pixVal) { return fabs(pixVal - m_ContourValue) < mitk::eps ? 1.0 : 0.0; });

  // Pad the mask since the contour extractor would not close contours along pixels at the border

  typename ImageType::SizeType bound;
  bound.Fill(1);

  auto padFilter = PadFilter::New();

  padFilter->SetInput(binaryFilter->GetOutput());
  padFilter->SetPadLowerBound(bound);
  padFilter->SetPadUpperBound(bound);

  // Extract the contour(s)

  auto contourExtractorFilter = ContourExtractorFilter::New();

  contourExtractorFilter->SetInput(padFilter->GetOutput());
  contourExtractorFilter->SetContourValue(0.5);
  contourExtractorFilter->Update();

  unsigned int foundPaths = contourExtractorFilter->GetNumberOfOutputs();

  auto contourSurface = vtkSmartPointer<vtkPolyData>::New();
  auto points = vtkSmartPointer<vtkPoints>::New();
  auto polygons = vtkSmartPointer<vtkCellArray>::New();

  unsigned int pointId = 0;

  for (unsigned int i = 0; i < foundPaths; i++)
  {
    const auto* currentPath = contourExtractorFilter->GetOutput(i)->GetVertexList();

    auto polygon = vtkSmartPointer<vtkPolygon>::New();
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