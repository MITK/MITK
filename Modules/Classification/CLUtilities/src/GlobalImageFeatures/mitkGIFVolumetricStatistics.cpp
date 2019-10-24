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

#include <mitkGIFVolumetricStatistics.h>

// MITK
#include <mitkITKImageImport.h>
#include <mitkImageCast.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageVtkAccessor.h>

// ITK
#include <itkLabelStatisticsImageFilter.h>
#include <itkNeighborhoodIterator.h>

// VTK
#include <vtkSmartPointer.h>
#include <vtkImageMarchingCubes.h>
#include <vtkMassProperties.h>

// STL
#include <sstream>
#include <vnl/vnl_math.h>

template<typename TPixel, unsigned int VImageDimension>
void
  CalculateVolumeStatistic(itk::Image<TPixel, VImageDimension>* itkImage, mitk::Image::Pointer mask, mitk::GIFVolumetricStatistics::FeatureListType & featureList)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<int, VImageDimension> MaskType;
  typedef itk::LabelStatisticsImageFilter<ImageType, MaskType> FilterType;

  typename MaskType::Pointer maskImage = MaskType::New();
  mitk::CastToItkImage(mask, maskImage);

  typename FilterType::Pointer labelStatisticsImageFilter = FilterType::New();
  labelStatisticsImageFilter->SetInput( itkImage );
  labelStatisticsImageFilter->SetLabelInput(maskImage);
  labelStatisticsImageFilter->Update();

  double volume = labelStatisticsImageFilter->GetCount(1);
  for (int i = 0; i < (int)(VImageDimension); ++i)
  {
    volume *= itkImage->GetSpacing()[i];
  }

  featureList.push_back(std::make_pair("Volumetric Features Volume (pixel based)",volume));
}

template<typename TPixel, unsigned int VImageDimension>
void
  CalculateLargestDiameter(itk::Image<TPixel, VImageDimension>* mask, mitk::GIFVolumetricStatistics::FeatureListType & featureList)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef typename ImageType::PointType PointType;
  typename ImageType::SizeType radius;
  for (int i=0; i < (int)VImageDimension; ++i)
    radius[i] = 1;
  itk::NeighborhoodIterator<ImageType> iterator(radius,mask, mask->GetRequestedRegion());
  std::vector<PointType> borderPoints;
  while(!iterator.IsAtEnd())
  {
    if (iterator.GetCenterPixel() == 0)
    {
      ++iterator;
      continue;
    }

    bool border = false;
    for (int i = 0; i < (int)(iterator.Size()); ++i)
    {
      if (iterator.GetPixel(i) == 0)
      {
        border = true;
        break;
      }
    }
    if (border)
    {
      auto centerIndex = iterator.GetIndex();
      PointType centerPoint;
      mask->TransformIndexToPhysicalPoint(centerIndex, centerPoint );
      borderPoints.push_back(centerPoint);
    }
    ++iterator;
  }

  double longestDiameter = 0;
  unsigned long numberOfBorderPoints = borderPoints.size();
  for (int i = 0; i < (int)numberOfBorderPoints; ++i)
  {
    auto point = borderPoints[i];
    for (int j = i; j < (int)numberOfBorderPoints; ++j)
    {
      double newDiameter=point.EuclideanDistanceTo(borderPoints[j]);
      if (newDiameter > longestDiameter)
        longestDiameter = newDiameter;
    }
  }

  featureList.push_back(std::make_pair("Volumetric Features Maximum 3D diameter",longestDiameter));
}

mitk::GIFVolumetricStatistics::GIFVolumetricStatistics()
{
}

mitk::GIFVolumetricStatistics::FeatureListType mitk::GIFVolumetricStatistics::CalculateFeatures(const Image::Pointer & image, const Image::Pointer &mask)
{
  FeatureListType featureList;

  AccessByItk_2(image, CalculateVolumeStatistic, mask, featureList);
  AccessByItk_1(mask, CalculateLargestDiameter, featureList);

  vtkSmartPointer<vtkImageMarchingCubes> mesher = vtkSmartPointer<vtkImageMarchingCubes>::New();
  vtkSmartPointer<vtkMassProperties> stats = vtkSmartPointer<vtkMassProperties>::New();
  mitk::ImageVtkAccessor accessor(mask);
  {
    mitk::ImageAccessLock lock(&accessor);
    mesher->SetInputData(accessor.getVtkImageData());
    stats->SetInputConnection(mesher->GetOutputPort());
    stats->Update();
  }

  double pi = vnl_math::pi;

  double meshVolume = stats->GetVolume();
  double meshSurf = stats->GetSurfaceArea();
  double pixelVolume = featureList[0].second;

  double compactness1 = pixelVolume / ( std::sqrt(pi) * std::pow(meshSurf, 2.0/3.0));
  double compactness2 = 36*pi*pixelVolume*pixelVolume/meshSurf/meshSurf/meshSurf;

  double sphericity=std::pow(pi,1/3.0) *std::pow(6*pixelVolume, 2.0/3.0) / meshSurf;
  double surfaceToVolume = meshSurf / pixelVolume;
  double sphericalDisproportion = meshSurf / 4 / pi / std::pow(3.0 / 4.0 / pi * pixelVolume, 2.0 / 3.0);

  featureList.push_back(std::make_pair("Volumetric Features Volume (mesh based)",meshVolume));
  featureList.push_back(std::make_pair("Volumetric Features Surface area",meshSurf));
  featureList.push_back(std::make_pair("Volumetric Features Surface to volume ratio",surfaceToVolume));
  featureList.push_back(std::make_pair("Volumetric Features Sphericity",sphericity));
  featureList.push_back(std::make_pair("Volumetric Features Compactness 1",compactness1));
  featureList.push_back(std::make_pair("Volumetric Features Compactness 2",compactness2));
  featureList.push_back(std::make_pair("Volumetric Features Spherical disproportion",sphericalDisproportion));

  return featureList;
}

mitk::GIFVolumetricStatistics::FeatureNameListType mitk::GIFVolumetricStatistics::GetFeatureNames()
{
  FeatureNameListType featureList;
  featureList.push_back("Volumetric Features Compactness 1");
  featureList.push_back("Volumetric Features Compactness 2");
  featureList.push_back("Volumetric Features Sphericity");
  featureList.push_back("Volumetric Features Surface to volume ratio");
  featureList.push_back("Volumetric Features Surface area");
  featureList.push_back("Volumetric Features Volume (mesh based)");
  featureList.push_back("Volumetric Features Volume (pixel based)");
  featureList.push_back("Volumetric Features Spherical disproportion");
  featureList.push_back("Volumetric Features Maximum 3D diameter");
  return featureList;
}
