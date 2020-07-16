/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkGIFCooccurenceMatrix.h>

// MITK
#include <mitkITKImageImport.h>
#include <mitkImageCast.h>
#include <mitkImageAccessByItk.h>

// ITK
#include <itkEnhancedScalarImageToTextureFeaturesFilter.h>
#include <itkMinimumMaximumImageCalculator.h>

// STL
#include <sstream>

struct GIFCooccurenceMatrixConfiguration
{
  double range;
  unsigned int direction;
  double MinimumIntensity;
  bool UseMinimumIntensity;
  double MaximumIntensity;
  bool UseMaximumIntensity;
  int Bins;
  mitk::FeatureID id;
};

template<typename TPixel, unsigned int VImageDimension>
void
CalculateCoocurenceFeatures(const itk::Image<TPixel, VImageDimension>* itkImage, const mitk::Image* mask, mitk::GIFCooccurenceMatrix::FeatureListType & featureList, GIFCooccurenceMatrixConfiguration config)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<TPixel, VImageDimension> MaskType;
  typedef itk::Statistics::EnhancedScalarImageToTextureFeaturesFilter<ImageType> FilterType;
  typedef itk::MinimumMaximumImageCalculator<ImageType> MinMaxComputerType;
  typedef typename FilterType::TextureFeaturesFilterType TextureFilterType;

  typename MaskType::Pointer maskImage = MaskType::New();
  mitk::CastToItkImage(mask, maskImage);

  typename FilterType::Pointer filter = FilterType::New();

  typename FilterType::OffsetVector::Pointer newOffset = FilterType::OffsetVector::New();
  auto oldOffsets = filter->GetOffsets();
  auto oldOffsetsIterator = oldOffsets->Begin();
  while(oldOffsetsIterator != oldOffsets->End())
  {
    bool continueOuterLoop = false;
    typename FilterType::OffsetType offset = oldOffsetsIterator->Value();
    for (unsigned int i = 0; i < VImageDimension; ++i)
    {
      offset[i] *= config.range;
      if (config.direction == i + 2 && offset[i] != 0)
      {
        continueOuterLoop = true;
      }
    }
    if (config.direction == 1)
    {
      offset[0] = 0;
      offset[1] = 0;
      offset[2] = 1;
      newOffset->push_back(offset);
      break;
    }


    oldOffsetsIterator++;
    if (continueOuterLoop)
      continue;
    newOffset->push_back(offset);

  }
  filter->SetOffsets(newOffset);

  // All features are required
  typename FilterType::FeatureNameVectorPointer requestedFeatures = FilterType::FeatureNameVector::New();
  requestedFeatures->push_back(TextureFilterType::Energy);
  requestedFeatures->push_back(TextureFilterType::Entropy);
  requestedFeatures->push_back(TextureFilterType::Correlation);
  requestedFeatures->push_back(TextureFilterType::InverseDifferenceMoment);
  requestedFeatures->push_back(TextureFilterType::Inertia);
  requestedFeatures->push_back(TextureFilterType::ClusterShade);
  requestedFeatures->push_back(TextureFilterType::ClusterProminence);
  requestedFeatures->push_back(TextureFilterType::HaralickCorrelation);
  requestedFeatures->push_back(TextureFilterType::Autocorrelation);
  requestedFeatures->push_back(TextureFilterType::Contrast);
  requestedFeatures->push_back(TextureFilterType::Dissimilarity);
  requestedFeatures->push_back(TextureFilterType::MaximumProbability);
  requestedFeatures->push_back(TextureFilterType::InverseVariance);
  requestedFeatures->push_back(TextureFilterType::Homogeneity1);
  requestedFeatures->push_back(TextureFilterType::ClusterTendency);
  requestedFeatures->push_back(TextureFilterType::Variance);
  requestedFeatures->push_back(TextureFilterType::SumAverage);
  requestedFeatures->push_back(TextureFilterType::SumEntropy);
  requestedFeatures->push_back(TextureFilterType::SumVariance);
  requestedFeatures->push_back(TextureFilterType::DifferenceAverage);
  requestedFeatures->push_back(TextureFilterType::DifferenceEntropy);
  requestedFeatures->push_back(TextureFilterType::DifferenceVariance);
  requestedFeatures->push_back(TextureFilterType::InverseDifferenceMomentNormalized);
  requestedFeatures->push_back(TextureFilterType::InverseDifferenceNormalized);
  requestedFeatures->push_back(TextureFilterType::InverseDifference);
  requestedFeatures->push_back(TextureFilterType::JointAverage);
  requestedFeatures->push_back(TextureFilterType::FirstMeasureOfInformationCorrelation);
  requestedFeatures->push_back(TextureFilterType::SecondMeasureOfInformationCorrelation);

  typename MinMaxComputerType::Pointer minMaxComputer = MinMaxComputerType::New();
  minMaxComputer->SetImage(itkImage);
  minMaxComputer->Compute();

  filter->SetInput(itkImage);
  filter->SetMaskImage(maskImage);
  filter->SetRequestedFeatures(requestedFeatures);

  double min = minMaxComputer->GetMinimum() - 0.5;
  double max = minMaxComputer->GetMaximum() + 0.5;
  if (config.UseMinimumIntensity)
    min = config.MinimumIntensity;
  if (config.UseMaximumIntensity)
    max = config.MaximumIntensity;

  filter->SetPixelValueMinMax(min,max);
  //filter->SetPixelValueMinMax(-1024,3096);
  //filter->SetNumberOfBinsPerAxis(5);
  filter->Update();

  auto featureMeans = filter->GetFeatureMeans ();
  auto featureStd = filter->GetFeatureStandardDeviations();

  std::ostringstream  ss;
  ss << config.range;
  std::string strRange = ss.str();
  for (std::size_t i = 0; i < featureMeans->size(); ++i)
  {
    switch (i)
    {
    case TextureFilterType::Energy :
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. Energy Means"), featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. Energy Std."), featureStd->ElementAt(i)));
      break;
    case TextureFilterType::Entropy :
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. Entropy Means"), featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. Entropy Std."), featureStd->ElementAt(i)));
      break;
    case TextureFilterType::Correlation :
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. Correlation Means"), featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. Correlation Std."), featureStd->ElementAt(i)));
      break;
    case TextureFilterType::InverseDifferenceMoment :
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. InverseDifferenceMoment Means"), featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. InverseDifferenceMoment Std."), featureStd->ElementAt(i)));
      break;
    case TextureFilterType::Inertia :
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. Inertia Means"), featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. Inertia Std."), featureStd->ElementAt(i)));
      break;
    case TextureFilterType::ClusterShade :
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. ClusterShade Means"), featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. ClusterShade Std."), featureStd->ElementAt(i)));
      break;
    case TextureFilterType::ClusterProminence :
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. ClusterProminence Means"), featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. ClusterProminence Std."), featureStd->ElementAt(i)));
      break;
    case TextureFilterType::HaralickCorrelation :
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. HaralickCorrelation Means"), featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. HaralickCorrelation Std."), featureStd->ElementAt(i)));
      break;
    case TextureFilterType::Autocorrelation :
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. Autocorrelation Means"), featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. Autocorrelation Std."), featureStd->ElementAt(i)));
      break;
    case TextureFilterType::Contrast :
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. Contrast Means"), featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. Contrast Std."), featureStd->ElementAt(i)));
      break;
    case TextureFilterType::Dissimilarity :
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. Dissimilarity Means"), featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. Dissimilarity Std."), featureStd->ElementAt(i)));
      break;
    case TextureFilterType::MaximumProbability :
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. MaximumProbability Means"), featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. MaximumProbability Std."), featureStd->ElementAt(i)));
      break;
    case TextureFilterType::InverseVariance :
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. InverseVariance Means"), featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. InverseVariance Std."), featureStd->ElementAt(i)));
      break;
    case TextureFilterType::Homogeneity1 :
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. Homogeneity1 Means"), featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. Homogeneity1 Std."), featureStd->ElementAt(i)));
      break;
    case TextureFilterType::ClusterTendency :
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. ClusterTendency Means"), featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. ClusterTendency Std."), featureStd->ElementAt(i)));
      break;
    case TextureFilterType::Variance :
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. Variance Means"), featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. Variance Std."), featureStd->ElementAt(i)));
      break;
    case TextureFilterType::SumAverage :
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. SumAverage Means"), featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. SumAverage Std."), featureStd->ElementAt(i)));
      break;
    case TextureFilterType::SumEntropy :
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. SumEntropy Means"), featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. SumEntropy Std."), featureStd->ElementAt(i)));
      break;
    case TextureFilterType::SumVariance :
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. SumVariance Means"), featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. SumVariance Std."), featureStd->ElementAt(i)));
      break;
    case TextureFilterType::DifferenceAverage :
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. DifferenceAverage Means"), featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. DifferenceAverage Std."), featureStd->ElementAt(i)));
      break;
    case TextureFilterType::DifferenceEntropy :
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. DifferenceEntropy Means"), featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. DifferenceEntropy Std."), featureStd->ElementAt(i)));
      break;
    case TextureFilterType::DifferenceVariance :
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. DifferenceVariance Means"), featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. DifferenceVariance Std."), featureStd->ElementAt(i)));
      break;
    case TextureFilterType::InverseDifferenceMomentNormalized :
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. InverseDifferenceMomentNormalized Means"), featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. InverseDifferenceMomentNormalized Std."), featureStd->ElementAt(i)));
      break;
    case TextureFilterType::InverseDifferenceNormalized :
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. InverseDifferenceNormalized Means"), featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. InverseDifferenceNormalized Std."), featureStd->ElementAt(i)));
      break;
    case TextureFilterType::InverseDifference :
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. InverseDifference Means"), featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. InverseDifference Std."), featureStd->ElementAt(i)));
      break;
    case TextureFilterType::JointAverage :
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. JointAverage Means"), featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. JointAverage Std."), featureStd->ElementAt(i)));
      break;
    case TextureFilterType::FirstMeasureOfInformationCorrelation :
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. FirstMeasureOfInformationCorrelation Means"), featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. FirstMeasureOfInformationCorrelation Std."), featureStd->ElementAt(i)));
      break;
    case TextureFilterType::SecondMeasureOfInformationCorrelation :
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. SecondMeasureOfInformationCorrelation Means"), featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(config.id, "co-occ. SecondMeasureOfInformationCorrelation Std."), featureStd->ElementAt(i)));
      break;
    default:
      break;
    }
  }
}

