/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkAbstractGlobalImageFeature.h>

#include <mitkImageCast.h>
#include <mitkITKImageImport.h>
#include <iterator>


bool mitk::FeatureID::operator < (const FeatureID& rh) const
{
  if (this->featureClass < rh.featureClass) return true;
  if (this->featureClass == rh.featureClass)
  {
    if (this->name < rh.name) return true;
    if (this->name == rh.name && this->settingID < rh.settingID) return true;
  }
  return false;
}

bool mitk::FeatureID::operator ==(const FeatureID& rh) const
{
  if (this->name != rh.name) return false;
  if (this->settingID != rh.settingID) return false;
  if (this->featureClass != rh.featureClass) return false;
  return true;
}

mitk::FeatureID mitk::CreateFeatureID(FeatureID templateID, std::string name)
{
  auto newID = templateID;
  newID.name = name;
  return newID;
}

static void
ExtractSlicesFromImages(mitk::Image::Pointer image, mitk::Image::Pointer mask,
  int direction,
  std::vector<mitk::Image::Pointer> &imageVector,
  std::vector<mitk::Image::Pointer> &maskVector)
{
  typedef itk::Image< double, 2 >                 FloatImage2DType;
  typedef itk::Image< unsigned short, 2 >          MaskImage2DType;
  typedef itk::Image< double, 3 >                 FloatImageType;
  typedef itk::Image< unsigned short, 3 >          MaskImageType;

  FloatImageType::Pointer itkFloat = FloatImageType::New();
  MaskImageType::Pointer itkMask = MaskImageType::New();
  mitk::CastToItkImage(mask, itkMask);
  mitk::CastToItkImage(image, itkFloat);

  int idxA, idxB, idxC;
  switch (direction)
  {
  case 0:
    idxA = 1; idxB = 2; idxC = 0;
    break;
  case 1:
    idxA = 0; idxB = 2; idxC = 1;
    break;
  case 2:
    idxA = 0; idxB = 1; idxC = 2;
    break;
  default:
    idxA = 1; idxB = 2; idxC = 0;
    break;
  }

  auto imageSize = image->GetLargestPossibleRegion().GetSize();
  FloatImageType::IndexType index3D;
  FloatImage2DType::IndexType index2D;
  FloatImage2DType::SpacingType spacing2D;
  spacing2D[0] = itkFloat->GetSpacing()[idxA];
  spacing2D[1] = itkFloat->GetSpacing()[idxB];

  for (unsigned int i = 0; i < imageSize[idxC]; ++i)
  {
    FloatImage2DType::RegionType region;
    FloatImage2DType::IndexType start;
    FloatImage2DType::SizeType size;
    start[0] = 0; start[1] = 0;
    size[0] = imageSize[idxA];
    size[1] = imageSize[idxB];
    region.SetIndex(start);
    region.SetSize(size);

    FloatImage2DType::Pointer image2D = FloatImage2DType::New();
    image2D->SetRegions(region);
    image2D->Allocate();

    MaskImage2DType::Pointer mask2D = MaskImage2DType::New();
    mask2D->SetRegions(region);
    mask2D->Allocate();

    unsigned long voxelsInMask = 0;

    for (unsigned int a = 0; a < imageSize[idxA]; ++a)
    {
      for (unsigned int b = 0; b < imageSize[idxB]; ++b)
      {
        index3D[idxA] = a;
        index3D[idxB] = b;
        index3D[idxC] = i;
        index2D[0] = a;
        index2D[1] = b;
        image2D->SetPixel(index2D, itkFloat->GetPixel(index3D));
        mask2D->SetPixel(index2D, itkMask->GetPixel(index3D));
        voxelsInMask += (itkMask->GetPixel(index3D) > 0) ? 1 : 0;

      }
    }

    image2D->SetSpacing(spacing2D);
    mask2D->SetSpacing(spacing2D);

    mitk::Image::Pointer tmpFloatImage = mitk::Image::New();
    tmpFloatImage->InitializeByItk(image2D.GetPointer());
    mitk::GrabItkImageMemory(image2D, tmpFloatImage);

    mitk::Image::Pointer tmpMaskImage = mitk::Image::New();
    tmpMaskImage->InitializeByItk(mask2D.GetPointer());
    mitk::GrabItkImageMemory(mask2D, tmpMaskImage);

    if (voxelsInMask > 0)
    {
      imageVector.push_back(tmpFloatImage);
      maskVector.push_back(tmpMaskImage);
    }
  }
}

