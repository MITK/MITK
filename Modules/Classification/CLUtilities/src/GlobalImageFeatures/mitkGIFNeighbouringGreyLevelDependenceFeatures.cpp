#include <mitkGIFNeighbouringGreyLevelDependenceFeatures.h>

// MITK
#include <mitkITKImageImport.h>
#include <mitkImageCast.h>
#include <mitkImageAccessByItk.h>

// ITK
#include <itkEnhancedScalarImageToTextureFeaturesFilter.h>
#include <itkMinimumMaximumImageCalculator.h>
#include <itkNeighborhoodIterator.h>
#include <itkImageRegionConstIterator.h>

// STL
#include <sstream>

static
void MatrixFeaturesTo(mitk::NGLDMMatrixFeatures features,
                      std::string prefix,
                      mitk::GIFNeighbouringGreyLevelDependenceFeature::FeatureListType &featureList);
static
void CalculateMeanAndStdDevFeatures(std::vector<mitk::NGLDMMatrixFeatures> featureList,
                                    mitk::NGLDMMatrixFeatures &meanFeature,
                                    mitk::NGLDMMatrixFeatures  &stdFeature);
static
void NormalizeMatrixFeature(mitk::NGLDMMatrixFeatures &features,
                            std::size_t number);




mitk::NGLDMMatrixHolder::NGLDMMatrixHolder(double min, double max, int number, int depenence) :
                                            m_MinimumRange(min),
                                            m_MaximumRange(max),
                                            m_NumberOfBins(number),
                                            m_NumberOfDependences(depenence)
{
  m_Matrix.resize(number, depenence);
  m_Matrix.fill(0);
  m_Stepsize = (max - min) / (number);
}

int mitk::NGLDMMatrixHolder::IntensityToIndex(double intensity)
{
  return std::floor((intensity - m_MinimumRange) / m_Stepsize);
}

double mitk::NGLDMMatrixHolder::IndexToMinIntensity(int index)
{
  return m_MinimumRange + index * m_Stepsize;
}
double mitk::NGLDMMatrixHolder::IndexToMeanIntensity(int index)
{
  return m_MinimumRange + (index+0.5) * m_Stepsize;
}
double mitk::NGLDMMatrixHolder::IndexToMaxIntensity(int index)
{
  return m_MinimumRange + (index + 1) * m_Stepsize;
}

template<typename TPixel, unsigned int VImageDimension>
void
CalculateNGLDMMatrix(itk::Image<TPixel, VImageDimension>* itkImage,
                    itk::Image<unsigned char, VImageDimension>* mask,
                    int alpha,
                    int range,
                    int direction,
                    mitk::NGLDMMatrixHolder &holder)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<unsigned char, VImageDimension> MaskImageType;
  typedef itk::NeighborhoodIterator<ImageType> ShapeIterType;
  typedef itk::NeighborhoodIterator<MaskImageType> ShapeMaskIterType;

  holder.m_NumberOfCompleteNeighbourhoods = 0;
  holder.m_NumberOfNeighbourhoods = 0;
  holder.m_NumberOfNeighbourVoxels = 0;
  holder.m_NumberOfDependenceNeighbourVoxels = 0;

  MITK_INFO << direction;

  itk::Size<VImageDimension> radius;
  radius.Fill(range);

  if ((direction > 1) && (direction +2 <VImageDimension))
  {
    radius[direction - 2] = 0;
  }

  ShapeIterType imageIter(radius, itkImage, itkImage->GetLargestPossibleRegion());
  ShapeMaskIterType maskIter(radius, mask, mask->GetLargestPossibleRegion());

  auto region = mask->GetLargestPossibleRegion();

  auto center = imageIter.Size() / 2;
  auto iterSize = imageIter.Size();
  holder.m_NeighbourhoodSize = iterSize-1;
  while (!maskIter.IsAtEnd())
  {
    int sameValues = 0;
    bool completeNeighbourhood = true;

    int i = holder.IntensityToIndex(imageIter.GetCenterPixel());

    if (imageIter.GetCenterPixel() != imageIter.GetCenterPixel())
    {
      ++imageIter;
      ++maskIter;
      continue;
    }

    for (int position = 0; position < iterSize; ++position)
    {
      if ((position == center) ||
         ( ! region.IsInside(maskIter.GetIndex(position))))
      {
        completeNeighbourhood = false;
        continue;
      }
      bool isInBounds;
      auto jIntensity = imageIter.GetPixel(position, isInBounds);
      auto jMask = maskIter.GetPixel(position, isInBounds);
      if (jMask < 0 || (jIntensity != jIntensity) || ( ! isInBounds))
      {
        completeNeighbourhood = false;
        continue;
      }

      int j = holder.IntensityToIndex(jIntensity);
      holder.m_NumberOfNeighbourVoxels += 1;
      if (std::abs(i - j) <= alpha)
      {
        holder.m_NumberOfDependenceNeighbourVoxels += 1;
        ++sameValues;
      }
    }
    holder.m_Matrix(i, sameValues) += 1;
    holder.m_NumberOfNeighbourhoods += 1;
    if (completeNeighbourhood)
    {
      holder.m_NumberOfCompleteNeighbourhoods += 1;
    }

    ++imageIter;
    ++maskIter;
  }

}

