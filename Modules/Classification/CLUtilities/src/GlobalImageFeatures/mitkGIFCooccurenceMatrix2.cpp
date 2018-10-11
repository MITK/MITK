#include <mitkGIFCooccurenceMatrix2.h>

// MITK
#include <mitkITKImageImport.h>
#include <mitkImageCast.h>
#include <mitkImageAccessByItk.h>

// ITK
#include <itkEnhancedScalarImageToTextureFeaturesFilter.h>
#include <itkShapedNeighborhoodIterator.h>
#include <itkImageRegionConstIterator.h>

// STL
#include <sstream>
#include <cmath>

namespace mitk
{
  struct CoocurenceMatrixHolder
  {
  public:
    CoocurenceMatrixHolder(double min, double max, int number);

    int IntensityToIndex(double intensity);
    double IndexToMinIntensity(int index);
    double IndexToMeanIntensity(int index);
    double IndexToMaxIntensity(int index);

    double m_MinimumRange;
    double m_MaximumRange;
    double m_Stepsize;
    int m_NumberOfBins;
    Eigen::MatrixXd m_Matrix;

  };

  struct CoocurenceMatrixFeatures
  {
    CoocurenceMatrixFeatures() :
      JointMaximum(0),
      JointAverage(0),
      JointVariance(0),
      JointEntropy(0),
      RowMaximum(0),
      RowAverage(0),
      RowVariance(0),
      RowEntropy(0),
      FirstRowColumnEntropy(0),
      SecondRowColumnEntropy(0),
      DifferenceAverage(0),
      DifferenceVariance(0),
      DifferenceEntropy(0),
      SumAverage(0),
      SumVariance(0),
      SumEntropy(0),
      AngularSecondMoment(0),
      Contrast(0),
      Dissimilarity(0),
      InverseDifference(0),
      InverseDifferenceNormalised(0),
      InverseDifferenceMoment(0),
      InverseDifferenceMomentNormalised(0),
      InverseVariance(0),
      Correlation(0),
      Autocorrelation(0),
      ClusterTendency(0),
      ClusterShade(0),
      ClusterProminence(0),
      FirstMeasureOfInformationCorrelation(0),
      SecondMeasureOfInformationCorrelation(0)
    {
    }

  public:
    double JointMaximum;
    double JointAverage;
    double JointVariance;
    double JointEntropy;
    double RowMaximum;
    double RowAverage;
    double RowVariance;
    double RowEntropy;
    double FirstRowColumnEntropy;
    double SecondRowColumnEntropy;
    double DifferenceAverage;
    double DifferenceVariance;
    double DifferenceEntropy;
    double SumAverage;
    double SumVariance;
    double SumEntropy;
    double AngularSecondMoment;
    double Contrast;
    double Dissimilarity;
    double InverseDifference;
    double InverseDifferenceNormalised;
    double InverseDifferenceMoment;
    double InverseDifferenceMomentNormalised;
    double InverseVariance;
    double Correlation;
    double Autocorrelation;
    double ClusterTendency;
    double ClusterShade;
    double ClusterProminence;
    double FirstMeasureOfInformationCorrelation;
    double SecondMeasureOfInformationCorrelation;
  };

}

static
void MatrixFeaturesTo(mitk::CoocurenceMatrixFeatures features,
                      std::string prefix,
                      mitk::GIFCooccurenceMatrix2::FeatureListType &featureList);
static
void CalculateMeanAndStdDevFeatures(std::vector<mitk::CoocurenceMatrixFeatures> featureList,
                                    mitk::CoocurenceMatrixFeatures &meanFeature,
                                    mitk::CoocurenceMatrixFeatures  &stdFeature);
static
void NormalizeMatrixFeature(mitk::CoocurenceMatrixFeatures &features,
                            std::size_t number);




mitk::CoocurenceMatrixHolder::CoocurenceMatrixHolder(double min, double max, int number) :
m_MinimumRange(min),
m_MaximumRange(max),
m_NumberOfBins(number)
{
  m_Matrix.resize(number, number);
  m_Matrix.fill(0);
  m_Stepsize = (max - min) / (number);
}

