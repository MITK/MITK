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

// ITK
#include <itkLabelStatisticsImageFilter.h>

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
  typedef typename FilterType::HistogramType HistogramType;
  typedef typename HistogramType::IndexType HIndexType;

  typename MaskType::Pointer maskImage = MaskType::New();
  mitk::CastToItkImage(mask, maskImage);

  typename FilterType::Pointer labelStatisticsImageFilter = FilterType::New();
  labelStatisticsImageFilter->SetInput( itkImage );
  labelStatisticsImageFilter->SetLabelInput(maskImage);
  labelStatisticsImageFilter->Update();

  double volume = labelStatisticsImageFilter->GetCount(1);
  for (int i = 0; i < VImageDimension; ++i)
  {
    volume *= itkImage->GetSpacing()[i];
  }

  featureList.push_back(std::make_pair("Volumetric Features Volume (pixel based)",volume));
}

mitk::GIFVolumetricStatistics::GIFVolumetricStatistics()
{
}

mitk::GIFVolumetricStatistics::FeatureListType mitk::GIFVolumetricStatistics::CalculateFeatures(const Image::Pointer & image, const Image::Pointer &mask)
{
  FeatureListType featureList;

  AccessByItk_2(image, CalculateVolumeStatistic, mask, featureList);

  vtkSmartPointer<vtkImageMarchingCubes> mesher = vtkSmartPointer<vtkImageMarchingCubes>::New();
  vtkSmartPointer<vtkMassProperties> stats = vtkSmartPointer<vtkMassProperties>::New();
  mesher->SetInputData(mask->GetVtkImageData());
  stats->SetInputConnection(mesher->GetOutputPort());
  stats->Update();

  double pi = vnl_math::pi;

  double meshVolume = stats->GetVolume();
  double meshSurf = stats->GetSurfaceArea();
  double pixelVolume = featureList[0].second;

  double compactness1 = pixelVolume / ( std::sqrt(pi) * std::pow(meshSurf, 2.0/3.0));
  double compactness2 = 36*pi*pixelVolume*pixelVolume/meshSurf/meshSurf/meshSurf;

  double sphericity=std::pow(pi,1/3.0) *std::pow(6*pixelVolume, 2.0/3.0) / meshSurf;
  double surfaceToVolume = meshSurf / pixelVolume;

  featureList.push_back(std::make_pair("Volumetric Features Volume (mesh based)",meshVolume));
  featureList.push_back(std::make_pair("Volumetric Features Surface area",meshSurf));
  featureList.push_back(std::make_pair("Volumetric Features Surface to volume ratio",surfaceToVolume));
  featureList.push_back(std::make_pair("Volumetric Features Sphericity",sphericity));
  featureList.push_back(std::make_pair("Volumetric Features Compactness 1",compactness1));
  featureList.push_back(std::make_pair("Volumetric Features Compactness 2",compactness2));

  return featureList;
}

mitk::GIFVolumetricStatistics::FeatureNameListType mitk::GIFVolumetricStatistics::GetFeatureNames()
{
  FeatureNameListType featureList;
  featureList.push_back("FirstOrder Minimum");
  featureList.push_back("FirstOrder Maximum");
  featureList.push_back("FirstOrder Mean");
  featureList.push_back("FirstOrder Variance");
  featureList.push_back("FirstOrder Sum");
  featureList.push_back("FirstOrder Median");
  featureList.push_back("FirstOrder Standard deviation");
  featureList.push_back("FirstOrder No. of Voxel");
  return featureList;
}