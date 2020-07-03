/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkBooleanOperation.h"
#include <itkAndImageFilter.h>
#include <itkNotImageFilter.h>
#include <itkOrImageFilter.h>
#include <mitkExceptionMacro.h>
#include <mitkImageCast.h>
#include <mitkImageTimeSelector.h>

typedef itk::Image<mitk::Label::PixelType, 3> ImageType;

static mitk::Image::Pointer Get3DSegmentation(mitk::Image::Pointer segmentation, mitk::TimePointType time)
{
  if (segmentation->GetDimension() != 4)
    return segmentation;

  auto imageTimeSelector = mitk::ImageTimeSelector::New();
  imageTimeSelector->SetInput(segmentation);
  imageTimeSelector->SetTimeNr(segmentation->GetTimeGeometry()->TimePointToTimeStep(time));

  imageTimeSelector->UpdateLargestPossibleRegion();

  return imageTimeSelector->GetOutput();
}

static ImageType::Pointer CastTo3DItkImage(mitk::Image::Pointer segmentation, mitk::TimePointType time)
{
  ImageType::Pointer result;
  mitk::CastToItkImage(Get3DSegmentation(segmentation, time), result);
  return result;
}

mitk::BooleanOperation::BooleanOperation(Type type,
                                         mitk::Image::Pointer segmentationA,
                                         mitk::Image::Pointer segmentationB,
                                         TimePointType time)
  : m_Type(type), m_SegmentationA(segmentationA), m_SegmentationB(segmentationB), m_TimePoint(time)
{
  this->ValidateSegmentations();
}

mitk::BooleanOperation::~BooleanOperation()
{
}

mitk::LabelSetImage::Pointer mitk::BooleanOperation::GetResult() const
{
  switch (m_Type)
  {
    case Difference:
      return this->GetDifference();

    case Intersection:
      return this->GetIntersection();

    case Union:
      return this->GetUnion();

    default:
      mitkThrow() << "Unknown boolean operation type '" << m_Type << "'!";
  }
}

mitk::LabelSetImage::Pointer mitk::BooleanOperation::GetDifference() const
{
  auto input1 = CastTo3DItkImage(m_SegmentationA, m_TimePoint);
  auto input2 = CastTo3DItkImage(m_SegmentationB, m_TimePoint);

  auto notFilter = itk::NotImageFilter<ImageType, ImageType>::New();
  notFilter->SetInput(input2);

  auto andFilter = itk::AndImageFilter<ImageType, ImageType>::New();
  andFilter->SetInput1(input1);
  andFilter->SetInput2(notFilter->GetOutput());

  andFilter->UpdateLargestPossibleRegion();

  auto tempResult = Image::New();
  CastToMitkImage<ImageType>(andFilter->GetOutput(), tempResult);

  tempResult->DisconnectPipeline();

  auto result = mitk::LabelSetImage::New();
  result->InitializeByLabeledImage(tempResult);

  return result;
}

mitk::LabelSetImage::Pointer mitk::BooleanOperation::GetIntersection() const
{
  auto input1 = CastTo3DItkImage(m_SegmentationA, m_TimePoint);
  auto input2 = CastTo3DItkImage(m_SegmentationB, m_TimePoint);

  auto andFilter = itk::AndImageFilter<ImageType, ImageType>::New();
  andFilter->SetInput1(input1);
  andFilter->SetInput2(input2);

  andFilter->UpdateLargestPossibleRegion();

  auto tempResult = Image::New();
  CastToMitkImage<ImageType>(andFilter->GetOutput(), tempResult);

  tempResult->DisconnectPipeline();

  auto result = mitk::LabelSetImage::New();
  result->InitializeByLabeledImage(tempResult);

  return result;
}

mitk::LabelSetImage::Pointer mitk::BooleanOperation::GetUnion() const
{
  auto input1 = CastTo3DItkImage(m_SegmentationA, m_TimePoint);
  auto input2 = CastTo3DItkImage(m_SegmentationB, m_TimePoint);

  auto orFilter = itk::OrImageFilter<ImageType, ImageType>::New();
  orFilter->SetInput1(input1);
  orFilter->SetInput2(input2);

  orFilter->UpdateLargestPossibleRegion();

  auto tempResult = Image::New();
  CastToMitkImage<ImageType>(orFilter->GetOutput(), tempResult);

  tempResult->DisconnectPipeline();

  auto result = mitk::LabelSetImage::New();
  result->InitializeByLabeledImage(tempResult);

  return result;
}

void mitk::BooleanOperation::ValidateSegmentation(mitk::Image::Pointer segmentation) const
{
  if (segmentation.IsNull())
    mitkThrow() << "Segmentation is nullptr!";

  if (segmentation->GetImageDescriptor()->GetNumberOfChannels() != 1)
    mitkThrow() << "Segmentation has more than one channel!";

  auto pixelType = segmentation->GetImageDescriptor()->GetChannelDescriptor().GetPixelType();

  if (pixelType.GetPixelType() != itk::ImageIOBase::SCALAR ||
      (pixelType.GetComponentType() != itk::ImageIOBase::UCHAR &&
       pixelType.GetComponentType() != itk::ImageIOBase::USHORT))
    mitkThrow() << "Segmentation is neither of type 'unsigned char' nor type 'unsigned short'!";

  auto dimension = segmentation->GetDimension();

  if (dimension > 4)
    mitkThrow() << "Segmentation has more than four dimensions!";

  if (dimension < 3)
    mitkThrow() << "Segmentation has less than three dimensions!";

  if (!segmentation->GetTimeGeometry()->IsValidTimePoint(m_TimePoint))
    mitkThrow() << "Segmentation is not defined for specified time point. Time point: " << m_TimePoint;
}

void mitk::BooleanOperation::ValidateSegmentations() const
{
  this->ValidateSegmentation(m_SegmentationA);
  this->ValidateSegmentation(m_SegmentationB);

  if (m_SegmentationA->GetDimension() != m_SegmentationB->GetDimension())
    mitkThrow() << "Segmentations have different dimensions!";

  const auto geometryA = m_SegmentationA->GetTimeGeometry()->GetGeometryForTimePoint(m_TimePoint);
  const auto geometryB = m_SegmentationB->GetTimeGeometry()->GetGeometryForTimePoint(m_TimePoint);
  if (!mitk::Equal(*(geometryA.GetPointer()), *(geometryB.GetPointer()),eps,false))
  {
    mitkThrow() << "Segmentations have different geometries and cannot be used for boolean operations!";
  }
}
