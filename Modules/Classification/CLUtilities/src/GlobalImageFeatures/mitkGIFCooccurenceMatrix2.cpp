#include <mitkGIFCooccurenceMatrix2.h>

// MITK
#include <mitkITKImageImport.h>
#include <mitkImageCast.h>
#include <mitkImageAccessByItk.h>

// ITK
#include <itkEnhancedScalarImageToTextureFeaturesFilter.h>
#include <itkMinimumMaximumImageCalculator.h>
#include <itkShapedNeighborhoodIterator.h>
#include <itkImageRegionConstIterator.h>

// STL
#include <sstream>

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
  return std::floor((intensity - m_MinimumRange) / m_Stepsize);
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
                    itk::Image<unsigned char, VImageDimension>* mask,
                    itk::Offset<VImageDimension> offset,
                    int range,
                    mitk::CoocurenceMatrixHolder &holder)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<unsigned char, VImageDimension> MaskImageType;
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

  results.JointMaximum += pijMatrix.maxCoeff();


  //MITK_INFO << std::endl << holder.m_Matrix;
  //MITK_INFO << std::endl << pijMatrix;
  //MITK_INFO << std::endl << piMatrix;
  //MITK_INFO << std::endl << pjMatrix;

  //for (int i = 0; i < holder.m_NumberOfBins; ++i)
  //{
  //  MITK_INFO << "Bin " << i << " Min: " << holder.IndexToMinIntensity(i) << " Max: " << holder.IndexToMaxIntensity(i);
  //}


}