std::vector<double> mitk::AbstractGlobalImageFeature::SplitDouble(std::string str, char delimiter) {
  std::vector<double> internal;
  std::stringstream ss(str); // Turn the string into a stream.
  std::string tok;
  double val;
  while (std::getline(ss, tok, delimiter)) {
    std::stringstream s2(tok);
    s2 >> val;
    internal.push_back(val);
  }

  return internal;
}

void  mitk::AbstractGlobalImageFeature::AddQuantifierArguments(mitkCommandLineParser &parser) const
{
  std::string name = GetOptionPrefix();

  parser.addArgument(name + "::minimum", name + "::min", mitkCommandLineParser::Float, "Minium Intensity for Quantification", "Defines the minimum Intensity used for Quantification", us::Any());
  parser.addArgument(name + "::maximum", name + "::max", mitkCommandLineParser::Float, "Maximum Intensity for Quantification", "Defines the maximum Intensity used for Quantification", us::Any());
  parser.addArgument(name + "::bins", name + "::bins", mitkCommandLineParser::Int, "Number of Bins", "Define the number of bins that is used ", us::Any());
  parser.addArgument(name + "::binsize", name + "::binsize", mitkCommandLineParser::Float, "Binsize", "Define the size of the used bins", us::Any());
  parser.addArgument(name + "::ignore-global-histogram", name + "::ignore-global-histogram", mitkCommandLineParser::Bool, "Ignore the global histogram Parameters", "Ignores the global histogram parameters", us::Any());
  parser.addArgument(name + "::ignore-mask-for-histogram", name + "::ignore-mask", mitkCommandLineParser::Bool, "Ignore the global histogram Parameters", "Ignores the global histogram parameters", us::Any());
}

void  mitk::AbstractGlobalImageFeature::ConfigureQuantifierSettingsByParameters()
{
  unsigned int bins = 0;
  double binsize = 0;
  double minimum = 0;
  double maximum = 0;

  auto parsedArgs = GetParameters();
  std::string name = GetOptionPrefix();

  bool useGlobal = true;
  if (parsedArgs.count(name + "::ignore-global-histogram"))
  {
    useGlobal = false;
    SetUseMinimumIntensity(false);
    SetUseMaximumIntensity(false);
    SetUseBinsize(false);
    SetUseBins(false);
  }

  if (useGlobal)
  {
    if (parsedArgs.count("ignore-mask-for-histogram"))
    {
      bool tmp = us::any_cast<bool>(parsedArgs["ignore-mask-for-histogram"]);
      SetIgnoreMask(tmp);
    }
    if (parsedArgs.count("minimum-intensity"))
    {
      minimum = us::any_cast<float>(parsedArgs["minimum-intensity"]);
      SetMinimumIntensity(minimum);
      SetUseMinimumIntensity(true);
    }
    if (parsedArgs.count("maximum-intensity"))
    {
      maximum = us::any_cast<float>(parsedArgs["maximum-intensity"]);
      SetMaximumIntensity(maximum);
      SetUseMaximumIntensity(true);
    }
    if (parsedArgs.count("bins"))
    {
      bins = us::any_cast<int>(parsedArgs["bins"]);
      SetBins(bins);
      SetUseBins(true);
    }
    if (parsedArgs.count("binsize"))
    {
      binsize = us::any_cast<float>(parsedArgs["binsize"]);
      SetBinsize(binsize);
      SetUseBinsize(true);
    }
  }
  if (parsedArgs.count(name+"::ignore-mask-for-histogram"))
  {
    bool tmp = us::any_cast<bool>(parsedArgs[name+"::ignore-mask-for-histogram"]);
    SetIgnoreMask(tmp);
  }
  if (parsedArgs.count(name + "::minimum"))
  {
    minimum = us::any_cast<float>(parsedArgs[name + "::minimum"]);
    SetMinimumIntensity(minimum);
    SetUseMinimumIntensity(true);
  }
  if (parsedArgs.count(name + "::maximum"))
  {
    maximum = us::any_cast<float>(parsedArgs[name + "::maximum"]);
    SetMaximumIntensity(maximum);
    SetUseMaximumIntensity(true);
  }
  if (parsedArgs.count(name + "::bins"))
  {
    bins = us::any_cast<int>(parsedArgs[name + "::bins"]);
    SetBins(bins);
  }
  if (parsedArgs.count(name + "::binsize"))
  {
    binsize = us::any_cast<float>(parsedArgs[name + "::binsize"]);
    SetBinsize(binsize);
    SetUseBinsize(true);
  }
}

