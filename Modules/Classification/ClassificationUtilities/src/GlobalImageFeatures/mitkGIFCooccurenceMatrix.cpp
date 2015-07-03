#include <mitkGIFco-occurenceMatrix.h>

// MITK
#include <mitkITKImageImport.h>
#include <mitkImageCast.h>
#include <mitkImageAccessByItk.h>

// ITK
#include <itkScalarImageToTextureFeaturesFilter.h>

template<typename TPixel, unsigned int VImageDimension>
void
CalculateCoocurenceFeatures(itk::Image<TPixel, VImageDimension>* itkImage, mitk::Image::Pointer mask, mitk::GIFco-occurenceMatrix::FeatureListType & featureList)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<TPixel, VImageDimension> MaskType;
  typedef itk::Statistics::ScalarImageToTextureFeaturesFilter<ImageType> FilterType;
  typedef typename FilterType::TextureFeaturesFilterType TextureFilterType;

  typename MaskType::Pointer maskImage = MaskType::New();
  mitk::CastToItkImage(mask, maskImage);

  // All features are required
  typename FilterType::FeatureNameVectorPointer requestedFeatures = typename FilterType::FeatureNameVector::New();
  requestedFeatures->push_back(TextureFilterType::Energy);
  requestedFeatures->push_back(TextureFilterType::Entropy);
  requestedFeatures->push_back(TextureFilterType::Correlation);
  requestedFeatures->push_back(TextureFilterType::InverseDifferenceMoment);
  requestedFeatures->push_back(TextureFilterType::Inertia);
  requestedFeatures->push_back(TextureFilterType::ClusterShade);
  requestedFeatures->push_back(TextureFilterType::ClusterProminence);
  requestedFeatures->push_back(TextureFilterType::HaralickCorrelation);


  typename FilterType::Pointer filter = FilterType::New();
  filter->SetInput(itkImage);
  filter->SetMaskImage(maskImage);
  filter->SetRequestedFeatures(requestedFeatures);
  filter->Update();

  auto featureMeans = filter->GetFeatureMeans ();
  auto featureStd = filter->GetFeatureStandardDeviations();

  for (int i = 0; i < featureMeans->size(); ++i)
  {
    switch (i)
    {
      case TextureFilterType::Energy :
        featureList.push_back(std::make_pair("co-occ. Energy Means",featureMeans->ElementAt(i)));
        featureList.push_back(std::make_pair("co-occ. Energy Std.",featureStd->ElementAt(i)));
        break;
      case TextureFilterType::Entropy :
        featureList.push_back(std::make_pair("co-occ. Entropy Means",featureMeans->ElementAt(i)));
        featureList.push_back(std::make_pair("co-occ. Entropy Std.",featureStd->ElementAt(i)));
        break;
      case TextureFilterType::Correlation :
        featureList.push_back(std::make_pair("co-occ. Correlation Means",featureMeans->ElementAt(i)));
        featureList.push_back(std::make_pair("co-occ. Correlation Std.",featureStd->ElementAt(i)));
        break;
      case TextureFilterType::InverseDifferenceMoment :
        featureList.push_back(std::make_pair("co-occ. InverseDifferenceMoment Means",featureMeans->ElementAt(i)));
        featureList.push_back(std::make_pair("co-occ. InverseDifferenceMoment Std.",featureStd->ElementAt(i)));
        break;
      case TextureFilterType::Inertia :
        featureList.push_back(std::make_pair("co-occ. Inertia Means",featureMeans->ElementAt(i)));
        featureList.push_back(std::make_pair("co-occ. Inertia Std.",featureStd->ElementAt(i)));
        break;
      case TextureFilterType::ClusterShade :
        featureList.push_back(std::make_pair("co-occ. ClusterShade Means",featureMeans->ElementAt(i)));
        featureList.push_back(std::make_pair("co-occ. ClusterShade Std.",featureStd->ElementAt(i)));
        break;
      case TextureFilterType::ClusterProminence :
        featureList.push_back(std::make_pair("co-occ. ClusterProminence Means",featureMeans->ElementAt(i)));
        featureList.push_back(std::make_pair("co-occ. ClusterProminence Std.",featureStd->ElementAt(i)));
        break;
      case TextureFilterType::HaralickCorrelation :
        featureList.push_back(std::make_pair("co-occ. HaralickCorrelation Means",featureMeans->ElementAt(i)));
        featureList.push_back(std::make_pair("co-occ. HaralickCorrelation Std.",featureStd->ElementAt(i)));
        break;
      default:
        break;
    }
  }

}


mitk::GIFco-occurenceMatrix::FeatureListType mitk::GIFco-occurenceMatrix::CalculateFeatures(const Image::Pointer & image, const Image::Pointer &mask)
{
  FeatureListType featureList;

  AccessByItk_2(image, CalculateCoocurenceFeatures, mask, featureList);

  return featureList;
}

mitk::GIFco-occurenceMatrix::FeatureNameListType mitk::GIFco-occurenceMatrix::GetFeatureNames()
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