mitk::GIFCooccurenceMatrix::GIFCooccurenceMatrix():
  m_Ranges({ 1.0 })
{
  SetShortName("deprecated-cooc");
  SetLongName("deprecated-cooccurence");
  SetFeatureClassName("Deprecated Co-occurence Features");
}

void mitk::GIFCooccurenceMatrix::SetRanges(std::vector<double> ranges)
{
  m_Ranges = ranges;
  this->Modified();
}

void mitk::GIFCooccurenceMatrix::SetRange(double range)
{
  m_Ranges.resize(1);
  m_Ranges[0] = range;
  this->Modified();
}

mitk::AbstractGlobalImageFeature::FeatureListType mitk::GIFCooccurenceMatrix::DoCalculateFeatures(const Image* image, const Image* mask)
{
  FeatureListType featureList;

  InitializeQuantifier(image, mask);

  for (const auto& range : m_Ranges)
  {
    MITK_INFO << "Start calculating coocurence with range " << range << "....";

    GIFCooccurenceMatrixConfiguration config;
    config.direction = GetDirection();
    config.range = range;
    config.MinimumIntensity = GetQuantifier()->GetMinimum();
    config.MaximumIntensity = GetQuantifier()->GetMaximum();
    config.UseMinimumIntensity = GetUseMinimumIntensity();
    config.UseMaximumIntensity = GetUseMaximumIntensity();
    config.Bins = GetBins();
    config.id = this->CreateTemplateFeatureID(std::to_string(range), { {GetOptionPrefix() + "::range", range} });

    AccessByItk_3(image, CalculateCoocurenceFeatures, mask, featureList, config);

    MITK_INFO << "Finished calculating coocurence with range " << range << "....";
  }

  return featureList;
}

