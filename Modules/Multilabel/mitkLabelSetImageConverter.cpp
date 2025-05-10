/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <numeric>

#include <mitkITKImageImport.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>
#include <mitkLabelSetImageConverter.h>
#include <mitkLabelSetImageHelper.h>

#include <itkComposeImageFilter.h>
#include <itkExtractImageFilter.h>
#include <itkImageDuplicator.h>
#include <itkVectorIndexSelectionCastImageFilter.h>

template <typename TPixel, unsigned int VDimension>
static void ConvertLabelSetImageToImage(const itk::Image<TPixel, VDimension> *,
                                        mitk::MultiLabelSegmentation::ConstPointer labelSetImage,
                                        mitk::Image::Pointer &image)
{
  typedef itk::Image<TPixel, VDimension> ImageType;
  typedef itk::ComposeImageFilter<ImageType> ComposeFilterType;
  typedef itk::ImageDuplicator<ImageType> DuplicatorType;

  auto numberOfLayers = labelSetImage->GetNumberOfGroups();

  if (numberOfLayers > 1)
  {
    auto vectorImageComposer = ComposeFilterType::New();

    for (decltype(numberOfLayers) layer = 0; layer < numberOfLayers; ++layer)
    {
      auto layerImage = mitk::ImageToItkImage<TPixel, VDimension>(
        labelSetImage->GetGroupImage(layer));

      vectorImageComposer->SetInput(layer, layerImage);
    }

    vectorImageComposer->Update();
    // mitk::GrabItkImageMemory does not support 4D, this will handle 4D correctly
    // and create a memory managed copy
    image = mitk::ImportItkImage(vectorImageComposer->GetOutput())->Clone();
  }
  else
  {
    auto layerImage = mitk::ImageToItkImage<TPixel, VDimension>(labelSetImage->GetGroupImage(0));

    auto duplicator = DuplicatorType::New();
    duplicator->SetInputImage(layerImage);
    duplicator->Update();

    // mitk::GrabItkImageMemory does not support 4D, this will handle 4D correctly
    // and create a memory managed copy
    image = mitk::ImportItkImage(duplicator->GetOutput())->Clone();
  }
}

mitk::Image::Pointer mitk::ConvertLabelSetImageToImage(MultiLabelSegmentation::ConstPointer labelSetImage)
{
  Image::Pointer image;

  if (labelSetImage->GetNumberOfGroups() > 0)
  {
    if (labelSetImage->GetDimension() == 4)
    {
      ::ConvertLabelSetImageToImage<MultiLabelSegmentation::LabelValueType, 4>(nullptr, labelSetImage, image);
    }
    else
    {
      AccessByItk_2(labelSetImage->GetGroupImage(0), ::ConvertLabelSetImageToImage, labelSetImage, image);
    }

    image->SetTimeGeometry(labelSetImage->GetTimeGeometry()->Clone());
  }

  return image;
}


template <typename TPixel, unsigned int VDimensions>
static void SplitVectorImage(const itk::VectorImage<TPixel, VDimensions>* image,
  std::vector<mitk::Image::Pointer>& result)
{
  typedef itk::VectorImage<TPixel, VDimensions> VectorImageType;
  typedef itk::Image<TPixel, VDimensions> ImageType;
  typedef itk::VectorIndexSelectionCastImageFilter<VectorImageType, ImageType> VectorIndexSelectorType;

  auto numberOfLayers = image->GetVectorLength();
  for (decltype(numberOfLayers) layer = 0; layer < numberOfLayers; ++layer)
  {
    auto layerSelector = VectorIndexSelectorType::New();
    layerSelector->SetInput(image);
    layerSelector->SetIndex(layer);
    layerSelector->Update();

    mitk::Image::Pointer layerImage = mitk::GrabItkImageMemoryChannel(layerSelector->GetOutput(), nullptr, nullptr, false);
    result.push_back(layerImage);
  }
}

