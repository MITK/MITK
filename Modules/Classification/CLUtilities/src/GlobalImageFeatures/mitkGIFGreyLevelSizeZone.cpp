#include <mitkGIFGreyLevelSizeZone.h>

// MITK
#include <mitkITKImageImport.h>
#include <mitkImageCast.h>
#include <mitkImageAccessByItk.h>

// ITK
#include <itkImageRegionIteratorWithIndex.h>

// STL

namespace mitk
{
  struct GreyLevelSizeZoneMatrixHolder
  {
  public:
    GreyLevelSizeZoneMatrixHolder(double min, double max, int number, int maxSize);

    int IntensityToIndex(double intensity);
    double IndexToMinIntensity(int index);
    double IndexToMeanIntensity(int index);
    double IndexToMaxIntensity(int index);

    double m_MinimumRange;
    double m_MaximumRange;
    double m_Stepsize;
    int m_NumberOfBins;
    int m_MaximumSize;
    Eigen::MatrixXd m_Matrix;

  };

  struct GreyLevelSizeZoneFeatures
  {
    GreyLevelSizeZoneFeatures() :
      SmallZoneEmphasis(0),
      LargeZoneEmphasis(0),
      LowGreyLevelEmphasis(0),
      HighGreyLevelEmphasis(0),
      SmallZoneLowGreyLevelEmphasis(0),
      SmallZoneHighGreyLevelEmphasis(0),
      LargeZoneLowGreyLevelEmphasis(0),
      LargeZoneHighGreyLevelEmphasis(0),
      GreyLevelNonUniformity(0),
      GreyLevelNonUniformityNormalized(0),
      ZoneSizeNonUniformity(0),
      ZoneSizeNoneUniformityNormalized(0),
      ZonePercentage(0),
      GreyLevelMean(0),
      GreyLevelVariance(0),
      ZoneSizeMean(0),
      ZoneSizeVariance(0),
      ZoneSizeEntropy(0)
    {
    }

  public:
    double SmallZoneEmphasis;
    double LargeZoneEmphasis;
    double LowGreyLevelEmphasis;
    double HighGreyLevelEmphasis;
    double SmallZoneLowGreyLevelEmphasis;
    double SmallZoneHighGreyLevelEmphasis;
    double LargeZoneLowGreyLevelEmphasis;
    double LargeZoneHighGreyLevelEmphasis;
    double GreyLevelNonUniformity;
    double GreyLevelNonUniformityNormalized;
    double ZoneSizeNonUniformity;
    double ZoneSizeNoneUniformityNormalized;
    double ZonePercentage;
    double GreyLevelMean;
    double GreyLevelVariance;
    double ZoneSizeMean;
    double ZoneSizeVariance;
    double ZoneSizeEntropy;
  };
}

static
void MatrixFeaturesTo(mitk::GreyLevelSizeZoneFeatures features,
                      std::string prefix,
                      mitk::GIFGreyLevelSizeZone::FeatureListType &featureList);



mitk::GreyLevelSizeZoneMatrixHolder::GreyLevelSizeZoneMatrixHolder(double min, double max, int number, int maxSize) :
                    m_MinimumRange(min),
                    m_MaximumRange(max),
                    m_NumberOfBins(number),
                    m_MaximumSize(maxSize)
{
  m_Matrix.resize(number, maxSize);
  m_Matrix.fill(0);
  m_Stepsize = (max - min) / (number);
}

int mitk::GreyLevelSizeZoneMatrixHolder::IntensityToIndex(double intensity)
{
  return std::floor((intensity - m_MinimumRange) / m_Stepsize);
}

double mitk::GreyLevelSizeZoneMatrixHolder::IndexToMinIntensity(int index)
{
  return m_MinimumRange + index * m_Stepsize;
}
double mitk::GreyLevelSizeZoneMatrixHolder::IndexToMeanIntensity(int index)
{
  return m_MinimumRange + (index+0.5) * m_Stepsize;
}
double mitk::GreyLevelSizeZoneMatrixHolder::IndexToMaxIntensity(int index)
{
  return m_MinimumRange + (index + 1) * m_Stepsize;
}