template<typename TPixel, unsigned int VImageDimension>
void
CalculateCoocurenceFeatures(itk::Image<TPixel, VImageDimension>* itkImage, mitk::Image::Pointer mask, mitk::GIFCooccurenceMatrix2::FeatureListType & featureList, mitk::GIFCooccurenceMatrix2::GIFCooccurenceMatrix2Configuration config)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<unsigned char, VImageDimension> MaskType;
  typedef itk::MinimumMaximumImageCalculator<ImageType> MinMaxComputerType;
  typedef itk::Neighborhood<TPixel, VImageDimension > NeighborhoodType;
  typedef itk::Offset<VImageDimension> OffsetType;

  typedef itk::Statistics::EnhancedScalarImageToTextureFeaturesFilter<ImageType> FilterType;

  typedef typename FilterType::TextureFeaturesFilterType TextureFilterType;



  ///////////////////////////////////////////////////////////////////////////////////////////////

  typename MinMaxComputerType::Pointer minMaxComputer = MinMaxComputerType::New();
  minMaxComputer->SetImage(itkImage);
  minMaxComputer->Compute();

  double rangeMin = -0.5 + minMaxComputer->GetMinimum();
  double rangeMax = 0.5 + minMaxComputer->GetMaximum();

  // Define Range
  int numberOfBins = 6;

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
    for (int i = 0; i < VImageDimension; ++i)
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

  mitk::CoocurenceMatrixFeatures coocResults;
  coocResults.JointAverage = 0;
  coocResults.JointMaximum = 0;

  mitk::CoocurenceMatrixHolder holder(rangeMin, rangeMax, numberOfBins);
  for (int i = 0; i < offsetVector.size(); ++i)
  {
    offset = offsetVector[i];
    MITK_INFO << offset;
    CalculateCoOcMatrix<TPixel, VImageDimension>(itkImage, maskImage, offset, config.range, holder);
  }
  CalculateFeatures(holder, coocResults);
  //coocResults.JointMaximum /= offsetVector.size();
  MITK_INFO << coocResults.JointMaximum;

  ///////////////////////////////////////////////////////////////////////////////////////////////
  /*

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
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") Energy Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") Energy Std.",featureStd->ElementAt(i)));
      break;
    case TextureFilterType::Entropy :
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") Entropy Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") Entropy Std.",featureStd->ElementAt(i)));
      break;
    case TextureFilterType::Correlation :
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") Correlation Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") Correlation Std.",featureStd->ElementAt(i)));
      break;
    case TextureFilterType::InverseDifferenceMoment :
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") InverseDifferenceMoment Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") InverseDifferenceMoment Std.",featureStd->ElementAt(i)));
      break;
    case TextureFilterType::Inertia :
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") Inertia Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") Inertia Std.",featureStd->ElementAt(i)));
      break;
    case TextureFilterType::ClusterShade :
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") ClusterShade Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") ClusterShade Std.",featureStd->ElementAt(i)));
      break;
    case TextureFilterType::ClusterProminence :
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") ClusterProminence Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") ClusterProminence Std.",featureStd->ElementAt(i)));
      break;
    case TextureFilterType::HaralickCorrelation :
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") HaralickCorrelation Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") HaralickCorrelation Std.",featureStd->ElementAt(i)));
      break;
    case TextureFilterType::Autocorrelation :
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") Autocorrelation Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") Autocorrelation Std.",featureStd->ElementAt(i)));
      break;
    case TextureFilterType::Contrast :
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") Contrast Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") Contrast Std.",featureStd->ElementAt(i)));
      break;
    case TextureFilterType::Dissimilarity :
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") Dissimilarity Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") Dissimilarity Std.",featureStd->ElementAt(i)));
      break;
    case TextureFilterType::MaximumProbability :
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") MaximumProbability Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") MaximumProbability Std.",featureStd->ElementAt(i)));
      break;
    case TextureFilterType::InverseVariance :
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") InverseVariance Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") InverseVariance Std.",featureStd->ElementAt(i)));
      break;
    case TextureFilterType::Homogeneity1 :
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") Homogeneity1 Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") Homogeneity1 Std.",featureStd->ElementAt(i)));
      break;
    case TextureFilterType::ClusterTendency :
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") ClusterTendency Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") ClusterTendency Std.",featureStd->ElementAt(i)));
      break;
    case TextureFilterType::Variance :
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") Variance Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") Variance Std.",featureStd->ElementAt(i)));
      break;
    case TextureFilterType::SumAverage :
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") SumAverage Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") SumAverage Std.",featureStd->ElementAt(i)));
      break;
    case TextureFilterType::SumEntropy :
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") SumEntropy Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") SumEntropy Std.",featureStd->ElementAt(i)));
      break;
    case TextureFilterType::SumVariance :
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") SumVariance Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") SumVariance Std.",featureStd->ElementAt(i)));
      break;
    case TextureFilterType::DifferenceAverage :
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") DifferenceAverage Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") DifferenceAverage Std.",featureStd->ElementAt(i)));
      break;
    case TextureFilterType::DifferenceEntropy :
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") DifferenceEntropy Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") DifferenceEntropy Std.",featureStd->ElementAt(i)));
      break;
    case TextureFilterType::DifferenceVariance :
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") DifferenceVariance Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") DifferenceVariance Std.",featureStd->ElementAt(i)));
      break;
    case TextureFilterType::InverseDifferenceMomentNormalized :
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") InverseDifferenceMomentNormalized Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") InverseDifferenceMomentNormalized Std.",featureStd->ElementAt(i)));
      break;
    case TextureFilterType::InverseDifferenceNormalized :
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") InverseDifferenceNormalized Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") InverseDifferenceNormalized Std.",featureStd->ElementAt(i)));
      break;
    case TextureFilterType::InverseDifference :
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") InverseDifference Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") InverseDifference Std.",featureStd->ElementAt(i)));
      break;
    case TextureFilterType::JointAverage :
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") JointAverage Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") JointAverage Std.",featureStd->ElementAt(i)));
      break;
    case TextureFilterType::FirstMeasureOfInformationCorrelation :
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") FirstMeasureOfInformationCorrelation Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") FirstMeasureOfInformationCorrelation Std.",featureStd->ElementAt(i)));
      break;
    case TextureFilterType::SecondMeasureOfInformationCorrelation :
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") SecondMeasureOfInformationCorrelation Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("co-occ. ("+ strRange+") SecondMeasureOfInformationCorrelation Std.",featureStd->ElementAt(i)));
      break;
    default:
      break;
    }
  }*/
}

mitk::GIFCooccurenceMatrix2::GIFCooccurenceMatrix2():
m_Range(1.0), m_Direction(0)
{
}

