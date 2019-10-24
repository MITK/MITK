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

#include "mitkMorphologicalOperations.h"
#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>
#include <mitkImageTimeSelector.h>

#include <itkBinaryBallStructuringElement.h>
#include <itkBinaryCrossStructuringElement.h>
#include <itkBinaryDilateImageFilter.h>
#include <itkBinaryErodeImageFilter.h>
#include <itkBinaryFillholeImageFilter.h>
#include <itkBinaryMorphologicalClosingImageFilter.h>
#include <itkBinaryMorphologicalOpeningImageFilter.h>
#include <itkLabelMedianImageFilter.h>

#include <itkBinaryThresholdImageFilter.h>
#include <itkConnectedComponentImageFilter.h>
#include <itkLabelImageToShapeLabelMapFilter.h>
#include <itkLabelShapeKeepNObjectsImageFilter.h>


void mitk::MorphologicalOperations::Closing(mitk::Image::Pointer& image, int factor, mitk::MorphologicalOperations::StructuralElementType structuralElement)
{
  MITK_INFO << "Start Closing...";

  int timeSteps = static_cast<int>(image->GetTimeSteps());

  if (timeSteps > 1)
  {
    mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
    timeSelector->SetInput(image);

    for (int t = 0; t < timeSteps; ++t)
    {
      MITK_INFO << "  Processing time step " << t;

      timeSelector->SetTimeNr(t);
      timeSelector->Update();

      mitk::Image::Pointer img3D = timeSelector->GetOutput();
      img3D->DisconnectPipeline();

      AccessByItk_3(img3D, itkClosing, img3D, factor, structuralElement);

      mitk::ImageRegionAccessor accessor(img3D);
      mitk::ImageAccessLock lock(&accessor);
      image->SetVolume(accessor.getData(), t);
    }
  }
  else
  {
    AccessByItk_3(image, itkClosing, image, factor, structuralElement);
  }

  MITK_INFO << "Finished Closing";
}

void mitk::MorphologicalOperations::Erode(mitk::Image::Pointer& image, int factor, mitk::MorphologicalOperations::StructuralElementType structuralElement)
{
  MITK_INFO << "Start Erode...";

  int timeSteps = static_cast<int>(image->GetTimeSteps());

  if (timeSteps > 1)
  {
    mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
    timeSelector->SetInput(image);

    for (int t = 0; t < timeSteps; ++t)
    {
      MITK_INFO << "  Processing time step " << t;

      timeSelector->SetTimeNr(t);
      timeSelector->Update();

      mitk::Image::Pointer img3D = timeSelector->GetOutput();
      img3D->DisconnectPipeline();

      AccessByItk_3(img3D, itkErode,  img3D, factor, structuralElement);

      mitk::ImageRegionAccessor accessor(img3D);
      mitk::ImageAccessLock lock(&accessor);
      image->SetVolume(accessor.getData(), t);
    }
  }
  else
  {
    AccessByItk_3(image, itkErode, image, factor, structuralElement);
  }

  MITK_INFO << "Finished Erode";
}

void mitk::MorphologicalOperations::Dilate(mitk::Image::Pointer& image, int factor, mitk::MorphologicalOperations::StructuralElementType structuralElement)
{
  MITK_INFO << "Start Dilate...";

  int timeSteps = static_cast<int>(image->GetTimeSteps());

  if (timeSteps > 1)
  {
    mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
    timeSelector->SetInput(image);

    for (int t = 0; t < timeSteps; ++t)
    {
      MITK_INFO << "  Processing time step " << t;

      timeSelector->SetTimeNr(t);
      timeSelector->Update();

      mitk::Image::Pointer img3D = timeSelector->GetOutput();
      img3D->DisconnectPipeline();

      AccessByItk_3(img3D, itkDilate, img3D, factor, structuralElement);

      mitk::ImageRegionAccessor accessor(img3D);
      mitk::ImageAccessLock lock(&accessor);
      image->SetVolume(accessor.getData(), t);
    }
  }
  else
  {
    AccessByItk_3(image, itkDilate, image, factor, structuralElement);
  }

  MITK_INFO << "Finished Dilate";
}