template<typename TPixel, unsigned int VImageDimension>
static int
CalculateGlSZMatrix(itk::Image<TPixel, VImageDimension>* itkImage,
                    itk::Image<unsigned short, VImageDimension>* mask,
                    std::vector<itk::Offset<VImageDimension> > offsets,
                    bool estimateLargestRegion,
                    mitk::GreyLevelSizeZoneMatrixHolder &holder)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<unsigned short, VImageDimension> MaskImageType;
  typedef typename ImageType::IndexType IndexType;

  typedef itk::ImageRegionIteratorWithIndex<ImageType> ConstIterType;
  typedef itk::ImageRegionIteratorWithIndex<MaskImageType> ConstMaskIterType;

  auto region = mask->GetLargestPossibleRegion();
  typename MaskImageType::RegionType newRegion;
  newRegion.SetSize(region.GetSize());
  newRegion.SetIndex(region.GetIndex());

  ConstIterType imageIter(itkImage, itkImage->GetLargestPossibleRegion());
  ConstMaskIterType maskIter(mask, mask->GetLargestPossibleRegion());

  typename MaskImageType::Pointer visitedImage = MaskImageType::New();
  visitedImage->SetRegions(newRegion);
  visitedImage->Allocate();
  visitedImage->FillBuffer(0);

  int largestRegion = 0;

  while (!maskIter.IsAtEnd())
  {
    if (maskIter.Value() > 0 )
    {
      auto startIntensityIndex = holder.IntensityToIndex(imageIter.Value());
      std::vector<IndexType> indices;
      indices.push_back(maskIter.GetIndex());
      unsigned int steps = 0;

      while (indices.size() > 0)
      {
        auto currentIndex = indices.back();
        indices.pop_back();

        if (!region.IsInside(currentIndex))
        {
          continue;
        }

        auto wasVisited = visitedImage->GetPixel(currentIndex);
        auto newIntensityIndex = holder.IntensityToIndex(itkImage->GetPixel(currentIndex));
        auto isInMask = mask->GetPixel(currentIndex);

        if ((isInMask > 0) &&
            (newIntensityIndex == startIntensityIndex) &&
            (wasVisited < 1))
        {
          ++steps;
          visitedImage->SetPixel(currentIndex, 1);
          for (auto offset : offsets)
          {
            auto newIndex = currentIndex + offset;
            indices.push_back(newIndex);
            newIndex = currentIndex - offset;
            indices.push_back(newIndex);
          }

        }
      }
      if (steps > 0)
      {
        largestRegion = std::max<int>(steps, largestRegion);
        steps = std::min<unsigned int>(steps, holder.m_MaximumSize);
        if (!estimateLargestRegion)
        {
          holder.m_Matrix(startIntensityIndex, steps - 1) += 1;
        }
      }
    }
    ++imageIter;
    ++maskIter;
  }
  return largestRegion;
}