void mitk::AbstractGlobalImageFeature::ConfigureSettingsByParameters(const ParametersType& /*parameters*/)
{
  //Default implementation does nothing.
  //Override to change behavior.
}

void  mitk::AbstractGlobalImageFeature::InitializeQuantifier(const Image* image, const Image* mask, unsigned int defaultBins)
{
  m_Quantifier = IntensityQuantifier::New();
  if (GetUseMinimumIntensity() && GetUseMaximumIntensity() && GetUseBinsize())
    m_Quantifier->InitializeByBinsizeAndMaximum(GetMinimumIntensity(), GetMaximumIntensity(), GetBinsize());
  else if (GetUseMinimumIntensity() && GetUseBins() && GetUseBinsize())
    m_Quantifier->InitializeByBinsizeAndBins(GetMinimumIntensity(), GetBins(), GetBinsize());
  else if (GetUseMinimumIntensity() && GetUseMaximumIntensity() && GetUseBins())
    m_Quantifier->InitializeByMinimumMaximum(GetMinimumIntensity(), GetMaximumIntensity(), GetBins());
  // Intialize from Image and Binsize
  else if (GetUseBinsize() && GetIgnoreMask() && GetUseMinimumIntensity())
    m_Quantifier->InitializeByImageAndBinsizeAndMinimum(image, GetMinimumIntensity(), GetBinsize());
  else if (GetUseBinsize() && GetIgnoreMask() && GetUseMaximumIntensity())
    m_Quantifier->InitializeByImageAndBinsizeAndMaximum(image, GetMaximumIntensity(), GetBinsize());
  else if (GetUseBinsize() && GetIgnoreMask())
    m_Quantifier->InitializeByImageAndBinsize(image, GetBinsize());
  // Initialize form Image, Mask and Binsize
  else if (GetUseBinsize() && GetUseMinimumIntensity())
    m_Quantifier->InitializeByImageRegionAndBinsizeAndMinimum(image, mask, GetMinimumIntensity(), GetBinsize());
  else if (GetUseBinsize() && GetUseMaximumIntensity())
    m_Quantifier->InitializeByImageRegionAndBinsizeAndMaximum(image, mask, GetMaximumIntensity(), GetBinsize());
  else if (GetUseBinsize())
    m_Quantifier->InitializeByImageRegionAndBinsize(image, mask, GetBinsize());
  // Intialize from Image and Bins
  else if (GetUseBins() && GetIgnoreMask() && GetUseMinimumIntensity())
    m_Quantifier->InitializeByImageAndMinimum(image, GetMinimumIntensity(), GetBins());
  else if (GetUseBins() && GetIgnoreMask() && GetUseMaximumIntensity())
    m_Quantifier->InitializeByImageAndMaximum(image, GetMaximumIntensity(), GetBins());
  else if (GetUseBins())
    m_Quantifier->InitializeByImage(image, GetBins());
  // Intialize from Image, Mask and Bins
  else if (GetUseBins() && GetUseMinimumIntensity())
    m_Quantifier->InitializeByImageRegionAndMinimum(image, mask, GetMinimumIntensity(), GetBins());
  else if (GetUseBins() && GetUseMaximumIntensity())
    m_Quantifier->InitializeByImageRegionAndMaximum(image, mask, GetMaximumIntensity(), GetBins());
  else if (GetUseBins())
    m_Quantifier->InitializeByImageRegion(image, mask, GetBins());
  // Default
  else if (GetIgnoreMask())
    m_Quantifier->InitializeByImage(image, GetBins());
  else
    m_Quantifier->InitializeByImageRegion(image, mask, defaultBins);
}

std::string mitk::AbstractGlobalImageFeature::GenerateLegacyFeatureName(const FeatureID& id) const
{
  std::string output;
  output = m_FeatureClassName + "::";
  if (m_EncodeParametersInFeaturePrefix)
  {
    output += this->GenerateLegacyFeatureEncoding(id) + "::";
  }

  return output + this->GenerateLegacyFeatureNamePart(id);
};

std::string mitk::AbstractGlobalImageFeature::GenerateLegacyFeatureNamePart(const FeatureID& id) const
{
  return id.name;
}