mitk::GIFCooccurenceMatrix2::FeatureListType mitk::GIFCooccurenceMatrix2::CalculateFeatures(const Image::Pointer & image, const Image::Pointer &mask)
{
  FeatureListType featureList;

  GIFCooccurenceMatrix2Configuration config;
  config.direction = m_Direction;
  config.range = m_Range;

  AccessByItk_3(image, CalculateCoocurenceFeatures, mask, featureList,config);

  return featureList;
}

mitk::GIFCooccurenceMatrix2::FeatureNameListType mitk::GIFCooccurenceMatrix2::GetFeatureNames()
{
  FeatureNameListType featureList;
  featureList.push_back("co-occ. Energy Means");
  featureList.push_back("co-occ. Energy Std.");
  featureList.push_back("co-occ. Entropy Means");
  featureList.push_back("co-occ. Entropy Std.");
  featureList.push_back("co-occ. Correlation Means");
  featureList.push_back("co-occ. Correlation Std.");
  featureList.push_back("co-occ. InverseDifferenceMoment Means");
  featureList.push_back("co-occ. InverseDifferenceMoment Std.");
  featureList.push_back("co-occ. Inertia Means");
  featureList.push_back("co-occ. Inertia Std.");
  featureList.push_back("co-occ. ClusterShade Means");
  featureList.push_back("co-occ. ClusterShade Std.");
  featureList.push_back("co-occ. ClusterProminence Means");
  featureList.push_back("co-occ. ClusterProminence Std.");
  featureList.push_back("co-occ. HaralickCorrelation Means");
  featureList.push_back("co-occ. HaralickCorrelation Std.");
  featureList.push_back("co-occ. Autocorrelation Means");
  featureList.push_back("co-occ. Autocorrelation Std.");
  featureList.push_back("co-occ. Contrast Means");
  featureList.push_back("co-occ. Contrast Std.");
  featureList.push_back("co-occ. Dissimilarity Means");
  featureList.push_back("co-occ. Dissimilarity Std.");
  featureList.push_back("co-occ. MaximumProbability Means");
  featureList.push_back("co-occ. MaximumProbability Std.");
  featureList.push_back("co-occ. InverseVariance Means");
  featureList.push_back("co-occ. InverseVariance Std.");
  featureList.push_back("co-occ. Homogeneity1 Means");
  featureList.push_back("co-occ. Homogeneity1 Std.");
  featureList.push_back("co-occ. ClusterTendency Means");
  featureList.push_back("co-occ. ClusterTendency Std.");
  featureList.push_back("co-occ. Variance Means");
  featureList.push_back("co-occ. Variance Std.");
  featureList.push_back("co-occ. SumAverage Means");
  featureList.push_back("co-occ. SumAverage Std.");
  featureList.push_back("co-occ. SumEntropy Means");
  featureList.push_back("co-occ. SumEntropy Std.");
  featureList.push_back("co-occ. SumVariance Means");
  featureList.push_back("co-occ. SumVariance Std.");
  featureList.push_back("co-occ. DifferenceAverage Means");
  featureList.push_back("co-occ. DifferenceAverage Std.");
  featureList.push_back("co-occ. DifferenceEntropy Means");
  featureList.push_back("co-occ. DifferenceEntropy Std.");
  featureList.push_back("co-occ. DifferenceVariance Means");
  featureList.push_back("co-occ. DifferenceVariance Std.");
  featureList.push_back("co-occ. InverseDifferenceMomentNormalized Means");
  featureList.push_back("co-occ. InverseDifferenceMomentNormalized Std.");
  featureList.push_back("co-occ. InverseDifferenceNormalized Means");
  featureList.push_back("co-occ. InverseDifferenceNormalized Std.");
  featureList.push_back("co-occ. InverseDifference Means");
  featureList.push_back("co-occ. InverseDifference Std.");
  featureList.push_back("co-occ. JointAverage Means");
  featureList.push_back("co-occ. JointAverage Std.");
  featureList.push_back("co-occ. FirstMeasurementOfInformationCorrelation Means");
  featureList.push_back("co-occ. FirstMeasurementOfInformationCorrelation Std.");
  featureList.push_back("co-occ. SecondMeasurementOfInformationCorrelation Means");
  featureList.push_back("co-occ. SecondMeasurementOfInformationCorrelation Std.");
  return featureList;
}