static void CalculateFeatures(
  mitk::GreyLevelSizeZoneMatrixHolder &holder,
  mitk::GreyLevelSizeZoneFeatures & results
  )
{
  auto SgzMatrix = holder.m_Matrix;
  auto pgzMatrix = holder.m_Matrix;
  auto pgMatrix = holder.m_Matrix;
  auto pzMatrix = holder.m_Matrix;

  double Ns = pgzMatrix.sum();
  pgzMatrix /= Ns;
  pgMatrix.rowwise().normalize();
  pzMatrix.colwise().normalize();

  for (int i = 0; i < holder.m_NumberOfBins; ++i)
    for (int j = 0; j < holder.m_NumberOfBins; ++j)
    {
      if (pgzMatrix(i, j) != pgzMatrix(i, j))
        pgzMatrix(i, j) = 0;
      if (pgMatrix(i, j) != pgMatrix(i, j))
        pgMatrix(i, j) = 0;
      if (pzMatrix(i, j) != pzMatrix(i, j))
        pzMatrix(i, j) = 0;
    }

  Eigen::VectorXd SgVector = SgzMatrix.rowwise().sum();
  Eigen::VectorXd SzVector = SgzMatrix.colwise().sum();

  for (int j = 0; j < SzVector.size(); ++j)
  {
    results.SmallZoneEmphasis += SzVector(j) / (j + 1) / (j + 1);
    results.LargeZoneEmphasis += SzVector(j) * (j + 1.0) * (j + 1.0);
    results.ZoneSizeNonUniformity += SzVector(j) * SzVector(j);
    results.ZoneSizeNoneUniformityNormalized += SzVector(j) * SzVector(j);
  }
  for (int i = 0; i < SgVector.size(); ++i)
  {
    results.LowGreyLevelEmphasis += SgVector(i) / (i + 1) / (i + 1);
    results.HighGreyLevelEmphasis += SgVector(i) * (i + 1) * (i + 1);
    results.GreyLevelNonUniformity += SgVector(i)*SgVector(i);
    results.GreyLevelNonUniformityNormalized += SgVector(i)*SgVector(i);
  }

  for (int i = 0; i < SgzMatrix.rows(); ++i)
  {
    for (int j = 0; j < SgzMatrix.cols(); ++j)
    {
      results.SmallZoneLowGreyLevelEmphasis += SgzMatrix(i, j) / (i + 1) / (i + 1) / (j + 1) / (j + 1);
      results.SmallZoneHighGreyLevelEmphasis += SgzMatrix(i, j) * (i + 1) * (i + 1) / (j + 1) / (j + 1);
      results.LargeZoneLowGreyLevelEmphasis += SgzMatrix(i, j) / (i + 1) / (i + 1) * (j + 1.0) * (j + 1.0);
      results.LargeZoneHighGreyLevelEmphasis += SgzMatrix(i, j) * (i + 1) * (i + 1) * (j + 1.0) * (j + 1.0);
      results.ZonePercentage += SgzMatrix(i, j)*(j + 1);

      results.GreyLevelMean += (i + 1)*pgzMatrix(i, j);
      results.ZoneSizeMean += (j + 1)*pgzMatrix(i, j);
      if (pgzMatrix(i, j) > 0)
        results.ZoneSizeEntropy -= pgzMatrix(i, j) * std::log(pgzMatrix(i, j)) / std::log(2);
    }
  }

  for (int i = 0; i < SgzMatrix.rows(); ++i)
  {
    for (int j = 0; j < SgzMatrix.cols(); ++j)
    {
      results.GreyLevelVariance += (i + 1 - results.GreyLevelMean)*(i + 1 - results.GreyLevelMean)*pgzMatrix(i, j);
      results.ZoneSizeVariance += (j + 1 - results.ZoneSizeMean)*(j + 1 - results.ZoneSizeMean)*pgzMatrix(i, j);
    }
  }

  results.SmallZoneEmphasis /= Ns;
  results.LargeZoneEmphasis /= Ns;
  results.LowGreyLevelEmphasis /= Ns;
  results.HighGreyLevelEmphasis /= Ns;

  results.SmallZoneLowGreyLevelEmphasis /= Ns;
  results.SmallZoneHighGreyLevelEmphasis /= Ns;
  results.LargeZoneLowGreyLevelEmphasis /= Ns;
  results.LargeZoneHighGreyLevelEmphasis /= Ns;
  results.GreyLevelNonUniformity /= Ns;
  results.GreyLevelNonUniformityNormalized /= Ns*Ns;
  results.ZoneSizeNonUniformity /= Ns;
  results.ZoneSizeNoneUniformityNormalized /= Ns*Ns;

  results.ZonePercentage = Ns / results.ZonePercentage;
}

