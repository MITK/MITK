#include <mitkGIFGreyLevelDistanceZone.h>

// MITK
#include <mitkITKImageImport.h>
#include <mitkImageCast.h>
#include <mitkImageAccessByItk.h>
#include <mitkIOUtil.h>
#include <mitkITKImageImport.h>

// ITK
#include <itkMinimumMaximumImageCalculator.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkBinaryCrossStructuringElement.h>
#include <itkBinaryErodeImageFilter.h>
#include <itkAddImageFilter.h>

static
void MatrixFeaturesTo(mitk::GreyLevelDistanceZoneFeatures features,
                      std::string prefix,
                      mitk::GIFGreyLevelDistanceZone::FeatureListType &featureList);



mitk::GreyLevelDistanceZoneMatrixHolder::GreyLevelDistanceZoneMatrixHolder(double min, double max, int number, int maxSize) :
                    m_MinimumRange(min),
                    m_MaximumRange(max),
                    m_NumberOfBins(number),
                    m_MaximumSize(maxSize),
                    m_NumerOfVoxels(0)
{
  m_Matrix.resize(number, maxSize);
  m_Matrix.fill(0);
  m_Stepsize = (max - min) / (number);
}

int mitk::GreyLevelDistanceZoneMatrixHolder::IntensityToIndex(double intensity)
{
  return std::floor((intensity - m_MinimumRange) / m_Stepsize);
}

double mitk::GreyLevelDistanceZoneMatrixHolder::IndexToMinIntensity(int index)
{
  return m_MinimumRange + index * m_Stepsize;
}
double mitk::GreyLevelDistanceZoneMatrixHolder::IndexToMeanIntensity(int index)
{
  return m_MinimumRange + (index+0.5) * m_Stepsize;
}
double mitk::GreyLevelDistanceZoneMatrixHolder::IndexToMaxIntensity(int index)
{
  return m_MinimumRange + (index + 1) * m_Stepsize;
}

template<typename TPixel, unsigned int VImageDimension>
int
CalculateGlSZMatrix(itk::Image<TPixel, VImageDimension>* itkImage,
                    itk::Image<unsigned short, VImageDimension>* mask,
                    itk::Image<unsigned short, VImageDimension>* distanceImage,
                    std::vector<itk::Offset<VImageDimension> > offsets,
                    bool estimateLargestRegion,
                    mitk::GreyLevelDistanceZoneMatrixHolder &holder)
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
  holder.m_NumberOfBins = 0;

  while (!maskIter.IsAtEnd())
  {
    if (maskIter.Value() > 0 )
    {
      auto startIntensityIndex = holder.IntensityToIndex(imageIter.Value());
      std::vector<IndexType> indices;
      indices.push_back(maskIter.GetIndex());
      unsigned int steps = 0;
      int smallestDistance = 500;

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
          ++(holder.m_NumerOfVoxels);
          smallestDistance = (smallestDistance > distanceImage->GetPixel(currentIndex)) ? distanceImage->GetPixel(currentIndex) : smallestDistance;
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
          holder.m_Matrix(startIntensityIndex, smallestDistance-1) += 1;
        }
      }
    }
    ++imageIter;
    ++maskIter;
  }
  return largestRegion;
}

template <typename TPixel, unsigned int VDimension>
void itkErode(
  itk::Image<TPixel, VDimension> *sourceImage,
  mitk::Image::Pointer &resultImage,
  bool &segmentationRemaining)
{
  typedef itk::Image<TPixel, VDimension> ImageType;
  typedef itk::BinaryCrossStructuringElement<TPixel, VDimension> CrossType;
  typedef typename itk::BinaryErodeImageFilter<ImageType, ImageType, CrossType> CrossErodeFilterType;

  CrossType cross;
  typename CrossType::SizeType size;
  size.Fill(1);
  cross.SetRadius(size);
  cross.CreateStructuringElement();

  typename CrossErodeFilterType::Pointer erodeFilter = CrossErodeFilterType::New();
  erodeFilter->SetKernel(cross);
  erodeFilter->SetInput(sourceImage);
  erodeFilter->SetErodeValue(1);
  erodeFilter->UpdateLargestPossibleRegion();

  segmentationRemaining = false;
  itk::ImageRegionConstIterator<ImageType> iter(erodeFilter->GetOutput(), erodeFilter->GetOutput()->GetLargestPossibleRegion());
  while ((!iter.IsAtEnd()) && (!segmentationRemaining))
  {
    if (iter.Get() > 0)
      segmentationRemaining = true;
    ++iter;
  }

  mitk::CastToMitkImage(erodeFilter->GetOutput(), resultImage);
}

