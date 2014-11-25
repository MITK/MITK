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
#include <vtkPolyLine.h>

#include <mitkDataNode.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>
#include <mitkITKImageImport.h>
#include <mitkImageStatisticsCalculator.h>
#include <vtkCellArray.h>

mitk::ImageToPointCloudFilter::ImageToPointCloudFilter():
  m_NumberOfExtractedPoints(0)
{
  m_PointSurface = mitk::Surface::New();
  m_Method = DetectConstant(0);
  m_EdgeImage = mitk::Image::New();
  m_EdgePoints = mitk::Image::New();

  this->SetNumberOfRequiredInputs(1);
  this->SetNumberOfRequiredOutputs(1);

  this->SetNthOutput(0, m_PointSurface);
}

mitk::ImageToPointCloudFilter::~ImageToPointCloudFilter(){}

void mitk::ImageToPointCloudFilter::GenerateData()
{
  mitk::Image::ConstPointer image = ImageToSurfaceFilter::GetInput();
  m_Geometry = image->GetGeometry();

  if ( image.IsNull() )
  {
    MITK_ERROR << "mitk::ImageToContourFilter: No input available. "
                  "Please set the input!" << std::endl;
    return;
  }

  switch(m_Method)
  {
  case 0:
    this->LaplacianStdDev(image, 2);
    break;

  case 1:
    this->LaplacianStdDev(image, 3);
    break;

  default:
    this->LaplacianStdDev(image, 2);
    break;
  }
}

void mitk::ImageToPointCloudFilter::
                    LaplacianStdDev(mitk::Image::ConstPointer image, int amount)
{
  mitk::Image::Pointer notConstImage = image->Clone();
  AccessByItk_1(notConstImage.GetPointer(), StdDeviations, amount)
}

template<typename TPixel, unsigned int VImageDimension>
void mitk::ImageToPointCloudFilter::
           StdDeviations(itk::Image<TPixel, VImageDimension>* image, int amount)
{
  typedef itk::Image<TPixel, VImageDimension> InputImageType;
  typedef itk::CastImageFilter< InputImageType, FloatImageType >
          ImagePTypeToFloatPTypeCasterType;
  typename LaplacianFilterType::Pointer lapFilter = LaplacianFilterType::New();

  typename ImagePTypeToFloatPTypeCasterType::Pointer caster =
           ImagePTypeToFloatPTypeCasterType::New();
  caster->SetInput( image );
  caster->Update();
  FloatImageType::Pointer fImage = caster->GetOutput();

  lapFilter->SetInput(fImage);
  lapFilter->UpdateLargestPossibleRegion();
  m_EdgeImage = mitk::ImportItkImage(lapFilter->GetOutput())->Clone();

  mitk::ImageStatisticsCalculator::Pointer statCalc =
                                         mitk::ImageStatisticsCalculator::New();
  statCalc->SetImage(m_EdgeImage);
  statCalc->ComputeStatistics();
  mitk::ImageStatisticsCalculator::Statistics stats = statCalc->GetStatistics();
  double mean = stats.GetMean();
  double stdDev = stats.GetSigma();

  double upperThreshold = mean + stdDev * amount;
  double lowerThreshold = mean - stdDev * amount;

  typename itk::ImageRegionIterator<FloatImageType> it(lapFilter->GetOutput(),
                                 lapFilter->GetOutput()->GetRequestedRegion());

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

      mitk::Point3D imagePoint;
      mitk::Point3D worldPoint;

      imagePoint[0] = it.GetIndex()[0];
      imagePoint[1] = it.GetIndex()[1];
      imagePoint[2] = it.GetIndex()[2];

      m_Geometry->IndexToWorld(imagePoint, worldPoint);

      points->InsertNextPoint(worldPoint[0],worldPoint[1],worldPoint[2]);
      m_NumberOfExtractedPoints++;
    }
    ++it;
  }

  vtkSmartPointer<vtkPolyLine> polyLine = vtkSmartPointer<vtkPolyLine>::New();
  polyLine->GetPointIds()->SetNumberOfIds(m_NumberOfExtractedPoints);

  for(unsigned int i = 0; i < m_NumberOfExtractedPoints; i++)
  {
    polyLine->GetPointIds()->SetId(i,i);
  }

  vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
  cells->InsertNextCell(polyLine);

  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
  polyData->SetPoints(points);
  polyData->SetLines(cells);

  m_EdgePoints = mitk::ImportItkImage(lapFilter->GetOutput())->Clone();

  polyData->BuildCells();
  polyData->BuildLinks();
  m_PointSurface->SetVtkPolyData(polyData);
}


void mitk::ImageToPointCloudFilter::GenerateOutputInformation()
{
  Superclass::GenerateOutputInformation();
}
