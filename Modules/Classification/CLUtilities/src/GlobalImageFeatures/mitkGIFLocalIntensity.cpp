/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
#include <itkLocalIntensityFilter.h>

// STL
#include <limits>

struct GIFLocalIntensityParameter
{
  double range;
  mitk::FeatureID id;
};


template<typename TPixel, unsigned int VImageDimension>
static void
CalculateIntensityPeak(const itk::Image<TPixel, VImageDimension>* itkImage, const mitk::Image* mask, mitk::GIFLocalIntensity::FeatureListType & featureList, GIFLocalIntensityParameter params)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<unsigned short, VImageDimension> MaskType;

  typename MaskType::Pointer itkMask = MaskType::New();
  mitk::CastToItkImage(mask, itkMask);

  double range = params.range;

  typename itk::LocalIntensityFilter<ImageType>::Pointer filter = itk::LocalIntensityFilter<ImageType>::New();
  filter->SetInput(itkImage);
  filter->SetMask(itkMask);
  filter->SetRange(range);
  filter->Update();

  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "2. Local Intensity Peak"), filter->GetLocalPeak()));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "2. Global Intensity Peak"), filter->GetGlobalPeak()));
}


mitk::GIFLocalIntensity::GIFLocalIntensity() :
m_Range(6.2)
{
  SetLongName("local-intensity");
  SetShortName("loci");
  SetFeatureClassName("Local Intensity");
}

std::string mitk::GIFLocalIntensity::GenerateLegacyFeatureEncoding(const FeatureID& id) const
{
  return "_Range-" + id.parameters.at(this->GetOptionPrefix() + "::range").ToString();
}

void mitk::GIFLocalIntensity::AddArguments(mitkCommandLineParser& parser) const
{
  std::string name = GetOptionPrefix();

  parser.addArgument(GetLongName(), name, mitkCommandLineParser::Bool, "Use Local Intensity", "calculates local intensity based features", us::Any());
  parser.addArgument(name + "::range", name + "::range", mitkCommandLineParser::Float, "Range for the local intensity", "Give the range that should be used for the local intensity in mm", us::Any());
}

mitk::AbstractGlobalImageFeature::FeatureListType mitk::GIFLocalIntensity::DoCalculateFeatures(const Image* image, const Image* mask)
{
  FeatureListType featureList;

  if (image->GetDimension() < 3)
  {
    MITK_INFO << "Skipped GIFLocalIntensity. Only supports 3D images ....";
  }
  else
  {
    MITK_INFO << "Start calculating local intensity features ....";

    GIFLocalIntensityParameter params;
    params.range = GetRange();
    params.id = this->CreateTemplateFeatureID();
    AccessByItk_3(image, CalculateIntensityPeak, mask, featureList, params);

    MITK_INFO << "Finished calculating local intensity features....";
  }

  return featureList;
}

mitk::AbstractGlobalImageFeature::FeatureListType mitk::GIFLocalIntensity::CalculateFeatures(const Image* image, const Image* mask, const Image*)
{
  return Superclass::CalculateFeatures(image, mask);
}

void mitk::GIFLocalIntensity::ConfigureSettingsByParameters(const ParametersType& parameters)
{
  auto name = GetOptionPrefix() + "::range";
  if (parameters.count(name))
  {
    double range = us::any_cast<float>(parameters.at(name));
    this->SetRange(range);
  }
}
