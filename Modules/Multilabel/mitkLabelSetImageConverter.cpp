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

#include "mitkLabelSetImageConverter.h"

// mitk includes
#include "mitkImageAccessByItk.h"
#include "mitkImageCast.h"
#include <mitkITKImageImport.h>

// itk includes
#include "itkImage.h"
#include "itkVectorImage.h"
#include <itkComposeImageFilter.h>
#include <itkVectorIndexSelectionCastImageFilter.h>

template < typename ImageType >
void VectorOfMitkImagesToMitkVectorImage(ImageType* source, mitk::Image::Pointer &output, mitk::LabelSetImage::Pointer input)
{
  typedef itk::ComposeImageFilter< ImageType > ComposeFilterType;

  unsigned int numberOfLayers = input->GetNumberOfLayers();

  ComposeFilterType::Pointer vectorImageComposer = ComposeFilterType::New();

  for (unsigned int layer(0); layer < numberOfLayers; layer++)
  {
    typename ImageType::Pointer itkCurrentLayer;
    mitk::CastToItkImage(input->GetLayerImage(layer), itkCurrentLayer);

    vectorImageComposer->SetInput(layer, itkCurrentLayer);
  }

  try
  {
    vectorImageComposer->Update();
  }
  catch (const itk::ExceptionObject& e)
  {
    MITK_ERROR << "Caugt exception while updating compose filter: " << e.what();
  }

  output = mitk::GrabItkImageMemory(vectorImageComposer->GetOutput());
}

mitk::Image::Pointer mitk::LabelSetImageConverter::ConvertLabelSetImageToImage(const mitk::LabelSetImage::Pointer input)
{
  unsigned int numberOfLayers = input->GetNumberOfLayers();

  if (numberOfLayers == 0)
  {
    mitkThrow() << "Tried to convert LabelSetImage without layers";
  }

  mitk::Image::Pointer output;
  AccessByItk_2(input->GetLayerImage(0), VectorOfMitkImagesToMitkVectorImage, output, input);

  return output;
}

template<typename TPixel, unsigned int VDimensions>
void MitkImageToMitkLabelSetImage(itk::Image< TPixel, VDimensions> * source, mitk::LabelSetImage::Pointer &output)
{
  // do nothing for non-vector images
}

template<typename TPixel, unsigned int VDimensions>
void MitkImageToMitkLabelSetImage(itk::VectorImage< TPixel, VDimensions> * source, mitk::LabelSetImage::Pointer &output)
{
  typedef itk::VectorImage< TPixel, VDimensions > VectorImageType;
  typedef itk::Image< TPixel, VDimensions > ImageType;
  typedef itk::VectorIndexSelectionCastImageFilter< VectorImageType, ImageType > VectorIndexSelectorType;

  unsigned int numberOfComponents = source->GetVectorLength();

  if (numberOfComponents < 1)
  {
    mitkThrow() << "At least one Component is required."
  }

  typename VectorIndexSelectorType::Pointer vectorIndexSelector = typename VectorIndexSelectorType::New();

  VectorIndexSelector->SetIndex(0);
  VectorIndexSelector->SetInput(source);

  mitk::Image::Pointer tempImage;
  mitk::GrabItkImageMemory(VectorIndexSelector->GetOutput(), tempImage);

  output = mitk::LabelSetImage::New();
  output->InitializeByLabeledImage(tempImage);

  for (unsigned int layer = 1; layer < numberOfComponents; ++layer)
  {
    typename VectorIndexSelectorType::Pointer vectorIndexSelectorLoop = typename VectorIndexSelectorType::New();
    VectorIndexSelectorLoop->SetIndex(layer);
    VectorIndexSelector->SetInput(source);

    mitk::Image::Pointer loopImage;
    mitk::GrabItkImageMemory(vectorIndexSelectorLoop->GetOutput(), loopImage);

    output->AddLayer(loopImage);
  }

}

mitk::LabelSetImage::Pointer mitk::LabelSetImageConverter::ConvertImageToLabelSetImage(const mitk::Image::Pointer input)
{
  mitk::LabelSetImage::Pointer output;

  AccessByItk_1(input, MitkImageToMitkLabelSetImage, output);

  // if no vector image was detected
  if ( output.IsNull() )
  {
    output = mitk::LabelSetImage::New();
    output->InitializeByLabeledImage( input );
  }

  return output;
}