std::string mitk::AbstractGlobalImageFeature::GenerateLegacyFeatureEncoding(const FeatureID& /*id*/) const
{
  return this->QuantifierParameterString();
}

std::string mitk::AbstractGlobalImageFeature::QuantifierParameterString() const
{
  std::stringstream ss;
  ss.imbue(std::locale("c"));
  if (GetUseMinimumIntensity() && GetUseMaximumIntensity() && GetUseBinsize())
    ss << "Min-" << GetMinimumIntensity() << "_Max-" << GetMaximumIntensity() << "_BS-" << GetBinsize();
  else if (GetUseMinimumIntensity() && GetUseBins() && GetUseBinsize())
    ss << "Min-" << GetMinimumIntensity() << "_Bins-" << GetBins() << "_BS-" << GetBinsize();
  else if (GetUseMinimumIntensity() && GetUseMaximumIntensity() && GetUseBins())
    ss << "Min-" << GetMinimumIntensity() << "_Max-" << GetMaximumIntensity() << "_Bins-" << GetBins();
  // Intialize from Image and Binsize
  else if (GetUseBinsize() && GetIgnoreMask() && GetUseMinimumIntensity())
    ss << "Min-" << GetMinimumIntensity() << "_BS-" << GetBinsize() << "_FullImage";
  else if (GetUseBinsize() && GetIgnoreMask() && GetUseMaximumIntensity())
    ss << "Max-" << GetMaximumIntensity() << "_BS-" << GetBinsize() << "_FullImage";
  else if (GetUseBinsize() && GetIgnoreMask())
    ss << "BS-" << GetBinsize() << "_FullImage";
  // Initialize form Image, Mask and Binsize
  else if (GetUseBinsize() && GetUseMinimumIntensity())
    ss << "Min-" << GetMinimumIntensity() << "_BS-" << GetBinsize();
  else if (GetUseBinsize() && GetUseMaximumIntensity())
    ss << "Max-" << GetMaximumIntensity() << "_BS-" << GetBinsize();
  else if (GetUseBinsize())
    ss << "BS-" << GetBinsize();
  // Intialize from Image and Bins
  else if (GetUseBins() && GetIgnoreMask() && GetUseMinimumIntensity())
    ss << "Min-" << GetMinimumIntensity() << "_Bins-" << GetBins() << "_FullImage";
  else if (GetUseBins() && GetIgnoreMask() && GetUseMaximumIntensity())
    ss << "Max-" << GetMaximumIntensity() << "_Bins-" << GetBins() << "_FullImage";
  else if (GetUseBins())
    ss << "Bins-" << GetBins() << "_FullImage";
  // Intialize from Image, Mask and Bins
  else if (GetUseBins() && GetUseMinimumIntensity())
    ss << "Min-" << GetMinimumIntensity() << "_Bins-" << GetBins();
  else if (GetUseBins() && GetUseMaximumIntensity())
    ss << "Max-" << GetMaximumIntensity() << "_Bins-" << GetBins();
  else if (GetUseBins())
    ss << "Bins-" << GetBins();
  // Default
  else if (GetIgnoreMask())
    ss << "Bins-" << GetBins() << "_FullImage";
  else
    ss << "Bins-" << GetBins();
  return ss.str();
}

mitk::FeatureID mitk::AbstractGlobalImageFeature::CreateTemplateFeatureID(std::string settingsSuffix, FeatureID::ParametersType additionalParams)
{
  FeatureID newID;
  newID.featureClass = this->GetFeatureClassName();
  newID.settingID = this->GetShortName();
  if (!settingsSuffix.empty())
  {
    newID.settingID += "_" + settingsSuffix;
  }

  std::string name = this->GetOptionPrefix();
  for (const auto& paramIter : m_Parameters)
  {
    if (paramIter.first.find(name) == 0)
    {
      newID.parameters.insert(std::make_pair(paramIter.first, paramIter.second));
    }
  }

  if (m_Quantifier.IsNotNull())
  { //feature class uses the quantifier. So store the information in the feature ID
    if (GetUseMinimumIntensity())
    {
      newID.parameters["minimum-intensity"] = us::Any(GetMinimumIntensity());
    }
    if (GetUseMaximumIntensity())
    {
      newID.parameters["minimum-intensity"] = us::Any(GetMaximumIntensity());
    }
    if (GetUseBinsize())
    {
      newID.parameters["binsize"] = us::Any(GetBinsize());
    }
    if (GetBins())
    {
      newID.parameters["bins"] = us::Any(GetBins());
    }
    if (GetIgnoreMask())
    {
      newID.parameters["ignore-mask-for-histogram"] = us::Any(true);
    }
  }

  for (const auto& paramIter : additionalParams)
  {
    newID.parameters[paramIter.first] = paramIter.second;
  }
  return newID;
}