int mitk::CoocurenceMatrixHolder::IntensityToIndex(double intensity)
{
  int index = std::floor((intensity - m_MinimumRange) / m_Stepsize);
  return std::max(0, std::min(index, m_NumberOfBins - 1));
}

double mitk::CoocurenceMatrixHolder::IndexToMinIntensity(int index)
{
  return m_MinimumRange + index * m_Stepsize;
}
double mitk::CoocurenceMatrixHolder::IndexToMeanIntensity(int index)
{
  return m_MinimumRange + (index+0.5) * m_Stepsize;
}
double mitk::CoocurenceMatrixHolder::IndexToMaxIntensity(int index)
{
  return m_MinimumRange + (index + 1) * m_Stepsize;
}

template<typename TPixel, unsigned int VImageDimension>
void
CalculateCoOcMatrix(itk::Image<TPixel, VImageDimension>* itkImage,
                    itk::Image<unsigned short, VImageDimension>* mask,
                    itk::Offset<VImageDimension> offset,
                    int range,
                    mitk::CoocurenceMatrixHolder &holder)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<unsigned short, VImageDimension> MaskImageType;
  typedef itk::ShapedNeighborhoodIterator<ImageType> ShapeIterType;
  typedef itk::ShapedNeighborhoodIterator<MaskImageType> ShapeMaskIterType;
  typedef itk::ImageRegionConstIterator<ImageType> ConstIterType;
  typedef itk::ImageRegionConstIterator<MaskImageType> ConstMaskIterType;


  itk::Size<VImageDimension> radius;
  radius.Fill(range+1);
  ShapeIterType imageOffsetIter(radius, itkImage, itkImage->GetLargestPossibleRegion());
  ShapeMaskIterType maskOffsetIter(radius, mask, mask->GetLargestPossibleRegion());
  imageOffsetIter.ActivateOffset(offset);
  maskOffsetIter.ActivateOffset(offset);
  ConstIterType imageIter(itkImage, itkImage->GetLargestPossibleRegion());
  ConstMaskIterType maskIter(mask, mask->GetLargestPossibleRegion());
  //  iterator.GetIndex() + ci.GetNeighborhoodOffset()
  auto region = mask->GetLargestPossibleRegion();


  while (!maskIter.IsAtEnd())
  {
    auto ciMask = maskOffsetIter.Begin();
    auto ciValue = imageOffsetIter.Begin();
    if (maskIter.Value() > 0 &&
      ciMask.Get() > 0 &&
      imageIter.Get() == imageIter.Get() &&
      ciValue.Get() == ciValue.Get() &&
      region.IsInside(maskOffsetIter.GetIndex() + ciMask.GetNeighborhoodOffset()))
    {
      int i = holder.IntensityToIndex(imageIter.Get());
      int j = holder.IntensityToIndex(ciValue.Get());
      holder.m_Matrix(i, j) += 1;
      holder.m_Matrix(j, i) += 1;
    }
    ++imageOffsetIter;
    ++maskOffsetIter;
    ++imageIter;
    ++maskIter;
  }
}