void mitk::MorphologicalOperations::Opening(mitk::Image::Pointer& image, int factor, mitk::MorphologicalOperations::StructuralElementType structuralElement)
{
  MITK_INFO << "Start Opening...";

  int timeSteps = static_cast<int>(image->GetTimeSteps());

  if (timeSteps > 1)
  {
    mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
    timeSelector->SetInput(image);

    for (int t = 0; t < timeSteps; ++t)
    {
      MITK_INFO << "  Processing time step " << t;

      timeSelector->SetTimeNr(t);
      timeSelector->Update();

      mitk::Image::Pointer img3D = timeSelector->GetOutput();
      img3D->DisconnectPipeline();

      AccessByItk_3(img3D, itkOpening, img3D, factor, structuralElement);

      mitk::ImageRegionAccessor accessor(img3D);
      mitk::ImageAccessLock lock(&accessor);
      image->SetVolume(accessor.getData(), t);
    }
  }
  else
  {
    AccessByItk_3(image, itkOpening, image, factor, structuralElement);
  }

  MITK_INFO << "Finished Opening";
}

void mitk::MorphologicalOperations::FillHoles(mitk::Image::Pointer &image)
{
  MITK_INFO << "Start FillHole...";

  int timeSteps = static_cast<int>(image->GetTimeSteps());

  if (timeSteps > 1)
  {
    mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
    timeSelector->SetInput(image);

    for (int t = 0; t < timeSteps; ++t)
    {
      MITK_INFO << "  Processing time step " << t;

      timeSelector->SetTimeNr(t);
      timeSelector->Update();

      mitk::Image::Pointer img3D = timeSelector->GetOutput();
      img3D->DisconnectPipeline();

      AccessByItk_1(img3D, itkFillHoles, img3D);

      mitk::ImageRegionAccessor accessor(img3D);
      mitk::ImageAccessLock lock(&accessor);
      image->SetVolume(accessor.getData(), t);
    }
  }
  else
  {
    AccessByItk_1(image, itkFillHoles, image);
  }

  MITK_INFO << "Finished FillHole";
}

void mitk::MorphologicalOperations::RemoveFragments(mitk::Image::Pointer& image, int percent)
{
  MITK_INFO << "Start RemoveFragments...";

  int timeSteps = static_cast<int>(image->GetTimeSteps());

  if (timeSteps > 1)
  {
    mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
    timeSelector->SetInput(image);

    for (int t = 0; t < timeSteps; ++t)
    {
      MITK_INFO << "  Processing time step " << t;

      timeSelector->SetTimeNr(t);
      timeSelector->Update();

      mitk::Image::Pointer img3D = timeSelector->GetOutput();
      img3D->DisconnectPipeline();

      AccessByItk_2(img3D, itkRemoveFragments, percent, img3D);

      mitk::ImageRegionAccessor accessor(img3D);
      mitk::ImageAccessLock lock(&accessor);
      image->SetVolume(accessor.getData(), t);
    }
  } else
  {
    AccessByItk_2(image, itkRemoveFragments, percent, image);
  }

  MITK_INFO << "Finished RemoveFragments";
}

void mitk::MorphologicalOperations::WeightedMedian(mitk::Image::Pointer& image, int factor, int centerWeight)
{
  MITK_INFO << "Start WeihtedMedian...";

  int timeSteps = static_cast<int>(image->GetTimeSteps());

  if (timeSteps > 1)
  {
    mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
    timeSelector->SetInput(image);

    for (int t = 0; t < timeSteps; ++t)
    {
      MITK_INFO << "  Processing time step " << t;

      timeSelector->SetTimeNr(t);
      timeSelector->Update();

      mitk::Image::Pointer img3D = timeSelector->GetOutput();
      img3D->DisconnectPipeline();

      AccessByItk_3(img3D, itkWeightedMedian, img3D, factor, centerWeight);

      mitk::ImageRegionAccessor accessor(img3D);
      mitk::ImageAccessLock lock(&accessor);
      image->SetVolume(accessor.getData(), t);
    }
  } else
  {
    AccessByItk_3(image, itkWeightedMedian, image, factor, centerWeight);
  }

  MITK_INFO << "Finished WeihtedMedian";
}