void LocalCalculateFeatures(
  mitk::NGLDMMatrixHolder &holder,
  mitk::NGLDMMatrixFeatures & results
  )
{
  auto sijMatrix = holder.m_Matrix;
  auto piMatrix = holder.m_Matrix;
  auto pjMatrix = holder.m_Matrix;
  double Ng = holder.m_NumberOfBins;
  int NgSize = holder.m_NumberOfBins;
  double Ns = sijMatrix.sum();
  piMatrix.rowwise().normalize();
  pjMatrix.colwise().normalize();

  for (int i = 0; i < holder.m_NumberOfBins; ++i)
  {
    double sj = 0;
    for (int j = 0; j < holder.m_NumberOfDependences; ++j)
    {
      double iInt = holder.IndexToMeanIntensity(i);
      double sij = sijMatrix(i, j);
      double k = j+1;
      double pij = sij / Ns;

      results.LowDependenceEmphasis += sij / k / k;
      results.HighDependenceEmphasis += sij * k*k;
      if (iInt != 0)
      {
        results.LowGreyLevelCountEmphasis += sij / iInt / iInt;
      }
      results.HighGreyLevelCountEmphasis += sij * iInt*iInt;
      if (iInt != 0)
      {
        results.LowDependenceLowGreyLevelEmphasis += sij / k / k / iInt / iInt;
      }
      results.LowDependenceHighGreyLevelEmphasis += sij * iInt*iInt / k / k;
      if (iInt != 0)
      {
        results.HighDependenceLowGreyLevelEmphasis += sij *k * k / iInt / iInt;
      }
      results.HighDependenceHighGreyLevelEmphasis += sij * k*k*iInt*iInt;


      results.MeanGreyLevelCount += iInt * pij;
      results.MeanDependenceCount += k * pij;
      if (pij > 0)
      {
        results.DependenceCountEntropy -= pij * std::log(pij) / std::log(2);
      }
      sj += sij;
    }
    results.GreyLevelNonUniformity += sj*sj;
    results.GreyLevelNonUniformityNormalised += sj*sj;
  }

  for (int j = 0; j < holder.m_NumberOfDependences; ++j)
  {
    double si = 0;
    for (int i = 0; i < holder.m_NumberOfBins; ++i)
    {
      double sij = sijMatrix(i, j);
      si += sij;
    }
    results.DependenceCountNonUniformity += si*si;
    results.DependenceCountNonUniformityNormalised += si*si;
  }
  for (int i = 0; i < holder.m_NumberOfBins; ++i)
  {
    for (int j = 0; j < holder.m_NumberOfDependences; ++j)
    {
      double iInt = holder.IndexToMeanIntensity(i);
      double sij = sijMatrix(i, j);
      double k = j + 1;
      double pij = sij / Ns;

      results.GreyLevelVariance += (iInt - results.MeanGreyLevelCount)* (iInt - results.MeanGreyLevelCount) * pij;
      results.DependenceCountVariance += (k - results.MeanDependenceCount)* (k - results.MeanDependenceCount) * pij;
    }
  }
  results.LowDependenceEmphasis /= Ns;
  results.HighDependenceEmphasis /= Ns;
  results.LowGreyLevelCountEmphasis /= Ns;
  results.HighGreyLevelCountEmphasis /= Ns;
  results.LowDependenceLowGreyLevelEmphasis /= Ns;
  results.LowDependenceHighGreyLevelEmphasis /= Ns;
  results.HighDependenceLowGreyLevelEmphasis /= Ns;
  results.HighDependenceHighGreyLevelEmphasis /= Ns;

  results.GreyLevelNonUniformity /= Ns;
  results.GreyLevelNonUniformityNormalised /= (Ns*Ns);
  results.DependenceCountNonUniformity /= Ns;
  results.DependenceCountNonUniformityNormalised /= (Ns*Ns);

  results.DependenceCountPercentage = 1;

  results.ExpectedNeighbourhoodSize = holder.m_NeighbourhoodSize;
  results.AverageNeighbourhoodSize = holder.m_NumberOfNeighbourVoxels / (1.0 * holder.m_NumberOfNeighbourhoods);
  results.AverageIncompleteNeighbourhoodSize = (holder.m_NumberOfNeighbourVoxels - holder.m_NumberOfCompleteNeighbourhoods* holder.m_NeighbourhoodSize) / (1.0 * (holder.m_NumberOfNeighbourhoods - holder.m_NumberOfCompleteNeighbourhoods));
  results.PercentageOfCompleteNeighbourhoods = holder.m_NumberOfCompleteNeighbourhoods / (1.0 * holder.m_NumberOfNeighbourhoods);
  results.PercentageOfDependenceNeighbours = holder.m_NumberOfDependenceNeighbourVoxels / (1.0 * holder.m_NumberOfNeighbourVoxels);
}

