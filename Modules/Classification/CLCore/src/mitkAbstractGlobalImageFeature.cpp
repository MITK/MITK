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

#include <mitkAbstractGlobalImageFeature.h>

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

void  mitk::AbstractGlobalImageFeature::AddQuantifierArguments(mitkCommandLineParser &parser)
{
  std::string name = GetOptionPrefix();

  parser.addArgument(name + "::minimum", name + "::min", mitkCommandLineParser::Float, "Minium Intensity for Quantification", "Defines the minimum Intensity used for Quantification", us::Any());
  parser.addArgument(name + "::maximum", name + "::max", mitkCommandLineParser::Float, "Maximum Intensity for Quantification", "Defines the maximum Intensity used for Quantification", us::Any());
  parser.addArgument(name + "::bins", name + "::bins", mitkCommandLineParser::Int, "Number of Bins", "Define the number of bins that is used ", us::Any());
  parser.addArgument(name + "::binsize", name + "::binsize", mitkCommandLineParser::Float, "Binsize", "Define the size of the used bins", us::Any());
  parser.addArgument(name + "::ignore-global-histogram", name + "::ignore-global-histogram", mitkCommandLineParser::Bool, "Ignore the global histogram Parameters", "Ignores the global histogram parameters", us::Any());
  parser.addArgument(name + "::ignore-mask-for-histogram", name + "::ignore-mask", mitkCommandLineParser::Bool, "Ignore the global histogram Parameters", "Ignores the global histogram parameters", us::Any());

}

void  mitk::AbstractGlobalImageFeature::InitializeQuantifierFromParameters(const Image::Pointer & feature, const Image::Pointer &mask, unsigned int defaultBins)
{
  unsigned int bins = 0;
  double binsize = 0;
  double minimum = 0;
  double maximum = 0;

  auto parsedArgs = GetParameter();
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
  InitializeQuantifier(feature, mask, defaultBins);
}

void  mitk::AbstractGlobalImageFeature::InitializeQuantifier(const Image::Pointer & feature, const Image::Pointer &mask, unsigned int defaultBins)
{
  MITK_INFO << GetUseMinimumIntensity() << " " << GetUseMaximumIntensity() << " " << GetUseBins() << " " << GetUseBinsize();

  m_Quantifier = IntensityQuantifier::New();
  if (GetUseMinimumIntensity() && GetUseMaximumIntensity() && GetUseBinsize())
    m_Quantifier->InitializeByBinsizeAndMaximum(GetMinimumIntensity(), GetMaximumIntensity(), GetBinsize());
  else if (GetUseMinimumIntensity() && GetUseBins() && GetUseBinsize())
    m_Quantifier->InitializeByBinsizeAndBins(GetMinimumIntensity(), GetBins(), GetBinsize());
  else if (GetUseMinimumIntensity() && GetUseMaximumIntensity() && GetUseBins())
    m_Quantifier->InitializeByMinimumMaximum(GetMinimumIntensity(), GetMaximumIntensity(), GetBins());
  // Intialize from Image and Binsize
  else if (GetUseBinsize() && GetIgnoreMask() && GetUseMinimumIntensity())
    m_Quantifier->InitializeByImageAndBinsizeAndMinimum(feature, GetMinimumIntensity(), GetBinsize());
  else if (GetUseBinsize() && GetIgnoreMask() && GetUseMaximumIntensity())
    m_Quantifier->InitializeByImageAndBinsizeAndMaximum(feature, GetMaximumIntensity(), GetBinsize());
  else if (GetUseBinsize() && GetIgnoreMask())
    m_Quantifier->InitializeByImageAndBinsize(feature, GetBinsize());
  // Initialize form Image, Mask and Binsize
  else if (GetUseBinsize() && GetUseMinimumIntensity())
    m_Quantifier->InitializeByImageRegionAndBinsizeAndMinimum(feature, mask, GetMinimumIntensity(), GetBinsize());
  else if (GetUseBinsize() && GetUseMaximumIntensity())
    m_Quantifier->InitializeByImageRegionAndBinsizeAndMaximum(feature, mask, GetMaximumIntensity(), GetBinsize());
  else if (GetUseBinsize())
    m_Quantifier->InitializeByImageRegionAndBinsize(feature, mask, GetBinsize());
  // Intialize from Image and Bins
  else if (GetUseBins() && GetIgnoreMask() && GetUseMinimumIntensity())
    m_Quantifier->InitializeByImageAndMinimum(feature, GetMinimumIntensity(), GetBins());
  else if (GetUseBins() && GetIgnoreMask() && GetUseMaximumIntensity())
    m_Quantifier->InitializeByImageAndMaximum(feature, GetMaximumIntensity(), GetBins());
  else if (GetUseBins())
    m_Quantifier->InitializeByImage(feature, GetBins());
  // Intialize from Image, Mask and Bins
  else if (GetUseBins() && GetUseMinimumIntensity())
    m_Quantifier->InitializeByImageRegionAndMinimum(feature, mask, GetMinimumIntensity(), GetBins());
  else if (GetUseBins() && GetUseMaximumIntensity())
    m_Quantifier->InitializeByImageRegionAndMaximum(feature, mask, GetMaximumIntensity(), GetBins());
  else if (GetUseBins())
    m_Quantifier->InitializeByImageRegion(feature, mask, GetBins());
  // Default
  else if (GetIgnoreMask())
    m_Quantifier->InitializeByImage(feature, GetBins());
  else
    m_Quantifier->InitializeByImageRegion(feature, mask, defaultBins);
}

std::string mitk::AbstractGlobalImageFeature::GetCurrentFeatureEncoding()
{
  return "";
}

std::string mitk::AbstractGlobalImageFeature::FeatureDescriptionPrefix()
{
  std::string output;
  output = m_FeatureClassName + "::";
  if (m_EncodeParameters)
  {
    output += GetCurrentFeatureEncoding() + "::";
  }
  return output;
}

std::string mitk::AbstractGlobalImageFeature::QuantifierParameterString()
{
  std::stringstream ss;
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