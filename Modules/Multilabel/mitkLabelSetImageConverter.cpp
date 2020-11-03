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
static void ConvertImageToLabelSetImage(const itk::VectorImage<TPixel, VDimensions> *image,
                                        mitk::LabelSetImage::Pointer &labelSetImage)
{
  typedef itk::VectorImage<TPixel, VDimensions> VectorImageType;
  typedef itk::Image<TPixel, VDimensions> ImageType;
  typedef itk::VectorIndexSelectionCastImageFilter<VectorImageType, ImageType> VectorIndexSelectorType;

  labelSetImage = mitk::LabelSetImage::New();

  auto numberOfLayers = image->GetVectorLength();
  for (decltype(numberOfLayers) layer = 0; layer < numberOfLayers; ++layer)
  {
    auto layerSelector = VectorIndexSelectorType::New();
    layerSelector->SetInput(image);
    layerSelector->SetIndex(layer);
    layerSelector->Update();

    mitk::Image::Pointer layerImage;
    mitk::CastToMitkImage(layerSelector->GetOutput(), layerImage);

    if (layer == 0)
    {
      labelSetImage->InitializeByLabeledImage(layerImage);
    }
    else
    {
      labelSetImage->AddLayer(layerImage);
    }
  }
}

mitk::LabelSetImage::Pointer mitk::ConvertImageToLabelSetImage(Image::Pointer image)
{
  LabelSetImage::Pointer labelSetImage;

  if (image.IsNotNull())
  {
    if (image->GetChannelDescriptor().GetPixelType().GetPixelType() == itk::ImageIOBase::VECTOR)
    {
      if (4 == image->GetDimension())
      {
        AccessVectorFixedDimensionByItk_n(image, ::ConvertImageToLabelSetImage, 4, (labelSetImage));
      }
      else
      {
        AccessVectorPixelTypeByItk_n(image, ::ConvertImageToLabelSetImage, (labelSetImage));
      }
    }
    else
    {
      labelSetImage = mitk::LabelSetImage::New();
      labelSetImage->InitializeByLabeledImage(image);
    }
    labelSetImage->SetTimeGeometry(image->GetTimeGeometry()->Clone());
  }

  return labelSetImage;
}
