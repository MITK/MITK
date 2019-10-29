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
#include <vtkDoubleArray.h>
#include <vtkPCAStatistics.h>
#include <vtkTable.h>

// STL
#include <vnl/vnl_math.h>

template<typename TPixel, unsigned int VImageDimension>
void
  CalculateVolumeStatistic(itk::Image<TPixel, VImageDimension>* itkImage, mitk::Image::Pointer mask, mitk::GIFVolumetricStatistics::FeatureListType & featureList, std::string prefix)
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

  featureList.push_back(std::make_pair(prefix + "Voxel Volume", voxelVolume));
  featureList.push_back(std::make_pair(prefix + "Volume (voxel based)", volume));
}

template<typename TPixel, unsigned int VImageDimension>
void
  CalculateLargestDiameter(itk::Image<TPixel, VImageDimension>* mask, mitk::Image::Pointer valueImage, mitk::GIFVolumetricStatistics::FeatureListType & featureList, std::string prefix)
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

  unsigned int maskDimensionX = mask->GetLargestPossibleRegion().GetSize()[0];
  unsigned int maskDimensionY = mask->GetLargestPossibleRegion().GetSize()[1];
  unsigned int maskDimensionZ = mask->GetLargestPossibleRegion().GetSize()[2];

  double maskVoxelSpacingX = mask->GetSpacing()[0];
  double maskVoxelSpacingY = mask->GetSpacing()[1];
  double maskVoxelSpacingZ = mask->GetSpacing()[2];

  int maskMinimumX = maskDimensionX;
  int maskMaximumX = 0;
  int maskMinimumY = maskDimensionY;
  int maskMaximumY = 0;
  int maskMinimumZ = maskDimensionZ;
  int maskMaximumZ = 0;

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

    maskMinimumX = (maskMinimumX > iterator.GetIndex()[0]) ? iterator.GetIndex()[0] : maskMinimumX;
    maskMaximumX = (maskMaximumX < iterator.GetIndex()[0]) ? iterator.GetIndex()[0] : maskMaximumX;
    maskMinimumY = (maskMinimumY > iterator.GetIndex()[1]) ? iterator.GetIndex()[1] : maskMinimumY;
    maskMaximumY = (maskMaximumY < iterator.GetIndex()[1]) ? iterator.GetIndex()[1] : maskMaximumY;
    maskMinimumZ = (maskMinimumZ > iterator.GetIndex()[2]) ? iterator.GetIndex()[2] : maskMinimumZ;
    maskMaximumZ = (maskMaximumZ < iterator.GetIndex()[2]) ? iterator.GetIndex()[2] : maskMaximumZ;

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

  double boundingBoxVolume = maskVoxelSpacingX* (maskMaximumX - maskMinimumX) * maskVoxelSpacingY* (maskMaximumY - maskMinimumY) * maskVoxelSpacingZ* (maskMaximumZ - maskMinimumZ);

  featureList.push_back(std::make_pair(prefix + "Maximum 3D diameter", longestDiameter));
  featureList.push_back(std::make_pair(prefix + "Surface (voxel based)", surface));
  featureList.push_back(std::make_pair(prefix + "Centre of mass shift", differenceOfCenters));
  featureList.push_back(std::make_pair(prefix + "Centre of mass shift (uncorrected)", differenceOfCentersUncorrected));
  featureList.push_back(std::make_pair(prefix + "Bounding Box Volume", boundingBoxVolume));
}

mitk::GIFVolumetricStatistics::GIFVolumetricStatistics()
{
  SetLongName("volume");
  SetShortName("vol");
  SetFeatureClassName("Volumetric Features");
}