std::vector<mitk::Image::Pointer> mitk::SplitVectorImage(const Image* vecImage)
{
  if (nullptr == vecImage)
  {
    mitkThrow() << "Invalid usage; nullptr passed to SplitVectorImage.";
  }

  if (vecImage->GetChannelDescriptor().GetPixelType().GetPixelType() != itk::IOPixelEnum::VECTOR)
  {
    mitkThrow() << "Invalid usage of SplitVectorImage; passed image is not a vector image. Present pixel type: "<< vecImage->GetChannelDescriptor().GetPixelType().GetPixelTypeAsString();
  }

  std::vector<mitk::Image::Pointer> result;

  if (4 == vecImage->GetDimension())
  {
    AccessVectorFixedDimensionByItk_n(vecImage, ::SplitVectorImage, 4, (result));
  }
  else
  {
    AccessVectorPixelTypeByItk_n(vecImage, ::SplitVectorImage, (result));
  }

  for (auto image : result)
  {
    image->SetTimeGeometry(vecImage->GetTimeGeometry()->Clone());
  }

  return result;
}

mitk::MultiLabelSegmentation::Pointer mitk::ConvertImageToLabelSetImage(Image::Pointer image)
{
  std::vector<mitk::Image::Pointer> groupImages;

  if (image.IsNotNull())
  {
    if (image->GetChannelDescriptor().GetPixelType().GetPixelType() == itk::IOPixelEnum::VECTOR)
    {
      groupImages = SplitVectorImage(image);
    }
    else
    {
      groupImages.push_back(image);
    }
  }
  auto labelSetImage = ConvertImageVectorToLabelSetImage(groupImages, image->GetTimeGeometry());

  return labelSetImage;
}

mitk::MultiLabelSegmentation::Pointer mitk::ConvertImageVectorToLabelSetImage(const std::vector<mitk::Image::Pointer>& images, const mitk::TimeGeometry* timeGeometry)
{
  MultiLabelSegmentation::Pointer labelSetImage = mitk::MultiLabelSegmentation::New();

  for (auto& groupImage : images)
  {
    if (groupImage== images.front())
    {
      labelSetImage->InitializeByLabeledImage(groupImage);
    }
    else
    {
      labelSetImage->AddGroup(groupImage);
    }
  }

  labelSetImage->SetTimeGeometry(timeGeometry->Clone());
  return labelSetImage;
}

mitk::MultiLabelSegmentation::LabelVectorType mitk::GenerateLabelSetWithMappedValues(const MultiLabelSegmentation::ConstLabelVectorType& sourceLabelset, LabelValueMappingVector labelMapping)
{
  MultiLabelSegmentation::LabelVectorType result;

  for (auto oldLabel : sourceLabelset)
  {
    auto finding = std::find_if(labelMapping.begin(), labelMapping.end(), [oldLabel](const std::pair<Label::PixelType, Label::PixelType>& mapping) {return oldLabel->GetValue() == mapping.first; });
    if (finding != labelMapping.end())
    {
      auto clonedLabel = oldLabel->Clone();
      clonedLabel->SetValue(finding->second);
      result.push_back(clonedLabel);
    }
  }
  return result;
}

template <typename SourceImageType>
void ConvertImageToGroupImageInternal(const SourceImageType* sourceImage, mitk::Image* groupImage, mitk::MultiLabelSegmentation::LabelValueVectorType& foundLabels)
{
  using GroupImageType = typename SourceImageType::template Rebind<mitk::MultiLabelSegmentation::LabelValueType>::Type;

  using SourceIteratorType = itk::ImageRegionConstIteratorWithIndex<SourceImageType>;
  using TargetIteratorType = itk::ImageRegionIterator<GroupImageType>;

  auto targetImage = mitk::ImageToItkImage< mitk::MultiLabelSegmentation::LabelValueType, SourceImageType::ImageDimension>(groupImage);

  TargetIteratorType targetIter(targetImage, targetImage->GetRequestedRegion());
  targetIter.GoToBegin();

  SourceIteratorType sourceIter(sourceImage, sourceImage->GetRequestedRegion());
  sourceIter.GoToBegin();

  std::set<mitk::MultiLabelSegmentation::LabelValueType> detectedValues;

  while (!sourceIter.IsAtEnd())
  {
    const auto originalSourceValue = sourceIter.Get();
    const auto sourceValue = static_cast<mitk::MultiLabelSegmentation::LabelValueType>(originalSourceValue);

    if (originalSourceValue > mitk::Label::MAX_LABEL_VALUE)
    {
      mitkThrow() << "Cannot initialize MultiLabelSegmentation by image. Image contains a pixel value that exceeds the label value range. Invalid pixel value:" << originalSourceValue;
    }

    targetIter.Set(sourceValue);

    if (sourceValue != mitk::Label::UNLABELED_VALUE)
      detectedValues.insert(sourceValue);

    ++sourceIter;
    ++targetIter;
  }

  foundLabels.clear();
  foundLabels.insert(foundLabels.begin(), detectedValues.begin(), detectedValues.end());
}