template <typename TPixel, unsigned int VDimension>
void itkAdd(
  itk::Image<TPixel, VDimension> *imageA, mitk::Image::Pointer mitkImageB,
  mitk::Image::Pointer &resultImage)
{
  typedef itk::Image<TPixel, VDimension> ImageType;
  typedef itk::AddImageFilter<ImageType, ImageType, ImageType> AddFilterType;

  typename ImageType::Pointer imageB;
  mitk::CastToItkImage(mitkImageB, imageB);

  typename AddFilterType::Pointer addFilter = AddFilterType::New();
  addFilter->SetInput1(imageA);
  addFilter->SetInput2(imageB);
  addFilter->UpdateLargestPossibleRegion();

  mitk::CastToMitkImage(addFilter->GetOutput(), resultImage);
}

void erode(mitk::Image::Pointer input, mitk::Image::Pointer &output, bool &segmentationRemaining)
{
  AccessByItk_2(input, itkErode, output, segmentationRemaining);
}
void add(mitk::Image::Pointer inputA, mitk::Image::Pointer inputB, mitk::Image::Pointer &output)
{
  AccessByItk_2(inputA, itkAdd, inputB, output);
}

void erodeAndAdd(mitk::Image::Pointer input, mitk::Image::Pointer& finalOutput, int &maxDistance)
{
  mitk::Image::Pointer workingImage = input;
  mitk::Image::Pointer output = input;
  maxDistance = 0;
  bool segmentationRemaining = true;
  while (segmentationRemaining)
  {
    mitk::Image::Pointer erodedImage = mitk::Image::New();
    mitk::Image::Pointer result = mitk::Image::New();
    erode(workingImage, erodedImage, segmentationRemaining);
    workingImage = erodedImage;
    add(output, erodedImage, result);
    output = result;
    ++maxDistance;
  }
  finalOutput = output;
}


void static CalculateFeatures(
  mitk::GreyLevelDistanceZoneMatrixHolder &holder,
  mitk::GreyLevelDistanceZoneFeatures & results
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
    results.SmallDistanceEmphasis += SzVector(j) / (j+1) / (j+1);
    results.LargeDistanceEmphasis += SzVector(j) * (j + 1.0) * (j + 1.0);
    results.ZoneDistanceNonUniformity += SzVector(j) * SzVector(j);
    results.ZoneDistanceNoneUniformityNormalized += SzVector(j) * SzVector(j);
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
      results.SmallDistanceLowGreyLevelEmphasis += SgzMatrix(i, j) / (i + 1) / (i + 1) / (j + 1) / (j + 1);
      results.SmallDistanceHighGreyLevelEmphasis += SgzMatrix(i, j) * (i + 1) * (i + 1) / (j + 1) / (j + 1);
      results.LargeDistanceLowGreyLevelEmphasis += SgzMatrix(i, j) / (i + 1) / (i + 1) * (j + 1.0) * (j + 1.0);
      results.LargeDistanceHighGreyLevelEmphasis += SgzMatrix(i, j) * (i + 1) * (i + 1) * (j + 1.0) * (j + 1.0);
      results.ZonePercentage += SgzMatrix(i, j);

      results.GreyLevelMean += (i + 1)*pgzMatrix(i, j);
      results.ZoneDistanceMean += (j + 1)*pgzMatrix(i, j);
      if (pgzMatrix(i, j) > 0)
        results.ZoneDistanceEntropy -= pgzMatrix(i, j) * std::log(pgzMatrix(i, j)) / std::log(2);
    }
  }

  for (int i = 0; i < SgzMatrix.rows(); ++i)
  {
    for (int j = 0; j < SgzMatrix.cols(); ++j)
    {
      results.GreyLevelVariance += (i + 1 - results.GreyLevelMean)*(i + 1 - results.GreyLevelMean)*pgzMatrix(i, j);
      results.ZoneDistanceVariance += (j + 1 - results.ZoneDistanceMean)*(j + 1 - results.ZoneDistanceMean)*pgzMatrix(i, j);
    }
  }

  results.SmallDistanceEmphasis /= Ns;
  results.LargeDistanceEmphasis /= Ns;
  results.LowGreyLevelEmphasis /= Ns;
  results.HighGreyLevelEmphasis /= Ns;

  results.SmallDistanceLowGreyLevelEmphasis /= Ns;
  results.SmallDistanceHighGreyLevelEmphasis /= Ns;
  results.LargeDistanceLowGreyLevelEmphasis /= Ns;
  results.LargeDistanceHighGreyLevelEmphasis /= Ns;
  results.GreyLevelNonUniformity /= Ns;
  results.GreyLevelNonUniformityNormalized /= Ns*Ns;
  results.ZoneDistanceNonUniformity /= Ns;
  results.ZoneDistanceNoneUniformityNormalized /= Ns*Ns;

  results.ZonePercentage = Ns / holder.m_NumerOfVoxels;// results.ZonePercentage;
}

