/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkGIFImageDescriptionFeatures.h>

// MITK
#include <mitkITKImageImport.h>
#include <mitkImageCast.h>
#include <mitkImageAccessByItk.h>

// ITK
#include <itkLabelStatisticsImageFilter.h>
#include <itkMinimumMaximumImageCalculator.h>
#include <itkImageRegionConstIteratorWithIndex.h>

// STL
#include <sstream>

template<typename TPixel, unsigned int VImageDimension>
static void
CalculateFirstOrderStatistics(const itk::Image<TPixel, VImageDimension>* itkImage, const mitk::Image* mask, mitk::GIFImageDescriptionFeatures::FeatureListType & featureList, const mitk::FeatureID& featureID)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<unsigned short, VImageDimension> MaskType;
  typedef itk::MinimumMaximumImageCalculator<ImageType> MinMaxComputerType;

  typename MaskType::Pointer maskImage = MaskType::New();
  mitk::CastToItkImage(mask, maskImage);

  unsigned int imageDimensionX = itkImage->GetLargestPossibleRegion().GetSize()[0];
  unsigned int imageDimensionY = itkImage->GetLargestPossibleRegion().GetSize()[1];
  unsigned int imageDimensionZ = itkImage->GetLargestPossibleRegion().GetSize()[2];

  double imageVoxelSpacingX = itkImage->GetSpacing()[0];
  double imageVoxelSpacingY = itkImage->GetSpacing()[1];
  double imageVoxelSpacingZ = itkImage->GetSpacing()[2];

  typename MinMaxComputerType::Pointer minMaxComputer = MinMaxComputerType::New();
  minMaxComputer->SetImage(itkImage);
  minMaxComputer->Compute();

  double imageMinimum = minMaxComputer->GetMinimum();
  double imageMaximum = minMaxComputer->GetMaximum();


  unsigned int maskDimensionX = maskImage->GetLargestPossibleRegion().GetSize()[0];
  unsigned int maskDimensionY = maskImage->GetLargestPossibleRegion().GetSize()[1];
  unsigned int maskDimensionZ = maskImage->GetLargestPossibleRegion().GetSize()[2];

  double maskVoxelSpacingX = maskImage->GetSpacing()[0];
  double maskVoxelSpacingY = maskImage->GetSpacing()[1];
  double maskVoxelSpacingZ = maskImage->GetSpacing()[2];


  unsigned int voxelCount = 0;
  unsigned int maskVoxelCount = 0;
  double maskMinimum = imageMaximum;
  double maskMaximum = imageMinimum;
  double imageMean = 0;
  double maskMean = 0;

  int maskMinimumX = maskDimensionX;
  int maskMaximumX = 0;
  int maskMinimumY = maskDimensionY;
  int maskMaximumY = 0;
  int maskMinimumZ = maskDimensionZ;
  int maskMaximumZ = 0;


  itk::ImageRegionConstIteratorWithIndex<ImageType> imIter(itkImage, itkImage->GetLargestPossibleRegion());
  itk::ImageRegionConstIteratorWithIndex<MaskType> maIter(maskImage, maskImage->GetLargestPossibleRegion());

  while (!imIter.IsAtEnd())
  {
    auto pixelValue = imIter.Get();
    if (maIter.Get() > 0)
    {
      ++maskVoxelCount;
      maskMean += pixelValue;
      maskMinimum = (maskMinimum > pixelValue) ? pixelValue : maskMinimum;
      maskMaximum = (maskMaximum < pixelValue) ? pixelValue : maskMaximum;
      maskMinimumX = (maskMinimumX > imIter.GetIndex()[0]) ? imIter.GetIndex()[0] : maskMinimumX;
      maskMaximumX = (maskMaximumX < imIter.GetIndex()[0]) ? imIter.GetIndex()[0] : maskMaximumX;
      maskMinimumY = (maskMinimumY > imIter.GetIndex()[1]) ? imIter.GetIndex()[1] : maskMinimumY;
      maskMaximumY = (maskMaximumY < imIter.GetIndex()[1]) ? imIter.GetIndex()[1] : maskMaximumY;
      maskMinimumZ = (maskMinimumZ > imIter.GetIndex()[2]) ? imIter.GetIndex()[2] : maskMinimumZ;
      maskMaximumZ = (maskMaximumZ < imIter.GetIndex()[2]) ? imIter.GetIndex()[2] : maskMaximumZ;
    }
    ++voxelCount;
    imageMean += pixelValue;
    ++imIter;
    ++maIter;
  }
  imageMean /= voxelCount;
  maskMean /= maskVoxelCount;

  featureList.push_back(std::make_pair(mitk::CreateFeatureID(featureID, "Image Dimension X"), imageDimensionX));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(featureID, "Image Dimension Y"), imageDimensionY));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(featureID, "Image Dimension Z"), imageDimensionZ));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(featureID, "Image Spacing X"), imageVoxelSpacingX));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(featureID, "Image Spacing Y"), imageVoxelSpacingY));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(featureID, "Image Spacing Z"), imageVoxelSpacingZ));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(featureID, "Image Mean intensity"), imageMean));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(featureID, "Image Minimum intensity"), imageMinimum));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(featureID, "Image Maximum intensity"), imageMaximum));

  featureList.push_back(std::make_pair(mitk::CreateFeatureID(featureID, "Mask Dimension X"), maskDimensionX));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(featureID, "Mask Dimension Y"), maskDimensionY));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(featureID, "Mask Dimension Z"), maskDimensionZ));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(featureID, "Mask bounding box X"), maskMaximumX - maskMinimumX + 1));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(featureID, "Mask bounding box Y"), maskMaximumY - maskMinimumY + 1));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(featureID, "Mask bounding box Z"), maskMaximumZ - maskMinimumZ + 1));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(featureID, "Mask Spacing X"), maskVoxelSpacingX));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(featureID, "Mask Spacing Y"), maskVoxelSpacingY));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(featureID, "Mask Spacing Z"), maskVoxelSpacingZ));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(featureID, "Mask Voxel Count"), maskVoxelCount));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(featureID, "Mask Mean intensity"), maskMean));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(featureID, "Mask Minimum intensity"), maskMinimum));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(featureID, "Mask Maximum intensity"), maskMaximum));

}

mitk::GIFImageDescriptionFeatures::GIFImageDescriptionFeatures()
{
  SetShortName("id");
  SetLongName("image-diagnostic");

  SetFeatureClassName("Diagnostic");
}

void mitk::GIFImageDescriptionFeatures::AddArguments(mitkCommandLineParser &parser) const
{
  std::string name = GetOptionPrefix();
  parser.addArgument(GetLongName(), name, mitkCommandLineParser::Bool, "Use Image Description", "calculates image description features", us::Any());
}

mitk::AbstractGlobalImageFeature::FeatureListType mitk::GIFImageDescriptionFeatures::DoCalculateFeatures(const Image* image, const Image* mask)
{
  FeatureListType featureList;

  MITK_INFO << "Start calculating image description features....";
  auto id = this->CreateTemplateFeatureID();
  AccessByItk_3(image, CalculateFirstOrderStatistics, mask, featureList, id);
  MITK_INFO << "Finished calculating image description features....";

  return featureList;
}

mitk::AbstractGlobalImageFeature::FeatureListType mitk::GIFImageDescriptionFeatures::CalculateFeatures(const Image* image, const Image*, const Image* maskNoNAN)
{
  return Superclass::CalculateFeatures(image, maskNoNAN);
}
