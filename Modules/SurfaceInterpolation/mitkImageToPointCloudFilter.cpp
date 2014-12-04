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
#include <vtkPolyVertex.h>
#include <vtkUnstructuredGrid.h>

#include <mitkDataNode.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>
#include <mitkITKImageImport.h>
#include <mitkImageStatisticsCalculator.h>
#include <vtkCellArray.h>

mitk::ImageToPointCloudFilter::ImageToPointCloudFilter():
  m_NumberOfExtractedPoints(0)
{
  m_PointGrid = mitk::UnstructuredGrid::New();
  m_Method = DetectConstant(0);
  m_EdgeImage = mitk::Image::New();
  m_EdgePoints = mitk::Image::New();

  this->SetNumberOfRequiredInputs(1);
  this->SetNumberOfRequiredOutputs(1);

  this->SetNthOutput(0, m_PointGrid);
}

mitk::ImageToPointCloudFilter::~ImageToPointCloudFilter(){}

void mitk::ImageToPointCloudFilter::GenerateData()
{
  mitk::Image::ConstPointer image = ImageToUnstructuredGridFilter::GetInput();
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

  double greatX=0, greatY=0, greatZ=0;

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

      if(worldPoint[0]>greatX)
        greatX=worldPoint[0];
      if(worldPoint[1]>greatY)
        greatY=worldPoint[1];
      if(worldPoint[2]>greatZ)
        greatZ=worldPoint[2];

      points->InsertNextPoint(worldPoint[0],worldPoint[1],worldPoint[2]);
      m_NumberOfExtractedPoints++;
    }
    ++it;
  }

  m_EdgePoints = mitk::ImportItkImage(lapFilter->GetOutput())->Clone();

  vtkSmartPointer<vtkPolyVertex> verts = vtkSmartPointer<vtkPolyVertex>::New();

  verts->GetPointIds()->SetNumberOfIds(m_NumberOfExtractedPoints);
  for(int i=0; i<m_NumberOfExtractedPoints; i++)
  {
    verts->GetPointIds()->SetId(i,i);
  }

  vtkSmartPointer<vtkUnstructuredGrid> uGrid = vtkSmartPointer<vtkUnstructuredGrid>::New();
  uGrid->Allocate(1);

  uGrid->InsertNextCell(verts->GetCellType(), verts->GetPointIds());
  uGrid->SetPoints(points);

  m_PointGrid->SetVtkUnstructuredGrid(uGrid);
}


void mitk::ImageToPointCloudFilter::GenerateOutputInformation()
{
  Superclass::GenerateOutputInformation();
}
