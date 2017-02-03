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
  CalculateLargestDiameter(itk::Image<TPixel, VImageDimension>* mask, mitk::Image::Pointer valueImage, mitk::GIFVolumetricStatistics::FeatureListType & featureList)
{
  typedef itk::Image<double, VImageDimension> ValueImageType;

  typename ValueImageType::Pointer itkValueImage = ValueImageType::New();
  mitk::CastToItkImage(valueImage, itkValueImage);


  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef typename ImageType::PointType PointType;
  typename ImageType::SizeType radius;
  for (int i=0; i < (int)VImageDimension; ++i)
    radius[i] = 1;
  itk::NeighborhoodIterator<ImageType> iterator(radius, mask, mask->GetRequestedRegion());
  itk::NeighborhoodIterator<ValueImageType> valueIter(radius, itkValueImage, itkValueImage->GetRequestedRegion());
  std::vector<PointType> borderPoints;

  //
  //  Calculate surface in different directions
  //
  double surface = 0;
  std::vector<double> directionSurface;
  for (int i = 0; i < (int)(iterator.Size()); ++i)
  {
    auto offset = iterator.GetOffset(i);
    double deltaS = 1;
    int nonZeros = 0;
    for (unsigned int j = 0; j < VImageDimension; ++j)
    {
      if (offset[j] != 0 && nonZeros == 0)
      {
        for (unsigned int k = 0; k < VImageDimension; ++k)
        {
          if (k != j)
            deltaS *= mask->GetSpacing()[k];
        }
        nonZeros++;
      }
      else if (offset[j] != 0)
      {
        deltaS = 0;
      }
    }
    if (nonZeros < 1)
      deltaS = 0;
    directionSurface.push_back(deltaS);
  }

  //
  // Prepare calulation of Centre of mass shift
  //
  PointType normalCenter(0);
  PointType normalCenterUncorrected(0);
  PointType weightedCenter(0);
  PointType currentPoint;
  int numberOfPoints = 0;
  int numberOfPointsUncorrected = 0;
  double sumOfPoints = 0;

  while(!iterator.IsAtEnd())
  {
    if (iterator.GetCenterPixel() == 0)
    {
      ++iterator;
      ++valueIter;
      continue;
    }

    mask->TransformIndexToPhysicalPoint(iterator.GetIndex(), currentPoint);

    normalCenterUncorrected += currentPoint.GetVectorFromOrigin();
    ++numberOfPointsUncorrected;

    double intensityValue = valueIter.GetCenterPixel();
    if (intensityValue == intensityValue)
    {
      normalCenter += currentPoint.GetVectorFromOrigin();
      weightedCenter += currentPoint.GetVectorFromOrigin() * intensityValue;
      sumOfPoints += intensityValue;
      ++numberOfPoints;
    }

    bool border = false;
    for (int i = 0; i < (int)(iterator.Size()); ++i)
    {
      if (iterator.GetPixel(i) == 0 || ( ! iterator.IndexInBounds(i)))
      {
        border = true;
        surface += directionSurface[i];
        //break;
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
    ++valueIter;
  }

  auto normalCenterVector = normalCenter.GetVectorFromOrigin() / numberOfPoints;
  auto normalCenterVectorUncorrected = normalCenter.GetVectorFromOrigin() / numberOfPointsUncorrected;
  auto weightedCenterVector = weightedCenter.GetVectorFromOrigin() / sumOfPoints;
  auto differenceOfCentersUncorrected = (normalCenterVectorUncorrected - weightedCenterVector).GetNorm();
  auto differenceOfCenters = (normalCenterVector - weightedCenterVector).GetNorm();

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

  featureList.push_back(std::make_pair("Volumetric Features Maximum 3D diameter", longestDiameter));
  featureList.push_back(std::make_pair("Volumetric Features Surface (Voxel based)", surface));
  featureList.push_back(std::make_pair("Volumetric Features Centre of mass shift", differenceOfCenters));
  featureList.push_back(std::make_pair("Volumetric Features Centre of mass shift (Uncorrected)", differenceOfCentersUncorrected));
}

mitk::GIFVolumetricStatistics::GIFVolumetricStatistics()
{
  SetLongName("volume");
  SetShortName("vol");
}

mitk::GIFVolumetricStatistics::FeatureListType mitk::GIFVolumetricStatistics::CalculateFeatures(const Image::Pointer & image, const Image::Pointer &mask)
{
  FeatureListType featureList;
  if (image->GetDimension() < 3)
  {
    return featureList;
  }


  AccessByItk_2(image, CalculateVolumeStatistic, mask, featureList);
  AccessByItk_2(mask, CalculateLargestDiameter, image, featureList);

  vtkSmartPointer<vtkImageMarchingCubes> mesher = vtkSmartPointer<vtkImageMarchingCubes>::New();
  vtkSmartPointer<vtkMassProperties> stats = vtkSmartPointer<vtkMassProperties>::New();
  mesher->SetInputData(mask->GetVtkImageData());
  mesher->SetValue(0, 0.5);
  stats->SetInputConnection(mesher->GetOutputPort());
  stats->Update();

  double pi = vnl_math::pi;

  double meshVolume = stats->GetVolume();
  double meshSurf = stats->GetSurfaceArea();
  double pixelVolume = featureList[0].second;
  double pixelSurface = featureList[3].second;

  MITK_INFO << "Surf: " << pixelSurface << " Vol " << pixelVolume;

  double compactness1 = pixelVolume / (std::sqrt(pi) * std::pow(meshSurf, 2.0 / 3.0));
  double compactness1Pixel = pixelVolume / (std::sqrt(pi) * std::pow(pixelSurface, 2.0 / 3.0));
  //This is the definition used by Aertz. However, due to 2/3 this feature is not demensionless. Use compactness3 instead.

  double compactness2 = 36 * pi*pixelVolume*pixelVolume / meshSurf / meshSurf / meshSurf;
  double compactness2Pixel = 36 * pi*pixelVolume*pixelVolume / pixelSurface / pixelSurface / pixelSurface;
  double compactness3 = pixelVolume / (std::sqrt(pi) * std::pow(meshSurf, 3.0 / 2.0));
  double compactness3Pixel = pixelVolume / (std::sqrt(pi) * std::pow(pixelSurface, 3.0 / 2.0));

  double sphericity = std::pow(pi, 1 / 3.0) *std::pow(6 * pixelVolume, 2.0 / 3.0) / meshSurf;
  double sphericityPixel = std::pow(pi, 1 / 3.0) *std::pow(6 * pixelVolume, 2.0 / 3.0) / pixelSurface;
  double surfaceToVolume = meshSurf / pixelVolume;
  double surfaceToVolumePixel = pixelSurface / pixelVolume;
  double sphericalDisproportion = meshSurf / 4 / pi / std::pow(3.0 / 4.0 / pi * pixelVolume, 2.0 / 3.0);
  double sphericalDisproportionPixel = pixelSurface / 4 / pi / std::pow(3.0 / 4.0 / pi * pixelVolume, 2.0 / 3.0);
  double asphericity = std::pow(1.0/compactness2, (1.0 / 3.0)) - 1;
  double asphericityPixel = std::pow(1.0/compactness2Pixel, (1.0 / 3.0)) - 1;

  //Calculate center of mass shift
  int xx = mask->GetDimensions()[0];
  int yy = mask->GetDimensions()[1];
  int zz = mask->GetDimensions()[2];

  double xd = mask->GetGeometry()->GetSpacing()[0];
  double yd = mask->GetGeometry()->GetSpacing()[1];
  double zd = mask->GetGeometry()->GetSpacing()[2];


  std::vector<cv::Point3d> pointsForPCA;
  std::vector<cv::Point3d> pointsForPCAUncorrected;

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
        if (pxMask > 0)
        {
          cv::Point3d tmp;
          tmp.x = x * xd;
          tmp.y = y * yd;
          tmp.z = z * zd;
          pointsForPCAUncorrected.push_back(tmp);

          if (pxImage == pxImage)
          {
            pointsForPCA.push_back(tmp);
          }
        }
      }
    }
  }


  //Calculate PCA Features
  int sz = pointsForPCA.size();
  cv::Mat data_pts = cv::Mat(sz, 3, CV_64FC1);
  for (int i = 0; i < data_pts.rows; ++i)
  {
      data_pts.at<double>(i, 0) = pointsForPCA[i].x;
      data_pts.at<double>(i, 1) = pointsForPCA[i].y;
      data_pts.at<double>(i, 2) = pointsForPCA[i].z;
  }

  //Calculate PCA Features
  int szUC = pointsForPCAUncorrected.size();
  cv::Mat data_ptsUC = cv::Mat(szUC, 3, CV_64FC1);
  for (int i = 0; i < data_ptsUC.rows; ++i)
  {
    data_ptsUC.at<double>(i, 0) = pointsForPCAUncorrected[i].x;
    data_ptsUC.at<double>(i, 1) = pointsForPCAUncorrected[i].y;
    data_ptsUC.at<double>(i, 2) = pointsForPCAUncorrected[i].z;
  }

  //Perform PCA analysis
  cv::PCA pca_analysis(data_pts, cv::Mat(), CV_PCA_DATA_AS_ROW);
  cv::PCA pca_analysisUC(data_ptsUC, cv::Mat(), CV_PCA_DATA_AS_ROW);

  //Store the eigenvalues
  std::vector<double> eigen_val(3);
  std::vector<double> eigen_valUC(3);
  for (int i = 0; i < 3; ++i)
  {
    eigen_val[i] = pca_analysis.eigenvalues.at<double>(0, i);
    eigen_valUC[i] = pca_analysisUC.eigenvalues.at<double>(0, i);
  }

  std::sort(eigen_val.begin(), eigen_val.end());
  std::sort(eigen_valUC.begin(), eigen_valUC.end());
  double major = eigen_val[2];
  double minor = eigen_val[1];
  double least = eigen_val[0];
  double elongation = major == 0 ? 0 : minor/major;
  double flatness = major == 0 ? 0 : least / major;
  double majorUC = eigen_valUC[2];
  double minorUC = eigen_valUC[1];
  double leastUC = eigen_valUC[0];
  double elongationUC = majorUC == 0 ? 0 : minorUC / majorUC;
  double flatnessUC = majorUC == 0 ? 0 : leastUC / majorUC;


  featureList.push_back(std::make_pair("Volumetric Features Volume (mesh based)",meshVolume));
  featureList.push_back(std::make_pair("Volumetric Features Surface area",meshSurf));
  featureList.push_back(std::make_pair("Volumetric Features Surface to volume ratio",surfaceToVolume));
  featureList.push_back(std::make_pair("Volumetric Features Sphericity",sphericity));
  featureList.push_back(std::make_pair("Volumetric Features Asphericity",asphericity));
  featureList.push_back(std::make_pair("Volumetric Features Compactness 1",compactness1));
  featureList.push_back(std::make_pair("Volumetric Features Compactness 2",compactness2));
  featureList.push_back(std::make_pair("Volumetric Features Compactness 3",compactness3));
  featureList.push_back(std::make_pair("Volumetric Features Spherical disproportion", sphericalDisproportion));
  featureList.push_back(std::make_pair("Volumetric Features Surface to volume ratio (Voxel based)", surfaceToVolumePixel));
  featureList.push_back(std::make_pair("Volumetric Features Sphericity (Voxel based)", sphericityPixel));
  featureList.push_back(std::make_pair("Volumetric Features Asphericity (Voxel based)", asphericityPixel));
  featureList.push_back(std::make_pair("Volumetric Features Compactness 1 (Voxel based)", compactness1Pixel));
  featureList.push_back(std::make_pair("Volumetric Features Compactness 2 (Voxel based)", compactness2Pixel));
  featureList.push_back(std::make_pair("Volumetric Features Compactness 3 (Voxel based)", compactness3Pixel));
  featureList.push_back(std::make_pair("Volumetric Features Spherical disproportion (Voxel based)", sphericalDisproportionPixel));
  featureList.push_back(std::make_pair("Volumetric Features PCA Major Axis",major));
  featureList.push_back(std::make_pair("Volumetric Features PCA Minor Axis",minor));
  featureList.push_back(std::make_pair("Volumetric Features PCA Least Axis",least));
  featureList.push_back(std::make_pair("Volumetric Features PCA Elongation",elongation));
  featureList.push_back(std::make_pair("Volumetric Features PCA Flatness",flatness));
  featureList.push_back(std::make_pair("Volumetric Features PCA Major Axis (Uncorrected)", majorUC));
  featureList.push_back(std::make_pair("Volumetric Features PCA Minor Axis (Uncorrected)", minorUC));
  featureList.push_back(std::make_pair("Volumetric Features PCA Least Axis (Uncorrected)", leastUC));
  featureList.push_back(std::make_pair("Volumetric Features PCA Elongation (Uncorrected)", elongationUC));
  featureList.push_back(std::make_pair("Volumetric Features PCA Flatness (Uncorrected)", flatnessUC));


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


void mitk::GIFVolumetricStatistics::AddArguments(mitkCommandLineParser &parser)
{
  std::string name = GetOptionPrefix();

  parser.addArgument(GetLongName(), name, mitkCommandLineParser::String, "Use Volume-Statistic", "calculates volume based features", us::Any());
}

void
mitk::GIFVolumetricStatistics::CalculateFeaturesUsingParameters(const Image::Pointer & feature, const Image::Pointer &mask, const Image::Pointer &, FeatureListType &featureList)
{
  auto parsedArgs = GetParameter();
  if (parsedArgs.count(GetLongName()))
  {
    MITK_INFO << "Start calculating volumetric features ....";
    auto localResults = this->CalculateFeatures(feature, mask);
    featureList.insert(featureList.end(), localResults.begin(), localResults.end());
    MITK_INFO << "Finished calculating volumetric features....";
  }
}

