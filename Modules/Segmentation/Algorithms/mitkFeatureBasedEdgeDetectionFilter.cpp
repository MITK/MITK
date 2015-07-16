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

#include <itkBinaryThresholdImageFilter.h>
#include <itkImage.h>

#include <SegmentationUtilities/MorphologicalOperations/mitkMorphologicalOperations.h>
#include <mitkUnstructuredGrid.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageStatisticsCalculator.h>
#include <mitkImage.h>
#include <mitkITKImageImport.h>
#include <mitkMaskImageFilter.h>
#include <mitkImageToPointCloudFilter.h>
#include <mitkUnstructuredGrid.h>
#include <mitkProgressBar.h>

#include <mitkIOUtil.h>
#include <mitkBaseData.h>
#include <itkNeighborhoodIterator.h>
#include <mitkImageToUnstructuredGridFilter.h>

#include <itkSimpleContourExtractorImageFilter.h>
#include <mitkImageCast.h>
#include <itkTimeProbe.h>

mitk::FeatureBasedEdgeDetectionFilter::FeatureBasedEdgeDetectionFilter()
{
  m_PointGrid = mitk::UnstructuredGrid::New();
  m_SegmentationMask = mitk::Image::New();
  m_thresholdImage = mitk::Image::New();
  m_TestImage = mitk::Image::New();

  this->SetNumberOfRequiredInputs(1);

  this->SetNumberOfIndexedOutputs(1);
}

mitk::FeatureBasedEdgeDetectionFilter::~FeatureBasedEdgeDetectionFilter(){}
#include <itkErodeObjectMorphologyImageFilter.h>
#include <itkDilateObjectMorphologyImageFilter.h>
void mitk::FeatureBasedEdgeDetectionFilter::GenerateData()
{
  mitk::Image::ConstPointer image = ImageToUnstructuredGridFilter::GetInput();
  mitk::Image::Pointer ncImage = const_cast<mitk::Image*>(image.GetPointer());

  //statistics
  mitk::ImageStatisticsCalculator::Pointer statCalc = mitk::ImageStatisticsCalculator::New();
  statCalc->SetImage(image);
  statCalc->SetMaskingModeToImage();
  if(m_SegmentationMask->IsEmpty())
  {
    MITK_WARN << "Please set a segmentation mask first" << std::endl;
    return;
  }
  statCalc->SetImageMask(m_SegmentationMask);
  statCalc->ComputeStatistics();

  mitk::ImageStatisticsCalculator::Statistics stats = statCalc->GetStatistics();
  double mean = stats.GetMean();
  double stdDev = stats.GetSigma();

  double upperThreshold = mean + stdDev;
  double lowerThreshold = mean - stdDev;

  //thresholding
  AccessByItk_2(ncImage.GetPointer(), ITKThresholding, lowerThreshold, upperThreshold)

  mitk::ProgressBar::GetInstance()->Progress();

  //fill holes
//  mitk::MorphologicalOperations::FillHoles(m_thresholdImage);

  mitk::ProgressBar::GetInstance()->Progress();

//  mitk::MorphologicalOperations::Closing(m_thresholdImage, 1, mitk::MorphologicalOperations::Ball);
//  mitk::MorphologicalOperations::FillHoles(m_thresholdImage);
////  mitk::MorphologicalOperations::Opening(m_thresholdImage,1,mitk::MorphologicalOperations::Ball);
///

  itk::TimeProbe clock;
  clock.Start();

  AccessByItk(m_thresholdImage, ThreadedClosing);
//  AccessByItk(m_TestImage, ThreadedOpening);
  mitk::MorphologicalOperations::FillHoles(m_TestImage);

  clock.Stop();
  std::cout << "Time needed: " << clock.GetTotal() << std::endl;

  mitk::ProgressBar::GetInstance()->Progress();

//  AccessByItk(m_thresholdImage,ContourSearch)
  AccessByItk(m_TestImage,ContourSearch)

      //m_TestImage
  mitk::ImageToUnstructuredGridFilter::Pointer i2UFilter = mitk::ImageToUnstructuredGridFilter::New();
  i2UFilter->SetInput(m_TestImage);
  i2UFilter->SetThreshold(1.0);
  i2UFilter->Update();

  m_PointGrid->SetVtkUnstructuredGrid( i2UFilter->GetOutput()->GetVtkUnstructuredGrid() );
  m_PointGrid = this->GetOutput();

  mitk::ProgressBar::GetInstance()->Progress();
}

#include <itkBinaryBallStructuringElement.h>

