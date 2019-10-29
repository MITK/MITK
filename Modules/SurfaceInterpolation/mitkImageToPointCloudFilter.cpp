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

#include <itkImageRegionIterator.h>
#include <itkLaplacianImageFilter.h>

#include <vtkPolyVertex.h>
#include <vtkUnstructuredGrid.h>
#include <vtkSmartPointer.h>

#include <mitkITKImageImport.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageStatisticsCalculator.h>
#include <mitkUnstructuredGrid.h>
#include <mitkImageStatisticsConstants.h>

mitk::ImageToPointCloudFilter::ImageToPointCloudFilter() : m_Geometry(nullptr)
{
  m_Method = DetectionMethod(0);

  this->SetNumberOfRequiredInputs(1);

  this->SetNumberOfIndexedOutputs(1);
}

mitk::ImageToPointCloudFilter::~ImageToPointCloudFilter()
{
}

void mitk::ImageToPointCloudFilter::GenerateData()
{
  mitk::Image::ConstPointer image = ImageToUnstructuredGridFilter::GetInput();
  m_Geometry = image->GetGeometry();

  if (image.IsNull())
  {
    MITK_ERROR << "mitk::ImageToContourFilter: No input available. "
                  "Please set the input!"
               << std::endl;
    return;
  }

  mitk::Image::Pointer notConstImage = const_cast<mitk::Image *>(image.GetPointer());

  switch (m_Method)
  {
    case 0:
      AccessByItk_1(notConstImage.GetPointer(), StdDeviations, 2) break;

    case 1:
      AccessByItk_1(notConstImage.GetPointer(), StdDeviations, 3) break;

    case 2:
      AccessByItk_1(notConstImage.GetPointer(), StdDeviations, 4) break;

    default:
      AccessByItk_1(notConstImage.GetPointer(), StdDeviations, 2) break;
  }
}

template <typename TPixel, unsigned int VImageDimension>
void mitk::ImageToPointCloudFilter::StdDeviations(itk::Image<TPixel, VImageDimension> *image, int amount)
{
  typedef itk::Image<TPixel, VImageDimension> InputImageType;
  typedef itk::CastImageFilter<InputImageType, FloatImageType> ImagePTypeToFloatPTypeCasterType;
  typedef itk::LaplacianImageFilter<FloatImageType, FloatImageType> LaplacianFilterType;
  typename LaplacianFilterType::Pointer lapFilter = LaplacianFilterType::New();

  typename ImagePTypeToFloatPTypeCasterType::Pointer caster = ImagePTypeToFloatPTypeCasterType::New();
  caster->SetInput(image);
  caster->Update();
  FloatImageType::Pointer fImage = caster->GetOutput();

  lapFilter->SetInput(fImage);
  lapFilter->UpdateLargestPossibleRegion();
  auto edgeImage = mitk::ImportItkImage(lapFilter->GetOutput());

  mitk::ImageStatisticsCalculator::Pointer statCalc =
                                         mitk::ImageStatisticsCalculator::New();
  statCalc->SetInputImage(edgeImage.GetPointer());
  auto stats = statCalc->GetStatistics()->GetStatisticsForTimeStep(0);
  auto mean = stats.GetValueConverted<double>(mitk::ImageStatisticsConstants::MEAN());
  auto stdDev = stats.GetValueConverted<double>(mitk::ImageStatisticsConstants::STANDARDDEVIATION());

  double upperThreshold = mean + stdDev * amount;
  double lowerThreshold = mean - stdDev * amount;

  typename itk::ImageRegionIterator<FloatImageType> it(lapFilter->GetOutput(),
                                                       lapFilter->GetOutput()->GetRequestedRegion());

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

  double greatX = 0, greatY = 0, greatZ = 0;

  it.GoToBegin();
  while (!it.IsAtEnd())
  {
    if (it.Get() > lowerThreshold && it.Get() < upperThreshold)
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

      if (worldPoint[0] > greatX)
        greatX = worldPoint[0];
      if (worldPoint[1] > greatY)
        greatY = worldPoint[1];
      if (worldPoint[2] > greatZ)
        greatZ = worldPoint[2];

      points->InsertNextPoint(worldPoint[0], worldPoint[1], worldPoint[2]);
      m_NumberOfExtractedPoints++;
    }
    ++it;
  }

  /*need to build the UnstructuredGrid with at least one vertex otherwise its
  not visible*/
  vtkSmartPointer<vtkPolyVertex> verts = vtkSmartPointer<vtkPolyVertex>::New();

  verts->GetPointIds()->SetNumberOfIds(m_NumberOfExtractedPoints);
  for (int i = 0; i < m_NumberOfExtractedPoints; i++)
  {
    verts->GetPointIds()->SetId(i, i);
  }

  vtkSmartPointer<vtkUnstructuredGrid> uGrid = vtkSmartPointer<vtkUnstructuredGrid>::New();
  uGrid->Allocate(1);

  uGrid->InsertNextCell(verts->GetCellType(), verts->GetPointIds());
  uGrid->SetPoints(points);

  mitk::UnstructuredGrid::Pointer outputGrid = mitk::UnstructuredGrid::New();
  outputGrid->SetVtkUnstructuredGrid(uGrid);
  this->SetNthOutput(0, outputGrid);
}

void mitk::ImageToPointCloudFilter::GenerateOutputInformation()
{
  Superclass::GenerateOutputInformation();
}