template<typename TPixel, unsigned int VImageDimension>
static void
CalculateGreyLevelDistanceZoneFeatures(itk::Image<TPixel, VImageDimension>* itkImage, mitk::Image::Pointer mask, mitk::GIFGreyLevelDistanceZone::FeatureListType & featureList, mitk::GIFGreyLevelDistanceZone::GIFGreyLevelDistanceZoneConfiguration config)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<unsigned short, VImageDimension> MaskType;
  typedef itk::MinimumMaximumImageCalculator<ImageType> MinMaxComputerType;
  typedef itk::Neighborhood<TPixel, VImageDimension > NeighborhoodType;
  typedef itk::Offset<VImageDimension> OffsetType;

  ///////////////////////////////////////////////////////////////////////////////////////////////
  double rangeMin = config.MinimumIntensity;
  double rangeMax = config.MaximumIntensity;
  int numberOfBins = config.Bins;

  int maximumDistance = 0;
  mitk::Image::Pointer mitkDistanceImage = mitk::Image::New();
  erodeAndAdd(config.distanceMask, mitkDistanceImage, maximumDistance);
  typename MaskType::Pointer distanceImage = MaskType::New();
  mitk::CastToItkImage(mitkDistanceImage, distanceImage);

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

  MITK_INFO << "Maximum Distance: " << maximumDistance;
  std::vector<mitk::GreyLevelDistanceZoneFeatures> resultVector;
  mitk::GreyLevelDistanceZoneMatrixHolder holderOverall(rangeMin, rangeMax, numberOfBins, maximumDistance+1);
  mitk::GreyLevelDistanceZoneFeatures overallFeature;
  CalculateGlSZMatrix<TPixel, VImageDimension>(itkImage, maskImage, distanceImage, offsetVector, false, holderOverall);
  CalculateFeatures(holderOverall, overallFeature);

  MatrixFeaturesTo(overallFeature, "Grey Level Distance Zone::", featureList);
}


