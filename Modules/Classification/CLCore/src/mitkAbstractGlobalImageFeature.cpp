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

}
void  mitk::AbstractGlobalImageFeature::InitializeQuantifier(const Image::Pointer & feature, const Image::Pointer &mask, const Image::Pointer &maskNoNAN, FeatureListType &featureList, unsigned int defaultBins)
{
  bool useBins = false;
  unsigned int bins = 0;

  bool useBinsize = false;
  double binsize = 0;

  bool useMinimum = false;
  double minimum = 0;

  bool useMaximum = false;
  double maximum = 0;

  auto parsedArgs = GetParameter();
  std::string name = GetOptionPrefix();

  if (parsedArgs.count("bins"))
  {
    bins = us::any_cast<int>(parsedArgs["bins"]);
    useBins = true;
  }
  if (parsedArgs.count(name + "::bins"))
  {
    bins = us::any_cast<int>(parsedArgs[name + "::bins"]);
    useBins = true;
  }
  if (parsedArgs.count("binsize"))
  {
    binsize = us::any_cast<float>(parsedArgs["binsize"]);
    useBinsize = true;
  }
  if (parsedArgs.count(name + "::binsize"))
  {
    binsize = us::any_cast<float>(parsedArgs[name + "::binsize"]);
    useBinsize = true;
  }
  if (parsedArgs.count("minimum-intensity"))
  {
    minimum = us::any_cast<float>(parsedArgs["minimum-intensity"]);
    useMinimum = true;
  }
  if (parsedArgs.count(name + "::minimum"))
  {
    minimum = us::any_cast<float>(parsedArgs[name + "::minimum"]);
    useMinimum = true;
  }
  if (parsedArgs.count("maximum-intensity"))
  {
    maximum = us::any_cast<float>(parsedArgs["maximum-intensity"]);
    useMaximum = true;
  }
  if (parsedArgs.count(name + "::maximum"))
  {
    maximum = us::any_cast<float>(parsedArgs[name + "::maximum"]);
    useMaximum = true;
  }

  MITK_INFO << useMinimum << " " << useMaximum << " " << useBins << " " << useBinsize;

  m_Quantifier = IntensityQuantifier::New();
  if (useMinimum && useMaximum && useBinsize)
    m_Quantifier->InitializeByBinsizeAndMaximum(minimum, maximum, binsize);
  else if (useMinimum && useBins && useBinsize)
    m_Quantifier->InitializeByBinsizeAndBins(minimum, bins, binsize);
  else if (useMinimum && useMaximum && useBins)
    m_Quantifier->InitializeByMinimumMaximum(minimum, maximum, bins);
  else if (useBinsize)
    m_Quantifier->InitializeByImageRegionAndBinsize(feature, mask, binsize);
  else if (useBins)
    m_Quantifier->InitializeByImageRegion(feature, mask, bins);
  else
    m_Quantifier->InitializeByImage(feature, defaultBins);
}