void CalculateFeatures(
  mitk::CoocurenceMatrixHolder &holder,
  mitk::CoocurenceMatrixFeatures & results
  )
{
  auto pijMatrix = holder.m_Matrix;
  auto piMatrix = holder.m_Matrix;
  auto pjMatrix = holder.m_Matrix;
  double Ng = holder.m_NumberOfBins;
  int NgSize = holder.m_NumberOfBins;
  pijMatrix /= pijMatrix.sum();
  piMatrix.rowwise().normalize();
  pjMatrix.colwise().normalize();

  for (int i = 0; i < holder.m_NumberOfBins; ++i)
    for (int j = 0; j < holder.m_NumberOfBins; ++j)
    {
      if (pijMatrix(i, j) != pijMatrix(i, j))
        pijMatrix(i, j) = 0;
      if (piMatrix(i, j) != piMatrix(i, j))
        piMatrix(i, j) = 0;
      if (pjMatrix(i, j) != pjMatrix(i, j))
        pjMatrix(i, j) = 0;
    }

  Eigen::VectorXd piVector = pijMatrix.colwise().sum();
  Eigen::VectorXd pjVector = pijMatrix.rowwise().sum();
  double sigmai = 0;;
  for (int i = 0; i < holder.m_NumberOfBins; ++i)
  {
    double iInt = i + 1;// holder.IndexToMeanIntensity(i);
    results.RowAverage += iInt * piVector(i);
    if (piVector(i) > 0)
    {
      results.RowEntropy -= piVector(i) * std::log(piVector(i)) / std::log(2);
    }
  }
  for (int i = 0; i < holder.m_NumberOfBins; ++i)
  {
    double iInt = i + 1; // holder.IndexToMeanIntensity(i);
    results.RowVariance += (iInt - results.RowAverage)*(iInt - results.RowAverage) * piVector(i);
  }
  results.RowMaximum = piVector.maxCoeff();
  sigmai = std::sqrt(results.RowVariance);

  Eigen::VectorXd pimj(NgSize);
  pimj.fill(0);
  Eigen::VectorXd pipj(2*NgSize);
  pipj.fill(0);


  results.JointMaximum += pijMatrix.maxCoeff();

  for (int i = 0; i < holder.m_NumberOfBins; ++i)
  {
    for (int j = 0; j < holder.m_NumberOfBins; ++j)
    {
      //double iInt = holder.IndexToMeanIntensity(i);
      //double jInt = holder.IndexToMeanIntensity(j);
      double iInt = i + 1;// holder.IndexToMeanIntensity(i);
      double jInt = j + 1;// holder.IndexToMeanIntensity(j);
      double pij = pijMatrix(i, j);

      int deltaK = (i - j)>0?(i-j) : (j-i);
      pimj(deltaK) += pij;
      pipj(i + j) += pij;

      results.JointAverage += iInt * pij;
      if (pij > 0)
      {
        results.JointEntropy -= pij * std::log(pij) / std::log(2);
        results.FirstRowColumnEntropy -= pij * std::log(piVector(i)*pjVector(j)) / std::log(2);
      }
      if (piVector(i) > 0 && pjVector(j) > 0 )
      {
        results.SecondRowColumnEntropy -= piVector(i)*pjVector(j) * std::log(piVector(i)*pjVector(j)) / std::log(2);
      }
      results.AngularSecondMoment += pij*pij;
      results.Contrast += (iInt - jInt)* (iInt - jInt) * pij;
      results.Dissimilarity += std::abs<double>(iInt - jInt) * pij;
      results.InverseDifference += pij / (1 + (std::abs<double>(iInt - jInt)));
      results.InverseDifferenceNormalised += pij / (1 + (std::abs<double>(iInt - jInt) / Ng));
      results.InverseDifferenceMoment += pij / (1 + (iInt - jInt)*(iInt - jInt));
      results.InverseDifferenceMomentNormalised += pij / (1 + (iInt - jInt)*(iInt - jInt)/Ng/Ng);
      results.Autocorrelation += iInt*jInt * pij;
      double cluster = (iInt + jInt - 2 * results.RowAverage);
      results.ClusterTendency += cluster*cluster * pij;
      results.ClusterShade += cluster*cluster*cluster * pij;
      results.ClusterProminence += cluster*cluster*cluster*cluster * pij;
      if (iInt != jInt)
      {
        results.InverseVariance += pij / (iInt - jInt) / (iInt - jInt);
      }
    }
  }
  results.Correlation = 1 / sigmai / sigmai * (-results.RowAverage*results.RowAverage+ results.Autocorrelation);
  results.FirstMeasureOfInformationCorrelation = (results.JointEntropy - results.FirstRowColumnEntropy) / results.RowEntropy;
  if (results.JointEntropy < results.SecondRowColumnEntropy)
  {
    results.SecondMeasureOfInformationCorrelation = sqrt(1 - exp(-2 * (results.SecondRowColumnEntropy - results.JointEntropy)));
  }
  else
  {
    results.SecondMeasureOfInformationCorrelation = 0;
  }

  for (int i = 0; i < holder.m_NumberOfBins; ++i)
  {
    for (int j = 0; j < holder.m_NumberOfBins; ++j)
    {
      //double iInt = holder.IndexToMeanIntensity(i);
      //double jInt = holder.IndexToMeanIntensity(j);
      double iInt = i + 1;
      double pij = pijMatrix(i, j);

      results.JointVariance += (iInt - results.JointAverage)* (iInt - results.JointAverage)*pij;
    }
  }

  for (int k = 0; k < NgSize; ++k)
  {
    results.DifferenceAverage += k* pimj(k);
    if (pimj(k) > 0)
    {
      results.DifferenceEntropy -= pimj(k) * log(pimj(k)) / std::log(2);
    }
  }
  for (int k = 0; k < NgSize; ++k)
  {
    results.DifferenceVariance += (results.DifferenceAverage-k)* (results.DifferenceAverage-k)*pimj(k);
  }


  for (int k = 0; k <2* NgSize ; ++k)
  {
    results.SumAverage += (2+k)* pipj(k);
    if (pipj(k) > 0)
    {
      results.SumEntropy -= pipj(k) * log(pipj(k)) / std::log(2);
    }
  }
  for (int k = 0; k < 2*NgSize; ++k)
  {
    results.SumVariance += (2+k - results.SumAverage)* (2+k - results.SumAverage)*pipj(k);
  }

  //MITK_INFO << std::endl << holder.m_Matrix;
  //MITK_INFO << std::endl << pijMatrix;
  //MITK_INFO << std::endl << piMatrix;
  //MITK_INFO << std::endl << pjMatrix;

  //for (int i = 0; i < holder.m_NumberOfBins; ++i)
  //{
  //  MITK_INFO << "Bin " << i << " Min: " << holder.IndexToMinIntensity(i) << " Max: " << holder.IndexToMaxIntensity(i);
  //}
  //MITK_INFO << pimj;
  //MITK_INFO << pipj;

}

