/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkITKImageImport.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>
#include <mitkLabelSetImageConverter.h>

#include <itkComposeImageFilter.h>
#include <itkExtractImageFilter.h>
#include <itkImageDuplicator.h>
#include <itkVectorIndexSelectionCastImageFilter.h>

template <typename TPixel, unsigned int VDimension>
static void ConvertLabelSetImageToImage(const itk::Image<TPixel, VDimension> *,
                                        mitk::LabelSetImage::ConstPointer labelSetImage,
                                        mitk::Image::Pointer &image)
{
  typedef itk::Image<TPixel, VDimension> ImageType;
  typedef itk::ComposeImageFilter<ImageType> ComposeFilterType;
  typedef itk::ImageDuplicator<ImageType> DuplicatorType;

  auto numberOfLayers = labelSetImage->GetNumberOfLayers();

  if (numberOfLayers > 1)
  {
    auto vectorImageComposer = ComposeFilterType::New();
    auto activeLayer = labelSetImage->GetActiveLayer();

    for (decltype(numberOfLayers) layer = 0; layer < numberOfLayers; ++layer)
    {
      auto layerImage = mitk::ImageToItkImage<TPixel, VDimension>(
        layer != activeLayer ? labelSetImage->GetLayerImage(layer) : labelSetImage);

      vectorImageComposer->SetInput(layer, layerImage);
    }

    vectorImageComposer->Update();
    // mitk::GrabItkImageMemory does not support 4D, this will handle 4D correctly
    // and create a memory managed copy
    image = mitk::ImportItkImage(vectorImageComposer->GetOutput())->Clone();
  }
  else
  {
    auto layerImage = mitk::ImageToItkImage<TPixel, VDimension>(labelSetImage);

    auto duplicator = DuplicatorType::New();
    duplicator->SetInputImage(layerImage);
    duplicator->Update();

    // mitk::GrabItkImageMemory does not support 4D, this will handle 4D correctly
    // and create a memory managed copy
    image = mitk::ImportItkImage(duplicator->GetOutput())->Clone();
  }
}

mitk::Image::Pointer mitk::ConvertLabelSetImageToImage(LabelSetImage::ConstPointer labelSetImage)
{
  Image::Pointer image;

  if (labelSetImage->GetNumberOfLayers() > 0)
  {
    if (labelSetImage->GetDimension() == 4)
    {
      AccessFixedDimensionByItk_n(labelSetImage, ::ConvertLabelSetImageToImage, 4, (labelSetImage, image));
    }
    else
    {
      AccessByItk_2(labelSetImage->GetLayerImage(0), ::ConvertLabelSetImageToImage, labelSetImage, image);
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

mitk::LabelSetImage::Pointer mitk::ConvertImageToLabelSetImage(Image::Pointer image)
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

mitk::LabelSetImage::Pointer mitk::ConvertImageVectorToLabelSetImage(const std::vector<mitk::Image::Pointer>& images, const mitk::TimeGeometry* timeGeometry)
{
  LabelSetImage::Pointer labelSetImage = mitk::LabelSetImage::New();

  for (auto& groupImage : images)
  {
    if (groupImage== images.front())
    {
      labelSetImage->InitializeByLabeledImage(groupImage);
    }
    else
    {
      labelSetImage->AddLayer(groupImage);
    }
  }

  labelSetImage->SetTimeGeometry(timeGeometry->Clone());
  return labelSetImage;
}

mitk::LabelSet::Pointer mitk::GenerateLabelSetWithMappedValues(const LabelSet* sourceLabelset, std::vector<std::pair<Label::PixelType, Label::PixelType> > labelMapping)
{
  if (nullptr == sourceLabelset)
  {
    mitkThrow() << "Invalid usage; nullptr passed as labelset to GenerateLabelSetWithMappedValues.";
  }

  auto result = LabelSet::New();

  for (auto [sourceLabelID, destLabelID] : labelMapping)
  {
    auto clonedLabel = sourceLabelset->GetLabel(sourceLabelID)->Clone();
    clonedLabel->SetValue(destLabelID);
    result->AddLabel(clonedLabel, false);
  }
  result->SetLayer(sourceLabelset->GetLayer());

  return result;
}
