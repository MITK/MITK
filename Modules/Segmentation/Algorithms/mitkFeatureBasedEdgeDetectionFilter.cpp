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

#include "mitkFeatureBasedEdgeDetectionFilter.h"

#include <SegmentationUtilities/MorphologicalOperations/mitkMorphologicalOperations.h>
#include <mitkITKImageImport.h>
#include <mitkImage.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageStatisticsCalculator.h>
#include <mitkImageToUnstructuredGridFilter.h>
#include <mitkProgressBar.h>
#include <mitkUnstructuredGrid.h>
#include <SegmentationUtilities/MorphologicalOperations/mitkMorphologicalOperations.h>
#include <mitkImageMaskGenerator.h>
#include <mitkImageStatisticsConstants.h>

#include <itkBinaryBallStructuringElement.h>
#include <itkBinaryContourImageFilter.h>
#include <itkBinaryThresholdImageFilter.h>
#include <itkDilateObjectMorphologyImageFilter.h>
#include <itkErodeObjectMorphologyImageFilter.h>
#include <itkImage.h>

mitk::FeatureBasedEdgeDetectionFilter::FeatureBasedEdgeDetectionFilter()
{
  this->SetNumberOfRequiredInputs(1);

  this->SetNumberOfIndexedOutputs(1);
}

mitk::FeatureBasedEdgeDetectionFilter::~FeatureBasedEdgeDetectionFilter()
{
}

void mitk::FeatureBasedEdgeDetectionFilter::GenerateData()
{
  mitk::Image::ConstPointer image = ImageToUnstructuredGridFilter::GetInput();

  if (m_SegmentationMask.IsNull())
  {
    MITK_WARN << "Please set a segmentation mask first" << std::endl;
    return;
  }
  // First create a threshold segmentation of the image. The threshold is determined
  // by the mean +/- stddev of the pixel values that are covered by the segmentation mask

  // Compute mean and stdDev based on the current segmentation
  mitk::ImageStatisticsCalculator::Pointer statCalc = mitk::ImageStatisticsCalculator::New();
  statCalc->SetInputImage(image);

  mitk::ImageMaskGenerator::Pointer imgMask = mitk::ImageMaskGenerator::New();
  imgMask->SetImageMask(m_SegmentationMask);

  auto stats = statCalc->GetStatistics()->GetStatisticsForTimeStep(0);
  double mean = stats.GetValueConverted<double>(mitk::ImageStatisticsConstants::MEAN());
  double stdDev = stats.GetValueConverted<double>(mitk::ImageStatisticsConstants::STANDARDDEVIATION());

  double upperThreshold = mean + stdDev;
  double lowerThreshold = mean - stdDev;

  // Perform thresholding
  mitk::Image::Pointer thresholdImage = mitk::Image::New();
  AccessByItk_3(image.GetPointer(), ITKThresholding, lowerThreshold, upperThreshold, thresholdImage)

    mitk::ProgressBar::GetInstance()
      ->Progress(2);

  // Postprocess threshold segmentation
  // First a closing will be executed
  mitk::Image::Pointer closedImage = mitk::Image::New();
  AccessByItk_1(thresholdImage, ThreadedClosing, closedImage);

  // Then we will holes that might exist
  mitk::MorphologicalOperations::FillHoles(closedImage);

  mitk::ProgressBar::GetInstance()->Progress();

  // Extract the binary edges of the resulting segmentation
  mitk::Image::Pointer edgeImage = mitk::Image::New();
  AccessByItk_1(closedImage, ContourSearch, edgeImage);

  // Convert the edge image into an unstructured grid
  mitk::ImageToUnstructuredGridFilter::Pointer i2UFilter = mitk::ImageToUnstructuredGridFilter::New();
  i2UFilter->SetInput(edgeImage);
  i2UFilter->SetThreshold(1.0);
  i2UFilter->Update();

  m_PointGrid = this->GetOutput();
  if (m_PointGrid.IsNull())
    m_PointGrid = mitk::UnstructuredGrid::New();

  m_PointGrid->SetVtkUnstructuredGrid(i2UFilter->GetOutput()->GetVtkUnstructuredGrid());

  mitk::ProgressBar::GetInstance()->Progress();
}