template <typename TPixel, unsigned int VImageDimension>
void mitk::FeatureBasedEdgeDetectionFilter::ThreadedClosing( itk::Image<TPixel, VImageDimension>* originalImage)
{
  typedef itk::BinaryBallStructuringElement<TPixel, VImageDimension> myKernelType;

  myKernelType ball;
  ball.SetRadius(1);
  ball.CreateStructuringElement();

  typedef typename itk::Image<TPixel, VImageDimension> ImageType;

  typename itk::DilateObjectMorphologyImageFilter<ImageType, ImageType, myKernelType>::Pointer dilationFilter = itk::DilateObjectMorphologyImageFilter<ImageType, ImageType, myKernelType>::New();
  dilationFilter->SetInput(originalImage);
  dilationFilter->SetKernel(ball);
  dilationFilter->Update();

  typename itk::Image<TPixel, VImageDimension>::Pointer dilatedImage = dilationFilter->GetOutput();

  typename itk::ErodeObjectMorphologyImageFilter<ImageType, ImageType, myKernelType>::Pointer erodeFilter = itk::ErodeObjectMorphologyImageFilter<ImageType, ImageType, myKernelType>::New();
  erodeFilter->SetInput(dilatedImage);
  erodeFilter->SetKernel(ball);
  erodeFilter->Update();

  mitk::CastToMitkImage(erodeFilter->GetOutput(), m_TestImage);
}

template <typename TPixel, unsigned int VImageDimension>
void mitk::FeatureBasedEdgeDetectionFilter::ThreadedOpening( itk::Image<TPixel, VImageDimension>* originalImage)
{
  typedef itk::BinaryBallStructuringElement<TPixel, VImageDimension> myKernelType;

  myKernelType ball;
  ball.SetRadius(1);
  ball.CreateStructuringElement();

  typedef typename itk::Image<TPixel, VImageDimension> ImageType;

  typename itk::ErodeObjectMorphologyImageFilter<ImageType, ImageType, myKernelType>::Pointer erodeFilter = itk::ErodeObjectMorphologyImageFilter<ImageType, ImageType, myKernelType>::New();
  erodeFilter->SetInput(originalImage);
  erodeFilter->SetKernel(ball);
  erodeFilter->Update();

  typename itk::Image<TPixel, VImageDimension>::Pointer erodedImage = erodeFilter->GetOutput();

  typename itk::DilateObjectMorphologyImageFilter<ImageType, ImageType, myKernelType>::Pointer dilationFilter = itk::DilateObjectMorphologyImageFilter<ImageType, ImageType, myKernelType>::New();
  dilationFilter->SetInput(erodedImage);
  dilationFilter->SetKernel(ball);
  dilationFilter->Update();

  mitk::CastToMitkImage(erodeFilter->GetOutput(), m_TestImage);
}

template <typename TPixel, unsigned int VImageDimension>
void mitk::FeatureBasedEdgeDetectionFilter::ContourSearch( itk::Image<TPixel, VImageDimension>* originalImage)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::BinaryContourImageFilter<ImageType, ImageType> binaryContourImageFilterType;
  typedef unsigned char OutputPixelType;
  typedef itk::Image< OutputPixelType, VImageDimension > OutputImageType;

  typename binaryContourImageFilterType::Pointer binaryContourFilter = binaryContourImageFilterType::New();
  binaryContourFilter->SetInput(originalImage);
  binaryContourFilter->SetForegroundValue(1);
  binaryContourFilter->SetBackgroundValue(0);
  binaryContourFilter->Update();

  typename itk::Image<TPixel, VImageDimension>::Pointer itkImage = itk::Image<TPixel, VImageDimension>::New();
  itkImage->Graft(binaryContourFilter->GetOutput());

  mitk::CastToMitkImage(itkImage, m_TestImage);
}

template <typename TPixel, unsigned int VImageDimension>
void mitk::FeatureBasedEdgeDetectionFilter::ITKThresholding( itk::Image<TPixel, VImageDimension>* originalImage, double lower, double upper)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<unsigned char, VImageDimension> SegmentationType;
  typedef itk::BinaryThresholdImageFilter<ImageType, SegmentationType> ThresholdFilterType;

  if( typeid(TPixel) != typeid(float) && typeid(TPixel) != typeid(double))
  {
    //round the thresholds if we have nor a float or double image
    lower = std::floor(lower + 0.5);
    upper = std::floor(upper - 0.5);
  }
  if(lower >= upper)
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

  mitk::CastToMitkImage(filter->GetOutput(),m_thresholdImage);
}

void mitk::FeatureBasedEdgeDetectionFilter::SetSegmentationMask(mitk::Image::Pointer segmentation)
{
  this->m_SegmentationMask = segmentation;
}

void mitk::FeatureBasedEdgeDetectionFilter::GenerateOutputInformation()
{
  Superclass::GenerateOutputInformation();
}