template<typename TPixel, unsigned int VImageDimension>
void
CalculateCoocurenceFeatures(itk::Image<TPixel, VImageDimension>* itkImage, mitk::Image::Pointer mask, mitk::GIFCooccurenceMatrix2::FeatureListType & featureList, mitk::GIFCooccurenceMatrix2::GIFCooccurenceMatrix2Configuration config)
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
      offset[i] *= config.range;
      if (config.direction == i + 2 && offset[i] != 0)
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
  }

  std::vector<mitk::CoocurenceMatrixFeatures> resultVector;
  mitk::CoocurenceMatrixHolder holderOverall(rangeMin, rangeMax, numberOfBins);
  mitk::CoocurenceMatrixFeatures overallFeature;
  for (std::size_t i = 0; i < offsetVector.size(); ++i)
  {
    if (config.direction > 1)
    {
      if (offsetVector[i][config.direction - 2] != 0)
      {
        continue;
      }
    }


    offset = offsetVector[i];
    mitk::CoocurenceMatrixHolder holder(rangeMin, rangeMax, numberOfBins);
    mitk::CoocurenceMatrixFeatures coocResults;
    CalculateCoOcMatrix<TPixel, VImageDimension>(itkImage, maskImage, offset, config.range, holder);
    holderOverall.m_Matrix += holder.m_Matrix;
    CalculateFeatures(holder, coocResults);
    resultVector.push_back(coocResults);
  }
  CalculateFeatures(holderOverall, overallFeature);
  //NormalizeMatrixFeature(overallFeature, offsetVector.size());


  mitk::CoocurenceMatrixFeatures featureMean;
  mitk::CoocurenceMatrixFeatures featureStd;
  CalculateMeanAndStdDevFeatures(resultVector, featureMean, featureStd);

  std::ostringstream  ss;
  ss << config.range;
  std::string strRange = ss.str();

  MatrixFeaturesTo(overallFeature, config.prefix + "Overall ", featureList);
  MatrixFeaturesTo(featureMean, config.prefix + "Mean ", featureList);
  MatrixFeaturesTo(featureStd, config.prefix + "Std.Dev. ", featureList);
}