template<typename TPixel, unsigned int VImageDimension>
void
CalculateCoocurenceFeatures(itk::Image<TPixel, VImageDimension>* itkImage, mitk::Image::Pointer mask, mitk::GIFNeighbouringGreyLevelDependenceFeature::FeatureListType & featureList, mitk::GIFNeighbouringGreyLevelDependenceFeature::GIFNeighbouringGreyLevelDependenceFeatureConfiguration config)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<unsigned char, VImageDimension> MaskType;
  typedef itk::MinimumMaximumImageCalculator<ImageType> MinMaxComputerType;
  typedef itk::Neighborhood<TPixel, VImageDimension > NeighborhoodType;
  typedef itk::Offset<VImageDimension> OffsetType;


  ///////////////////////////////////////////////////////////////////////////////////////////////


  typename MinMaxComputerType::Pointer minMaxComputer = MinMaxComputerType::New();
  minMaxComputer->SetImage(itkImage);
  minMaxComputer->Compute();

  double rangeMin = -0.5 + minMaxComputer->GetMinimum();
  double rangeMax = 0.5 + minMaxComputer->GetMaximum();

  if (config.UseMinimumIntensity)
    rangeMin = config.MinimumIntensity;
  if (config.UseMaximumIntensity)
    rangeMax = config.MaximumIntensity;

  // Define Range
  int numberOfBins = config.Bins;

  typename MaskType::Pointer maskImage = MaskType::New();
  mitk::CastToItkImage(mask, maskImage);

  std::vector<mitk::NGLDMMatrixFeatures> resultVector;
  mitk::NGLDMMatrixHolder holderOverall(rangeMin, rangeMax, numberOfBins,37);
  mitk::NGLDMMatrixFeatures overallFeature;
  CalculateNGLDMMatrix<TPixel, VImageDimension>(itkImage, maskImage, config.alpha, config.range, config.direction, holderOverall);
  LocalCalculateFeatures(holderOverall, overallFeature);

  std::ostringstream  ss;
  ss << config.range;
  std::string strRange = ss.str();

  MatrixFeaturesTo(overallFeature, "NGLDM (" + strRange + ") overall", featureList);
}


static
void MatrixFeaturesTo(mitk::NGLDMMatrixFeatures features,
                      std::string prefix,
                      mitk::GIFNeighbouringGreyLevelDependenceFeature::FeatureListType &featureList)
{
  featureList.push_back(std::make_pair(prefix + " Low Dependence Emphasis", features.LowDependenceEmphasis));
  featureList.push_back(std::make_pair(prefix + " High Dependence Emphasis", features.HighDependenceEmphasis));
  featureList.push_back(std::make_pair(prefix + " Low Grey Level Count Emphasis", features.LowGreyLevelCountEmphasis));
  featureList.push_back(std::make_pair(prefix + " High Grey Level Count Emphasis", features.HighGreyLevelCountEmphasis));
  featureList.push_back(std::make_pair(prefix + " Low Dependence Low Grey Level Emphasis", features.LowDependenceLowGreyLevelEmphasis));
  featureList.push_back(std::make_pair(prefix + " Low Dependence High Grey Level Emphasis", features.LowDependenceHighGreyLevelEmphasis));
  featureList.push_back(std::make_pair(prefix + " High Dependence Low Grey Level Emphasis", features.HighDependenceLowGreyLevelEmphasis));
  featureList.push_back(std::make_pair(prefix + " High Dependence High Grey Level Emphasis", features.HighDependenceHighGreyLevelEmphasis));

  featureList.push_back(std::make_pair(prefix + " Grey Level Non-Uniformity", features.GreyLevelNonUniformity));
  featureList.push_back(std::make_pair(prefix + " Grey Level Non-Uniformity Normalised", features.GreyLevelNonUniformityNormalised));
  featureList.push_back(std::make_pair(prefix + " Dependence Count Non-Uniformity", features.DependenceCountNonUniformity));
  featureList.push_back(std::make_pair(prefix + " Dependence Count Non-Uniformtiy Normalised", features.DependenceCountNonUniformityNormalised));

  featureList.push_back(std::make_pair(prefix + " Dependence Count Percentage", features.DependenceCountPercentage));
  featureList.push_back(std::make_pair(prefix + " Grey Level Mean", features.MeanGreyLevelCount));
  featureList.push_back(std::make_pair(prefix + " Grey Level Variance", features.GreyLevelVariance));
  featureList.push_back(std::make_pair(prefix + " Dependence Count Mean", features.MeanDependenceCount));
  featureList.push_back(std::make_pair(prefix + " Dependence Count Variance", features.DependenceCountVariance));
  featureList.push_back(std::make_pair(prefix + " Dependence Count Entropy", features.DependenceCountEntropy));

  featureList.push_back(std::make_pair(prefix + " Expected Neighbourhood Size", features.ExpectedNeighbourhoodSize));
  featureList.push_back(std::make_pair(prefix + " Average Neighbourhood Size", features.AverageNeighbourhoodSize));
  featureList.push_back(std::make_pair(prefix + " Average Incomplete Neighbourhood Size", features.AverageIncompleteNeighbourhoodSize));
  featureList.push_back(std::make_pair(prefix + " Percentage of complete Neighbourhoods", features.PercentageOfCompleteNeighbourhoods));
  featureList.push_back(std::make_pair(prefix + " Percentage of Dependence Neighbour Voxels", features.PercentageOfDependenceNeighbours));

}