template<typename TPixel, unsigned int VImageDimension>
static void
CalculateGreyLevelSizeZoneFeatures(itk::Image<TPixel, VImageDimension>* itkImage, mitk::Image::Pointer mask, mitk::GIFGreyLevelSizeZone::FeatureListType & featureList, mitk::GIFGreyLevelSizeZone::GIFGreyLevelSizeZoneConfiguration config)
{
  typedef itk::Image<unsigned short, VImageDimension> MaskType;
  typedef itk::Neighborhood<TPixel, VImageDimension > NeighborhoodType;
  typedef itk::Offset<VImageDimension> OffsetType;

  ///////////////////////////////////////////////////////////////////////////////////////////////
  double rangeMin = config.MinimumIntensity;
  double rangeMax = config.MaximumIntensity;
  int numberOfBins = config.Bins;

  typename MaskType::Pointer maskImage = MaskType::New();
  mitk::CastToItkImage(mask, maskImage);

  //Find possible directions
  std::vector < itk::Offset<VImageDimension> > offsetVector;
  NeighborhoodType hood;
  hood.SetRadius(1);
  unsigned int        centerIndex = hood.GetCenterNeighborhoodIndex();
  OffsetType          offset;
  for (unsigned int d = 0; d < centerIndex; d++)
  {
    offset = hood.GetOffset(d);
    bool useOffset = true;
    for (unsigned int i = 0; i < VImageDimension; ++i)
    {
      if ((config.direction == i + 2) && offset[i] != 0)
      {
        useOffset = false;
      }
    }
    if (useOffset)
    {
      offsetVector.push_back(offset);
      MITK_INFO << offset;
    }
  }
  if (config.direction == 1)
  {
    offsetVector.clear();
    offset[0] = 0;
    offset[1] = 0;
    offset[2] = 1;
    offsetVector.push_back(offset);
  }

  std::vector<mitk::GreyLevelSizeZoneFeatures> resultVector;
  mitk::GreyLevelSizeZoneMatrixHolder tmpHolder(rangeMin, rangeMax, numberOfBins, 3);
  int largestRegion = CalculateGlSZMatrix<TPixel, VImageDimension>(itkImage, maskImage, offsetVector, true, tmpHolder);
  mitk::GreyLevelSizeZoneMatrixHolder holderOverall(rangeMin, rangeMax, numberOfBins,largestRegion);
  mitk::GreyLevelSizeZoneFeatures overallFeature;
  CalculateGlSZMatrix<TPixel, VImageDimension>(itkImage, maskImage, offsetVector, false, holderOverall);
  CalculateFeatures(holderOverall, overallFeature);

  MatrixFeaturesTo(overallFeature, config.prefix, featureList);
}


