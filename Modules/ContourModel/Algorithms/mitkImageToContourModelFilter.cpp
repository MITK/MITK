/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkImageToContourModelFilter.h"
#include "mitkImageAccessByItk.h"

#include <itkConstantPadImageFilter.h>
#include <itkContourExtractor2DImageFilter.h>


mitk::ImageToContourModelFilter::ImageToContourModelFilter() : m_SliceGeometry(nullptr), m_ContourValue(0.5)
{
}

mitk::ImageToContourModelFilter::~ImageToContourModelFilter()
{
}

void mitk::ImageToContourModelFilter::SetInput(const mitk::ImageToContourModelFilter::InputType *input)
{
  this->SetInput(0, input);
}

void mitk::ImageToContourModelFilter::SetInput(unsigned int idx,
                                               const mitk::ImageToContourModelFilter::InputType *input)
{
  if (idx + 1 > this->GetNumberOfInputs())
  {
    this->SetNumberOfRequiredInputs(idx + 1);
  }
  if (input != static_cast<InputType *>(this->ProcessObject::GetInput(idx)))
  {
    this->ProcessObject::SetNthInput(idx, const_cast<InputType *>(input));
    this->Modified();
  }
}

const mitk::ImageToContourModelFilter::InputType *mitk::ImageToContourModelFilter::GetInput(void)
{
  if (this->GetNumberOfInputs() < 1)
    return nullptr;
  return static_cast<const mitk::ImageToContourModelFilter::InputType *>(this->ProcessObject::GetInput(0));
}

const mitk::ImageToContourModelFilter::InputType *mitk::ImageToContourModelFilter::GetInput(unsigned int idx)
{
  if (this->GetNumberOfInputs() < 1)
    return nullptr;
  return static_cast<const mitk::ImageToContourModelFilter::InputType *>(this->ProcessObject::GetInput(idx));
}

void mitk::ImageToContourModelFilter::SetContourValue(float contourValue)
{
  this->m_ContourValue = contourValue;
  this->Modified();
}

float mitk::ImageToContourModelFilter::GetContourValue()
{
  return this->m_ContourValue;
}

void mitk::ImageToContourModelFilter::GenerateData()
{
  mitk::Image::ConstPointer sliceImage = this->GetInput();

  if (!sliceImage)
  {
    MITK_ERROR << "mitk::ImageToContourModelFilter: No input available. Please set the input!" << std::endl;
    itkExceptionMacro("mitk::ImageToContourModelFilter: No input available. Please set the input!");
    return;
  }

  if (sliceImage->GetDimension() > 2 || sliceImage->GetDimension() < 2)
  {
    MITK_ERROR << "mitk::ImageToContourModelFilter::GenerateData() works only with 2D images. Please assure that your "
                  "input image is 2D!"
               << std::endl;
    itkExceptionMacro("mitk::ImageToContourModelFilter::GenerateData() works only with 2D images. Please assure that "
                      "your input image is 2D!");
    return;
  }

  m_SliceGeometry = sliceImage->GetGeometry();

  AccessFixedDimensionByItk(sliceImage, Itk2DContourExtraction, 2);
}

template <typename TPixel, unsigned int VImageDimension>
void mitk::ImageToContourModelFilter::Itk2DContourExtraction(const itk::Image<TPixel, VImageDimension> *sliceImage)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::ContourExtractor2DImageFilter<ImageType> ContourExtractor;

  typedef itk::PolyLineParametricPath<2> PolyLineParametricPath2D;
  typedef PolyLineParametricPath2D::VertexListType ContourPath;

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
  contourExtractor->SetContourValue(m_ContourValue);

  contourExtractor->Update();

  unsigned int foundPaths = contourExtractor->GetNumberOfOutputs();
  this->SetNumberOfIndexedOutputs(foundPaths);

  for (unsigned int i = 0; i < foundPaths; i++)
  {
    const ContourPath *currentPath = contourExtractor->GetOutput(i)->GetVertexList();

    mitk::Point3D currentPoint;
    mitk::Point3D currentWorldPoint;

    mitk::ContourModel::Pointer contour = this->GetOutput(i);
    if (contour.IsNull())
    {
      contour = mitk::ContourModel::New();
    }

    if (contour.IsNull())
      contour = mitk::ContourModel::New();

    for (unsigned int j = 0; j < currentPath->Size(); j++)
    {
      currentPoint[0] = currentPath->ElementAt(j)[0];
      currentPoint[1] = currentPath->ElementAt(j)[1];
      currentPoint[2] = 0;

      m_SliceGeometry->IndexToWorld(currentPoint, currentWorldPoint);

      contour->AddVertex(currentWorldPoint);
    } // for2

    contour->Close();

  } // for1
}
