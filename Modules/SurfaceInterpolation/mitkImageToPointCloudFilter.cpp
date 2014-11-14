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

  this->SetNumberOfRequiredInputs(1);
  this->SetNumberOfRequiredOutputs(1);

  this->SetNthOutput(0, m_PointSurface);
}

mitk::ImageToPointCloudFilter::~ImageToPointCloudFilter(){}

void mitk::ImageToPointCloudFilter::GenerateData()
{
  mitk::Image::ConstPointer image = ImageToSurfaceFilter::GetInput();

  if ( image.IsNull() )
  {
    MITK_ERROR << "mitk::ImageToContourFilter: No input available. Please set the input!" << std::endl;
    return;
  }

  ImageType::Pointer itkImage = ImageType::New();
  mitk::Image::Pointer final = mitk::Image::New();

  CastToItkImage( image, itkImage );

  ImagePTypeToFloatPTypeCasterType::Pointer caster = ImagePTypeToFloatPTypeCasterType::New();
  caster->SetInput( itkImage );
  caster->Update();
  FloatImageType::Pointer fImage = caster->GetOutput();

  LaplacianFilterType::Pointer laplacianFilter = LaplacianFilterType::New();
  laplacianFilter->SetInput( fImage );
  laplacianFilter->UpdateLargestPossibleRegion();
  final = mitk::ImportItkImage(laplacianFilter->GetOutput())->Clone();

  mitk::ImageStatisticsCalculator::Pointer statCalc = mitk::ImageStatisticsCalculator::New();
  statCalc->SetImage(final);
  statCalc->ComputeStatistics();
  mitk::ImageStatisticsCalculator::Statistics stats = statCalc->GetStatistics();
  double mean = stats.GetMean();
  double stdDev = stats.GetSigma();

  AccessByItk_2(final, StdDeviations, mean, stdDev);
}

template<typename TPixel, unsigned int VImageDimension>
void mitk::ImageToPointCloudFilter::StdDeviations(itk::Image<TPixel, VImageDimension>* image, double mean, double stdDev)
{
  typedef itk::Image<TPixel, VImageDimension> InputImageType;
  itk::ImageRegionIterator<InputImageType> it(image,
                                              image->GetRequestedRegion());

  double upperThreshold = mean + stdDev * 2;
  double lowerThreshold = mean - stdDev * 2;

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
  }

  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
  polyData->SetPoints(points);
  m_PointSurface->SetVtkPolyData(polyData);
}

void mitk::ImageToPointCloudFilter::GenerateOutputInformation()
{
  Superclass::GenerateOutputInformation();
}
