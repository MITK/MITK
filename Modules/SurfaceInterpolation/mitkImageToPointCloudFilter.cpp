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

#include "mitkImageToPointCloudFilter.h"

#include <itkImageIterator.h>
#include <itkImageRegionIterator.h>

#include <mitkDataNode.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>
#include <mitkITKImageImport.h>
#include <mitkImageStatisticsCalculator.h>

mitk::ImageToPointCloudFilter::ImageToPointCloudFilter()
{
  m_PointSurface = mitk::Surface::New();
  m_method = 0;
  m_EdgeImage = mitk::Image::New();

  this->SetNumberOfRequiredInputs(1);
  this->SetNumberOfRequiredOutputs(1);
//  this->SetNumberOfRequiredOutputs(2);

  this->SetNthOutput(0, m_PointSurface);
//  this->SetNthOutput(1, m_EdgeImage);
}

mitk::ImageToPointCloudFilter::~ImageToPointCloudFilter(){}

void mitk::ImageToPointCloudFilter::SetDetectionMethod(DetectionMethod method)
{
  this->m_method = method;
}

void mitk::ImageToPointCloudFilter::GenerateData()
{
  mitk::Image::ConstPointer image = ImageToSurfaceFilter::GetInput();

  if ( image.IsNull() )
  {
    MITK_ERROR << "mitk::ImageToContourFilter: No input available. Please set the input!" << std::endl;
    return;
  }

  ImageType::Pointer itkImage = ImageType::New();

  CastToItkImage( image, itkImage );

  if(!itkImage)
  {
    MITK_ERROR << "Cannot cast mitk::Image::ConstPointer to itk::Image<short, 3>::Pointer" << std::endl;
    return;
  }

  switch(m_method)
  {
  case 0:
    this->LaplacianStdDev(itkImage,2);
    break;

  case 1:
    this->LaplacianStdDev(itkImage,3);
    break;

  default:
    this->LaplacianStdDev(itkImage,2);
    break;
  }
}

void mitk::ImageToPointCloudFilter::LaplacianStdDev(itk::Image<short, 3>::Pointer image, int amount)
{
  ImagePTypeToFloatPTypeCasterType::Pointer caster = ImagePTypeToFloatPTypeCasterType::New();
  caster->SetInput( image );
  caster->Update();
  FloatImageType::Pointer fImage = caster->GetOutput();

  LaplacianFilterType::Pointer laplacianFilter = LaplacianFilterType::New();
  laplacianFilter->SetInput( fImage );
  laplacianFilter->UpdateLargestPossibleRegion();
  m_EdgeImage = mitk::ImportItkImage(laplacianFilter->GetOutput())->Clone();

  mitk::ImageStatisticsCalculator::Pointer statCalc = mitk::ImageStatisticsCalculator::New();
  statCalc->SetImage(m_EdgeImage);
  statCalc->ComputeStatistics();
  mitk::ImageStatisticsCalculator::Statistics stats = statCalc->GetStatistics();
  double mean = stats.GetMean();
  double stdDev = stats.GetSigma();

  AccessByItk_3(m_EdgeImage, StdDeviations, mean, stdDev, amount);
}

template<typename TPixel, unsigned int VImageDimension>
void mitk::ImageToPointCloudFilter::StdDeviations(itk::Image<TPixel, VImageDimension>* image, double mean, double stdDev, int amount)
{
  typedef itk::Image<TPixel, VImageDimension> InputImageType;
  itk::ImageRegionIterator<InputImageType> it(image,
                                              image->GetRequestedRegion());

  double upperThreshold = mean + stdDev * amount;
  double lowerThreshold = mean - stdDev * amount;

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

  it.GoToBegin();
  while( !it.IsAtEnd() )
  {
    if(it.Get() > lowerThreshold && it.Get() < upperThreshold)
    {
      it.Set(0);
    }
    else
    {
      it.Set(1);

      points->InsertNextPoint(it.GetIndex()[0],it.GetIndex()[1],it.GetIndex()[2]);
    }
    ++it;
  }

  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
  polyData->SetPoints(points);
  m_PointSurface->SetVtkPolyData(polyData);
}

void mitk::ImageToPointCloudFilter::GenerateOutputInformation()
{
  Superclass::GenerateOutputInformation();
}