template<typename TPixel, unsigned int VDimension>
void mitk::MorphologicalOperations::itkClosing(itk::Image<TPixel, VDimension>* sourceImage, mitk::Image::Pointer& resultImage, int factor, mitk::MorphologicalOperations::StructuralElementType structuralElementFlags)
{
  typedef itk::Image<TPixel, VDimension> ImageType;
  typedef itk::BinaryBallStructuringElement<TPixel, VDimension> BallType;
  typedef itk::BinaryCrossStructuringElement<TPixel, VDimension> CrossType;
  typedef typename itk::BinaryMorphologicalClosingImageFilter<ImageType, ImageType, BallType> BallClosingFilterType;
  typedef typename itk::BinaryMorphologicalClosingImageFilter<ImageType, ImageType, CrossType> CrossClosingFilterType;

  if (structuralElementFlags & (Ball_Axial | Ball_Coronal |  Ball_Sagital))
  {
    BallType ball = CreateStructuringElement<BallType>(structuralElementFlags,factor);

    typename BallClosingFilterType::Pointer closingFilter = BallClosingFilterType::New();
    closingFilter->SetKernel(ball);
    closingFilter->SetInput(sourceImage);
    closingFilter->SetForegroundValue(1);
    closingFilter->UpdateLargestPossibleRegion();

    mitk::CastToMitkImage(closingFilter->GetOutput(), resultImage);
  }
  else
  {
    CrossType cross = CreateStructuringElement<CrossType>(structuralElementFlags,factor);

    typename CrossClosingFilterType::Pointer closingFilter = CrossClosingFilterType::New();
    closingFilter->SetKernel(cross);
    closingFilter->SetInput(sourceImage);
    closingFilter->SetForegroundValue(1);
    closingFilter->UpdateLargestPossibleRegion();

    mitk::CastToMitkImage(closingFilter->GetOutput(), resultImage);
  }
}

template<typename TPixel, unsigned int VDimension>
void mitk::MorphologicalOperations::itkErode(itk::Image<TPixel, VDimension>* sourceImage, mitk::Image::Pointer& resultImage, int factor, mitk::MorphologicalOperations::StructuralElementType structuralElementFlags)
{
  typedef itk::Image<TPixel, VDimension> ImageType;
  typedef itk::BinaryBallStructuringElement<TPixel, VDimension> BallType;
  typedef itk::BinaryCrossStructuringElement<TPixel, VDimension> CrossType;
  typedef typename itk::BinaryErodeImageFilter<ImageType, ImageType, BallType> BallErodeFilterType;
  typedef typename itk::BinaryErodeImageFilter<ImageType, ImageType, CrossType> CrossErodeFilterType;

  if (structuralElementFlags & (Ball_Axial | Ball_Coronal |  Ball_Sagital))
  {
    BallType ball = CreateStructuringElement<BallType>(structuralElementFlags,factor);

    typename BallErodeFilterType::Pointer erodeFilter = BallErodeFilterType::New();
    erodeFilter->SetKernel(ball);
    erodeFilter->SetInput(sourceImage);
    erodeFilter->SetErodeValue(1);
    erodeFilter->UpdateLargestPossibleRegion();

    mitk::CastToMitkImage(erodeFilter->GetOutput(), resultImage);
  }
  else
  {
    CrossType cross = CreateStructuringElement<CrossType>(structuralElementFlags,factor);

    typename CrossErodeFilterType::Pointer erodeFilter = CrossErodeFilterType::New();
    erodeFilter->SetKernel(cross);
    erodeFilter->SetInput(sourceImage);
    erodeFilter->SetErodeValue(1);
    erodeFilter->UpdateLargestPossibleRegion();

    mitk::CastToMitkImage(erodeFilter->GetOutput(), resultImage);
  }
}

