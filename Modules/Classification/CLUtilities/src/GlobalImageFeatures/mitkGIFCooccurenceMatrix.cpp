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

template<typename TPixel, unsigned int VImageDimension>
void
CalculateCoocurenceFeatures(itk::Image<TPixel, VImageDimension>* itkImage, mitk::Image::Pointer mask, mitk::GIFCooccurenceMatrix::FeatureListType & featureList, mitk::GIFCooccurenceMatrix::GIFCooccurenceMatrixConfiguration config)
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

  typename MinMaxComputerType::Pointer minMaxComputer = MinMaxComputerType::New();
  minMaxComputer->SetImage(itkImage);
  minMaxComputer->Compute();

  filter->SetInput(itkImage);
  filter->SetMaskImage(maskImage);
  filter->SetRequestedFeatures(requestedFeatures);
  filter->SetPixelValueMinMax(minMaxComputer->GetMinimum()-0.5,minMaxComputer->GetMaximum()+0.5);
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
    default:
      break;
    }
  }
}

mitk::GIFCooccurenceMatrix::GIFCooccurenceMatrix():
m_Range(1.0), m_Direction(0)
{
}

mitk::GIFCooccurenceMatrix::FeatureListType mitk::GIFCooccurenceMatrix::CalculateFeatures(const Image::Pointer & image, const Image::Pointer &mask)
{
  FeatureListType featureList;

  GIFCooccurenceMatrixConfiguration config;
  config.direction = m_Direction;
  config.range = m_Range;

  AccessByItk_3(image, CalculateCoocurenceFeatures, mask, featureList,config);

  return featureList;
}

mitk::GIFCooccurenceMatrix::FeatureNameListType mitk::GIFCooccurenceMatrix::GetFeatureNames()
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
  return featureList;
}