void mitk::AbstractGlobalImageFeature::CalculateAndAppendFeaturesSliceWise(const Image::Pointer & image, const Image::Pointer &mask, int sliceID, FeatureListType &featureList, bool checkParameterActivation)
{
  auto parsedArgs = GetParameters();

  if (!checkParameterActivation || parsedArgs.count(GetLongName()))
  {
    auto result = CalculateFeaturesSlicewise(image, mask, sliceID);
    featureList.insert(featureList.end(), result.begin(), result.end());
  }
}

mitk::AbstractGlobalImageFeature::FeatureListType mitk::AbstractGlobalImageFeature::CalculateFeaturesSlicewise(const Image::Pointer & image, const Image::Pointer &mask, int sliceID)
{
  std::vector<mitk::Image::Pointer> imageVector;
  std::vector<mitk::Image::Pointer> maskVector;

  ExtractSlicesFromImages(image, mask,sliceID, imageVector, maskVector);
  std::vector<mitk::AbstractGlobalImageFeature::FeatureListType> statVector;

  for (std::size_t index = 0; index < imageVector.size(); ++index)
  {
    auto stat = this->CalculateFeatures(imageVector[index], maskVector[index]);
    statVector.push_back(stat);
  }

  if (statVector.size() < 1)
    return FeatureListType();

  FeatureListType statMean, statStd, result;
  for (std::size_t i = 0; i < statVector[0].size(); ++i)
  {
    auto cElement1 = statVector[0][i];
    cElement1.first.name = "SliceWise Mean " + cElement1.first.name;
    cElement1.first.legacyName = this->GenerateLegacyFeatureName(cElement1.first);
    cElement1.second = 0.0;
    auto cElement2 = statVector[0][i];
    cElement2.first.name = "SliceWise Var. " + cElement2.first.name;
    cElement2.first.legacyName = this->GenerateLegacyFeatureName(cElement2.first);
    cElement2.second = 0.0;
    statMean.push_back(cElement1);
    statStd.push_back(cElement2);
  }
  for (auto cStat : statVector)
  {
    for (std::size_t i = 0; i < cStat.size(); ++i)
    {
      statMean[i].second += cStat[i].second / (1.0*statVector.size());
    }
  }

  for (auto cStat : statVector)
  {
    for (std::size_t i = 0; i < cStat.size(); ++i)
    {
      statStd[i].second += (cStat[i].second - statMean[i].second)*(cStat[i].second - statMean[i].second) / (1.0*statVector.size());
    }
  }

  for (auto cStat : statVector)
  {
    std::copy(cStat.begin(), cStat.end(), std::back_inserter(result));
  }
  std::copy(statMean.begin(), statMean.end(), std::back_inserter(result));
  std::copy(statStd.begin(), statStd.end(), std::back_inserter(result));
  return result;
}

void mitk::AbstractGlobalImageFeature::CalculateAndAppendFeatures(const Image* image, const Image* mask, const Image* maskNoNAN, FeatureListType& featureList, bool checkParameterActivation)
{
  auto parsedArgs = this->GetParameters();

  if (!checkParameterActivation || parsedArgs.count(this->GetLongName()))
  {
    auto result = this->CalculateFeatures(image, mask, maskNoNAN);
    featureList.insert(featureList.end(), result.begin(), result.end());
  }
}

mitk::AbstractGlobalImageFeature::FeatureListType mitk::AbstractGlobalImageFeature::CalculateFeatures(const Image* image, const Image* mask)
{
  auto result = this->DoCalculateFeatures(image, mask);

  //ensure legacy names
  for (auto& feature : result)
  {
    feature.first.legacyName = this->GenerateLegacyFeatureName(feature.first);
  }

  return result;
}

std::string mitk::AbstractGlobalImageFeature::GenerateLegacyFeatureNameWOEncoding(const mitk::FeatureID& id)
{
  std::string result;

  if (id.name.find("SliceWise") == 0)
  {
    result = id.name.substr(0, 14) + " " + id.featureClass + "::" + id.name.substr(15, -1);
  }
  else
  {
    result = id.featureClass + "::" + id.name;
  }

  return result;
}