template<typename TPixel, unsigned int VDimension>
void mitk::MorphologicalOperations::itkDilate(itk::Image<TPixel, VDimension>* sourceImage, mitk::Image::Pointer& resultImage, int factor,  mitk::MorphologicalOperations::StructuralElementType structuralElementFlags)
{
  typedef itk::Image<TPixel, VDimension> ImageType;
  typedef itk::BinaryBallStructuringElement<TPixel, VDimension> BallType;
  typedef itk::BinaryCrossStructuringElement<TPixel, VDimension> CrossType;
  typedef typename itk::BinaryDilateImageFilter<ImageType, ImageType, BallType> BallDilateFilterType;
  typedef typename itk::BinaryDilateImageFilter<ImageType, ImageType, CrossType> CrossDilateFilterType;

  if (structuralElementFlags & (Ball_Axial | Ball_Coronal |  Ball_Sagital))
  {
    BallType ball = CreateStructuringElement<BallType>(structuralElementFlags,factor);

    typename BallDilateFilterType::Pointer dilateFilter = BallDilateFilterType::New();
    dilateFilter->SetKernel(ball);
    dilateFilter->SetInput(sourceImage);
    dilateFilter->SetDilateValue(1);
    dilateFilter->UpdateLargestPossibleRegion();

    mitk::CastToMitkImage(dilateFilter->GetOutput(), resultImage);
  }
  else
  {
    CrossType cross = CreateStructuringElement<CrossType>(structuralElementFlags,factor);

    typename CrossDilateFilterType::Pointer dilateFilter = CrossDilateFilterType::New();
    dilateFilter->SetKernel(cross);
    dilateFilter->SetInput(sourceImage);
    dilateFilter->SetDilateValue(1);
    dilateFilter->UpdateLargestPossibleRegion();

    mitk::CastToMitkImage(dilateFilter->GetOutput(), resultImage);
  }
}

template<typename TPixel, unsigned int VDimension>
void mitk::MorphologicalOperations::itkOpening(itk::Image<TPixel, VDimension>* sourceImage, mitk::Image::Pointer& resultImage, int factor, mitk::MorphologicalOperations::StructuralElementType structuralElementFlags)
{
  typedef itk::Image<TPixel, VDimension> ImageType;
  typedef itk::BinaryBallStructuringElement<TPixel, VDimension> BallType;
  typedef itk::BinaryCrossStructuringElement<TPixel, VDimension> CrossType;
  typedef typename itk::BinaryMorphologicalOpeningImageFilter<ImageType, ImageType, BallType> BallOpeningFiltertype;
  typedef typename itk::BinaryMorphologicalOpeningImageFilter<ImageType, ImageType, CrossType> CrossOpeningFiltertype;

  if (structuralElementFlags & (Ball_Axial | Ball_Coronal |  Ball_Sagital))
  {
    BallType ball = CreateStructuringElement<BallType>(structuralElementFlags,factor);

    typename BallOpeningFiltertype::Pointer openingFilter = BallOpeningFiltertype::New();
    openingFilter->SetKernel(ball);
    openingFilter->SetInput(sourceImage);
    openingFilter->SetForegroundValue(1);
    openingFilter->SetBackgroundValue(0);
    openingFilter->UpdateLargestPossibleRegion();

    mitk::CastToMitkImage(openingFilter->GetOutput(), resultImage);
  }
  else
  {
    CrossType cross = CreateStructuringElement<CrossType>(structuralElementFlags,factor);

    typename CrossOpeningFiltertype::Pointer openingFilter = CrossOpeningFiltertype::New();
    openingFilter->SetKernel(cross);
    openingFilter->SetInput(sourceImage);
    openingFilter->SetForegroundValue(1);
    openingFilter->SetBackgroundValue(0);
    openingFilter->UpdateLargestPossibleRegion();

    mitk::CastToMitkImage(openingFilter->GetOutput(), resultImage);
  }
}

template<typename TPixel, unsigned int VDimension>
void mitk::MorphologicalOperations::itkFillHoles(itk::Image<TPixel, VDimension>* sourceImage, mitk::Image::Pointer& resultImage)
{
  typedef itk::Image<TPixel, VDimension> ImageType;
  typedef typename itk::BinaryFillholeImageFilter<ImageType> FillHoleFilterType;

  typename FillHoleFilterType::Pointer fillHoleFilter = FillHoleFilterType::New();
  fillHoleFilter->SetInput(sourceImage);
  fillHoleFilter->SetForegroundValue(1);
  fillHoleFilter->UpdateLargestPossibleRegion();

  mitk::CastToMitkImage(fillHoleFilter->GetOutput(), resultImage);
}