mitk::Image::Pointer mitk::ConvertImageToGroupImage(const Image* inputImage, mitk::MultiLabelSegmentation::LabelValueVectorType& foundLabels)
{
  if (nullptr == inputImage || inputImage->IsEmpty() || !inputImage->IsInitialized())
    mitkThrow() << "Invalid labeled image.";

  auto result = Image::New();
  result->Initialize(mitk::MakePixelType<mitk::MultiLabelSegmentation::LabelValueType, mitk::MultiLabelSegmentation::LabelValueType, 1>(), *(inputImage->GetTimeGeometry()));

  try
  {
    if (result->GetDimension() == 3)
    {
      AccessFixedDimensionByItk_2(inputImage, ConvertImageToGroupImageInternal, 3, result, foundLabels);
    }
    else if (result->GetDimension() == 4)
    {
      AccessFixedDimensionByItk_2(inputImage, ConvertImageToGroupImageInternal, 4, result, foundLabels);
    }
    else
    {
      mitkThrow() << result->GetDimension() << "-dimensional group images not yet supported";
    }
  }
  catch (Exception& e)
  {
    mitkReThrow(e) << "Could not initialize by provided labeled image.";
  }
  catch (...)
  {
    mitkThrow() << "Could not initialize by provided labeled image due to unknown error.";
  }

  return result;
}

bool mitk::CheckForLabelValueConflictsAndResolve(const mitk::MultiLabelSegmentation::LabelValueVectorType& newValues, mitk::MultiLabelSegmentation::LabelValueVectorType& usedLabelValues, mitk::MultiLabelSegmentation::LabelValueVectorType& correctedLabelValues)
{
  bool corrected = false;
  correctedLabelValues.clear();

  for (const auto newV : newValues)
  {
    auto finding = std::find(usedLabelValues.begin(), usedLabelValues.end(), newV);
    if (finding == usedLabelValues.end())
    {
      correctedLabelValues.push_back(newV);
      usedLabelValues.push_back(newV);
    }
    else
    {
      const auto maxValue = *(std::max_element(usedLabelValues.begin(), usedLabelValues.end()));
      if (maxValue == Label::MAX_LABEL_VALUE) mitkThrow() << "Cannot correct label values. All available values are used. A segmentation cannot contain more labels.";

      const auto correctedV = maxValue+1;
      correctedLabelValues.push_back(correctedV);
      usedLabelValues.push_back(correctedV);
      corrected = true;
    }
  }

  return corrected;
}

namespace
{
  template <typename ImageType>
  void ClearBufferProcessing(ImageType* itkImage)
  {
    itkImage->FillBuffer(0);
  }

  void ClearImageBuffer(mitk::Image* image)
  {
    if (image->GetDimension() == 4)
    { //remark: this extra branch was added, because MultiLabelSegmentation instances can be
      //dynamic (4D), but AccessByItk by support only supports 2D and 3D.
      //The option to change the CMake default dimensions for AccessByItk was
      //dropped (for details see discussion in T28756)
      AccessFixedDimensionByItk(image, ClearBufferProcessing, 4);
    }
    else
    {
      AccessByItk(image, ClearBufferProcessing);
    }
  }
}