static
void MatrixFeaturesTo(mitk::CoocurenceMatrixFeatures features,
                      std::string prefix,
                      mitk::GIFCooccurenceMatrix2::FeatureListType &featureList)
{
  featureList.push_back(std::make_pair(prefix + "Joint Maximum", features.JointMaximum));
  featureList.push_back(std::make_pair(prefix + "Joint Average", features.JointAverage));
  featureList.push_back(std::make_pair(prefix + "Joint Variance", features.JointVariance));
  featureList.push_back(std::make_pair(prefix + "Joint Entropy", features.JointEntropy));
  featureList.push_back(std::make_pair(prefix + "Difference Average", features.DifferenceAverage));
  featureList.push_back(std::make_pair(prefix + "Difference Variance", features.DifferenceVariance));
  featureList.push_back(std::make_pair(prefix + "Difference Entropy", features.DifferenceEntropy));
  featureList.push_back(std::make_pair(prefix + "Sum Average", features.SumAverage));
  featureList.push_back(std::make_pair(prefix + "Sum Variance", features.SumVariance));
  featureList.push_back(std::make_pair(prefix + "Sum Entropy", features.SumEntropy));
  featureList.push_back(std::make_pair(prefix + "Angular Second Moment", features.AngularSecondMoment));
  featureList.push_back(std::make_pair(prefix + "Contrast", features.Contrast));
  featureList.push_back(std::make_pair(prefix + "Dissimilarity", features.Dissimilarity));
  featureList.push_back(std::make_pair(prefix + "Inverse Difference", features.InverseDifference));
  featureList.push_back(std::make_pair(prefix + "Inverse Difference Normalized", features.InverseDifferenceNormalised));
  featureList.push_back(std::make_pair(prefix + "Inverse Difference Moment", features.InverseDifferenceMoment));
  featureList.push_back(std::make_pair(prefix + "Inverse Difference Moment Normalized", features.InverseDifferenceMomentNormalised));
  featureList.push_back(std::make_pair(prefix + "Inverse Variance", features.InverseVariance));
  featureList.push_back(std::make_pair(prefix + "Correlation", features.Correlation));
  featureList.push_back(std::make_pair(prefix + "Autocorrelation", features.Autocorrelation));
  featureList.push_back(std::make_pair(prefix + "Cluster Tendency", features.ClusterTendency));
  featureList.push_back(std::make_pair(prefix + "Cluster Shade", features.ClusterShade));
  featureList.push_back(std::make_pair(prefix + "Cluster Prominence", features.ClusterProminence));
  featureList.push_back(std::make_pair(prefix + "First Measure of Information Correlation", features.FirstMeasureOfInformationCorrelation));
  featureList.push_back(std::make_pair(prefix + "Second Measure of Information Correlation", features.SecondMeasureOfInformationCorrelation));
  featureList.push_back(std::make_pair(prefix + "Row Maximum", features.RowMaximum));
  featureList.push_back(std::make_pair(prefix + "Row Average", features.RowAverage));
  featureList.push_back(std::make_pair(prefix + "Row Variance", features.RowVariance));
  featureList.push_back(std::make_pair(prefix + "Row Entropy", features.RowEntropy));
  featureList.push_back(std::make_pair(prefix + "First Row-Column Entropy", features.FirstRowColumnEntropy));
  featureList.push_back(std::make_pair(prefix + "Second Row-Column Entropy", features.SecondRowColumnEntropy));
}

