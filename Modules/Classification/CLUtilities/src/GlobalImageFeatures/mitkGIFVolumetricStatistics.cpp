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
#include <mitkPixelTypeMultiplex.h>
#include <mitkImagePixelReadAccessor.h>

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

// OpenCV
#include <opencv2/opencv.hpp>

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
  double voxelVolume = 1;
  for (int i = 0; i < (int)(VImageDimension); ++i)
  {
    volume *= itkImage->GetSpacing()[i];
    voxelVolume *= itkImage->GetSpacing()[i];
  }

  featureList.push_back(std::make_pair("Volumetric Features Volume (pixel based)", volume));
  featureList.push_back(std::make_pair("Volumetric Features Voxel Volume", voxelVolume));
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
  mesher->SetInputData(mask->GetVtkImageData());
  stats->SetInputConnection(mesher->GetOutputPort());
  stats->Update();

  double pi = vnl_math::pi;

  double meshVolume = stats->GetVolume();
  double meshSurf = stats->GetSurfaceArea();
  double pixelVolume = featureList[0].second;

  double compactness1 = pixelVolume / ( std::sqrt(pi) * std::pow(meshSurf, 2.0/3.0));
  //This is the definition used by Aertz. However, due to 2/3 this feature is not demensionless. Use compactness3 instead.

  double compactness2 = 36*pi*pixelVolume*pixelVolume/meshSurf/meshSurf/meshSurf;
  double compactness3 = pixelVolume / ( std::sqrt(pi) * std::pow(meshSurf, 3.0/2.0));

  double sphericity=std::pow(pi,1/3.0) *std::pow(6*pixelVolume, 2.0/3.0) / meshSurf;
  double surfaceToVolume = meshSurf / pixelVolume;
  double sphericalDisproportion = meshSurf / 4 / pi / std::pow(3.0 / 4.0 / pi * pixelVolume, 2.0 / 3.0);
  double asphericity = std::pow(compactness2,(-1.0/3.0)) - 1;

  //Calculate center of mass shift
  int xx = mask->GetDimensions()[0];
  int yy = mask->GetDimensions()[1];
  int zz = mask->GetDimensions()[2];

  mitk::Point3D geom;
  mitk::Point3D weighted;
  unsigned int noOfPx = 0;
  double totalGrayValue = 0.0;

  std::vector<cv::Point3d> pointsForPCA;

  MITK_WARN << xx << "  " << yy << "  " << zz;
  for (int x = 0; x < xx; x++)
  {
    for (int y = 0; y < yy; y++)
    {
      for (int z = 0; z < zz; z++)
      {
        itk::Image<int,3>::IndexType index;

        index[0] = x;
        index[1] = y;
        index[2] = z;

        mitk::ScalarType pxImage;
        mitk::ScalarType pxMask;

        mitkPixelTypeMultiplex5(
              mitk::FastSinglePixelAccess,
              image->GetChannelDescriptor().GetPixelType(),
              image,
              image->GetVolumeData(),
              index,
              pxImage,
              0);

        mitkPixelTypeMultiplex5(
              mitk::FastSinglePixelAccess,
              mask->GetChannelDescriptor().GetPixelType(),
              mask,
              mask->GetVolumeData(),
              index,
              pxMask,
              0);

        //Check if voxel is contained in segmentation
        if(pxMask > 0)
        {
          geom[0] += x;
          geom[1] += y;
          geom[2] += z;

          weighted[0] += pxImage*x;
          weighted[1] += pxImage*y;
          weighted[2] += pxImage*z;

          noOfPx++;
          totalGrayValue += pxImage;

          cv::Point3d tmp;
          tmp.x = x;
          tmp.y = y;
          tmp.z = z;

          pointsForPCA.push_back(tmp);
        }

      }
    }
  }

  geom[0] = geom[0] / noOfPx;
  geom[1] = geom[1] / noOfPx;
  geom[2] = geom[2] / noOfPx;

  weighted[0] = weighted[0] / totalGrayValue;
  weighted[1] = weighted[1] / totalGrayValue;
  weighted[2] = weighted[2] / totalGrayValue;

  double shift = std::sqrt(
        (geom[0] - weighted[0])*(geom[0] - weighted[0])
        + (geom[1] - weighted[1])*(geom[1] - weighted[1])
        + (geom[2] - weighted[2])*(geom[2] - weighted[2])
        );

  //Calculate PCA Features
  int sz = pointsForPCA.size();
  cv::Mat data_pts = cv::Mat(sz, 3, CV_64FC1);
  for (int i = 0; i < data_pts.rows; ++i)
  {
      data_pts.at<double>(i, 0) = pointsForPCA[i].x;
      data_pts.at<double>(i, 1) = pointsForPCA[i].y;
      data_pts.at<double>(i, 2) = pointsForPCA[i].z;
  }

  //Perform PCA analysis
  cv::PCA pca_analysis(data_pts, cv::Mat(), CV_PCA_DATA_AS_ROW);

  //Store the eigenvalues
  std::vector<double> eigen_val(3);
  for (int i = 0; i < 3; ++i)
  {
      eigen_val[i] = pca_analysis.eigenvalues.at<double>(0, i);
  }

  std::sort (eigen_val.begin(), eigen_val.end());
  double major = eigen_val[2];
  double minor = eigen_val[1];
  double least = eigen_val[0];
  double elongation = major == 0 ? 0 : minor/major;
  double flatness = major == 0 ? 0 :least/major;


  featureList.push_back(std::make_pair("Volumetric Features Volume (mesh based)",meshVolume));
  featureList.push_back(std::make_pair("Volumetric Features Surface area",meshSurf));
  featureList.push_back(std::make_pair("Volumetric Features Surface to volume ratio",surfaceToVolume));
  featureList.push_back(std::make_pair("Volumetric Features Sphericity",sphericity));
  featureList.push_back(std::make_pair("Volumetric Features Asphericity",asphericity));
  featureList.push_back(std::make_pair("Volumetric Features Compactness 1",compactness1));
  featureList.push_back(std::make_pair("Volumetric Features Compactness 2",compactness2));
  featureList.push_back(std::make_pair("Volumetric Features Compactness 3",compactness3));
  featureList.push_back(std::make_pair("Volumetric Features Spherical disproportion",sphericalDisproportion));
  featureList.push_back(std::make_pair("Volumetric Features Center of mass Shift",shift));
  featureList.push_back(std::make_pair("Volumetric Features PCA Major Axis",major));
  featureList.push_back(std::make_pair("Volumetric Features PCA Minor Axis",minor));
  featureList.push_back(std::make_pair("Volumetric Features PCA Least Axis",least));
  featureList.push_back(std::make_pair("Volumetric Features PCA Elongation",elongation));
  featureList.push_back(std::make_pair("Volumetric Features PCA Flatnes",flatness));


  return featureList;
}

mitk::GIFVolumetricStatistics::FeatureNameListType mitk::GIFVolumetricStatistics::GetFeatureNames()
{
  FeatureNameListType featureList;
  featureList.push_back("Volumetric Features Compactness 1");
  featureList.push_back("Volumetric Features Compactness 2");
  featureList.push_back("Volumetric Features Compactness 3");
  featureList.push_back("Volumetric Features Sphericity");
  featureList.push_back("Volumetric Features Asphericity");
  featureList.push_back("Volumetric Features Surface to volume ratio");
  featureList.push_back("Volumetric Features Surface area");
  featureList.push_back("Volumetric Features Volume (mesh based)");
  featureList.push_back("Volumetric Features Volume (pixel based)");
  featureList.push_back("Volumetric Features Spherical disproportion");
  featureList.push_back("Volumetric Features Maximum 3D diameter");
  return featureList;
}