static
void MatrixFeaturesTo(mitk::GreyLevelSizeZoneFeatures features,
                      std::string prefix,
                      mitk::GIFGreyLevelSizeZone::FeatureListType &featureList)
{
  featureList.push_back(std::make_pair(prefix + "Small Zone Emphasis", features.SmallZoneEmphasis));
  featureList.push_back(std::make_pair(prefix + "Large Zone Emphasis", features.LargeZoneEmphasis));
  featureList.push_back(std::make_pair(prefix + "Low Grey Level Emphasis", features.LowGreyLevelEmphasis));
  featureList.push_back(std::make_pair(prefix + "High Grey Level Emphasis", features.HighGreyLevelEmphasis));
  featureList.push_back(std::make_pair(prefix + "Small Zone Low Grey Level Emphasis", features.SmallZoneLowGreyLevelEmphasis));
  featureList.push_back(std::make_pair(prefix + "Small Zone High Grey Level Emphasis", features.SmallZoneHighGreyLevelEmphasis));
  featureList.push_back(std::make_pair(prefix + "Large Zone Low Grey Level Emphasis", features.LargeZoneLowGreyLevelEmphasis));
  featureList.push_back(std::make_pair(prefix + "Large Zone High Grey Level Emphasis", features.LargeZoneHighGreyLevelEmphasis));
  featureList.push_back(std::make_pair(prefix + "Grey Level Non-Uniformity", features.GreyLevelNonUniformity));
  featureList.push_back(std::make_pair(prefix + "Grey Level Non-Uniformity Normalized", features.GreyLevelNonUniformityNormalized));
  featureList.push_back(std::make_pair(prefix + "Zone Size Non-Uniformity", features.ZoneSizeNonUniformity));
  featureList.push_back(std::make_pair(prefix + "Zone Size Non-Uniformity Normalized", features.ZoneSizeNoneUniformityNormalized));
  featureList.push_back(std::make_pair(prefix + "Zone Percentage", features.ZonePercentage));
  featureList.push_back(std::make_pair(prefix + "Grey Level Mean", features.GreyLevelMean));
  featureList.push_back(std::make_pair(prefix + "Grey Level Variance", features.GreyLevelVariance));
  featureList.push_back(std::make_pair(prefix + "Zone Size Mean", features.ZoneSizeMean));
  featureList.push_back(std::make_pair(prefix + "Zone Size Variance", features.ZoneSizeVariance));
  featureList.push_back(std::make_pair(prefix + "Zone Size Entropy", features.ZoneSizeEntropy));
}

  mitk::GIFGreyLevelSizeZone::GIFGreyLevelSizeZone()
{
  SetShortName("glsz");
  SetLongName("grey-level-sizezone");
  SetFeatureClassName("Grey Level Size Zone");
}

mitk::GIFGreyLevelSizeZone::FeatureListType mitk::GIFGreyLevelSizeZone::CalculateFeatures(const Image::Pointer & image, const Image::Pointer &mask)
{
  InitializeQuantifier(image, mask);

  FeatureListType featureList;

  GIFGreyLevelSizeZoneConfiguration config;
  config.direction = GetDirection();

  config.MinimumIntensity = GetQuantifier()->GetMinimum();
  config.MaximumIntensity = GetQuantifier()->GetMaximum();
  config.Bins = GetQuantifier()->GetBins();
  config.prefix = FeatureDescriptionPrefix();

  AccessByItk_3(image, CalculateGreyLevelSizeZoneFeatures, mask, featureList, config);

  return featureList;
}

mitk::GIFGreyLevelSizeZone::FeatureNameListType mitk::GIFGreyLevelSizeZone::GetFeatureNames()
{
  FeatureNameListType featureList;
  return featureList;
}




void mitk::GIFGreyLevelSizeZone::AddArguments(mitkCommandLineParser &parser)
{
  std::string name = GetOptionPrefix();

  parser.addArgument(GetLongName(), name, mitkCommandLineParser::Bool, "Use Grey Level Size Zone", "Calculates the size zone based features.", us::Any());
  AddQuantifierArguments(parser);
}

void
mitk::GIFGreyLevelSizeZone::CalculateFeaturesUsingParameters(const Image::Pointer & feature, const Image::Pointer &, const Image::Pointer &maskNoNAN, FeatureListType &featureList)
{
  auto parsedArgs = GetParameter();
  std::string name = GetOptionPrefix();

  if (parsedArgs.count(GetLongName()))
  {
    InitializeQuantifierFromParameters(feature, maskNoNAN);

    MITK_INFO << "Start calculating  Grey leve size zone ...";
    auto localResults = this->CalculateFeatures(feature, maskNoNAN);
    featureList.insert(featureList.end(), localResults.begin(), localResults.end());
    MITK_INFO << "Finished calculating Grey level size zone ...";
  }

}

std::string mitk::GIFGreyLevelSizeZone::GetCurrentFeatureEncoding()
{
  return QuantifierParameterString();
}