static
void CalculateMeanAndStdDevFeatures(std::vector<mitk::CoocurenceMatrixFeatures> featureList,
  mitk::CoocurenceMatrixFeatures &meanFeature,
  mitk::CoocurenceMatrixFeatures  &stdFeature)
{
#define ADDFEATURE(a)                                                                         \
  if ( ! (featureList[i].a == featureList[i].a)) featureList[i].a = 0;                        \
   meanFeature.a += featureList[i].a;stdFeature.a += featureList[i].a*featureList[i].a
#define CALCVARIANCE(a) stdFeature.a =sqrt(stdFeature.a - meanFeature.a*meanFeature.a)

  for (std::size_t i = 0; i < featureList.size(); ++i)
  {
    ADDFEATURE(JointMaximum);
    ADDFEATURE(JointAverage);
    ADDFEATURE(JointVariance);
    ADDFEATURE(JointEntropy);
    ADDFEATURE(RowMaximum);
    ADDFEATURE(RowAverage);
    ADDFEATURE(RowVariance);
    ADDFEATURE(RowEntropy);
    ADDFEATURE(FirstRowColumnEntropy);
    ADDFEATURE(SecondRowColumnEntropy);
    ADDFEATURE(DifferenceAverage);
    ADDFEATURE(DifferenceVariance);
    ADDFEATURE(DifferenceEntropy);
    ADDFEATURE(SumAverage);
    ADDFEATURE(SumVariance);
    ADDFEATURE(SumEntropy);
    ADDFEATURE(AngularSecondMoment);
    ADDFEATURE(Contrast);
    ADDFEATURE(Dissimilarity);
    ADDFEATURE(InverseDifference);
    ADDFEATURE(InverseDifferenceNormalised);
    ADDFEATURE(InverseDifferenceMoment);
    ADDFEATURE(InverseDifferenceMomentNormalised);
    ADDFEATURE(InverseVariance);
    ADDFEATURE(Correlation);
    ADDFEATURE(Autocorrelation);
    ADDFEATURE(ClusterShade);
    ADDFEATURE(ClusterTendency);
    ADDFEATURE(ClusterProminence);
    ADDFEATURE(FirstMeasureOfInformationCorrelation);
    ADDFEATURE(SecondMeasureOfInformationCorrelation);
  }
  NormalizeMatrixFeature(meanFeature, featureList.size());
  NormalizeMatrixFeature(stdFeature, featureList.size());

  CALCVARIANCE(JointMaximum);
  CALCVARIANCE(JointAverage);
  CALCVARIANCE(JointVariance);
  CALCVARIANCE(JointEntropy);
  CALCVARIANCE(RowMaximum);
  CALCVARIANCE(RowAverage);
  CALCVARIANCE(RowVariance);
  CALCVARIANCE(RowEntropy);
  CALCVARIANCE(FirstRowColumnEntropy);
  CALCVARIANCE(SecondRowColumnEntropy);
  CALCVARIANCE(DifferenceAverage);
  CALCVARIANCE(DifferenceVariance);
  CALCVARIANCE(DifferenceEntropy);
  CALCVARIANCE(SumAverage);
  CALCVARIANCE(SumVariance);
  CALCVARIANCE(SumEntropy);
  CALCVARIANCE(AngularSecondMoment);
  CALCVARIANCE(Contrast);
  CALCVARIANCE(Dissimilarity);
  CALCVARIANCE(InverseDifference);
  CALCVARIANCE(InverseDifferenceNormalised);
  CALCVARIANCE(InverseDifferenceMoment);
  CALCVARIANCE(InverseDifferenceMomentNormalised);
  CALCVARIANCE(InverseVariance);
  CALCVARIANCE(Correlation);
  CALCVARIANCE(Autocorrelation);
  CALCVARIANCE(ClusterShade);
  CALCVARIANCE(ClusterTendency);
  CALCVARIANCE(ClusterProminence);
  CALCVARIANCE(FirstMeasureOfInformationCorrelation);
  CALCVARIANCE(SecondMeasureOfInformationCorrelation);

#undef ADDFEATURE
#undef CALCVARIANCE
}