mitk::GIFNeighbouringGreyLevelDependenceFeature::GIFNeighbouringGreyLevelDependenceFeature() :
m_Range(1.0), m_Bins(6)
{
  SetShortName("ngldm");
  SetLongName("ngldm");
}

mitk::GIFNeighbouringGreyLevelDependenceFeature::FeatureListType mitk::GIFNeighbouringGreyLevelDependenceFeature::CalculateFeatures(const Image::Pointer & image, const Image::Pointer &mask)
{
  FeatureListType featureList;

  GIFNeighbouringGreyLevelDependenceFeatureConfiguration config;
  config.direction = GetDirection();
  config.range = m_Range;
  config.alpha = 0;

  config.MinimumIntensity = GetMinimumIntensity();
  config.MaximumIntensity = GetMaximumIntensity();
  config.UseMinimumIntensity = GetUseMinimumIntensity();
  config.UseMaximumIntensity = GetUseMaximumIntensity();
  config.Bins = GetBins();

  AccessByItk_3(image, CalculateCoocurenceFeatures, mask, featureList,config);

  return featureList;
}

mitk::GIFNeighbouringGreyLevelDependenceFeature::FeatureNameListType mitk::GIFNeighbouringGreyLevelDependenceFeature::GetFeatureNames()
{
  FeatureNameListType featureList;
  featureList.push_back("co-occ. Energy Means");
  featureList.push_back("co-occ. Energy Std.");

  return featureList;
}



void mitk::GIFNeighbouringGreyLevelDependenceFeature::AddArguments(mitkCommandLineParser &parser)
{
  std::string name = GetOptionPrefix();

  parser.addArgument(GetLongName(), name, mitkCommandLineParser::String, "Calculate Neighbouring grey level dependence based features", "Calculate Neighbouring grey level dependence based features", us::Any());
  parser.addArgument(name + "::range", name + "::range", mitkCommandLineParser::String, "NGLD Range", "Define the range that is used (Semicolon-separated)", us::Any());
  parser.addArgument(name + "::bins", name + "::bins", mitkCommandLineParser::String, "NGLD Number of Bins", "Define the number of bins that is used ", us::Any());
}

void
mitk::GIFNeighbouringGreyLevelDependenceFeature::CalculateFeaturesUsingParameters(const Image::Pointer & feature, const Image::Pointer &mask, const Image::Pointer &maskNoNAN, FeatureListType &featureList)
{
  auto parsedArgs = GetParameter();
  std::string name = GetOptionPrefix();

  if (parsedArgs.count(GetLongName()))
  {
    std::vector<double> ranges;
    if (parsedArgs.count(name + "::range"))
    {
      ranges = SplitDouble(parsedArgs[name + "::range"].ToString(), ';');
    }
    else
    {
      ranges.push_back(1);
    }
    if (parsedArgs.count(name + "::bins"))
    {
      auto bins = SplitDouble(parsedArgs[name + "::bins"].ToString(), ';')[0];
      this->SetBins(bins);
    }

    for (std::size_t i = 0; i < ranges.size(); ++i)
    {
      MITK_INFO << "Start calculating coocurence with range " << ranges[i] << "....";
      this->SetRange(ranges[i]);
      auto localResults = this->CalculateFeatures(feature, maskNoNAN);
      featureList.insert(featureList.end(), localResults.begin(), localResults.end());
      MITK_INFO << "Finished calculating coocurence with range " << ranges[i] << "....";
    }
  }
}