template <typename TPixel, unsigned int VImageDimension>
void mitk::FeatureBasedEdgeDetectionFilter::ThreadedClosing(itk::Image<TPixel, VImageDimension> *originalImage,
                                                            mitk::Image::Pointer &result)
{
  typedef itk::BinaryBallStructuringElement<TPixel, VImageDimension> myKernelType;

  myKernelType ball;
  ball.SetRadius(1);
  ball.CreateStructuringElement();

  typedef typename itk::Image<TPixel, VImageDimension> ImageType;

  typename itk::DilateObjectMorphologyImageFilter<ImageType, ImageType, myKernelType>::Pointer dilationFilter =
    itk::DilateObjectMorphologyImageFilter<ImageType, ImageType, myKernelType>::New();
  dilationFilter->SetInput(originalImage);
  dilationFilter->SetKernel(ball);
  dilationFilter->Update();

  typename itk::Image<TPixel, VImageDimension>::Pointer dilatedImage = dilationFilter->GetOutput();

  typename itk::ErodeObjectMorphologyImageFilter<ImageType, ImageType, myKernelType>::Pointer erodeFilter =
    itk::ErodeObjectMorphologyImageFilter<ImageType, ImageType, myKernelType>::New();
  erodeFilter->SetInput(dilatedImage);
  erodeFilter->SetKernel(ball);
  erodeFilter->Update();

  mitk::GrabItkImageMemory(erodeFilter->GetOutput(), result);
}

template <typename TPixel, unsigned int VImageDimension>
void mitk::FeatureBasedEdgeDetectionFilter::ContourSearch(itk::Image<TPixel, VImageDimension> *originalImage,
                                                          mitk::Image::Pointer &result)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::BinaryContourImageFilter<ImageType, ImageType> binaryContourImageFilterType;

  typename binaryContourImageFilterType::Pointer binaryContourFilter = binaryContourImageFilterType::New();
  binaryContourFilter->SetInput(originalImage);
  binaryContourFilter->SetForegroundValue(1);
  binaryContourFilter->SetBackgroundValue(0);
  binaryContourFilter->Update();

  typename itk::Image<TPixel, VImageDimension>::Pointer itkImage = itk::Image<TPixel, VImageDimension>::New();
  itkImage->Graft(binaryContourFilter->GetOutput());

  mitk::GrabItkImageMemory(itkImage, result);
}

template <typename TPixel, unsigned int VImageDimension>
void mitk::FeatureBasedEdgeDetectionFilter::ITKThresholding(const itk::Image<TPixel, VImageDimension> *originalImage,
                                                            double lower,
                                                            double upper,
                                                            mitk::Image::Pointer &result)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<unsigned short, VImageDimension> SegmentationType;
  typedef itk::BinaryThresholdImageFilter<ImageType, SegmentationType> ThresholdFilterType;

  if (typeid(TPixel) != typeid(float) && typeid(TPixel) != typeid(double))
  {
    // round the thresholds if we have nor a float or double image
    lower = std::floor(lower + 0.5);
    upper = std::floor(upper - 0.5);
  }
  if (lower >= upper)
  {
    upper = lower;
  }

  typename ThresholdFilterType::Pointer filter = ThresholdFilterType::New();
  filter->SetInput(originalImage);
  filter->SetLowerThreshold(lower);
  filter->SetUpperThreshold(upper);
  filter->SetInsideValue(1);
  filter->SetOutsideValue(0);
  filter->Update();

  mitk::GrabItkImageMemory(filter->GetOutput(), result);
}

void mitk::FeatureBasedEdgeDetectionFilter::SetSegmentationMask(mitk::Image::Pointer segmentation)
{
  this->m_SegmentationMask = segmentation;
}

void mitk::FeatureBasedEdgeDetectionFilter::GenerateOutputInformation()
{
  Superclass::GenerateOutputInformation();
}
