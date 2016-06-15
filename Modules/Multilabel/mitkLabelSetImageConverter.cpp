/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include <mitkLabelSetImageConverter.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>
#include <mitkITKImageImport.h>

#include <itkComposeImageFilter.h>
#include <itkExtractImageFilter.h>
#include <itkImageDuplicator.h>
#include <itkVectorIndexSelectionCastImageFilter.h>

template <typename TPixel, unsigned int VDimension>
static void ConvertLabelSetImageToImage(const itk::Image<TPixel, VDimension>*, mitk::LabelSetImage::ConstPointer labelSetImage, mitk::Image::Pointer& image)
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
      auto layerImage = mitk::ImageToItkImage<TPixel, VDimension>(layer != activeLayer
        ? labelSetImage->GetLayerImage(layer)
        : labelSetImage);

      vectorImageComposer->SetInput(layer, layerImage);
    }

    vectorImageComposer->Update();
    image = mitk::GrabItkImageMemory(vectorImageComposer->GetOutput());
  }
  else
  {
    auto layerImage = mitk::ImageToItkImage<TPixel, VDimension>(labelSetImage);

    auto duplicator = DuplicatorType::New();
    duplicator->SetInputImage(layerImage);
    duplicator->Update();

    image = mitk::GrabItkImageMemory(duplicator->GetOutput());
  }
}

mitk::Image::Pointer mitk::ConvertLabelSetImageToImage(LabelSetImage::ConstPointer labelSetImage)
{
  Image::Pointer image;

  if (labelSetImage->GetNumberOfLayers() > 1)
    mitkThrow() << "Multi-layer label set images not yet supported!";

  if (labelSetImage->GetNumberOfLayers() != 0)
  {
    if (labelSetImage->GetDimension() == 4)
    {
      // Multi-layer label set images not yet supported
      // AccessByItk_2(labelSetImage3D, ::ConvertLabelSetImageToImage, labelSetImage, image);

      auto layerImage = mitk::ImageToItkImage<unsigned short, 4>(labelSetImage);

      auto duplicator = itk::ImageDuplicator<itk::Image<unsigned short, 4>>::New(); // TODO: Check why a simple clone isn't working!
      duplicator->SetInputImage(layerImage);
      duplicator->Update();

      // GrabItkImageMemory doesn't support 4D images!
      // image = mitk::GrabItkImageMemory(duplicator->GetOutput());

      auto output = duplicator->GetOutput();

      image = Image::New();
      image->InitializeByItk(output);

      const auto numberOfVoxels = labelSetImage->GetDimension(0) * labelSetImage->GetDimension(1) * labelSetImage->GetDimension(2);

      for (unsigned int t = 0; t < labelSetImage->GetTimeSteps(); ++t)
        image->SetImportVolume(output->GetBufferPointer() + t * numberOfVoxels, t, 0);
    }
    else
    {
      // Multi-layer label set images not yet supported
      // AccessByItk_2(labelSetImage->GetLayerImage(0), ::ConvertLabelSetImageToImage, labelSetImage, image);

      auto layerImage = mitk::ImageToItkImage<unsigned short, 3>(labelSetImage);

      auto duplicator = itk::ImageDuplicator<itk::Image<unsigned short, 3>>::New(); // TODO: Check why a simple clone isn't working!
      duplicator->SetInputImage(layerImage);
      duplicator->Update();

      image = mitk::GrabItkImageMemory(duplicator->GetOutput());
    }
  }

  return image;
}

template <typename TPixel, unsigned int VDimensions>
static void ConvertImageToLabelSetImage(const itk::VectorImage<TPixel, VDimensions>* image, mitk::LabelSetImage::Pointer& labelSetImage)
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
      AccessVectorPixelTypeByItk_n(image, ::ConvertImageToLabelSetImage, (labelSetImage));
    }
    else
    {
      labelSetImage = mitk::LabelSetImage::New();
      labelSetImage->InitializeByLabeledImage(image);
    }
  }

  return labelSetImage;
}