mitk::AbstractGlobalImageFeature::FeatureListType mitk::GIFCooccurenceMatrix::CalculateFeatures(const Image* image, const Image*, const Image* maskNoNAN)
{
  return Superclass::CalculateFeatures(image, maskNoNAN);
}

void mitk::GIFCooccurenceMatrix::AddArguments(mitkCommandLineParser& parser) const
{
  std::string name = this->GetOptionPrefix();

  parser.addArgument(this->GetLongName(), name, mitkCommandLineParser::Bool, "Use Co-occurence matrix", "calculates Co-occurence based features", us::Any());
  parser.addArgument(name + "::range", name + "::range", mitkCommandLineParser::String, "Cooc Range", "Define the range that is used (Semicolon-separated)", us::Any());
}

std::string mitk::GIFCooccurenceMatrix::GenerateLegacyFeatureNamePart(const FeatureID& id) const
{
  auto result = id.name;
  result.insert(7, " (" + id.parameters.at(this->GetOptionPrefix() + "::range").ToString() + ")");
  return result;
}

std::string mitk::GIFCooccurenceMatrix::GenerateLegacyFeatureEncoding(const FeatureID& /*id*/) const
{
  //deprecated GIFCooccurenceMatrix does not support feature encoding
  return "";
}

void mitk::GIFCooccurenceMatrix::ConfigureSettingsByParameters(const ParametersType& parameters)
{
  auto name = GetOptionPrefix() + "::range";
  if (parameters.count(name))
  {
    m_Ranges = SplitDouble(parameters.at(name).ToString(), ';');
  }
}