static
void NormalizeMatrixFeature(mitk::CoocurenceMatrixFeatures &features,
                            std::size_t number)
{
  features.JointMaximum = features.JointMaximum / number;
  features.JointAverage = features.JointAverage / number;
  features.JointVariance = features.JointVariance / number;
  features.JointEntropy = features.JointEntropy / number;
  features.RowMaximum = features.RowMaximum / number;
  features.RowAverage = features.RowAverage / number;
  features.RowVariance = features.RowVariance / number;
  features.RowEntropy = features.RowEntropy / number;
  features.FirstRowColumnEntropy = features.FirstRowColumnEntropy / number;
  features.SecondRowColumnEntropy = features.SecondRowColumnEntropy / number;
  features.DifferenceAverage = features.DifferenceAverage / number;
  features.DifferenceVariance = features.DifferenceVariance / number;
  features.DifferenceEntropy = features.DifferenceEntropy / number;
  features.SumAverage = features.SumAverage / number;
  features.SumVariance = features.SumVariance / number;
  features.SumEntropy = features.SumEntropy / number;
  features.AngularSecondMoment = features.AngularSecondMoment / number;
  features.Contrast = features.Contrast / number;
  features.Dissimilarity = features.Dissimilarity / number;
  features.InverseDifference = features.InverseDifference / number;
  features.InverseDifferenceNormalised = features.InverseDifferenceNormalised / number;
  features.InverseDifferenceMoment = features.InverseDifferenceMoment / number;
  features.InverseDifferenceMomentNormalised = features.InverseDifferenceMomentNormalised / number;
  features.InverseVariance = features.InverseVariance / number;
  features.Correlation = features.Correlation / number;
  features.Autocorrelation = features.Autocorrelation / number;
  features.ClusterShade = features.ClusterShade / number;
  features.ClusterTendency = features.ClusterTendency / number;
  features.ClusterProminence = features.ClusterProminence / number;
  features.FirstMeasureOfInformationCorrelation = features.FirstMeasureOfInformationCorrelation / number;
  features.SecondMeasureOfInformationCorrelation = features.SecondMeasureOfInformationCorrelation / number;
}

mitk::GIFCooccurenceMatrix2::GIFCooccurenceMatrix2():
m_Range(1.0)
{
  SetShortName("cooc2");
  SetLongName("cooccurence2");
  SetFeatureClassName("Co-occurenced Based Features");
}

mitk::GIFCooccurenceMatrix2::FeatureListType mitk::GIFCooccurenceMatrix2::CalculateFeatures(const Image::Pointer & image, const Image::Pointer &mask)
{
  InitializeQuantifier(image, mask);

  FeatureListType featureList;

  GIFCooccurenceMatrix2Configuration config;
  config.direction = GetDirection();
  config.range = m_Range;

  config.MinimumIntensity = GetQuantifier()->GetMinimum();
  config.MaximumIntensity = GetQuantifier()->GetMaximum();
  config.Bins = GetQuantifier()->GetBins();
  config.prefix = FeatureDescriptionPrefix();

  AccessByItk_3(image, CalculateCoocurenceFeatures, mask, featureList,config);

  return featureList;
}

mitk::GIFCooccurenceMatrix2::FeatureNameListType mitk::GIFCooccurenceMatrix2::GetFeatureNames()
{
  FeatureNameListType featureList;
  return featureList;
}




void mitk::GIFCooccurenceMatrix2::AddArguments(mitkCommandLineParser &parser)
{
  std::string name = GetOptionPrefix();

  parser.addArgument(GetLongName(), name, mitkCommandLineParser::Bool, "Use Co-occurence matrix", "calculates Co-occurence based features (new implementation)", us::Any());
  parser.addArgument(name+"::range", name+"::range", mitkCommandLineParser::String, "Cooc 2 Range", "Define the range that is used (Semicolon-separated)", us::Any());
  AddQuantifierArguments(parser);
}

void
mitk::GIFCooccurenceMatrix2::CalculateFeaturesUsingParameters(const Image::Pointer & feature, const Image::Pointer &, const Image::Pointer &maskNoNAN, FeatureListType &featureList)
{
  auto parsedArgs = GetParameter();
  std::string name = GetOptionPrefix();

  if (parsedArgs.count(GetLongName()))
  {
    InitializeQuantifierFromParameters(feature, maskNoNAN);
    std::vector<double> ranges;

    if (parsedArgs.count(name + "::range"))
    {
      ranges = SplitDouble(parsedArgs[name + "::range"].ToString(), ';');
    }
    else
    {
      ranges.push_back(1);
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


std::string mitk::GIFCooccurenceMatrix2::GetCurrentFeatureEncoding()
{
  std::ostringstream  ss;
  ss << m_Range;
  std::string strRange = ss.str();
  return QuantifierParameterString() + "_Range-" + ss.str();
}