mitk::Image::Pointer mitk::CreateLabelMask(const MultiLabelSegmentation* segmentation, MultiLabelSegmentation::LabelValueType labelValue, bool createBinaryMap)
{
  if (nullptr==segmentation)
    mitkThrow() << "Error, cannot create label mask. Passed segmentation is nullptr.";
  
  if (!segmentation->ExistLabel(labelValue))
    mitkThrow() << "Error, cannot create label mask. Label ID is invalid. Invalid ID: " << labelValue;

  auto mask = mitk::Image::New();

  // mask->Initialize(segmentation) does not work here if this label set image has a single slice,
  // since the mask would be automatically flattened to a 2-d image, whereas we expect the
  // original dimension of this label set image. Hence, initialize the mask more explicitly:
  mask->Initialize(MultiLabelSegmentation::GetPixelType(), segmentation->GetDimension(), segmentation->GetDimensions().data());
  mask->SetTimeGeometry(segmentation->GetTimeGeometry()->Clone());

  ClearImageBuffer(mask);

  auto destinationLabel = segmentation->GetLabel(labelValue)->Clone();
  if (createBinaryMap) destinationLabel->SetValue(1);

  const auto groupID = segmentation->GetGroupIndexOfLabel(labelValue);
  const auto groupImage = segmentation->GetGroupImage(groupID);

  TransferLabelContent(groupImage,
    mask.GetPointer(),
    { destinationLabel },
    MultiLabelSegmentation::UNLABELED_VALUE,
    MultiLabelSegmentation::UNLABELED_VALUE, false,
    { { labelValue, destinationLabel->GetValue()} },
    MultiLabelSegmentation::MergeStyle::Replace,
    MultiLabelSegmentation::OverwriteStyle::IgnoreLocks);

  return mask;
}

std::pair<mitk::Image::Pointer, mitk::IDToLabelClassNameMapType> mitk::CreateLabelClassMap(const MultiLabelSegmentation* segmentation, MultiLabelSegmentation::GroupIndexType groupID, const MultiLabelSegmentation::LabelValueVectorType& selectedLabels)
{
  if (nullptr == segmentation) mitkThrow() << "Error, cannot create label class map. Passed segmentation is nullptr.";
  if (!segmentation->ExistGroup(groupID)) mitkThrow() << "Error, cannot create label  class map. GroupID is invalid. Invalid ID: " << groupID;

  auto map = mitk::Image::New();

  // map->Initialize(segmentation) does not work here if this label set image has a single slice,
  // since the map would be automatically flattened to a 2-d image, whereas we expect the
  // original dimension of this label set image. Hence, initialize the map more explicitly:
  map->Initialize(MultiLabelSegmentation::GetPixelType(), segmentation->GetDimension(), segmentation->GetDimensions().data());
  map->SetTimeGeometry(segmentation->GetTimeGeometry()->Clone());

  ClearImageBuffer(map);

  // get relevant labels (as intersect of groupLabels and selectedLabels
  auto groupValues = segmentation->GetLabelValuesByGroup(groupID);
  auto relevantDetectLamba = [&selectedLabels](MultiLabelSegmentation::LabelValueVectorType& result, MultiLabelSegmentation::LabelValueType element)
    {
      if (std::find(selectedLabels.begin(), selectedLabels.end(), element) != selectedLabels.end())
      {
        result.push_back(element);
      }
      return result;
    };

  auto relevantGroupValues = std::accumulate(groupValues.begin(),
    groupValues.end(),
    MultiLabelSegmentation::LabelValueVectorType(),
    relevantDetectLamba);

  // construct class mapping
  auto classToValueMap = LabelSetImageHelper::SplitLabelValuesByClassName(segmentation, groupID, relevantGroupValues);

  ConstLabelVector destLabels;
  LabelValueMappingVector transferMapping;
  IDToLabelClassNameMapType classLookUp;

  for (const auto& [className, labelValues] : classToValueMap)
  {
    MultiLabelSegmentation::LabelValueType classValue = classLookUp.size() + 1;
    classLookUp.insert(std::make_pair(classValue, className));
    destLabels.push_back(Label::New(classValue, className));
    for (const auto& labelValue : labelValues)
    {
      transferMapping.emplace_back(std::make_pair(labelValue, classValue));
    }
  }

  TransferLabelContent(segmentation->GetGroupImage(groupID), map.GetPointer(),
    destLabels, MultiLabelSegmentation::UNLABELED_VALUE, MultiLabelSegmentation::UNLABELED_VALUE, false, transferMapping, MultiLabelSegmentation::MergeStyle::Replace, MultiLabelSegmentation::OverwriteStyle::IgnoreLocks);

  return std::make_pair(map, classLookUp);
}

std::pair<mitk::Image::Pointer, mitk::IDToLabelClassNameMapType> mitk::CreateLabelClassMap(const MultiLabelSegmentation* segmentation, MultiLabelSegmentation::GroupIndexType groupID)
{
  return CreateLabelClassMap(segmentation, groupID, segmentation->GetLabelValuesByGroup(groupID));
}