template<typename TPixel, unsigned int VDimension>
void mitk::MorphologicalOperations::itkWeightedMedian(itk::Image<TPixel, VDimension>* sourceImage, mitk::Image::Pointer& resultImage, int factor, int centerWeight)
{
  typedef itk::Image<TPixel, VDimension> ImageType;
  typedef typename itk::LabelMedianImageFilter<ImageType, ImageType> FilterType;

  typename FilterType::Pointer filter = FilterType::New();
  typename ImageType::SizeType size;
  size.Fill(factor);
  filter->SetRadius(size);
  filter->SetCenterWeight(centerWeight);
  filter->SetInput(sourceImage);
  filter->UpdateLargestPossibleRegion();

  mitk::CastToMitkImage(filter->GetOutput(), resultImage);
}

template<typename TPixel, unsigned int VDimension>
void mitk::MorphologicalOperations::itkRemoveFragments(itk::Image<TPixel, VDimension>* sourceImage, int percent, mitk::Image::Pointer& resultImage)
{
  typedef itk::Image<TPixel, VDimension> ImageType;
  typedef itk::Image<unsigned int, VDimension> OutputImageType;
  typedef itk::ConnectedComponentImageFilter<ImageType, OutputImageType> ConnectedComponentImageFilterType;
  typename ConnectedComponentImageFilterType::Pointer connected = ConnectedComponentImageFilterType::New();
  connected->SetInput(sourceImage);
  connected->SetFullyConnected(true);
  connected->Update();
  auto connectedImage = connected->GetOutput();

  typedef itk::LabelImageToShapeLabelMapFilter<OutputImageType> LabelImageToShapeLabelMapFilterType;
  typename LabelImageToShapeLabelMapFilterType::Pointer labelShapeMapFilter = LabelImageToShapeLabelMapFilterType::New();
  labelShapeMapFilter->SetInput(connectedImage);
  labelShapeMapFilter->Update();

  // Loop over each region
  typedef std::pair<int, int> LabelInfo;
  typename std::vector<LabelInfo> labels;
  for (int i = 0; i < labelShapeMapFilter->GetOutput()->GetNumberOfLabelObjects(); i++) {
    typename LabelImageToShapeLabelMapFilterType::OutputImageType::LabelObjectType* labelObject = labelShapeMapFilter->GetOutput()->GetNthLabelObject(i);
    labels.push_back(std::pair<int, int>(i, int(labelObject->GetNumberOfPixels())));
  }
  std::sort(labels.begin(), labels.end(), [](LabelInfo &left, LabelInfo &right)
  {
    return left.second > right.second;
  });

  // Count number of remaining fragments, using given percent
  int remainingFragments = labels.size();
  for (int i = 1; i < labels.size(); ++i) {
    double ratio = double(labels[i].second) * 100.0 / double(labels[0].second);
    if (ratio < percent) {
      remainingFragments = i;
      break;
    }
  }

  typedef itk::LabelShapeKeepNObjectsImageFilter<OutputImageType> LabelShapeKeepNObjectsImageFilterType;
  typename LabelShapeKeepNObjectsImageFilterType::Pointer labelShapeKeepNObjectsImageFilter = LabelShapeKeepNObjectsImageFilterType::New();
  labelShapeKeepNObjectsImageFilter->SetInput(connectedImage);
  labelShapeKeepNObjectsImageFilter->SetBackgroundValue(0);
  labelShapeKeepNObjectsImageFilter->SetNumberOfObjects(remainingFragments);
  labelShapeKeepNObjectsImageFilter->SetAttribute(LabelShapeKeepNObjectsImageFilterType::LabelObjectType::NUMBER_OF_PIXELS);
  labelShapeKeepNObjectsImageFilter->Update();

  typedef itk::BinaryThresholdImageFilter< OutputImageType, ImageType > BinaryThresholdImageFilter;
  typename BinaryThresholdImageFilter::Pointer thresholdFilter = BinaryThresholdImageFilter::New();
  thresholdFilter->SetLowerThreshold(1);
  thresholdFilter->SetUpperThreshold(std::numeric_limits<unsigned int>::max());
  thresholdFilter->SetOutsideValue(0);
  thresholdFilter->SetInsideValue(1);
  thresholdFilter->SetInput(labelShapeKeepNObjectsImageFilter->GetOutput());
  thresholdFilter->Update();

  mitk::CastToMitkImage(thresholdFilter->GetOutput(), resultImage);
}
