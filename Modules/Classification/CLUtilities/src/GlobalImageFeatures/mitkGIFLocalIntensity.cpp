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

#include <mitkGIFLocalIntensity.h>

// MITK
#include <mitkITKImageImport.h>
#include <mitkImageCast.h>
#include <mitkImageAccessByItk.h>
#include <mitkPixelTypeMultiplex.h>
#include <mitkImagePixelReadAccessor.h>

// ITK
#include <itkImageRegionIteratorWithIndex.h>
#include <itkNeighborhoodIterator.h>
// STL
#include <limits>

struct GIFLocalIntensityParameter
{
  double range;
  std::string prefix;
};


template<typename TPixel, unsigned int VImageDimension>
static void
CalculateIntensityPeak(itk::Image<TPixel, VImageDimension>* itkImage, mitk::Image::Pointer mask, mitk::GIFLocalIntensity::FeatureListType & featureList, GIFLocalIntensityParameter params)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<unsigned short, VImageDimension> MaskType;

  typename MaskType::Pointer itkMask = MaskType::New();
  mitk::CastToItkImage(mask, itkMask);

  double range = params.range;
  double minimumSpacing = std::numeric_limits<double>::max();
  itkImage->GetSpacing();
  for (unsigned int i = 0; i < VImageDimension; ++i)
  {
    minimumSpacing = (minimumSpacing < itkImage->GetSpacing()[i]) ? minimumSpacing : itkImage->GetSpacing()[i];
  }
  typename ImageType::SizeType regionSize;
  int offset = std::ceil(range / minimumSpacing);
  regionSize.Fill(offset);

  itk::NeighborhoodIterator<ImageType> iter(regionSize, itkImage, itkImage->GetLargestPossibleRegion());
  itk::NeighborhoodIterator<MaskType> iterMask(regionSize, itkMask, itkMask->GetLargestPossibleRegion());

  typename ImageType::PointType origin;
  typename ImageType::PointType localPoint;
  itk::Index<VImageDimension> index;

  double tmpPeakValue;
  double globalPeakValue = 0;
  double localPeakValue = 0;
  TPixel localMaximum = 0;

  int count = 0;
  while (!iter.IsAtEnd())
  {
    if (iterMask.GetCenterPixel() > 0)
    {
      tmpPeakValue = 0;
      count = 0;
      index = iter.GetIndex();
      itkImage->TransformIndexToPhysicalPoint(index, origin);
      for (itk::SizeValueType i = 0; i < iter.Size(); ++i)
      {
        itkImage->TransformIndexToPhysicalPoint(iter.GetIndex(i), localPoint);
        double dist = origin.EuclideanDistanceTo(localPoint);
        if (dist < 6.2)
        {
          if (iter.IndexInBounds(i))
          {
            tmpPeakValue += iter.GetPixel(i);
            ++count;
          }
        }
      }
      tmpPeakValue /= count;
      globalPeakValue = std::max<double>(tmpPeakValue, globalPeakValue);
      if (localMaximum == iter.GetCenterPixel())
      {
        localPeakValue = std::max<double>(tmpPeakValue,localPeakValue);
      }
      else if (localMaximum < iter.GetCenterPixel())
      {
        localMaximum = iter.GetCenterPixel();
        localPeakValue = tmpPeakValue;
      }
    }
    ++iterMask;
    ++iter;
  }
  featureList.push_back(std::make_pair(params.prefix + "Local Intensity Peak", localPeakValue));
  featureList.push_back(std::make_pair(params.prefix + "Global Intensity Peak", globalPeakValue));
}


mitk::GIFLocalIntensity::GIFLocalIntensity() :
m_Range(6.2)
{
  SetLongName("local-intensity");
  SetShortName("loci");
  SetFeatureClassName("Local Intensity");
}

mitk::GIFLocalIntensity::FeatureListType mitk::GIFLocalIntensity::CalculateFeatures(const Image::Pointer & image, const Image::Pointer &mask)
{
  FeatureListType featureList;
  if (image->GetDimension() < 3)
  {
    return featureList;
  }
  GIFLocalIntensityParameter params;
  params.range = GetRange();
  params.prefix = FeatureDescriptionPrefix();
  AccessByItk_3(image, CalculateIntensityPeak, mask, featureList, params);
  return featureList;
}

mitk::GIFLocalIntensity::FeatureNameListType mitk::GIFLocalIntensity::GetFeatureNames()
{
  FeatureNameListType featureList;
  return featureList;
}


void mitk::GIFLocalIntensity::AddArguments(mitkCommandLineParser &parser)
{
  std::string name = GetOptionPrefix();

  parser.addArgument(GetLongName(), name, mitkCommandLineParser::Bool, "Use Local Intensity", "calculates local intensity based features", us::Any());
  parser.addArgument(name + "::range", name+"::range", mitkCommandLineParser::Float, "Range for the local intensity", "Give the range that should be used for the local intensity in mm", us::Any());
}

void
mitk::GIFLocalIntensity::CalculateFeaturesUsingParameters(const Image::Pointer & feature, const Image::Pointer &mask, const Image::Pointer &, FeatureListType &featureList)
{
  std::string name = GetOptionPrefix();
  auto parsedArgs = GetParameter();
  if (parsedArgs.count(GetLongName()))
  {
    if (parsedArgs.count(name + "::range"))
    {
      double range = us::any_cast<float>(parsedArgs[name + "::range"]);
      this->SetRange(range);
    }
    MITK_INFO << "Start calculating local intensity features ....";
    auto localResults = this->CalculateFeatures(feature, mask);
    featureList.insert(featureList.end(), localResults.begin(), localResults.end());
    MITK_INFO << "Finished calculating local intensity features....";
  }
}

std::string mitk::GIFLocalIntensity::GetCurrentFeatureEncoding()
{
  std::ostringstream  ss;
  ss << m_Range;
  std::string strRange = ss.str();
  return "Range-" + ss.str();
}