mitk::GIFVolumetricStatistics::FeatureListType mitk::GIFVolumetricStatistics::CalculateFeatures(const Image::Pointer & image, const Image::Pointer &mask)
{
  FeatureListType featureList;
  if (image->GetDimension() < 3)
  {
    return featureList;
  }


  AccessByItk_3(image, CalculateVolumeStatistic, mask, featureList, FeatureDescriptionPrefix());
  AccessByItk_3(mask, CalculateLargestDiameter, image, featureList, FeatureDescriptionPrefix());

  vtkSmartPointer<vtkImageMarchingCubes> mesher = vtkSmartPointer<vtkImageMarchingCubes>::New();
  vtkSmartPointer<vtkMassProperties> stats = vtkSmartPointer<vtkMassProperties>::New();
  mesher->SetInputData(mask->GetVtkImageData());
  mesher->SetValue(0, 0.5);
  stats->SetInputConnection(mesher->GetOutputPort());
  stats->Update();

  double pi = vnl_math::pi;

  double meshVolume = stats->GetVolume();
  double meshSurf = stats->GetSurfaceArea();
  double pixelVolume = featureList[1].second;
  double pixelSurface = featureList[3].second;

  MITK_INFO << "Surface: " << pixelSurface << " Volume: " << pixelVolume;

  double compactness1 = pixelVolume / (std::sqrt(pi) * std::pow(meshSurf, 2.0 / 3.0));
  double compactness1Pixel = pixelVolume / (std::sqrt(pi) * std::pow(pixelSurface, 2.0 / 3.0));
  //This is the definition used by Aertz. However, due to 2/3 this feature is not demensionless. Use compactness3 instead.

  double compactness2 = 36 * pi*pixelVolume*pixelVolume / meshSurf / meshSurf / meshSurf;
  double compactness2MeshMesh = 36 * pi*meshVolume*meshVolume / meshSurf / meshSurf / meshSurf;
  double compactness2Pixel = 36 * pi*pixelVolume*pixelVolume / pixelSurface / pixelSurface / pixelSurface;
  double compactness3 = pixelVolume / (std::sqrt(pi) * std::pow(meshSurf, 3.0 / 2.0));
  double compactness3MeshMesh = meshVolume / (std::sqrt(pi) * std::pow(meshSurf, 3.0 / 2.0));
  double compactness3Pixel = pixelVolume / (std::sqrt(pi) * std::pow(pixelSurface, 3.0 / 2.0));

  double sphericity = std::pow(pi, 1 / 3.0) *std::pow(6 * pixelVolume, 2.0 / 3.0) / meshSurf;
  double sphericityMesh = std::pow(pi, 1 / 3.0) *std::pow(6 * meshVolume, 2.0 / 3.0) / meshSurf;
  double sphericityPixel = std::pow(pi, 1 / 3.0) *std::pow(6 * pixelVolume, 2.0 / 3.0) / pixelSurface;
  double surfaceToVolume = meshSurf / meshVolume;
  double surfaceToVolumePixel = pixelSurface / pixelVolume;
  double sphericalDisproportion = meshSurf / 4 / pi / std::pow(3.0 / 4.0 / pi * pixelVolume, 2.0 / 3.0);
  double sphericalDisproportionMesh = meshSurf / 4 / pi / std::pow(3.0 / 4.0 / pi * meshVolume, 2.0 / 3.0);
  double sphericalDisproportionPixel = pixelSurface / 4 / pi / std::pow(3.0 / 4.0 / pi * pixelVolume, 2.0 / 3.0);
  double asphericity = std::pow(1.0/compactness2, (1.0 / 3.0)) - 1;
  double asphericityMesh = std::pow(1.0 / compactness2MeshMesh, (1.0 / 3.0)) - 1;
  double asphericityPixel = std::pow(1.0/compactness2Pixel, (1.0 / 3.0)) - 1;

  //Calculate center of mass shift
  int xx = mask->GetDimensions()[0];
  int yy = mask->GetDimensions()[1];
  int zz = mask->GetDimensions()[2];

  double xd = mask->GetGeometry()->GetSpacing()[0];
  double yd = mask->GetGeometry()->GetSpacing()[1];
  double zd = mask->GetGeometry()->GetSpacing()[2];

  vtkSmartPointer<vtkDoubleArray> dataset1Arr = vtkSmartPointer<vtkDoubleArray>::New();
  vtkSmartPointer<vtkDoubleArray> dataset2Arr = vtkSmartPointer<vtkDoubleArray>::New();
  vtkSmartPointer<vtkDoubleArray> dataset3Arr = vtkSmartPointer<vtkDoubleArray>::New();
  dataset1Arr->SetNumberOfComponents(1);
  dataset2Arr->SetNumberOfComponents(1);
  dataset3Arr->SetNumberOfComponents(1);
  dataset1Arr->SetName("M1");
  dataset2Arr->SetName("M2");
  dataset3Arr->SetName("M3");

  vtkSmartPointer<vtkDoubleArray> dataset1ArrU = vtkSmartPointer<vtkDoubleArray>::New();
  vtkSmartPointer<vtkDoubleArray> dataset2ArrU = vtkSmartPointer<vtkDoubleArray>::New();
  vtkSmartPointer<vtkDoubleArray> dataset3ArrU = vtkSmartPointer<vtkDoubleArray>::New();
  dataset1ArrU->SetNumberOfComponents(1);
  dataset2ArrU->SetNumberOfComponents(1);
  dataset3ArrU->SetNumberOfComponents(1);
  dataset1ArrU->SetName("M1");
  dataset2ArrU->SetName("M2");
  dataset3ArrU->SetName("M3");

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
          dataset1ArrU->InsertNextValue(x*xd);
          dataset2ArrU->InsertNextValue(y*yd);
          dataset3ArrU->InsertNextValue(z*zd);

          if (pxImage == pxImage)
          {
            dataset1Arr->InsertNextValue(x*xd);
            dataset2Arr->InsertNextValue(y*yd);
            dataset3Arr->InsertNextValue(z*zd);
          }
        }
      }
    }
  }

  vtkSmartPointer<vtkTable> datasetTable = vtkSmartPointer<vtkTable>::New();
  datasetTable->AddColumn(dataset1Arr);
  datasetTable->AddColumn(dataset2Arr);
  datasetTable->AddColumn(dataset3Arr);

  vtkSmartPointer<vtkTable> datasetTableU = vtkSmartPointer<vtkTable>::New();
  datasetTableU->AddColumn(dataset1ArrU);
  datasetTableU->AddColumn(dataset2ArrU);
  datasetTableU->AddColumn(dataset3ArrU);

  vtkSmartPointer<vtkPCAStatistics> pcaStatistics = vtkSmartPointer<vtkPCAStatistics>::New();
  pcaStatistics->SetInputData(vtkStatisticsAlgorithm::INPUT_DATA, datasetTable);
  pcaStatistics->SetColumnStatus("M1", 1);
  pcaStatistics->SetColumnStatus("M2", 1);
  pcaStatistics->SetColumnStatus("M3", 1);
  pcaStatistics->RequestSelectedColumns();
  pcaStatistics->SetDeriveOption(true);
  pcaStatistics->Update();

  vtkSmartPointer<vtkDoubleArray> eigenvalues = vtkSmartPointer<vtkDoubleArray>::New();
  pcaStatistics->GetEigenvalues(eigenvalues);

  pcaStatistics->SetInputData(vtkStatisticsAlgorithm::INPUT_DATA, datasetTableU);
  pcaStatistics->Update();
  vtkSmartPointer<vtkDoubleArray> eigenvaluesU = vtkSmartPointer<vtkDoubleArray>::New();
  pcaStatistics->GetEigenvalues(eigenvaluesU);

  std::vector<double> eigen_val(3);
  std::vector<double> eigen_valUC(3);
  eigen_val[2] = eigenvalues->GetValue(0);
  eigen_val[1] = eigenvalues->GetValue(1);
  eigen_val[0] = eigenvalues->GetValue(2);
  eigen_valUC[2] = eigenvaluesU->GetValue(0);
  eigen_valUC[1] = eigenvaluesU->GetValue(1);
  eigen_valUC[0] = eigenvaluesU->GetValue(2);

  double major = 4*sqrt(eigen_val[2]);
  double minor = 4*sqrt(eigen_val[1]);
  double least = 4*sqrt(eigen_val[0]);
  double elongation = (major == 0) ? 0 : sqrt(eigen_val[1] / eigen_val[2]);
  double flatness = (major == 0) ? 0 : sqrt(eigen_val[0] / eigen_val[2]);
  double majorUC = 4*sqrt(eigen_valUC[2]);
  double minorUC = 4*sqrt(eigen_valUC[1]);
  double leastUC = 4*sqrt(eigen_valUC[0]);
  double elongationUC = majorUC == 0 ? 0 : sqrt(eigen_valUC[1] / eigen_valUC[2]);
  double flatnessUC = majorUC == 0 ? 0 : sqrt(eigen_valUC[0] / eigen_valUC[2]);

  std::string prefix = FeatureDescriptionPrefix();
  featureList.push_back(std::make_pair(prefix + "Volume (mesh based)",meshVolume));
  featureList.push_back(std::make_pair(prefix + "Surface (mesh based)",meshSurf));
  featureList.push_back(std::make_pair(prefix + "Surface to volume ratio (mesh based)",surfaceToVolume));
  featureList.push_back(std::make_pair(prefix + "Sphericity (mesh based)",sphericity));
  featureList.push_back(std::make_pair(prefix + "Sphericity (mesh, mesh based)", sphericityMesh));
  featureList.push_back(std::make_pair(prefix + "Asphericity (mesh based)", asphericity));
  featureList.push_back(std::make_pair(prefix + "Asphericity (mesh, mesh based)", asphericityMesh));
  featureList.push_back(std::make_pair(prefix + "Compactness 1 (mesh based)", compactness3));
  featureList.push_back(std::make_pair(prefix + "Compactness 1 old (mesh based)" ,compactness1));
  featureList.push_back(std::make_pair(prefix + "Compactness 2 (mesh based)",compactness2));
  featureList.push_back(std::make_pair(prefix + "Compactness 1 (mesh, mesh based)", compactness3MeshMesh));
  featureList.push_back(std::make_pair(prefix + "Compactness 2 (mesh, mesh based)", compactness2MeshMesh));
  featureList.push_back(std::make_pair(prefix + "Spherical disproportion (mesh based)", sphericalDisproportion));
  featureList.push_back(std::make_pair(prefix + "Spherical disproportion (mesh, mesh based)", sphericalDisproportionMesh));
  featureList.push_back(std::make_pair(prefix + "Surface to volume ratio (voxel based)", surfaceToVolumePixel));
  featureList.push_back(std::make_pair(prefix + "Sphericity (voxel based)", sphericityPixel));
  featureList.push_back(std::make_pair(prefix + "Asphericity (voxel based)", asphericityPixel));
  featureList.push_back(std::make_pair(prefix + "Compactness 1 (voxel based)", compactness3Pixel));
  featureList.push_back(std::make_pair(prefix + "Compactness 1 old (voxel based)", compactness1Pixel));
  featureList.push_back(std::make_pair(prefix + "Compactness 2 (voxel based)", compactness2Pixel));
  featureList.push_back(std::make_pair(prefix + "Spherical disproportion (voxel based)", sphericalDisproportionPixel));
  featureList.push_back(std::make_pair(prefix + "PCA Major axis length",major));
  featureList.push_back(std::make_pair(prefix + "PCA Minor axis length",minor));
  featureList.push_back(std::make_pair(prefix + "PCA Least axis length",least));
  featureList.push_back(std::make_pair(prefix + "PCA Elongation",elongation));
  featureList.push_back(std::make_pair(prefix + "PCA Flatness",flatness));
  featureList.push_back(std::make_pair(prefix + "PCA Major axis length (uncorrected)", majorUC));
  featureList.push_back(std::make_pair(prefix + "PCA Minor axis length (uncorrected)", minorUC));
  featureList.push_back(std::make_pair(prefix + "PCA Least axis length (uncorrected)", leastUC));
  featureList.push_back(std::make_pair(prefix + "PCA Elongation (uncorrected)", elongationUC));
  featureList.push_back(std::make_pair(prefix + "PCA Flatness (uncorrected)", flatnessUC));

  return featureList;
}

mitk::GIFVolumetricStatistics::FeatureNameListType mitk::GIFVolumetricStatistics::GetFeatureNames()
{
  FeatureNameListType featureList;
  return featureList;
}


void mitk::GIFVolumetricStatistics::AddArguments(mitkCommandLineParser &parser)
{
  std::string name = GetOptionPrefix();

  parser.addArgument(GetLongName(), name, mitkCommandLineParser::Bool, "Use Volume-Statistic", "calculates volume based features", us::Any());
}

void
mitk::GIFVolumetricStatistics::CalculateFeaturesUsingParameters(const Image::Pointer & feature, const Image::Pointer &mask, const Image::Pointer &, FeatureListType &featureList)
{
  auto parsedArgs = GetParameter();
  if (parsedArgs.count(GetLongName()))
  {
    MITK_INFO << "Start calculating Volumetric Features::....";
    auto localResults = this->CalculateFeatures(feature, mask);
    featureList.insert(featureList.end(), localResults.begin(), localResults.end());
    MITK_INFO << "Finished calculating volumetric features....";
  }
}

