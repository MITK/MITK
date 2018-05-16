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
CalculateFirstOrderStatistics(itk::Image<TPixel, VImageDimension>* itkImage, mitk::Image::Pointer mask, mitk::GIFImageDescriptionFeatures::FeatureListType & featureList, std::string prefix)
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

  featureList.push_back(std::make_pair(prefix + "Image Dimension X", imageDimensionX));
  featureList.push_back(std::make_pair(prefix + "Image Dimension Y", imageDimensionY));
  featureList.push_back(std::make_pair(prefix + "Image Dimension Z", imageDimensionZ));
  featureList.push_back(std::make_pair(prefix + "Image Spacing X", imageVoxelSpacingX));
  featureList.push_back(std::make_pair(prefix + "Image Spacing Y", imageVoxelSpacingY));
  featureList.push_back(std::make_pair(prefix + "Image Spacing Z", imageVoxelSpacingZ));
  featureList.push_back(std::make_pair(prefix + "Image Mean intensity", imageMean));
  featureList.push_back(std::make_pair(prefix + "Image Minimum intensity", imageMinimum));
  featureList.push_back(std::make_pair(prefix + "Image Maximum intensity", imageMaximum));

  featureList.push_back(std::make_pair(prefix + "Mask Dimension X", maskDimensionX));
  featureList.push_back(std::make_pair(prefix + "Mask Dimension Y", maskDimensionY));
  featureList.push_back(std::make_pair(prefix + "Mask Dimension Z", maskDimensionZ));
  featureList.push_back(std::make_pair(prefix + "Mask bounding box X", maskMaximumX - maskMinimumX + 1));
  featureList.push_back(std::make_pair(prefix + "Mask bounding box Y", maskMaximumY - maskMinimumY + 1));
  featureList.push_back(std::make_pair(prefix + "Mask bounding box Z", maskMaximumZ - maskMinimumZ + 1));
  featureList.push_back(std::make_pair(prefix + "Mask Spacing X", maskVoxelSpacingX));
  featureList.push_back(std::make_pair(prefix + "Mask Spacing Y", maskVoxelSpacingY));
  featureList.push_back(std::make_pair(prefix + "Mask Spacing Z", maskVoxelSpacingZ));
  featureList.push_back(std::make_pair(prefix + "Mask Voxel Count", maskVoxelCount));
  featureList.push_back(std::make_pair(prefix + "Mask Mean intensity", maskMean));
  featureList.push_back(std::make_pair(prefix + "Mask Minimum intensity", maskMinimum));
  featureList.push_back(std::make_pair(prefix + "Mask Maximum intensity", maskMaximum));

}

mitk::GIFImageDescriptionFeatures::GIFImageDescriptionFeatures()
{
  SetShortName("id");
  SetLongName("image-diagnostic");

  SetFeatureClassName("Diagnostic");
}

mitk::GIFImageDescriptionFeatures::FeatureListType mitk::GIFImageDescriptionFeatures::CalculateFeatures(const Image::Pointer & image, const Image::Pointer &mask)
{
  FeatureListType featureList;
  AccessByItk_3(image, CalculateFirstOrderStatistics, mask, featureList, FeatureDescriptionPrefix());

  return featureList;
}

mitk::GIFImageDescriptionFeatures::FeatureNameListType mitk::GIFImageDescriptionFeatures::GetFeatureNames()
{
  FeatureNameListType featureList;
  return featureList;
}


void mitk::GIFImageDescriptionFeatures::AddArguments(mitkCommandLineParser &parser)
{
  std::string name = GetOptionPrefix();
  parser.addArgument(GetLongName(), name, mitkCommandLineParser::Bool, "Use Image Description", "calculates image description features", us::Any());
}

void
mitk::GIFImageDescriptionFeatures::CalculateFeaturesUsingParameters(const Image::Pointer & feature, const Image::Pointer &, const Image::Pointer &maskNoNAN, FeatureListType &featureList)
{
  auto parsedArgs = GetParameter();
  if (parsedArgs.count(GetLongName()))
  {
    MITK_INFO << "Start calculating image description features....";
    auto localResults = this->CalculateFeatures(feature, maskNoNAN);
    featureList.insert(featureList.end(), localResults.begin(), localResults.end());
    MITK_INFO << "Finished calculating image description features....";
  }
}

