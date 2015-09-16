#include <mitkGIFCooccurenceMatrix.h>

// MITK
#include <mitkITKImageImport.h>
#include <mitkImageCast.h>
#include <mitkImageAccessByItk.h>

// ITK
#include <itkScalarImageToTextureFeaturesFilter.h>
#include <itkMinimumMaximumImageCalculator.h>

// STL
#include <sstream>

template<typename TPixel, unsigned int VImageDimension>
void
CalculateCoocurenceFeatures(itk::Image<TPixel, VImageDimension>* itkImage, mitk::Image::Pointer mask, mitk::GIFCooccurenceMatrix::FeatureListType & featureList, double range)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<TPixel, VImageDimension> MaskType;
  typedef itk::Statistics::ScalarImageToTextureFeaturesFilter<ImageType> FilterType;
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
    typename FilterType::OffsetType offset = oldOffsetsIterator->Value();
    for (unsigned int i = 0; i < VImageDimension; ++i)
    {
      offset[i] *= range;
    }
    newOffset->push_back(offset);
    oldOffsetsIterator++;
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

  typename MinMaxComputerType::Pointer minMaxComputer = MinMaxComputerType::New();
  minMaxComputer->SetImage(itkImage);
  minMaxComputer->Compute();

  filter->SetInput(itkImage);
  filter->SetMaskImage(maskImage);
  filter->SetRequestedFeatures(requestedFeatures);
  filter->SetPixelValueMinMax(minMaxComputer->GetMinimum(),minMaxComputer->GetMaximum());
  filter->Update();

  auto featureMeans = filter->GetFeatureMeans ();
  auto featureStd = filter->GetFeatureStandardDeviations();

  std::ostringstream  ss;
  ss << range;
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
      default:
        break;
    }
  }

}

mitk::GIFCooccurenceMatrix::GIFCooccurenceMatrix():
  m_Range(1.0)
{
}

mitk::GIFCooccurenceMatrix::FeatureListType mitk::GIFCooccurenceMatrix::CalculateFeatures(const Image::Pointer & image, const Image::Pointer &mask)
{
  FeatureListType featureList;

  AccessByItk_3(image, CalculateCoocurenceFeatures, mask, featureList,m_Range);

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
  return featureList;
}