static
void MatrixFeaturesTo(mitk::GreyLevelDistanceZoneFeatures features,
                      std::string prefix,
                      mitk::GIFGreyLevelDistanceZone::FeatureListType &featureList)
{
  featureList.push_back(std::make_pair(prefix + "Small Distance Emphasis", features.SmallDistanceEmphasis));
  featureList.push_back(std::make_pair(prefix + "Large Distance Emphasis", features.LargeDistanceEmphasis));
  featureList.push_back(std::make_pair(prefix + "Low Grey Level Emphasis", features.LowGreyLevelEmphasis));
  featureList.push_back(std::make_pair(prefix + "High Grey Level Emphasis", features.HighGreyLevelEmphasis));
  featureList.push_back(std::make_pair(prefix + "Small Distance Low Grey Level Emphasis", features.SmallDistanceLowGreyLevelEmphasis));
  featureList.push_back(std::make_pair(prefix + "Small Distance High Grey Level Emphasis", features.SmallDistanceHighGreyLevelEmphasis));
  featureList.push_back(std::make_pair(prefix + "Large Distance Low Grey Level Emphasis", features.LargeDistanceLowGreyLevelEmphasis));
  featureList.push_back(std::make_pair(prefix + "Large Distance High Grey Level Emphasis", features.LargeDistanceHighGreyLevelEmphasis));
  featureList.push_back(std::make_pair(prefix + "Grey Level Non-Uniformity", features.GreyLevelNonUniformity));
  featureList.push_back(std::make_pair(prefix + "Grey Level Non-Uniformity Normalized", features.GreyLevelNonUniformityNormalized));
  featureList.push_back(std::make_pair(prefix + "Distance Size Non-Uniformity", features.ZoneDistanceNonUniformity));
  featureList.push_back(std::make_pair(prefix + "Distance Size Non-Uniformity Normalized", features.ZoneDistanceNoneUniformityNormalized));
  featureList.push_back(std::make_pair(prefix + "Zone Percentage", features.ZonePercentage));
  featureList.push_back(std::make_pair(prefix + "Grey Level Mean", features.GreyLevelMean));
  featureList.push_back(std::make_pair(prefix + "Grey Level Variance", features.GreyLevelVariance));
  featureList.push_back(std::make_pair(prefix + "Zone Distance Mean", features.ZoneDistanceMean));
  featureList.push_back(std::make_pair(prefix + "Zone Distance Variance", features.ZoneDistanceVariance));
  featureList.push_back(std::make_pair(prefix + "Zone Distance Entropy", features.ZoneDistanceEntropy));
  featureList.push_back(std::make_pair(prefix + "Grey Level Entropy", features.ZoneDistanceEntropy));
}

  mitk::GIFGreyLevelDistanceZone::GIFGreyLevelDistanceZone()
{
  SetShortName("gldz");
  SetLongName("distance-zone");
}

mitk::GIFGreyLevelDistanceZone::FeatureListType mitk::GIFGreyLevelDistanceZone::CalculateFeatures(const Image::Pointer & image, const Image::Pointer &mask)
{
  InitializeQuantifier(image, mask);
  FeatureListType featureList;

  GIFGreyLevelDistanceZoneConfiguration config;
  config.direction = GetDirection();

  if (GetMorphMask().IsNull())
  {
    config.distanceMask = mask->Clone();
  }
  else
  {
    config.distanceMask = GetMorphMask();
  }

  config.MinimumIntensity = GetQuantifier()->GetMinimum();
  config.MaximumIntensity = GetQuantifier()->GetMaximum();
  config.Bins = GetQuantifier()->GetBins();

  AccessByItk_3(image, CalculateGreyLevelDistanceZoneFeatures, mask, featureList, config);

  return featureList;
}

mitk::GIFGreyLevelDistanceZone::FeatureNameListType mitk::GIFGreyLevelDistanceZone::GetFeatureNames()
{
  FeatureNameListType featureList;
  return featureList;
}




void mitk::GIFGreyLevelDistanceZone::AddArguments(mitkCommandLineParser &parser)
{
  AddQuantifierArguments(parser);
  std::string name = GetOptionPrefix();

  parser.addArgument(GetLongName(), name, mitkCommandLineParser::Bool, "Use Grey Level Distance Zone", "Calculates the size zone based features.", us::Any());
}

void
mitk::GIFGreyLevelDistanceZone::CalculateFeaturesUsingParameters(const Image::Pointer & feature, const Image::Pointer &mask, const Image::Pointer &maskNoNAN, FeatureListType &featureList)
{
  auto parsedArgs = GetParameter();
  std::string name = GetOptionPrefix();

  if (parsedArgs.count(GetLongName()))
  {
    InitializeQuantifierFromParameters(feature, mask);

    MITK_INFO << "Start calculating Grey Level Distance Zone ....";
    auto localResults = this->CalculateFeatures(feature, maskNoNAN);
    featureList.insert(featureList.end(), localResults.begin(), localResults.end());
    MITK_INFO << "Finished calculating Grey Level Distance Zone.";
  }

}

